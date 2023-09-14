#ifndef PCL_FILTER_KERNEL_REGION_HELPER
#define PCL_FILTER_KERNEL_REGION_HELPER

#include <pcl/misc/DerivativeKernelGenerator.h>
#include <pcl/misc/GaussKernel1DGenerator.h>

namespace pcl
{
	namespace filter
	{
	
		class FilterKernelRegionHelper
		{
		public:
			template <class ImagePointer>
			static Region3D<int> GetGaussianKernelRegion(const ImagePointer& image, double sigmax, double sigmay, double sigmaz, bool use_image_spacing, double cutoff=0.0001, double max_kernel_width=33)
			{
				double sigma[3] = {sigmax, sigmay, sigmaz};
				if (use_image_spacing) {
					for (int i=0; i<3; ++i) sigma[i] /= image->getSpacing()[i];
				}
				Region3D<int> region; region.reset();
				misc::GaussianKernel1DGenerator kernel_gen;
				for (int i=0; i<3; ++i) if (sigma[i]>0) {
					auto kernel = kernel_gen.getKernel((misc::GaussianKernel1DGenerator::Axis) i, sigma[i], cutoff, (max_kernel_width-1)/2);
					region.add(kernel->getRegion());
				}
				return region;
			}
			
			static Region3D<int> GetFiniteDifferenceKernelRegion(int order_x, int order_y=0, int order_z=0)
			{
				misc::DerivativeKernelGenerator gen;
				auto kernel = gen.getDerivativeKernel(order_x, order_y, order_z);
				return kernel->getRegion();
			}

			static Region3D<int> GetHessianKernelRegion()
			{
				Region3D<int> region; region.reset();
				region.add(GetFiniteDifferenceKernelRegion(2,0,0));
				region.add(GetFiniteDifferenceKernelRegion(0,2,0));
				region.add(GetFiniteDifferenceKernelRegion(0,0,2));
				region.add(GetFiniteDifferenceKernelRegion(1,1,0));
				region.add(GetFiniteDifferenceKernelRegion(1,0,1));
				region.add(GetFiniteDifferenceKernelRegion(0,1,1));
				return region;
			}
		};

	}
}

#endif