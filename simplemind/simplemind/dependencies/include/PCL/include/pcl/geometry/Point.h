#ifndef PCL_POINT_TYPE
#define PCL_POINT_TYPE

#include <pcl/macro.h>
#include <boost/utility.hpp>
#include <boost/type_traits.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include <string>
#include <sstream>
#include <ostream>
#include <math.h>

#pragma warning ( push )
#pragma warning ( disable : 4244 )

namespace pcl
{
	struct PointBase {};

	template <class Type, int Dim>
	class Point: public PointBase
	{
	public:
		typedef Point<Type,Dim> Self;
		typedef Type ValueType;
		static const int Dimension = Dim;

		Point() {}
		Point(const Point& obj) 
		{
			copy(obj);
		}
		template <class PT>
        Point(const PT& obj, typename boost::enable_if<boost::is_base_of<PointBase,PT>>::type* dummy=0)
		{
            pcl_UnreferencedParameter(dummy);
			*this = obj;
		}
		template <class T>
		Point(const T* arr, int num=Dimension, ValueType trailing_val=0) 
		{
			assign(arr, num, 0);
		}

		inline int getDimension() const
		{ 
			return Dimension; 
		}

		inline ValueType& operator[](int i) 
		{ 
			return m_Data[i]; 
		}
		inline const ValueType& operator[](int i) const 
		{ 
			return m_Data[i]; 
		}
		
		inline bool isFinite()
		{
			for (int i=0; i<Dimension; i++) if (!boost::math::isfinite(m_Data[i])) return false;
			return true;
		}

		operator const Type*() const
		{
			return m_Data;
		}

		operator Type*()
		{
			return m_Data;
		}

		/************************ Math related ************************/

		inline Self& negate()
		{
			for (int i=0; i<Dimension; i++) m_Data[i] = -m_Data[i];
			return *this;
		}

