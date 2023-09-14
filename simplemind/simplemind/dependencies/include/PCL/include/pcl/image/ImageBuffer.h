#ifndef PCL_IMAGE_BUFFER
#define PCL_IMAGE_BUFFER

#ifdef PCL_DEBUG
#include <pcl/exception.h>
#include <boost/lexical_cast.hpp>
#endif

#include<pcl/geometry/Point.h>
#include <pcl/misc/BitType.h>

#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

namespace pcl
{

	template <class Type>
	class ImageBuffer: private boost::noncopyable
	{
	public:
		enum {
			ItkAliasable = true
		};
		typedef ImageBuffer Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef Type ValueType;
		typedef Type& ReferenceValueType;
		typedef const Type& ConstantValueType;
		typedef Type IoValueType;
		typedef ValueType* BufferType;
		
		virtual ~ImageBuffer()
		{
			if (m_Own) delete[] m_Data;
		}

		inline ReferenceValueType at(long index)
		{
#ifdef PCL_DEBUG
			if (index>=m_OffsetTable[3] || index<0) pcl_ThrowException(Exception(), "Index " + boost::lexical_cast<std::string>(index) + " out of range");
#endif
			return m_Data[index];
		}
		
		inline ConstantValueType at(long index) const
		{
#ifdef PCL_DEBUG
			if (index>=m_OffsetTable[3] || index<0) pcl_ThrowException(Exception(), "Index " + boost::lexical_cast<std::string>(index) + " out of range");
#endif
			return m_Data[index];
		}

	protected:
		void setupBuffer(int sx, int sy, int sz)
		{
			long size = sx;
			size *= sy;
			size *= sz;
			m_Data = new Type[size];
		}

#include <pcl/image/ImageBuffer.txx>
	};

	/**************** Special case for bits ****************/

	template <>
	class ImageBuffer<Bit>: private boost::noncopyable
	{
	public:
		enum {
			ItkAliasable = false
		};
		typedef ImageBuffer Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef Bit ValueType;
		typedef Bit ReferenceValueType;
		typedef bool ConstantValueType;
		typedef bool IoValueType;
		typedef std::vector<bool>* BufferType;
		
		virtual ~ImageBuffer()
		{
			if (m_Own) delete m_Data;
		}

		inline ReferenceValueType at(long index)
		{
#ifdef PCL_DEBUG
			if (index>=m_OffsetTable[3] || index<0) pcl_ThrowException(Exception(), "Index " + boost::lexical_cast<std::string>(index) + " out of range");
#endif
			return m_Data->at(index);
		}
		
		inline ConstantValueType at(long index) const
		{
#ifdef PCL_DEBUG
			if (index>=m_OffsetTable[3] || index<0) pcl_ThrowException(Exception(), "Index " + boost::lexical_cast<std::string>(index) + " out of range");
#endif
			return m_Data->at(index);
		}

	protected:
		void setupBuffer(int sx, int sy, int sz)
		{
			long size = sx;
			size *= sy;
			size *= sz;
			m_Data = new std::vector<bool>(size);
		}

#include <pcl/image/ImageBuffer.txx>
	};

}

#endif