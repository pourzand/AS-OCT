#ifndef PCL_FILE_STREAM_HELPER
#define PCL_FILE_STREAM_HELPER

#include <fstream>
#include <pcl/exception.h>

namespace pcl
{

	struct FileStreamException: public pcl::Exception
	{};

	class FileStreamHelper
	{
	public:
		static std::ifstream CreateIfstream(const std::string& file, std::ios_base::openmode mode=std::ios_base::in)
		{
			std::ifstream stream(file,mode);
			if (stream.fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for input");
			return stream;
		}

		static std::ofstream CreateOfstream(const std::string& file, std::ios_base::openmode mode=std::ios_base::out)
		{
			std::ofstream stream(file,mode);
			if (stream.fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for output");
			return stream;
		}

		static std::fstream CreateFstream(const std::string& file, std::ios_base::openmode mode=std::ios_base::in|std::ios_base::out)
		{
			std::fstream stream(file,mode);
			if (stream.fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for I/O");
			return stream;
		}
	};

}


#endif