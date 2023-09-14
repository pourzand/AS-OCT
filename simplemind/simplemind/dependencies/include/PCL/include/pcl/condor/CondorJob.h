#ifndef PCL_CONDOR_JOB
#define PCL_CONDOR_JOB

#include <boost/algorithm/string.hpp>
#include <pcl/misc/popen.h>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

namespace pcl
{
	namespace condor
	{

		class CondorJob: private boost::noncopyable
		{
		public:
			typedef CondorJob Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<const Self> ConstantPointer;

			static bool IsCondorAvailable()
			{
				return system("condor_version")==0;
			}

			static Pointer New(const std::string& universe, const std::string& executable, const std::string& requirements, const std::string& ouput, const std::string& error, const std::string& log)
			{
				return Pointer(new CondorJob(universe, executable, requirements, ouput, error, log));
			}

			void set(const std::string& key, const std::string& value)
			{
				if (stricmp(key.c_str(), "ARGUMENTS")==0) pcl_ThrowExceptions(pcl::Exceptions(), "Invalid use of \""+key+"\" in . Use addProcess() instead");
				m_Config[boost::to_upper_copy(key)] = value;
			}

			std::string& get(const std::string& key)
			{
				return m_Config.at(boost::to_upper_copy(key));
			}

			const std::string& get(const std::string& key) const
			{
				return m_Config.at(boost::to_upper_copy(key));
			}

			void addProcess(const std::string& arg)
			{
				m_Process.push_back(arg);
			}

			int getProcessNum() const
			{
				return m_Process.size();
			}

			std::vector<std::string>& getProcess()
			{
				return m_Process;
			}

			const std::vector<std::string>& getProcess() const
			{
				return m_Process;
			}

			std::string& getProcess(int i)
			{
				return m_Process[i];
			}

			const std::string& getProcess(int i) const
			{
				return m_Process[i];
			}

			int submit()
			{
				FILE *fpipe = pcl_popen("condor_submit", "w");
				print(fpipe);
				return pcl_pclose(fpipe);
			}

			bool isDone()
			{
				return system(("condor_wait "+get("log")).c_str())==0;
			}

			bool isDone(long max_time_sec)
			{
				return system(("condor_wait -wait "+boost::lexical_cast<std::string>(max_time_sec)+" "+get("log")).c_str())==0;
			}

			void print()
			{
				printf(stdout);
			}

		protected:
			boost::unordered_map<std::string, std::string> m_Config;
			std::vector<std::string> m_Process;

			CondorJob(const std::string& universe, const std::string& executable, const std::string& requirements, const std::string& ouput, const std::string& error, const std::string& log)
			{
				set("universe", universe);
				set("executable", executable);
				set("requirements", requirements);
				set("output", output);
				set("error", error);
				set("log", log);
			}

			void print(FILE* out)
			{
				pcl_ForEach(m_Config, item) {
					fprintf(out, "%s = %s\n", item->first.c_string(), item->second.c_string());
				}
				pcl_ForEach(m_Process, item) {
					fprintf(out, "ARGUMENTS = %s\n", item->c_string());
					fprintf(out, "QUEUE\n");
				}
				fprintf(out, "\n");
			}
		};

	}
}

#endif