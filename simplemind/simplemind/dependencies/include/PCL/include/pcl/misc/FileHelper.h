#ifndef PCL_FILE_HELPER
#define PCL_FILE_HELPER

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace pcl
{
	namespace misc
	{
		using namespace boost::filesystem;

		class FileHelper
		{
		public:
			static std::string GetNewFile(const std::string& prefix, int* start_id, const std::string& postfix)
			{
				std::string filename;
				bool done = false;
				while (!done) {
					 filename = prefix + boost::lexical_cast<std::string>(*start_id) + postfix;
					 if (!exists(filename)) {
						 done = true;
					 } else {
						 ++(*start_id);
					 }
				}
				return filename;
			}

			static std::string GetNewFile(const std::string& prefix, int start_id, const std::string& postfix)
			{
				std::string filename;
				bool done = false;
				while (!done) {
					 filename = prefix + boost::lexical_cast<std::string>(start_id) + postfix;
					 if (!exists(filename)) {
						 done = true;
					 } else {
						 ++start_id;
					 }
				}
				return filename;
			}
		};

	}
}

#endif