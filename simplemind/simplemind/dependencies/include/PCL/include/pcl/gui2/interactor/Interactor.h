#ifndef PCL_GUI_INTERACTOR
#define PCL_GUI_INTERACTOR

#include <boost/smart_ptr.hpp>
#include <QWheelEvent>

namespace pcl
{
	namespace gui
	{
		class View;

		class Interactor
		{
		public:
			typedef Interactor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { NULLSTATE=0 };

			virtual ~Interactor() {}

			void enable(bool en)
			{
				m_Enable = en;
			}

			bool isEnabled() const
			{
				return m_Enable;
			}

			virtual void wheelEvent(View*, QWheelEvent*) {}
			virtual void mouseMoveEvent(View*, QMouseEvent*) {}
			virtual void mousePressEvent(View*, QMouseEvent*) {}
			virtual void mouseReleaseEvent(View*, QMouseEvent*) {}
			virtual void keyPressEvent(View*, QKeyEvent*) {}
			virtual void keyReleaseEvent(View*, QKeyEvent*) {}
			virtual void mouseDoubleClickEvent(View*, QMouseEvent*) {}

			virtual void update(View*) {}

		protected:
			QPoint m_MousePos, m_GlobalMousePos;
			int m_State;
			bool m_Enable;

			Interactor(): m_State(NULLSTATE), m_Enable(true)
			{}

			void setState(int state)
			{
				m_State = state;
			}
			void setIfStateIsNull(int state)
			{
				if (m_State==NULLSTATE) {
					m_State = state;
				}
			}
			int getState() const
			{
				return m_State;
			}
			bool isState(int state) const
			{
				return m_State == state;
			}

			void storeMousePos(QMouseEvent *event)
			{
				m_MousePos = event->pos();
			}
			QPoint& getMousePos()
			{
				return m_MousePos;
			}

			void storeGlobalMousePos(QMouseEvent *event)
			{
				m_GlobalMousePos = event->globalPos();
			}
			QPoint& getGlobalMousePos()
			{
				return m_GlobalMousePos;
			}
		};

	}
}

#endif // VTKIMAGEWIDGETINTERACTORBASE_H
