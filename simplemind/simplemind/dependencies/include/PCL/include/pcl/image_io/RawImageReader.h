#ifndef PCL_RAW_IMAGE_READER
#define PCL_RAW_IMAGE_READER

#include <pcl/image_io/ImageReader.h>

namespace pcl
{
	namespace image_io
	{

		template <class ImageType, class FileValueType=typename ImageType::IoValueType>
		class RawImageReader: public ImageReader<ImageType>
		{
		public:
			typedef RawImageReader Self;
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
				New(filename, size.x(), size.y(), size.z());
			}

			static Pointer New(std::ostream& os, int sx, int sy, int sz)
			{
				Pointer obj(new Self());
				obj->setInputStream(os);
				obj->m_Size.set(sx,sy,sz);
				return obj;
			}
			static Pointer New(std::ostream& os, const Point3D<int>& size)
			{
				New(os, size.x(), size.y(), size.z());
			}
			
			void setIteratorSequence(ImageIterator::Axis a, ImageIterator::Axis b, ImageIterator::Axis c)
			{
				m_IteratorAxis[0] = a;
				m_IteratorAxis[1] = b;
				m_IteratorAxis[2] = c;
			}

			virtual void read()
			{
				std::istream &is = *(this->m_InputStream);
				FileValueType buffer;
				int size = sizeof(FileValueType);

				this->m_OutputImage = OutputImageType::New(m_Size);

				ImageIterator iter(this->m_OutputImage, m_IteratorAxis[0], m_IteratorAxis[1], m_IteratorAxis[2]);
				pcl_ForIterator(iter) {
					if (!is.good()) {
						pcl_ThrowException(ImageReaderException(), "Error encountered while reading!");
					}
					is.read((char*)&buffer, size);
					this->m_OutputImage->set(iter, buffer);
				}
			}

		protected:
			Point3D<int> m_Size;
			ImageIterator::Axis m_IteratorAxis[3];

			RawImageReader() 
			{
				m_IteratorAxis[0] = ImageIterator::X;
				m_IteratorAxis[1] = ImageIterator::Y;
				m_IteratorAxis[2] = ImageIterator::Z;
			}
		};

	}
}

#endif