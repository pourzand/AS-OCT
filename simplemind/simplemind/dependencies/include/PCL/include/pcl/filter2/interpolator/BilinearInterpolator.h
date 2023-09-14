#ifndef PCL2_BILINEAR_INTERPOLATOR
#define PCL2_BILINEAR_INTERPOLATOR

#include <pcl/image.h>
#include <pcl/math.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace filter2
	{

		template <class BdType>
		class BilinearInterpolator
		{
		public:
			typedef BdType BoundaryType;
			typedef typename BoundaryType::ImageType ImageType;

			BilinearInterpolator() {}
			BilinearInterpolator(const BoundaryType& bound, int axis1, int axis2)
			{
				m_Axis1 = axis1;
				m_Axis2 = axis2;
				setBoundary(bound);
			}
			BilinearInterpolator(const BilinearInterpolator& obj) 
			{
				*this = obj;
			}
			
			BilinearInterpolator& operator=(const BilinearInterpolator& obj)
			{
				m_SafeRegion = obj.m_SafeRegion;
				m_Boundary = obj.m_Boundary;
				for (int i=0; i<3; ++i) m_IndexOffset[i] = obj.m_IndexOffset[i];
				m_Axis1 = obj.m_Axis1;
				m_Axis2 = obj.m_Axis2;
				m_SkipAxis = obj.m_SkipAxis;
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
				//Based on http://en.wikipedia.org/wiki/Bilinear_interpolation
				Point3D<int> p0, p1;
				p0[m_SkipAxis] = pcl_Round(p[m_SkipAxis]);
				p0[m_Axis1] = floor(p[m_Axis1]);
				p0[m_Axis2] = floor(p[m_Axis2]);
				p1[m_SkipAxis] = pcl_Round(p[m_SkipAxis]);
				p1[m_Axis1] = p0[m_Axis1]+1;
				p1[m_Axis2] = p0[m_Axis2]+1;
				
				long index = m_Boundary.getImage()->localToIndex(p0);
				double f00, f10, f01, f11;
				if (m_SafeRegion.contain(p0)) {
					f00 = m_Boundary.getImage()->get(index);
					f10 = m_Boundary.getImage()->get(index+m_IndexOffset[m_Axis1]);
					f01 = m_Boundary.getImage()->get(index+m_IndexOffset[m_Axis2]);
					f11 = m_Boundary.getImage()->get(index+m_IndexOffset[m_Axis1]+m_IndexOffset[m_Axis2]);
				} else {
					Point3D<int> t;
					t = p0;
					f00 = m_Boundary.get(t, index);
					t[m_Axis1] += 1;
					f10 = m_Boundary.get(t, index+m_IndexOffset[m_Axis1]);
					t[m_Axis2] += 1;
					f11 = m_Boundary.get(t, index+m_IndexOffset[m_Axis1]+m_IndexOffset[m_Axis2]);
					t = p0;
					t[m_Axis2] += 1;
					f01 = m_Boundary.get(t, index+m_IndexOffset[m_Axis2]);
				}
				double x = p[m_Axis1] - p0[m_Axis1],
					y = p[m_Axis2] - p0[m_Axis2];
				return f00 + (f10-f00)*x + (f01-f00)*y  + (f00-f10-f01+f11)*x*y;
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
			int m_Axis1, m_Axis2, m_SkipAxis;
			
			void setBoundary(const BoundaryType& bound)
			{
				m_Boundary = bound;
				m_SafeRegion = m_Boundary.getImage()->getRegion();
				m_SafeRegion.getMaxPoint()[m_Axis1] -= 1;
				m_SafeRegion.getMaxPoint()[m_Axis2] -= 1;
				m_SafeRegion.getMaxPoint().max(m_SafeRegion.getMinPoint());
				for (int i=0; i<3; i++) {
					m_IndexOffset[i] = m_Boundary.getImage()->getOffsetTable()[i];
					if (i!=m_Axis1 && i!=m_Axis2) m_SkipAxis = i;
				}
			}
		};

	}
}

#endif