#ifndef ELASTIX_ELASTIX
#define ELASTIX_ELASTIX

#include <pcl/elastix/Environment.h>
#include <pcl/image_io.h>
#include <pcl/misc/FileNameTokenizer.h>
#include <pcl/misc/SystemHelper.h>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace elastix
	{

		class Elastix
		{
		public:
			Elastix()
			{}

			Elastix(const Environment::ConstantPointer& env)
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
			void setFixedImage(const ImagePointer& img, unsigned int channel=0) 
			{
				std::string file = m_Environment->getWorkingDirectory()+"/fixed."+boost::lexical_cast<std::string>(channel)+".mhd";
				pcl::ImageIoHelper::Write(file, img);
				add(channel, file, true, m_FixedImageFile);
			}

			void setFixedImageFile(const std::string& file, unsigned int channel=0) 
			{
				add(channel, file, false, m_FixedImageFile);
			}

			template <class ImagePointer>
			void setMovingImage(ImagePointer& img, unsigned int channel=0) 
			{
				std::string file = m_Environment->getWorkingDirectory()+"/moving."+boost::lexical_cast<std::string>(channel)+".mhd";
				pcl::ImageIoHelper::Write(file, img);
				add(channel, file, true, m_MovingImageFile);
			}

			void setMovingImageFile(const std::string& file, unsigned int channel=0) 
			{
				add(channel, file, false, m_MovingImageFile);
			}

			template <class ImagePointer>
			void setFixedMask(ImagePointer& img) 
			{
				std::string file = m_Environment->getWorkingDirectory()+"/fixed_mask.mhd";
				pcl::ImageIoHelper::Write(file, img);
				m_FixedMask.set(file, true);
			}

			void setFixedMaskFile(const std::string& file) 
			{
				m_FixedMask.set(file, false);
			}

			template <class ImagePointer>
			void setMovingMask(ImagePointer& img) 
			{
				std::string file = m_Environment->getWorkingDirectory()+"/moving_mask.mhd";
				pcl::ImageIoHelper::Write(file, img);
				m_MovingMask.set(file, true);
			}

			void setMovingMaskFile(const std::string& file) 
			{
				m_MovingMask.set(file, false);
			}

			void setInitialTransformFile(const std::string& filename) 
			{
				m_InitialTransformFile = boost::filesystem::system_complete(filename).string(); 
			}

			void addParameterFile(const std::string& filename) 
			{ 
				m_ParameterFileList.push_back(boost::filesystem::system_complete(filename).string()); 
			}

			std::string update(bool no_execution=false) 
			{
				if (m_FixedImageFile.size()!=m_MovingImageFile.size()) pcl_ThrowException(pcl::Exception(), "Different channel number for moving and fixed image!");
				int channel_num = m_FixedImageFile.size();
				for (int i=0; i<channel_num; ++i) {
					m_FixedImageFile[i].throwMessageIfInvalid("Channel "+boost::lexical_cast<std::string>(i)+" of fixed image is undefined!");
					m_MovingImageFile[i].throwMessageIfInvalid("Channel "+boost::lexical_cast<std::string>(i)+" of moving image is undefined!");
				}

				std::string command = Environment::Encapsulate(m_Environment->getElastix());
				if (channel_num==1) {
					command += " -f " + Environment::Encapsulate(m_FixedImageFile[0].file)
						+ " -m " + Environment::Encapsulate(m_MovingImageFile[0].file);
				} else {
					for (int i=0; i<channel_num; i++) {
						command += " -f" + boost::lexical_cast<std::string>(channel_num) + Environment::Encapsulate(m_FixedImageFile[i].file)
							+ " -m" + boost::lexical_cast<std::string>(channel_num) + Environment::Encapsulate(m_FixedImageFile[i].file);
					}
				}

				if (!m_FixedMask.file.empty()) command += " -fMask " + Environment::Encapsulate(m_FixedMask.file);
				if (!m_MovingMask.file.empty()) command += " -mMask " + Environment::Encapsulate(m_MovingMask.file);

				command += " -out " + Environment::Encapsulate(m_Environment->getWorkingDirectory());
				if (!m_InitialTransformFile.empty()) command += " -t0 " + Environment::Encapsulate(m_InitialTransformFile);
				pcl_ForEach(m_ParameterFileList, item) command += " -p " + Environment::Encapsulate(*item);

				if (!no_execution) {
					pcl::misc::SystemHelper::SystemForLongCommand(command, m_Environment->getWorkingDirectory() + "/exec.bat");
				}
				return command;
			}

			template <class ImageType>
			typename ImageType::Pointer getResultImage() 
			{
				std::string filename = m_Environment->getWorkingDirectory() + "/result." + boost::lexical_cast<std::string>(m_ParameterFileList.size()-1) + ".mhd";
				if (!boost::filesystem::exists(filename)) return ImageType::Pointer();
				return pcl::ImageIoHelper::Read<ImageType>(filename);
			}

			int getTransformParameterNum()
			{
				return m_ParameterFileList.size();
			}

			std::string getTransformParameterFile(int i) 
			{
				std::string filename = m_Environment->getWorkingDirectory() + "/TransformParameters." + boost::lexical_cast<std::string>(i) + ".txt";
				if (!boost::filesystem::exists(filename)) return "";
				return filename;
			}

			std::string getTransformParameterFile() 
			{
				return getTransformParameterFile(getTransformParameterNum()-1);
			}

			void clean(bool include_result_image=true)
			{
				pcl_ForEach(m_FixedImageFile, item) if (item->is_temp) Environment::DeleteMhd(item->file);
				pcl_ForEach(m_MovingImageFile, item) if (item->is_temp) Environment::DeleteMhd(item->file);
				if (!m_FixedMask.file.empty() && m_FixedMask.is_temp) Environment::DeleteMhd(m_FixedMask.file);
				if (!m_MovingMask.file.empty() && m_MovingMask.is_temp) Environment::DeleteMhd(m_MovingMask.file);
				if (include_result_image) {
					for (int i=0; i<m_ParameterFileList.size(); i++) {
						Environment::DeleteMhd(m_Environment->getWorkingDirectory() + "/result." + boost::lexical_cast<std::string>(i) + ".mhd");
					}
				}
			}

		protected:
			Environment::ConstantPointer m_Environment;
			std::string m_InitialTransformFile;
			std::list<std::string> m_ParameterFileList;

			struct FileInfo
			{
				std::string file;
				bool is_temp;

				FileInfo()
				{
					is_temp = false;
				}

				void set(const std::string& f, bool t)
				{
					file = f;
					is_temp = t;
				}

				void throwMessageIfInvalid(const std::string& msg)
				{
					if (file.empty()) pcl_ThrowException(pcl::Exception(), msg);
				}
			};
			std::vector<FileInfo> m_FixedImageFile, m_MovingImageFile;
			FileInfo m_FixedMask, m_MovingMask;

			void add(unsigned int channel, const std::string& file, bool is_temp, std::vector<FileInfo>& list)
			{
				if (list.size()<(channel+1)) list.resize(channel+1);
				list[channel].set(file, is_temp);
			}


		};

	}
}

#endif