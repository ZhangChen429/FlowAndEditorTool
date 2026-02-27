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
from library import Reader
import numpy as np

import GeometryTypes.AlterMeshGeometryAsset
import GeometryTypes.AlterMeshGeometrySpline

def import_obj(object_name):
    
    NumSplines = Reader(np.int32).as_value()
    
    if NumSplines > 0:    
        imported_splines = []
        for i in range(NumSplines):
            imported_splines += [GeometryTypes.AlterMeshGeometrySpline.import_obj(object_name + "_" + str(i))]

        bpy.ops.object.select_all(action='DESELECT')
        for spline in imported_splines:
            spline.select_set(True)
            
        bpy.context.view_layer.objects.active = imported_splines[0]
        bpy.ops.object.join()            
        return imported_splines[0]
    else:
        obj = GeometryTypes.AlterMeshGeometryAsset.import_obj(object_name)    
        return obj
    
def used_for_object(obj):
    return False

def get_defaults(obj):
    pass