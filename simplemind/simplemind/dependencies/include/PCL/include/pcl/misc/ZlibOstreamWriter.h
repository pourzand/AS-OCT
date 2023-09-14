#ifndef PCL_ZLIBOSTREAMWRITER
#define PCL_ZLIBOSTREAMWRITER

#include <pcl/exception.h>
#include <boost/scoped_array.hpp>
#include <ostream>
#include <zlib.h>

namespace pcl
{
	namespace misc
	{

		class ZlibOstreamWriter
		{
		public:
			typedef ZlibOstreamWriter Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(std::ostream& os, int level, long output_buffer_size)
			{
				return Pointer(new Self(os, level, output_buffer_size));
			}

			~ZlibOstreamWriter()
			{
				deflateEnd(&m_ZStream);
			}

			bool write(char* buffer, long size, bool is_end)
			{
				m_ZStream.avail_in = size;
				m_ZStream.next_in = (unsigned char*)buffer;

				int flush = Z_NO_FLUSH;
				if (is_end) flush = Z_FINISH;

				int status;
				bool done = false;
				while (!done) {
					status = deflate(&m_ZStream, flush);
					if (flush==Z_FINISH) {
						if (status==Z_STREAM_END) done = true;
						m_Os->write((char*)&(m_OutputBuffer[0]), m_OutputBufferSize-m_ZStream.avail_out);
						if (!m_Os->good()) return false;
						resetOutputBuffer();
					} else {
						if (m_ZStream.avail_out==0) {
							m_Os->write((char*)&(m_OutputBuffer[0]), m_OutputBufferSize);
							if (!m_Os->good()) return false;
							resetOutputBuffer();
						}
						if (m_ZStream.avail_in==0) done = true;
					}
				}
				return true;
			}

		protected:
			std::ostream *m_Os;
			z_stream m_ZStream;
			boost::scoped_array<unsigned char> m_OutputBuffer;
			long m_OutputBufferSize;
			bool m_Continue;

			ZlibOstreamWriter(std::ostream& os, int level, long output_buffer_size)
			{
				m_OutputBufferSize = output_buffer_size;
				m_OutputBuffer.reset(new unsigned char[m_OutputBufferSize]);
				m_Os = &os;
				m_ZStream.zalloc = Z_NULL;
				m_ZStream.zfree = Z_NULL;
				m_ZStream.opaque = Z_NULL;
				deflateInit(&m_ZStream, level);
				resetOutputBuffer();
			}

			void resetOutputBuffer()
			{
				m_ZStream.avail_out = m_OutputBufferSize;
				m_ZStream.next_out = &(m_OutputBuffer[0]);
			}
		};

	}
}

#endif