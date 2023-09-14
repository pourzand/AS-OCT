#ifndef PCL_MATRIX_POINT_HELPER
#define PCL_MATRIX_POINT_HELPER

#include <pcl/geometry/Point.h>
#include <vnl/vnl_matrix_fixed.h>

namespace pcl
{
	namespace geometry
	{

		class MatrixPointHelper
		{
		public:
			template <class OutputPointType, class InputMatrixType, class InputPointType>
			static OutputPointType MultiplyMatrixWithPoint(const InputMatrixType& a, const InputPointType& b)
			{
				OutputPointType result;
				if (result.getDimension()!=b.getDimension() || a.cols()!=b.getDimension()) pcl_ThrowException(pcl::Exception(), "Invalid input dimension!");
				for (int r=0; r<a.rows(); ++r) {
					double temp = 0;
					for (int c=0; c<a.cols(); ++c) temp += a(r,c)*b[c];
					result[r] = temp;
				}
				return result;
			}

			template <class OutputPointType, class InputPointType, class InputMatrixType>
			static OutputPointType MultiplyPointWithMatrix(const InputPointType& a, const InputMatrixType& b)
			{
				OutputPointType result;
				if (result.getDimension()!=a.getDimension() || b.rows()!=a.getDimension()) pcl_ThrowException(pcl::Exception(), "Invalid input dimension!");
				for (int c=0; c<b.cols(); ++c) {
					double temp = 0;
					for (int r=0; r<b.rows(); ++r) temp += b(r,c)*a[r];
					result[c] = temp;
				}
				return result;
			}
		};

	}
}

#endif