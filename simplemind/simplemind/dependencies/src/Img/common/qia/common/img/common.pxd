# distutils: language = c++

from libcpp.vector cimport vector

cdef extern from "boost/shared_ptr.hpp" namespace "boost":
    cppclass shared_ptr[T]:
        T* get() nogil
        void reset(T*) nogil

cdef extern from "pcl/geometry/Point.h" namespace "pcl":
    cdef cppclass Point3D[T]:
        Point3D() nogil
        Point3D(T,T,T) nogil
        T x() nogil
        T y() nogil
        T z() nogil
        Point3D& set(T,T,T) nogil

cdef extern from "pcl/geometry/Region3D.h" namespace "pcl":
    cdef cppclass Region3D[T]:
        Region3D() nogil
        Region3D(const Point3D[T]&, const Point3D[T]&) nogil
        void set(const Point3D[T]&, const Point3D[T]&) nogil
        const Point3D[T]& getMinPoint() nogil
        const Point3D[T]& getMaxPoint() nogil
        Region3D[T]& reset() nogil
        bint contain(const Region3D[T]&) nogil
        Region3D[T]& add(const Region3D[T]&) nogil

cdef extern from "pcl/exception/python.h":
    cdef void raisePyError() nogil