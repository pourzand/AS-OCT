#pragma once

#include <pcl/filter2/image/ImageConvolutionFilter.h>
#include <pcl/filter2/image/ImageFilterBase.h>
#include <pcl/filter2/boundary/ZeroFluxBoundary.h>

namespace pcl
{
	namespace filter2
	{

		template <class BoundaryType, class KernelType, class OutputImageType>
		class ImageSeparableFilter: public ImageFilterBase
		{
		public:
			static typename OutputImageType::Pointer Compute(const BoundaryType& input, const std::vector<typename KernelType::Pointer>& kernels, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ImageSeparableFilter filter;
				filter.setInput(input);
				filter.setKernels(kernels);
				filter.setOutputRegion(output_region);
				filter.update();
				return filter.getOutput();
			}


			ImageSeparableFilter() 
			{}

			void setInput(const BoundaryType& input)
			{
				m_Input = input;
			}
			
			void setKernels(const std::vector<typename KernelType::Pointer>& kernels) 
			{
				m_Kernels = kernels;
			}
			
			void update()
			{
				//Actual processing
				pcl::Region3D<int> output_region = this->m_OutputRegion;
				if (output_region.empty()) m_Input.getImage()->getRegion();
				for (int kernel_index=0; kernel_index<m_Kernels.size(); ++kernel_index) {
					//Computing touched region (actual region that need to be processed)
					Region3D<int> touched_region(output_region);
					for (int i=kernel_index+1; i<m_Kernels.size(); ++i) {
						touched_region.getMinPoint() += m_Kernels[i]->getMinPoint();
						touched_region.getMaxPoint() += m_Kernels[i]->getMaxPoint();
					}
					if (kernel_index==0) {
						m_Output = ImageConvolutionFilter<BoundaryType, KernelType, OutputImageType>::Compute(m_Input, m_Kernels[kernel_index], touched_region);
					} else {
						ZeroFluxBoundary<OutputImageType> boundary(m_Output);
						m_Output = ImageConvolutionFilter<ZeroFluxBoundary<OutputImageType>, KernelType, OutputImageType>::Compute(boundary, m_Kernels[kernel_index], touched_region);
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			BoundaryType m_Input;
			typename OutputImageType::Pointer m_Output;
			std::vector<typename KernelType::Pointer> m_Kernels;
		};

	}
}
