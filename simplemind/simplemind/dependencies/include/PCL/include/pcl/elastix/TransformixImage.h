#ifndef ELASTIX_TRANSFORMIX_IMAGE
#define ELASTIX_TRANSFORMIX_IMAGE

#include <pcl/elastix/Environment.h>
#include <pcl/image_io.h>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace elastix
	{

		class TransformixImage
		{
		public:
			TransformixImage()
			{}

			TransformixImage(const Environment::ConstantPointer& env)
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

			template <class ImagePointer>
			void setImage(const ImagePointer& image) 
			{
				std::string file = m_Environment->getWorkingDirectory()+"/input_image.mhd";
				pcl::ImageIoHelper::Write(file, image);
				m_InputImage.set(file, true);

			}

			void setImageFile(const std::string& file) 
			{
				m_InputImage.set(file, false);

			}

			void setTransformParameter(const std::string& file) 
			{ 
				m_TransformParameterFile = file; 
			}

			std::string update(bool no_execution=false)
			{
				std::string command = Environment::Encapsulate(m_Environment->getTransformix());
				command += " -in " + Environment::Encapsulate(m_Environment->getWorkingDirectory()+"/input_image.mhd")
					+ " -out " + Environment::Encapsulate(m_Environment->getWorkingDirectory())
					+ " -tp " + Environment::Encapsulate(m_TransformParameterFile);
				if (!no_execution) system(command.c_str());
				return command;
			}

			template <class ImageType>
			typename ImageType::Pointer getResultImage() 
			{
				std::string filename = m_Environment->getWorkingDirectory() + "/result.mhd";
				if (!boost::filesystem::exists(filename)) return ImageType::Pointer();
				return pcl::ImageIoHelper::Read<ImageType>(filename);
			}

			void clean(bool include_result_image=true)
			{
				if (m_InputImage.is_temp) Environment::DeleteMhd(m_InputImage.file);
				if (include_result_image) {
					Environment::DeleteMhd(m_Environment->getWorkingDirectory() + "/result.mhd");
				}
			}

		protected:
			Environment::ConstantPointer m_Environment;
			std::string m_TransformParameterFile;
			struct FileInfo 
			{
				std::string file;
				bool is_temp;

				FileInfo()
				{
					is_temp = false;
				}

				void set(const std::string& f, bool i)
				{
					file = f;
					is_temp = i;
				}
			};
			FileInfo m_InputImage;
		};

	}
}

#endif