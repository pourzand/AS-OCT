#ifndef PCL_IMAGE_FILTER2_BASE
#define PCL_IMAGE_FILTER2_BASE

#include <pcl/image.h>

namespace pcl
{
	namespace filter2
	{

		class ImageFilterBase: private boost::noncopyable
		{
		public:
			void setOutputRegion(const Region3D<int>& reg)
			{
				m_OutputRegion = reg;
			}

		protected:
			Region3D<int> m_OutputRegion;
			
			ImageFilterBase()
			{
				m_OutputRegion.reset();
			}
			
			template <class T, class ImagePointerType>
			typename T::Pointer createImage(ImagePointerType input)
			{
				if (m_OutputRegion.empty()) {
					return T::New(input);
				} else {
					if (T::UseOrientationMatrix)
						return T::New(m_OutputRegion.getMinPoint(), m_OutputRegion.getMaxPoint(), input->getSpacing(), input->getOrigin(), input->getOrientationMatrix());
					else
						return T::New(m_OutputRegion.getMinPoint(), m_OutputRegion.getMaxPoint(), input->getSpacing(), input->getOrigin());
				}
			}

			template <class T, class ImagePointerType>
			typename T::Pointer createImage(ImagePointerType input, const pcl::Region3D<int>& region)
			{
				if (T::UseOrientationMatrix)
					return T::New(region.getMinPoint(), region.getMaxPoint(), input->getSpacing(), input->getOrigin(), input->getOrientationMatrix());
				else
					return T::New(region.getMinPoint(), region.getMaxPoint(), input->getSpacing(), input->getOrigin());
			}

			template <class ImagePointerType>
			pcl::Region3D<int> getOutputRegion(ImagePointerType input)
			{
				if (m_OutputRegion.empty()) return input->getRegion();
				return m_OutputRegion;
			}
		};

	}
}

#endif