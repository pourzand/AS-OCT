#ifndef PCL_IMAGE_PYRAMID_HELPER
#define PCL_IMAGE_PYRAMID_HELPER

#ifndef NO_VNL

#include <pcl/filter/resampler/ImageResampleHelper.h>
#include <pcl/filter/image/ImageGaussianFilter.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;

		class ImagePyramidHelper
		{
		public:
			template <class ResultImagePointer, template<class> class Interpolator, class ImagePointerType>
			static typename std::vector<ResultImagePointer> GetPyramid(const ImagePointerType& image, int level_num, const Point3D<double>& sampling_rate)
			{
				typedef typename ptr_base_type<ResultImagePointer>::type ResultImageType;
				std::vector<ResultImagePointer> result(level_num);
				auto trans = pcl::geometry::AffineTransformation::New();
				trans->addScaling(sampling_rate);
				for (int i=0; i<level_num; ++i) {
					if (i==0) {
						result[i] = ImageHelper::GetCopyIfNeeded<ResultImagePointer>(image);
					} else {
						result[i] = pcl::filter::ImageResampleHelper::Resample<ResultImageType, Interpolator>(result[i-1], trans, false);
					}
				}
				return std::move(result);
			}
			
			template <class ResultImagePointer, template<class> class Interpolator, class ImagePointerType>
			static typename std::vector<ResultImagePointer> GetGaussianPyramid(const ImagePointerType& image, int level_num, const Point3D<double>& scale, const Point3D<double>& sampling_rate)
			{
				typedef typename ptr_base_type<ResultImagePointer>::type ResultImageType;
				std::vector<ResultImagePointer> result(level_num);
				auto trans = pcl::geometry::AffineTransformation::New();
				trans->addScaling(sampling_rate);
				for (int i=0; i<level_num; ++i) {
					if (i==0) {
						result[i] = ImageHelper::GetCopyIfNeeded<ResultImagePointer>(image);
					} else {
						auto temp = filter::ImageGaussianFilter<ResultImageType, ResultImageType>::Compute(result[i-1], scale.x(), scale.y(), scale.z(), false);
						result[i] = pcl::filter::ImageResampleHelper::Resample<ResultImageType, Interpolator>(temp, trans, false);
					}
				}
				return std::move(result);
			}
			
		};
		
	}
}

#endif

#endif