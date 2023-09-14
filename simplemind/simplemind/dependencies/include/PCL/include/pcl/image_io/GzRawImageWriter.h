#ifndef PCL_GZ_RAW_IMAGE_WRITER
#define PCL_GZ_RAW_IMAGE_WRITER

#include <pcl/image_io/ImageWriter.h>
#include <pcl/misc/BufferedStreamWriter.h>
#include <pcl/misc/ZlibOstreamWriter.h>
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/filter/zlib.hpp>

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class GzRawImageWriter: public ImageWriter<ImageType>
		{
		public:
			typedef GzRawImageWriter Self;
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

			static Pointer New(std::ostream& os)
			{
				Pointer obj(new Self());
				obj->setOutputStream(os);
				return obj;
			}
			static Pointer New(std::ostream& os, const typename InputImageType::ConstantPointer& input, int compression_level=Z_DEFAULT_COMPRESSION)
			{
				Pointer obj = New(os);
				obj->setInputImage(input);
				obj->setCompressionLevel(compression_level);
				return obj;
			}

			void setCompressionLevel(int level) //0-9, where 9 is the best
			{
				m_CompressionLevel = level;
			}
			
			void setIteratorSequence(ImageIterator::Axis a, ImageIterator::Axis b, ImageIterator::Axis c)
			{
				m_IteratorAxis[0] = a;
				m_IteratorAxis[1] = b;
				m_IteratorAxis[2] = c;
			}

			/*virtual void write()
			{
				boost::iostreams::zlib_params p;
				p.level = m_CompressionLevel;
				//long buffer_size = (static_cast<long>(this->m_InputImage->getSize().x())*static_cast<long>(this->m_InputImage->getSize().y())*static_cast<long>(this->m_InputImage->getSize().z())*sizeof(FileValueType))/5;
				boost::iostreams::zlib_compressor compressor(p, 1024);
				boost::iostreams::filtering_streambuf<boost::iostreams::output> out;
				out.push(compressor);
				out.push(*(this->m_OutputStream));
				std::ostream os(&out);

				FileValueType buffer;
				int size = sizeof(FileValueType);

				ImageIterator iter(this->m_InputImage, m_IteratorAxis[0], m_IteratorAxis[1], m_IteratorAxis[2]);
				pcl_ForIterator(iter) {
					buffer = this->m_InputImage->get(iter);
					os.write((const char*)&buffer, size);
					if (!os.good()) {
						pcl_ThrowException(ImageWriterException(), "Error encountered while writing!");
					}
				}
			}*/

			virtual void write()
			{
				pcl::misc::BufferedStreamWriter<pcl::misc::ZlibOstreamWriter> writer(
					pcl::misc::ZlibOstreamWriter::New(*(this->m_OutputStream), m_CompressionLevel, 1024), 
					1024
					);
				ImageIterator iter(this->m_InputImage, m_IteratorAxis[0], m_IteratorAxis[1], m_IteratorAxis[2]);
				pcl_ForIterator(iter) {
					try {
						writer.add(this->m_InputImage->get(iter));
					} catch(pcl::Exception) {
						pcl_ThrowException(ImageWriterException(), "Error encountered while writing!");
					}
				}
			}

		protected:
			int m_CompressionLevel;
			ImageIterator::Axis m_IteratorAxis[3];

			GzRawImageWriter() 
			{
				m_CompressionLevel = Z_DEFAULT_COMPRESSION;
				m_IteratorAxis[0] = ImageIterator::X;
				m_IteratorAxis[1] = ImageIterator::Y;
				m_IteratorAxis[2] = ImageIterator::Z;
			}
		};

	}
}

#endif