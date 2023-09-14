# distutils: language = c++

from cython.operator cimport dereference as deref, preincrement 

cdef class _StatCalc:
    def __dealloc__(self):
        del self.ptr
        
    def add(self, double val, int count=1):
        self.ptr.addValue(val)
        if count>1:
            for i in range(count-1):
                self.ptr.addValue(val)

    def mean(self):
        return self.ptr.getMean()
    
    def stddev(self, bint biascorrected=True):
        return self.ptr.getStandardDeviation(biascorrected)

    def variance(self, bint biascorrected=True):
        return self.ptr.getVariance(biascorrected)
        
    def skewness(self, bint biascorrected=True):
        return self.ptr.getSkewness(biascorrected)
        
    def kurtosis(self, bint biascorrected=True):
        return self.ptr.getKurtosis(biascorrected)
        
    def min(self):
        return self.ptr.getMin()
        
    def max(self):
        return self.ptr.getMax()
        
    def num(self):
        return self.ptr.getNum()
    

cdef class _PercCalc:
    def __dealloc__(self):
        del self.ptr
        
    def add(self, double val, int count=1):
        self.ptr.addValue(val, count)
    
    def entropy(self):
        return self.ptr.getEntropy()
    
    def median(self, bint interpolate=True):
        return self.ptr.getMedian(interpolate)
        
    def percentile(self, double perc, bint interpolate=True):
        return self.ptr.getPercentile(perc, interpolate)
        
    def num(self):
        return self.ptr.getNum()
        
    def min(self):
        return self.ptr.getMin()
        
    def max(self):
        return self.ptr.getMax()
        
    def histogram(self):
        cdef std_map[double,long].const_iterator iter = self.ptr.getMap().const_begin()
        cdef std_map[double,long].const_iterator end = self.ptr.getMap().const_end()
        res = {}
        while iter!=end:
            res[deref(iter).first] = deref(iter).second
            preincrement(iter)
        return res
        

def new_statistics_calculator():
    obj = _StatCalc()
    obj.ptr = new StatisticsCalculator()
    return obj
    
def new_percentile_calculator():
    obj = _PercCalc()
    obj.ptr = new PercentileCalculator[double]()
    return obj