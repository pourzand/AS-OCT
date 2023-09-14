#ifndef PCL_IMAGE_GAUSSIAN_FILTER
#define PCL_IMAGE_GAUSSIAN_FILTER

#include <pcl/filter/image/ImageFilterBase.h>
#include <pcl/misc/GaussKernel1DGenerator.h>

namespace pcl
{
	namespace filter
	{

		template <class InputImageType, class OutputImageType, template<class> class BoundaryHandlerClass=RepeatingBoundaryHandler>
		class ImageGaussianFilter: public ImageFilterBase
		{
		public:
			typedef typename ImageConvolutionFilter<OutputImageType, misc::GaussianKernel1DGenerator::KernelType, OutputImageType, BoundaryHandlerClass>::BoundaryHandlerType BoundaryHandlerType;

			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, double sigmax, double sigmay, double sigmaz, bool use_image_spacing, int max_kernel_width, double cutoff, const Region3D<int>& process_region=Region3D<int>().reset())
			{
				ImageGaussianFilter filter;
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setSigma(sigmax, sigmay, sigmaz);
				filter.setUseImageSpacing(use_image_spacing);
				filter.setMaxKernelWidth(max_kernel_width);
				filter.setKernelCutoff(cutoff);
				filter.update();
				return filter.getOutput();
			}

			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, double sigmax, double sigmay, double sigmaz, bool use_image_spacing, const Region3D<int>& process_region=Region3D<int>().reset())
			{
				ImageGaussianFilter filter;
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setSigma(sigmax, sigmay, sigmaz);
				filter.setUseImageSpacing(use_image_spacing);
				filter.update();
				return filter.getOutput();
			}


			ImageGaussianFilter() 
			{
				for (int i=0; i<3; i++) m_Sigma[i] = -1;
				m_UseImageSpacing = true;
				m_MaxKernelWidth = 33;
				m_KernelCutoff =  0.0001;
			}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
			}

			void setSigma(double sx, double sy, double sz)
			{
				m_Sigma[0] = sx;
				m_Sigma[1] = sy;
				m_Sigma[2] = sz;
			}
			void setSigma(int index, double sigma)
			{
				m_Sigma[index] = sigma;
			}

			void setUseImageSpacing(bool stat)
			{
				m_UseImageSpacing = stat;
			}

			void setMaxKernelWidth(int width)
			{
				m_MaxKernelWidth = width;
			}

			void setKernelCutoff(double cutoff)
			{
				m_KernelCutoff = cutoff;
			}
			
			BoundaryHandlerType& getBoundaryHandler() 
			{
				return m_BoundaryHandler;
			}

			void update()
			{
				//Set up sigma
				double sigma[3];
				for (int i=0; i<3; i++) {
					if (m_UseImageSpacing) {
						sigma[i] = m_Sigma[i]/m_Input->getSpacing()[i];
					} else {
						sigma[i] = m_Sigma[i];
					}
				}

				//Setting up environment
				this->m_ProcessRegion.setIntersect(m_Input->getRegion()); //Making sure that process region is within the input
				m_Output = OutputImageType::New(m_Input);
				if (sigma[0]<=0 && sigma[1]<=0 && sigma[2]<=0) {	
					ImageHelper::Copy(m_Input, m_Output, this->m_ProcessRegion.getMinPoint(), this->m_ProcessRegion.getMaxPoint());
					return;
				}

				//Computing the kernel
				misc::GaussianKernel1DGenerator kernel_gen;
				misc::GaussianKernel1DGenerator::KernelType::Pointer kernel[3];
				for (int i=0; i<3; i++) if (sigma[i]>0) {
					kernel[i] = kernel_gen.getKernel((misc::GaussianKernel1DGenerator::Axis) i, sigma[i], m_KernelCutoff, (m_MaxKernelWidth-1)/2);
				}
				
				//Actual processing
				bool init = true;
				for (int kernel_index=0; kernel_index<3; kernel_index++) if (m_Sigma[kernel_index]>0) {
					//Computing touched region (actual region that need to be processed)
					Region3D<int> touched_region(this->m_ProcessRegion);
					for (int i=kernel_index+1; i<3; i++) if (sigma[i]>0) {
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
					ImageConvolutionFilter<OutputImageType, misc::GaussianKernel1DGenerator::KernelType, OutputImageType, BoundaryHandlerClass> filter;
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
			double m_Sigma[3];
			bool m_UseImageSpacing;
			double m_KernelCutoff;
			int m_MaxKernelWidth;
			BoundaryHandlerType m_BoundaryHandler;
		};

	}
}

#endif