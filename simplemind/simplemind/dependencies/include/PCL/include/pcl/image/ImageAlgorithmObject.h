#ifndef PCL_IMAGE_ALGORITHM_OBJECT
#define PCL_IMAGE_ALGORITHM_OBJECT

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Region3D.h>
#include <pcl/type_utility.h>
#include <boost/utility/declval.hpp>
#include <ostream>

namespace pcl
{

	template <class T>
	class getPoint_exists
	{
		typedef char yes[1];
		typedef char no[2];

		static int func_test(const Point3D<int>&) {}
		template <class C> static yes &test(decltype(func_test(boost::declval<C>().getPoint()))*);
		template <class C> static no &test(...);
	public:
		enum { value = sizeof(test<T>(0)) == sizeof(yes) };
	};

	struct PointIndexObject
	{
		Point3D<int> point;
		long index;

		PointIndexObject() {}
		PointIndexObject(const Point3D<int>& p, long i): point(p), index(i) {}
		template <class IteratorType>
		PointIndexObject(const IteratorType& iter): point(iter.getPoint()), index(iter) {}

		template <class IteratorType>
		PointIndexObject& operator=(const IteratorType& iter)
		{
			index = iter;
			point = iter.getPoint();
			return *this;
		}

		long getIndex() const
		{
			return index;
		}
		operator long() const
		{
			return index;
		}

		const Point3D<int>& getPoint() const 
		{ 
			return point; 
		}

		friend std::ostream& operator<<(std::ostream& os, const PointIndexObject& obj)
		{
			os << obj.point << " " << obj.index;
			return os;
		}
	};
	
	struct ComponentInfo
	{
		Region3D<int> region;
		long count;
		
		ComponentInfo()
		{
			count = 0;
			region.reset();
		}
		void add(const Point3D<int>& p)
		{
			count++;
			region.add(p);
		}
		std::string toString() const
		{
			std::stringstream ss;
			ss << region.toString() << " " << count;
			return std::move(ss.str());
		}
		friend std::ostream& operator<<(std::ostream& os, const ComponentInfo& info)
		{
			os << info.region << " " << info.count;
			return os;
		}
	};

}

#endif