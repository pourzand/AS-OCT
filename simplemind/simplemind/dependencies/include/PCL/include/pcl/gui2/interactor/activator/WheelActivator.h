#ifndef PCL_GUI_WHEEL_ACTIVATOR
#define PCL_GUI_WHEEL_ACTIVATOR

#include <pcl/gui2/interactor/activator/Activator.h>

namespace pcl
{
	namespace gui
	{

		class WheelActivator: public Activator
		{
		public:
			typedef WheelActivator Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(Qt::KeyboardModifiers modifiers = Qt::NoModifier)
			{
				return Pointer(new Self(modifiers));
			}

			virtual bool isActivated(QWheelEvent* event)
			{
				if (event->modifiers()==m_Modifiers) return true;
				else return false;
			}

		protected:
			Qt::KeyboardModifiers m_Modifiers;

			WheelActivator(Qt::KeyboardModifiers m): m_Modifiers(m)
			{}
		};

	}
}

#endif