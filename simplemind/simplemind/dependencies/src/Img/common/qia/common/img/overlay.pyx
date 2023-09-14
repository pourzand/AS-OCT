# distutils: language = c++

from enum import Enum
from libcpp.string cimport string
from qia.common.img.common cimport Point3D, Region3D
from qia.common.img.image cimport ImageObject, _Image, boost_tuple, get0, get1
from qia.common.img.lut cimport CLookupTable, LookupTable

cdef extern from "overlay.h" namespace "pcl::misc":
    cdef cppclass ColorImageGenerator:
        ColorImageGenerator(const Point3D[double]&, double, double, double, const Point3D[double]&, const Point3D[double]&) nogil
        void save(const string&) nogil except +
        
cdef extern from "overlay.h":
    cdef void setImage(ColorImageGenerator&, const ImageObject&, double, double, bint, bint, double) nogil
    cdef void setImage(ColorImageGenerator&, const ImageObject&, CLookupTable*, bint, bint, double) nogil
    cdef void addImage(ColorImageGenerator&, const ImageObject&, CLookupTable*, bint, bint, double) nogil
    cdef void addOverlay(ColorImageGenerator&, const ImageObject&, unsigned char, unsigned char, unsigned char, double, bint, bint, double) nogil
    ColorImageGenerator* autoNew(ImageObject*, const Point3D[double]&, const Region3D[double]&, const Point3D[double]&, const Point3D[double]&, double) nogil
    ColorImageGenerator* autoNew(const Point3D[double]&, const Region3D[double]&, const Point3D[double]&, const Point3D[double]&, double) nogil
    boost_tuple[Point3D[double], Point3D[double]] getExtent(ImageObject*, const Point3D[double]&) nogil
    Point3D[double] getCentroid(ImageObject*) nogil
    
cdef class _ColorImageGen:
    cdef ColorImageGenerator *ptr

    def __dealloc__(self):
        del self.ptr
        
    def set(self, _Image obj, minval=None, maxval=None, lut=None, nn=True, boundval=None):
        cdef double bval
        cdef bint use_fixed
        if boundval is not None:
            bval = boundval
            use_fixed = True
        else:
            bval = 0
            use_fixed = False
            
        if lut is not None:
            setImage(self.ptr[0], obj.ptr[0], (<LookupTable>lut).ptr, nn, use_fixed, bval)
        else:
            setImage(self.ptr[0], obj.ptr[0], minval, maxval, nn, use_fixed, bval)
        
    def add(self, _Image obj, color=None, alpha=1, lut=None, nn=True, boundval=None):
        cdef double bval
        cdef bint use_fixed
        if boundval is not None:
            bval = boundval
            use_fixed = True
        else:
            bval = 0
            use_fixed = False
            
        if lut is not None:
            addImage(self.ptr[0], obj.ptr[0], (<LookupTable>lut).ptr, nn, use_fixed, bval)
        else:
            addOverlay(self.ptr[0], obj.ptr[0], color[0], color[1], color[2], alpha, nn, use_fixed, bval)
            
    def write(self, file):
        self.ptr.save(str(file).encode("utf-8"))
        
class CrossSection(Enum):
    axial=1
    coronal=2
    sagittal=3
    custom=4
        
def new(origin, double width_mm, double height_mm, double spacing_mm, crsstype=CrossSection.axial, xaxis=None, yaxis=None):
    cdef Point3D[double] corigin
    cdef Point3D[double] cxaxis
    cdef Point3D[double] cyaxis
    corigin.set(origin[0], origin[1], origin[2])
    if crsstype is CrossSection.axial:
        cxaxis.set(1,0,0)
        cyaxis.set(0,1,0)
    elif crsstype is CrossSection.coronal:
        cxaxis.set(1,0,0)
        cyaxis.set(0,0,-1)
    elif crsstype is CrossSection.sagittal:
        cxaxis.set(0,1,0)
        cyaxis.set(0,0,-1)
    else:
        cxaxis.set(xaxis[0], xaxis[1], xaxis[2])
        cyaxis.set(yaxis[0], yaxis[1], yaxis[2])
    if crsstype is not CrossSection.custom:
        if xaxis is not None:
            cxaxis.set(xaxis[0], xaxis[1], xaxis[2])
        if yaxis is not None:
            cyaxis.set(yaxis[0], yaxis[1], yaxis[2])
    ret_obj = _ColorImageGen()
    ret_obj.ptr = new ColorImageGenerator(corigin, width_mm, height_mm, spacing_mm, cxaxis, cyaxis)
    return ret_obj
    
def auto(point, _Image image=None, crsstype=CrossSection.axial, xaxis=None, yaxis=None, region=None, spacing_mm=None):
    cdef Point3D[double] cpoint
    cdef Point3D[double] cxaxis
    cdef Point3D[double] cyaxis
    cdef Point3D[double] cmin
    cdef Point3D[double] cmax
    cdef Region3D[double] cregion
    if region is None and image is None:
        raise ValueError('"image" or "region" cannot both be None!')
    if image is None and spacing_mm is None:
        raise ValueError('"spacing_mm" must be provided if "image" is None!')
    cpoint.set(point[0], point[1], point[2])
    if crsstype is CrossSection.axial:
        cxaxis.set(1,0,0)
        cyaxis.set(0,1,0)
    elif crsstype is CrossSection.coronal:
        cxaxis.set(1,0,0)
        cyaxis.set(0,0,-1)
    elif crsstype is CrossSection.sagittal:
        cxaxis.set(0,1,0)
        cyaxis.set(0,0,-1)
    else:
        cxaxis.set(xaxis[0], xaxis[1], xaxis[2])
        cyaxis.set(yaxis[0], yaxis[1], yaxis[2])
    if crsstype is not CrossSection.custom:
        if xaxis is not None:
            cxaxis.set(xaxis[0], xaxis[1], xaxis[2])
        if yaxis is not None:
            cyaxis.set(yaxis[0], yaxis[1], yaxis[2])
    if region is not None:
        cmin.set(region[0][0], region[0][1], region[0][2])
        cmax.set(region[1][0], region[1][1], region[1][2])
        cregion.set(cmin, cmax)
    else:
        cregion.reset()
    ret_obj = _ColorImageGen()
    if image is not None:
        if spacing_mm is None:
            spacing_mm = -1
        ret_obj.ptr = autoNew(image.ptr, cpoint, cregion, cxaxis, cyaxis, spacing_mm)
    else:
        ret_obj.ptr = autoNew(cpoint, cregion, cxaxis, cyaxis, spacing_mm)
        
    return ret_obj
    
def get_extent(_Image obj, normal):
    cdef Point3D[double] cnormal
    cdef boost_tuple[Point3D[double], Point3D[double]] result
    cdef Point3D[double] minp
    cdef Point3D[double] maxp
    cnormal.set(normal[0], normal[1], normal[2])
    result = getExtent(obj.ptr, cnormal)
    minp = get0[Point3D[double]](result)
    maxp = get1[Point3D[double]](result)
    return (minp.x(), minp.y(), minp.z()), (maxp.x(), maxp.y(), maxp.z())
    
def get_centroid(_Image obj):
    cdef Point3D[double] result = getCentroid(obj.ptr)
    return (result.x(), result.y(), result.z())