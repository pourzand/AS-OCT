#ifndef PCL_IMAGE_TO_BOUNDARY_HANDLED_COD
#define PCL_IMAGE_TO_BOUNDARY_HANDLED_COD

#include <pcl/cod/CodBase.h>
#include <pcl/image.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class ImageType, template<class> class BoundaryHandlerClass>
		class ImageToBoundaryHandledCod: public CodBase<false>
		{
		public:
			typedef ImageToBoundaryHandledCod Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename ImageType::IoValueType ReturnType;
			typedef BoundaryHandlerClass<ImageType> BoundaryHandlerType;

			static Pointer New(const typename ImageType::ConstantPointer& image)
			{
				if (!image) {
					pcl_ThrowException(IncompatibleCodException(), "Empty image object provided");
				}
				Pointer obj(new Self);
				obj->setImage(image);
				return obj;
			}

			BoundaryHandlerType& getBoundaryHandler()
			{
				return m_BoundaryHandler;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return m_BoundaryHandler.get(iter.getPoint(), iter);
			}

			inline ReturnType get(const Point3D<int>& p) const
			{
				return m_BoundaryHandler.get(p);
			}

			inline ReturnType get(const Point3D<int>& p, long index) const
			{
				return m_BoundaryHandler.get(p,index);
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				return DummyImage::New(m_Image);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			BoundaryHandlerType m_BoundaryHandler;

			ImageToBoundaryHandledCod() {}

			void setImage(const typename ImageType::ConstantPointer& image) 
			{
				m_Image = image;
				m_BoundaryHandler.setImage(m_Image);
				setRegionInfo();
			}
		};

	}
}

#endif