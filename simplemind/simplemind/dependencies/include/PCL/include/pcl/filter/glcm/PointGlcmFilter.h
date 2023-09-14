#ifndef PCL_POINT_GLCM_FILTER
#define PCL_POINT_GLCM_FILTER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/filter/glcm/GlcmFeatures.h>
#include <pcl/filter/glcm/GlcmHelper.h>
#include <pcl/filter/glcm/GrayLevelCooccurrenceMatrix.h>
#include <pcl/geometry/Region3D.h>

#include <iostream>
#include <vector>


namespace pcl
{
    namespace filter
    {
        using namespace pcl::iterator;
        using std::cout;
        using std::endl;

        /**
         * Encapsulates a point filter for computing GLCM texture features.
         *
         * Example usage:
         *
         *   // Alias common types
         *   typedef pcl::Point3D<int>                                  PointType;
         *   typedef pcl::Region3D<int>                                 RegionType;
         *   typedef pcl::Image<int, true>                              ImageType;
         *   typedef pcl::filter::PointGlcmFilter<ImageType>::GlcmIndex GlcmIndexType;
         *
         *   // Load image
         *   auto image = pcl::ImageIoHelper::Read<ImageType>(INPUT_SERIES);
         *
         *   // Define region within which to look for GLCM
         *   auto region = RegionType(PointType(-2,-2,-2), PointType(2,2,2));
         *
         *   // Define list of offsets
         *   std::vector<PointType> offsetList;
         *   offsetList.push_back(PointType(1, 0, 0));
         *   offsetList.push_back(PointType(0, 1, 0));
         *   offsetList.push_back(PointType(0, 0, 1));
         *
         *   // Create PointGlcmFilter object
         *   auto glcmFilter = pcl::filter::PointGlcmFilter<ImageType>::New(image, region, offsetList, NUM_GRAYLEVELS);
         *
         *   // Apply PointGlcmFilter
         *   glcmFilter->apply(TARGET_POINT);
         *
         *   // Retrieve GLCM features
         *   std::cout << glcmFilter->getResult(GlcmIndexType::encodeOffset(0, GlcmIndexType::HOMOGENEITY)) << std::endl;
         *   std::cout << glcmFilter->getResult(GlcmIndexType::encodeOffset(1, GlcmIndexType::HOMOGENEITY)) << std::endl;
         *   std::cout << glcmFilter->getResult(GlcmIndexType::encodeOffset(2, GlcmIndexType::HOMOGENEITY)) << std::endl;
         *   std::cout << glcmFilter->getResult(GlcmIndexType::encodeAggregate(GlcmIndexType::AGGREGATE_MEAN, GlcmIndexType::ENTROPY)) << std::endl;
         *   std::cout << glcmFilter->getResult(GlcmIndexType::encodeAggregate(GlcmIndexType::AGGREGATE_RANGE, GlcmIndexType::ENTROPY)) << std::endl;
         */
        template <class T_InputImageType, template<class> class T_BoundaryHandlerClass = RepeatingBoundaryHandler>
        class PointGlcmFilter : public PointFilterBase
        {
        public:

            //// S T R U C T ////////////////////////////////////////////////////////////////
            /**
             * Helper class for representing a specific feature after applying PointGlcmFilter.
             * A GlcmIndex is defined by two pieces of information:
             *   - the step offset (direction and distance; or range/mean)
             *   - the base feature (e.g. GLCM contrast, GLCM sum entropy, etc)
             * The step and base feature are mapped to a numeric index which summarizes this information.
             *
             * Three different factory methods are provided to create a GlcmIndex object.
             *   - encodeOffset     --  Create GlcmIndex based on offset, feature
             *   - encodeAggregate  --  Create GlcmIndex based on aggregation (range/mean), feature
             *   - decodeIndex      --  Create GlcmIndex based on numeric index
             */
            class GlcmIndex
            {
            public:
                enum AggregateIndexEnum
                {
                    AGGREGATE_MEAN = 0,
                    AGGREGATE_RANGE,
                    NUM_AGGREGATIONS                // Must be the last enum
                };

