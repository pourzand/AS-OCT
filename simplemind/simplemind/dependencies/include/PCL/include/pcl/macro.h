#ifndef PCL_MACRO
#define PCL_MACRO

#include <pcl/constant.h>
#include <pcl/exception.h>
#include <algorithm>

#ifdef USE_MAIN_TIMING
#include <pcl/misc/Timing.h>
#include <iostream>
#endif

#define pcl_ForEach(source, item) for (auto item=(source).begin(), m_end_##item=(source).end(); item!=m_end_##item; ++item)
#define pcl_ForEachReverse(source, item) for (auto item=(source).rbegin(), m_end_##item=(source).rend(); item!=m_end_##item; ++item)

#define pcl_Max(x,y) ((x)>(y)?(x):(y))
#define pcl_Min(x,y) ((x)<(y)?(x):(y))

#define pcl_Square(x) ((x)*(x))
#define pcl_Cube(x) ((x)*(x)*(x))
#define pcl_Abs(x) ((x)<0?-(x):(x))
#define pcl_Swap(a, b) std::swap(a,b)
/*{ \
	typename boost::remove_reference<typename boost::remove_cv<decltype(a)>::type>::type _swap_temp_; \
	_swap_temp_ = a; \
	a = b; \
	b = _swap_temp_; \
}*/
#define pcl_Round(x) ( int( (x)>0?(x)+0.5:(x)-0.5 ) )
#define pcl_DoubleEqual(x,v) ( ( ((v)-pcl::Epsilon)<(x) ) && ( (x)<((v)+pcl::Epsilon) ) )
#define pcl_Sign(x) ((x)==0?0:((x)>0?1:-1))

#define pcl_UnreferencedParameter(x) x;

#ifdef NO_MAIN_CATCHALL
/******************************** Without catch all ********************************/
#ifdef USE_MAIN_TIMING
#define pcl_MainStart { \
	int _Main_Return_Status_ = 0; \
	pcl::Timing _Main_Timer_; \
	_Main_Timer_.tic(); \
	try 
#define pcl_MainEnd(error_stream) catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} \
	_Main_Timer_.toc(); \
	std::cout << std::endl << "Completed normally (with status " << _Main_Return_Status_ << ") after " << _Main_Timer_.getClockInSeconds() << " seconds" << std::endl; \
	return _Main_Return_Status_; \
}
#define pcl_Main(main_func, error_stream) int main(int argc, char *argv[]) {\
	pcl::Timing _Main_Timer_; \
	int _Main_Return_Status_ = 0; \
	_Main_Timer_.tic(); \
	try { \
		_Main_Return_Status_ = main_func(argc, argv); \
	} catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} \
	_Main_Timer_.toc(); \
	std::cout << std::endl << "Completed normally (with status " << _Main_Return_Status_ << ") after " << _Main_Timer_.getClockInSeconds() << " seconds" << std::endl; \
	return _Main_Return_Status_; \
}
#else
#define pcl_MainStart { \
	int _Main_Return_Status_ = 0; \
	try
#define pcl_MainEnd(error_stream) catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} \
	return _Main_Return_Status_; \
}
#define pcl_Main(main_func, error_stream) int main(int argc, char *argv[]) {\
	int _Main_Return_Status_ = 0; \
	try { \
		_Main_Return_Status_ = main_func(argc, argv); \
	} catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} \
	return _Main_Return_Status_; \
}
#endif
#else
/******************************** With catch all ********************************/
#ifdef USE_MAIN_TIMING
#define pcl_MainStart { \
	int _Main_Return_Status_ = 0; \
	pcl::Timing _Main_Timer_; \
	_Main_Timer_.tic(); \
	try 
#define pcl_MainEnd(error_stream) catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} catch (...) { \
		error_stream << "Error: Unexpected exception encountered!" << std::endl; \
		return 1; \
	} \
	_Main_Timer_.toc(); \
	std::cout << std::endl << "Completed normally (with status " << _Main_Return_Status_ << ") after " << _Main_Timer_.getClockInSeconds() << " seconds" << std::endl; \
	return _Main_Return_Status_; \
}
#define pcl_Main(main_func, error_stream) int main(int argc, char *argv[]) {\
	pcl::Timing _Main_Timer_; \
	int _Main_Return_Status_ = 0; \
	_Main_Timer_.tic(); \
	try { \
		_Main_Return_Status_ = main_func(argc, argv); \
	} catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	}  catch (...) { \
		error_stream << "Error: Unexpected exception encountered!" << std::endl; \
		return 1; \
	} \
	_Main_Timer_.toc(); \
	std::cout << std::endl << "Completed normally (with status " << _Main_Return_Status_ << ") after " << _Main_Timer_.getClockInSeconds() << " seconds" << std::endl; \
	return _Main_Return_Status_; \
}
#else
#define pcl_MainStart { \
	int _Main_Return_Status_ = 0; \
	try
