#ifndef PCL_EUCLIDEAN_DISTANCE_SQUARE_MEASURE
#define PCL_EUCLIDEAN_DISTANCE_SQUARE_MEASURE

#include <pcl/image.h>
#include <pcl/math.h>
#include <math.h>

namespace pcl
{
	namespace filter
	{

		class EuclideanDistanceSquareMeasure
		{
		public:
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b, const Point3D<double>& spacing) const
			{
				double result = 0;
				for (int i=0; i<3; ++i) result += ::pcl::square((a[i]-b[i])*spacing[i]);
				return result;
			}
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b) const
			{
				return a.getEuclideanDistanceSqr(b);
			}
		};

	}
}

#endif