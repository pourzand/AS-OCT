#ifndef PCL_STREAM_EXCEPTION
#define PCL_STREAM_EXCEPTION

#include <iostream>
#include <boost/noncopyable.hpp>

namespace pcl
{
	template <class StreamType>
	class StreamExceptionObject: boost::noncopyable 
	{
	public:
		StreamExceptionObject()
		{
			m_Stream = NULL;
		}

		StreamExceptionObject(StreamType& s)
		{
			m_Stream = &s;
			m_IoState = s.exceptions();
		}

		StreamExceptionObject(StreamType& s, std::ios_base::iostate state)
		{
			m_Stream = &s;
			m_IoState = s.exceptions();
			exceptions(state);
		}

		StreamExceptionObject(StreamExceptionObject&& obj) 
		{
			move(std::move(obj));
		}

		StreamExceptionObject operator=(StreamExceptionObject&& obj)
		{
			move(std::move(obj));
			return *this;
		}

		void exceptions(std::ios_base::iostate state)
		{
			m_Stream->exceptions(state);
		}

		std::ios_base::iostate exceptions() const
		{
			return m_Stream->exceptions();
		}

		~StreamExceptionObject()
		{
			if (m_Stream!=NULL) {
				m_Stream->exceptions(m_IoState);
			}
		}

	protected:
		StreamType *m_Stream;
		std::ios_base::iostate m_IoState;

		void move(StreamExceptionObject&& obj)
		{
			m_Stream = obj.m_Stream;
			m_IoState = obj.m_IoState;
			obj.m_Stream = NULL;
		}

	private:
		StreamExceptionObject(const StreamExceptionObject&);
		StreamExceptionObject& operator=(const StreamExceptionObject&);
	};


	class StreamExceptionHelper
	{
	public:
		template <class StreamType>
		static StreamExceptionObject<StreamType> GetStreamExceptionObject(StreamType& s)
		{
			return std::move(StreamExceptionObject<StreamType>(s));
		}

		template <class StreamType>
		static StreamExceptionObject<StreamType> GetStreamExceptionObject(StreamType& s, std::ios_base::iostate state)
		{
			return std::move(StreamExceptionObject<StreamType>(s, state));
		}

	};
}

#endif