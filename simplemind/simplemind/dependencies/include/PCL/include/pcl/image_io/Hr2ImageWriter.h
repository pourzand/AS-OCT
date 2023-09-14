#ifndef PCL_HR2_IMAGE_WRITER
#define PCL_HR2_IMAGE_WRITER

#include <pcl/image_io/ImageWriter.h>
#include <pcl/image_io/GzRawImageWriter.h>

#pragma warning ( push )
#pragma warning ( disable: 4101 )

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class Hr2ImageWriter: public ImageWriter<ImageType>
		{
		public:
			typedef Hr2ImageWriter Self;
			typedef ImageWriter<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::InputImageType InputImageType;
			typedef typename ImageType::IoValueType FileValueType;

			static void WriteImage(const std::string& filename, const typename InputImageType::ConstantPointer& input, int compression_level=Z_DEFAULT_COMPRESSION)
			{
				Pointer writer = New(filename, input, compression_level);
				writer->write();
			}

			static Pointer New(const std::string& filename)
			{
				Pointer obj(new Self());
				obj->setOutputStream(filename);
				return obj;
			}
			static Pointer New(const std::string& filename, const typename InputImageType::ConstantPointer& input, int compression_level=Z_DEFAULT_COMPRESSION)
			{
				Pointer obj = New(filename);
				obj->setInputImage(input);
				obj->setCompressionLevel(compression_level);
				return obj;
			}

			void setCompressionLevel(int level) //0-9, where 9 is the best
			{
				m_CompressionLevel = level;
			}

			virtual void write()
			{
				std::ostream &os = *(this->m_OutputStream);
				writeHeader(os);
				//Writing image data
				std::string buffer("ImageData");
				unsigned char tag_size = (unsigned char)buffer.length();
				os.write((char*)(&tag_size), sizeof(unsigned char));
				os.write(buffer.c_str(), tag_size);
				std::streampos image_data_size_pos = os.tellp();
				unsigned int image_data_size = 0;
				os.write((char*)(&image_data_size), sizeof(unsigned int)); //Writing a dummy value
				image_data_size = static_cast<unsigned int>(os.tellp());
				typename GzRawImageWriter<ImageType>::Pointer writer = GzRawImageWriter<ImageType>::New(os, this->m_InputImage, m_CompressionLevel);
				writer->write();
				image_data_size = static_cast<unsigned int>(os.tellp()) - image_data_size;
				os.seekp(image_data_size_pos, std::ios_base::beg);
				os.write((char*)(&image_data_size), sizeof(unsigned int)); //Writing actual value;
			}

		protected:
			int m_CompressionLevel;
			unsigned int m_ImageDataPos;

			Hr2ImageWriter() 
			{
				m_CompressionLevel = Z_DEFAULT_COMPRESSION;
			}

			void writeMetaDataItem(std::ostream& os, const char* tag, const char* buffer, size_t bsize)
			{
				unsigned short buffer_size = static_cast<unsigned short>(bsize);
				unsigned char tag_size = static_cast<unsigned char>(strlen(tag));
				os.write((char*)(&tag_size), sizeof(unsigned char));
				os.write(tag, tag_size);
				os.write((char*)(&buffer_size), sizeof(unsigned short));
				os.write(buffer, buffer_size);
			}
			void writeHeader(std::ostream& os) 
			{
				try {
					auto exception_obj = pcl::StreamExceptionHelper::GetStreamExceptionObject(os, std::ios_base::failbit | std::ios_base::badbit);
					std::stringstream ss;
					{
						char temp[] = "HR2";
						os.write(&temp[0], 3);
					}
					//Writing pixel type
					{
						std::string pixel_type;
						if (typeid(FileValueType)==typeid(char)) pixel_type = "char";
						else if (typeid(FileValueType)==typeid(unsigned char)) pixel_type = "unsigned char";
						else if (typeid(FileValueType)==typeid(int)) pixel_type = "int";
						else if (typeid(FileValueType)==typeid(unsigned int)) pixel_type = "unsigned int";
						else if (typeid(FileValueType)==typeid(long int)) pixel_type = "long int";
						else if (typeid(FileValueType)==typeid(short)) pixel_type = "short";
						else if (typeid(FileValueType)==typeid(unsigned short)) pixel_type = "unsigned short";
						else if (typeid(FileValueType)==typeid(float)) pixel_type = "float";
						else if (typeid(FileValueType)==typeid(double)) pixel_type = "double";
						else {
							pcl_ThrowException(ImageWriterException(), "Invalid file type encountered!");
						}
						writeMetaDataItem(os, "PixelType", pixel_type.c_str(), pixel_type.length());
					}
					//Writing dimension
					{
						ss << 3;
						writeMetaDataItem(os, "Dimension", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing size
					{
						const Point3D<int> &size = this->m_InputImage->getSize();
						ss << size.x() << " " << size.y() << " " << size.z();
						writeMetaDataItem(os, "Size", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing origin
					{
						const Point3D<double> &origin = this->m_InputImage->getOrigin();
						ss << origin.x() << " " << origin.y() << " " << origin.z();
						writeMetaDataItem(os, "Origin", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing spacing
					{
						const Point3D<double> &spacing = this->m_InputImage->getSpacing();
						ss << spacing.x() << " " << spacing.y() << " " << spacing.z();
						writeMetaDataItem(os, "Spacing", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing orientation matrix (only supported by pcl)
					{
						auto matrix = this->m_InputImage->getOrientationMatrix();
						bool init = true;
						for (int c=0; c<3; ++c) for (int r=0; r<3; ++r) {
							if (init) init = false;
							else ss << " ";
							ss << matrix(r,c);
						}
						writeMetaDataItem(os, "Orientation", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing min point (only supported by pcl)
					{
						const Point3D<int> &minp = this->m_InputImage->getMinPoint();
						ss << minp.x() << " " << minp.y() << " " << minp.z();
						writeMetaDataItem(os, "MinPoint", ss.str().c_str(), ss.str().length());
						ss.str("");
					}
					//Writing compression
					{
						std::string str("ZLib");
						writeMetaDataItem(os, "Compression", str.c_str(), str.length());
					}

					//Writing metadata
					if (this->m_InputImage->getMetadata()) {
						std::string critical_tag[] = {"PixelType", "Dimension", "Size", "Origin", "Spacing", "Compression"};
						pcl_ForEach(*(this->m_InputImage->getMetadata()), item) {
							bool is_critical = false;
							for (int i=0; i<6; i++) {
								if (item->first==critical_tag[i]) {
									is_critical = true;
									break;
								}
							}
							if (is_critical) continue; //Skipping critical tags

							std::string data = item->second->toString();
							writeMetaDataItem(os, item->first.c_str(), data.c_str(), data.length());
						}
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(ImageWriterException(), "Error encountered while writing! "+std::string(e.what()));
				}
			}
		};

	}
}

#pragma warning ( pop )
#endif