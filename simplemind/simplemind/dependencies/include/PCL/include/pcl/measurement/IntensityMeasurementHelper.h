#ifndef PCL_INTENSITY_MEASUREMENT_HELPER
#define PCL_INTENSITY_MEASUREMENT_HELPER

#include <pcl/statistics.h>
#include <pcl/image.h>
#include <pcl/type_utility.h>

namespace pcl
{
	namespace measurement
	{

		class IntensityMeasurementHelper
		{
		public:
			template <class ImagePointer, class MaskPointer>
			static boost::shared_ptr<pcl::statistics::PercentileCalculator<typename pcl::ptr_base_type<ImagePointer>::type::IoValueType>> GetPercentileCalculator(const ImagePointer& image, const MaskPointer& mask)
			{
				boost::shared_ptr<pcl::statistics::PercentileCalculator<typename pcl::ptr_base_type<ImagePointer>::type::IoValueType>> calculator;
				calculator.reset(new pcl::statistics::PercentileCalculator<typename pcl::ptr_base_type<ImagePointer>::type::IoValueType>());
				pcl::ImageIteratorWithPoint iter(image);
				pcl_ForIterator(iter) if (mask->get(iter)!=0) {
					calculator->addValue(image->get(iter));
				}
				return calculator;
			}

			template <class ImagePointer, class MaskPointer>
			static boost::shared_ptr<pcl::statistics::StatisticsCalculator> GetStatisticsCalculator(const ImagePointer& image, const MaskPointer& mask)
			{
				boost::shared_ptr<pcl::statistics::StatisticsCalculator> calculator;
				calculator.reset(new pcl::statistics::StatisticsCalculator());
				pcl::ImageIteratorWithPoint iter(image);
				pcl_ForIterator(iter) if (mask->get(iter)!=0) {
					calculator->addValue(image->get(iter));
				}
				return calculator;
			}
		};

	}
}

#endif