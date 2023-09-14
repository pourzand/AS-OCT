#ifndef PCL_POINT_HELPER
#define PCL_POINT_HELPER

#include <pcl/geometry/Point.h>
#include <math.h>

namespace pcl
{

	class PointHelper
	{
	public:
		template <class ReturnPointType, class InputPointType>
		static ReturnPointType EuclideanToSpherical(const InputPointType& input) 
		{
			double r = sqrt(input.x()*input.x() + input.y()*input.y() + input.z()*input.z());
			return ReturnPointType(
				r,
				acos(input.z()/r),
				atan(input.y()/input.x())
				);
		}

		template <class ReturnPointType, class InputPointType>
		static ReturnPointType SphericalToEuclidean(const InputPointType& input) 
		{
			double sin_theta = sin(input.y());
			return ReturnPointType(
				input.x()*sin_theta*cos(input.z()),
				input.x()*sin_theta*sin(input.z()),
				input.x()*cos(input.y())
				);
		}
	};

}

#endif