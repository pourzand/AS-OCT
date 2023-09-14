#ifndef PCL_POINT_FINITE_DIFFERENCE_FILTER
#define PCL_POINT_FINITE_DIFFERENCE_FILTER

#include <pcl/filter/point/PointConvolutionFilter.h>
#include <pcl/misc/DerivativeKernelGenerator.h>
#include <pcl/filter/boundary_handler/MirroringBoundaryHandler.h>

namespace pcl
{
	namespace filter
	{

		template <class InputImageType, template<class> class BoundaryHandlerClass=MirroringBoundaryHandler, class InternalValueType=misc::DerivativeKernelGenerator::KernelType::IoValueType>
		class PointFiniteDifferenceFilter: public PointConvolutionFilter<InputImageType, misc::DerivativeKernelGenerator::KernelType, BoundaryHandlerClass, InternalValueType>
		{
		public:
			typedef PointFiniteDifferenceFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef BoundaryHandlerClass<InputImageType> BoundaryHandlerType;

			static Pointer New(const typename InputImageType::ConstantPointer& image, int order_x, int order_y=0, int order_z=0)
			{
				Pointer obj(new Self);
				obj->setInput(image);
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