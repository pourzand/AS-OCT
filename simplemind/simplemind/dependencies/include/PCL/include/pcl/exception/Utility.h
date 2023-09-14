#ifndef PCL_EXCEPTION_UTILITY
#define PCL_EXCEPTION_UTILITY

#include <pcl/exception/Exception.h>

namespace pcl
{

	template <class F>
	void TryCatchBlock(std::ostream& os, F obj, bool* exception_occured=NULL, const std::string& err_msg="")
	{
		if (exception_occured) *exception_occured = false;
		try {
			obj();
		} catch (const Exception& e) {
			if (!err_msg.empty()) std::cout << err_msg << std::endl;
			os << e;
			if (exception_occured) *exception_occured = true;
		}
	}

	template <class F>
	auto TryCatchBlockWithReturn(std::ostream& os, F obj, bool* exception_occured=NULL, const std::string& err_msg="") -> decltype(obj())
	{
		if (exception_occured) *exception_occured = false;
		decltype(obj()) return_object;
		try {
			return_object = obj();
		} catch (const Exception& e) {
			if (!err_msg.empty()) std::cout << err_msg << std::endl;
			os << e;
			if (exception_occured) *exception_occured = true;
		}
		return return_object;
	}

}

#endif