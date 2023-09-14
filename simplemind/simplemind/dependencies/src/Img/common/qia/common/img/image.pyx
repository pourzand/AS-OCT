# distutils: language = c++

from enum import Enum
from libcpp.string cimport string
from libcpp.vector cimport vector
from qia.common.img.common cimport shared_ptr, Point3D, Region3D
from qia.common.img.element cimport _Element
from qia.common.img.statistics cimport StatisticsCalculator, PercentileCalculator, _PercCalc, _StatCalc
from cython.operator cimport dereference as deref, preincrement 
import warnings

import numpy as np
cimport numpy as np
np.import_array()

init()

class Type(Enum):
    char = "char"
    uchar = "uchar"
    short = "short"
    ushort = "ushort"
    int = "int"
    uint = "uint"
    long = "long"
    ulong = "ulong"
    float = "float"
    double = "double"
    dummy = "dummy"
    auto = "auto"

NP_DTYPE_LOOKUP = {
    np.dtype(np.int8): Type.char,
    np.dtype(np.uint8): Type.uchar,
    np.dtype(np.int16): Type.short,
    np.dtype(np.uint16): Type.ushort,
    np.dtype(np.int32): Type.int,
    np.dtype(np.uint32): Type.uint,
    np.dtype(np.int64): Type.long,
    np.dtype(np.uint64): Type.ulong,
    np.dtype(np.float32): Type.float,
    np.dtype(np.float64): Type.double
}


cdef class _GlcmFeatures:
    def __dealloc__(self):
        del self.ptr
        
    def matrix(self):
        ret_val = []
        res = self.ptr.getMatrix()
        for r in range(res.rows()):
            ret_val.append([res.get(r,c) for c in range(res.columns())])
        return ret_val
        
    def contrast(self):
        return self.ptr.getContrast()

    def dissimilarity(self):
        return self.ptr.getDissimilarity()
        
    def homogeneity(self):
        return self.ptr.getHomogeneity()
        
    def angular_2nd_moment(self):
        return self.ptr.getAngularSecondMoment()
    
    def energy(self):
        return self.ptr.getEnergy()
        
    def max(self):
        return self.ptr.getMax()
        
    def min(self):
        return self.ptr.getMin()
        
    def entropy(self):
        return self.ptr.getEntropy()
        
    def mean(self):
        return self.ptr.getMean()
        
    def variance(self):
        return self.ptr.getVariance()
        
    def correlation(self):
        return self.ptr.getCorrelation()
        
    def sum_average(self):
        return self.ptr.getSumAverage()
        
    def sum_variance(self):
        return self.ptr.getSumVariance()
        
    def sum_entropy(self):
        return self.ptr.getSumEntropy()
        
    def diff_average(self):
        return self.ptr.getDiffAverage()
    
    def diff_variance(self):
        return self.ptr.getDiffVariance()
        
    def diff_entropy(self):
        return self.ptr.getDiffEntropy()
        
    def info_corr_a(self):
        return self.ptr.getInfoCorrelationA()
        
    def info_corr_b(self):
        return self.ptr.getInfoCorrelationB()
        
    def max_corr_coef(self):
        return self.ptr.getMaximalCorrelationCoefficient()
    
