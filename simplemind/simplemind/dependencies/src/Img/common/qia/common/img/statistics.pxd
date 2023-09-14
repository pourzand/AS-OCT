# distutils: language = c++

cdef extern from "pcl/statistics/StatisticsCalculator.h" namespace "pcl::statistics":
    cdef cppclass StatisticsCalculator:
        StatisticsCalculator()
        void addValue(double)
        double getMean() const
        double getStandardDeviation(bint) const
        double getVariance(bint) const
        double getSkewness(bint) const
        double getKurtosis(bint) const
        double getMin() const
        double getMax() const
        double getNum() const 

cdef extern from "pcl/statistics/PercentileCalculator.h" namespace "std":
    cdef cppclass std_pair "std::pair"[T,U]:
        T first
        U second
    cdef cppclass std_map "std::map"[T, U]:
        std_map()
        cppclass iterator:
            std_pair[T, U]& operator*()
            iterator operator++()
            iterator operator--()
            bint operator==(iterator)
            bint operator!=(iterator)
        cppclass const_iterator:
            std_pair[T, U]& operator*()
            const_iterator operator++()
            const_iterator operator--()
            bint operator==(const_iterator)
            bint operator!=(const_iterator)
        iterator begin()
        const_iterator const_begin "begin"() const
        iterator end()
        const_iterator const_end "end"() const
        
cdef extern from "pcl/statistics/PercentileCalculator.h" namespace "pcl::statistics":
    cdef cppclass PercentileCalculator[T]:
        PercentileCalculator()
        void addValue(const T&, int)
        double getEntropy() const
        double getMedian(bint) const
        double getPercentile(double, bint) const
        long getNum() const 
        double getMin() const 
        double getMax() const 
        const std_map[T,long]& getMap() const
        
cdef class _StatCalc:
    cdef StatisticsCalculator *ptr
    
cdef class _PercCalc:
    cdef PercentileCalculator[double] *ptr