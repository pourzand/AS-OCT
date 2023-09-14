#ifndef PCL_DERIVATIVE_KERNEL_GENERATOR
#define PCL_DERIVATIVE_KERNEL_GENERATOR

#include <pcl/image.h>
#include <vector>

#pragma warning (push)
#pragma warning (disable: 4018)

namespace pcl
{
	namespace misc
	{
		namespace derivative_kernel_generator_details
		{
			static const double m_BaseKernel1[3] = {-0.5, 0, 0.5};
			static const double m_BaseKernel2[3] = {1, -2, 1};
		}
		
		class DerivativeKernelGenerator { //Results are preflipped to be used with convolution!
		public:
			typedef Image<double> KernelType;

			DerivativeKernelGenerator() {}
			
			KernelType::Pointer getDerivativeKernel(int order_x, int order_y, int order_z)
			{
				int order[] = {order_x, order_y, order_z};
				Point3D<int> kernel_minp(0,0,0);
				std::vector<BaseKernelType> base_kernel;
				std::vector<int> base_kernel_axis;
				for (int i=0; i<3; i++) {
					if (order[i]!=0) {
						base_kernel.push_back(getDerivativeKernel(order[i]));
						base_kernel_axis.push_back(i);
						kernel_minp[i] = -((int)base_kernel.back()->size() - 1)/2;
					}
				}
				//Creating the actual kernel
				Image<double>::Pointer kernel = Image<double>::New(kernel_minp, -kernel_minp);
				ImageHelper::Fill(kernel, 0);
				kernel->set(0,0,0, 1);
				//Populating the kernel
				for (int i=0; i<base_kernel.size(); i++) {
					std::vector<double> &cur_base_kernel = *(base_kernel[i]);
					int offset = static_cast<int>((cur_base_kernel.size()-1)/2);
					int cur_axis = base_kernel_axis[i];
					//Iterating through the plane of the axis
					Point3D<int> minp(kernel->getMinPoint()),
						maxp(kernel->getMaxPoint());
					minp[cur_axis] = 0;
					maxp[cur_axis] = 0;
					pcl_ForXYZ(x,y,z, minp, maxp) {
						double val = kernel->get(x,y,z);
						if (val!=0) {
							Point3D<int> p(x,y,z);
							for (int i=0; i<cur_base_kernel.size(); i++) {
								p[cur_axis] = i-offset;
								kernel->set(p, val*cur_base_kernel[i]);
							}
						}
					}
				}
				return kernel;
			}

		protected:
			typedef boost::shared_ptr<std::vector<double> > BaseKernelType;

			BaseKernelType getDerivativeKernel(int order) {
				BaseKernelType org(new std::vector<double>(1,1));
				return getDerivativeKernel(org, order);
			}
			BaseKernelType getDerivativeKernel(BaseKernelType org, int order) {
				if (order==0) return org;
				BaseKernelType result(new std::vector<double>(org->size()+2, 0));
				//Determining the base kernel
				double const *kernel;
				if (order==1) {
					kernel = &derivative_kernel_generator_details::m_BaseKernel1[0];
					order = 0;
				} else {
					kernel = &derivative_kernel_generator_details::m_BaseKernel2[0];
					order -= 2;
				}
				//Actual calculation
				for (int result_index=0; result_index<result->size(); result_index++) {
					for (int j=-1; j<=1; j++) {
						int org_index = (result_index-1)+j,
							kernel_index = 1+j;
						if (org_index>=0 && org_index<org->size()) {
							(*result)[result_index] += kernel[kernel_index]*(*org)[org_index];
						}
					}
				}
				return getDerivativeKernel(result, order);
			}
		};

	}
}

#pragma warning (pop)
#endif