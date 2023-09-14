#ifndef PCL_GLOBAL_FIXED_VALUE_BOUNDARY_HANDLER
#define PCL_GLOBAL_FIXED_VALUE_BOUNDARY_HANDLER

#include <pcl/image.h>

namespace pcl
{
	namespace filter
	{

		template <class ImageType>
		class GlobalFixedValueBoundaryHandler: private boost::noncopyable
		{
		public:
			static typename ImageType::IoValueType BoundaryValue;
			
			GlobalFixedValueBoundaryHandler() {}
			GlobalFixedValueBoundaryHandler(const typename ImageType::ConstantPointer& img) 
			{
				setImage(img);
			}
			
			GlobalFixedValueBoundaryHandler& operator=(const GlobalFixedValueBoundaryHandler& obj)
			{
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage()
			{
				return m_Image;
			}
			
			void setImage(const typename ImageType::ConstantPointer& img)
			{
				m_Image = img;
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p) const
			{
				if (m_Image->contain(p)) return m_Image->get(p);
				else return BoundaryValue;
			}

			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				if (m_Image->contain(p)) return m_Image->get(index);
				else return BoundaryValue;
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint(), iter);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
		};
		template <class ImageType> typename ImageType::IoValueType GlobalFixedValueBoundaryHandler<ImageType>::BoundaryValue;

	}
}

#endif