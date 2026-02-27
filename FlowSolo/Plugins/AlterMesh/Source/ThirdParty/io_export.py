# ##### BEGIN GPL LICENSE BLOCK #####
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software Foundation,
#  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# ##### END GPL LICENSE BLOCK #####

import bpy
import json
import ctypes
import numpy as np
import bpy_types
from array import array
from mathutils import Matrix, Vector
from utils import get_geometry_nodes_obj, remap_param_type
from library import AlterMesh, AlterMeshHandle, Writer


def get_materials(mesh):
    names = []
    for material in mesh.materials[:]:
        if material is not None:
            names += [material.name_full]
        else:
            names += ["Default"]
    return names


def get_mesh_data(mesh):
    mesh.calc_loop_triangles()
    
    locations = np.empty(len(mesh.vertices) * 3, dtype=np.float32)
    mesh.vertices.foreach_get("co", locations)
    
    if bpy.app.version < (4, 1, 0):
        mesh.calc_normals_split()
        normals = np.empty(len(mesh.loop_triangles) * 3 * 3, dtype=np.float32)
        mesh.loop_triangles.foreach_get("split_normals", normals)
    else:
        normals = np.empty(len(mesh.corner_normals) * 3, dtype=np.float32)
        mesh.corner_normals.foreach_get("vector", normals)

    # converted normals have a small inconsistency with the original normals
    # causing verts to be split and become different than original mesh
    normals = np.round(normals, 4)

    indices = np.empty(len(mesh.loop_triangles) * 3, dtype=np.int32)
    mesh.loop_triangles.foreach_get("vertices", indices)

    triangle_loops = np.empty(len(mesh.loop_triangles) * 3, dtype=np.int32)
    mesh.loop_triangles.foreach_get("loops", triangle_loops)

    loops = np.empty(len(mesh.loops), dtype=np.int32)
    mesh.loops.foreach_get("vertex_index", loops)

    materials = np.empty(len(mesh.loop_triangles), dtype=np.int32)
    mesh.loop_triangles.foreach_get("material_index", materials)

    material_names = {"Materials": get_materials(mesh)}
    material_names = json.dumps(material_names)

    mesh_hash = np.asarray(hash(mesh), np.int64)
    material_names = np.frombuffer(bytes(material_names, "UTF-16-LE"), np.byte)
    version = np.asarray(list(bpy.app.version))

    return (
        locations,
        normals,
        indices,
        triangle_loops,
        loops,
        materials,
        material_names,
        mesh_hash,
        version,
    )


def get_mesh_attributes(mesh, attribute_names):
    allowed_types = {
        "FLOAT": {"accessor": "value", "components": 1, "type": np.float32},
        "INT": {"accessor": "value", "components": 1, "type": np.int32},
        "FLOAT_VECTOR": {"accessor": "vector", "components": 3, "type": np.float32},
        "BOOLEAN": {"accessor": "value", "components": 1, "type": np.bool8},
        "FLOAT2": {"accessor": "vector", "components": 2, "type": np.float32},
        "INT8": {"accessor": "value", "components": 1, "type": np.int8},
        "INT32_2D": {"accessor": "vector", "components": 2, "type": np.int32},
        "FLOAT_COLOR": {"accessor": "color", "components": 4, "type": np.float32},
        "BYTE_COLOR": {"accessor": "color", "components": 4, "type": np.float32},
    }

    allowed_domains = {
        "POINT",
        "CORNER",
    }

    attributes = []
    attribute_indexing = []

    for attribute_name in attribute_names:
        attribute = mesh.attributes.get(attribute_name)
        if (
            attribute
            and len(attribute.data) > 0
            and attribute.data_type in allowed_types
            and attribute.domain in allowed_domains
        ):
            num_components = allowed_types[attribute.data_type]["components"]
            attr = np.empty(
                len(attribute.data) * num_components,
                dtype=allowed_types[attribute.data_type]["type"],
            )
            attribute.data.foreach_get(
                allowed_types[attribute.data_type]["accessor"], attr
            )

            # make attribute vec4
            num_rows = int(len(attr) / num_components)
            attr = attr.reshape(num_rows, num_components)
            column = np.zeros(
                (num_rows, 1),
                dtype=allowed_types[attribute.data_type]["type"],
            )
            for i in range(4 - num_components):
                attr = np.concatenate((attr, column), 1)

            attributes += [attr.astype(np.float32)]
            attribute_indexing += [attribute.domain == "POINT"]
        else:
            attr = np.zeros(len(mesh.vertices) * 4, dtype=np.float32)
            attributes += [attr]
            attribute_indexing += [True]

    attribute_indexing = np.asarray(attribute_indexing, np.bool8)
    attributes += [attribute_indexing]

    return attributes

