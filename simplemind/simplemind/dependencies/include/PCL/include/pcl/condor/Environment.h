#ifndef CONDOR_ENVIRONMENT
#define CONDOR_ENVIRONMENT

#include <string>
#include <pcl/misc/ConfigFileReader.h>
#include <pcl/misc/popen.h>
#include <boost/smart_ptr.hpp>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace condor
	{

		/*
			QCondorSubmit fields from ConfigFileReader!
		*/
		class Environment
		{
		public:
			typedef Environment Self;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;

			static Pointer New(const pcl::misc::ConfigFileReader& conf=pcl::misc::ConfigFileReader())
			{
				return Pointer(new Environment(conf));
			}

			bool IsCondorAvailable() const
			{
				return pcl_pclose(pcl_popen("condor_version", "r"))==0;
			}

			bool isDone(const std::string& logfile) const
			{
				return pcl_pclose(pcl_popen(("condor_wait "+logfile).c_str(),"r"))==0;
			}

			bool isDone(const std::string& logfile, long max_time_sec) const
			{
				return pcl_pclose(pcl_popen(("condor_wait -wait "+boost::lexical_cast<std::string>(max_time_sec)+" "+logfile).c_str(),"r"))==0;
			}

			const std::string& getQCondorSubmit() const
			{
				return m_QCondorSubmit;
			}

			void setQCondorSubmit(const std::string& str)
			{
				m_QCondorSubmit = str;
			}

		protected:
			std::string m_QCondorSubmit;

			Environment(const pcl::misc::ConfigFileReader& conf)
			{
				setQCondorSubmit("M:\\apps\\personal\\pechin\\qcondor_submit\\qcondor_submit.py");
				if (!conf.empty()) {
					if (conf.exist("QCondorSubmit")) setQCondorSubmit(conf.getFieldValue("QCondorSubmit"));
				}
			}
		};

	}
}

#endif