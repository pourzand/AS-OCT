#ifndef PCL_FEATURE_IO_HELPER
#define PCL_FEATURE_IO_HELPER

#include <vector>
#include <ostream>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>
#include <pcl/macro.h>
#include <pcl/misc/CsvTools.h>
#include <pcl/type_utility.h>

#define FEATURE_IO_DEFAULT_PRECISION 20

namespace pcl
{

	class FeatureIoHelper
	{
	public:
		//**************************************** Misc
		static void WriteHeader(std::ostream& os, bool use_binary)
		{
			if (use_binary) os << "BIN\n";
			else os << "TXT\n";
		}

		static bool IsBinary(std::istream& is)
		{
			char header[5];
			is.read(header, 4); header[4] = NULL;
			if (strcmp(header, "TXT\n")==0) return false;
			else if (strcmp(header, "BIN\n")==0) return true;
			pcl_ThrowException(pcl::Exception(), "Invalid header encountered!");
		}

		template <class T>
		static void Write(std::ostream& os, bool use_binary, const T& data, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			if (use_binary) BinWrite(os, data);
			else TxtWrite(os, data, precision);
		}

		template <class T>
		static void Read(std::istream& is, bool use_binary, T& data)
		{
			if (use_binary) BinRead(is, data);
			else TxtRead(is, data);
		}

		//**************************************** Text element write operations
		template <class T>
		static typename boost::enable_if<boost::is_float<T>>::type TxtWriteElement(std::ostream& os, const T& elem, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			os << std::setprecision(precision) << elem;
		}

		template <class T>
		static typename boost::enable_if<boost::is_integral<T>>::type TxtWriteElement(std::ostream& os, const T& elem, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			os << static_cast<long>(elem);
		}

		static void TxtWriteElement(std::ostream& os, const std::string& elem, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			if (elem=="#")  os << "\"#\"";
			else os << pcl::misc::CsvHelper::GetCsvValidString(elem);
		}

		//**************************************** Text data write operations
		template<class T, class A>
		static void TxtWrite(std::ostream& os, const std::vector<T,A>& data, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			bool init = true;
			pcl_ForEach(data, item) {
				if (!init) os << ",";
				TxtWriteElement(os, *item, precision);
				init = false;
			}
			os << std::endl;
		}

		template <class T, class A>
		static typename void TxtWrite(std::ostream& os, const std::vector<std::vector<T,A>>& data, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			pcl_ForEach(data, item) TxtWrite(os, *item, precision);
			os << "#" << std::endl;
		}

		template<class K, class M, class H, class P, class A>
		static void TxtWrite(std::ostream& os, const boost::unordered_map<K,M,H,P,A>& data, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			pcl_ForEach(data, item) {
				TxtWriteElement(os, item->first);
				os << ",";
				TxtWriteElement(os, item->second);
				os << std::endl;
			}
			os << "#" << std::endl;
		}

		//This function is only meant for writing in text mode!
		template<class K, class M, class H, class P, class A>
		static void TxtWrite(std::ostream& os, const std::vector<boost::unordered_map<K,M,H,P,A>>& data, const std::vector<K>& keys=std::vector<K>(), int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			if (!data.empty()) {
				std::vector temp_key<K>;
				const std::vector *keys_ptr;
				if (keys.empty()) {
					keys_ptr = &temp_key;
					pcl_ForEach(data[0], item) temp_key.push_back(item->first);
				} else keys_ptr = &keys;
				{//Writing header
					bool init = true;
					pcl_ForEach(data[0], item) {
						if (!init) os << ",";
						TxtWriteElement(os, item->first, precision);
						init = false;
					}
					os << std::endl;
				}
				pcl_ForEach(data, item) {
					bool init = true;
					pcl_ForEach(*key_ptr, k) {
						if (!init) os << ",";
						TxtWriteElement(os, item->at(*k), precision);
						init = false;
					}
					os << std::endl;
				}
			}
			os << "#" << std::endl;
		}

		//**************************************** Text data read operations
		template <class T>
		static T GetValueFromTxt(const std::string& val)
		{
			return boost::lexical_cast<T>(val);
		}

		template <>
		static char GetValueFromTxt<char>(const std::string& val)
		{
			return static_cast<char>(boost::lexical_cast<short>(val));
		}

		template <>
		static unsigned char GetValueFromTxt<unsigned char>(const std::string& val)
		{
			return static_cast<char>(boost::lexical_cast<short>(val));
		}

		template<class T, class A>
		static void TxtRead(std::istream& is, std::vector<T,A>& data)
		{
			pcl::misc::CsvReader reader(is);
			reader.begin();
			pcl_ForEach(reader.getRow(), item) {
				data.push_back(GetValueFromTxt<T>(*item));
			}
		}

		template <class T, class A>
		static typename void TxtRead(std::istream& is, std::vector<std::vector<T,A>>& data)
		{
			pcl::misc::CsvReader reader(is);
			for (reader.begin(); !reader.end(); reader.next()) {
				if (reader.get()=="#") break;
				std::vector<T,A> entry;
				std::cout << reader.get() << std::endl;
				pcl_ForEach(reader.getRow(), item) {
					entry.push_back(GetValueFromTxt<T>(*item));
				}
				data.push_back(entry);
			}
		}

