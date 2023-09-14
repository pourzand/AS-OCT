#ifndef PCL2_CHESSBOARD_DISTANCE_MEASURE
#define PCL2_CHESSBOARD_DISTANCE_MEASURE

#include <pcl/image.h>
#include <pcl/math.h>
#include <math.h>

namespace pcl
{
	namespace filter2
	{

		class ChessboardDistanceMeasure
		{
		public:
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b, const Point3D<double>& spacing) const
			{
				double max_val = ::pcl::abs(a[0]-b[0])*spacing[0];
				for (int i=1; i<3; ++i) max_val = std::max(max_val, ::pcl::abs(a[i]-b[i])*spacing[i]);
				return max_val;
			}
			template <class PT1, class PT2>
			inline double operator()(const PT1& a, const PT2& b) const
			{
				double max_val = ::pcl::abs(a[0]-b[0]);
				for (int i=1; i<3; ++i) max_val = std::max(max_val, ::pcl::abs(a[i]-b[i]));
				return max_val;
			}
		};

	}
}

#endif