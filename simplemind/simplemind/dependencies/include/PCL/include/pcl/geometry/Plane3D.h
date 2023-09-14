#ifndef PCL_PLANE_3D
#define PCL_PLANE_3D

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Line3D.h>

namespace pcl
{
	namespace geometry
	{
		template <class VT> class Line3D;

		template <class VT>
		class Plane3D 
		{
		public:
			typedef Plane3D Self;
			typedef VT ValueType;

			Plane3D(double tol = std::numeric_limits<double>::epsilon())
			{
				m_Tol = tol;
			}

			// ax + by + cz + d = 0
			Plane3D(double a, double b, double c, double d, double tol = std::numeric_limits<double>::epsilon())
			{
				double n = sqrt(a*a + b*b + c*c);
				m_Normal.set(a/n, b/n, c/n);
				m_Origin.set(0, 0, -d/c);
				m_Tol = tol;
			}

			template <class T1, class T2, class T3>
			Plane3D(const pcl::Point3D<T1>& a, const pcl::Point3D<T2>& b, const pcl::Point3D<T3>& c, double tol = std::numeric_limits<double>::epsilon())
			{
				setNormal(Point3D<VT>(b-a).getCrossProduct(Point3D<VT>(c-a)));
				m_Origin = a;
				m_Tol = tol;
			}

			template <class T1, class T2>
			Plane3D(const pcl::Point3D<T1>& origin, const pcl::Point3D<T2>& normal, double tol = std::numeric_limits<double>::epsilon())
			{
				setOrigin(origin);
				setNormal(normal);
				m_Tol = tol;
			}

			void setTolerance(double v)
			{
				m_Tol = v;
			}
			double getTolerance() const
			{
				return m_Tol;
			}

			template <class PT>
			void setOrigin(const PT& p)
			{
				m_Origin.assign(p);
			}

			template <class PT>
			void setNormal(const PT& p)
			{
				m_Normal.assign(p);
				m_Normal.normalize();
			}

			const pcl::Point3D<ValueType>& getOrigin() const
			{
				return m_Origin;
			}

			const pcl::Point3D<ValueType>& getNormal() const
			{
				return m_Normal;
			}

			template <class T>
			Point3D<double> getIntersection(const Line3D<T>& l) const
			{
				return getIntersection<Point3D<double>>(l);
			}

			template <class PT, class T>
			PT getIntersection(const Line3D<T>& l) const
			{
				double t = (m_Normal.getDotProduct(m_Origin) - m_Normal.getDotProduct(l.getOrigin())) / m_Normal.getDotProduct(l.getVector());
				Point3D<double> res(l.getOrigin());
				res += (Point3D<double>(l.getVector())*t);
				return PT(res);
			}

			template <class T>
			Line3D<double> getIntersection(const Plane3D<T>& p) const
			{
				return getIntersection<Line3D<double>>(p);
			}

			template <class LT, class T>
			LT getIntersection(const Plane3D<T>& p) const
			{
				//Based on the implementation documented in http://geomalgorithms.com/a05-_intersect-1.html with some slight modifications
				pcl::Point3D<double> vector = pcl::Point3D<double>(p.getNormal()).getCrossProduct(Point3D<double>(m_Normal));
				int max_elem = 0;
				for (int i=1; i<3; ++i) if (pcl::abs(vector[max_elem])<pcl::abs(vector[i])) max_elem = i;
				pcl::Point3D<double> point;
				double d1 = m_Normal.getDotProduct(m_Origin),
					d2 = p.getNormal().getDotProduct(p.getOrigin()); //This is different from the original algorithm
				switch (max_elem) {
				case 0: // intersect with x=0
					point.x() = 0;
					point.y() = (d2*m_Normal.z() - d1*p.getNormal().z()) /  vector.x();
					point.z() = (d1*p.getNormal().y() - d2*m_Normal.y()) /  vector.x();
					break;
				case 1: // intersect with y=0
					point.x() = (d1*p.getNormal().z() - d2*m_Normal.z()) /  vector.y();
					point.y() = 0;
					point.z() = (d2*m_Normal.x() - d1*p.getNormal().x()) /  vector.y();
					break;
				case 2: // intersect with z=0
					point.x() = (d2*m_Normal.y() - d1*p.getNormal().y()) /  vector.z();
					point.y() = (d1*p.getNormal().x() - d2*m_Normal.x()) /  vector.z();
					point.z() = 0;
				}
				return LT(point, vector, m_Tol);
			}

			template <class T>
			bool intersect(const Line3D<T>& l) const
			{
				return !pcl::approxZero(l.getVector().getDotProduct(m_Normal), m_Tol);
			}

			template <class T>
			bool intersect(const Plane3D<T>& p) const
			{
				return !pcl::approxZero(p.m_Normal.getCrossProduct(m_Normal).getNorm(), m_Tol);
			}
			
			template <class T>
			bool contain(const pcl::Point3D<T>& p) const
			{
				return sign(p)==0;
			}

			template <class T>
			bool coincide(const Plane3D<T>& p) const
			{
				return (contain(p.getOrigin()) && pcl::approxZero(m_Normal.getCrossProduct(p.getNormal()).getNorm(), m_Tol));
			}
			
			template <class T>
			char sign(const pcl::Point3D<T>& p) const
			{
				double val = (p-m_Origin).getDotProduct(m_Normal);
				if (pcl::approxZero(val, m_Tol)) return 0;
				else if (val>0) return 1;
				else return -1;
			}

			template <class T>
			double orthogonalDistanceFrom(const pcl::Point3D<T>& p) const
			{
				return (p - m_Origin).getDotProduct(m_Normal);
			}

			inline std::ostream& print(std::ostream& os) const
			{
				os << m_Normal << ".( x - " << m_Origin << " ) = 0";
				return os;
			}

			friend std::ostream& operator<<(std::ostream& os, const Self& obj)
			{
				return obj.print(os);
			}

			template <class T>
			Plane3D operator=(const Plane3D<T>& obj) {
				m_Origin = obj.m_Origin;
				m_Normal = obj.m_Normal;
				m_Tol = obj.m_Tol;
				return *this;
			}

		protected:
			Point3D<ValueType> m_Origin, m_Normal;
			double m_Tol;
		};

	}
}

#endif