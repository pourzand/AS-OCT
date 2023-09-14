#ifndef PCL_POINT_FILTER_TO_COD_CONVERTER
#define PCL_POINT_FILTER_TO_COD_CONVERTER

#include <pcl/cod/CodBase.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class FType, bool Flag=false>
		class PointFilterToCodConverter: public CodBase<Flag>
		{
		public:
			typedef PointFilterToCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef FType FilterType;
			typedef double ReturnType;

			static Pointer New(const typename FilterType::Pointer& filter, int result_index)
			{
				if (!filter) {
					pcl_ThrowException(IncompatibleCodException(), "Empty filter object provided");
				}
				Pointer obj(new Self);
				obj->m_Filter = filter;
				obj->m_ResultIndex = result_index;
				obj->setRegionInfo();
				return obj;
			}

			template<class ImagePointerType>
			static Pointer New(const typename FilterType::Pointer& filter, const ImagePointerType& image, int result_index)
			{
				if (!filter) {
#ifndef NO_WARNING
					std::cout << "Warning at PointFilterToCodConverter::New(): Empty filter object provided, a NULL pointer is returned!" << std::endl;
#endif
					return Pointer();
				}
				Pointer obj(new Self);
				obj->m_Filter = filter;
				obj->m_ResultIndex = result_index;
				if (image) obj->localSetRegionInfo(image);
				else obj->setRegionInfo();
				return obj;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return get(iter.getPoint(), iter);
			}
			
			inline ReturnType get(const Point3D<int>& p) const
			{
				m_Filter->apply(p);
				return m_Filter->getResult(m_ResultIndex);
			}

			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				m_Filter->apply(p, i);
				return m_Filter->getResult(m_ResultIndex);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_TemplateImage;
			}
			
		protected:
			int m_ResultIndex;
			typename FilterType::Pointer m_Filter;
			DummyImage::ConstantPointer m_TemplateImage;

			PointFilterToCodConverter() {}

			using CodBase<Flag>::setRegionInfo;

			template <class ImagePointerType>
			typename boost::enable_if<boost::is_same<typename pcl::ptr_base_type<ImagePointerType>::type, DummyImage>>::type localSetRegionInfo(const ImagePointerType& image) 
			{
				setRegionInfo(image->localToIndex(image->getMinPoint()), image->localToIndex(image->getMaxPoint()), image->getRegion());
				m_TemplateImage = image;
			}

			template <class ImagePointerType>
			typename boost::disable_if<boost::is_same<typename pcl::ptr_base_type<ImagePointerType>::type, DummyImage>>::type localSetRegionInfo(const ImagePointerType& image) 
			{
				setRegionInfo(image->localToIndex(image->getMinPoint()), image->localToIndex(image->getMaxPoint()), image->getRegion());
				m_TemplateImage = DummyImage::New(image);
			}
		};

	}
}

#endif