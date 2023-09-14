#ifndef PCL_FAST_LOOKUP_IMAGE_BUFFER
#define PCL_FAST_LOOKUP_IMAGE_BUFFER

#include <pcl/image/ImageBuffer.h>

namespace pcl
{

	template <class Type>
	class FastLookupImageBuffer: private boost::noncopyable
	{
	public:
		typedef FastLookupImageBuffer Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef Type ValueType;
		typedef Type& ReferenceValueType;
		typedef const Type& ConstantValueType;
		typedef Type IoValueType;
		typedef ValueType* BufferType;

		inline ReferenceValueType at(long index)
		{
			return m_Data[index];
		}

	protected:
		void setupBuffer(int sx, int sy, int sz)
		{
			m_Data = new Type[sx*sy*sz];
		}

#include <pcl/image/FastLookupImageBuffer.txx>
	};

	/**************** Special case for bits ****************/

	template <>
	class FastLookupImageBuffer<Bit>: private boost::noncopyable
	{
	public:
		typedef FastLookupImageBuffer Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef Bit ValueType;
		typedef Bit ReferenceValueType;
		typedef bool ConstantValueType;
		typedef bool IoValueType;
		typedef std::vector<bool>* BufferType;

		inline ReferenceValueType atIndex(long index)
		{
			return m_Data->at(index);
		}

	protected:
		void setupBuffer(int sx, int sy, int sz)
		{
			m_Data = new std::vector<bool>(sx*sy*sz);
		}

#include <pcl/image/FastLookupImageBuffer.txx>
	};


	//template <class Type>
	//class FastLookupImageBuffer: public ImageBuffer<Type>
	//{
	//public:
	//	typedef FastLookupImageBuffer Self;
	//	typedef ImageBuffer<Type> Parent;
	//	typedef boost::shared_ptr<Self> Pointer;
	//	typedef typename Parent::ValueType ValueType;
	//	typedef typename Parent::ReferenceValueType ReferenceValueType;
	//	typedef typename Parent::ConstantValueType ConstantValueType;
	//	typedef typename Parent::IoValueType IoValueType;
	//	typedef typename Parent::BufferType BufferType;

	//	static Pointer New(long sx, long sy, long sz) {
	//		Pointer obj(new Self());
	//		obj->setupBuffer(sx, sy, sz);
	//		obj->m_Own = true;
	//		obj->setSize(sx, sy, sz);
	//		return obj;
	//	}

	//	static Pointer New(BufferType ptr, long sx, long sy, long sz, bool own=false) {
	//		Pointer obj(new Self());
	//		obj->m_Data = ptr;
	//		obj->m_Own = own;
	//		obj->setSize(sx, sy, sz);
	//		return obj;
	//	}

	//	/******** Reference retrieval ********/
	//	using Parent::at;
	//	inline ReferenceValueType at(int x, int y, int z)
	//	{
	//		return at(x + m_LookupTableY[y] + m_LookupTableZ[z]);
	//	}
	//	inline ReferenceValueType at(const Point3D<int>& p)
	//	{
	//		return at(p.x() + m_LookupTableY[p.y()] + m_LookupTableZ[p.z()]);
	//	}

	//	/******** Buffer information retrieval ********/
	//	inline long toIndex(const Point3D<int>& p) const
	//	{
	//		return toIndex(p.x(), p.y(), p.z());
	//	}
	//	inline long toIndex(int x, int y, int z) const
	//	{
	//		return x + m_LookupTableY[y] + m_LookupTableZ[z];
	//	}

	//protected:
	//	boost::scoped_array<long> m_LookupTableY, m_LookupTableZ;

	//	FastLookupImageBuffer() {}

	//	void setSize(int sx, int sy, int sz)
	//	{
	//		Parent::setSize(sx, sy, sz);

	//		m_LookupTableY.reset(new long[m_Size.y()]);
	//		for (int i=0; i<m_Size.y(); i++) {
	//			m_LookupTableY[i] = i*m_OffsetTable[1];
	//		}

	//		m_LookupTableZ.reset(new long[m_Size.z()]);
	//		for (int i=0; i<m_Size.z(); i++) {
	//			m_LookupTableZ[i] = i*m_OffsetTable[2];
	//		}
	//	}
	//};

}

#endif