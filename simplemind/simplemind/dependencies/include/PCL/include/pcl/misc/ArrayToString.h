#ifndef PCL_ARRAY_TO_STRING
#define PCL_ARRAY_TO_STRING

#include <string>
#include <sstream>

namespace pcl
{
	namespace misc
	{
		
		template <class T>
		std::string getStringFromArray(const T* arr, int num, const std::string& delimiter=" ")
		{
			std::stringstream result;
			result << arr[0];
			for (int i=1; i<num; ++i) {
				result << delimiter << arr[i];
			}
			return result.str();
		}
		
	}
}

#endif