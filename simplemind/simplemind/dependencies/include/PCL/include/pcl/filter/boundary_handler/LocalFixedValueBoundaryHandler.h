#ifndef PCL_LOCAL_FIXED_VALUE_BOUNDARY_HANDLER
#define PCL_LOCAL_FIXED_VALUE_BOUNDARY_HANDLER

#include <pcl/image.h>

namespace pcl
{
	namespace filter
	{

		template <class ImageType>
		class LocalFixedValueBoundaryHandler: private boost::noncopyable
		{
		public:
			LocalFixedValueBoundaryHandler() 
			{
				m_BoundaryValue = typename ImageType::IoValueType();
			}
			LocalFixedValueBoundaryHandler(const typename ImageType::ConstantPointer& img) 
			{
				m_BoundaryValue = typename ImageType::IoValueType();
				setImage(img);
			}

			LocalFixedValueBoundaryHandler& operator=(const LocalFixedValueBoundaryHandler& obj)
			{
				m_BoundaryValue = obj.m_BoundaryValue;
				return *this;
			}

			LocalFixedValueBoundaryHandler& operator=(const typename ImageType::IoValueType& val)
			{
				m_BoundaryValue = val;
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
			
			void setBoundaryValue(typename ImageType::IoValueType val)
			{
				m_BoundaryValue = val;
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p) const
			{
				if (m_Image->contain(p)) return m_Image->get(p);
				else return m_BoundaryValue;
			}

			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				if (m_Image->contain(p)) return m_Image->get(index);
				else return m_BoundaryValue;
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint(), iter);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			typename ImageType::IoValueType m_BoundaryValue;
		};

	}
}

#endif