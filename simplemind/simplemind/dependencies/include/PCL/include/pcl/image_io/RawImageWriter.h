#ifndef PCL_RAW_IMAGE_WRITER
#define PCL_RAW_IMAGE_WRITER

#include <pcl/image_io/ImageWriter.h>

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class RawImageWriter: public ImageWriter<ImageType>
		{
		public:
			typedef RawImageWriter Self;
			typedef ImageWriter<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::InputImageType InputImageType;
			typedef typename ImageType::IoValueType FileValueType;

			static void WriteImage(const std::string& filename, const typename InputImageType::ConstantPointer& input)
			{
				Pointer writer = New(filename, input);
				writer->write();
			}

			static Pointer New(const std::string& filename)
			{
				Pointer obj(new RawImageWriter());
				obj->setOutputStream(filename);
				return obj;
			}
			static Pointer New(const std::string& filename, const typename InputImageType::ConstantPointer& input)
			{
				Pointer obj = New(filename);
				obj->setInputImage(input);
				return obj;
			}

			static Pointer New(std::ostream& os)
			{
				Pointer obj(new Self());
				obj->setOutputStream(os);
				return obj;
			}
			static Pointer New(std::ostream& os, const typename InputImageType::ConstantPointer& input)
			{
				Pointer obj = New(os);
				obj->setInpuImage(input);
			}
			
			void setIteratorSequence(ImageIterator::Axis a, ImageIterator::Axis b, ImageIterator::Axis c)
			{
				m_IteratorAxis[0] = a;
				m_IteratorAxis[1] = b;
				m_IteratorAxis[2] = c;
			}

			virtual void write()
			{
				std::ostream &os = *(this->m_OutputStream);
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
			}

		protected:
			ImageIterator::Axis m_IteratorAxis[3];
			
			RawImageWriter() 
			{
				m_IteratorAxis[0] = ImageIterator::X;
				m_IteratorAxis[1] = ImageIterator::Y;
				m_IteratorAxis[2] = ImageIterator::Z;
			}
		};

	}
}

#endif