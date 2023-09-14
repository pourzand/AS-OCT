#ifndef PCL2_TRILINEAR_INTERPOLATOR
#define PCL2_TRILINEAR_INTERPOLATOR

#include <pcl/image.h>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace pcl
{
	namespace filter2
	{

		template <class BdType>
		class TrilinearInterpolator
		{
		public:
			typedef BdType BoundaryType;
			typedef typename BoundaryType::ImageType ImageType;

			TrilinearInterpolator() {}
			TrilinearInterpolator(const BoundaryType& bound)
			{
				setBoundary(bound);
			}
			TrilinearInterpolator(const TrilinearInterpolator& obj) 
			{
				*this = obj;
			}
			
			TrilinearInterpolator& operator=(const TrilinearInterpolator& obj)
			{
				m_Boundary = obj.m_Boundary;
				m_SafeRegion = obj.m_SafeRegion;
				for (int i=0; i<3; ++i) m_IndexOffset[i] = obj.m_IndexOffset[i];
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
			typename boost::enable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> p0(floor(p.x()), floor(p.y()), floor(p.z())),
					p1(p0.x()+1, p0.y()+1, p0.z()+1);
				long index = m_Boundary.getImage()->localToIndex(p0);
				double xd = p.x()-p0.x(),
					yd = p.y()-p0.y(),
					zd = p.z()-p0.z();
				double i1, i2, j1, j2;
				if (m_SafeRegion.contain(p0)) {
					i1 = m_Boundary.getImage()->get(index)*(1-zd) + m_Boundary.getImage()->get(index+m_IndexOffset[2])*zd;
					i2 = m_Boundary.getImage()->get(index+m_IndexOffset[1])*(1-zd) + m_Boundary.getImage()->get(index+m_IndexOffset[1]+m_IndexOffset[2])*zd;
					j1 = m_Boundary.getImage()->get(index+m_IndexOffset[0])*(1-zd) + m_Boundary.getImage()->get(index+m_IndexOffset[0]+m_IndexOffset[2])*zd;
					j2 = m_Boundary.getImage()->get(index+m_IndexOffset[0]+m_IndexOffset[1])*(1-zd) + m_Boundary.getImage()->get(index+m_IndexOffset[0]+m_IndexOffset[1]+m_IndexOffset[2])*zd;
				} else {
					Point3D<int> t1(p0), t2(p0);
					t2.z() += 1;
					i1 = m_Boundary.get(t1,index)*(1-zd) + m_Boundary.get(t2,index+m_IndexOffset[2])*zd;
					t1.y() += 1;
					t2.y() += 1;
					i2 = m_Boundary.get(t1,index+m_IndexOffset[1])*(1-zd) + m_Boundary.get(t2,index+m_IndexOffset[1]+m_IndexOffset[2])*zd;
					t1 = p0; t2 = p0;
					t1.x() += 1; 
					t2.x() += 1; t2.z() += 1;
					j1 = m_Boundary.get(t1,index+m_IndexOffset[0])*(1-zd) + m_Boundary.get(t2,index+m_IndexOffset[0]+m_IndexOffset[2])*zd;
					t1.y() += 1;
					t2.y() += 1;
					j2 = m_Boundary.get(t1,index+m_IndexOffset[0]+m_IndexOffset[1])*(1-zd) + m_Boundary.get(t2, index+m_IndexOffset[0]+m_IndexOffset[1]+m_IndexOffset[2])*zd;
				}
				double w1 = i1*(1-yd) + i2*yd,
					w2 = j1*(1-yd) + j2*yd;
				return w1*(1-xd) + w2*xd;
			}
			template <class PointType>
			typename boost::disable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> temp; temp.assign(p);
				return m_Boundary.get(p);
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
			Region3D<int> m_SafeRegion;
			long m_IndexOffset[3];
			
			void setBoundary(const BoundaryType& bound)
			{
				m_Boundary = bound;
				m_SafeRegion = m_Boundary.getImage()->getRegion();
				m_SafeRegion.getMaxPoint() -= 1;
				if (m_SafeRegion.empty()) m_SafeRegion.reset();
				for (int i=0; i<3; i++) {
					m_IndexOffset[i] = m_Boundary.getImage()->getOffsetTable()[i];
				}
			}
		};

	}
}

#endif