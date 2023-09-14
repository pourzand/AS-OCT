#ifndef PCL_CSV_READER
#define PCL_CSV_READER

#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_map.hpp>
#include <boost/smart_ptr.hpp>

#include <pcl/exception.h>
#include <pcl/misc/StringTokenizer.h>
#include <pcl/misc/IoTools.h>

namespace pcl
{
	namespace misc
	{

		class CsvHelper
		{
		public:
			static std::string GetCsvValidString(const std::string& str, char delim=',')
			{
				if (str.find(delim)==std::string::npos && str.find('\"')==std::string::npos) return str;
				std::string result;
				for (int i=0; i<str.size(); ++i) {
					if (str[i]=='\"') {
						result.push_back('\"');
						result.push_back('\"');
					} else result.push_back(str[i]);
				}
				return std::move("\""+result+"\"");
			}
		};


		class CsvReader
		{
		public:
			CsvReader(std::istream& is, char delim=',')
			{
				m_Delimiter = delim;
				m_LineIterator.reset(new IsLineIterator(is));
			}
			
			CsvReader(const std::string& file, char delim=',')
			{
				m_Delimiter = delim;
				m_LineIterator.reset(new IsLineIterator(file));
			}

			void begin()
			{
				m_Row.clear();
				for (m_LineIterator->begin(); !m_LineIterator->end(); m_LineIterator->next()) {
					boost::trim_copy(m_LineIterator->get());
					if (!boost::trim_copy(m_LineIterator->get()).empty()) {
						break;
					}
				}
			}

			void next()
			{
				if (end()) return;
				m_Row.clear();
				m_LineIterator->next();
				for (; !m_LineIterator->end(); m_LineIterator->next()) {
					boost::trim_copy(m_LineIterator->get());
					if (!boost::trim_copy(m_LineIterator->get()).empty()) {
						break;
					}
				}
			}

			bool end() const
			{
				return m_LineIterator->end();
			}

			const std::vector<std::string>& getRow() const
			{
				if (m_Row.empty() && !end()) readRow();
				return m_Row;
			}
			
			const std::string get() const
			{
				return m_LineIterator->get();
			}

		protected:
			char m_Delimiter;
			boost::scoped_ptr<IsLineIterator> m_LineIterator;
			mutable std::vector<std::string> m_Row;
			
			void readRow() const
			{
				std::string token;
				bool ignore_delim = false;
				std::string &buffer = m_LineIterator->get();
				for (int i=0; i<buffer.size(); ++i) {
					if (buffer[i]=='\r' || buffer[i]=='\n') break;
					if (!ignore_delim && buffer[i]==m_Delimiter) {
						m_Row.push_back(token);
						token.clear();
					} else {
						if (buffer[i]=='\"') {
							if ((i+1)<buffer.size() && buffer[i+1]=='\"') {
								token.push_back('\"');
								++i;
							} else ignore_delim = !ignore_delim;
						} else token.push_back(buffer[i]);
					}
				}
				if (!token.empty() || buffer[buffer.size()-1]==m_Delimiter) m_Row.push_back(token);
			}

			/*
			void cleanIs()
			{
				if (m_OwnStream && m_Is!=NULL) {
					static_cast<std::ifstream*>(m_Is)->close();
					delete m_Is;
					m_Is = NULL;
				}
			}

			void readLine()
			{
				m_Row.clear();
				if (m_Is->eof()) {
					m_IsEnd = true;
					cleanIs();
					return;
				}
				std::string buffer;
				std::getline(*m_Is, buffer, '\n');
				if (m_Is->fail() && !m_Is->eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				while (boost::trim_copy(buffer).empty()) {
					if (m_Is->eof()) {
						m_IsEnd = true;
						cleanIs();
						return;
					}
					std::getline(*m_Is, buffer, '\n');
					if (m_Is->fail() && !m_Is->eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				}

				std::string token;
				bool ignore_delim = false;
				for (int i=0; i<buffer.size(); ++i) {
					if (buffer[i]=='\r' || buffer[i]=='\n') break;
					if (!ignore_delim && buffer[i]==m_Delimiter) {
						m_Row.push_back(token);
						token.clear();
					} else {
						if (buffer[i]=='\"') {
							if ((i+1)<buffer.size() && buffer[i+1]=='\"') {
								token.push_back('\"');
								++i;
							} else ignore_delim = !ignore_delim;
						} else token.push_back(buffer[i]);
					}
				}
				if (!token.empty() || buffer[buffer.size()-1]==m_Delimiter) m_Row.push_back(token);
			}
			*/
		};


