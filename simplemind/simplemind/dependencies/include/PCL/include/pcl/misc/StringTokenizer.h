#ifndef PCL_STRING_TOKENIZER
#define PCL_STRING_TOKENIZER

#include <string.h>
#include <string>

#pragma warning (push)
#pragma warning (disable: 4018)

namespace pcl
{
	namespace misc
	{

		class StringTokenizer 
		{
		public:
			StringTokenizer() 
			{
				setRemoveDelimiter(true);
			}
			StringTokenizer(const char* str) 
			{
				setString(str);
				setRemoveDelimiter(true);
			}

			void setString(const char* str) 
			{ 
				m_Buffer = str;
				m_BufferLength = strlen(m_Buffer);
				reset();
			}

			void setRemoveDelimiter(bool status)
			{
				m_RemoveDelimiter = status;
			}

			//Begin methods
			void begin(char delimiter) const
			{ 
				reset();
				next(delimiter);
			}
			void begin(const std::string& delimiter) const
			{
				reset();
				next(delimiter);
			}
			void begin(const std::string& char_list, bool list_is_delimiter) const
			{ 
				reset();
				next(char_list, list_is_delimiter);
			}
			template <class Functor>
			void begin(Functor& func)
			{
				reset();
				next(func);
			}

			//Next methods
			void next(char delimiter) const
			{
				if (m_End) return;
				if (m_Pos==m_BufferLength) {
					m_End = true;
					return;
				}

				m_DelimiterFound = false;
				size_t new_pos;
				for (size_t i=m_Pos; i<m_BufferLength; i++) {
					if (m_Buffer[i]==delimiter) {
						m_DelimiterFound = true;
						new_pos = i;
						break;
					}
				}

				setNewPos(new_pos, 1);
			}
			void next(const std::string& delimiter) const
			{
				if (m_End) return;
				if (m_Pos==m_BufferLength) {
					m_End = true;
					return;
				}

				m_DelimiterFound = false;
				size_t new_pos;
				for (size_t i=m_Pos; i<m_BufferLength; i++) {
					if (isSameAsString(i, delimiter)) {
						m_DelimiterFound = true;
						new_pos = i;
						break;
					}
				}

				setNewPos(new_pos, delimiter.length());
			}
			void next(const std::string& char_list, bool list_is_delimiter) const
			{
				if (m_End) return;
				if (m_Pos==m_BufferLength) {
					m_End = true;
					return;
				}

				m_DelimiterFound = false;
				size_t new_pos;
				if (list_is_delimiter) {
					for (size_t i=m_Pos; i<m_BufferLength; i++) {
						if (isCharOfString(i, char_list)) {
							m_DelimiterFound = true;
							new_pos = i;
							break;
						}
					}
				} else {
					for (size_t i=m_Pos; i<m_BufferLength; i++) {
						if (!isCharOfString(i, char_list)) {
							m_DelimiterFound = true;
							new_pos = i;
							break;
						}
					}
				}

				setNewPos(new_pos, 1);
			}
			template <class Functor>
			void next(Functor& func) const
			{
				if (m_End) return;
				if (m_Pos==m_BufferLength) {
					m_End = true;
					return;
				}

				m_DelimiterFound = false;
				size_t new_pos;
				for (size_t i=m_Pos; i<m_BufferLength; i++) {
					if (func(m_Buffer[i])) {
						m_DelimiterFound = true;
						new_pos = i;
						break;
					}
				}

				setNewPos(new_pos, 1);
			}

			//End method
			inline bool end() const
			{
				return m_End;
			}

			//Information extraction
			bool delimiterFound() const
			{ 
				return m_DelimiterFound; 
			}

			const std::string& getToken() const
			{ 
				return m_Token; 
			}

			size_t getPos() const 
			{ 
				return m_Pos; 
			}

			const char* getSource() const
			{ 
				return m_Buffer; 
			}
			
			const char* getRemainder() const 
			{ 
				if (m_End) return NULL;
				return m_Buffer+m_Pos; 
			}

		protected:
			const char *m_Buffer;
			size_t m_BufferLength;
			bool m_RemoveDelimiter;
			mutable std::string m_Token;
			mutable size_t m_Pos;
			mutable bool m_End;
			mutable bool m_DelimiterFound;

			void reset() const
			{
				m_Pos = 0;
				m_End = false;
				m_Token.clear();
			}

			void setNewPos(size_t new_pos, size_t offset) const
			{
				if (!m_DelimiterFound) {
					m_Token = &(m_Buffer[m_Pos]);
					m_Pos = m_BufferLength;
				} else {
					m_Token.clear();
					m_Token.append(&m_Buffer[m_Pos], new_pos-m_Pos);
					if (m_RemoveDelimiter) m_Pos = new_pos+offset;
					else m_Pos = new_pos;
				}
			}

			bool isCharOfString(size_t pos, const std::string& delimiter) const
			{
				for (int i=0; i<delimiter.length(); i++) {
					if (m_Buffer[pos]==delimiter[i]) return true;
				}
				return false;
			}

			bool isSameAsString(size_t pos, const std::string& delimiter) const
			{
				if (m_BufferLength-pos<delimiter.length()) return false;
				for (int i=0; i<delimiter.length(); i++) {
					if (m_Buffer[pos+i]!=delimiter[i]) return false;
				}
				return true;
			}
		};

	}
}

#pragma warning (pop)
#endif