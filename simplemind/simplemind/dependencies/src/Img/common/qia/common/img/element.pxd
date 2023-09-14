from libcpp.vector cimport vector
from qia.common.img.common cimport shared_ptr, Point3D

cdef class _Element:
    cdef shared_ptr[vector[Point3D[int]]] ptr