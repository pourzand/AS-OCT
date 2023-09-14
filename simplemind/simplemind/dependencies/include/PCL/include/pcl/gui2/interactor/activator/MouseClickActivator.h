#ifndef PCL_GUI_MOUSE_CLICK_ACTIVATOR
#define PCL_GUI_MOUSE_CLICK_ACTIVATOR

#include <pcl/gui2/interactor/activator/Activator.h>

namespace pcl
{
	namespace gui
	{

		class MouseClickActivator: public Activator
		{
		public:
			typedef MouseClickActivator Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
			{
				return Pointer(new Self(button, modifiers));
			}

			virtual bool isActivated(EventType type, QMouseEvent* event)
			{
				if (type==RELEASE && event->button()==m_Button && event->modifiers()==m_Modifiers) return true;
				else return false;
			}

		protected:
			Qt::MouseButton m_Button;
			Qt::KeyboardModifiers m_Modifiers;

			MouseClickActivator(Qt::MouseButton b, Qt::KeyboardModifiers m): m_Button(b), m_Modifiers(m)
			{}
		};

	}
}

#endif