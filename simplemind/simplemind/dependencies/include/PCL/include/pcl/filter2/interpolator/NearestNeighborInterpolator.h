#ifndef PCL2_NEAREST_NEIGHBOR_INTERPOLATOR
#define PCL2_NEAREST_NEIGHBOR_INTERPOLATOR

#include <pcl/image.h>
#include <pcl/macro.h>

namespace pcl
{
	namespace filter2
	{

		template <class BdType>
		class NearestNeighborInterpolator
		{
		public:
			typedef BdType BoundaryType;
			typedef typename BoundaryType::ImageType ImageType;

			NearestNeighborInterpolator() {}
			NearestNeighborInterpolator(const BoundaryType& bound)
			{
				m_Boundary = bound;
			}
			NearestNeighborInterpolator(const NearestNeighborInterpolator& obj) 
			{
				*this = obj;
			}
			
			NearestNeighborInterpolator& operator=(const NearestNeighborInterpolator& obj)
			{
				m_Boundary = obj.m_Boundary;
				return *this;
			}
			
			typename ImageType::ConstantPointer getImage() const
			{
				return m_Boundary.getImage();
			}
			
			const BoundaryType& getBoundary() const
			{
				return m_Boundary;
			}

			template <class PointType>
			double get(const PointType& p) const
			{
				Point3D<int> temp(pcl_Round(p[0]), pcl_Round(p[1]), pcl_Round(p[2]));
				return m_Boundary.get(temp);
			}
			
			inline typename ImageType::IoValueType get(const Point3D<int>& p, long index) const
			{
				return m_Boundary.get(p, index);
			}
			
			template <class Iterator>
			inline typename boost::enable_if<getPoint_exists<Iterator>, typename ImageType::IoValueType>::type get(const Iterator& iter) const
			{
				return m_Boundary.get(iter);
			}

		protected:
			BoundaryType m_Boundary;
		};
	}
}

#endif