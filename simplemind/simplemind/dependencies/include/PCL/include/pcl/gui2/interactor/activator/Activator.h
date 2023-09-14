#ifndef PCL_GUI_ACTIVATOR
#define PCL_GUI_ACTIVATOR

#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>

namespace pcl
{
	namespace gui
	{

		class Activator
		{
		public:
			typedef Activator Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum EventType { PRESS, RELEASE, DOUBLECLICK };

			static Pointer New()
			{
				return Pointer(new Self());
			}

			virtual bool isActivated(EventType type, QMouseEvent* event)
			{
				return false;
			}

			virtual bool isActivated(EventType type, QKeyEvent* event)
			{
				return false;
			}

			virtual bool isActivated(QWheelEvent* event)
			{
				return false;
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
			Activator() 
			{}
		};

	}
}

#endif