                enum FeatureIndexEnum
                {
                    CONTRAST = 0,                   // Relies upon the first feature being equal to 0, then each additional feature being sequential
                    DISSIMILARITY,
                    HOMOGENEITY,
                    ANGULAR_SECOND_MOMENT,
                    ENERGY,
                    MAX,
                    ENTROPY,
                    MEAN,
                    VARIANCE,
                    STANDARD_DEVIATION,
                    CORRELATION,
                    SUM_AVERAGE,
                    SUM_VARIANCE,
                    SUM_ENTROPY,
                    DIFF_AVERAGE,
                    DIFF_VARIANCE,
                    DIFF_ENTROPY,
                    INFO_CORRELATION_A,
                    INFO_CORRELATION_B,
                    MAX_CORRELATION_COEFFICIENT,
                    NUM_GLCM_FEATURES               // Must be the last item enum
                };

                /**
                 * Factory method to create a GlcmIndex object based on the specified offset and base feature.
                 * Parameters:
                 *   - offsetIndex -- an index into the vector of offsets passed when calling PointGlcmFilter::New( ... )
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, defined above
                 */
                static GlcmIndex encodeOffset(int offsetIndex, int featureIndex)
                {
                    return GlcmIndex(offsetIndex + NUM_AGGREGATIONS, featureIndex);
                }

                /**
                 * Factory method to create a GlcmIndex object based on an aggregation and base feature.
                 * Parameters:
                 *   - aggregateIndex -- an element of the enum AggregateIndexEnum, defined above
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, above
                 */
                static GlcmIndex encodeAggregate(int aggregateIndex, int featureIndex)
                {
                    return GlcmIndex(aggregateIndex, featureIndex);
                }

                /**
                 * Factory method to create a GlcmIndex object based on a numeric index (as returned by getIndex()).
                 * Parameters:
                 *   - index -- a numeric index representing a GlcmIndex
                 */
                static GlcmIndex decodeIndex(int index)
                {
                    return GlcmIndex(index);
                }

                /**
                 * Returns the numeric index associated with this GlcmIndex object.
                 */
                int getIndex() const
                {
                    return m_index;
                }

                /**
                 * Returns the aggregate index associated with this GlcmIndex object.
                 * If the GlcmIndex object does not represent an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 * The return value corresponds to AggregateIndexEnum.
                 */
                int getAggregateIndex() const
                {
                    return m_aggregateIndex;
                }

                /**
                 * Returns the offset index associated with this GlcmIndex object. The offset is indexed
                 * in the same order as the vector of offsets passed to PointGlcmFilter::New( ... ).
                 * If the GlcmIndex object represents an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 */
                int getOffsetIndex() const
                {
                    return m_aggregateIndex - NUM_AGGREGATIONS;
                }

                /**
                 * Returns the base feature index associated with this GlcmIndex object.
                 * The return value corresponds to FeatureIndexEnum.
                 */
                int getFeatureIndex() const
                {
                    return m_featureIndex;
                }

                /**
                 * Returns true if this GlcmIndex object represents an aggregate feature (e.g. MEAN or RANGE).
                 */
                bool isAggregate() const
                {
                    return m_aggregateIndex < NUM_AGGREGATIONS;
                }

                std::string describe() const
                {
                    if (isAggregate())
                    {
                        return describeAggregation(getAggregateIndex()) + "_GLCM_" + describeFeature(getFeatureIndex());
                    }
                    else
                    {
                        return "O" + boost::lexical_cast<std::string>(getOffsetIndex()) + "_GLCM_" + describeFeature(getFeatureIndex());
                    }
                }

                static std::string describeAggregation(int aggregateIndex)
                {
                    switch (aggregateIndex)
                    {
                    case AGGREGATE_MEAN:    return "MEAN";
                    case AGGREGATE_RANGE:   return "RANGE";
                    default:                return "<INVALID AGGREGATION>";
                    }
                }

