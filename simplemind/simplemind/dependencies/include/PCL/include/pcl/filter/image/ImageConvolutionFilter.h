#ifndef PCL_IMAGE_CONVOLUTION_FILTER
#define PCL_IMAGE_CONVOLUTION_FILTER

#include <pcl/filter/image/ImageFilterBase.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/ImageWindowIterator.h>

namespace pcl
{
	namespace filter
	{

		template <class InputImageType, class KernelImageType, class OutputImageType, template <class> class BoundaryHandlerClass=RepeatingBoundaryHandler, class InternalValueType=typename KernelImageType::IoValueType >
		class ImageConvolutionFilter: public ImageFilterBase
		{
		public:
			typedef BoundaryHandlerClass<InputImageType> BoundaryHandlerType;

			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const typename KernelImageType::ConstantPointer& kernel, const Region3D<int>& process_region=Region3D<int>().reset())
			{
				ImageConvolutionFilter filter;
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setKernel(kernel);
				filter.update();
				return filter.getOutput();
			}


			ImageConvolutionFilter() {}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
			}

			void setKernel(const typename KernelImageType::ConstantPointer& kernel)
			{
				m_Kernel = kernel;
			}
			
			BoundaryHandlerType& getBoundaryHandler() 
			{
				return m_BoundaryHandler;
			}

			void update()
			{
				//Setting up environment
				m_BoundaryHandler.setImage(m_Input);
				this->m_ProcessRegion.setIntersect(m_Input->getRegion()); //Making sure that process region is within the input
				m_Output = OutputImageType::New(m_Input); //Output is created based on the input as template (same buffer size!)

				misc::SafeUnsafeRegionGenerator region_gen(
					m_ProcessRegion,
					m_Input->getRegion(), 
					m_Kernel->getRegion()
					);
				ImageIterator kernel_iter(m_Kernel, ImageIterator::RX, ImageIterator::RY, ImageIterator::RZ);

				if (!region_gen.getSafeRegion().empty()) {
					//Processing safe region
					iterator::ImageWindowIterator safe_window_iter(m_Input, m_Kernel->getRegion());
					ImageIterator safe_image_iter(m_Input);
					safe_image_iter.setRegion(region_gen.getSafeRegion());
					pcl_ForIterator(safe_image_iter) {
						InternalValueType buffer = 0;
						safe_window_iter.setWindowOrigin(safe_image_iter);
						for (safe_window_iter.begin(), kernel_iter.begin(); !kernel_iter.end(); safe_window_iter.next(), kernel_iter.next()) {
							buffer += m_Input->get(safe_window_iter)*m_Kernel->get(kernel_iter);
						}
						m_Output->set(safe_image_iter, (typename OutputImageType::IoValueType)buffer);
					}
				}
				//Processing unsafe region
				{
					ImageIteratorWithPoint image_iter(m_Input);
					iterator::ImageWindowIteratorWithPoint window_iter(m_Input, m_Kernel->getRegion());
					pcl_ForEach(region_gen.getUnsafeRegion(), item) {
						image_iter.setRegion(*item);
						pcl_ForIterator(image_iter) {
							InternalValueType buffer = 0;
							window_iter.setWindowOrigin(image_iter.getPoint(), image_iter);
							for (window_iter.begin(), kernel_iter.begin(); !kernel_iter.end(); window_iter.next(), kernel_iter.next()) {
								buffer += m_BoundaryHandler.get(window_iter.getPoint(),window_iter) * m_Kernel->get(kernel_iter);
							}
							m_Output->set(image_iter, (typename OutputImageType::IoValueType)buffer);
						}
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typename InputImageType::ConstantPointer m_Input;
			typename KernelImageType::ConstantPointer m_Kernel;
			typename OutputImageType::Pointer m_Output;
			BoundaryHandlerType m_BoundaryHandler;
		};

	}
}

#endif