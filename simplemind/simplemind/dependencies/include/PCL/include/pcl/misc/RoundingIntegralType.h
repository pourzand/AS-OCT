#ifndef PCL_ROUNDING_INTEGRAL_TYPE
#define PCL_ROUNDING_INTEGRAL_TYPE

#include <pcl/macro.h>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace pcl
{

	template <class Type>
	class RoundingIntegralType
	{
	public:
		typedef RoundingIntegralType Self;
		typedef Type BaseType;

		RoundingIntegralType() {}
		RoundingIntegralType(const Type& obj):m_Value(obj) {}

		template <class T>
		RoundingIntegralType(const T& obj)
		{
			operator=(obj);
		}

		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>,Self&>::type operator=(const T& obj)
		{
			m_Value = obj;
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>,Self&>::type operator=(const T& obj)
		{
			m_Value = pcl_Round(obj);
			return *this;
		}

		/*template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>,Self&>::type operator+=(const float& obj)
		{
			m_Value += obj;
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>,Self&>::type operator+=(const double& obj)
		{
			m_Value += pcl_Round(obj);
			return *this;
		}
		
		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>,Self&>::type operator-=(const float& obj)
		{
			m_Value -= obj;
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>,Self&>::type operator-=(const double& obj)
		{
			m_Value -= pcl_Round(obj);
			return *this;
		}
		
		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>,Self&>::type operator/=(const float& obj)
		{
			m_Value /= obj;
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>,Self&>::type operator/=(const double& obj)
		{
			m_Value /= pcl_Round(obj);
			return *this;
		}
		
		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>,Self&>::type operator*=(const float& obj)
		{
			m_Value *= obj;
			return *this;
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>,Self&>::type operator*=(const double& obj)
		{
			m_Value *= pcl_Round(obj);
			return *this;
		}*/

		inline operator const Type&() const
		{
			return m_Value;
		}

		inline operator Type&()
		{
			return m_Value;
		}
	private:
		Type m_Value;
	};

}

#endif