		template <class T>
		inline Self& operator+=(const T* a)
		{
			for (int i=0; i<Dimension; i++) m_Data[i] += a[i];
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_base_of<PointBase,T>, Self&>::type operator+=(const T& a) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] += a[i];
			return *this;
		}
		template <class T>
		inline typename boost::disable_if<boost::is_base_of<PointBase,T>, Self&>::type operator+=(const T& val) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] += val;
			return *this;
		}

		template <class T>
		inline Self& operator-=(const T* a)
		{
			for (int i=0; i<Dimension; i++) m_Data[i] -= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_base_of<PointBase,T>, Self&>::type operator-=(const T& a) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] -= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::disable_if<boost::is_base_of<PointBase,T>, Self&>::type operator-=(const T& val) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] -= val;
			return *this;
		}

		template <class T>
		inline Self& operator*=(const T* a)
		{
			for (int i=0; i<Dimension; i++) m_Data[i] *= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_base_of<PointBase,T>, Self&>::type operator*=(const T& a) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] *= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::disable_if<boost::is_base_of<PointBase,T>, Self&>::type operator*=(const T& val) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] *= val;
			return *this;
		}

		template <class T>
		inline Self& operator/=(const T* a)
		{
			for (int i=0; i<Dimension; i++) m_Data[i] /= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_base_of<PointBase,T>, Self&>::type operator/=(const T& a) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] /= a[i];
			return *this;
		}
		template <class T>
		inline typename boost::disable_if<boost::is_base_of<PointBase,T>, Self&>::type operator/=(const T& val) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] /= val;
			return *this;
		}

		template <class T>
		inline Self& operator+(const T* a) const
		{
			return Self(*this)+=a;
		}
		template <class T>
		inline Self operator+(const T& val) const 
		{ 
			return Self(*this)+=val; 
		}

		template <class T>
		inline Self& operator-(const T* a) const
		{
			return Self(*this)-=a;
		}
		template <class T>
		inline Self operator-(const T& val) const 
		{ 
			return Self(*this)-=val; 
		}

		template <class T>
		inline Self& operator*(const T* a) const
		{
			return Self(*this)*=a;
		}
		template <class T>
		inline Self operator*(const T& val) const 
		{ 
			return Self(*this)*=val; 
		}

		template <class T>
		inline Self& operator/(const T* a) const
		{
			return Self(*this)/=a;
		}
		template <class T>
		inline Self operator/(const T& val) const 
		{ 
			return Self(*this)/=val; 
		}

		inline Self operator-() const 
		{
			return Self(*this).negate(); 
		}

		inline Self& max(const Self& p) 
		{
			for (int i=0; i<Dimension; i++) {
				if (p[i]>m_Data[i]) m_Data[i] = p[i];
			}
			return *this;
		}
		inline Self& min(const Self& p) 
		{
			for (int i=0; i<Dimension; i++) {
				if (p[i]<m_Data[i]) m_Data[i] = p[i];
			}
			return *this;
		}

		inline double getNormSqr() const 
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) result += double(m_Data[i])*double(m_Data[i]);
			return result;
		}
		inline double getNorm() const 
		{
			return std::sqrt(this->getNormSqr()); 
		}

		inline Self& normalize() 
		{
			return *this /= getNorm();
		}
		inline Self getNormalized() const
		{
			return Self(*this).normalize();
		}

		inline double getEuclideanDistanceSqr(const Self& obj) const
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) {
				double temp = static_cast<double>(obj[i]) - static_cast<double>(m_Data[i]);
				result += pcl_Square(temp);
			}
			return result;
		}
		template <class T>
		inline double getEuclideanDistanceSqr(const Point<T,Dimension>& obj) const
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) {
				double temp = static_cast<double>(obj[i]) - static_cast<double>(m_Data[i]);
				result += pcl_Square(temp);
			}
			return result;
		}

		inline double getEuclideanDistance(const Self& obj) const
		{
			return std::sqrt(this->getEuclideanDistanceSqr(obj));
		}
		template <class T>
		inline double getEuclideanDistance(const Point<T,Dimension>& obj) const
		{
			return std::sqrt(this->getEuclideanDistanceSqr<T>(obj));
		}
		
		template <class PT>
		inline double getEuclideanDistanceSqr(const Self& obj, const PT& spacing) const
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) {
				double temp = (static_cast<double>(obj[i]) - static_cast<double>(m_Data[i]))*spacing[i];
				result += pcl_Square(temp);
			}
			return result;
		}
		template <class PT>
		inline double getEuclideanDistance(const Self& obj, const PT& spacing) const
		{
			return std::sqrt(this->getEuclideanDistanceSqr(obj, spacing));
		}

		inline double getDotProduct(const Self& obj) const
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) {
				result += double(obj[i]) * double(m_Data[i]);
			}
			return result;
		}
		template <class T>
		inline double getDotProduct(const Point<T,Dimension>& obj) const
		{
			double result = 0;
			for (int i=0; i<Dimension; i++) {
				result += double(obj[i]) * double(m_Data[i]);
			}
			return result;
		}

		/************************ Data assignment related ************************/
		template <class T>
		inline typename boost::enable_if_c<boost::is_base_of<PointBase,T>::value && T::Dimension!=Dimension, Self&>::type operator=(const T& val)
		{
			assign(val);
			return *this;
		}
		template <class T>
		inline typename boost::enable_if_c<boost::is_base_of<PointBase,T>::value && T::Dimension==Dimension, Self&>::type operator=(const T& val)
		{
			copy(val);
			return *this;
		}
		//Note assignment to a primitive variable is purposely removed to allow function to differentiate whether a point or a variable is provided!
		/*template <class T>
		inline typename boost::disable_if<boost::is_base_of<PointBase,T>, Self&>::type operator=(const T& val)
		{
			set(val);
			return *this;
		}*/
		template <class T>
		inline Self& operator=(const T* arr) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] = arr[i];
			return *this;
		}
		
		template <class T>
		inline Self& set(const T& val)
		{
			for (int i=0; i<Dimension; i++) m_Data[i] = val;
			return *this;
		}

		template <class PT>
		inline Self& assign(const PT& p, ValueType trailing_val=0)
		{
			return assign(&p[0], p.getDimension(), trailing_val);
		}
		template <class PT>
		inline Self& assignRound(const PT& p, ValueType trailing_val=0)
		{
			return assignRound(&p[0], p.getDimension(), trailing_val);
		}
		template <class PT, class F>
		inline Self& assignCustom(const PT& p, F func, int num=Dimension, ValueType trailing_val=0)
		{
			return assignCustom(&p[0], func, p.getDimension(), trailing_val);
		}

		template <class T>
		inline Self& assign(const T* p, int num=Dimension, ValueType trailing_val=0)
		{
			if (Dimension>num) {
				for (int i=0; i<num; i++) m_Data[i] = p[i];
				for (int i=num; i<Dimension; i++) m_Data[i] = trailing_val;
			} else {
				for (int i=0; i<Dimension; i++) m_Data[i] = p[i];
			}
			return *this;
		}
		template <class T>
		inline Self& assignRound(const T* p, int num=Dimension, ValueType trailing_val=0)
		{
			if (Dimension>num) {
				for (int i=0; i<num; i++) m_Data[i] = pcl_Round(p[i]);
				for (int i=num; i<Dimension; i++) m_Data[i] = trailing_val;
			} else {
				for (int i=0; i<Dimension; i++) m_Data[i] = pcl_Round(p[i]);
			}
			return *this;
		}
		template <class T, class F>
		inline Self& assignCustom(const T* p, F func, int num=Dimension, ValueType trailing_val=0)
		{
			if (Dimension>num) {
				for (int i=0; i<num; i++) m_Data[i] = func(p[i]);
				for (int i=num; i<Dimension; i++) m_Data[i] = trailing_val;
			} else {
				for (int i=0; i<Dimension; i++) m_Data[i] = func(p[i]);
			}
			return *this;
		}

		void swap(Point& p) {
			for (int i=0; i<Dimension; i++) {
				pcl_Swap(this->m_Data[i], p[i]);
			}
		}

		/************************ Comparison related ************************/

		template <class PT>
		inline bool operator==(const PT& obj) const 
		{
			for (int i=0; i<Dimension; i++) {
				if (m_Data[i]!=obj[i]) return false;
			}
			return true;
		}

		template <class PT>
		inline bool operator!=(const PT& obj) const 
		{
			return !(*this == obj);
		}

		template <class PT>
		inline bool epsilonEqual(const PT& obj, double epsilon=pcl::Epsilon) const 
		{
			for (int i=0; i<Dimension; i++) {
				double val = static_cast<double>(m_Data[i])-static_cast<double>(obj[i]);
				val = pcl_Abs(val);
				if (val>epsilon) {
					return false;
				}
			}
			return true;
		}

		/************************ Conversion related ************************/
		
		inline Type* getPointer()
		{
			return m_Data;
		}
		inline const Type* getPointer() const
		{
			return m_Data;
		}

		inline std::string toString() const 
		{
			std::stringstream buffer;
			print(buffer);
			return std::move(buffer.str());
		}

		inline operator std::string () const
		{
			return toString();
		}

		inline std::ostream& print(std::ostream& os) const
		{
			os << "(";
			if (sizeof(Type)==1) {
				os << (int)m_Data[0];
				for (int i=1; i<Dimension; i++) {
					os << "," << (int)m_Data[i];
				}
			} else {
				os << m_Data[0];
				for (int i=1; i<Dimension; i++) {
					os << "," << m_Data[i];
				}
			}
			os << ")";
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const Self& obj)
		{
			return obj.print(os);
		}

	protected:
		ValueType m_Data[Dimension];

		template <class PT>
		inline void copy(const PT& obj) 
		{
			for (int i=0; i<Dimension; i++) m_Data[i] = obj[i];
		}
	};

	/*************************************************************************************/

	template <class Type>
	class Point3D: public Point<Type,3> 
	{
	public:
		typedef Point3D<Type> Self;
		typedef Point<Type,3> Parent;
		typedef Type ValueType;

		Point3D() {}
		Point3D(ValueType a, ValueType b, ValueType c)
		{
			x() = a; y() = b; z() = c;
		}
		Point3D(const Self& obj):Parent(obj) {}
		template <class PT>
		Point3D(const PT& obj, typename boost::enable_if<boost::is_base_of<PointBase,PT>>::type* dummy=0)
		{
			*this = obj;
		}
		template <class T>
		Point3D(const T* arr, int num=Self::Dimension, ValueType trailing_val=0) 
		{
            pcl_UnreferencedParameter(trailing_val);
			assign(arr, num, 0);
		}
		Point3D(const Parent& obj):Parent(obj) {}

		Point3D<double> getPolar() const 
		{
			//According to http://electron9.phys.utk.edu/vectors/3dcoordinates.htm
			double r, q, f;
			r = sqrt(pcl_Square((double)x())+pcl_Square((double)y())+pcl_Square((double)x()));
			q = atan((double)z()/sqrt(pcl_Square((double)x())+pcl_Square((double)y())));
			f = atan((double)y()/(double)x());
			return Point3D<double>(r,q,f);
		}

		inline Self& set(ValueType xval, ValueType yval, ValueType zval) 
		{
			x() = xval;
			y() = yval;
			z() = zval;
			return *this;
		}

		inline Self& setCrossProduct(const Self& b) 
		{ 
			Type rx = y()*b.z() - z()*b.y(),
				ry = z()*b.x() - x()*b.z(),
				rz = x()*b.y() - y()*b.x();
			x() = rx;
			y() = ry;
			z() = rz;
			return *this;
		}

		Self getPerpendicularVector() const
		{
			Self temp(*this);
			if (temp.x()==0) temp.x() += 1;
			else if (temp.y()==0) temp.y() += 1;
			else if (temp.z()==0) temp.z() += 1;
			else temp.x() += 1;
			return Self(*this).setCrossProduct(temp).normalize();
		}

		inline Self getCrossProduct(const Self& b) const
		{
			return Self(*this).setCrossProduct(b);
		}

		inline ValueType& x() 
		{
			return this->m_Data[0];
		}
		inline ValueType x() const
		{
			return this->m_Data[0];
		}

		inline ValueType& y()
		{
			return this->m_Data[1];
		}
		inline ValueType y() const
		{
			return this->m_Data[1];
		}

		inline ValueType& z()
		{
			return this->m_Data[2];
		}
		inline ValueType z() const
		{
			return this->m_Data[2];
		}

#include <pcl/geometry/PointRecast.txx>
	};

	/*************************************************************************************/

	template <class Type>
	class Point2D: public Point<Type,2> 
	{
	public:
		typedef Point2D<Type> Self;
		typedef Point<Type,2> Parent;
		typedef Type ValueType;

		Point2D() {}
		Point2D(const ValueType a, const ValueType b)
		{
			x() = a; y() = b;
		}
		Point2D(const Self& obj):Parent(obj) {}
		template <class PT>
		Point2D(const PT& obj, typename boost::enable_if<boost::is_base_of<PointBase,PT>>::type* dummy=0)
		{
			*this = obj;
		}
		template <class T>
		Point2D(const T* arr, int num=Self::Dimension, ValueType trailing_val=0) 
		{
			assign(arr, num, 0);
		}
		Point2D(const Parent& obj):Parent(obj) {}

		void set(ValueType xval, ValueType yval) 
		{
			x() = xval;
			y() = yval;
		}

		inline ValueType& x() 
		{
			return this->m_Data[0];
		}
		inline ValueType x() const
		{
			return this->m_Data[0];
		}

		inline ValueType& y()
		{
			return this->m_Data[1];
		}
		inline ValueType y() const
		{
			return this->m_Data[1];
		}

#include <pcl/geometry/PointRecast.txx>

		operator Point3D<Type>() const
		{
			return Point3D<Type>(x,y);
		}

		Point3D<Type> toPoint3D(int skip_axis=2) const
		{
			Point3D<Type> result;
			int count = 0;
			for (int i=0; i<3; i++) {
				if (i==skip_axis) result[i] = 0;
				else {
					result[i] = (*this)[count];
					count++;
				}
			}
			return result;
		}
	};

}

#pragma warning ( pop )
#endif
