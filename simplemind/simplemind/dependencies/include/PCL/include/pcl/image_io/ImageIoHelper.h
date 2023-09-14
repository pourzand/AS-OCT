#ifndef PCL_IMAGE_IO_HELPER
#define PCL_IMAGE_IO_HELPER

#include <pcl/image/DummyImage.h>
#include <pcl/image_io.h>
#include <pcl/exception.h>
#include <pcl/image_io/ImageFileInfo.h>
#include <pcl/image_io/DicomImageSeriesReader.h>
#include <pcl/misc/FileNameTokenizer.h>
#include <iostream>
#include <ctype.h>

namespace pcl
{
	
	template <class T=void>
	class ImageIoHelperBase
	{
	public:
		static bool ItkUseCompression;
		static int Hr2CompressionLevel;

		static ImageFileInfo GetImageFileInfo(const std::string& filename)
		{
			std::string extension = FileNameTokenizer(filename).getExtensionWithoutDot();
			for (int i=0; i<extension.size(); i++) extension[i] = tolower(extension[i]);
			try {
				if (extension.compare("hr2")==0) {
					return image_io::Hr2ImageReader<Image<char>>::ReadImageInformation(filename);
				} else {
#ifndef NO_ITK
					if (boost::filesystem::is_directory(filename)) {
						std::string file;
						boost_file::directory_iterator end_itr;
						for (boost_file::directory_iterator itr(filename); itr!=end_itr; itr++) {
							if (!boost_file::is_directory(itr->status())) {
								file = itr->path().string();
								break;
							}
						}
						return image_io::ItkImageReader<Image<char>>::ReadImageInformation(file);
					} else if (extension.compare("ser")==0 || extension.compare("seri")==0 || extension.compare("sers")==0) {
						std::string file = getFirstFileInList(filename);
						return image_io::ItkImageReader<Image<char>>::ReadImageInformation(file);
					} else if (extension.compare("dummy")==0) {
						return ImageFileInfo(typeid(char), 3, 1, "dummy");
					} else {
						return image_io::ItkImageReader<Image<char>>::ReadImageInformation(filename);
					}
#else
					pcl_ThrowException(pcl::Exception(), "Cannot file suitable reader for "+filename);
#endif
				}
			} catch (const pcl::Exception& e) {
#ifndef NO_WARNING
				std::cout << "Exception occured while reading image information from file " << filename << std::endl;
#endif
				boost::throw_exception(e);
			}
		}
		
		static DummyImage::Pointer ReadDummy(const std::string& filename, bool ignore_duplicates=false)
		{
			std::string extension = FileNameTokenizer(filename).getExtensionWithoutDot();
			for (int i=0; i<extension.size(); i++) extension[i] = tolower(extension[i]);
			if (extension.compare("dummy")==0) return DummyImage::New(filename);
			else {
				auto image = Read<pcl::Image<char,true>>(filename, ignore_duplicates);
				return DummyImage::New(image);
			}
		}

		template <class ImagePointerType>
		static void WriteDummy(const std::string& filename, const ImagePointerType& image)
		{
			auto dummy = DummyImage::New(image);
			dummy->write(filename);
		}

		template <class ImageType>
		static typename ImageType::Pointer Read(const std::string& filename, bool ignore_duplicates=false)
		{
			std::string extension = FileNameTokenizer(filename).getExtensionWithoutDot();
			for (int i=0; i<extension.size(); i++) extension[i] = tolower(extension[i]);
			
			typename ImageType::Pointer result;
			try {
				if (extension.compare("hr2")==0) {
					result = image_io::Hr2ImageReader<ImageType>::ReadImage(filename);
				} else {
#ifndef NO_ITK
					if (boost::filesystem::is_directory(filename)) {
						typename pcl::DicomImageSeriesReader<ImageType>::Pointer reader = pcl::DicomImageSeriesReader<ImageType>::New();
						reader->addFilesInDirectory(filename);
						reader->read(pcl::DicomImageSeriesReader<ImageType>::POSITION, ignore_duplicates);
						result = reader->getOutputImage();
					} else if (extension.compare("ser")==0) {
						result = ReadDicomInText<ImageType>(filename, pcl::DicomImageSeriesReader<ImageType>::POSITION, ignore_duplicates);
					} else if (extension.compare("seri")==0) {
						result = ReadDicomInText<ImageType>(filename, pcl::DicomImageSeriesReader<ImageType>::INSTANCE, ignore_duplicates);
					} else if (extension.compare("sers")==0) {
						result = ReadDicomInText<ImageType>(filename, pcl::DicomImageSeriesReader<ImageType>::ADDSEQ, ignore_duplicates);
					} else if (extension.compare("dummy")==0) {
						auto dummy = DummyImage::New(filename);
						result = ImageType::New(dummy);
						pcl::ImageHelper::Fill(result, 0);
					} else {
						result = image_io::ItkImageReader<ImageType>::ReadImage(filename);
					}
#else
					pcl_ThrowException(pcl::Exception(), "Cannot file suitable reader for "+filename);
#endif
				} 
			} catch (const pcl::Exception& e) {
#ifndef NO_WARNING
				std::cout << "Exception occured while reading file " << filename << std::endl;
#endif
				boost::throw_exception(e);
			}
			return result;
		}


