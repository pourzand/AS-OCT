#ifndef PCL_GUI_FIXED_ACTIVATOR
#define PCL_GUI_FIXED_ACTIVATOR

#include <pcl/gui2/interactor/activator/Activator.h>
namespace pcl
{
	namespace gui
	{

		class FixedActivator: public Activator
		{
		public:
			typedef FixedActivator Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New()
			{
				return Pointer(new Self());
			}

			virtual bool isActivated(EventType type, QMouseEvent* event)
			{
				return true;
			}

			virtual bool isActivated(EventType type, QKeyEvent* event)
			{
				return true;
			}

			virtual bool isActivated(QWheelEvent* event)
			{
				return true;
			}

			virtual bool isDeactivated(EventType type, QMouseEvent* event)
			{
				return false;
			}

			virtual bool isDeactivated(EventType type, QKeyEvent* event)
			{
				return false;
			}

			virtual bool isDeactivated(QWheelEvent* event)
			{
				return false;
			}

		protected:
			FixedActivator() 
			{}
		};

	}
}

#endif