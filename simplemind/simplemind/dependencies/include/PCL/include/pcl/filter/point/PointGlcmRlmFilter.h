#ifndef PCL_POINT_GLCM_RLM_FILTER
#define PCL_POINT_GLCM_RLM_FILTER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/filter/glcm/GlcmFeatures.h>
#include <pcl/filter/glcm/GlcmHelper.h>
#include <pcl/filter/glcm/GrayLevelCooccurrenceMatrix.h>
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
        class PointGlcmRlmFilter : public PointFilterBase
        {
        public:
            //// S T R U C T ////////////////////////////////////////////////////////////////
            /**
             * Helper class for representing a specific feature after applying PointGlcmRlmFilter.
             * A FeatureIndex is defined by two pieces of information:
             *   - the step offset (direction and distance; or range/mean)
             *   - the base feature (e.g. GLCM contrast, RLM short run emphasis, etc)
             * The step and base feature are mapped to a numeric index which summarizes this information.
             *
             * Three different factory methods are provided to create a GlcmIndex object.
             *   - encodeOffset     --  Create FeatureIndex based on offset, feature
             *   - encodeAggregate  --  Create FeatureIndex based on aggregation (range/mean), feature
             *   - decodeIndex      --  Create FeatureIndex based on numeric index
             */
            class FeatureIndex
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
                    GLCM_CONTRAST = 0,                   // Relies upon the first feature being equal to 0, then each additional feature being sequential
                    GLCM_DISSIMILARITY,
                    GLCM_HOMOGENEITY,
                    GLCM_ANGULAR_SECOND_MOMENT,
                    GLCM_ENERGY,
                    GLCM_MAX,
                    GLCM_ENTROPY,
                    GLCM_MEAN,
                    GLCM_VARIANCE,
                    GLCM_STANDARD_DEVIATION,
                    GLCM_CORRELATION,
                    GLCM_SUM_AVERAGE,
                    GLCM_SUM_VARIANCE,
                    GLCM_SUM_ENTROPY,
                    GLCM_DIFF_AVERAGE,
                    GLCM_DIFF_VARIANCE,
                    GLCM_DIFF_ENTROPY,
                    GLCM_INFO_CORRELATION_A,
                    GLCM_INFO_CORRELATION_B,
                    GLCM_MAX_CORRELATION_COEFFICIENT,
                    RLM_SHORT_RUN_EMPHASIS,           
                    RLM_LONG_RUN_EMPHASIS,
                    RLM_LOW_GRAYLEVEL_RUN_EMPHASIS,
                    RLM_HIGH_GRAYLEVEL_RUN_EMPHASIS,
                    RLM_SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS,
                    RLM_SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS,
                    RLM_LONG_RUN_LOW_GRAYLEVEL_EMPHASIS,
                    RLM_LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS,
                    RLM_GRAYLEVEL_NONUNIFORMITY,
                    RLM_RUNLENGTH_NONUNIFORMITY,
                    RLM_RUN_PERCENTAGE,
                    NUM_FEATURES               // Must be the last item enum
                };

                /**
                 * Factory method to create a FeatureIndex object based on the specified offset and base feature.
                 * Parameters:
                 *   - offsetIndex -- an index into the vector of offsets passed when calling PointGlcmRlmFilter::New( ... )
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, defined above
                 */
                static FeatureIndex encodeOffset(int offsetIndex, int featureIndex)
                {
                    return FeatureIndex(offsetIndex + NUM_AGGREGATIONS, featureIndex);
                }

                /**
                 * Factory method to create a FeatureIndex object based on an aggregation and base feature.
                 * Parameters:
                 *   - aggregateIndex -- an element of the enum AggregateIndexEnum, defined above
                 *   - featureIndex -- an element of the enum FeatureIndexEnum, above
                 */
                static FeatureIndex encodeAggregate(int aggregateIndex, int featureIndex)
                {
                    return FeatureIndex(aggregateIndex, featureIndex);
                }

                /**
                 * Factory method to create a FeatureIndex object based on a numeric index (as returned by getIndex()).
                 * Parameters:
                 *   - index -- a numeric index representing a FeatureIndex
                 */
                static FeatureIndex decodeIndex(int index)
                {
                    return FeatureIndex(index);
                }

                /**
                 * Returns the numeric index associated with this FeatureIndex object.
                 */
                int getIndex() const
                {
                    return m_index;
                }

                /**
                 * Returns the aggregate index associated with this FeatureIndex object.
                 * If the FeatureIndex object does not represent an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 * The return value corresponds to AggregateIndexEnum.
                 */
                int getAggregateIndex() const
                {
                    return m_aggregateIndex;
                }

                /**
                 * Returns the offset index associated with this FeatureIndex object. The offset is indexed
                 * in the same order as the vector of offsets passed to PointGlcmRlmFilter::New( ... ).
                 * If the FeatureIndex object represents an aggregate feature (e.g. MEAN or RANGE),
                 * then the return value is ill-defined.
                 */
                int getOffsetIndex() const
                {
                    return m_aggregateIndex - NUM_AGGREGATIONS;
                }

                /**
                 * Returns the base feature index associated with this FeatureIndex object.
                 * The return value corresponds to FeatureIndexEnum.
                 */
                int getBaseFeatureIndex() const
                {
                    return m_featureIndex;
                }

                /**
                 * Returns true if the FeatureIndex object refers to a Glcm feature.
                 * Relies on hard-coded ordering of Glcm features in the FeatureIndexEnum.
                 */
                bool isGlcm() const
                {
                    return getFeatureIndex() >= GLCM_CONTRAST && getFeatureIndex() <= GLCM_MAX_CORRELATION_COEFFICIENT;
                }

                /**
                 * Returns true if the FeatureIndex object refers to a Rlm feature.
                 * Relies on hard-coded ordering of Rlm features in the FeatureIndexEnum.
                 */
                bool isRlm() const
                {
                    return getFeatureIndex() >= RLM_SHORT_RUN_EMPHASIS && getFeatureIndex() <= RLM_RUN_PERCENTAGE;
                }

                /**
                 * Returns true if this FeatureIndex object represents an aggregate feature (e.g. MEAN or RANGE).
                 */
                bool isAggregate() const
                {
                    return m_aggregateIndex < NUM_AGGREGATIONS;
                }

                std::string describe() const
                {
                    if (isAggregate())
                    {
                        return describeAggregation(getAggregateIndex()) + "_" + describeFeature(getBaseFeatureIndex());
                    }
                    else
                    {
                        return "O" + boost::lexical_cast<std::string>(getOffsetIndex()) + "_" + describeFeature(getBaseFeatureIndex());
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
                    case GLCM_CONTRAST:                         return "GLCM_CONTRAST";
                    case GLCM_DISSIMILARITY:                    return "GLCM_DISSIMILARITY";
                    case GLCM_HOMOGENEITY:                      return "GLCM_HOMOGENEITY";
                    case GLCM_ANGULAR_SECOND_MOMENT:            return "GLCM_ANGULAR_SECOND_MOMENT";
                    case GLCM_ENERGY:                           return "GLCM_ENERGY";
                    case GLCM_MAX:                              return "GLCM_MAX";
                    case GLCM_ENTROPY:                          return "GLCM_ENTROPY";
                    case GLCM_MEAN:                             return "GLCM_MEAN";
                    case GLCM_VARIANCE:                         return "GLCM_VARIANCE";
                    case GLCM_STANDARD_DEVIATION:               return "GLCM_STANDARD_DEVIATION";
                    case GLCM_CORRELATION:                      return "GLCM_CORRELATION";
                    case GLCM_SUM_AVERAGE:                      return "GLCM_SUM_AVERAGE";
                    case GLCM_SUM_VARIANCE:                     return "GLCM_SUM_VARIANCE";
                    case GLCM_SUM_ENTROPY:                      return "GLCM_SUM_ENTROPY";
                    case GLCM_DIFF_AVERAGE:                     return "GLCM_DIFF_AVERAGE";
                    case GLCM_DIFF_VARIANCE:                    return "GLCM_DIFF_VARIANCE";
                    case GLCM_DIFF_ENTROPY:                     return "GLCM_DIFF_ENTROPY";
                    case GLCM_INFO_CORRELATION_A:               return "GLCM_INFO_CORRELATION_A";
                    case GLCM_INFO_CORRELATION_B:               return "GLCM_INFO_CORRELATION_B";
                    case GLCM_MAX_CORRELATION_COEFFICIENT:      return "GLCM_MAX_CORRELATION_COEFFICIENT";
                    case RLM_SHORT_RUN_EMPHASIS:                return "RLM_SHORT_RUN_EMPHASIS";
                    case RLM_LONG_RUN_EMPHASIS:                 return "RLM_LONG_RUN_EMPHASIS";
                    case RLM_LOW_GRAYLEVEL_RUN_EMPHASIS:        return "RLM_LOW_GRAYLEVEL_RUN_EMPHASIS";
                    case RLM_HIGH_GRAYLEVEL_RUN_EMPHASIS:       return "RLM_HIGH_GRAYLEVEL_RUN_EMPHASIS";
                    case RLM_SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS:  return "RLM_SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS";
                    case RLM_SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS: return "RLM_SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS";
                    case RLM_LONG_RUN_LOW_GRAYLEVEL_EMPHASIS:   return "RLM_LONG_RUN_LOW_GRAYLEVEL_EMPHASIS";
                    case RLM_LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS:  return "RLM_LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS";
                    case RLM_GRAYLEVEL_NONUNIFORMITY:           return "RLM_GRAYLEVEL_NONUNIFORMITY";
                    case RLM_RUNLENGTH_NONUNIFORMITY:           return "RLM_RUNLENGTH_NONUNIFORMITY";
                    case RLM_RUN_PERCENTAGE:                    return "RLM_RUN_PERCENTAGE";
                    default:                                    return "<INVALID FEATURE>";
                    }
                }

            private:
                int m_index;
                int m_aggregateIndex;
                int m_featureIndex;

                FeatureIndex(int index)
                {
                    m_index = index;
                    m_aggregateIndex = (int)(index / NUM_FEATURES);
                    m_featureIndex = index % NUM_FEATURES;
                }

                FeatureIndex(int aggregateIndex, int featureIndex)
                {
                    m_index = aggregateIndex * NUM_FEATURES + featureIndex;
                    m_aggregateIndex = aggregateIndex;
                    m_featureIndex = featureIndex;
                }
            };

            struct AggregateStruct
            {
                double mean, range;
                
                AggregateStruct() {}

                AggregateStruct(double m, double r)
                {
                    mean = m;
                    range = r;
                }
            };

            struct BinRunLengthStruct
            {
                int binNumber;
                int runLength;
                
                BinRunLengthStruct(int b, int r)
                {
                    binNumber = b;
                    runLength = r;
                }
                
                friend std::ostream &operator<<(std::ostream &out, const BinRunLengthStruct &obj)
                {
                    out << "(" << obj.binNumber << ", " << obj.runLength << ")";
                    return out;
                }
            };
            /////////////////////////////////////////////////////////////////////////////////

            //// T Y P E D E F S ////////////////////////////////////////////////////////////
            typedef PointGlcmRlmFilter                          Self;
            typedef boost::shared_ptr<Self>                     Pointer;
            typedef T_InputImageType                            ImageInputType;
            typedef Image<int, true>                            ImageIntType;
            typedef Image<char, true>                           ImageMaskType;
            typedef Image<bool, true>                           ImageBoolType;
            typedef Point3D<int>                                PointIntType;

            typedef T_BoundaryHandlerClass<ImageInputType>      BoundaryHandlerType;
            typedef T_BoundaryHandlerClass<ImageMaskType>       MaskBoundaryHandlerType;
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
            void provideImageMask(ImageMaskType::ConstantPointer mask)
            {
                m_mask = mask;
                m_boundaryHandlerMask.setImage(mask);
            }

            bool hasImageMask() const
            {
                return m_mask;
            }

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

                // Obtain image window
                ImageIntType::Pointer window = buildImageWindowWithDynamicQuantization(point, index, m_numGraylevels);

                // Initialize matrices
                GrayLevelCooccurrenceMatrix glcm(m_numGraylevels, true);
                RunLengthMatrix rlm;

                // Reset results vectors
                m_glcmResults.clear();
                m_rlmResults.clear();

                // Point checker
                auto pointCheckerNull = [this](const PointIntType &p, long index)->bool
                {
                    return m_image->contain(p);
                };                
                auto pointCheckerWithMask = [this](const PointIntType &p, long index)->bool
                {
                    return m_image->contain(p) && m_mask->get(p) > 0;
                };


                // Loop through vector of offsets
                for (auto iter = m_pointOffsets.begin(); iter != m_pointOffsets.end(); iter++)
                {
                    Point3D<int> pointOffset = *iter;
                    PointIndexObject offset = GlcmHelper::GetOffset(pointOffset, window);

                    glcm.clear();
                    rlm.clear();
                    if (hasImageMask())
                        computeFromImage(glcm, rlm, window, m_numGraylevels, offset, pointCheckerWithMask);
                    else
                        computeFromImage(glcm, rlm, window, m_numGraylevels, offset, pointCheckerNull);

                    m_glcmResults.push_back(GlcmFeatures(glcm));
                    m_rlmResults.push_back(RlmFeatures(rlm));
                }

                // Compute aggregate feature values (e.g. MEAN and RANGE)
                computeAggregateResults();
            }

            /**
             * Returns the value of the feature specified by the given FeatureIndex object.
             */
            double getResult(FeatureIndex featureIndex)
            {
                if (featureIndex.isAggregate())
                {
                    switch (featureIndex.getAggregateIndex())
                    {
                        case FeatureIndex::AGGREGATE_MEAN:  return getMeanResult(featureIndex.getBaseFeatureIndex());
                        case FeatureIndex::AGGREGATE_RANGE: return getRangeResult(featureIndex.getBaseFeatureIndex());
                        default: return std::numeric_limits<double>::quiet_NaN();
                    }
                }
                else
                {
                    return getResult(featureIndex.getOffsetIndex(), featureIndex.getBaseFeatureIndex());
                }
            }

            /**
             * Returns the value of the feature specified by the given numerical index.
             * The GlcmIndex class is used to decode the numerical index.
             */
            double getResult(int index)
            {
                return getResult(FeatureIndex::decodeIndex(index));
            }

            double getResult(int offsetIndex, int featureIndex)
            {
                switch (featureIndex)
                {
                    case FeatureIndex::GLCM_CONTRAST:                           return m_glcmResults[offsetIndex].getContrast();
                    case FeatureIndex::GLCM_DISSIMILARITY:                      return m_glcmResults[offsetIndex].getDissimilarity();
                    case FeatureIndex::GLCM_HOMOGENEITY:                        return m_glcmResults[offsetIndex].getHomogeneity();
                    case FeatureIndex::GLCM_ANGULAR_SECOND_MOMENT:              return m_glcmResults[offsetIndex].getAngularSecondMoment();
                    case FeatureIndex::GLCM_ENERGY:                             return m_glcmResults[offsetIndex].getEnergy();
                    case FeatureIndex::GLCM_MAX:                                return m_glcmResults[offsetIndex].getMax();
                    case FeatureIndex::GLCM_ENTROPY:                            return m_glcmResults[offsetIndex].getEntropy();
                    case FeatureIndex::GLCM_MEAN:                               return m_glcmResults[offsetIndex].getMean();
                    case FeatureIndex::GLCM_VARIANCE:                           return m_glcmResults[offsetIndex].getVariance();
                    case FeatureIndex::GLCM_STANDARD_DEVIATION:                 return m_glcmResults[offsetIndex].getStandardDeviation();
                    case FeatureIndex::GLCM_CORRELATION:                        return m_glcmResults[offsetIndex].getCorrelation();
                    case FeatureIndex::GLCM_SUM_AVERAGE:                        return m_glcmResults[offsetIndex].getSumAverage();
                    case FeatureIndex::GLCM_SUM_VARIANCE:                       return m_glcmResults[offsetIndex].getSumVariance();
                    case FeatureIndex::GLCM_SUM_ENTROPY:                        return m_glcmResults[offsetIndex].getSumEntropy();
                    case FeatureIndex::GLCM_DIFF_AVERAGE:                       return m_glcmResults[offsetIndex].getDiffAverage();
                    case FeatureIndex::GLCM_DIFF_VARIANCE:                      return m_glcmResults[offsetIndex].getDiffVariance();
                    case FeatureIndex::GLCM_DIFF_ENTROPY:                       return m_glcmResults[offsetIndex].getDiffEntropy();
                    case FeatureIndex::GLCM_INFO_CORRELATION_A:                 return m_glcmResults[offsetIndex].getInfoCorrelationA();
                    case FeatureIndex::GLCM_INFO_CORRELATION_B:                 return m_glcmResults[offsetIndex].getInfoCorrelationB();
                    case FeatureIndex::GLCM_MAX_CORRELATION_COEFFICIENT:        return m_glcmResults[offsetIndex].getMaximalCorrelationCoefficient();
                    case FeatureIndex::RLM_SHORT_RUN_EMPHASIS:                  return m_rlmResults[offsetIndex].getShortRunEmphasis();
                    case FeatureIndex::RLM_LONG_RUN_EMPHASIS:                   return m_rlmResults[offsetIndex].getLongRunEmphasis();
                    case FeatureIndex::RLM_LOW_GRAYLEVEL_RUN_EMPHASIS:          return m_rlmResults[offsetIndex].getLowGraylevelRunEmphasis();
                    case FeatureIndex::RLM_HIGH_GRAYLEVEL_RUN_EMPHASIS:         return m_rlmResults[offsetIndex].getHighGraylevelRunEmphasis();
                    case FeatureIndex::RLM_SHORT_RUN_LOW_GRAYLEVEL_EMPHASIS:    return m_rlmResults[offsetIndex].getShortRunLowGraylevelEmphasis();
                    case FeatureIndex::RLM_SHORT_RUN_HIGH_GRAYLEVEL_EMPHASIS:   return m_rlmResults[offsetIndex].getShortRunHighGraylevelEmphasis();
                    case FeatureIndex::RLM_LONG_RUN_LOW_GRAYLEVEL_EMPHASIS:     return m_rlmResults[offsetIndex].getLongRunLowGraylevelEmphasis();
                    case FeatureIndex::RLM_LONG_RUN_HIGH_GRAYLEVEL_EMPHASIS:    return m_rlmResults[offsetIndex].getLongRunHighGraylevelEmphasis();
                    case FeatureIndex::RLM_GRAYLEVEL_NONUNIFORMITY:             return m_rlmResults[offsetIndex].getGraylevelNonuniformity();
                    case FeatureIndex::RLM_RUNLENGTH_NONUNIFORMITY:             return m_rlmResults[offsetIndex].getRunlengthNonuniformity();
                    case FeatureIndex::RLM_RUN_PERCENTAGE:                      return m_rlmResults[offsetIndex].getRunPercentage();
                    default:                                                    return std::numeric_limits<double>::quiet_NaN();
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
            typename T_InputImageType::ConstantPointer      m_image;                // Image
            Region3D<int>                                   m_region;               // Window from which to compute GLCM and RLM
            std::vector<Point3D<int>>                       m_pointOffsets;
            int                                             m_numGraylevels;

            // Derived parameters
            BoundaryHandlerType                             m_boundaryHandler;
            ImageWindowIteratorWithPoint                    m_imageIterator;
            //Region3D<int>                                   m_safeRegion;

            ImageMaskType::ConstantPointer                  m_mask;                 // Optional mask for image access (e.g. lung roi)
            MaskBoundaryHandlerType                         m_boundaryHandlerMask;

            // Results
            std::vector<GlcmFeatures>                       m_glcmResults;
            std::vector<RlmFeatures>                        m_rlmResults;
            std::vector<AggregateStruct>                    m_resultsAggregate;

            // Debug switches
            bool d_useOldBinCounting;
            /////////////////////////////////////////////////////////////////////////////////

            
            //// P R O T E C T E D  M E T H O D S ///////////////////////////////////////////
            PointGlcmRlmFilter()
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


            ImageIntType::Pointer buildImageWindowWithDynamicQuantization(const PointIntType &point, long index, int numGraylevels)
            {
                // Determine dimensions of window
                PointIntType minp, maxp;
                minp.x() = point.x() + m_region.getMinPoint().x();
                minp.y() = point.y() + m_region.getMinPoint().y();
                minp.z() = point.z() + m_region.getMinPoint().z();
                maxp.x() = point.x() + m_region.getMaxPoint().x();
                maxp.y() = point.y() + m_region.getMaxPoint().y();
                maxp.z() = point.z() + m_region.getMaxPoint().z();

                // Initialize window
                auto window = ImageIntType::New(minp, maxp, m_image->getSpacing(), m_image->getOrigin(), m_image->getOrientationMatrix());

                m_imageIterator.setWindowOrigin(point, index);

                // Determine min and max value of window
                double minValue = std::numeric_limits<double>::infinity();
                double maxValue = -std::numeric_limits<double>::infinity();
                pcl_ForIterator(m_imageIterator)
                {
                    // Replace this with roi mask instead
                    if (m_image->contain(m_imageIterator.getPoint()))
                    {
                        double value = m_image->get(m_imageIterator);

                        if (value < minValue) minValue = value;
                        if (value > maxValue) maxValue = value;
                    }
                }

                // Determine step size
                double step = d_useOldBinCounting ? (maxValue - minValue) / numGraylevels : (maxValue- minValue + 1) / numGraylevels;

                // Build and bin window
                pcl_ForIterator(m_imageIterator)
                {
                    if (m_image->contain(m_imageIterator.getPoint()))
                    {
                        double value = m_image->get(m_imageIterator);

                        int binNumber = static_cast<unsigned>(std::floor((value - minValue) / step));
                        if (d_useOldBinCounting && binNumber >= numGraylevels) binNumber = numGraylevels - 1;
                        
                        window->set(m_imageIterator.getPoint(), binNumber);
                    }
                    else
                    {
                        window->set(m_imageIterator.getPoint(), -1);
                    }
                }

                return window;
            }

            template <class CheckFunc>
            static void computeFromImage(GrayLevelCooccurrenceMatrix &glcm, RunLengthMatrix &rlm, const ImageIntType::Pointer &image, int numGraylevels, const PointIndexObject &offset, CheckFunc &checker)
            {
                ImageIteratorWithPoint iter(image);
                std::vector<BinRunLengthStruct> runLengthList;
                int maxRunlength = 0;

                // Build visited image for RLM
                ImageBoolType::Pointer visited = ImageBoolType::New(image);
                ImageHelper::Fill(visited, false);

                pcl_ForIterator(iter)
                {
                    if (!checker(iter.getPoint(), iter.getIndex()))
                        continue;

                    // Variables named iter refer to the base point from walking through the iterator
                    // Variables named next refer to the points visited by stepping forwards and backwards from the base point
                    PointIntType iterPoint;
                    PointIndexObject nextPoint;
                    int iterValue, nextValue;
                    int runLength;

                    // Obtain current point
                    iterPoint = iter.getPoint();
                    iterValue = image->get(iterPoint);

                    /***** GLCM */
                    // Build neighbor
                    nextPoint.point = iter.getPoint() + offset.point;
                    nextPoint.index = iter.getIndex() + offset.index;

                    // Check neighbor
                    if (image->contain(nextPoint.getPoint()) && checker(nextPoint.point, nextPoint.index))
                    {
                        nextValue = image->get(nextPoint);
                        glcm.add(iterValue, nextValue);
                    }
                    /***** End GLCM */

                    /***** RLM */
                    // If the current voxel has not already been visited
                    if (!visited->get(iterPoint))
                    {
                        // Mark the voxel as having been visited
                        visited->set(iterPoint, true);

                        // Count runlength starting at 1
                        runLength = 1;

                        // Step forwards
                        nextPoint.point = iter.getPoint() + offset.getPoint();
                        nextPoint.index = iter.getIndex() + offset.getIndex();
                        while (image->contain(nextPoint.point) && checker(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            if (nextValue != iterValue) break;                          // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point += offset.point;                        // Step next
                            nextPoint.index += offset.index;
                        }

                        // Step backwards
                        nextPoint.point = iter.getPoint() - offset.getPoint();
                        nextPoint.index = iter.getIndex() - offset.getIndex();
                        while (image->contain(nextPoint.point) && checker(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            if (nextValue != iterValue) break;                      // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point -= offset.point;                        // Step next
                            nextPoint.index -= offset.index;
                        }

                        // Add (bin number, runlength) to the running total
                        runLengthList.push_back(BinRunLengthStruct(iterValue, runLength));

                        // Keep track of maximum runlength encountered
                        if (runLength > maxRunlength) maxRunlength = runLength;
                    }
                    /***** End RLM */
                }

                // Create an empty RLM based on number of bins and max runlength
                rlm.setSize(numGraylevels, maxRunlength);

                // Populate RLM
                for (auto i = runLengthList.begin(); i != runLengthList.end(); i++)
                    rlm.add(i->binNumber, i->runLength);
            }



            void computeAggregateResults()
            {
                m_resultsAggregate.clear();
                m_resultsAggregate.resize(FeatureIndex::NUM_FEATURES);

                // Iterate over all base features (GLCM and RLM)
                for (int f = 0; f < FeatureIndex::NUM_FEATURES; f++)
                {
                    double mean, range;
                    double sum;
                    double min = std::numeric_limits<double>::infinity();
                    double max = -std::numeric_limits<double>::infinity();

                    // Iterate over all offsets
                    sum = 0.0;
                    for (int o = 0; o < m_pointOffsets.size(); o++)
                    {
                        double value = getResult(o, f);

                        sum += value;
                        if (value < min) min = value;
                        if (value > max) max = value;
                    }

                    mean = sum / m_pointOffsets.size();
                    range = max - min;

                    m_resultsAggregate[f] = AggregateStruct(mean, range);
                }
            }
            /////////////////////////////////////////////////////////////////////////////////
        };
    }
}

#endif
