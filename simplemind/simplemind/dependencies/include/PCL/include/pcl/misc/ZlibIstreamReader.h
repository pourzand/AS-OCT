#ifndef PCL_ZLIBISTREAMREADER
#define PCL_ZLIBISTREAMREADER

#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <istream>
#include <zlib.h>

namespace pcl
{
	namespace misc
	{

		class ZlibIstreamReader
		{
		public:
			typedef ZlibIstreamReader Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(std::istream& is, long input_buffer_size)
			{
				return Pointer(new Self(is, input_buffer_size));
			}

			~ZlibIstreamReader()
			{
				inflateEnd(&m_ZStream);
			}

			long read(char* buffer, long size)
			{
				m_ZStream.avail_out = size;
				m_ZStream.next_out = (unsigned char*)buffer;

				bool done = false;
				int status;
				while (!done) {
					if (!m_Continue) {
						if (!resetInputBuffer()) return 0;
						m_Continue = true;
					}
					status = inflate(&m_ZStream, Z_NO_FLUSH);
					if (m_ZStream.avail_in==0) {
						m_InputBuffer.reset();
						m_Continue = false;
					}
					if (status!=Z_OK) done = true;
					else if (m_ZStream.avail_out==0) done = true;
				}
				if (status==Z_STREAM_END) {
					m_InputBuffer.reset();
				}
				return size-m_ZStream.avail_out;
			}

		protected:
			std::istream *m_Is;
			z_stream m_ZStream;
			boost::scoped_array<unsigned char> m_InputBuffer;
			long m_InputBufferSize;
			bool m_Continue;

			ZlibIstreamReader(std::istream& is, long input_buffer_size)
			{
				m_InputBufferSize = input_buffer_size;
				m_Is = &is;
				m_ZStream.zalloc = Z_NULL;
				m_ZStream.zfree = Z_NULL;
				m_ZStream.opaque = Z_NULL;
				m_ZStream.avail_in = 0;
				m_ZStream.next_in = Z_NULL;
				inflateInit(&m_ZStream);
				m_Continue = false;
			}

			bool resetInputBuffer()
			{
				m_InputBuffer.reset(new unsigned char[m_InputBufferSize]);
				m_Is->read((char*)(&(m_InputBuffer[0])), m_InputBufferSize);
				if (m_Is->gcount()==0) {
					m_InputBuffer.reset();
					return false;
				}
				m_ZStream.avail_in = m_Is->gcount();
				m_ZStream.next_in = &(m_InputBuffer[0]);
				return true;
			}
		};

	}
}

#endif