cdef class _Image:
    def __dealloc__(self):
        del self.ptr
        
    def get_type(self):
        return Type[self.ptr.getTypeString().c_str().decode('utf-8')]
        
    def get_value(self, p):
        return self.ptr.getValue(p[0],p[1],p[2])
        
    def get_values(self, iter, reversecoord=False):
        result = []
        cdef int x
        cdef int y
        cdef int z
        if reversecoord:
            for z,y,x in iter:
                result.append(self.ptr.getValue(x,y,z))
        else:
            for x,y,z in iter:
                result.append(self.ptr.getValue(x,y,z))
        return result
        
    def set_value(self, p, double val):
        if self.ptr.getTypeString().c_str().decode('utf-8')=="dummy":
            raise ValueError("Cannot set dummy image!")
        self.ptr.setValue(p[0],p[1],p[2],val)
        
    def set_values(self, iter, reversecoord=False):
        cdef int x
        cdef int y
        cdef int z
        cdef double v
        if self.ptr.getTypeString().c_str().decode('utf-8')=="dummy":
            raise ValueError("Cannot set dummy image!")
        if reversecoord:
            for z,y,x,v in iter:
                self.ptr.setValue(x,y,z,v)
        else:
            for x,y,z,v in iter:
                self.ptr.setValue(x,y,z,v)

    def to_physical_coordinates(self, p):
        cdef Point3D[double] a = self.ptr.toPhysicalCoordinates(p[0],p[1],p[2])
        return (a.x(), a.y(), a.z())
        
    def to_image_coordinates(self, p):
        cdef Point3D[double] a = self.ptr.toImageCoordinates(p[0],p[1],p[2])
        return (a.x(), a.y(), a.z())
        
    def get_size(self):
        cdef Point3D[int] a = self.ptr.getSize()
        return (a.x(), a.y(), a.z())
        
    def get_min_point(self):
        cdef Point3D[int] a = self.ptr.getMinPoint()
        return (a.x(), a.y(), a.z())
        
    def get_max_point(self):
        cdef Point3D[int] a = self.ptr.getMaxPoint()
        return (a.x(), a.y(), a.z())
        
    def get_spacing(self):
        cdef Point3D[double] a = self.ptr.getSpacing()
        return (a.x(), a.y(), a.z())
        
    def get_origin(self):
        cdef Point3D[double] a = self.ptr.getOrigin()
        return (a.x(), a.y(), a.z())
        
    def get_orientation(self):
        cdef vector[double] a = self.ptr.getOrientation()
        return (a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8])
        
    def get_region(self):
        cdef Region3D[int] region = self.ptr.getRegion()
        return (
            (region.getMinPoint().x(), region.getMinPoint().y(), region.getMinPoint().z()), 
            (region.getMaxPoint().x(), region.getMaxPoint().y(), region.getMaxPoint().z())
        )
        
    def get_physical_region(self):
        cdef Region3D[double] region = self.ptr.getPhysicalRegion()
        return (
            (region.getMinPoint().x(), region.getMinPoint().y(), region.getMinPoint().z()), 
            (region.getMaxPoint().x(), region.getMaxPoint().y(), region.getMaxPoint().z())
        )
        
    def get_min_max(self):
        cdef boost_tuple[double,double] a
        with nogil:
            a = self.ptr.getMinMax()
        return (get0[double](a), get1[double](a))
        
    def contain(self, obj):
        cdef Point3D[int] p1
        cdef Point3D[int] p2
        if len(obj)==2:
            p1.set(obj[0][0], obj[0][1], obj[0][2])
            p2.set(obj[1][0], obj[1][1], obj[1][2])
            return self.ptr.contain(p1,p2)
        else:
            p1.set(obj[0], obj[1], obj[2])
            return self.ptr.contain(p1)
                    
    def find(self, minv, maxv):
        cdef double min_val
        cdef double max_val
        if minv is None:
            min_val = -get_infinity()
        else:
            min_val = minv
        if maxv is None:
            max_val = get_infinity()
        else:
            max_val = maxv
        cdef vector[PointValue] cres
        with nogil:
            cres = self.ptr.find(min_val, max_val)
        cdef vector[PointValue].iterator end = cres.end()
        cdef vector[PointValue].iterator iter = cres.begin()
        result = []
        while iter!=end:
            result.append((deref(iter).point.x(), deref(iter).point.y(), deref(iter).point.z(), deref(iter).value))
            preincrement(iter)
        return result
        
    def find_region(self, minv, maxv):
        cdef double min_val
        cdef double max_val
        if minv is None:
            min_val = -get_infinity()
        else:
            min_val = minv
        if maxv is None:
            max_val = get_infinity()
        else:
            max_val = maxv
        with nogil:
            res = self.ptr.findRegion(min_val, max_val)
        return (
            (res.getMinPoint().x(), res.getMinPoint().y(), res.getMinPoint().z()), 
            (res.getMaxPoint().x(), res.getMaxPoint().y(), res.getMaxPoint().z())
        )
        
    def find_physical_region(self, minv, maxv):
        cdef double min_val
        cdef double max_val
        if minv is None:
            min_val = -get_infinity()
        else:
            min_val = minv
        if maxv is None:
            max_val = get_infinity()
        else:
            max_val = maxv
        with nogil:
            res = self.ptr.findPhysicalRegion(min_val, max_val)
        return (
            (res.getMinPoint().x(), res.getMinPoint().y(), res.getMinPoint().z()), 
            (res.getMaxPoint().x(), res.getMaxPoint().y(), res.getMaxPoint().z())
        )
            
    def fill(self, val, region=None):
        cdef Point3D[int] minp
        cdef Point3D[int] maxp
        cdef double cval
        if region is None:
            minp = self.ptr.getMinPoint()
            maxp = self.ptr.getMaxPoint()
        else:
            minp.set(region[0][0], region[0][1], region[0][2])
            maxp.set(region[1][0], region[1][1], region[1][2])
        if type(val) is _Image:
            with nogil:
                self.ptr.fill((<_Image>val).ptr[0], minp, maxp)
        else:
            cval = float(val)
            with nogil:
                self.ptr.fill(cval, minp, maxp)
    
    def fill_with_roi(self, file, double val=1):
        cdef Region3D[int] region
        cdef string cfile = file.encode("utf-8")
        with nogil:
            region = self.ptr.fillRoi(cfile, val)
        return (
            (region.getMinPoint().x(), region.getMinPoint().y(), region.getMinPoint().z()),
            (region.getMaxPoint().x(), region.getMaxPoint().y(), region.getMaxPoint().z())
        )
        
    def write(self, file, bint compress=True):
        cdef string cfile = file.encode("utf-8")
        with nogil:
            self.ptr.write(cfile, compress)
        
    def abs(self):
        with nogil:
            self.ptr.abs()
    def get_abs(self):
        ret = _Image()
        with nogil:
            ret.ptr = self.ptr.getAbs()
        return ret

    def exp(self):
        with nogil:
            self.ptr.exp()
    def get_exp(self):
        ret = _Image()
        with nogil:
            ret.ptr = self.ptr.getExp()
        return ret

    def log(self):
        with nogil:
            self.ptr.log()
    def get_log(self):
        ret = _Image()
        with nogil:
            ret.ptr = self.ptr.getLog()
        return ret
        
    def negate(self):
        with nogil:
            self.ptr.negate()
    def get_negate(self):
        ret = _Image()
        with nogil:
            ret.ptr = self.ptr.getNegate()
        return ret
        
    def add(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceAdd((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceAdd(cval)
    def subtract(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceSubtract((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceSubtract(cval)
    def multiply(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceMultiply((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceMultiply(cval)
    def divide(self, val, double out_val=1):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceDivide((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceDivide(cval)
    def eq(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceEq((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceEq(cval)
    def ne(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceNe((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceNe(cval)
    def gt(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceGt((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceGt(cval)
    def ge(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceGe((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceGe(cval)
    def lt(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceLt((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceLt(cval)
    def le(self, val, double out_val=0):
        cdef double cval
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceLe((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceLe(cval)
    def bit_and(self, val, double out_val=0):
        cdef double cval
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        if type=="float" or type=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceBitAnd((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceBitAnd(cval)
    def bit_or(self, val, double out_val=0):
        cdef double cval
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        if type=="float" or type=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceBitOr((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceBitOr(cval)
    def bit_xor(self, val, double out_val=0):
        cdef double cval
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        if type=="float" or type=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                self.ptr.inplaceBitXor((<_Image>val).ptr[0], out_val)
        else:
            cval = float(val)
            with nogil:
                self.ptr.inplaceBitXor(cval)

    def get_add(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retAdd((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retAdd(cval)
        return ret
    def get_subtract(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retSubtract((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retSubtract(cval)
        return ret
    def get_multiply(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retMultiply((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retMultiply(cval)
        return ret
    def get_divide(self, val, double out_val=1, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retDivide((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retDivide(cval)
        return ret
    def get_eq(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retEq((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retEq(cval)
        return ret
    def get_ne(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retNe((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retNe(cval)
        return ret
    def get_gt(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retGt((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retGt(cval)
        return ret
    def get_ge(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retGe((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retGe(cval)
        return ret
    def get_lt(self, val, double out_val=0, double src_out_val=0):
        cdef double cval
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retLt((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retLt(cval)
        return ret
    def get_le(self, val, double out_val=0, double src_out_val=0):
        cdef double cval    
        ret = _Image()
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retLe((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retLe(cval)
        return ret
    def get_bit_and(self, val, double out_val=0, double src_out_val=0):
        ret = _Image()
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        cdef double cval
        if type_str=="float" or type_str=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retBitAnd((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retBitAnd(cval)
        return ret
    def get_bit_or(self, val, double out_val=0, double src_out_val=0):
        ret = _Image()
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        cdef double cval
        if type_str=="float" or type_str=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retBitOr((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retBitOr(cval)
        return ret
    def get_bit_xor(self, val, double out_val=0, double src_out_val=0):
        ret = _Image()
        type_str = self.ptr.getTypeString().c_str().decode("utf-8")
        cdef double cval
        if type_str=="float" or type_str=="double":
            raise ValueError("Floating point type not allowed for bitwise logical operations!")
        if type(val) is _Image:
            with nogil:
                ret.ptr = self.ptr.retBitXor((<_Image>val).ptr[0], out_val, src_out_val)
        else:
            cval = float(val)
            with nogil:
                ret.ptr = self.ptr.retBitXor(cval)
        return ret

    def __exp__(self):
        return self.get_exp()
    def __abs__(self):
        return self.get_abs()
    def __neg__(self):
        return self.get_negate()
    def __invert__(self):
        return self.get_le(0)
        
    def __iadd__(self, val):
        self.add(val)
        return self
    def __isub__(self, val):
        self.subtract(val)
        return self
    def __imul__(self, val):
        self.multiply(val)
        return self
    def __itruediv__(self, val):
        self.divide(val)
        return self
    def __iand__(self, val):
        self.bit_and(val)
        return self
    def __ior__(self, val):
        self.bit_or(val)
        return self
    def __ixor__(self, val):
        self.bit_xor(val)
        return self
        
    def __add__(x, y):
        if isinstance(x, _Image):
            return x.get_add(y)
        else:
            return y.get_add(x)
    def __sub__(x, y):
        if isinstance(x, _Image):
            return x.get_subtract(y)
        else:
            res = y.get_negate()
            res.add(x)
            return res
    def __mul__(x, y):
        if isinstance(x, _Image):
            return x.get_multiply(y)
        else:
            return y.get_multiply(x)
    def __truediv__(x, y):
        if isinstance(x, _Image):
            return x.get_divide(y)
        else:
            res = cast(y, fillval=x)
            res.divide(y)
            return res
    
    def __richcmp__(x, y, int op):
        if isinstance(x, _Image):
            if op==0:
                return x.get_lt(y)
            elif op==2:
                return x.get_eq(y)
            elif op==4:
                return x.get_gt(y)
            elif op==1:
                return x.get_le(y)
            elif op==3:
                return x.get_ne(y)
            else:
                return x.get_ge(y)
        else:
            res = cast(y, fillval=x)
            if op==0:
                res.lt(y)
            elif op==2:
                res.eq(y)
            elif op==4:
                res.gt(y)
            elif op==1:
                res.le(y)
            elif op==3:
                res.ne(y)
            else:
                res.ge(y)
            return res
        
    def __and__(x, y):
        if isinstance(x, _Image):
            return x.get_bit_and(y)
        else:
            return y.get_bit_and(x)
    def __or__(x, y):
        if isinstance(x, _Image):
            return x.get_bit_or(y)
        else:
            return y.get_bit_or(x)
    def __xor__(x, y):
        if isinstance(x, _Image):
            return x.get_bit_xor(y)
        else:
            return y.get_bit_xor(x)
            
    def get_binary_dilation(self, _Element obj):
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getBinaryDilation(obj.ptr)
        return ret_val
        
    def get_binary_erosion(self, _Element obj):
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getBinaryErosion(obj.ptr)
        return ret_val
        
    def get_binary_opening(self, _Element obj):
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getBinaryOpening(obj.ptr)
        return ret_val
        
    def get_binary_closing(self, _Element obj):
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getBinaryClosing(obj.ptr)
        return ret_val
        
    def get_distance_transform(self, bint is_signed=False, bint use_square=False, bint use_spacing=True, region=None):
        cdef Region3D[int] cregion
        if region is None:
            cregion.reset()
        else:
            cregion.set(
                Point3D[int](region[0][0], region[0][1], region[0][2]),
                Point3D[int](region[1][0], region[1][1], region[1][2])
            )
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getDistanceTransform(is_signed, use_square, use_spacing, cregion)
        return ret_val
        
    def get_resampled(self, _Image obj, bint nn=False, outval=None):
        cdef bint use_out_value
        cdef int coutval
        if outval is None:
            use_out_value = False
            coutval = 1
        else:
            use_out_value = True
            coutval = outval
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getResampled(obj.ptr[0], nn, use_out_value, coutval)
        return ret_val
            
    def get_crop(self, minp, maxp, double outval=0):
        cdef Point3D[int] cminp
        cminp.set(minp[0], minp[1], minp[2])
        cdef Point3D[int] cmaxp
        cmaxp.set(maxp[0], maxp[1], maxp[2])
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getCrop(cminp, cmaxp, outval)
        return ret_val
        
    def get_flip(self, int flip_axis):
        ret_val = _Image()
        with nogil:
            ret_val.ptr = self.ptr.getFlip(flip_axis)
        return ret_val
        
    def get_histogram(self, region=None):
        cdef Region3D[int] cregion
        if region is None:
            cregion.reset()
        else:
            cregion.set(
                Point3D[int](region[0][0], region[0][1], region[0][2]),
                Point3D[int](region[1][0], region[1][1], region[1][2])
            )
        cdef unordered_map[double,long long] cres
        with nogil:
             cres = self.ptr.getHistogram(cregion)
        cdef unordered_map[double,long long].iterator end = cres.end()
        cdef unordered_map[double,long long].iterator iter = cres.begin()
        res = {}
        while iter!=end:
            res[deref(iter).first] = deref(iter).second
            preincrement(iter)
        return res
        
    def get_joint_histogram(self, _Image obj, region=None, double outval=0):
        cdef Region3D[int] cregion
        if region is None:
            cregion.reset()
        else:
            cregion.set(
                Point3D[int](region[0][0], region[0][1], region[0][2]),
                Point3D[int](region[1][0], region[1][1], region[1][2])
            )
        cdef unordered_map_full[boost_tuple[double,double],long long,ihash[double],iequal_to[double]] cres
        with nogil:
            cres = self.ptr.getJointHistogram(obj.ptr[0], cregion, outval)
        cdef unordered_map_full[boost_tuple[double,double],long long,ihash[double],iequal_to[double]].iterator end = cres.end()
        cdef unordered_map_full[boost_tuple[double,double],long long,ihash[double],iequal_to[double]].iterator iter = cres.begin()
        res = {}
        while iter!=end:
            key = (get0[double](deref(iter).first), get1[double](deref(iter).first))
            res[key] = deref(iter).second
            preincrement(iter)
        return res

    def get_statistics_calculator(self, _Image obj):
        
        warnings.warn('Method get_statistics_calculator is deprecated! per QIA-633')

        ret_obj = _StatCalc()
        with nogil:
            ret_obj.ptr = self.ptr.getStatisticsCalculator(obj.ptr[0])
        return ret_obj
        
    def get_percentile_calculator(self, _Image obj):
        ret_obj = _PercCalc()
        with nogil:
            ret_obj.ptr = self.ptr.getPercentileCalculator(obj.ptr[0])
        return ret_obj
     
    # original Java implementation for longest axis diameter computation	 
    def compute_original_longest_axial_diameter(self):
        with nogil:
            res = self.ptr.computeOriginalLongestAxialDiameter()
        return {
            "diameter":res[0],
            "perpendicular_diameter":res[1],
            "z":res[2],
            "angle_difference":res[3],
            "start":(res[4],res[5]),
            "end":(res[6],res[7]),
            "perpendicular_start":(res[8],res[9]),
            "perpendicular_end":(res[10],res[11]),
        }
        
    def compute_longest_axial_diameter(self):
        with nogil:
            res = self.ptr.computeLongestAxialDiameter()
        return {
            "diameter":res[0],
            "perpendicular_diameter":res[1],
            "z":res[2],
            "angle_difference":res[3],
            "start":(res[4],res[5]),
            "end":(res[6],res[7]),
            "perpendicular_start":(res[8],res[9]),
            "perpendicular_end":(res[10],res[11]),
        }

    def compute_longest_diameter(self):
        with nogil:
            res = self.ptr.computeLongestDiameter()
        return {
            "diameter":res[0],
            "start":(res[1],res[2],res[3]),
            "end":(res[4],res[5],res[6]),
        }
    # DISABLED, cmake compilation failed 
    #def get_glcm_features(self, _Image obj, offsets, int level, bint symmetric=True):
    #    cdef Point3D[int] coffset
    #    cdef vector[Point3D[int]] pointvec
    #    if not isinstance(offsets[0], tuple) and not isinstance(offsets[0], list):
    #        coffset.set(offsets[0], offsets[1], offsets[2])
    #        pointvec.push_back(coffset)
    #    else:
    #        for o in offsets:
    #            coffset.set(o[0], o[1], o[2])
    #            pointvec.push_back(coffset)
    #    ret_obj = _GlcmFeatures()
    #    with nogil:
    #        ret_obj.ptr = self.ptr.getGlcmFeatures(obj.ptr[0], pointvec, level, symmetric)
    #    return ret_obj
        
    def get_alias(self, min_point=None, spacing=None, orientation=None, origin=None, bint fixphysical=True):
        cdef Point3D[int] cminp
        cdef Point3D[double] cspacing
        cdef Point3D[double] corigin
        cdef vector[double] corientation
        ret = _Image()
        if spacing is None and orientation is None and origin is None and min_point is not None:
            cminp.set(min_point[0],min_point[1],min_point[2])
            ret.ptr = self.ptr.getAlias(cminp, fixphysical)
        else:
            if min_point is None:
                cminp = self.ptr.getMinPoint()
            else:
                cminp.set(min_point[0], min_point[1], min_point[2])
            if spacing is None:
                cspacing = self.ptr.getSpacing()
            else:
                cspacing.set(spacing[0], spacing[1], spacing[2])
            if origin is None:
                corigin = self.ptr.getOrigin()
            else:
                corigin.set(origin[0], origin[1], origin[2])
            if orientation is None:
                corientation = self.ptr.getOrientation()
            else:
                for i in orientation:
                    corientation.push_back(i)
            ret.ptr = self.ptr.getAlias(cminp, cspacing, corigin, corientation)
        return ret
        
    def get_sub_image(self, min_point, max_point):
        cdef Point3D[int] cminp
        cdef Point3D[int] cmaxp
        cminp.set(min_point[0],min_point[1],min_point[2])
        cmaxp.set(max_point[0],max_point[1],max_point[2])
        ret = _Image()
        ret.ptr = self.ptr.getSubImage(cminp, cmaxp)
        return ret
        
    def get_whole_image(self):
        ret = _Image()
        ret.ptr = self.ptr.getWholeImage()
        return ret
        
    def is_sub_image(self):
        return self.ptr.isSubImage()
    
    def get_array(self, region=None):
        cdef np.NPY_TYPES np_type
        type = self.get_type()
        if type==Type.char:
            np_type = np.NPY_INT8
        elif type==Type.uchar:
            np_type = np.NPY_UINT8
        elif type==Type.short:
            np_type = np.NPY_INT16
        elif type==Type.ushort:
            np_type = np.NPY_UINT16
        elif type==Type.int:
            np_type = np.NPY_INT32
        elif type==Type.uint:
            np_type = np.NPY_UINT32
        elif type==Type.long:
            np_type = np.NPY_INT64
        elif type==Type.ulong:
            np_type = np.NPY_UINT64
        elif type==Type.float:
            np_type = np.NPY_FLOAT32
        elif type==Type.double:
            np_type = np.NPY_FLOAT64
        else:
            raise ValueError("Invalid type %s encountered" % type)
        # get image pointing to the original image
        whole_image = (<_Image>self.get_whole_image())
        cdef void* buffer = whole_image.ptr.getBuffer()
        # get size of current image
        if region is None:
            region = self.get_region()
        request_size = [j-i+1 for i,j in zip(*region)]
        for i,j in zip(request_size, whole_image.get_size()):
            if i>j:
                raise ValueError("Requested region is larger than image")
        # get offset relative to the whole image
        for i,j in zip(whole_image.get_min_point(), region[0]):
            if j<i:
                raise ValueError("Min point of requested region is less than image")
        request_offset = [j-i for i,j in zip(whole_image.get_min_point(), region[0])]
                
        cdef vector[np.npy_intp] dims
        dims.push_back(whole_image.get_size()[2])
        dims.push_back(whole_image.get_size()[1])
        dims.push_back(whole_image.get_size()[0])
        arr = np.PyArray_SimpleNewFromData(3, &dims[0], np_type, buffer)

        return arr[
            request_offset[2]:request_offset[2]+request_size[2],
            request_offset[1]:request_offset[1]+request_size[1],
            request_offset[0]:request_offset[0]+request_size[0],
        ]
        
def new(type, minp, maxp, spacing=(1,1,1), origin=(0,0,0), orientation=(1,0,0,0,1,0,0,0,1), double fillval=0):
    cdef Point3D[int] cminp
    cdef Point3D[int] cmaxp
    cdef Point3D[double] cspacing
    cdef Point3D[double] corigin
    cdef vector[double] corientation
    cminp.set(minp[0],minp[1],minp[2])
    cmaxp.set(maxp[0],maxp[1],maxp[2])
    cspacing.set(spacing[0],spacing[1],spacing[2])
    corigin.set(origin[0],origin[1],origin[2])
    cdef double i
    for i in orientation:
        corientation.push_back(i)
    ret_obj = _Image()
    ret_obj._data = None
    cdef string ctype = type.value.encode("utf-8")
    ret_obj.ptr = new ImageObject(ctype, cminp, cmaxp, cspacing, corigin, corientation, fillval)
    return ret_obj
    
def cast(_Image obj, type=Type.auto, bint copy=False, double fillval=0):
    ret_obj = _Image()
    ret_obj._data = None
    ret_obj.ptr = new ImageObject(type.value.encode("utf-8"), obj.ptr[0], copy, fillval)
    return ret_obj
    
def from_array(array, template=None):
    cdef Point3D[int] cminp
    cdef Point3D[int] cmaxp
    cdef Point3D[double] cspacing
    cdef Point3D[double] corigin
    cdef vector[double] corientation

    if len(array.shape)!=3:
            raise ValueError("Only 3D array is supported")
    if template:
        for i,j in zip(reversed(array.shape), template.get_size()):
            if i!=j:
                raise ValueError("Array size %s is different from template size %s" % (str(tuple(reversed(array.shape))), str(template.get_size())))
        temp = template.get_min_point()
        cminp.set(temp[0], temp[1], temp[2])
        temp = template.get_max_point()
        cmaxp.set(temp[0], temp[1], temp[2])
        temp = template.get_spacing()
        cspacing.set(temp[0], temp[1], temp[2])
        temp = template.get_origin()
        corigin.set(temp[0], temp[1], temp[2])
        for i in template.get_orientation():
            corientation.push_back(i)
    else:
        cminp.set(0,0,0)
        cmaxp.set(array.shape[2]-1, array.shape[1]-1, array.shape[0]-1)
        cspacing.set(1,1,1)
        corigin.set(0,0,0)
        for i in (1,0,0,0,1,0,0,0,1):
            corientation.push_back(i)
    type = None
    if array.dtype not in NP_DTYPE_LOOKUP:
        raise ValueError("Conversion from %s is not supported" % array.dtype)
    type = NP_DTYPE_LOOKUP[array.dtype]
    ret_obj = _Image()
    ret_obj._data = np.ascontiguousarray(array)
    cdef string ctype = type.value.encode("utf-8")
    ret_obj.ptr = new ImageObject(np.PyArray_GETPTR3(ret_obj._data, 0,0,0),
        ctype,
        cminp, cmaxp, cspacing, corigin, corientation
    )
    return ret_obj

def read(file, type=Type.auto):
    cdef string cfile = str(file).encode("utf-8")
    cdef string ctype = type.value.encode("utf-8")
    ret_obj = _Image()
    with nogil:
        ret_obj.ptr = new ImageObject(ctype, cfile)
    return ret_obj