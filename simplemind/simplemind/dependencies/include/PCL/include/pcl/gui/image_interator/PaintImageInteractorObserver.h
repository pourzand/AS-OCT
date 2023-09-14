#ifndef PCL_GUI_PAINT_IMAGE_INTERACTOR_OBSERVER
#define PCL_GUI_PAINT_IMAGE_INTERACTOR_OBSERVER

#include <pcl/geometry.h>

namespace pcl
{
	namespace gui
	{

		class PaintImageInteractorObserver
		{
		public:
			virtual void inform(const pcl::Point3D<double>& point, bool added)=0;
		};

	}
}

#endif