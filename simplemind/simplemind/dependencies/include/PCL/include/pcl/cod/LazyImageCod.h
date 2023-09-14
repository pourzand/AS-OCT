#ifndef PCL_LAZY_IMAGE_COD
#define PCL_LAZY_IMAGE_COD

#include <functional>
#include <pcl/cod/CodBase.h>
#include <pcl/image/ImageHelper.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class ImageType>
		class LazyImageCod: public CodBase<true>
		{
		public:
			typedef LazyImageCod Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename ImageType::IoValueType ReturnType;

			template <class FunctionType, class TemplateImagePointerType>
			static Pointer New(FunctionType& func, TemplateImagePointerType& template_image, const pcl::Region3D<int>& region=pcl::Region3D<int>().reset())
			{
				if (!template_image) {
#ifndef NO_WARNING
					std::cout << "Warning at LazyImageCod::New(): Empty template image provided, a NULL pointer is returned!" << std::endl;
#endif
					return Pointer();
				}
				Pointer obj(new Self);
				obj->setRegionInfo(template_image, region);
				obj->m_Func = func;
				return obj;
			}


			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				eval();
				return m_Image->get(iter);
			}
			
			inline ReturnType get(const Point3D<int>& p) const
			{
				eval();
				return m_Image->get(p);
			}

			inline ReturnType get(const Point3D<int>& /*p*/, long i) const
			{
				eval();
				return m_Image->get(i);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_TemplateImage;
			}
			
		protected:
			DummyImage::ConstantPointer m_TemplateImage;
			std::function<typename ImageType::Pointer()> m_Func;
			typename ImageType::Pointer m_Image;

			LazyImageCod() {}

			using CodBase<true>::setRegionInfo;
			
			void eval()
			{
				if (!m_Image) {
					m_Image = m_Func();
					if (!pcl::ImageHelper::IsMemoryStructureSame(m_Image, m_TemplateImage) || 
						m_Image->getMinPoint()!=m_TemplateImage->getMinPoint() ||
						m_Image->getMaxPoint()!=m_TemplateImage->getMaxPoint()
					) {
						pcl_ThrowException(pcl::Exception(), "LazyImageCod::eval(): Encountered difference between template and resulting image!");
					}
				}
			}

			template <class ImagePointerType>
			void localSetRegionInfo(const ImagePointerType& image, const pcl::Region3D<int>& region)
			{
				if (region.empty()) {
					if (boost::is_same<typename pcl::ptr_base_type<ImagePointerType>::type, DummyImage>::value) m_TemplateImage = image;
					else m_TemplateImage = DummyImage::New(image);
				} else {
					m_TemplateImage = DummyImage::New(region.getMinPoint(), region.getMaxPoint(), image->getSpacing(), image->getOrigin(), image->getOrientationMatrix());
				}
				setRegionInfo(m_TemplateImage->localToIndex(m_TemplateImage->getMinPoint()), m_TemplateImage->localToIndex(m_TemplateImage->getMaxPoint()), m_TemplateImage->getRegion());
			}
		};

	}
}

#endif