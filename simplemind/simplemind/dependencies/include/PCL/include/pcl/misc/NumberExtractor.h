#ifndef PCL_NUMBER_EXTRACTOR
#define PCL_NUMBER_EXTRACTOR

#include <pcl/misc/StringTokenizer.h>

#pragma warning (push)
#pragma warning (disable: 4018)

namespace pcl
{
	namespace misc
	{

		class NumberExtractor 
		{
		public:
			enum Mode
			{
				FLOAT,
				INTEGER,
				UNSIGNED
			};

			NumberExtractor(const char* str)
			{
				m_Tokenizer.setString(str);
			}

			void begin(Mode mode) const
			{
				switch(mode) {
				case FLOAT:
					m_Tokenizer.begin(m_FloatChar, false);
					break;
				case INTEGER:
					m_Tokenizer.begin(m_IntegerChar, false);
					break;
				case UNSIGNED:
					m_Tokenizer.begin(m_UnsignedChar, false);
				}
				if (m_Tokenizer.getToken().empty()) next(mode);
			}

			void next(Mode mode) const
			{
				switch(mode) {
				case FLOAT:
					next(m_FloatChar);
					break;
				case INTEGER:
					next(m_IntegerChar);
					break;
				case UNSIGNED:
					next(m_UnsignedChar);
				}
			}

			inline bool end() const
			{
				return m_Tokenizer.end();
			}

			inline double getValue() const
			{
				return atof(m_Tokenizer.getToken().c_str());
			}

		protected:
			StringTokenizer m_Tokenizer;
			static const std::string m_FloatChar, m_IntegerChar, m_UnsignedChar;

			void next(const std::string& list) const
			{
				do {
					m_Tokenizer.next(list, false);
				} while (m_Tokenizer.getToken().empty() && !m_Tokenizer.end());
			}
		};

		const std::string NumberExtractor::m_FloatChar = "0123456789+-.eE";
		const std::string NumberExtractor::m_IntegerChar = "0123456789+-";
		const std::string NumberExtractor::m_UnsignedChar = "0123456789";

	}
}

#pragma warning (pop)
#endif