		template<class K, class M, class H, class P, class A>
		static void TxtRead(std::istream& is, boost::unordered_map<K,M,H,P,A>& data)
		{
			pcl::misc::CsvReader reader(is);
			for (reader.begin(); !reader.end(); reader.next()) {
				if (reader.get()=="#") break;
				data[GetValueFromTxt<K>(reader.getRow()[0])] = GetValueFromTxt<M>(reader.getRow()[1]);
			}
		}

		template<class K, class M, class H, class P, class A>
		static void TxtRead(std::istream& is, std::vector<boost::unordered_map<K,M,H,P,A>>& data)
		{
			pcl::misc::CsvMapReader reader(is);
			for (reader.begin(); !reader.end(); reader.next()) {
				if (reader.get()=="#") break;
				data.push_back(reader.getRow());
			}
		}

		//**************************************** Binary element write operations
		template <class T>
		static void BinWriteElement(std::ostream& os, const T& elem)
		{
			os.write((const char*)&elem, sizeof(T));
		}

		static void BinWriteElement(std::ostream& os, const std::string& elem)
		{
			BinWriteElement(os, elem.size());
			for (size_t i=0; i<elem.size(); ++i) os.write(&(elem[i]), 1);
		}

		//**************************************** Binary data write operations
		template<class T, class A>
		static void BinWrite(std::ostream& os, const std::vector<T,A>& data)
		{
			BinWriteElement(os, data.size());
			pcl_ForEach(data, item) BinWriteElement(os, *item);
		}

		template <class T, class A>
		static typename void BinWrite(std::ostream& os, const std::vector<std::vector<T,A>>& data)
		{
			BinWriteElement(os, data.size());
			pcl_ForEach(data, item) BinWrite(os, *item);
		}

		template<class K, class M, class H, class P, class A>
		static void BinWrite(std::ostream& os, const boost::unordered_map<K,M,H,P,A>& data)
		{
			BinWriteElement(os, data.size());
			pcl_ForEach(data, item) {
				BinWriteElement(os, item->first);
				BinWriteElement(os, item->second);
			}
		}

		//**************************************** Binary element read operations
		template <class T>
		static T BinReadElement(std::istream& is)
		{
			T val;
			is.read((char*)&val, sizeof(T));
			return val;
		}

		template <>
		static std::string BinReadElement<std::string>(std::istream& is)
		{
			size_t size = BinReadElement<size_t>(is);
			std::string buffer;
			char b;
			for (int i=0; i<size; ++i) {
				is.read(&b, 1);
				buffer.push_back(b);
			}
			return std::move(buffer);
		}

		//**************************************** Binary data read operations
		template<class T, class A>
		static void BinRead(std::istream& is, std::vector<T,A>& data)
		{
			size_t size = BinReadElement<size_t>(is);
			data.reserve(data.size()+size);
			for (size_t i=0; i<size; ++i) {
				data.push_back(BinReadElement<T>(is));
			}
		}

		template <class T, class A>
		static typename void BinRead(std::istream& is, std::vector<std::vector<T,A>>& data)
		{
			size_t size = BinReadElement<size_t>(is);
			data.reserve(data.size()+size);
			for (size_t i=0; i<size; ++i) {
				std::vector<T,A> val;
				BinRead(is, val);
				data.push_back(std::move(val));
			}
		}

		template<class K, class M, class H, class P, class A>
		static void BinRead(std::istream& is, boost::unordered_map<K,M,H,P,A>& data)
		{
			size_t size = BinReadElement<size_t>(is);
			for (size_t i=0; i<size; ++i) {
				data[BinReadElement<K>(is)] = BinReadElement<M>(is);
			}
		}

	};


	class FeatureWriter
	{
	public:
		FeatureWriter(std::ostream& os, bool use_binary, bool write_header=true, int precision=FEATURE_IO_DEFAULT_PRECISION)
		{
			if (write_header) FeatureIoHelper::WriteHeader(os, use_binary);
			m_Os = &os;
			m_UseBinary = use_binary;
			m_Precision = precision;
		}

		template <class T>
		void write(const T& data)
		{
			FeatureIoHelper::Write(*m_Os, m_UseBinary, data, m_Precision);
		}
		
	protected:
		std::ostream *m_Os;
		bool m_UseBinary;
		int m_Precision;
	};


	class FeatureReader
	{
	public:
		FeatureReader(std::istream &is)
		{
			m_UseBinary = FeatureIoHelper::IsBinary(is);
			m_Is = &is;
		}

		FeatureReader(std::istream &is, bool use_binary)
		{
			m_UseBinary = use_binary;
			m_Is = &is;
		}

		template <class T>
		void read(T& data)
		{
			FeatureIoHelper::Read(*m_Is, m_UseBinary, data);
		}
	protected:
		std::istream *m_Is;
		bool m_UseBinary;
	};
	
}

#endif