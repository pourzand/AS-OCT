# distutils: language = c++

from libcpp.string cimport string
from libcpp.vector cimport vector
from qia.common.img.common cimport Point3D, Region3D, raisePyError
from qia.common.img.statistics cimport StatisticsCalculator, PercentileCalculator

cdef extern from "boost/shared_ptr.hpp" namespace "boost":
    cppclass shared_ptr[T]:
        T* get() 

cdef extern from "boost/unordered_map.hpp" namespace "boost":
    cdef cppclass pair[T,U]:
        T first
        U second

cdef extern from "boost/tuple/tuple.hpp" namespace "boost":
    cdef cppclass boost_tuple "boost::tuple"[T,T]:
        pass

cdef extern from "boost/unordered_map.hpp" namespace "boost":
    cdef cppclass unordered_map_full "boost::unordered_map"[T, U, H, E]:
        unordered_map_full()
        cppclass iterator:
            pair[T, U]& operator*()
            iterator operator++()
            iterator operator--()
            bint operator==(iterator)
            bint operator!=(iterator)
        iterator begin()
        iterator end()
        
    cdef cppclass unordered_map "boost::unordered_map"[T, U]:
        unordered_map()
        cppclass iterator:
            pair[T, U]& operator*()
            iterator operator++()
            iterator operator--()
            bint operator==(iterator)
            bint operator!=(iterator)
        iterator begin()
        iterator end()

cdef extern from "image.h":
    void init()
    double get_infinity()
    double get_nan()
    T get0[T](const boost_tuple[T,T]&)
    T get1[T](const boost_tuple[T,T]&)
    cdef cppclass ihash[T]:
        pass
    cdef cppclass iequal_to[T]:
        pass
    struct PointValue:
        Point3D[int] point
        double value
        
    cdef cppclass vnl_matrix[T]:
        unsigned rows() const
        unsigned columns() const
        T get(unsigned, unsigned) const

cdef extern from "image.h" namespace "pcl::filter":
    cdef cppclass GlcmFeatures:
        const vnl_matrix[double]& getMatrix() nogil const 
        double getContrast() nogil
        double getDissimilarity() nogil
        double getHomogeneity() nogil
        double getAngularSecondMoment() nogil
        double getEnergy() nogil
        double getMax() nogil
        double getMin() nogil
        double getEntropy() nogil
        double getMean() nogil
        double getVariance() nogil
        double getCorrelation() nogil
        double getSumAverage() nogil
        double getSumVariance() nogil
        double getSumEntropy() nogil
        double getDiffAverage() nogil
        double getDiffVariance() nogil
        double getDiffEntropy() nogil
        double getInfoCorrelationA() nogil
        double getInfoCorrelationB() nogil
        double getMaximalCorrelationCoefficient() nogil
        
