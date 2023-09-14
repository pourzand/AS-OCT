#ifndef _POINT_DESCRIPTIVE_FILTER_
#define _POINT_DESCRIPTIVE_FILTER_
#include <pcl/misc/Timing.h>

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/statistics/PercentileCalculator.h>
#include <pcl/statistics/StatisticsCalculator.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl::iterator;
        using namespace pcl::statistics;

        /**
         * Encapsulates a point filter for computing first-order descriptive features.
         *
         * Example usage:
         *   // Alias common types
         *   typedef pcl::Point3D<int>                                  PointType;
         *   typedef pcl::Region3D<int>                                 RegionType;
         *   typedef pcl::Image<int, true>                              ImageType;
         *   typedef pcl::filter::PointDescriptiveFilter<ImageType>     PointDescriptiveFilterType;
         *
         *   // Load image
         *   auto image = pcl::ImageIoHelper::Read<ImageType>(INPUT_SERIES);
         *
         *   // Define region within which compute features
         *   auto region = RegionType(PointType(-2,-2,-2), PointType(2,2,2));
         *
         *   // Create PointDescriptiveFilter object
         *   auto descFilter = PointDescriptiveFilterType::New(image, region);
         *
         *   // Apply filter
         *   descFilter->apply(TARGET_POINT);
         *
         *   // Retrieve descriptive features
         *   std::cout << descFilter->getResult(PointDescriptiveFilterType::MEAN) << std::endl;
         *   std::cout << descFilter->getResult(PointDescriptiveFilterType::STDEV) << std::endl;
         *   std::cout << descFilter->getResult(PointDescriptiveFilterType::Q1) << std::endl;
         *   std::cout << descFilter->getResult(PointDescriptiveFilterType::MEDIAN) << std::endl;
         *   std::cout << descFilter->getResult(PointDescriptiveFilterType::Q3) << std::endl;
         */
		template <class T_InputImageType, template<class> class T_BoundaryHandlerClass = RepeatingBoundaryHandler>
		class PointDescriptiveFilter : public PointFilterBase
		{
		public:
            //// S T R U C T ////////////////////////////////////////////////////////////////
            class DescriptiveIndex
            {
            public:
                enum FeatureIndexEnum
                {
                    MEAN = 0,
                    STDEV,
                    MIN,
                    Q1,
                    MEDIAN,
                    Q3,
                    MAX,
                    NUM_DESCRIPTIVE_FEATURES
                };

                static std::string describeFeature(int featureIndex)
                {
                    switch (featureIndex)
                    {
                    case MEAN:      return "MEAN";
                    case STDEV:     return "STDEV";
                    case MIN:       return "MIN";
                    case Q1:        return "Q1";
                    case MEDIAN:    return "MEDIAN";
                    case Q3:        return "Q3";
                    case MAX:       return "MAX";
                    default:        return "<INVALID FEATURE>";
                    }
                }
            };
			/////////////////////////////////////////////////////////////////////////////////


            //// T Y P E D E F S ////////////////////////////////////////////////////////////
			typedef PointDescriptiveFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef T_BoundaryHandlerClass<T_InputImageType> BoundaryHandlerType;
            typedef Image<char, true>                           ImageMaskType;
            typedef T_BoundaryHandlerClass<ImageMaskType>   MaskBoundaryHandlerType;
			/////////////////////////////////////////////////////////////////////////////////
			

			//// F A C T O R Y  M E T H O D S ///////////////////////////////////////////////
            /**
             * Factory method for instantiating a PointDescriptiveFilter object.
             * Parameters:
             *   - image -- Pointer to image on which to apply the filter
             *   - region -- 3D region within which to compute features
             */
			static Pointer New(const typename T_InputImageType::ConstantPointer &inputImage, const Region3D<int> &region)
			{
				Pointer obj(new Self);                  // boost::shared_ptr<PointDescriptiveFilter> obj(new PointDescriptiveFilter);

				obj->initialize(inputImage, region);

				return obj;
			}
			/////////////////////////////////////////////////////////////////////////////////


			//// P U B L I C  M E T H O D S /////////////////////////////////////////////////
            void provideImageMask(ImageMaskType::ConstantPointer mask)
            {
                m_mask = mask;
                m_boundaryHandlerMask.setImage(m_mask);
            }

            bool hasImageMask() const
            {
                return m_mask;
            }

            bool checkMask(const Point3D<int> &point)
            {
                return !hasImageMask() || m_mask->get(point) > 0;
            }

            bool checkMaskBoundary(const Point3D<int> &point)
            {
                return !hasImageMask() || m_boundaryHandler.get(point) > 0;
            }

			template <class T_IteratorType>
			void apply(const T_IteratorType &iter)
			{
				apply(iter.getPoint(), iter.getIndex());
			}

			void apply(const Point3D<int> &point)
			{
				apply(point, m_inputImage->localToIndex(point));
			}

            void apply(long index)
            {
                apply(m_inputImage->localToPoint(index), index);
            }

            /**
             * Applies the PointDescriptiveFilter at the specified point.
             */
			void apply(const Point3D<int> &point, long index)
			{
				if (isPreviousIndex(index)) return;
                m_statisticsCalculator.reset();
                m_percentileCalculator = PercentileCalculator<typename T_InputImageType::ValueType>();

				if (m_safeRegion.contain(point))
				{
					// Region of iteration is contained within image, so no further boundary checking is needed
					m_inputIterator.setWindowOrigin(point, index);
					for (m_inputIterator.begin(); !m_inputIterator.end(); m_inputIterator.next())
					{
                        if (checkMask(m_inputIterator.getPoint()))
                        {
                            double value = m_inputImage->get(m_inputIterator);

                            // Update statistics
                            m_statisticsCalculator.addValue(value);
                            m_percentileCalculator.addValue(value);
                        }
					}
				}
				else
				{
                    // Rely on boundary handler for oob indexing
					m_inputIterator.setWindowOrigin(point, index);
					for (m_inputIterator.begin(); !m_inputIterator.end(); m_inputIterator.next())
					{
                        if (checkMaskBoundary(m_inputIterator.getPoint()))
                        {
                            double value = m_boundaryHandler.get(m_inputIterator.getPoint(), m_inputIterator);

                            // Update statistics
                            m_statisticsCalculator.addValue(value);
                            m_percentileCalculator.addValue(value);
                        }
					}
				}
			}

            /**
             * Returns the specified feature value after applying filter.
             * Parameters:
             *   - i -- Element of DescriptiveFeatureEnum, defined above
             */ 
            double getResult(int i)
            {
                switch (i)
                {
                    case DescriptiveIndex::MEAN:      return m_statisticsCalculator.getMean();
                    case DescriptiveIndex::STDEV:     return m_statisticsCalculator.getStandardDeviation();
                    case DescriptiveIndex::MIN:       return m_statisticsCalculator.getMin();
                    case DescriptiveIndex::Q1:        return m_percentileCalculator.getInterpolatedPercentile(0.25);
                    case DescriptiveIndex::MEDIAN:    return m_percentileCalculator.getInterpolatedMedian();
                    case DescriptiveIndex::Q3:        return m_percentileCalculator.getInterpolatedPercentile(0.75);
                    case DescriptiveIndex::MAX:       return m_statisticsCalculator.getMax();
                    default:                          return std::numeric_limits<double>::quiet_NaN();
                }
            }
            /////////////////////////////////////////////////////////////////////////////////


		protected:
			//// P R O T E C T ED  M E M B E R S ////////////////////////////////////////////
			typename T_InputImageType::ConstantPointer	m_inputImage;		        // The image on which to apply the point filter
			Region3D<int>								m_region;			        // The rectangular subimage over which to compute the descriptive features (image offset coordinates)
			
			BoundaryHandlerType							m_boundaryHandler;	        // Boundary handler object specified by template definition
			ImageWindowIteratorWithPoint				m_inputIterator;	        // Iterator to iterate over the region within the image
			Region3D<int>								m_safeRegion;		        // The region of the image which fully contains m_region (image coordinates)

            ImageMaskType::ConstantPointer              m_mask;
            MaskBoundaryHandlerType                      m_boundaryHandlerMask;

			// Results
            StatisticsCalculator                        m_statisticsCalculator;     // Helper to compute descriptive stats
            PercentileCalculator<typename T_InputImageType::IoValueType>
                                                        m_percentileCalculator;     // Helper to compute percentile stats
			/////////////////////////////////////////////////////////////////////////////////


			//// P R O T E C T E D  M E T H O D S ///////////////////////////////////////////
			// Protected constructor
			PointDescriptiveFilter() {}

			void initialize(const typename T_InputImageType::ConstantPointer &inputImage, const Region3D<int> &region)
			{
				m_inputImage = inputImage;		// Shallow copy...?
				m_region = region;				// Deep copy

				m_boundaryHandler.setImage(m_inputImage);

				// Define window iterator
				m_inputIterator.setImage(m_inputImage, m_region);

				// Define safe region of image
				m_safeRegion.set(m_inputImage->getMinPoint() - m_region.getMinPoint(),
					             m_inputImage->getMaxPoint() - m_region.getMaxPoint());
			}
			/////////////////////////////////////////////////////////////////////////////////
		};
	}
}

#endif
