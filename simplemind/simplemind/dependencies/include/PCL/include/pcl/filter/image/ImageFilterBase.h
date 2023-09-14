#ifndef PCL_IMAGE_FILTER_BASE
#define PCL_IMAGE_FILTER_BASE

#include <pcl/image.h>

namespace pcl
{
	namespace filter
	{

		class ImageFilterBase: private boost::noncopyable
		{
		public:
			void setProcessRegion(const Region3D<int>& reg)
			{
				m_ProcessRegion = reg;
			}

		protected:
			Region3D<int> m_ProcessRegion;
			
			ImageFilterBase()
			{
				m_ProcessRegion.reset();
			}
		};

	}
}

#endif