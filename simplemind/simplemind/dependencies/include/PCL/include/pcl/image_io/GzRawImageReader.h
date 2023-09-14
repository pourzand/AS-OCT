#ifndef PCL_GZ_RAW_IMAGE_READER
#define PCL_GZ_RAW_IMAGE_READER

#include <pcl/image_io/ImageReader.h>
#include <pcl/misc/BufferedStreamReader.h>
#include <pcl/misc/ZlibIstreamReader.h>
//#include <boost/iostreams/filtering_streambuf.hpp>
//#include <boost/iostreams/filter/zlib.hpp>

namespace pcl
{
	namespace image_io
	{

		template <class ImageType, class FileValueType=typename ImageType::IoValueType>
		class GzRawImageReader: public ImageReader<ImageType>
		{
		public:
			typedef GzRawImageReader Self;
			typedef ImageReader<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::OutputImageType OutputImageType;

			static typename OutputImageType::Pointer ReadImage(const std::string& filename, int sx, int sy, int sz)
			{
				Pointer reader = New(filename, sx, sy, sz);
				typename OutputImageType::Pointer result;
				reader->read();
				result = reader->getOutputImage();
				return result;
			}
			static typename OutputImageType::Pointer ReadImage(const std::string& filename, const Point3D<int>& size)
			{
				return ReadImage(filename, size.x(), size.y(), size.z());
			}

			static Pointer New(const std::string& filename, int sx, int sy, int sz)
			{
				Pointer obj(new Self());
				obj->setInputStream(filename);
				obj->m_Size.set(sx,sy,sz);
				return obj;
			}
			static Pointer New(const std::string& filename, const Point3D<int>& size)
			{
				return New(filename, size.x(), size.y(), size.z());
			}

			static Pointer New(std::istream& is, int sx, int sy, int sz)
			{
				Pointer obj(new Self());
				obj->setInputStream(is);
				obj->m_Size.set(sx,sy,sz);
				return obj;
			}
			static Pointer New(std::istream& is, const Point3D<int>& size)
			{
				return New(is, size.x(), size.y(), size.z());
			}
			
			void setIteratorSequence(ImageIterator::Axis a, ImageIterator::Axis b, ImageIterator::Axis c)
			{
				m_IteratorAxis[0] = a;
				m_IteratorAxis[1] = b;
				m_IteratorAxis[2] = c;
			}

			/*virtual void read()
			{
				boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
				//long buffer_size = (static_cast<long>(m_Size.x())*static_cast<long>(m_Size.y())*static_cast<long>(m_Size.z())*sizeof(FileValueType));
				in.push(boost::iostreams::zlib_decompressor(boost::iostreams::zlib::default_window_bits, 1024));
				in.push(*(this->m_InputStream));
				std::istream is(&in);
				pcl::misc::IstreamReader stream_reader(is);
				pcl::misc::BufferedStreamReader<pcl::misc::IstreamReader> reader(stream_reader, 1024);

				//FileValueType buffer;
				//int size = sizeof(FileValueType);

				this->m_OutputImage = OutputImageType::New(m_Size);
				ImageIterator iter(this->m_OutputImage, m_IteratorAxis[0], m_IteratorAxis[1], m_IteratorAxis[2]);
				pcl_ForIterator(iter) {
					const FileValueType *ptr = reader.get<FileValueType>();
					if (ptr==NULL) pcl_ThrowException(ImageReaderException(), "Error encountered while reading!");
					this->m_OutputImage->set(iter, *ptr);
				}
			}*/

			virtual void read()
			{
				pcl::misc::BufferedStreamReader<pcl::misc::ZlibIstreamReader> reader(
					pcl::misc::ZlibIstreamReader::New(*(this->m_InputStream), 1024), 
					1024
					);

				this->m_OutputImage = OutputImageType::New(m_Size);
				ImageIterator iter(this->m_OutputImage, m_IteratorAxis[0], m_IteratorAxis[1], m_IteratorAxis[2]);
				pcl_ForIterator(iter) {
					const FileValueType *ptr = reader.get<FileValueType>();
					if (ptr==NULL) pcl_ThrowException(ImageReaderException(), "Error encountered while reading!");
					this->m_OutputImage->set(iter, *ptr);
				}
			}

		protected:
			Point3D<int> m_Size;
			ImageIterator::Axis m_IteratorAxis[3];

			GzRawImageReader() 
			{
				m_IteratorAxis[0] = ImageIterator::X;
				m_IteratorAxis[1] = ImageIterator::Y;
				m_IteratorAxis[2] = ImageIterator::Z;
			}
		};

	}
}

#endif