#ifndef PCL_LINE_3D
#define PCL_LINE_3D

#include <pcl/geometry/Point.h>
//#include <pcl/geometry/Plane3D.h> //comment out  due to circular dependency.

namespace pcl
{
	namespace geometry
	{
		template <class VT> 
		class Plane3D;

		template <class VT>
		class Line3D 
		{
		public:
			typedef Line3D Self;
			typedef VT ValueType;

			Line3D(double tol=std::numeric_limits<double>::epsilon())
			{
				m_Tol = tol;
			}

			template <class T1, class T2>
			Line3D(const Point3D<T1> origin, const Point3D<T2> vector, double tol=std::numeric_limits<double>::epsilon())
			{
				setOrigin(origin);
				setVector(vector);
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
			void setVector(const PT& p)
			{
				m_Vector.assign(p);
				m_Vector.normalize();
			}

			const Point3D<ValueType>& getOrigin() const
			{
				return m_Origin;
			}

			const Point3D<ValueType>& getVector() const
			{
				return m_Vector;
			}

			template <class T>
			Point3D<double> getIntersection(const Line3D<T>& l) const
			{
				return getIntersection<Point3D<double>>(l);
			}

			template <class PT, class T>
			PT getIntersection(const Line3D<T>& l) const
			{
				Point3D<double> a(m_Vector), 
					b(l.getVector()), 
					c(l.getOrigin()-m_Origin);
				Point3D<double> a_cross_b = a.getCrossProduct(b);
				double s = (c.getCrossProduct(b).getDotProduct(a_cross_b))/pcl::square(a_cross_b.getNorm());
				Point3D<double> res;
				res.assign(m_Origin);
				res += (a*s);
				return PT(res);
			}

			template <class T>
			Point3D<double> getIntersection(const Plane3D<T>& p) const
			{
				return p.getIntersection(*this);
			}

			template <class PT, class T>
			PT getIntersection(const Plane3D<T>& p) const
			{
				return p.template getIntersection<PT>(*this);
			}

			template <class T>
			bool isParallel(const Line3D<T>& l) const
			{
				return pcl::approxZero(m_Vector.getCrossProduct(l.getVector()).getNorm(), m_Tol);
			}

			template <class T>
			bool isPerpendicular(const Line3D<T>& l) const
			{
				return pcl::approxZero(m_Vector.getDotProduct(l.getVector()), m_Tol);
			}

			template <class T>
			bool intersect(const Plane3D<T>& p) const
			{
				return !pcl::approxZero(m_Vector.getDotProduct(p.getNormal()), m_Tol);
			}

			template <class T>
			bool intersect(const Line3D<T>& l) const
			{
				return !pcl::approxZero(m_Vector.getCrossProduct(l.getVector()).getNorm(), m_Tol);
			}
			
			template <class T>
			bool contain(const pcl::Point3D<T>& p) const
			{
				return pcl::approxZero(m_Vector.getCrossProduct(m_Origin-p).getNorm(), m_Tol);
			}

			inline std::ostream& print(std::ostream& os) const
			{
				os << m_Origin << " + t" << m_Vector;
				return os;
			}

			friend std::ostream& operator<<(std::ostream& os, const Self& obj)
			{
				return obj.print(os);
			}

			template <class T>
			Line3D operator=(const Line3D<T>& obj) {
				m_Origin = obj.m_Origin;
				m_Vector = obj.m_Vector;
				m_Tol = obj.m_Tol;
				return *this;
			}

		protected:
			Point3D<ValueType> m_Origin, m_Vector;
			double m_Tol;
		};

	}
}

#endif