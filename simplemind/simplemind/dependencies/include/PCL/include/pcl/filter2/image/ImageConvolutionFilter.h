#pragma once

#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/ImageWindowIterator.h>
#include <pcl/filter2/image/ImageFilterBase.h>

namespace pcl
{
	namespace filter2
	{

		template <class BoundaryType, class KernelImageType, class OutputImageType, class InternalValueType=typename KernelImageType::IoValueType >
		class ImageConvolutionFilter: public ImageFilterBase
		{
		public:
			static typename OutputImageType::Pointer Compute(const BoundaryType& input, const typename KernelImageType::ConstantPointer& kernel, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ImageConvolutionFilter filter;
				filter.setInput(input);
				filter.setKernel(kernel);
				filter.setOutputRegion(output_region);
				filter.update();
				return filter.getOutput();
			}


			ImageConvolutionFilter() {}

			void setInput(const BoundaryType& input)
			{
				m_Input = input;
			}

			void setKernel(const typename KernelImageType::ConstantPointer& kernel)
			{
				m_Kernel = kernel;
			}
			
			void update()
			{
				//Setting up environment
				m_Output = this->createImage<OutputImageType>(m_Input.getImage());

				misc::SafeUnsafeRegionGenerator region_gen(
					m_Input.getImage()->getRegion(),
					m_Input.getImage()->getRegion(), 
					m_Kernel->getRegion()
					);
				ImageRegionsIteratorWithPoint<> input_iter(m_Input.getImage());
				ImageRegionsIterator<> output_iter(m_Output);
				input_iter.add(region_gen.getSafeRegion(), 1); output_iter.add(region_gen.getSafeRegion(), 1);
				input_iter.addList(region_gen.getUnsafeRegion(), 0); output_iter.addList(region_gen.getUnsafeRegion(), 0);
				ImageIterator kernel_iter(m_Kernel, ImageIterator::RX, ImageIterator::RY, ImageIterator::RZ);
				iterator::ImageWindowIteratorWithPoint window_iter(m_Input.getImage(), m_Kernel->getRegion());
				
				pcl_ForIterator2(input_iter, output_iter) {
					InternalValueType buffer = 0;
					window_iter.setWindowOrigin(input_iter.getPoint(), input_iter);
					if (input_iter.getInfo()) {
						pcl_ForIterator2(window_iter, kernel_iter) {
							buffer += m_Input.getImage()->get(window_iter)*m_Kernel->get(kernel_iter);
						}
					} else {
						pcl_ForIterator2(window_iter, kernel_iter) {
							buffer += m_Input.get(window_iter)*m_Kernel->get(kernel_iter);
						}
					}
					m_Output->set(output_iter, buffer);
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			BoundaryType m_Input;
			typename KernelImageType::ConstantPointer m_Kernel;
			typename OutputImageType::Pointer m_Output;
		};

	}
}