		class CsvMapReader
		{
		public:
			CsvMapReader(std::istream& is, char delim=',', const std::vector<std::string>& header=std::vector<std::string>())
			{
				m_Delimiter = delim;
				m_Header = header;
				m_LineIterator.reset(new IsLineIterator(is));
			}
			
			CsvMapReader(const std::string& file, char delim=',', const std::vector<std::string>& header=std::vector<std::string>())
			{
				m_Delimiter = delim;
				m_Header = header;
				m_LineIterator.reset(new IsLineIterator(file));
			}
			
			void begin()
			{
				m_Row.clear();
				for (m_LineIterator->begin(); !m_LineIterator->end(); m_LineIterator->next()) {
					boost::trim_copy(m_LineIterator->get());
					if (!boost::trim_copy(m_LineIterator->get()).empty()) {
						break;
					}
				}
				if (m_Header.empty()) {
					readHeader();
					next();
				}
			}

			void next()
			{
				if (end()) return;
				m_Row.clear();
				m_LineIterator->next();
				for (; !m_LineIterator->end(); m_LineIterator->next()) {
					boost::trim_copy(m_LineIterator->get());
					if (!boost::trim_copy(m_LineIterator->get()).empty()) {
						break;
					}
				}
			}

			bool end() const
			{
				return m_LineIterator->end();
			}

			const boost::unordered_map<std::string, std::string>& getRow() const
			{
				if (m_Row.empty() && !end()) readRow();
				return m_Row;
			}
			
			const std::string get() const
			{
				return m_LineIterator->get();
			}

			const std::vector<std::string>& getHeader() const
			{
				return m_Header;
			}

		protected:
			char m_Delimiter;
			boost::scoped_ptr<IsLineIterator> m_LineIterator;
			std::vector<std::string> m_Header;
			mutable boost::unordered_map<std::string, std::string> m_Row;
			
			void readHeader()
			{
				std::string &buffer = m_LineIterator->get();
				std::string token;
				bool ignore_delim = false;
				for (int i=0; i<buffer.size(); ++i) {
					if (buffer[i]=='\r' || buffer[i]=='\n') break;
					if (!ignore_delim && buffer[i]==m_Delimiter) {
						m_Header.push_back(token);
						token.clear();
					} else {
						if (buffer[i]=='\"') {
							if ((i+1)<buffer.size() && buffer[i+1]=='\"') {
								token.push_back('\"');
								++i;
							} else ignore_delim = !ignore_delim;
						} else token.push_back(buffer[i]);
					}
				}
				if (!token.empty() || buffer[buffer.size()-1]==m_Delimiter) m_Header.push_back(token);
			}
			
			void readRow() const
			{
				std::string &buffer = m_LineIterator->get();
				int header_index = 0;
				std::string token;
				bool ignore_delim = false;
				for (int i=0; i<buffer.size(); ++i) {
					if (buffer[i]=='\r' || buffer[i]=='\n') break;
					if (!ignore_delim && buffer[i]==m_Delimiter) {
						if (header_index>=m_Header.size()) pcl_ThrowException(pcl::Exception(), "Invalid column count encountered!\nRow: "+buffer);
						m_Row[m_Header[header_index]] = token;
						++header_index;
						token.clear();
					} else {
						if (buffer[i]=='\"') {
							if ((i+1)<buffer.size() && buffer[i+1]=='\"') {
								token.push_back('\"');
								++i;
							} else ignore_delim = !ignore_delim;
						} else token.push_back(buffer[i]);
					}
				}
				if (!token.empty()) {
					if (header_index>=m_Header.size()) pcl_ThrowException(pcl::Exception(), "Invalid column count encountered!\nRow: "+buffer);
					m_Row[m_Header[header_index]] = token;
					++header_index;
				}
				for (int i=header_index; i<m_Header.size(); ++i) m_Row[m_Header[i]] = "";
			}

