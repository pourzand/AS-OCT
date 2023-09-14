#ifndef ELASTIX_TRANSFORMIX_POINT
#define ELASTIX_TRANSFORMIX_POINT

#include <pcl/elastix/Environment.h>
#include <pcl/geometry.h>
#include <pcl/misc/StringTokenizer.h>
#include <pcl/misc/FileStreamHelper.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

namespace pcl
{
	namespace elastix
	{

		class TransformixPoint
		{
		public:
			TransformixPoint()
			{}

			TransformixPoint(const Environment::ConstantPointer& env)
			{
				setEnvironment(env);
			}

			void setEnvironment(const Environment::ConstantPointer& env)
			{
				m_Environment = env;
			}

			const Environment::ConstantPointer& getEnvironment() const
			{
				return m_Environment;
			}

			void setTransformParameter(const std::string& file) 
			{ 
				m_TransformParameterFile = file; 
			}

			template <class PointList>
			std::string update(const PointList& points, bool no_execution=false) 
			{
				writePointFile(m_Environment->getWorkingDirectory() + "/input_points.txt", points);
				std::string command = " " + Environment::Encapsulate(m_Environment->getTransformix())
					+ " -def " + Environment::Encapsulate(m_Environment->getWorkingDirectory()+"/input_points.txt")
					+ " -out " + Environment::Encapsulate(m_Environment->getWorkingDirectory())
					+ " -tp " + Environment::Encapsulate(m_TransformParameterFile)
					+ " > NUL";
				if (!no_execution) system(command.c_str());
				return command;
			}

			std::vector<pcl::Point3D<double>> getResult() const
			{
				std::vector<pcl::Point3D<double>> result;
				auto is = pcl::FileStreamHelper::CreateIfstream(m_Environment->getWorkingDirectory()+"/outputpoints.txt");
				char buffer[1000];
				while (is.good()) {
					buffer[0] = 0;
					is.getline(buffer, 1000);
					if (buffer[0]==0) break;
					result.push_back(processLine(buffer));
				}
				return std::move(result);
			}

			void clean()
			{
				Environment::DeleteFile(m_Environment->getWorkingDirectory() + "/input_points.txt");
				Environment::DeleteFile(m_Environment->getWorkingDirectory() + "/outputpoints.txt");
			}

		protected:
			Environment::ConstantPointer m_Environment;
			std::vector<pcl::Point3D<double>> m_PointList;
			std::string m_OutputPath, m_TransformParameterFile;

			template <class PointList>
			void writePointFile(const std::string& file, const PointList& list) {
				std::ofstream os = pcl::FileStreamHelper::CreateOfstream(file);
				os << "point" << std::endl;
				os << list.size() << std::endl;
				pcl_ForEach(list, item) {
					for (int i=0; i<3; ++i) os << std::setprecision(10) << (*item)[i] << " ";
					os << std::endl;
				}
				os.close();
			}

			pcl::Point3D<double> processLine(char* buffer) const
			{
				pcl::misc::StringTokenizer tokenizer(buffer);
				tokenizer.begin(';'); //Getting point number
				tokenizer.next(';'); //Getting input index
				tokenizer.next(';'); //Getting input point
				tokenizer.next(';'); //Getting output index fixed
				//Now at output point
				tokenizer.next(std::string("[ "));
				pcl::Point3D<double> result;
				for (int i=0; i<3; ++i) {
					tokenizer.next(' ');
					result[i] = atof(tokenizer.getToken().c_str());
				}
				return result;
			}
		};

	}
}

#endif