def get_instance_data(
    geometry_nodes_obj, mesh, original_mesh, matrix_local, matrix_world
):
    if original_mesh is not None:
        asset_path = np.asarray(original_mesh.get('path_id', 0), np.uint64)
    else:
        asset_path = np.asarray(0, np.uint64)

    matrix_local = np.array(matrix_local, np.float32)
    matrix_world = np.array(matrix_world, np.float32)
    mesh_hash = np.asarray(hash(mesh), np.int64)

    return asset_path, matrix_local, matrix_world, mesh_hash


def export(geometry_nodes_obj, attributes):
    if AlterMesh.WriteLock(AlterMeshHandle):
        print("Exporting")

        depsgraph = bpy.context.evaluated_depsgraph_get()
        evaluated_geometry_nodes_data = geometry_nodes_obj.evaluated_get(depsgraph).data

        mesh_hashes = []
        mesh_datablocks = []
        instance_datablocks = []
        instance_original_datablocks = []
        instance_local_matrices = []
        instance_world_matrices = []

        # Get number of meshes and instances
        if geometry_nodes_obj.type == "MESH":
            mesh_hashes = [hash(geometry_nodes_obj)]
            mesh_datablocks = [evaluated_geometry_nodes_data]
            instance_datablocks = [evaluated_geometry_nodes_data]
            instance_original_datablocks = [None]
            instance_local_matrices = [
                Matrix() * bpy.context.scene.unit_settings.scale_length
            ]
            instance_world_matrices = [
                geometry_nodes_obj.matrix_world.copy()
                * bpy.context.scene.unit_settings.scale_length
            ]

        for instance in depsgraph.object_instances:
            if (
                instance.instance_object
                and instance.parent
                and instance.parent.original == geometry_nodes_obj
            ):
                if instance.object.type == "MESH":
                    # No need to export instances that aren't realized
                    if hash(instance.object.data) not in mesh_hashes:
                        mesh_hashes += [hash(instance.object.data)]
                        mesh_datablocks += [instance.object.data]
                    if hash(instance.object.data) in mesh_hashes:
                        instance_datablocks += [instance.object.data]
                        instance_original_datablocks += [instance.object.original.data]
                        local_matrix = (
                            instance.parent.matrix_world.inverted()
                            @ instance.matrix_world
                        )
                        instance_local_matrices += [
                            local_matrix * bpy.context.scene.unit_settings.scale_length
                        ]
                        instance_world_matrices += [
                            instance.matrix_world.copy()
                            * bpy.context.scene.unit_settings.scale_length
                        ]

        Writer(np.int32).from_value(len(mesh_datablocks))
        Writer(np.int32).from_value(len(instance_datablocks))

        attribute_names = []
        for attribute in attributes:
            attribute_names += [attribute["AttributeName"]]

        # Export meshes
        for mesh in mesh_datablocks:
            for array in get_mesh_data(mesh):
                Writer().from_array(array)
            for array in get_mesh_attributes(mesh, attribute_names):
                Writer().from_array(array)

        # Export Instances
        for mesh, original_mesh, matrix_local, matrix_world in zip(
            instance_datablocks,
            instance_original_datablocks,
            instance_local_matrices,
            instance_world_matrices,
        ):
            for array in get_instance_data(
                geometry_nodes_obj, mesh, original_mesh, matrix_local, matrix_world
            ):
                Writer().from_array(array)

        AlterMesh.WriteUnlock(AlterMeshHandle)
