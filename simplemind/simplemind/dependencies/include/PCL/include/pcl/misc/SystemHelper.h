#ifndef PCL_SYSTEM_HELPER
#define PCL_SYSTEM_HELPER

#include <stdlib.h> 
#include <algorithm>
#include <boost/filesystem.hpp>
#include <pcl/misc/FileStreamHelper.h>

namespace pcl
{
	namespace misc
	{

		class SystemHelper
		{
		public:
			static int SystemForLongCommand(const std::string& command, const std::string& temp_file)
			{
				auto os = pcl::FileStreamHelper::CreateOfstream(temp_file);
				os << command;
				os.close();

				std::string exec_command = temp_file;
				std::replace(exec_command.begin(), exec_command.end(), '/', '\\');
				int return_val = system(exec_command.c_str());

				boost::filesystem::remove(temp_file);
				return return_val;
			}
		};

	}
}

#endif