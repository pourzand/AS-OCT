#ifndef PCL_POINT_FINITE_DIFFERENCE_FILTER
#define PCL_POINT_FINITE_DIFFERENCE_FILTER

#include <pcl/filter2/point/PointConvolutionFilter.h>
#include <pcl/misc/DerivativeKernelGenerator.h>

namespace pcl
{
	namespace filter
	{

		template <class BoundaryType, class InternalValueType=misc::DerivativeKernelGenerator::KernelType::IoValueType>
		class PointFiniteDifferenceFilter: public PointConvolutionFilter<BoundaryType, misc::DerivativeKernelGenerator::KernelType>
		{
		public:
			typedef PointFiniteDifferenceFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef BoundaryType BoundaryHandlerType;

			static Pointer New(const BoundaryType& bound, int order_x, int order_y=0, int order_z=0)
			{
				Pointer obj(new Self);
				obj->setInput(bound);
				misc::DerivativeKernelGenerator gen;
				obj->setKernel(gen.getDerivativeKernel(order_x, order_y, order_z));
				obj->update();
				return obj;
			}

		protected:
			PointFiniteDifferenceFilter() {}
		};

	}
}

#endif