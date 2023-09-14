#ifndef PCL_IMAGE_TYPE
#define PCL_IMAGE_TYPE

#include <pcl/image/ImagePhysicalLayer.h>
#include <pcl/image/ImageBuffer.h>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

namespace pcl
{

	template <class Type, bool Flag=false, class Buffer=ImageBuffer<Type>, class Enable=void>
	class Image: public ImagePhysicalLayer<Flag>
	{
#include <pcl/image/Image.txx>
	public:
#include <pcl/image/image_base_virtual_get.txx>
#include <pcl/image/image_base_virtual_set.txx>

		template <class T>
		inline void set(int x, int y, int z, const T& val)
		{
			m_Buffer->at(this->toBufferCoordinate(x,y,z)) = static_cast<IoValueType>(val);
		}
		template <class T>
		inline void set(const Point3D<int>& p, const T& val)
		{
			this->set(p.x(), p.y(), p.z(), val);
		}
		template <class T>
		inline void set(long index, const T& val)
		{
			m_Buffer->at(index) = static_cast<IoValueType>(val);
		}
	};


	template <class Type, bool Flag, class Buffer>
	class Image<Type, Flag, Buffer, typename boost::enable_if<boost::is_integral<Type> >::type>: public ImagePhysicalLayer<Flag>
	{
#include <pcl/image/Image.txx>
	public:
#include <pcl/image/image_base_virtual_get.txx>
#include <pcl/image/image_base_virtual_set.txx>

		template <class T>
		inline void set(const Point3D<int>& p, const T& val)
		{
			this->set(p.x(), p.y(), p.z(), val);
		}
		
		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>, void>::type set(long index, const T& val)
		{
			m_Buffer->at(index) = static_cast<IoValueType>(val);
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>, void>::type set(long index, const T& val)
		{
			m_Buffer->at(index) = static_cast<IoValueType>(pcl_Round(val));
		}

		template <class T>
		inline typename boost::disable_if<boost::is_floating_point<T>, void>::type set(int x, int y, int z, const T& val)
		{
			m_Buffer->at(this->toBufferCoordinate(x,y,z)) = static_cast<IoValueType>(val);
		}
		template <class T>
		inline typename boost::enable_if<boost::is_floating_point<T>, void>::type set(int x, int y, int z, const T& val)
		{
			m_Buffer->at(this->toBufferCoordinate(x,y,z)) = static_cast<IoValueType>(pcl_Round(val));
		}
	};

}

#endif