                /**
                 * Returns a string describing the specified feature
                 */
                static std::string describeFeature(int featureIndex)
                {
                    switch (featureIndex)
                    {
                    case CONTRAST:                      return "CONTRAST";
                    case DISSIMILARITY:                 return "DISSIMILARITY";
                    case HOMOGENEITY:                   return "HOMOGENEITY";
                    case ANGULAR_SECOND_MOMENT:         return "ANGULAR_SECOND_MOMENT";
                    case ENERGY:                        return "ENERGY";
                    case MAX:                           return "MAX";
                    case ENTROPY:                       return "ENTROPY";
                    case MEAN:                          return "MEAN";
                    case VARIANCE:                      return "VARIANCE";
                    case STANDARD_DEVIATION:            return "STANDARD_DEVIATION";
                    case CORRELATION:                   return "CORRELATION";
                    case SUM_AVERAGE:                   return "SUM_AVERAGE";
                    case SUM_VARIANCE:                  return "SUM_VARIANCE";
                    case SUM_ENTROPY:                   return "SUM_ENTROPY";
                    case DIFF_AVERAGE:                  return "DIFF_AVERAGE";
                    case DIFF_VARIANCE:                 return "DIFF_VARIANCE";
                    case DIFF_ENTROPY:                  return "DIFF_ENTROPY";
                    case INFO_CORRELATION_A:            return "INFO_CORRELATION_A";
                    case INFO_CORRELATION_B:            return "INFO_CORRELATION_B";
                    case MAX_CORRELATION_COEFFICIENT:   return "MAX_CORRELATION_COEFFICIENT";
                    default:                            return "<INVALID FEATURE>";
                    }
                }

            private:
                int m_index;
                int m_aggregateIndex;
                int m_featureIndex;

                GlcmIndex(int index)
                {
                    m_index = index;
                    m_aggregateIndex = (int)(index / NUM_GLCM_FEATURES);
                    m_featureIndex = index % NUM_GLCM_FEATURES;
                }

                GlcmIndex(int aggregateIndex, int featureIndex)
                {
                    m_index = aggregateIndex * NUM_GLCM_FEATURES + featureIndex;
                    m_aggregateIndex = aggregateIndex;
                    m_featureIndex = featureIndex;
                }
            };

            struct GlcmAggregateStruct
            {
                double mean, range;
                
                GlcmAggregateStruct() {}

                GlcmAggregateStruct(double m, double r)
                {
                    mean = m;
                    range = r;
                }
            };
            /////////////////////////////////////////////////////////////////////////////////

            //// T Y P E D E F S ////////////////////////////////////////////////////////////
            typedef PointGlcmFilter                             Self;
            typedef boost::shared_ptr<Self>                     Pointer;
            typedef T_BoundaryHandlerClass<T_InputImageType>    BoundaryHandlerType;
            /////////////////////////////////////////////////////////////////////////////////

            //// F A C T O R Y //////////////////////////////////////////////////////////////
            /**
             * Factory method for instantiating a PointGlcmFilter object.
             * Parameters:
             *   - image -- Pointer to image on which to apply the filter
             *   - region -- 3D region within which to compute GLCM
             *   - offsets -- Vector of direction/distance offsets
             *   - numGraylevels -- Number of graylevel bins to use for dynamic quantization
             */
            static Pointer New(const typename T_InputImageType::ConstantPointer &image, const Region3D<int> &region, const std::vector<Point3D<int>> &offsets, int numGraylevels)
            {
                Pointer obj(new Self);      // boost::shared_ptr<PointGlcmFilter> obj(new PointGlcmFilter)
                obj->initialize(image, region, offsets, numGraylevels);
                return obj;
            }
            /////////////////////////////////////////////////////////////////////////////////


            //// P U B L I C  I N T E R F A C E /////////////////////////////////////////////
            template <class T_IteratorType>
            void apply(const T_IteratorType &iter)
            {
                apply(iter.getPoint(), iter.getIndex());
            }

            void apply(const Point3D<int> &point)
            {
                apply(point, m_image->localToIndex(point));
            }

            void apply(long index)
            {
                apply(m_image->localToPoint(index), index);
            }

