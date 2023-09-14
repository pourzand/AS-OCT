#ifndef PCL_POINT_RLM_FILTER
#define PCL_POINT_RLM_FILTER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/filter/rlm/RlmFeatures.h>
#include <pcl/filter/rlm/RlmHelper.h>
#include <pcl/filter/rlm/RunLengthMatrix.h>
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
         * Encapsulates a point filter for computing RLM features.
         *
         * Example usage:
         *   // Alias common types
         *   typedef pcl::Point3D<int>                                  PointType;
         *   typedef pcl::Region3D<int>                                 RegionType;
         *   typedef pcl::Image<int, true>                              ImageType;
         *   typedef pcl::filter::PointRlmFilter<ImageType>::RlmIndex   RlmIndexType;
         *
         *   // Load image
         *   auto image = pcl::ImageIoHelper::Read<ImageType>(INPUT_SERIES);
         *
         *   // Define region within which to look for RLM
         *   auto region = RegionType(PointType(-2,-2,-2), PointType(2,2,2));
         *
         *   // Define list of offsets
         *   std::vector<PointType> offsetList;
         *   offsetList.push_back(PointType(1, 0, 0));
         *   offsetList.push_back(PointType(0, 1, 0));
         *   offsetList.push_back(PointType(0, 0, 1));
         *
         *   // Create PointRlmFilter object
         *   auto rlmFilter = pcl::filter::PointRlmFilter<ImageType>::New(image, region, offsetList, NUM_GRAYLEVELS);
         *
         *   // Apply PointRlmFilter
         *   rlmFilter->apply(TARGET_POINT);
         *
         *   // Retrieve RLM features
         *   std::cout << rlmFilter->getResult(RlmIndexType::encodeOffset(0, RlmIndexType::SHORT_RUN_EMPHASIS)) << std::endl;
         *   std::cout << rlmFilter->getResult(RlmIndexType::encodeOffset(1, RlmIndexType::SHORT_RUN_EMPHASIS)) << std::endl;
         *   std::cout << rlmFilter->getResult(RlmIndexType::encodeOffset(2, RlmIndexType::SHORT_RUN_EMPHASIS)) << std::endl;
         *   std::cout << rlmFilter->getResult(RlmIndexType::encodeAggregate(RlmIndexType::AGGREGATE_MEAN, RlmIndexType::SHORT_RUN_EMPHASIS)) << std::endl;
         *   std::cout << rlmFilter->getResult(RlmIndexType::encodeAggregate(RlmIndexType::AGGREGATE_RANGE, RlmIndexType::SHORT_RUN_EMPHASIS)) << std::endl;
         */
        template <class T_InputImageType, template<class> class T_BoundaryHandlerClass = RepeatingBoundaryHandler>
        class PointRlmFilter : public PointFilterBase
        {
        public:
            //// S T R U C T ////////////////////////////////////////////////////////////////
            /**
             * Helper class for representing a specific feature after applying PointRlmFilter.
             * A RlmIndex is defined by two pieces of information:
             *   - the step offset (direction and distance; or range/mean)
             *   - the base feature (e.g. RLM short run emphasis, RLM run percentage, etc)
             * The step and base feature are mapped to a numeric index which summarizes this information.
             *
             * Three different factory methods are provided to create a RlmIndex object.
             *   - encodeOffset     --  Create RlmIndex based on offset, feature
             *   - encodeAggregate  --  Create RlmIndex based on aggregation (range/mean), feature
             *   - decodeIndex      --  Create RlmIndex based on numeric index
             */
            class RlmIndex
            {
            public:
                enum AggregateIndexEnum
                {
                    AGGREGATE_MEAN = 0,
                    AGGREGATE_RANGE,
                    NUM_AGGREGATIONS
                };

                enum FeatureIndexEnum
                {
                    SHORT_RUN_EMPHASIS = 0,           // Relies upon the first feature being equal to 0, then each additional feature being sequential
                    LONG_RUN_EMPHASIS,
                    LOW_GRAYLEVEL_RUN_EMPHASIS,
                    HIGH_GRAYLEVEL_RUN_EMPHASIS,
                    SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS,
                    SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS,
                    LONG_RUN_LOW_GRAYLEVEL_EMPHASIS,
                    LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS,
                    GRAYLEVEL_NONUNIFORMITY,
                    RUNLENGTH_NONUNIFORMITY,
                    RUN_PERCENTAGE,
                    NUM_RLM_FEATURES
                };

                /**
                 * Factory method to create a RlmIndex object based on the specified offset and base feature.
                 * Parameters:
                 *   - offsetIndex -- an index into the vector of offsets passed when calling PointRlmFilter::New( ... )
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, defined above
                 */
                static RlmIndex encodeOffset(int offsetIndex, int featureIndex)
                {
                    return RlmIndex(offsetIndex + NUM_AGGREGATIONS, featureIndex);
                }

                /**
                 * Factory method to create a RlmIndex object based on an aggregation and base feature.
                 * Parameters:
                 *   - aggregateIndex -- an element of the enum AggregateIndexEnum, defined above
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, above
                 */
                static RlmIndex encodeAggregate(int aggregateIndex, int featureIndex)
                {
                    return RlmIndex(aggregateIndex, featureIndex);
                }

                /**
                 * Factory method to create a RlmIndex object based on a numeric index (as returned by getIndex()).
                 * Parameters:
                 *   - index -- a numeric index representing a RlmIndex
                 */
                static RlmIndex decodeIndex(int index)
                {
                    return RlmIndex(index);
                }

                /**
                 * Returns the numeric index associated with this RlmIndex object.
                 */
                int getIndex() const
                {
                    return m_index;
                }

                /**
                 * Returns the aggregate index associated with this RlmIndex object.
                 * If the RlmIndex object does not represent an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 * The return value corresponds to AggregateIndexEnum.
                 */
                int getAggregateIndex() const
                {
                    return m_aggregateIndex;
                }

                /**
                 * Returns the offset index associated with this RlmIndex object. The offset is indexed
                 * in the same order as the vector of offsets passed to PointRlmFilter::New( ... ).
                 * If the RlmIndex object represents an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 */
                int getOffsetIndex() const
                {
                    return m_aggregateIndex - NUM_AGGREGATIONS;
                }

                /**
                 * Returns the base feature index associated with this RlmIndex object.
                 * The return value corresponds to FeatureIndexEnum.
                 */
                int getFeatureIndex() const
                {
                    return m_featureIndex;
                }

                /**
                 * Returns true if this RlmIndex object represents an aggregate feature (e.g. MEAN or RANGE).
                 */
                bool isAggregate() const
                {
                    return m_aggregateIndex < NUM_AGGREGATIONS;
                }

                /**
                 * Stream output operator (for debugging purposes).
                 */
                friend std::ostream &operator<<(std::ostream &out, const RlmIndex &obj)
                {
                    return out << "(" << obj.m_index << ", " << obj.m_aggregateIndex << ", " << obj.m_featureIndex << ")";
                }

                std::string describe() const
                {
                    if (isAggregate())
                    {
                        return describeAggregation(getAggregateIndex()) + "_RLM_" + describeFeature(getFeatureIndex());
                    }
                    else
                    {
                        return "O" + boost::lexical_cast<std::string>(getOffsetIndex()) + "_RLM_" + describeFeature(getFeatureIndex());
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

                static std::string describeFeature(int featureIndex)
                {
                    switch (featureIndex)
                    {
                    case SHORT_RUN_EMPHASIS:                return "SHORT_RUN_EMPHASIS";
                    case LONG_RUN_EMPHASIS:                 return "LONG_RUN_EMPHASIS";
                    case LOW_GRAYLEVEL_RUN_EMPHASIS:        return "LOW_GRAYLEVEL_RUN_EMPHASIS";
                    case HIGH_GRAYLEVEL_RUN_EMPHASIS:       return "HIGH_GRAYLEVEL_RUN_EMPHASIS";
                    case SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS:  return "SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS";
                    case SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS: return "SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS";
                    case LONG_RUN_LOW_GRAYLEVEL_EMPHASIS:   return "LONG_RUN_LOW_GRAYLEVEL_EMPHASIS";
                    case LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS:  return "LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS";
                    case GRAYLEVEL_NONUNIFORMITY:           return "GRAYLEVEL_NONUNIFORMITY";
                    case RUNLENGTH_NONUNIFORMITY:           return "RUNLENGTH_NONUNIFORMITY";
                    case RUN_PERCENTAGE:                    return "RUN_PERCENTAGE";
                    default:                                return "<INVALID FEATURE>";
                    }
                }

            private:
                int m_index;
                int m_aggregateIndex;
                int m_featureIndex;

                RlmIndex(int index)
                {
                    m_index = index;
                    m_aggregateIndex = (int)(index / NUM_RLM_FEATURES);
                    m_featureIndex = index % NUM_RLM_FEATURES;
                }

                RlmIndex(int aggregateIndex, int featureIndex)
                {
                    m_index = aggregateIndex * NUM_RLM_FEATURES + featureIndex;
                    m_aggregateIndex = aggregateIndex;
                    m_featureIndex = featureIndex;
                }
            };

            struct RlmAggregateStruct
            {
                double mean, range;
                
                RlmAggregateStruct() {}

                RlmAggregateStruct(double m, double r)
                {
                    mean = m;
                    range = r;
                }
            };
            /////////////////////////////////////////////////////////////////////////////////

            //// T Y P E D E F S ////////////////////////////////////////////////////////////
            typedef PointRlmFilter                              Self;
            typedef boost::shared_ptr<Self>                     Pointer;
            typedef T_BoundaryHandlerClass<T_InputImageType>    BoundaryHandlerType;
            /////////////////////////////////////////////////////////////////////////////////

            //// F A C T O R Y //////////////////////////////////////////////////////////////
            /**
             * Factory method for instantiating a PointRlmFilter object.
             * Parameters:
             *   - image -- Pointer to image on which to apply the filter
             *   - region -- 3D region within which to compute RLM
             *   - offsets -- Vector of direction/distance offsets
             *   - numGraylevels -- Number of graylevel bins to use for dynamic quantization
             */
            static Pointer New(const typename T_InputImageType::ConstantPointer &image, const Region3D<int> &region, const std::vector<Point3D<int>> &offsets, int numGraylevels)
            {
                Pointer obj(new Self);      // boost::shared_ptr<PointRlmFilter> obj(new PointRlmFilter)
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
             * Applies the PointRlmFilter at the specified point.
             */
            void apply(const Point3D<int> &point, long index)
            {
                if (isPreviousIndex(index)) return;
//clock.tic();
                // Initialize window iterator
                m_imageIterator.setWindowOrigin(point, index);

                RunLengthMatrix rlm;
                Point3D<int> pointOffset;
                PointIndexObject offset;

                // Reset results vector
                m_results.clear();

                // Loop through vector of offsets
                for (auto iter = m_pointOffsets.begin(); iter != m_pointOffsets.end(); iter++)
                {
                    Point3D<int> pointOffset = *iter;
                    //cout << "Processing offset " << pointOffset << endl;

                    // Use RlmHelper to create RLM
                    rlm.clear();
                    offset = RlmHelper::GetOffset(pointOffset, m_image);
                    rlm = RlmHelper::ComputeFromIteratorWithDynamicQuantization<typename T_InputImageType::ConstantPointer, ImageWindowIteratorWithPoint>
                        (m_image,
                         m_numGraylevels, 
                         m_imageIterator, 
                         offset, 
                         [&](const pcl::Point3D<int> &point, int index)
                         {
                             return m_imageIterator.getRegion().contain(point);
                         },
                         d_useOldBinCounting);

                    // Compute RLM features and add to results vector
                    m_results.push_back(RlmFeatures(rlm));
                }
//elapsed += clock.toc().getClock();

                // Compute aggregate feature values (e.g. MEAN and RANGE)
                computeAggregateResults();
            }

            /**
             * Returns the entire results vector.
             */
            std::vector<RlmFeatures> getResults()
            {
                return m_results;
            }

            /**
             * Returns the value of the feature specified by the given RlmIndex object.
             */
            double getResult(RlmIndex rlmIndex)
            {
                if (rlmIndex.isAggregate())
                {
                    switch (rlmIndex.getAggregateIndex())
                    {
                        case RlmIndex::AGGREGATE_MEAN:  return getMeanResult(rlmIndex.getFeatureIndex());
                        case RlmIndex::AGGREGATE_RANGE: return getRangeResult(rlmIndex.getFeatureIndex());
                        default: return std::numeric_limits<double>::quiet_NaN();
                    }
                }
                else
                {
                    return getResult(rlmIndex.getOffsetIndex(), rlmIndex.getFeatureIndex());
                }
            }

            /**
             * Returns the value of the feature specified by the given numerical index.
             * The RlmIndex class is used to decode the numerical index.
             */
            double getResult(int index)
            {
                return getResult(RlmIndex::decodeIndex(index));
            }

            /**
             * Returns the value of the specified feature.
             */
            double getResult(int offsetIndex, int featureIndex)
            {
                switch (featureIndex)
                {
                    case RlmIndex::SHORT_RUN_EMPHASIS:                return m_results[offsetIndex].getShortRunEmphasis();
                    case RlmIndex::LONG_RUN_EMPHASIS:                 return m_results[offsetIndex].getLongRunEmphasis();
                    case RlmIndex::LOW_GRAYLEVEL_RUN_EMPHASIS:        return m_results[offsetIndex].getLowGraylevelRunEmphasis();
                    case RlmIndex::HIGH_GRAYLEVEL_RUN_EMPHASIS:       return m_results[offsetIndex].getHighGraylevelRunEmphasis();
                    case RlmIndex::SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS:  return m_results[offsetIndex].getShortRunLowGraylevelEmphasis();
                    case RlmIndex::SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS: return m_results[offsetIndex].getShortRunHighGraylevelEmphasis();
                    case RlmIndex::LONG_RUN_LOW_GRAYLEVEL_EMPHASIS:   return m_results[offsetIndex].getLongRunLowGraylevelEmphasis();
                    case RlmIndex::LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS:  return m_results[offsetIndex].getLongRunHighGraylevelEmphasis();
                    case RlmIndex::GRAYLEVEL_NONUNIFORMITY:           return m_results[offsetIndex].getGraylevelNonuniformity();
                    case RlmIndex::RUNLENGTH_NONUNIFORMITY:           return m_results[offsetIndex].getRunlengthNonuniformity();
                    case RlmIndex::RUN_PERCENTAGE:                    return m_results[offsetIndex].getRunPercentage();

                    default:                                          return std::numeric_limits<double>::quiet_NaN();
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
            std::vector<RlmFeatures>                        m_results;
            std::vector<RlmAggregateStruct>                 m_resultsAggregate;

            // Debug switches
            bool d_useOldBinCounting;
            /////////////////////////////////////////////////////////////////////////////////

            
            //// P R O T E C T E D  M E T H O D S ///////////////////////////////////////////
            PointRlmFilter()
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
                m_resultsAggregate.resize(RlmIndex::NUM_RLM_FEATURES);

                for (int f = 0; f < RlmIndex::NUM_RLM_FEATURES; f++)
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

                    m_resultsAggregate[f] = RlmAggregateStruct(mean, range);
                }
            }
            /////////////////////////////////////////////////////////////////////////////////
        };
    }
}

#endif
