#ifndef PCL2_ALT_SOURCE_BOUNDARY
#define PCL2_ALT_SOURCE_BOUNDARY

#include <pcl/image.h>

namespace pcl
{
	namespace filter2
	{

		template <class ImgType, class SecondaryBoundary>
		class AltSourceBoundary
		{
		public:
			typedef ImgType ImageType;
		
			AltSourceBoundary() 
			{}
			AltSourceBoundary(const typename ImageType::ConstantPointer& img, const SecondaryBoundary& bound) 
			{
				m_Image = img;
				m_Bound = bound;
			}
			AltSourceBoundary(const AltSourceBoundary& obj)
			{
				*this = obj;
			}

			AltSourceBoundary& operator=(const AltSourceBoundary& obj)
			{
				m_Image = obj.m_Image;
				m_Bound = obj.m_Bound;
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Image;
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p) const
			{
				if (m_Image->contain(p)) return m_Image->get(p);
				else return m_Bound->get(p);
			}

			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				if (m_Image->contain(p)) return m_Image->get(index);
				else return m_Bound->get(p, index);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint(), iter);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			SecondaryBoundary m_Bound;
		};


		template <class ImgType, class SecondaryBoundary>
		class PointOnlyAltSourceBoundary
		{
		public:
			typedef ImgType ImageType;
		
			PointOnlyAltSourceBoundary() 
			{}
			PointOnlyAltSourceBoundary(const typename ImageType::ConstantPointer& img, const SecondaryBoundary& bound) 
			{
				m_Image = img;
				m_Bound = bound;
			}
			PointOnlyAltSourceBoundary(const PointOnlyAltSourceBoundary& obj)
			{
				*this = obj;
			}

			PointOnlyAltSourceBoundary& operator=(const PointOnlyAltSourceBoundary& obj)
			{
				m_Image = obj.m_Image;
				m_Bound = obj.m_Bound;
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Image;
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p) const
			{
				if (m_Image->contain(p)) return m_Image->get(p);
				else return m_Bound->get(p);
			}

			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				if (m_Image->contain(p)) return m_Image->get(index);
				else return m_Bound->get(p);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint(), iter);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			SecondaryBoundary m_Bound;
		};

	}
}

#endif