			/*
			void cleanIs()
			{
				if (m_OwnStream && m_Is!=NULL) {
					static_cast<std::ifstream*>(m_Is)->close();
					delete m_Is;
					m_Is = NULL;
				}
			}

			void readLine()
			{
				if (m_Header.empty()) readHeader();

				m_Row.clear();
				if (m_Is->eof()) {
					m_IsEnd = true;
					cleanIs();
					return;
				}
				std::string buffer;
				std::getline(*m_Is, buffer, '\n');
				if (m_Is->fail() && !m_Is->eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				while (boost::trim_copy(buffer).empty()) {
					if (m_Is->eof()) {
						m_IsEnd = true;
						cleanIs();
						return;
					}
					std::getline(*m_Is, buffer, '\n');
					if (m_Is->fail() && !m_Is->eof()) pcl_ThrowException(Exception(), "Error at reading stream");
				}

				int header_index = 0;
				std::string token;
				bool ignore_delim = false;
				for (int i=0; i<buffer.size(); ++i) {
					if (buffer[i]=='\r' || buffer[i]=='\n') break;
					if (!ignore_delim && buffer[i]==m_Delimiter) {
						if (header_index>=m_Header.size()) pcl_ThrowException(pcl::Exception(), "Invalid column count encountered!\nRow: "+buffer);
						m_Row[m_Header[header_index]] = token;
						++header_index;
						token.clear();
					} else {
						if (buffer[i]=='\"') {
							if ((i+1)<buffer.size() && buffer[i+1]=='\"') {
								token.push_back('\"');
								++i;
							} else ignore_delim = !ignore_delim;
						} else token.push_back(buffer[i]);
					}
				}
				if (!token.empty()) {
					if (header_index>=m_Header.size()) pcl_ThrowException(pcl::Exception(), "Invalid column count encountered!\nRow: "+buffer);
					m_Row[m_Header[header_index]] = token;
					++header_index;
				}
				for (int i=header_index; i<m_Header.size(); ++i) m_Row[m_Header[i]] = "";
			}
			*/
		};


		/*
		class CsvReader {
		public:
		typedef unsigned int IndexType;

		CsvReader(char delimiter=',') 
		{
		m_Delimiter = delimiter; 
		}

		void read(const std::string& filename, bool concatenate=false) 
		{
		std::ifstream is(filename.c_str(), std::ifstream::in);
		if (is.fail()) pcl_ThrowException(Exception(), "Failed at opening "+filename);
		try {
		read(is, concatenate);
		} catch (Exception&) {
		pcl_ThrowException(Exception(), "Error when reading "+filename);
		}
		is.close();
		}
		std::istream& read(std::istream& is, bool concatenate=false) 
		{
		if (!concatenate) m_Data.clear();
		while (!is.eof()) {
		std::string str_buffer;
		std::getline(is, str_buffer, '\n');
		if (is.fail() && !is.eof()) pcl_ThrowException(Exception(), "Error at reading stream");
		boost::trim(str_buffer);
		if (!str_buffer.empty()) Add(str_buffer);
		}
		return is;
		}

		IndexType size() const
		{ 
		return m_Data.size(); 
		}

		IndexType rowSize(IndexType i) const
		{
		return m_Data[i].size();
		}

		std::vector<std::string>& getRow(IndexType i) 
		{ 
		return m_Data[i]; 
		}
		std::vector<std::string> getRow(IndexType i) const
		{ 
		return m_Data[i]; 
		}

		std::string getElement(IndexType row, IndexType col) 
		{ 
		return m_Data[row][col]; 
		}
		template <class T>
		T getElement(IndexType row, IndexType col) 
		{ 
		return boost::lexical_cast<T>(m_Data[row][col]); 
		}

		protected:
		char m_Delimiter;
		std::vector< std::vector<std::string> > m_Data;

		void Add(std::string& line) {
		m_Data.push_back(std::vector<std::string>());
		std::vector<std::string> &row = m_Data[m_Data.size()-1];
		StringTokenizer tokenizer(line.c_str());
		for (tokenizer.begin(m_Delimiter); !tokenizer.end(); tokenizer.next(m_Delimiter)) {
		std::string token = tokenizer.getToken();
		boost::trim(token);
		row.push_back(token);
		}
		}
		};
		*/

	}
}

#endif