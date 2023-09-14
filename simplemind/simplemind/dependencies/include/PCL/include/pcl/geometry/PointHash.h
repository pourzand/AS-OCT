#ifndef PCL_POINT_HASH
#define PCL_POINT_HASH

#include <boost/functional/hash.hpp>

namespace pcl
{

	struct PointHash
	{
		template <class PointType>
		std::size_t operator()(const PointType& p) const
		{
			std::size_t seed = 0;
			for (int i=0; i<p.getDimension(); ++i)
				boost::hash_combine(seed, p[i]);
			return seed;
		}
	};

}

#endif