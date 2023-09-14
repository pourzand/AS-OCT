#ifndef PCL2_IMAGE_GAUSSIAN_FILTER
#define PCL2_IMAGE_GAUSSIAN_FILTER

#include <pcl/misc/GaussKernel1DGenerator.h>
#include <pcl/filter2/image/ImageFilterBase.h>
#include <pcl/filter2/image/ImageSeparableFilter.h>

namespace pcl
{
	namespace filter2
	{

		template <class BoundaryType, class OutputImageType>
		class ImageGaussianFilter: public ImageFilterBase
		{
		public:
			static typename OutputImageType::Pointer Compute(const BoundaryType& input, double sigmax, double sigmay, double sigmaz, bool use_image_spacing, int max_kernel_width, double cutoff, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ImageGaussianFilter filter;
				filter.setInput(input);
				filter.setSigma(sigmax, sigmay, sigmaz);
				filter.setUseImageSpacing(use_image_spacing);
				filter.setMaxKernelWidth(max_kernel_width);
				filter.setKernelCutoff(cutoff);
				filter.setOutputRegion(output_region);
				filter.update();
				return filter.getOutput();
			}

			static typename OutputImageType::Pointer Compute(const BoundaryType& input, double sigmax, double sigmay, double sigmaz, bool use_image_spacing, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				ImageGaussianFilter filter;
				filter.setInput(input);
				filter.setSigma(sigmax, sigmay, sigmaz);
				filter.setUseImageSpacing(use_image_spacing);
				filter.setOutputRegion(output_region);
				filter.update();
				return filter.getOutput();
			}


			ImageGaussianFilter() 
			{
				for (int i=0; i<3; i++) m_Sigma[i] = -1;
				m_UseImageSpacing = true;
				m_MaxKernelWidth = 33;
				m_KernelCutoff = 0.0001;
			}

			void setInput(const BoundaryType& input)
			{
				m_Input = input;
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
			
			void update()
			{
				//Set up sigma
				double sigma[3];
				for (int i=0; i<3; i++) {
					if (m_UseImageSpacing) {
						sigma[i] = m_Sigma[i]/m_Input.getImage()->getSpacing()[i];
					} else {
						sigma[i] = m_Sigma[i];
					}
				}

				//Computing the kernel
				std::vector<misc::GaussianKernel1DGenerator::KernelType::Pointer> kernels;
				kernels.reserve(3);
				misc::GaussianKernel1DGenerator kernel_gen;
				for (int i=0; i<3; i++) if (sigma[i]>0) {
					kernels.push_back(kernel_gen.getKernel((misc::GaussianKernel1DGenerator::Axis) i, sigma[i], m_KernelCutoff, (m_MaxKernelWidth-1)/2));
				}
				
				//Actual processing
				m_Output = ImageSeparableFilter<BoundaryType, misc::GaussianKernel1DGenerator::KernelType, OutputImageType>::Compute(m_Input, kernels, this->m_OutputRegion);
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			BoundaryType m_Input;
			typename OutputImageType::Pointer m_Output;
			double m_Sigma[3];
			bool m_UseImageSpacing;
			double m_KernelCutoff;
			int m_MaxKernelWidth;
		};

	}
}

#endif