#ifndef PCL_QCONDOR_SUBMIT_HANDLER
#define PCL_QCONDOR_SUBMIT_HANDLER

#include <boost/algorithm/string.hpp>
#include <pcl/misc/popen.h>
#include <pcl/condor/Environment.h>
#include <pcl/exception.h>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

namespace pcl
{
	namespace condor
	{

		class QCondorSubmitHandler: private boost::noncopyable
		{
		public:
			typedef QCondorSubmitHandler Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<const Self> ConstantPointer;

			static Pointer New(const std::string& list_file, const Environment::ConstantPointer& env, const std::string& working_directory="")
			{
				return Pointer(new QCondorSubmitHandler(list_file, env, working_directory));
			}

			static Pointer New(const std::vector<std::string>& commands, const Environment::ConstantPointer& env, const std::string& working_directory="")
			{
				std::string list_file = "command_list.txt";
				if (working_directory!="") {
					list_file = working_directory+"/"+list_file;
					if (!boost::filesystem::is_directory(working_directory)) {
						if (!boost::filesystem::create_directories(working_directory)) pcl_ThrowException(pcl::Exception(), "Failed creating "+working_directory);
					}
				}
				std::ofstream os(list_file.c_str());
				pcl_ForEach(commands, item) {
					os << *item << std::endl;
				}
				os.close();
				return Pointer(new QCondorSubmitHandler(list_file, env, working_directory));
			}

			void enableLimit25(bool en)
			{
				m_Limit25 = en;
			}

			bool submit()
			{
				std::string cmd = m_Environment->getQCondorSubmit();
				cmd += " -l " + m_ListFile;
				if (m_WorkingDirectory!="") cmd += " -w " + m_WorkingDirectory;
				if (m_Limit25) cmd += " --limit25";

				std::string last_line;
				{
					FILE *fpipe = pcl_popen(cmd.c_str(), "r");
					if (!fpipe) pcl_ThrowException(pcl::Exception(), std::string("Failure in executing \"")+cmd+"\"");
					char buff[1000];
					while(fgets(buff, sizeof(buff), fpipe)!=NULL) last_line = buff;
					pcl_pclose(fpipe);
				}

				if (!parseLastLine(last_line)) {
					pcl_ThrowException(pcl::Exception(), std::string("Failure in parsing last line \"")+last_line+"\"");
				}
				return true;
			}

			int jobNum()
			{
				return m_JobNum;
			}

			bool isDone()
			{
				if (m_JobNum==0) return false;
				std::string prefix = "log/"+boost::lexical_cast<std::string>(m_Cluster)+".";
				if (m_WorkingDirectory!="") prefix = m_WorkingDirectory+"/"+prefix;
				for (int i=0; i<m_JobNum; ++i) {
					if (!m_Environment->isDone(prefix+boost::lexical_cast<std::string>(i)+".log")) return false;
				}
				return true;
			}

			bool isDone(int max_time_sec_per_job)
			{
				if (m_JobNum==0) return false;
				std::string prefix = "log/"+boost::lexical_cast<std::string>(m_Cluster)+".";
				if (m_WorkingDirectory!="") prefix = m_WorkingDirectory+"/"+prefix;
				for (int i=0; i<m_JobNum; ++i) {
					if (!m_Environment->isDone(prefix+boost::lexical_cast<std::string>(i)+".log", max_time_sec_per_job)) return false;
				}
				return true;
			}

		protected:
			std::string m_ListFile;
			std::string m_WorkingDirectory;
			Environment::ConstantPointer m_Environment;
			bool m_Limit25;
			int m_JobNum, m_Cluster;

			QCondorSubmitHandler(const std::string& list_file, const Environment::ConstantPointer& env, const std::string& working_directory)
			{
				m_ListFile = list_file;
				m_Environment = env;
				m_WorkingDirectory = working_directory;
				m_JobNum = 0;
				m_Cluster = 0;
			}

			bool parseLastLine(const std::string& last_line)
			{
				size_t space_pos = last_line.find_first_of(' ');
				if (space_pos<=0) return false;
				m_JobNum = boost::lexical_cast<int>(last_line.substr(0, space_pos));
				space_pos = last_line.find_last_of(' ');
				if (space_pos<=0) return false;
				size_t dot_pos = last_line.find_last_of('.');
				if (space_pos>=dot_pos) return false;
				m_Cluster = boost::lexical_cast<int>(last_line.substr(space_pos+1, dot_pos-(space_pos+1)));
				return true;
			}
		};

	}
}

#endif