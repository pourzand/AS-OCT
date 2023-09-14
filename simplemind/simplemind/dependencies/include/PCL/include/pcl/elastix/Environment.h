#ifndef ELASTIX_ENVIRONMENT
#define ELASTIX_ENVIRONMENT

#include <string>
#include <pcl/os.h>
#include <pcl/misc/FileStreamHelper.h>
#include <pcl/misc/FileNameTokenizer.h>
#include <pcl/misc/ConfigFileReader.h>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>

namespace pcl
{
	namespace elastix
	{

		/*
			Uses ElastixExe and TransformixExe fields from ConfigFileReader!
		*/
		class Environment
		{
		public:
			typedef Environment Self;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;

			static Pointer New(const std::string& working_dir="", const pcl::misc::ConfigFileReader& conf=pcl::misc::ConfigFileReader())
			{
				return Pointer(new Environment(working_dir, conf));
			}

#ifdef WINDOWS
			static std::string Encapsulate(const std::string& str)
			{
				if (str.find(' ')==std::string::npos) return str;
				else return "\""+str+"\"";
			}
#else
			static const std::string& Encapsulate(const std::string& str)
			{
				return str;
			}
#endif

			static void DeleteFile(const std::string& file)
			{
				if (boost::filesystem::exists(file)) boost::filesystem::remove(file);
			}

			static void DeleteMhd(const std::string& file)
			{
				if (boost::filesystem::exists(file)) {
					boost::filesystem::remove(file);
					pcl::FileNameTokenizer fname(file);
					std::string raw_file = fname.getPath()+fname.getFileName()+".raw";
					if (boost::filesystem::exists(raw_file)) boost::filesystem::remove(raw_file);
				}
			}

			void setWorkingDirectory(const std::string& str)
			{
				m_WorkingDirectory = str;
				if (m_WorkingDirectory!="") {
					if (!boost::filesystem::is_directory(m_WorkingDirectory)) boost::filesystem::create_directories(m_WorkingDirectory);
				}
			}

			void setTransformixExe(const std::string& str)
			{
				m_TransformixExe = str;
			}

			void setElastixExe(const std::string& str)
			{
				m_ElastixExe = str;
			}

			std::string getWorkingDirectory() const
			{
				return m_WorkingDirectory;
			}

			std::string getElastix() const
			{
				return m_ElastixExe;
			}

			std::string getTransformix() const
			{
				return m_TransformixExe;
			}

		protected:
			std::string m_TransformixExe;
			std::string m_ElastixExe;
			std::string m_WorkingDirectory;

			Environment(const std::string& working_dir, const pcl::misc::ConfigFileReader& conf)
			{
				setElastixExe("C:\\pechin\\CppLibraries\\Elastix\\elastix_windows64_v4.4\\elastix.exe");
				setTransformixExe("C:\\pechin\\CppLibraries\\Elastix\\elastix_windows64_v4.4\\transformix.exe");
				if (!conf.empty()) {
					if (conf.exist("ElastixExe")) setElastixExe(conf.getFieldValue("ElastixExe"));
					if (conf.exist("TransformixExe")) setTransformixExe(conf.getFieldValue("TransformixExe"));
				}
				setWorkingDirectory(working_dir);
			}
		};

	}
}

#endif