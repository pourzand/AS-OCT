# distutils: language = c++

import numbers
from enum import Enum

class Ramp(Enum):
    linear = 0
    scurve = 1
    sqrt = 2
    
class Scale(Enum):
    linear = 0
    log10 = 1

cdef class LookupTable:
    def __cinit__(self, size=None):
        self.ptr = new CLookupTable()
        if size is not None:
            self.resize(size)
        
    def __dealloc__(self):
        del self.ptr
        
    def resize(self, int v=256):
        self.ptr.resize(v)
        
    def set_indexed_lookup(self, bint en):
        self.ptr.setIndexedLookup(en)

    def set_table(self, r):
        self.ptr.setTableRange(r[0], r[1])

    def set_value(self, r):
        self.ptr.setValueRange(r[0], r[1])
    
    def set_hue(self, r):
        self.ptr.setHueRange(r[0], r[1])
    def set_saturation(self, r):
        self.ptr.setSaturationRange(r[0], r[1])
    def set_alpha(self, r):
        self.ptr.setAlphaRange(r[0], r[1])

    def set_below(self, bint en, rgba=None):
        if rgba is None:
            self.ptr.setBelowRangeColor(en)
        else:
            self.ptr.setBelowRangeColor(en,rgba[0],rgba[1],rgba[2],rgba[3])
    def set_above(self, bint en, rgba=None):
        if rgba is None:
            self.ptr.setAboveRangeColor(en)
        else:
            self.ptr.setAboveRangeColor(en,rgba[0],rgba[1],rgba[2],rgba[3])
        
    def set_nan(self, rgba):
        self.ptr.setNanColor(rgba[0],rgba[1],rgba[2],rgba[3])
        
    def set_scale(self, i):
        self.ptr.setScale(i.value)
    
    def set_ramp(self, i):
        self.ptr.setRamp(i.value)

    def __setitem__(self, unsigned int x, y):
        if len(y)>=4:
            self.ptr.set(x, y[0], y[1], y[2], y[3])
        else:
            self.ptr.set(x, y[0], y[1], y[2], 1)
    
    def __getitem__(self, unsigned int x):
        cdef double res[4]
        self.ptr.get(x, res)
        return (res[0], res[1], res[2], res[3])
    
    def get_mapped(self, double x):
        cdef const unsigned char *res
        res = self.ptr.mapValue(x)
        return (res[0], res[1], res[2])
    
    def get_alpha(self, double x):
        return self.ptr.getAlpha(x)
    
    def get_color(self, double x): 
        cdef double res[3]
        self.ptr.getColor(x, res)
        return (res[0], res[1], res[2])
    
    def get_rgba(self, double x):
        cdef double res[4]
        self.ptr.getRGBA(x, res)
        return (res[0], res[1], res[2], res[3])
    
    def build(self, param=None):
        if param is None:
            self.ptr.build()
            return
        lookup = dict(
            ramp = self.set_ramp,
            indexed_lookup = self.set_indexed_lookup,
            table = self.set_table,
            value = self.set_value,
            hue = self.set_hue,
            saturation = self.set_saturation,
            alpha = self.set_alpha,
            below = self.set_below,
            above = self.set_above,
            nan = self.set_nan,
            scale = self.set_scale,
            ramp = self.set_ramp
        )
        max_val = -1
        for i in param.keys():
            if isinstance(i, numbers.Number):
                max_val = max(max_val, i)
        if max_val>=0:
            self.resize(max_val+1)
        for k,v in param.items():
            if isinstance(k, numbers.Number):
                self[k] = v
            elif k in lookup:
                lookup[k](v)
        if max_val<0:
            self.ptr.build()

    def __len__(self):
        return self.ptr.size()
        
def new(param=None):
    ret = LookupTable()
    if param is not None:
        ret.build(param)
    return ret