#ifndef PCL2_MIRRORING_BOUNDARY
#define PCL2_MIRRORING_BOUNDARY

#include <pcl/image.h>

namespace pcl
{
	namespace filter2
	{

		template <class ImgType>
		class MirroringBoundary
		{
		public:
			typedef ImgType ImageType;
		
			MirroringBoundary() {}
			MirroringBoundary(const typename ImageType::ConstantPointer& img) 
			{
				setImage(img);
			}
			MirroringBoundary(const MirroringBoundary& obj) 
			{
				setImage(obj.m_Image);
			}

			MirroringBoundary& operator=(const MirroringBoundary& obj)
			{
				setImage(obj.m_Image);
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Image;
			}
			
			typename ImageType::IoValueType get(const Point3D<int>& p) const
			{
				Point3D<int> point(p);
				bool is_within = checkPoint(point);
				if (is_within) return m_Image->get(p);
				else return m_Image->get(point);
			}
			
			typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				Point3D<int> point(p);
				bool is_within = checkPoint(point);
				if (is_within) return m_Image->get(index);
				else return m_Image->get(point);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint(), iter);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			Point3D<int> m_MinPoint, m_MaxPoint;
			
			void setImage(const typename ImageType::ConstantPointer& img)
			{
				m_Image = img;
				m_MinPoint = m_Image->getMinPoint();
				m_MaxPoint = m_Image->getMaxPoint();
			}
			
			inline bool checkPoint(Point3D<int>& point) const
			{
				bool is_within = true;
				for (int i=0; i<3; i++) {
					if (point[i]<m_MinPoint[i]) {
						point[i] = m_MaxPoint[i]-(m_MinPoint[i]-point[i]-1);
						is_within = false;
					} else if (point[i]>m_MaxPoint[i]) {
						point[i] = m_MinPoint[i]+(point[i]-m_MaxPoint[i]-1);
						is_within = false;
					}
				}
				return is_within;
			}
			
		};

	}
}

#endif