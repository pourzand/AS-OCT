#ifndef PCL_GUI_BRIGHTNESS_CONTRAST_IMAGE_INTERACTOR_OBSERVER
#define PCL_GUI_BRIGHTNESS_CONTRAST_IMAGE_INTERACTOR_OBSERVER

#include <pcl/gui/ImageWidget.h>

namespace pcl
{
	namespace gui
	{

		class BrightnessConstrastImageInteractorObserver
		{
		public:
			virtual void inform(double offset, double width)=0;
		};

	}
}

#endif