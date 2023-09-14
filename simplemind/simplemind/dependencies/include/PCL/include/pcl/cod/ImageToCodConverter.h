#ifndef PCL_IMAGE_TO_COD_CONVERTER
#define PCL_IMAGE_TO_COD_CONVERTER

#include <pcl/cod/CodBase.h>
#include <pcl/image.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class ImageType>
		class ImageToCodConverter: public CodBase<true>
		{
		public:
			typedef ImageToCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename ImageType::IoValueType ReturnType;
			typedef decltype(boost::declval<ImageType>().get(long())) InternalReturnType;

			static Pointer New(const typename ImageType::ConstantPointer& image)
			{
				if (!image) {
					pcl_ThrowException(IncompatibleCodException(), "Empty image object provided");
				}
				Pointer obj(new Self);
				obj->setImage(image);
				return obj;
			}

			template <class IteratorType>
			inline InternalReturnType get(const IteratorType& iter) const
			{
				return m_Image->get(iter);
			}

			inline InternalReturnType get(const Point3D<int>& p) const
			{
				return m_Image->get(p);
			}

            inline InternalReturnType get(const Point3D<int>& /*p*/, long index) const
			{
				return m_Image->get(index);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return DummyImage::New(m_Image);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;

			ImageToCodConverter() {}

			void setImage(const typename ImageType::ConstantPointer& image) 
			{
				m_Image = image;
				setRegionInfo(image);
			}
		};

	}
}

#endif
