#ifndef PCL_COMPLETION_TRACKER
#define PCL_COMPLETION_TRACKER

#include <string>
#include <stdio.h>
#include <fstream>

namespace pcl
{
	namespace misc
	{

		class CompletionTracker
		{
		public:
			CompletionTracker(const std::string& file)
			{
				m_File = file;
			}

			bool reset()
			{
				return remove(m_File.c_str())!=0;
			}

			bool complete()
			{
				FILE *fptr = fopen(m_File.c_str(), "w");
				if (fptr==NULL) return false;
				fclose(fptr);
				return true;
			}

			bool isCompleted() const
			{
				std::ifstream infile(m_File.c_str());
				return infile.good();
			}

		protected:
			std::string m_File;
		};

	}
}

#endif