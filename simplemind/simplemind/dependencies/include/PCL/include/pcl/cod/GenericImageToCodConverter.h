#ifndef PCL_GENERIC_IMAGE_TO_COD_CONVERTER
#define PCL_GENERIC_IMAGE_TO_COD_CONVERTER

#include <pcl/cod/CodBase.h>
#include <pcl/image.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		class GenericImageToCodConverter: public CodBase<true>
		{
		public:
			typedef GenericImageToCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef double ReturnType;
			typedef double InternalReturnType;

			static Pointer New(const typename ImagePhysicalLayer::ConstantPointer& image)
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
				return m_Image->getValue(iter);
			}

			inline InternalReturnType get(const Point3D<int>& p) const
			{
				return m_Image->getValue(p);
			}

            inline InternalReturnType get(const Point3D<int>& /*p*/, long index) const
			{
				return m_Image->getValue(index);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return DummyImage::New(m_Image);
			}

		protected:
			typename ImagePhysicalLayer::ConstantPointer m_Image;

			ImageToCodConverter() {}

			void setImage(const typename ImagePhysicalLayer::ConstantPointer& image) 
			{
				m_Image = image;
				setRegionInfo(image);
			}
		};

	}
}

#endif
