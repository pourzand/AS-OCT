#ifndef PCL_ITK_IMAGE_READER
#define PCL_ITK_IMAGE_READER

#include <pcl/image_io/ImageReader.h>

#include "itkImage.h"
#include "itkImageFileReader.h"
#include <pcl/misc/StringTokenizer.h>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class ItkImageReader: public ImageReader<ImageType>
		{
		public:
			typedef ItkImageReader Self;
			typedef ImageReader<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::OutputImageType OutputImageType;

			static ImageFileInfo ReadImageInformation(const std::string& filename)
			{
				RegisterImageIoFactory();
				typedef itk::ImageIOBase::IOComponentType ScalarPixelType;
				itk::ImageIOBase::Pointer image_io = itk::ImageIOFactory::CreateImageIO(filename.c_str(), itk::ImageIOFactory::ReadMode);
				if (image_io.IsNull()) {
					pcl_ThrowException(ImageWriterException(), "Null image IO encountered!");
				}
				try {
					image_io->SetFileName(filename.c_str());
					image_io->ReadImageInformation();
				} catch (itk::ExceptionObject & e) {
					pcl_ThrowException(ImageReaderException(), e.what());
				}
				const ScalarPixelType pixelType = image_io->GetComponentType();
				switch (pixelType) {
				case itk::ImageIOBase::CHAR: 
					return ImageFileInfo::New<char>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::UCHAR: 
					return ImageFileInfo::New<unsigned char>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::SHORT:
					return ImageFileInfo::New<short>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::USHORT: 
					return ImageFileInfo::New<unsigned short>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::INT: 
					return ImageFileInfo::New<int>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::UINT: 
					return ImageFileInfo::New<unsigned int>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::LONG: 
					return ImageFileInfo::New<long>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::ULONG: 
					return ImageFileInfo::New<unsigned long>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::FLOAT: 
					return ImageFileInfo::New<float>(filename, image_io->GetNumberOfDimensions());
				case itk::ImageIOBase::DOUBLE: 
					return ImageFileInfo::New<double>(filename, image_io->GetNumberOfDimensions());
				default:
					pcl_ThrowException(ImageReaderException(), "Invalid type encountered!");
				};
			}

			static typename OutputImageType::Pointer ReadImage(const std::string& filename, itk::ImageIOBase::Pointer io = itk::ImageIOBase::Pointer())
			{
				Pointer reader = New(filename);
				typename OutputImageType::Pointer result;
				if (!io.IsNull()) reader->setImageIo(io);
				reader->read();
				result = reader->getOutputImage();
				return result;
			}

			static Pointer New(const std::string& filename)
			{
				Pointer obj(new Self());
				obj->m_FileName = filename;
				return obj;
			}

			void setImageIo(itk::ImageIOBase::Pointer io)
			{
				m_IoBase = io;
			}

			virtual void read()
			{
				if (m_IoBase.IsNull()) RegisterImageIoFactory();
				typedef itk::Image<typename ImageType::ValueType,3 > ItkImageType;
				typedef itk::ImageFileReader<ItkImageType> ItkReaderType;
				ItkReaderType::GlobalWarningDisplayOff();

				typename ItkReaderType::Pointer reader = ItkReaderType::New();
				if (!m_IoBase.IsNull()) {
					reader->SetImageIO(m_IoBase);
				}
				reader->SetFileName(m_FileName);
				try {
					reader->Update();
				} catch (itk::ExceptionObject & e) {
					pcl_ThrowException(ImageReaderException(), e.what());
				}

				this->m_OutputImage = ImageHelper::CreateFromItkImage<OutputImageType>(typename ItkImageType::Pointer(reader->GetOutput()), true);
				if (strcmp(reader->GetImageIO()->GetNameOfClass(), "GDCMImageIO") == 0) correctForGDCM();
			}

		protected:
			std::string m_FileName;
			itk::ImageIOBase::Pointer m_IoBase;

			ItkImageReader() {}

			pcl::Point3D<double> getPointFromMetadata(const std::string& str)
			{
				pcl::Point3D<double> result(0, 0, 0);
				int i = 0;
				pcl::misc::StringTokenizer tokenize(str.c_str());
				for (tokenize.begin('\\'); !tokenize.end(); tokenize.next('\\')) {
					std::string temp = tokenize.getToken();
					boost::trim(temp);
					result[i] = boost::lexical_cast<double>(temp);
					++i;
					if (i == 3) break;
				}
				return result;
			}

			boost::tuple<pcl::Point3D<double>,pcl::Point3D<double>> getAxisFromMetadata(const std::string& str)
			{
				pcl::Point3D<double> x, y;
				int i = 0;
				pcl::misc::StringTokenizer tokenize(str.c_str());
				for (tokenize.begin('\\'); !tokenize.end(); tokenize.next('\\')) {
					std::string temp = tokenize.getToken();
					boost::trim(temp);
					if (i<3) x[i] = boost::lexical_cast<double>(temp);
					else y[i-3] = boost::lexical_cast<double>(temp);
					++i;
					if (i == 6) break;
				}
				return boost::tuple<pcl::Point3D<double>, pcl::Point3D<double>>(x,y);
			}

			//A correction for when GDCM became too clever for it's own good in ITK 4.8
			void correctForGDCM()
			{
#if ITK_VERSION_MAJOR>=4
				auto metadata = this->m_OutputImage->getMetadata();
				auto spacing = this->m_OutputImage->getSpacing(),
					origin = this->m_OutputImage->getOrigin();
				auto orientation = this->m_OutputImage->getOrientationMatrix();
				bool modified = false;
				if (origin == pcl::Point3D<double>(0, 0, 0)) {
					auto iter = metadata->find("0020|0032");
					if (iter != metadata->end()) {
						origin = getPointFromMetadata(std::string(metadata->template getValue<std::string>(iter).c_str()));
						modified = true;
					}
				}
				if (spacing[0]==1 && spacing[1]==1) {
					auto iter = metadata->find("0028|0030");
					if (iter != metadata->end()) {
						auto temp = getPointFromMetadata(std::string(metadata->template getValue<std::string>(iter).c_str()));
						spacing[0] = temp[0];
						spacing[1] = temp[1];
						modified = true;
					}
				}
				if (orientation.is_identity()) {
					auto iter = metadata->find("0020|0037");
					if (iter != metadata->end()) {
						auto xy = getAxisFromMetadata(std::string(metadata->template getValue<std::string>(iter).c_str()));
						auto z = boost::get<0>(xy).getCrossProduct(boost::get<1>(xy));
						//auto z = xy.get<0>().getCrossProduct(xy.get<1>());
						for (int i = 0; i < 3; ++i) {
							orientation(i, 0) = boost::get<0>(xy)[i];
							orientation(i, 1) = boost::get<1>(xy)[i];
							orientation(i, 2) = z[i];
						}
						modified = true;
					}
				}
				if (modified) {
					auto temp = this->m_OutputImage->getAlias(
						this->m_OutputImage->getMinPoint(),
						spacing,
						origin,
						orientation
						);
					temp->setMetadata(this->m_OutputImage->getMetadata());
					this->m_OutputImage = temp;
				}
#endif
			}
		};

	}
}

#endif