		template <class ImagePointerType>
		static void Write(const std::string& filename, const ImagePointerType& img)
		{
			std::string extension = FileNameTokenizer(filename).getExtensionWithoutDot();
			for (int i=0; i<extension.size(); i++) extension[i] = tolower(extension[i]);
			try {
				if (extension.compare("hr2")==0) {
					image_io::Hr2ImageWriter<typename ImagePointerType::element_type>::WriteImage(filename, img, Hr2CompressionLevel);
				} else if (extension.compare("dummy")==0) {
					WriteDummy(filename, img);
				} else {
#ifndef NO_ITK
					image_io::ItkImageWriter<typename ImagePointerType::element_type>::WriteImage(filename, img, ItkUseCompression);
#else
					pcl_ThrowException(pcl::Exception(), "Cannot file suitable writer for "+filename);
#endif
				}
			} catch (const pcl::Exception& e) {
#ifndef NO_WARNING
				std::cout << "Exception occured while writing file " << filename << std::endl;
#endif
				boost::throw_exception(e);
			}
		}

		template <class ImageType>
		static typename ImageType::Pointer ReadDicomInText(const std::string& list_file, typename pcl::DicomImageSeriesReader<ImageType>::SortType sort_type, bool ignore_duplicates=false)
		{
			std::ifstream is(list_file.c_str(), std::ifstream::in);
			if (is.fail()) pcl_ThrowException(Exception(), "Failed at opening "+list_file);
			std::string base_path = boost::filesystem::absolute(list_file).parent_path().string();
			typename pcl::DicomImageSeriesReader<ImageType>::Pointer reader = pcl::DicomImageSeriesReader<ImageType>::New();
			bool occupied = false;
			while (!is.eof()) {
				std::string str_buffer;
				std::getline(is, str_buffer, '\n');
				if (is.fail() && !is.eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				boost::trim(str_buffer);
				if (!str_buffer.empty()) {
					reader->addFile(boost::filesystem::absolute(str_buffer, base_path).string());
					occupied = true;
				}
			}
			is.close();
			if (!occupied) pcl_ThrowException(Exception(), "List file empty");
			reader->read(sort_type, ignore_duplicates);
			return reader->getOutputImage();
		}

		static std::string getFirstFileInList(const std::string& list_file)
		{
			std::string base_path = boost::filesystem::absolute(list_file).parent_path().string();
			std::string result;
			std::ifstream is(list_file.c_str(), std::ifstream::in);
			if (is.fail()) pcl_ThrowException(Exception(), "Failed at opening "+list_file);
			while (!is.eof()) {
				std::string str_buffer;
				std::getline(is, str_buffer, '\n');
				if (is.fail() && !is.eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				boost::trim(str_buffer);
				if (!str_buffer.empty()) {
					result = str_buffer;
					break;
				}
			}
			is.close();
			if (result.empty()) pcl_ThrowException(Exception(), "List file empty");
			return boost::filesystem::absolute(result, base_path).string();
		}
	};
	
	typedef ImageIoHelperBase<> ImageIoHelper; //This is to avoid error in compilation of the static members
	
	template <> bool ImageIoHelperBase<void>::ItkUseCompression = false;
	template <> int ImageIoHelperBase<void>::Hr2CompressionLevel = Z_DEFAULT_COMPRESSION;

}

#endif