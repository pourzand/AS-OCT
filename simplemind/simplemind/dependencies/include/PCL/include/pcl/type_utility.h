#ifndef PCL_TYPE_UTILITY
#define PCL_TYPE_UTILITY

#include <boost/type_traits.hpp>

#define pcl_CreateMethodTester(name, func) \
template <class T, class Def>	\
class name	\
{	\
	typedef char yes[1];	\
	typedef char no[2]; \
	template <class U, U> struct type_check;	\
	template <class U> static yes &check(	\
		type_check<Def, &U::func>*	\
		);	\
	template <class U> static no &check(...);	\
public: \
	static const bool value = sizeof(check<T>(0))==sizeof(yes);	\
}

#define pcl_CreateMemberTester(name, var) \
template <class T, class Type>	\
class name	\
{	\
	typedef char yes[1];	\
	typedef char no[2]; \
	template <class U, U> struct type_check;	\
	template <class U> static yes &check(	\
		type_check<Type(T::*), &U::var>*	\
		);	\
	template <class U> static no &check(...);	\
public: \
	static const bool value = sizeof(check<T>(0))==sizeof(yes);	\
}

namespace pcl
{

	template <class T>
	struct base_type 
	{
		typedef typename boost::remove_cv<
			typename boost::remove_reference<
			typename boost::remove_pointer<
			typename boost::remove_all_extents<T>::type 
			>::type
			>::type
		>::type type;
	};

	template <class Pointer, class Enable=void>
	struct ptr_base_type
	{
		typedef typename boost::remove_cv<
			typename boost::remove_reference<
			typename Pointer::element_type
			>::type
		>::type type;
	};
	template <class Pointer>
	struct ptr_base_type<Pointer, typename boost::enable_if<boost::is_pointer<Pointer>>::type>
	{
		typedef typename base_type<Pointer>::type type;
	};

	/**
		Meant for pulling class type from decltype
	**/
	template <class T>
	struct identity 
	{
		typedef T type;
	};
}

/*
#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include <utility>

std::string get_demangled_name(const std::string& name)
{
	char *realname;
	int status;
	std::string result;
	realname = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
	result = realname;
	free(realname);
	return std::move(result);
}*/

#endif