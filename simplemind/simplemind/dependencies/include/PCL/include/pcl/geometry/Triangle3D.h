#ifndef PCL_TRIANGLE3D
#define PCL_TRIANGLE3D

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Region3D.h>

namespace pcl
{
	namespace geometry
	{

		template <class VT>
		class Triangle3D
		{
		public:
			typedef Triangle3D Self;
			typedef VT ValueType;

			Triangle3D()
			{}

			Triangle3D(const Point3D<ValueType>& p1, const Point3D<ValueType>& p2, const Point3D<ValueType>& p3)
			{
				m_Points[0] = p1;
				m_Points[1] = p2;
				m_Points[2] = p3;
			}

			Point3D<ValueType>& operator[](int i)
			{
				return m_Points[i];
			}

			const Point3D<ValueType>& operator[](int i) const
			{
				return m_Points[i];
			}

			bool isCollinear() const
			{
				Point3D<double> dir1 = m_Points[0]-m_Points[1]; dir1.normalize();
				Point3D<double> dir2 = m_Points[0]-m_Points[2]; dir1.normalize();
				Point3D<double> temp = dir1; temp.setCrossProduct(dir2);
				return temp[0]+temp[1]+temp[2] == 0;//pcl::Epsilon;
			}

			Point3D<double> getNormal() const
			{
				Point3D<double> dir1 = m_Points[0]-m_Points[1]; dir1.normalize();
				Point3D<double> dir2 = m_Points[0]-m_Points[2]; dir1.normalize();
				Point3D<double> normal = dir1; normal.setCrossProduct(dir2);
				normal.normalize();
				return normal;
			}

			Point3D<double> getCentroid() const
			{
				Point3D<double> centroid = m_Points[0];
				centroid += m_Points[1];
				centroid += m_Points[2];
				centroid /= 3;
				return centroid;
			}

			Region3D<ValueType> getRegion() const
			{
				Region3D<ValueType> region;
				region.reset();
				for (int i=0; i<3; ++i) region.add(m_Points[i]);
				return region;
			}

		protected:
			Point3D<ValueType> m_Points[3];
		};



		class Triangle3DContainmentTester
		{
		public:
			Triangle3DContainmentTester()
			{}

			template <class TriangleType>
			Triangle3DContainmentTester(const TriangleType& obj)
			{
				set(obj);
			}

			template <class TriangleType>
			void set(const TriangleType& obj)
			{
				m_Centroid = obj.getCentroid();
				m_Normal = obj.getNormal();
				for (int i=0; i<3; ++i) {
					m_LineOrigin[i] = obj[i];
					Point3D<double> direction;
					if (i!=2) direction = obj[i+1] - obj[i];
					else direction = obj[0] - obj[2];
					m_LineNormal[i] = m_Normal; m_LineNormal[i].setCrossProduct(direction);
					m_LineNormal[i].normalize();
					if (getDistanceFromLine(m_Centroid, i)>0) m_Sign[i] = 1;
					else m_Sign[i] = -1;
				}
			}

			template <class PT>
			bool contain(const PT& p, double tolerance) const
			{
				if (pcl::abs((p-m_Centroid).getDotProduct(m_Normal))>tolerance) return false;
				for (int i=0; i<3; ++i) {
					double dist = getDistanceFromLine(p,i);
					if (pcl::abs(dist)>tolerance && !isSameSign(dist,i)) return false;
				}
				return true;
			}

		protected:
			Point3D<double> m_Centroid;
			Point3D<double> m_Normal;
			Point3D<double> m_LineOrigin[3], m_LineNormal[3];
			char m_Sign[3];

			template <class PT>
			double getDistanceFromLine(const PT& p, int i) const
			{
				return (p-m_LineOrigin[i]).getDotProduct(m_LineNormal[i]);
			}

			inline bool isSameSign(double val, int i) const
			{
				if (val<0 && m_Sign[i]>0) return false;
				if (val>0 && m_Sign[i]<0) return false;
				return true;
			}
		};

	}
}

#endif