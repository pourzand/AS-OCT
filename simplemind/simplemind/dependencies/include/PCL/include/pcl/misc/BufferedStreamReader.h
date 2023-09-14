#ifndef PCL_BUFFEREDSTREAMREADER
#define PCL_BUFFEREDSTREAMREADER

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <istream>

namespace pcl
{
	namespace misc
	{

		class IstreamReader
		{
		public:
			typedef IstreamReader Self;
			typedef boost::shared_ptr<Self> Pointer;
			
			static Pointer New(std::istream& is)
			{
				return Pointer(new Self(is));
			}
			
			long read(char* buffer, long size)
			{
				m_Is->read(buffer, size);
				return m_Is->gcount();
			}

		protected:
			std::istream *m_Is;
			
			IstreamReader(std::istream& is)
			{
				m_Is = &is;
			}
		};



		template <class StreamReader>
		class BufferedStreamReader
		{
		public:
			BufferedStreamReader(const typename StreamReader::Pointer& reader, long buffer_size)
			{
				m_MaxBufferSize = buffer_size;
				m_Index = 0;
				m_BufferSize = 0;
				m_Buffer.reset(new char[m_MaxBufferSize]);
				m_Reader = reader;
			}

			template <class Type>
			const Type* get()
			{
				if (m_Index+sizeof(Type)>m_BufferSize) {
					fill();
					if (m_Index+sizeof(Type)>m_BufferSize) {
						return NULL;
					}
				}
				Type *ptr = (Type*)(&m_Buffer[m_Index]);
				m_Index += sizeof(Type);
				return ptr;
			}

		protected:
			typename StreamReader::Pointer m_Reader;
			long m_MaxBufferSize;
			boost::scoped_array<char> m_Buffer;
			long m_BufferSize;
			long m_Index;

			void fill()
			{
				long count = 0;
				for (long i=m_Index; i<m_BufferSize; ++i) {
					m_Buffer[count] = m_Buffer[i];
					++count;
				}
				long read_size = m_MaxBufferSize - (m_BufferSize-m_Index);
				long item_read = m_Reader->read(&(m_Buffer[count]), read_size);
				m_BufferSize = item_read + (m_BufferSize-m_Index);
				m_Index = 0;
			}
		};

	}
}

#endif