            /**
             * Applies the PointGlcmFilter at the specified point.
             */
            void apply(const Point3D<int> &point, long index)
            {
                if (isPreviousIndex(index)) return;

                // Initialize window iterator
//clock.tic();
                m_imageIterator.setWindowOrigin(point, index);

                GrayLevelCooccurrenceMatrix glcm(m_numGraylevels, true);
                Point3D<int> pointOffset;
                PointIndexObject offset;

                // Reset results vector
                m_results.clear();

                // Loop through vector of offsets
                for (auto iter = m_pointOffsets.begin(); iter != m_pointOffsets.end(); iter++)
                {
                    Point3D<int> pointOffset = *iter;
                    //cout << "Processing offset " << pointOffset << endl;

                    // Use GlcmHelper to create GLCM
                    glcm.clear();
                    offset = GlcmHelper::GetOffset(pointOffset, m_image);

                    glcm = GlcmHelper::ComputeFromIteratorWithDynamicQuantization<typename T_InputImageType::ConstantPointer, ImageWindowIteratorWithPoint>
                        (glcm, 
                         m_image, 
                         m_imageIterator, 
                         offset, 
                         [&](const pcl::Point3D<int>& point, long index)->bool {
                             return m_imageIterator.getRegion().contain(point);
                         },
                         d_useOldBinCounting);
                    //cout << glcm.getMatrix() << endl << endl;
                    //cout << glcm.getNormalizedMatrix() << endl;

                    // Compute GLCM features and add to results vector
                    m_results.push_back(GlcmFeatures(glcm));
                }
//elapsed1 += clock.toc().getClock();

                // Compute aggregate feature values (e.g. MEAN and RANGE)
                computeAggregateResults();
            }

            /**
             * Returns the entire results vector.
             */
            std::vector<GlcmFeatures> getResults()
            {
                return m_results;
            }

            /**
             * Returns the value of the feature specified by the given GlcmIndex object.
             */
            double getResult(GlcmIndex glcmIndex)
            {
                if (glcmIndex.isAggregate())
                {
                    switch (glcmIndex.getAggregateIndex())
                    {
                        case GlcmIndex::AGGREGATE_MEAN:  return getMeanResult(glcmIndex.getFeatureIndex());
                        case GlcmIndex::AGGREGATE_RANGE: return getRangeResult(glcmIndex.getFeatureIndex());
                        default: return std::numeric_limits<double>::quiet_NaN();
                    }
                }
                else
                {
                    return getResult(glcmIndex.getOffsetIndex(), glcmIndex.getFeatureIndex());
                }
            }

            /**
             * Returns the value of the feature specified by the given numerical index.
             * The GlcmIndex class is used to decode the numerical index.
             */
            double getResult(int index)
            {
                return getResult(GlcmIndex::decodeIndex(index));
            }

            /**
             * Returns the value of the specified feature.
             */
            double getResult(int offsetIndex, int featureIndex)
            {
                switch (featureIndex)
                {
                    case GlcmIndex::CONTRAST:                    return m_results[offsetIndex].getContrast();
                    case GlcmIndex::DISSIMILARITY:               return m_results[offsetIndex].getDissimilarity();
                    case GlcmIndex::HOMOGENEITY:                 return m_results[offsetIndex].getHomogeneity();
                    case GlcmIndex::ANGULAR_SECOND_MOMENT:       return m_results[offsetIndex].getAngularSecondMoment();
                    case GlcmIndex::ENERGY:                      return m_results[offsetIndex].getEnergy();
                    case GlcmIndex::MAX:                         return m_results[offsetIndex].getMax();
                    case GlcmIndex::ENTROPY:                     return m_results[offsetIndex].getEntropy();
                    case GlcmIndex::MEAN:                        return m_results[offsetIndex].getMean();
                    case GlcmIndex::VARIANCE:                    return m_results[offsetIndex].getVariance();
                    case GlcmIndex::STANDARD_DEVIATION:          return m_results[offsetIndex].getStandardDeviation();
                    case GlcmIndex::CORRELATION:                 return m_results[offsetIndex].getCorrelation();
                    case GlcmIndex::SUM_AVERAGE:                 return m_results[offsetIndex].getSumAverage();
                    case GlcmIndex::SUM_VARIANCE:                return m_results[offsetIndex].getSumVariance();
                    case GlcmIndex::SUM_ENTROPY:                 return m_results[offsetIndex].getSumEntropy();
                    case GlcmIndex::DIFF_AVERAGE:                return m_results[offsetIndex].getDiffAverage();
                    case GlcmIndex::DIFF_VARIANCE:               return m_results[offsetIndex].getDiffVariance();
                    case GlcmIndex::DIFF_ENTROPY:                return m_results[offsetIndex].getDiffEntropy();
                    case GlcmIndex::INFO_CORRELATION_A:          return m_results[offsetIndex].getInfoCorrelationA();
                    case GlcmIndex::INFO_CORRELATION_B:          return m_results[offsetIndex].getInfoCorrelationB();
                    case GlcmIndex::MAX_CORRELATION_COEFFICIENT: return m_results[offsetIndex].getMaximalCorrelationCoefficient();
                    default:                                     return std::numeric_limits<double>::quiet_NaN();
                }
            }

