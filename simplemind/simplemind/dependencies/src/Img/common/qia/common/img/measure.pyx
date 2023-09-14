# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from qia.common.img.element cimport _Element
from qia.common.img.image cimport _Image, ImageObject, get_nan
from qia.common.img.common cimport shared_ptr, Point3D, Region3D, raisePyError
from qia.common.img.image import Type


cdef extern from "measure.h":
	vector[Point3D[double]] airWayMeasurement(ImageObject* image, const Point3D[double]& centroid, double num_division) nogil except +raisePyError
 

def get_inner_outer_radius(_Image image, centroid, division=10): 
	cdef Point3D[double] ccentroid
	ccentroid.set(centroid[0], centroid[1], centroid[2])		
	cdef double cdivision=division
	cdef vector[Point3D[double]] res
	res = airWayMeasurement(image.ptr, ccentroid, cdivision)
	ret = []
	combo = []
	for i in range(res.size()):
		ret.append((res[i].x(),res[i].y(),res[i].z()))
	for j in range(0,len(ret),3):
		combo.append((ret[j],ret[j+1],ret[j+2]))
	return combo