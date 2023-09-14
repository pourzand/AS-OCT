# distutils: language = c++

cdef extern from "pcl/misc/ColorImageGenerator.h":
    cdef cppclass CLookupTable "pcl::misc::LookupTable":
        CLookupTable()
        void resize(int);
        void setIndexedLookup(bint)
        void setTableRange(double, double)
        double* getTableRange()
        void setValueRange(double, double)
        double* getValueRange()
        void setHueRange(double, double)
        double* getHueRange()
        void setSaturationRange(double, double)
        double* getSaturationRange()
        void setAlphaRange(double, double)
        double* getAlpha()
        void setBelowRangeColor(bint, double, double, double, double)
        void setBelowRangeColor(bint)
        double* getBelowRangeColor()
        void setAboveRangeColor(bint, double, double, double, double)
        void setAboveRangeColor(bint)
        double* getAboveRangeColor()
        void setNanColor(double, double, double, double)
        double* getNanColor()
        void setScale(int)
        int getScale()
        void setRamp(int)
        int getRamp()
        void set(size_t, double, double, double, double)
        double* get(size_t, double[4])
        const unsigned char* mapValue(double)
        double getAlpha(double)
        double* getColor(double, double[3]) 
        double* getRGBA(double, double[4])
        void build()
        size_t size() const

cdef class LookupTable:
    cdef CLookupTable *ptr