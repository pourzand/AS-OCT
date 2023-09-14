#ifndef PCL_GUI_IMAGE_WIDGET_INTERACTOR_BASE
#define PCL_GUI_IMAGE_WIDGET_INTERACTOR_BASE

#include <boost/smart_ptr.hpp>
#include <QWheelEvent>

namespace pcl
{
	namespace gui
	{

		class ImageWidget;

		class ImageWidgetInteractorBase
		{
		public:
			typedef ImageWidgetInteractorBase Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { NullState=0 };

			virtual ~ImageWidgetInteractorBase() {}

			void enable(bool en)
			{
				m_Enable = en;
			}

			bool isEnabled() const
			{
				return m_Enable;
			}

			void setState(int state)
			{
				m_State = state;
			}
			void setIfStateIsNull(int state)
			{
				if (m_State==NullState) {
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
			void storeGlobalMousePos(QMouseEvent *event)
			{
				m_GlobalMousePos = event->globalPos();
			}

			virtual void wheelEvent(ImageWidget* widget, QWheelEvent* event)=0;
			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)=0;
			virtual void mousePressEvent(ImageWidget* widget, QMouseEvent* event)=0;
			virtual void mouseReleaseEvent(ImageWidget* widget, QMouseEvent* event)=0;
			virtual void keyPressEvent(ImageWidget* widget, QKeyEvent* event)=0;
			virtual void keyReleaseEvent(ImageWidget* widget, QKeyEvent* event)=0;
			virtual void mouseDoubleClickEvent(ImageWidget* widget, QMouseEvent* event)=0;

		protected:
			QPoint m_MousePos, m_GlobalMousePos;
			int m_State;
			bool m_Enable;

			ImageWidgetInteractorBase(): m_State(NullState), m_Enable(true)
			{
			}
		};

	}
}

#endif // VTKIMAGEWIDGETINTERACTORBASE_H
