import json
import bpy
import os
import sys

dir = os.path.dirname(__file__)
if not dir in sys.path:
    sys.path.append(dir)


from utils import remap_param_type


def get_geometry_nodes_params(obj):
    """Get exposed geometry nodes params, defaults from OBJECT instead of node group, so user gets exactly whats in the blend file"""
    params = []
    for modifier in obj.modifiers:
        if modifier.type == "NODES":
            if modifier.node_group:
                for input in (
                    modifier.node_group.inputs
                    if bpy.app.version < (4, 0, 0)
                    else modifier.node_group.interface.items_tree
                ):
                    # ignore panels
                    if (
                        bpy.app.version >= (4, 0, 0)
                        and type(input) == bpy.types.NodeTreeInterfacePanel
                    ):
                        continue

                    # get input nodes
                    if bpy.app.version < (4, 0, 0) or input.in_out == "INPUT":
                        input_type = (
                            input.type
                            if bpy.app.version < (4, 0, 0)
                            else remap_param_type(input.socket_type)
                        )

                        # Ignored
                        if input_type == "MATERIAL" or input_type == "GEOMETRY":
                            continue

                        new_param = {}
                        new_param["NodeGroup"] = modifier.name
                        new_param["Id"] = input.identifier
                        new_param["Name"] = input.name
                        new_param["Type"] = input_type
                        new_param["Panel"] = ""
                        if bpy.app.version >= (4, 0, 0):
                            new_param["Panel"] = input.parent.name
                        new_param["Tooltip"] = input.description

                        if input_type == "MENU":
                            new_param["Entries"] = [x[0] for x in modifier.id_properties_ui(input.identifier).as_dict()['items']]
                            new_param["Values"] = [x[4] for x in modifier.id_properties_ui(input.identifier).as_dict()['items']]

                        if (
                            input_type == "INT"
                            or input_type == "VALUE"
                            or input_type == "VECTOR"
                        ):
                            new_param["MinValue"] = input.min_value
                            new_param["MaxValue"] = input.max_value

                        if input_type == "VECTOR" or input_type == "ROTATION":
                            new_param["DefaultValue"] = [
                                modifier[input.identifier][0],
                                modifier[input.identifier][1],
                                modifier[input.identifier][2],
                            ]
                        elif input_type == "RGBA":
                            new_param["DefaultValue"] = [
                                modifier[input.identifier][0],
                                modifier[input.identifier][1],
                                modifier[input.identifier][2],
                                modifier[input.identifier][3],
                            ]

                        if (
                            input_type == "INT"
                            or input_type == "VALUE"
                            or input_type == "BOOLEAN"
                            or input_type == "STRING"
                            or input_type == "MENU"
                        ):
                            new_param["DefaultValue"] = modifier[input.identifier]

                        params += [new_param]
    return params


def get_materials():
    names = []
    for material in bpy.data.materials[:]:
        names += [material.name]
    return names


geometry_nodes_objs = []
for obj in bpy.data.objects:
    for modifier in obj.modifiers:
        if modifier.type == "NODES" and obj not in geometry_nodes_objs:
            geometry_nodes_objs += [obj]

if len(geometry_nodes_objs) < 1:
    print("No geometry nodes object found", file=sys.stderr)

import ctypes

from library import setup_dll

guids = sys.argv[sys.argv.index("--") + 1 :]
setup_dll(guids[0], guids[1])

from library import AlterMesh, AlterMeshHandle, Writer
from utils import get_geometry_types
import numpy as np

while True:
    if AlterMesh.WriteLock(AlterMeshHandle):
        Writer(np.int32).from_value(len(geometry_nodes_objs))

        for obj in geometry_nodes_objs:
            Writer().from_buffer(bytes(obj.name, "UTF-16-LE"))

            export_params = {
                "Params": get_geometry_nodes_params(obj),
                "Materials": get_materials(),
            }

            print(export_params, flush=True)
            Writer().from_buffer(bytes(json.dumps(export_params), "UTF-16-LE"))

        # export geometry defaults (eg. curve defaults when a curve is assigned in the modifiers tab)
        geometry_types = get_geometry_types()
        for modifier in obj.modifiers:
            if modifier.type == "NODES":
                if modifier.node_group:
                    for input in (
                        modifier.node_group.inputs
                        if bpy.app.version < (4, 0, 0)
                        else modifier.node_group.interface.items_tree
                    ):
                        has_default = False

                        # ignore panels
                        if (
                            bpy.app.version >= (4, 0, 0)
                            and type(input) == bpy.types.NodeTreeInterfacePanel
                        ):
                            continue

                        # get input types
                        input_type = (
                            input.type
                            if bpy.app.version < (4, 0, 0)
                            else remap_param_type(input.socket_type)
                        )

                        if input_type == "OBJECT":
                            default_geometry_type = None

                            # Find which class we should use for defaults
                            for geometry_type in geometry_types:
                                if modifier[
                                    input.identifier
                                ] is not None and geometry_type.used_for_object(
                                    modifier[input.identifier]
                                ):
                                    default_geometry_type = geometry_type
                                    break

                            geometry_type_name_bytes = (
                                bytes(default_geometry_type.__name__, "UTF-16-LE")
                                if default_geometry_type is not None
                                else b""
                            )
                            Writer().from_buffer(geometry_type_name_bytes)

                            # Let it write defaults
                            if default_geometry_type is not None:
                                default_geometry_type.get_defaults(
                                    modifier[input.identifier]
                                )

        AlterMesh.WriteUnlock(AlterMeshHandle)
        break

import time

time.sleep(1)
AlterMesh.Free(AlterMeshHandle)