#define pcl_MainEnd(error_stream) catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	} catch (...) { \
		error_stream << "Error: Unexpected exception encountered!" << std::endl; \
		return 1; \
	} \
	return _Main_Return_Status_; \
}
#define pcl_Main(main_func, error_stream) int main(int argc, char *argv[]) {\
	int _Main_Return_Status_ = 0; \
	try { \
		_Main_Return_Status_ = main_func(argc, argv); \
	} catch (const pcl::NotAnException&) { \
	} catch (const pcl::Exception& e) { \
		error_stream << e; \
		return 1; \
	} catch (const std::exception& e) {\
		error_stream << "Error: " << e.what() << std::endl; \
		return 1; \
	}  catch (...) { \
		error_stream << "Error: Unexpected exception encountered!" << std::endl; \
		return 1; \
	} \
	return _Main_Return_Status_; \
}
#endif
#endif

namespace pcl
{
	struct NotAnException: public pcl::Exception {};
}

#define pcl_MainReturn(val) _Main_Return_Status_ = val; throw pcl::NotAnException()

#define pcl_CallTemplateFunction(type, function) \
	if (type==typeid(char)) ##function<int>(); \
	else if (type==typeid(unsigned char)) ##function<unsigned char>(); \
	else if (type==typeid(int)) ##function<int>(); \
	else if (type==typeid(unsigned int)) ##function<unsigned int>(); \
	else if (type==typeid(short)) ##function<short>(); \
	else if (type==typeid(unsigned short)) ##function<unsigned short>(); \
	else if (type==typeid(long)) ##function<long>(); \
	else if (type==typeid(unsigned long)) ##function<unsigned long>(); \
	else if (type==typeid(float)) ##function<float>(); \
	else if (type==typeid(double)) ##function<double>(); \
	else

#define pcl_CallTemplateFunction1(type, function, parameter) if (type==typeid(char)) ##function<int>(parameter); \
	else if (type==typeid(unsigned char)) ##function<unsigned char>(parameter); \
	else if (type==typeid(int)) ##function<int>(parameter); \
	else if (type==typeid(unsigned int)) ##function<unsigned int>(parameter); \
	else if (type==typeid(short)) ##function<short>(parameter); \
	else if (type==typeid(unsigned short)) ##function<unsigned short>(parameter); \
	else if (type==typeid(long)) ##function<char>(parameter); \
	else if (type==typeid(unsigned long)) ##function<unsigned long>(parameter); \
	else if (type==typeid(float)) ##function<float>(parameter); \
	else if (type==typeid(double)) ##function<double>(parameter); \
	else

#define pcl_CallTemplateFunction2(type, function, parameter1, parameter2) \
	if (type==typeid(char)) ##function<char>(parameter1, parameter2); \
	else if (type==typeid(unsigned char)) ##function<unsigned char>(parameter1, parameter2); \
	else if (type==typeid(int)) ##function<int>(parameter1, parameter2); \
	else if (type==typeid(unsigned int)) ##function<unsigned int>(parameter1, parameter2); \
	else if (type==typeid(short)) ##function<short>(parameter1, parameter2); \
	else if (type==typeid(unsigned short)) ##function<unsigned short>(parameter1, parameter2); \
	else if (type==typeid(long)) ##function<char>(parameter1, parameter2); \
	else if (type==typeid(unsigned long)) ##function<unsigned long>(parameter1, parameter2); \
	else if (type==typeid(float)) ##function<float>(parameter1, parameter2); \
	else if (type==typeid(double)) ##function<double>(parameter1, parameter2); \
	else

#define pcl_CallTemplateFunction3(type, function, parameter1, parameter2, parameter3) \
	if (type==typeid(char)) ##function<char>(parameter1, parameter2, parameter3); \
	else if (type==typeid(unsigned char)) ##function<unsigned char>(parameter1, parameter2, parameter3); \
	else if (type==typeid(int)) ##function<int>(parameter1, parameter2, parameter3); \
	else if (type==typeid(unsigned int)) ##function<unsigned int>(parameter1, parameter2, parameter3); \
	else if (type==typeid(short)) ##function<short>(parameter1, parameter2, parameter3); \
	else if (type==typeid(unsigned short)) ##function<unsigned short>(parameter1, parameter2, parameter3); \
	else if (type==typeid(long)) ##function<char>(parameter1, parameter2, parameter3); \
	else if (type==typeid(unsigned long)) ##function<unsigned long>(parameter1, parameter2, parameter3); \
	else if (type==typeid(float)) ##function<float>(parameter1, parameter2, parameter3); \
	else if (type==typeid(double)) ##function<double>(parameter1, parameter2, parameter3); \
	else

#endif