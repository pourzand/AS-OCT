#ifndef PCL_FILENAME_TOKENIZER
#define PCL_FILENAME_TOKENIZER

#include <pcl/os.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <ostream>

namespace pcl
{

	class FileNameTokenizer 
	{
	public:
#ifdef WINDOWS
		FileNameTokenizer(bool is_linux=false) 
#elif defined(LINUX)
		FileNameTokenizer(bool is_linux=true) 
#endif
		{ 
			m_IsLinux = is_linux;
		}

#ifdef WINDOWS
		FileNameTokenizer(const char* fname, bool is_linux=false) 
#elif defined(LINUX)
		FileNameTokenizer(const char* fname, bool is_linux=true) 
#endif
		{ 
			m_IsLinux = is_linux;
			setFile(fname); 
		}

#ifdef WINDOWS
		FileNameTokenizer(const std::string& fname, bool is_linux=false) 
#elif defined(LINUX)
		FileNameTokenizer(const std::string& fname, bool is_linux=true) 
#endif
		{ 
			m_IsLinux = is_linux;
			setFile(fname); 
		}

		void setFile(const std::string& fname) 
		{
			std::string temp;

			//Extracting path
			size_t path_pos;
			if (m_IsLinux) {
				path_pos = fname.find_last_of('/');
			} else {
				path_pos = fname.find_last_of("/\\");
			}
			if (path_pos!=std::string::npos) {
				m_Path = fname.substr(0, path_pos+1);
				temp = fname.substr(path_pos+1);
			} else {
				m_Path = "";
				temp = fname;
			}

			//Extracting filename and extension
			size_t ext_pos = temp.find_last_of('.');
			if (ext_pos!=std::string::npos) {
				m_FileName = temp.substr(0, ext_pos);
				m_Extension = temp.substr(ext_pos);
			} else {
				m_FileName = temp;
				m_Extension = "";
			}
		}

		void convertPathToForwardSlashes()
		{
			if (m_Path!="") {
				std::for_each(m_Path.begin(), m_Path.end(), [](char& val) {
					if (val=='/') val='\\';
				});
			}
		}

		void convertPathToBackwardSlashes()
		{
			if (m_Path!="") {
				std::for_each(m_Path.begin(), m_Path.end(), [](char& val) {
					if (val=='\\') val='/';
				});
			}
		}

		const std::string& getPath() const
		{
			return m_Path;
		}
		const std::string& getFileName() const
		{
			return m_FileName;
		}
		const std::string& getExtension() const
		{
			return m_Extension;
		}

		std::string getPathWithoutEndingSlash() const
		{
			if (m_Path=="") return m_Path;
			return std::move(m_Path.substr(0,m_Path.length()-1));
		}

		std::string getExtensionWithoutDot() 
		{
			if (m_Extension=="") return m_Extension;
			return std::move(m_Extension.substr(1));
		}

		operator std::string() const
		{
			return m_Path+m_FileName+m_Extension;
		}

		friend std::ostream& operator<<(std::ostream& os, const FileNameTokenizer& obj)
		{
			os << obj.m_Path << obj.m_FileName << obj.m_Extension;
			return os;
		}

	protected:
		std::string m_FileName;
		std::string m_Path;
		std::string m_Extension;
		bool m_IsLinux;
	};

}

#endif