#ifndef PCL_IO_TOOLS
#define PCL_IO_TOOLS
#include <ostream>
#include <pcl/iterator/IteratorMacro.h>

namespace pcl
{
	namespace misc
	{

		class IsLineIterator
		{
		public:
			IsLineIterator(std::istream& is)
			{
				m_OwnStream = false;
				m_Is = &is;
			}
			
			IsLineIterator(const std::string& file)
			{
				m_OwnStream = true;
				m_Is = new std::ifstream(file.c_str(), std::ifstream::in);
				if (m_Is->fail()) {
					pcl_ThrowException(Exception(), "Failed at opening "+file);
				}
			}
			
			virtual ~IsLineIterator()
			{
				if (m_OwnStream) {
					static_cast<std::ifstream*>(m_Is)->close();
					delete m_Is;
				}
			}

			void begin()
			{
				m_End = false;
				next();
			}

			bool end()
			{
				return m_End;
			}

			void next()
			{
				if (m_Is->eof()) {
					m_End = true;
					return;
				}
				std::getline(*m_Is, m_Buffer, '\n');
				if (m_Is->fail() && !m_Is->eof()) pcl_ThrowException(Exception(), "Error at reading stream");
			}

			std::string& get()
			{
				return m_Buffer;
			}
		protected:
			std::istream *m_Is;
			std::string m_Buffer;
			bool m_OwnStream;
			bool m_End;
		};

	}
}

#endif