cdef extern from "image.h":
    cdef cppclass ImageObject nogil: 
        ImageObject(const string&, const Point3D[int]&, const Point3D[int]&, const Point3D[double]&, const Point3D[double]&, const vector[double]&, double) nogil
        ImageObject(void*, const string&, const Point3D[int]&, const Point3D[int]&, const Point3D[double]&, const Point3D[double]&, const vector[double]&)
        ImageObject(const string&, const ImageObject&, bint, double) nogil
        ImageObject(const string&, const string&) nogil except +raisePyError
        const string& getTypeString() nogil
        double getValue(int, int, int) nogil
        void setValue(int, int, int, double) nogil
        Point3D[double] toPhysicalCoordinates(double, double, double) nogil
        Point3D[double] toImageCoordinates(double, double, double) nogil
        Point3D[int] getSize() nogil
        Point3D[int] getMinPoint() nogil
        Point3D[int] getMaxPoint() nogil
        Point3D[double] getSpacing() nogil
        Point3D[double] getOrigin() nogil
        vector[double] getOrientation() nogil
        bint contain(const Point3D[int]&) nogil
        bint contain(const Point3D[int]&, const Point3D[int]&) nogil
        Region3D[int] getRegion() nogil
        Region3D[double] getPhysicalRegion() nogil
        vector[PointValue] find(double, double) nogil
        Region3D[int] findRegion(double, double) nogil
        Region3D[double] findPhysicalRegion(double, double) nogil
        boost_tuple[double,double] getMinMax() nogil
        void fill(double, Point3D[int]&, Point3D[int]& maxp) nogil
        void fill(const ImageObject&, Point3D[int]&, Point3D[int]& maxp) nogil
        Region3D[int] fillRoi(const string&, double) nogil
        void write(const string&, bint compress) nogil except +raisePyError
        
        void abs() nogil
        void exp() nogil
        void log() nogil  except +raisePyError
        void negate() nogil
        ImageObject* getNegate() nogil
        ImageObject* getAbs() nogil
        ImageObject* getExp() nogil
        ImageObject* getLog() nogil  except +raisePyError
        
        void inplaceAdd(double) nogil
        void inplaceAdd(const ImageObject&, double) nogil
        void inplaceSubtract(double) nogil
        void inplaceSubtract(const ImageObject&, double) nogil
        void inplaceMultiply(double) nogil
        void inplaceMultiply(const ImageObject&, double) nogil
        void inplaceDivide(double) nogil except +raisePyError
        void inplaceDivide(const ImageObject&, double) nogil except +raisePyError
        void inplaceEq(double) nogil
        void inplaceEq(const ImageObject&, double) nogil
        void inplaceNe(double) nogil
        void inplaceNe(const ImageObject&, double) nogil
        void inplaceGt(double) nogil
        void inplaceGt(const ImageObject&, double) nogil
        void inplaceGe(double) nogil
        void inplaceGe(const ImageObject&, double) nogil
        void inplaceLt(double) nogil
        void inplaceLt(const ImageObject&, double) nogil
        void inplaceLe(double) nogil
        void inplaceLe(const ImageObject&, double) nogil
        void inplaceBitAnd(double) nogil
        void inplaceBitAnd(const ImageObject&, double) nogil
        void inplaceBitOr(double) nogil
        void inplaceBitOr(const ImageObject&, double) nogil
        void inplaceBitXor(double) nogil
        void inplaceBitXor(const ImageObject&, double) nogil
        
        ImageObject* retAdd(double) nogil
        ImageObject* retAdd(const ImageObject&, double, double) nogil
        ImageObject* retSubtract(double) nogil
        ImageObject* retSubtract(const ImageObject&, double, double) nogil
        ImageObject* retMultiply(double) nogil
        ImageObject* retMultiply(const ImageObject&, double, double) nogil
        ImageObject* retDivide(double) nogil except +raisePyError
        ImageObject* retDivide(const ImageObject&, double, double) nogil except +raisePyError
        ImageObject* retEq(double) nogil
        ImageObject* retEq(const ImageObject&, double, double) nogil
        ImageObject* retNe(double) nogil
        ImageObject* retNe(const ImageObject&, double, double) nogil
        ImageObject* retGt(double) nogil
        ImageObject* retGt(const ImageObject&, double, double) nogil
        ImageObject* retGe(double) nogil
        ImageObject* retGe(const ImageObject&, double, double) nogil
        ImageObject* retLt(double) nogil
        ImageObject* retLt(const ImageObject&, double, double) nogil
        ImageObject* retLe(double) nogil
        ImageObject* retLe(const ImageObject&, double, double) nogil
        ImageObject* retBitAnd(double) nogil
        ImageObject* retBitAnd(const ImageObject&, double, double) nogil
        ImageObject* retBitOr(double) nogil
        ImageObject* retBitOr(const ImageObject&, double, double) nogil
        ImageObject* retBitXor(double) nogil
        ImageObject* retBitXor(const ImageObject&, double, double) nogil
        
        ImageObject* getCrop(const Point3D[int]&, const Point3D[int]&, double) nogil
        ImageObject* getBinaryDilation(const shared_ptr[vector[Point3D[int]]]&) nogil
        ImageObject* getBinaryErosion(const shared_ptr[vector[Point3D[int]]]&) nogil
        ImageObject* getBinaryOpening(const shared_ptr[vector[Point3D[int]]]&) nogil
        ImageObject* getBinaryClosing(const shared_ptr[vector[Point3D[int]]]&) nogil except +raisePyError
        ImageObject* getDistanceTransform(bint, bint, bint, const Region3D[int]&) nogil
        ImageObject* getResampled(const ImageObject&, bint, bint, double) nogil
        ImageObject* getFlip(int) nogil except +raisePyError
        shared_ptr[vector[Point3D[int]]] getElement() nogil
        unordered_map[double,long long] getHistogram(const Region3D[int]&) nogil
        unordered_map_full[boost_tuple[double,double],long long,ihash[double],iequal_to[double]] getJointHistogram(const ImageObject&, const Region3D[int]&, double) nogil
        StatisticsCalculator* getStatisticsCalculator(const ImageObject&) nogil
        PercentileCalculator[double]* getPercentileCalculator(const ImageObject&) nogil
        vector[double] computeOriginalLongestAxialDiameter() nogil
        vector[double] computeLongestAxialDiameter() nogil
        vector[double] computeLongestDiameter() nogil
        GlcmFeatures* getGlcmFeatures(const ImageObject&, const vector[Point3D[int]]&, int, bint) nogil
        
        void modifyImage(const Point3D[int]&, bint) nogil
        void modifyImage(const Point3D[int]&, const Point3D[double]&, Point3D[double]&, const vector[double]&) nogil
        ImageObject* getAlias(const Point3D[int]&, bint) nogil
        ImageObject* getAlias(const Point3D[int]&, const Point3D[double]&, Point3D[double]&, const vector[double]&) nogil
        ImageObject* getSubImage(const Point3D[int]&, const Point3D[int]&) nogil except +raisePyError
        ImageObject* getWholeImage() nogil
        bint isSubImage() nogil
        unsigned long long getBufferSize() nogil  
        unsigned int getVoxelSize() nogil
        void* getBuffer() nogil
        
cdef class _GlcmFeatures:
    cdef GlcmFeatures *ptr

cdef class _Image:
    cdef ImageObject *ptr
    cdef public object _data
