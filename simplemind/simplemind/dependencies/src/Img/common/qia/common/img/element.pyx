# distutils: language = c++

from libcpp.vector cimport vector
from qia.common.img.common cimport shared_ptr, Point3D
import warnings

cdef extern from "element.h":
    cdef shared_ptr[vector[Point3D[int]]] getEllipsoidElement(int, int, int, bint) 
    cdef shared_ptr[vector[Point3D[int]]] getCubeElement(int, int, int, bint)
    cdef shared_ptr[vector[Point3D[int]]] getConnect6Element(bint)

def new_ellipsoid_element(int rx, int ry, int rz, bint skip_origin=True):
    elem = _Element()
    elem.ptr = getEllipsoidElement(rx, ry, rz, skip_origin)
    return elem

def new_cube_element(int x, int y, int z, bint skip_origin=True):
    elem = _Element()
    elem.ptr = getCubeElement(x, y, z, skip_origin)
    return elem
    
def new_connect6_element(bint skip_origin=True):
    elem = _Element()
    elem.ptr = getConnect6Element(skip_origin)
    return elem
    
def new(iter):
    cdef Point3D[int] temp
    cdef vector[Point3D[int]] *ptr = new vector[Point3D[int]]()
    for x,y,z in iter:
        temp.set(x,y,z)
        ptr.push_back(temp)
    elem = _Element()
    elem.ptr.reset(ptr)
    return elem

cdef class _Element:
    def __iter__(self):
        return iter(self.to_list())

    def to_list(self):
        point_list = []
        cdef int i
        cdef Point3D[int] temp
        for i in range(0, self.ptr.get().size()):
            temp = self.ptr.get()[0][i]
            point_list.append((temp.x(), temp.y(), temp.z()))
        return point_list