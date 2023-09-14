#ifndef PCL_IMAGE_FINITE_DIFFERENCE_FILTER
#define PCL_IMAGE_FINITE_DIFFERENCE_FILTER

#include <pcl/filter/image/ImageFilterBase.h>
#include <pcl/filter/boundary_handler/MirroringBoundaryHandler.h>
#include <pcl/misc/DerivativeKernelGenerator.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;

		template <class InputImageType, class OutputImageType, template<class> class BoundaryHandlerClass=MirroringBoundaryHandler>
		class ImageFiniteDifferenceFilter: public ImageFilterBase
		{
		public:
			typedef typename ImageConvolutionFilter<OutputImageType, misc::DerivativeKernelGenerator::KernelType, OutputImageType, BoundaryHandlerClass>::BoundaryHandlerType BoundaryHandlerType;

			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, int orderx, int ordery, int orderz, const Region3D<int>& process_region=Region3D<int>().reset())
			{
				ImageFiniteDifferenceFilter filter;
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setOrder(orderx, ordery, orderz);
				filter.update();
				return filter.getOutput();
			}


			ImageFiniteDifferenceFilter() 
			{
				for (int i=0; i<3; i++) m_Order[i] = 0;
			}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
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
			
			BoundaryHandlerType& getBoundaryHandler() 
			{
				return m_BoundaryHandler;
			}

			void update()
			{
				//Setting up environment
				this->m_ProcessRegion.setIntersect(m_Input->getRegion()); //Making sure that process region is within the input
				m_Output = OutputImageType::New(m_Input);
				if (m_Order[0]==0 && m_Order[1]==0 && m_Order[2]==0) {	
					ImageHelper::Copy(m_Input, m_Output, this->m_ProcessRegion.getMinPoint(), this->m_ProcessRegion.getMaxPoint());
					return;
				}

				//Computing the kernel
				misc::DerivativeKernelGenerator kernel_gen;
				misc::DerivativeKernelGenerator::KernelType::Pointer kernel[3];
				for (int i=0; i<3; i++) if (m_Order[i]>0) {
					int order[3] = {0,0,0};
					order[i] = m_Order[i];
					kernel[i] = kernel_gen.getDerivativeKernel(order[0], order[1], order[2]);
				}
				
				//Actual processing
				bool init = true;
				for (int kernel_index=0; kernel_index<3; kernel_index++) if (m_Order[kernel_index]>0) {
					//Computing touched region (actual region that need to be processed)
					Region3D<int> touched_region(this->m_ProcessRegion);
					for (int i=kernel_index+1; i<3; i++) if (m_Order[i]>0) {
						touched_region.setMinPoint(
							(Region3D<int>::Axis)i, 
							this->m_ProcessRegion.getMinPoint()[i] + kernel[i]->getMinPoint()[i]
						);
						touched_region.setMaxPoint(
							(Region3D<int>::Axis)i, 
							this->m_ProcessRegion.getMaxPoint()[i] + kernel[i]->getMaxPoint()[i]
						);
					}
					touched_region.setIntersect(m_Input->getRegion());
					if (init) {
						//Initialize output
						ImageHelper::Copy(m_Input, m_Output, touched_region.getMinPoint(), touched_region.getMaxPoint());
						init = false;
					}
					//Generating output
					ImageConvolutionFilter<OutputImageType, misc::DerivativeKernelGenerator::KernelType, OutputImageType, BoundaryHandlerClass> filter;
					filter.setInput(m_Output);
					filter.setKernel(kernel[kernel_index]);
					filter.setProcessRegion(touched_region);
					filter.getBoundaryHandler() = m_BoundaryHandler;
					filter.update();
					m_Output = filter.getOutput();
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typename InputImageType::ConstantPointer m_Input;
			typename OutputImageType::Pointer m_Output;
			int m_Order[3];
			BoundaryHandlerType m_BoundaryHandler;
		};

	}
}

#endif