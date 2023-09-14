#ifndef PCL2_FIXED_VALUE_BOUNDARY
#define PCL2_FIXED_VALUE_BOUNDARY

#include <pcl/image.h>

namespace pcl
{
	namespace filter2
	{

		template <class ImgType>
		class FixedValueBoundary
		{
		public:
			typedef ImgType ImageType;
		
			FixedValueBoundary() 
			{
				m_BoundaryValue = typename ImageType::IoValueType();
			}
			FixedValueBoundary(const typename ImageType::ConstantPointer& img, typename ImageType::IoValueType val=ImageType::IoValueType()) 
			{
				m_BoundaryValue = val;
				m_Image = img;
			}
			FixedValueBoundary(const FixedValueBoundary& obj)
			{
				*this = obj;
			}

			FixedValueBoundary& operator=(const FixedValueBoundary& obj)
			{
				m_BoundaryValue = obj.m_BoundaryValue;
				m_Image = obj.m_Image;
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Image;
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