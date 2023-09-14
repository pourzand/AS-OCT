#ifndef PCL_ITK_IMAGE_WRITER
#define PCL_ITK_IMAGE_WRITER

#include <pcl/image_io/ImageWriter.h>
#include "itkImage.h"
#include "itkImageFileWriter.h"

namespace pcl
{
	namespace image_io
	{

		template <class ImageType>
		class ItkImageWriter: public ImageWriter<ImageType>
		{
		public:
			typedef ItkImageWriter Self;
			typedef ImageWriter<ImageType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename Parent::InputImageType InputImageType;

			static void WriteImage(const std::string& filename, const typename InputImageType::ConstantPointer& input, bool use_compression=false)
			{
				Pointer writer = New(filename, input);
				writer->useCompression(use_compression);
				writer->write();
			}

			static Pointer New(const std::string& filename)
			{
				Pointer obj(new Self());
				obj->m_FileName = filename;
				return obj;
			}
			static Pointer New(const std::string& filename, const typename InputImageType::ConstantPointer& input)
			{
				Pointer obj = New(filename);
				obj->setInputImage(input);
				return obj;
			}

			void useCompression(bool en)
			{
				m_UseCompression = en;
			}

			virtual void write()
			{
				auto result = ImageHelper::GetItkImage(this->m_InputImage);
				writeItkImage(result);
			}

		protected:
			std::string m_FileName;
			bool m_UseCompression;

			ItkImageWriter() {
				RegisterImageIoFactory();
			}

			template <class ItkImagePointerType>
			void writeItkImage(ItkImagePointerType& itk_image)
			{
				typedef  itk::ImageFileWriter<typename ItkImagePointerType::ObjectType> WriterType;
				typename WriterType::Pointer writer = WriterType::New();
				writer->SetFileName(m_FileName);
				writer->SetUseCompression(m_UseCompression);
				writer->SetInput(itk_image);
				writer->UseInputMetaDataDictionaryOn();
				try {
					writer->Update();
				} catch (itk::ExceptionObject & e) {
					pcl_ThrowException(ImageWriterException(), e.what());
				}
			}
		};

	}
}

#endif