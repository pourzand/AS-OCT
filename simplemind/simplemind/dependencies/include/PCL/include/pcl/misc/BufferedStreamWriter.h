#ifndef PCL_BUFFEREDSTREAMWRITER
#define PCL_BUFFEREDSTREAMWRITER

#include <pcl/exception.h>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <ostream>

namespace pcl
{
	namespace misc
	{

		class OstreamWriter
		{
		public:
			typedef OstreamWriter Self;
			typedef boost::shared_ptr<Self> Pointer;
			
			static Pointer New(std::ostream& os)
			{
				return Pointer(new Self(os));
			}
			
			bool write(char* buffer, long size, bool)
			{
				m_Os->write(buffer, size);
				return m_Os->good();
			}

		protected:
			std::ostream *m_Os;
			
			OstreamWriter(std::ostream& os)
			{
				m_Os = &os;
			}
		};



		template <class StreamWriter>
		class BufferedStreamWriter
		{
		public:
			BufferedStreamWriter(const typename StreamWriter::Pointer& writer, long buffer_size, bool flush_on_destruction=true)
			{
				m_MaxBufferSize = buffer_size;
				m_Index = 0;
				m_Buffer.reset(new char[m_MaxBufferSize]);
				m_Writer = writer;
				m_FlushOnDestruction = flush_on_destruction;
				m_Ended = false;
			}

			~BufferedStreamWriter()
			{
				if (m_FlushOnDestruction) flush(true);
			}

			template <class Type>
			void add(const Type& data)
			{
				const char *ptr = (const char*)&data;
				for (int i=0; i<sizeof(Type); ++i) {
					m_Buffer[m_Index] = ptr[i];
					++m_Index;
					if (m_Index==m_MaxBufferSize) flush();
				}
			}

			void flush(bool is_end=false)
			{
				if (m_Ended) return;
				if (m_Index==0) {
					if (!is_end) return;
					else m_Ended = true;
				}
				if (!m_Writer->write(m_Buffer.get(), m_Index, is_end)) {
					pcl_ThrowException(pcl::Exception(), "Error encountered while writing!");
				}
				m_Index = 0;
			}

		protected:
			bool m_Ended;
			typename StreamWriter::Pointer m_Writer;
			long m_MaxBufferSize;
			boost::scoped_array<char> m_Buffer;
			long m_Index;
			bool m_FlushOnDestruction;
		};

	}
}

#endif