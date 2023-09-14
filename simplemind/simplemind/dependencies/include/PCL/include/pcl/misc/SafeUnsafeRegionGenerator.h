#ifndef PCL_SAFE_UNSAFE_REGION_GENERATOR
#define PCL_SAFE_UNSAFE_REGION_GENERATOR

#include <pcl/geometry.h>
#include <list>

namespace pcl
{
	namespace misc
	{
		using namespace pcl;

		class SafeUnsafeRegionGenerator
		{
		public:
			SafeUnsafeRegionGenerator(const Region3D<int>& full, const Region3D<int>& kernel)
			{
				process(full, full, kernel);
			}
			SafeUnsafeRegionGenerator(const Region3D<int>& needed, const Region3D<int>& full, const Region3D<int>& kernel)
			{
				process(needed, full, kernel);
			}

			inline const Region3D<int>& getSafeRegion() const
			{
				return m_SafeRegion;
			}

			inline const std::list<Region3D<int> >& getUnsafeRegion() const
			{
				return m_UnsafeRegion;
			}

		protected:
			Region3D<int> m_SafeRegion;
			std::list<Region3D<int> > m_UnsafeRegion;

			void process(const Region3D<int>& needed, const Region3D<int>& full, const Region3D<int>& kernel)
			{
				bool is_kernel_smaller = true;
				for (int i=0; i<3; i++) {
					if (kernel.getSize((Region3D<int>::Axis)i) > full.getSize((Region3D<int>::Axis)i)) {
						is_kernel_smaller = false;
						break;
					}
				}
				if (!is_kernel_smaller) {
					m_SafeRegion.reset();
					m_UnsafeRegion.push_back(needed);
					return;
				}

				Region3D<int> touched_region(needed);
				touched_region.getMinPoint() += kernel.getMinPoint();
				touched_region.getMaxPoint() += kernel.getMaxPoint();

				//Computing region where processing will stay well within the full region
				m_SafeRegion = touched_region;
				m_SafeRegion.setIntersect(full);
				m_SafeRegion.getMinPoint() -= kernel.getMinPoint();
				m_SafeRegion.getMaxPoint() -= kernel.getMaxPoint();
				//Computing region where checks for boundary are needed
				needed.getRegionsAfterSubtractionBy(m_SafeRegion, m_UnsafeRegion);
			}
		};

	}
}

#endif