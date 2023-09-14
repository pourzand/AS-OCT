#ifndef PCL_IMAGE_BASE
#define PCL_IMAGE_BASE

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Region3D.h>
#include <pcl/metadata.h>
#include <pcl/type_utility.h>

#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <typeinfo>

namespace pcl
{

	namespace image_base_details
	{
		struct ToDouble
		{
			template <class T>
			inline typename boost::enable_if<boost::is_convertible<T,double>, double>::type operator()(const T& val) const
			{
				return val;
			}
			template <class T>
			inline typename boost::disable_if<boost::is_convertible<T,double>, double>::type operator()(const T& val) const
			{
				return std::numeric_limits<double>::signaling_NaN();
			}
		};
		
		struct SetDouble
		{
			template <class Image>
			inline typename boost::enable_if<boost::is_convertible<double, typename Image::IoValueType>, bool>::type operator()(Image* const img, long index, double val)
			{
				img->set(index, val);
				return true;
			}
			template <class Image>
			inline typename boost::enable_if<boost::is_convertible<double, typename Image::IoValueType>, bool>::type operator()(Image* const img, int x, int y, int z, double val)
			{
				img->set(x,y,z, val);
				return true;
			}
			
			template <class Image>
			inline typename boost::disable_if<boost::is_convertible<double, typename Image::IoValueType>, bool>::type operator()(Image* const img, long index, double val) const
			{
				return false;
			}
			template <class Image>
			inline typename boost::disable_if<boost::is_convertible<double, typename Image::IoValueType>, bool>::type operator()(Image* const img, int x, int y, int z, double val)
			{
				return false;
			}
		};
	}

	class ImageBase: private boost::noncopyable
	{
	public:
		typedef ImageBase Self;
		typedef boost::shared_ptr< Self > Pointer;
		typedef boost::shared_ptr< Self const > ConstantPointer;

		/******** Getters ********/
		inline const Point3D<int>& getSize() const 
		{ 
			return this->m_Size; 
		}

		inline const Point3D<int>& getMinPoint() const
		{ 
			return this->m_Region.getMinPoint(); 
		}
		inline const Point3D<int>& getMaxPoint() const
		{ 
			return this->m_Region.getMaxPoint(); 
		}
		inline const Region3D<int>& getRegion() const
		{
			return this->m_Region;
		}

		inline const Point3D<double>& getSpacing() const
		{ 
			return this->m_Spacing; 
		}
		inline const Point3D<double>& getOneOverSpacing() const
		{ 
			return this->m_OneOverSpacing; 
		}
		inline const Point3D<double>& getOrigin() const
		{ 
			return this->m_Origin; 
		}
		
		inline const Point3D<int>& getBufferOffset() const
		{
			return m_Offset;
		}
		
		inline long const* getOffsetTable() const
		{
			return m_OffsetTable;
		}

		/******** Metadata ********/

		Metadata::ConstantPointer getMetadata() const
		{
			return m_Metadata;
		}

		Metadata::Pointer getMetadata()
		{
			return m_Metadata;
		}

		void setMetadata(const Metadata::Pointer& meta)
		{
			m_Metadata = meta;
		}

		/******** Boundary test ********/
		template <class PT>
		inline bool contain(const PT& p) const
		{
			return contain(p.x(), p.y(), p.z());
		}
		template <class T>
		inline bool contain(T x, T y, T z) const
		{
			return this->m_Region.contain(x,y,z);
		}

		/******** Coordinates conversion ********/
		inline Point3D<int> toBufferCoordinate(const Point3D<int>& p) const
		{
			return toBufferCoordinate(p.x(), p.y(), p.z());
		}
		inline Point3D<int> toBufferCoordinate(int x, int y, int z) const
		{
			return Point3D<int>(m_Offset.x()+x, m_Offset.y()+y, m_Offset.z()+z);
		}

		inline int toSliceNum(int z) const
		{
			return m_Offset.z()+z;
		}
		inline int toSliceNum(const Point3D<int>& p) const
		{
			return toSliceNum(p.z());
		}

		/******** Virtual methods ********/
		virtual long toIndex(const Point3D<int>& p) const
		{
			return toIndex(p.x(), p.y(), p.z());
		}
		virtual long toIndex(int x, int y, int z) const = 0;

		virtual Point3D<int> toPoint(long index) const = 0;

		virtual const Point3D<int>& getBufferSize() const = 0;
		
		virtual bool isType(const std::type_info& t) const = 0;

		/******** Virtual get methods ********/
		virtual double getValue(long index) const = 0;

		virtual double getValue(int x, int y, int z) const = 0;

		inline double getValue(const Point3D<int>& p) const
		{
			return getValue(p.x(), p.y(), p.z());
		}
		
		/******** Virtual set methods ********/
		virtual bool setValue(long index, double value) = 0;

		virtual bool setValue(int x, int y, int z, double value) = 0;

		inline bool setValue(const Point3D<int>& p, double value)
		{
			return setValue(p.x(), p.y(), p.z(), value);
		}

	protected:
		Point3D<double> m_Spacing, m_OneOverSpacing,
			m_Origin; //This is the physical coordinate at index space (0,0,0)
		Point3D<int> m_Size;
		Region3D<int> m_Region;
		Point3D<int> m_Offset;
		Metadata::Pointer m_Metadata;
		const long* m_OffsetTable;

		inline void setOrigin(const Point3D<double>& origin) 
		{ 
			this->m_Origin = origin; 
		}
		inline void setSpacing(const Point3D<double>& spacing) 
		{ 
			this->m_Spacing = spacing; 
			this->m_OneOverSpacing.set(1/spacing.x(), 1/spacing.y(), 1/spacing.z());
		}

		inline void setMinPoint(const Point3D<int>& minp, const Point3D<int>& buffer_coord) 
		{ 
			this->m_Region.setMinPoint(minp);
			this->m_Region.setMaxPoint(minp+m_Size-1);
			this->m_Offset = buffer_coord-minp;
		}

		ImageBase() {}
	};

}

#endif