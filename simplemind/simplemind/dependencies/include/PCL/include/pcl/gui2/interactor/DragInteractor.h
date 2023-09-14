#ifndef PCL_GUI_DRAG_INTERACTOR
#define PCL_GUI_DRAG_INTERACTOR

#include <pcl/gui2/interactor/Interactor.h>
#include <pcl/gui2/interactor/activator/Activator.h>
#include <pcl/gui2/view/View.h>

namespace pcl
{
	namespace gui
	{

		class DragInteractor: public Interactor
		{
		public:
			typedef DragInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { DRAGSTATE=1 };

			void setActivator(Activator::Pointer activator)
			{
				m_Activator = activator;
			}

			virtual void mouseMoveEvent(View* view, QMouseEvent* event)
			{
				if (isState(DRAGSTATE)) drag(view, event);
			}

			virtual void mousePressEvent(View* view, QMouseEvent* event)
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(Activator::PRESS, event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(Activator::PRESS, event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

			virtual void mouseReleaseEvent(View* view, QMouseEvent* event)
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(Activator::RELEASE, event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(Activator::RELEASE, event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

			virtual void keyPressEvent(View* view, QKeyEvent* event) 
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(Activator::PRESS, event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(Activator::PRESS, event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

			virtual void keyReleaseEvent(View* view, QKeyEvent* event) 
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(Activator::RELEASE, event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(Activator::RELEASE, event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

			virtual void mouseDoubleClickEvent(View* view, QMouseEvent* event) 
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(Activator::DOUBLECLICK, event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(Activator::DOUBLECLICK, event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

			virtual void wheelEvent(View* view, QWheelEvent* event) 
			{
				if (getState()==NULLSTATE) {
					if (m_Activator->isActivated(event)) {
						setState(DRAGSTATE);
						dragStart(view, event);
					}
				} else if (getState()==DRAGSTATE) {
					if (m_Activator->isDeactivated(event)) {
						setState(NULLSTATE);
						dragEnd(view, event);
					}
				}
			}

		protected:
			Activator::Pointer m_Activator;

			DragInteractor(Activator::Pointer activator): m_Activator(activator)
			{}

			virtual void drag(View* view, QMouseEvent* event)=0;

			virtual void dragStart(View* view, QMouseEvent* event) {}
			virtual void dragStart(View* view, QKeyEvent* event) {}
			virtual void dragStart(View* view, QWheelEvent* event) {}

			virtual void dragEnd(View* view, QMouseEvent* event) {}
			virtual void dragEnd(View* view, QKeyEvent* event) {}
			virtual void dragEnd(View* view, QWheelEvent* event) {}
		};

	}
}

#endif