#ifndef PCL_DENSE_POINT_SET
#define PCL_DENSE_POINT_SET

#include <pcl/geometry.h>
#include <boost/smart_ptr.hpp>
#include <map>

namespace pcl
{

	class DensePointSet
	{
	public:
		typedef boost::shared_ptr<DensePointSet> Pointer;

		static Pointer New()
		{
			return Pointer(new DensePointSet());
		}

		~DensePointSet() {}

		void add(const Point3D<int>& p, int trailing_x_num = 0)
		{

		}

	protected:
		typedef std::map<int, int> IntervalMap;
		typedef std::map<int, IntervalMap> LineMap;
		typedef std::map<int, LineMap> PlaneMap;
		PlaneMap m_Data;

		bool m_PrevExist;
		PlaneMap::iterator m_PrevPlaneIter;
		LineMap::iterator m_PrevLineIter;
		IntervalMap::iterator m_PrevInterval;

		DensePointSet() 
		{
			m_PrevExist = false;
		}
	};

}

#endif