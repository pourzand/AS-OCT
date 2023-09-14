#ifndef PCL_IMAGE_READER
#define PCL_IMAGE_READER

#include <pcl/image.h>
#include <pcl/exception.h>
#include <pcl/misc/FileStreamHelper.h>
#include <pcl/image_io/ImageFileInfo.h>

#include <istream>
#include <string>
#include <boost/smart_ptr.hpp>
#include <memory>

namespace pcl
{
	namespace image_io
	{

		struct ImageReaderException: public pcl::Exception {};

		template <class ImageType>
		class ImageReader: private boost::noncopyable
		{
		public:
			typedef boost::shared_ptr<ImageReader> Pointer;
			typedef ImageType OutputImageType;

			~ImageReader()
			{
				close();
			}

			virtual void read() = 0;

			typename OutputImageType::Pointer getOutputImage()
			{
				return m_OutputImage;
			}

			void close()
			{
				if (m_FileStream.get()!=NULL) m_FileStream->close();
			}

		protected:
			std::istream *m_InputStream;
			boost::scoped_ptr<std::ifstream> m_FileStream;
			typename OutputImageType::Pointer m_OutputImage;

			ImageReader() {}

			void setInputStream(std::istream& is) 
			{
				m_InputStream = &is;
			}

			void setInputStream(const std::string& file)
			{
				m_FileStream.reset(new std::ifstream(file.c_str(), std::iostream::binary));
				if (m_FileStream->fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for input");
				setInputStream(*m_FileStream);
			}
		};

	}
}

#endif