            /**
             * Returns the aggregate mean for the specified base feature.
             */
            double getMeanResult(int featureIndex)
            {
                return m_resultsAggregate[featureIndex].mean;
            }

            /**
             * Returns the aggregate range for the specified base feature.
             */
            double getRangeResult(int featureIndex)
            {
                return m_resultsAggregate[featureIndex].range;
            }

            /**
             * Debug method to force old bin counting method.
             *
             * New method: stepSize = (maxVal - minVal + 1) / numGraylevels
             * Old method: stepSize = (maxVal - minVal) / numGraylevels
             */
            void debug_useOldBinCounting()
            {
                d_useOldBinCounting = true;
            }
            /////////////////////////////////////////////////////////////////////////////////


        protected:
            //// P R O T E C T E D  M E M B E R S ///////////////////////////////////////////
            // User-specified parameters
            typename T_InputImageType::ConstantPointer      m_image;
            Region3D<int>                                   m_region;
            std::vector<Point3D<int>>                       m_pointOffsets;
            int                                             m_numGraylevels;

            // Derived parameters
            BoundaryHandlerType                             m_boundaryHandler;
            ImageWindowIteratorWithPoint                    m_imageIterator;
            //Region3D<int>                                   m_safeRegion;

            // Results
            std::vector<GlcmFeatures>                       m_results;
            std::vector<GlcmAggregateStruct>                m_resultsAggregate;

            // Debug switches
            bool d_useOldBinCounting;
            /////////////////////////////////////////////////////////////////////////////////

            
            //// P R O T E C T E D  M E T H O D S ///////////////////////////////////////////
            PointGlcmFilter()
            {
                d_useOldBinCounting = false;
            }

            void initialize(const typename T_InputImageType::ConstantPointer &image, const Region3D<int> &region, const std::vector<Point3D<int>> offsets, int numGraylevels)
            {
                m_image = image;                    // Shallow copy
                m_region = region;                  // Deep copy
                m_pointOffsets = offsets;
                m_numGraylevels = numGraylevels;    // Primitive copy

                m_boundaryHandler.setImage(m_image);
                m_imageIterator.setImage(m_image, m_region);

                // Define safe region of image
                //m_safeRegion.set(m_image->getMinPoint() - m_region.getMinPoint(),
                //                 m_image->getMaxPoint() - m_region.getMaxPoint());
            }

            void computeAggregateResults()
            {
                m_resultsAggregate.clear();
                m_resultsAggregate.resize(GlcmIndex::NUM_GLCM_FEATURES);

                for (int f = 0; f < GlcmIndex::NUM_GLCM_FEATURES; f++)
                {
                    double mean, range;
                    double sum;
                    double min = std::numeric_limits<double>::infinity();
                    double max = -std::numeric_limits<double>::infinity();

                    sum = 0.0;
                    for (int o = 0; o < m_results.size(); o++)
                    {
                        double value = getResult(o, f);

                        sum += value;
                        if (value < min) min = value;
                        if (value > max) max = value;
                    }

                    mean = sum / m_results.size();
                    range = max - min;

                    m_resultsAggregate[f] = GlcmAggregateStruct(mean, range);
                }
            }
            /////////////////////////////////////////////////////////////////////////////////
        };
    }
}

#endif
