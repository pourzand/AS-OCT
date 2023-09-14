# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from qia.common.img.element cimport _Element
from qia.common.img.image cimport _Image, ImageObject, get_nan, get_infinity
from qia.common.img.common cimport shared_ptr, Point3D, Region3D, raisePyError
from qia.common.img.image import Type
from numbers import Number

cdef extern from "filter.h":
    ImageObject* connectedComponentsFilter(ImageObject*, const shared_ptr[vector[Point3D[int]]]&, const string&) nogil except +raisePyError
    ImageObject* gaussianFilter(ImageObject*, double, double, double, bint, int, double, double) nogil except +raisePyError
    ImageObject* medianFilter(ImageObject*, double, double, double) nogil except +raisePyError
    ImageObject* n4BiasFieldCorrFilter(ImageObject* input, ImageObject* input_mask) nogil except +raisePyError
    #vector[ImageObject*] projectionDecompositionFilter(ImageObject*, double, double, double, int, double, const vector[double]&, double, const Region3D[int]&, const string&, bint) nogil except +raisePyError
    
def connected_components_filter(_Image image, _Element elem, type=None):
    if type is None:
        type = Type.int
    cdef string ctype = type.value.encode("utf-8")
    ret = _Image()
    with nogil:
        ret.ptr = connectedComponentsFilter(image.ptr, elem.ptr, ctype)
    return ret
    
def gaussian_filter(_Image image, sigma, bint use_image_spacing=True, int max_kernel_width=33, double kernel_cutoff=0.0001, outval=None):
    cdef double sigmax, sigmay, sigmaz, coutval
    if isinstance(sigma, Number):
        sigmax = sigma
        sigmay = sigma
        sigmaz = sigma
    else:
        sigmax = sigma[0]
        if len(sigma)>1:
            sigmay = sigma[1]
        if len(sigma)>2:
            sigmaz = sigma[2]
    if outval is None:
        coutval = get_infinity()
    else:
        coutval = outval
    ret = _Image()
    with nogil:
        ret.ptr = gaussianFilter(image.ptr, sigmax, sigmay, sigmaz, use_image_spacing, max_kernel_width, kernel_cutoff, coutval)
    return ret
    
def median_filter(_Image image, radius):
    cdef double radx, rady, radz
    if isinstance(radius, Number):
        radx = radius
        rady = radius
        radz = radius
    else:
        radx = radius[0]
        if len(radius)>1:
            rady = radius[1]
        if len(radius)>2:
            radz = radius[2]
    ret = _Image()
    with nogil:
        ret.ptr = medianFilter(image.ptr, radx, rady, radz)
    return ret

def n4_bias_field_correction_filter(_Image image, _Image mask):
    ret = _Image()
    with nogil:
        ret.ptr = n4BiasFieldCorrFilter(image.ptr, mask.ptr)
    return ret

"""
def projection_decomposition_filter(_Image image, double delta=50, double lambda_=1, double epsilon=0.01, int max=55, double mu=450, tau=None, region=None, outval=0, type=None, bint use_3d=False):
    if type is None:
        type = Type.float
    cdef string c_type = type.value.encode("utf-8")
    cdef double c_outval = get_nan()
    cdef Region3D[int] c_region
    cdef Point3D[int] minp
    cdef Point3D[int] maxp
    cdef vector[double] c_tau

    if outval is not None:
        c_outval = outval
        
    if tau is None:
        if use_3d:
            c_tau = vector[double](50, 0.05)
        else:
            c_tau = vector[double](50, 0.25)
    else:
        c_tau.reserve(len(tau))
        for t in tau:
            c_tau.push_back(t)

    if region is None:
        c_region.reset()
    else:
        minp.set(region[0][0], region[0][1], region[0][2])
        maxp.set(region[1][0], region[1][1], region[1][2])
        c_region.set(minp, maxp)
    
    cdef vector[ImageObject*] c_result
    with nogil:
        c_result = projectionDecompositionFilter(image.ptr, delta, epsilon, lambda_, max, mu, c_tau, c_outval, c_region, c_type, use_3d)

    geometric = _Image()
    geometric.ptr = c_result[0]
    texture = _Image()
    texture.ptr = c_result[1]
    noise = _Image()
    noise.ptr = c_result[2]
    return {
        "geometric": geometric,
        "texture": texture,
        "noise": noise
    }
"""