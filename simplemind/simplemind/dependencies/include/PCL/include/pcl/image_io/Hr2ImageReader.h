#ifndef PCL_HR2_IMAGE_READER
#define PCL_HR2_IMAGE_READER

#include <pcl/image_io/ImageReader.h>
#include <pcl/image_io/GzRawImageReader.h>
#include <pcl/misc/StringTokenizer.h>
#include <boost/lexical_cast.hpp>

#pragma warning ( push )
#pragma warning ( disable: 4101 )

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class Hr2ImageReader: public ImageReader<ImageType>
		{
		public:
			typedef Hr2ImageReader Self;
			typedef ImageReader<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::OutputImageType OutputImageType;

			static ImageFileInfo ReadImageInformation(const std::string& filename)
			{
				Pointer reader = New(filename);
				return reader->readImageInformation(filename);
			}

			static typename OutputImageType::Pointer ReadImage(const std::string& filename)
			{
				Pointer reader = New(filename);
				typename OutputImageType::Pointer result;
				reader->read();
				result = reader->getOutputImage();
				return result;
			}

			static Pointer New(const std::string& filename)
			{
				Pointer obj(new Self());
				obj->setInputStream(filename);
				return obj;
			}

			virtual void read()
			{
				std::istream &is = *(this->m_InputStream);
				readMetaData(is);
				if ((int)m_ImageDataPos==-1) {
					pcl_ThrowException(ImageReaderException(), "No image data found!");
				}
				is.clear();
				is.seekg(m_ImageDataPos, std::ios::beg);
				if (m_PixelType.compare("char")==0) readImage<char>(is); 
				else if (m_PixelType.compare("unsigned char")==0) readImage<unsigned char>(is);
				else if (m_PixelType.compare("int")==0) readImage<int>(is);
				else if (m_PixelType.compare("unsigned int")==0) readImage<unsigned int>(is);
				else if (m_PixelType.compare("long int")==0) readImage<long int>(is);
				else if (m_PixelType.compare("short")==0) readImage<short>(is);
				else if (m_PixelType.compare("unsigned short")==0) readImage<unsigned short>(is);
				else if (m_PixelType.compare("long")==0) readImage<long>(is);
				else if (m_PixelType.compare("unsigned long")==0) readImage<unsigned long>(is);
				else if (m_PixelType.compare("float")==0) readImage<float>(is);
				else if (m_PixelType.compare("double")==0) readImage<double>(is);
				else {
					std::stringstream ss;
					ss << "Invalid type \"" << m_PixelType << "\" encountered!";
					pcl_ThrowException(ImageReaderException(), ss.str());
				}

				this->m_OutputImage->setMetadata(m_Metadata);
			}

		protected:
			std::streampos m_ImageDataPos;
			std::string m_PixelType;
			Point3D<int> m_Size, m_MinPoint;
			Point3D<double> m_Origin, m_Spacing;
			typename ImageType::OrientationMatrixType m_Orientation;
			Metadata::Pointer m_Metadata;

			Hr2ImageReader() {}

			ImageFileInfo readImageInformation(const std::string& filename)
			{
				this->setInputStream(filename);
				std::istream &is = *(this->m_InputStream);
				readMetaData(is);
				if ((int)m_ImageDataPos==-1) {
					pcl_ThrowException(ImageReaderException(), "No image data found!");
				}
				if (m_PixelType.compare("char")==0) return ImageFileInfo::New<char>(filename, 3); 
				else if (m_PixelType.compare("unsigned char")==0) return ImageFileInfo::New<unsigned char>(filename, 3);
				else if (m_PixelType.compare("int")==0) return ImageFileInfo::New<int>(filename, 3);
				else if (m_PixelType.compare("unsigned int")==0) return ImageFileInfo::New<unsigned int>(filename, 3);
				else if (m_PixelType.compare("long int")==0) return ImageFileInfo::New<long int>(filename, 3);
				else if (m_PixelType.compare("short")==0) return ImageFileInfo::New<short>(filename, 3);
				else if (m_PixelType.compare("unsigned short")==0) return ImageFileInfo::New<unsigned short>(filename, 3);
				else if (m_PixelType.compare("long")==0) return ImageFileInfo::New<long>(filename, 3);
				else if (m_PixelType.compare("unsigned long")==0) return ImageFileInfo::New<unsigned long>(filename, 3);
				else if (m_PixelType.compare("float")==0) return ImageFileInfo::New<float>(filename, 3);
				else if (m_PixelType.compare("double")==0) return ImageFileInfo::New<double>(filename, 3);

				std::stringstream ss;
				ss << "Invalid type \"" << m_PixelType << "\" encountered!";
				pcl_ThrowException(ImageReaderException(), ss.str());
			}

			template <class Type>
			void readImage(std::istream& is)
			{
				typedef GzRawImageReader<OutputImageType, Type> ReaderType;
				typename ReaderType::Pointer reader = ReaderType::New(is, m_Size);
				reader->read();
				this->m_OutputImage = reader->getOutputImage();
				this->m_OutputImage = this->m_OutputImage->getAlias(m_MinPoint, m_Spacing, m_Origin, m_Orientation);
			}

			char* readData(std::istream& is, char* buffer)
			{
				unsigned short data_size;
				is.read((char*)&data_size, sizeof(unsigned short));
				is.read(buffer, data_size);
				buffer[data_size] = '\0';
				return buffer;
			}
			template <class PointType>
			PointType getPointFromBuffer(const char* buffer)
			{
				PointType result;
				misc::StringTokenizer tokenizer(buffer);				
				int count = 0;
				for (tokenizer.begin(' '); !tokenizer.end(); tokenizer.next(' ')) {
					result[count] = boost::lexical_cast<typename PointType::ValueType>(tokenizer.getToken());
					count++;
				}
				return result;
			}
			template <class T>
			std::vector<T> getListFromBuffer(const char* buffer)
			{
				std::vector<T> result;
				misc::StringTokenizer tokenizer(buffer);
				for (tokenizer.begin(' '); !tokenizer.end(); tokenizer.next(' ')) {
					result.push_back(boost::lexical_cast<T>(tokenizer.getToken()));
				}
				return result;
			}
			void readMetaData(std::istream& is) 
			{
				m_Size.set(1,1,1);
				m_MinPoint.set(0,0,0);
				m_Origin.set(0,0,0);
				m_Spacing.set(1,1,1);
				m_Orientation.set_identity();
				int dimension = 3;

				bool type_found = false,
					size_found = false,
					minp_found = false,
					origin_found = false,
					spacing_found = false,
					dimension_found = false,
					orientation_found = false;
				m_ImageDataPos = -1;

				char buffer[1000];
				unsigned char tag_size;

				is.read(&buffer[0],3); buffer[3] = 0;
				if (strcmp(buffer, "HR2")!=0) {
					pcl_ThrowException(ImageReaderException(), "Cannot find HR2 header!");
				}

				m_Metadata = Metadata::New();
				try {
					auto exception_obj = pcl::StreamExceptionHelper::GetStreamExceptionObject(is, std::ios_base::failbit | std::ios_base::badbit);
					while (!is.eof()) {
						buffer[0] = 0;//replaced NULL with 0 //TODO PENDING TESTING
						is.read((char*)&tag_size, sizeof(unsigned char));
						is.read(&buffer[0], tag_size);
						buffer[tag_size] = 0;//replaced NULL with 0 //TODO PENDING TESTING
						if ((int)m_ImageDataPos==-1 && strcmp(buffer,"ImageData")==0) {
							unsigned int data_size;
							is.read((char*)(&data_size), sizeof(unsigned int));
							m_ImageDataPos = is.tellg();
							is.seekg(data_size, std::ios_base::cur);
							is.peek();
						} else {
							std::string key = buffer;
							readData(is, buffer);
							MetadataItem<std::string>::Pointer data = MetadataItem<std::string>::New(1, buffer);
							m_Metadata->add(key, data);
						}
					}
				} catch (const std::ios_base::failure &e) {
					pcl_ThrowException(ImageReaderException(), "Error encountered while reading! "+std::string(e.what()));
				}

				Metadata::Iterator end = m_Metadata->end();
				Metadata::Iterator iter;
				if ((iter = m_Metadata->find("PixelType"))!=end) {
					m_PixelType = iter->second->toString();
					type_found = true;
				}
				if ((iter = m_Metadata->find("Origin"))!=end) {
					m_Origin = getPointFromBuffer<Point3D<double> >(iter->second->toString().c_str());
					origin_found = true;
				}
				if ((iter = m_Metadata->find("Spacing"))!=end) {
					m_Spacing = getPointFromBuffer<Point3D<double> >(iter->second->toString().c_str());
					spacing_found = true;
				}
				if ((iter = m_Metadata->find("Dimension"))!=end) {
					dimension = atoi(iter->second->toString().c_str());
					dimension_found = true;
				}
				if ((iter = m_Metadata->find("MinPoint"))!=end) {
					m_MinPoint = getPointFromBuffer<Point3D<int> >(iter->second->toString().c_str());
					minp_found = true;
				}
				if ((iter = m_Metadata->find("Size"))!=end) {
					m_Size = getPointFromBuffer<Point3D<int> >(iter->second->toString().c_str());
					size_found = true;
				}
				if ((iter = m_Metadata->find("Orientation"))!=end) {
					auto val = getListFromBuffer<double>(iter->second->toString().c_str());
					if (val.size()!=9) {
						pcl_ThrowException(ImageReaderException(), "Invalid orientation matrix size!");
					}
					int count = 0;
					for (int c=0; c<3; ++c) for (int r=0; r<3; ++r) {
						m_Orientation(r,c) = val[count];
						++count;
					}
					orientation_found = true;
				}

				if (!type_found ) {
					pcl_ThrowException(ImageReaderException(), "Missing type info!");
				} else if (!size_found) {
					pcl_ThrowException(ImageReaderException(), "Missing size info!");
				}
			}
		};

	}
}

#pragma warning ( pop )
#endif