#ifndef PCL_MATH
#define PCL_MATH

#include <pcl/macro.h>
#include <vector>
#include <utility>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <math.h>

namespace pcl
{

	template <class T>
	inline typename boost::disable_if<boost::is_unsigned<T>, T>::type abs(const T& x)
	{
		return pcl_Abs(x);
	}

	template <class T>
	inline typename boost::enable_if<boost::is_unsigned<T>, T>::type abs(const T& x)
	{
		return x;
	}
	
	template <class T>
	inline int round(const T& x)
	{
		return pcl_Round(x);
	}
	
	template <class T1, class T2>
	inline bool doubleEqual(const T1& x, const T2& v)
	{
		return pcl_DoubleEqual(x,v);
	}
	
	template <class T1, class T2, class E>
	inline bool doubleEqual(const T1& x, const T2& v, const E& epsilon)
	{
		return ( (v-epsilon)<x ) && ( x<(v+epsilon) );
	}
	
	template <class T>
	inline char sign(const T& x)
	{
		return pcl_Sign(x);
	}
	
	template <class T>
	inline T square(const T& x) 
	{
		return pcl_Square(x);
	}
	
	template <class T>
	inline T cube(const T& x)
	{
		return pcl_Cube(x);
	}
	
	template <class T>
	typename boost::enable_if<boost::is_floating_point<T>, bool>::type approxZero(const T& val)
	{
		return !(val<-std::numeric_limits<T>::epsilon() || val>std::numeric_limits<T>::epsilon());
	}
	
	template <class T>
	typename boost::disable_if<boost::is_floating_point<T>, bool>::type approxZero(const T& val)
	{
		return val==0;
	}

	template <class T>
	bool approxZero(const T& val, double tol)
	{
		return !(val<-tol || val>tol);
	}
	
	template <class Type>
	std::vector<Type> getExponentialDistributedValues(int num, Type min, Type max) {
		std::vector<Type> result;
		result.reserve(num);
		Type max_val = log(max),
			min_val = log(min),
			val = max_val-min_val;
		for (int i=0; i<num; i++) {
			Type cur_val = Type(i)/Type(num-1) * val + min_val;
			result.push_back(exp(cur_val));
		}
		return std::move(result);
	}

}

#endif