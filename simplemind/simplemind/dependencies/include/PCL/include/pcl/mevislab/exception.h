#ifndef PCL_MEVISLAB_EXCEPTION
#define PCL_MEVISLAB_EXCEPTION

#include <pcl/exception.h>

namespace pcl
{
	namespace mevislab
	{

		struct UnsupportedTypeException:public pcl::Exception
		{};
		
	}
}

#endif