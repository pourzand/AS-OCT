#ifndef PCL_CITY_BLOCK_DISTANCE_MEASURE
#define PCL_CITY_BLOCK_DISTANCE_MEASURE

#include <pcl/image.h>
#include <pcl/math.h>
#include <math.h>

namespace pcl
{
	namespace filter
	{

		class CityBlockDistanceMeasure
		{
		public:
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b, const Point3D<double>& spacing) const
			{
				double result = 0;
				for (int i=0; i<3; ++i) result += ::pcl::abs(a[i]-b[i])*spacing;
				return result;
			}
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b) const
			{
				double result = 0;
				for (int i=0; i<3; ++i) result += ::pcl::abs(a[i]-b[i]);
				return result;
			}
		};

	}
}

#endif