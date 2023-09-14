#ifndef PCL_GAUSSIAN_KERNEL_1D_GENERATOR
#define PCL_GAUSSIAN_KERNEL_1D_GENERATOR

#include <pcl/image.h>
#include <pcl/constant.h>

#pragma warning (push)
#pragma warning (disable: 4244)

namespace pcl
{
	namespace misc
	{

		class GaussianKernel1DGenerator
		{
		public:
			typedef Image<double> KernelType;

			enum Axis
			{
				X = 0,
				Y = 1,
				Z = 2
			};
			
			static int GetKernelExtent(double sigma, double cutoff, double max_half_width)
			{
				int width = ceil(computeWidth(sigma, cutoff))-1;
				if (max_half_width>0) width = pcl_Min(width, max_half_width);
				return width;
			}

			GaussianKernel1DGenerator() {}

			KernelType::Pointer getKernel(Axis axis, double sigma, double cutoff, double max_half_width)
			{
				int width = ceil(computeWidth(sigma, cutoff))-1;
				if (max_half_width>0) width = pcl_Min(width, max_half_width);

				Point3D<int> minp(0,0,0),
					maxp(0,0,0);
				minp[axis] = -width;
				maxp[axis] = width;

				KernelType::Pointer result = KernelType::New(minp, maxp);

				double in_scale = 0.5/(sigma * sigma);
				long origin = result->localToIndex(0,0,0);
				result->set(origin, 1);
				double area = 1;
				for (int i=1; i<=width; i++) {
					double val = getValueAt(i, in_scale);
					result->set(origin-i, val); area += val;
					result->set(origin+i, val); area += val;
				}

				int sz = 2*width + 1;
				area = 1/area;
				for (int i=0; i<sz; i++) {
					result->set(i, result->get(i)*area);
					//std::cout << result->get(i) << " ";
				}
				//std::cout << std::endl;
				//if (sigma==8) exit(1);

				return result;
			}

		protected:
			static inline double computeWidth(double sigma, double cutoff)
			{
				return sigma * sqrt(-2 * log(cutoff * sigma * sqrt(2*pcl::PI)));
			}

			inline double getValueAt(double x, double in_scale)
			{
				return exp(-x*x * in_scale);
			}
		};

	}
}

#pragma warning (pop)
#endif