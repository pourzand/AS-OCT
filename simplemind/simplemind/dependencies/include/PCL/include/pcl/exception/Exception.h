#ifndef PCL_EXCEPTION
#define PCL_EXCEPTION

#include <boost/exception/all.hpp>
#include <string>
#include <stdexcept>
#include <iostream>

#ifndef PCL_THROW_DETAILS
#define pcl_ThrowException(e, msg) boost::throw_exception( ::boost::enable_error_info(e) << \
	::pcl::exception_details::throw_message(msg) << \
	::boost::throw_function(BOOST_CURRENT_FUNCTION) << \
	::boost::throw_file(__FILE__) << \
	::boost::throw_line((int)__LINE__) )
#else
#define pcl_ThrowException(e, msg) boost::throw_exception(e << ::pcl::exception_details::throw_message(msg))
#endif

#define pcl_ThrowSimpleException(e, msg) boost::throw_exception(e << ::pcl::exception_details::throw_message(msg))

namespace pcl
{

	namespace exception_details
	{
	
		typedef boost::error_info<struct throw_message_, std::string> throw_message;

		struct Exception: virtual boost::exception, virtual std::exception 
		{
			virtual std::string getMessageHeader() const
			{
				return "Exception: ";
			}

			virtual std::ostream& print(std::ostream& os) const
			{
				if ( auto msg = boost::get_error_info<throw_message>(*this) ) {
					os << this->getMessageHeader() << *msg << std::endl;
				}
				if ( auto msg = boost::get_error_info<boost::throw_line>(*this) ) {
					os << "At line: " << *msg << std::endl;
				}
				if ( auto msg = boost::get_error_info<boost::throw_file>(*this) ) {
					os << "In file: " << *msg << std::endl;
				}
				if ( auto msg = boost::get_error_info<boost::throw_function>(*this) ) {
					os << "From function: " << *msg << std::endl;;
				}
				return os;
			}
			
			friend std::ostream& operator<<(std::ostream& os, const Exception& e)
			{
				return e.print(os);
			}
		};
	}

	typedef pcl::exception_details::Exception Exception;

	struct InvalidValueException: public Exception {};
	struct FileIOException: public Exception {};
}

#endif