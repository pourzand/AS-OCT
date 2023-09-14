#ifndef PCL_IMAGE_WRITER
#define PCL_IMAGE_WRITER

#include <pcl/image.h>
#include <pcl/exception.h>

#include <ostream>
#include <string>
#include <boost/smart_ptr.hpp>
#include <fstream>
#include <memory>

namespace pcl
{
	namespace image_io
	{

		struct ImageWriterException: public pcl::Exception {};

		template <class ImageType>
		class ImageWriter: private boost::noncopyable
		{
		public:
			typedef boost::shared_ptr<ImageWriter> Pointer;
			typedef ImageType InputImageType;

			~ImageWriter()
			{
				close();
			}

			void setInputImage(const typename InputImageType::ConstantPointer& image) 
			{
				m_InputImage = image;
			}

			virtual void write() = 0;

			void close()
			{
				if (m_FileStream.get()!=NULL) m_FileStream->close();
			}

		protected:
			std::ostream *m_OutputStream;
			boost::scoped_ptr<std::ofstream> m_FileStream;
			typename InputImageType::ConstantPointer m_InputImage;

			ImageWriter() {}

			void setOutputStream(std::ostream& os) 
			{
				m_OutputStream = &os;
			}

			void setOutputStream(const std::string& file)
			{
				m_FileStream.reset(new std::ofstream(file.c_str(), std::iostream::binary));
				if (m_FileStream->fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for output");
				setOutputStream(*m_FileStream);
			}
		};

	}
}

#endif