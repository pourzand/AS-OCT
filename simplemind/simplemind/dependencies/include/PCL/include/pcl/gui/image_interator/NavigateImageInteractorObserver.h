#ifndef PCL_GUI_NAVIGATE_IMAGE_INTERACTOR_OBSERVER
#define PCL_GUI_NAVIGATE_IMAGE_INTERACTOR_OBSERVER

#include <pcl/gui/ImageWidget.h>

namespace pcl
{
	namespace gui
	{

		class NavigateImageInteractorObserver
		{
		public:
			virtual void inform(ImageWidget* widget, const pcl::Point3D<double>& coord)=0;
		};

	}
}

#endif