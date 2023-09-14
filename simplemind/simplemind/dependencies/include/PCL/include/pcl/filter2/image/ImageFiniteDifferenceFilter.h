#ifndef PCL2_IMAGE_FINITE_DIFFERENCE_FILTER
#define PCL2_IMAGE_FINITE_DIFFERENCE_FILTER

#include <pcl/misc/DerivativeKernelGenerator.h>
#include <pcl/filter2/image/ImageFilterBase.h>
#include <pcl/filter2/image/ImageSeparableFilter.h>

namespace pcl
{
	namespace filter2
	{
		using namespace pcl;

		template <class BoundaryType, class OutputImageType>
		class ImageFiniteDifferenceFilter
		{
		public:
			static typename OutputImageType::Pointer Compute(const BoundaryType& input, int orderx, int ordery, int orderz, const Region3D<int>& output_region=Region3D<int>().reset())
			{
				ImageFiniteDifferenceFilter filter;
				filter.setOutputRegion(output_region);
				filter.setInput(input);
				filter.setOrder(orderx, ordery, orderz);
				filter.update();
				return filter.getOutput();
			}


			ImageFiniteDifferenceFilter() 
			{
				for (int i=0; i<3; i++) m_Order[i] = 0;
			}

			void setInput(const BoundaryType& input)
			{
				m_Input = input;
			}

			void setOrder(int ox, int oy, int oz)
			{
				m_Order[0] = ox;
				m_Order[1] = oy;
				m_Order[2] = oz;
			}
			void setOrder(int index, int order)
			{
				m_Order[index] = order;
			}

			void update()
			{
				//Computing the kernel
				misc::DerivativeKernelGenerator kernel_gen;
				std::vector<misc::DerivativeKernelGenerator::KernelType::Pointer> kernels;
				kernels.reserve(3);
				for (int i=0; i<3; i++) if (m_Order[i]>0) {
					int order[3] = {0,0,0};
					order[i] = m_Order[i];
					kernels.push_back(kernel_gen.getDerivativeKernel(order[0], order[1], order[2]));
				}
				//Actual processing
				m_Output = ImageSeparableFilter<BoundaryType, misc::DerivativeKernelGenerator::KernelType, OutputImageType>::Compute(m_Input, kernels, this->m_OuputRegion);
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			BoundaryType m_Input;
			typename OutputImageType::Pointer m_Output;
			int m_Order[3];
			BoundaryHandlerType m_BoundaryHandler;
		};

	}
}

#endif