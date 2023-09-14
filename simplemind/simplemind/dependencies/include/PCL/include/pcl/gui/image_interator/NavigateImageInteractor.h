#ifndef PCL_GUI_NAVIGATE_IMAGE_INTERACTOR
#define PCL_GUI_NAVIGATE_IMAGE_INTERACTOR

#include <pcl/gui/ImageWidgetGroupController.h>
#include <pcl/gui/ImageWidgetInteractorBase.h>
#include <pcl/gui/image_interator/NavigateImageInteractorObserver.h>

namespace pcl
{
	namespace gui
	{

		class NavigateImageInteractor: public ImageWidgetInteractorBase
		{
		public:
			typedef NavigateImageInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { SetCenter=1 };

			static Pointer New(const ImageWidgetGroupController::Pointer& controller)
			{
				return Pointer(new Self(controller));
			}

			void setObserver(NavigateImageInteractorObserver* observer)
			{
				m_Observer = observer;
			}

			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (widget->pick(event->pos())) {
					auto coord = widget->getPickPosition();
					if (m_Observer) m_Observer->inform(widget, coord);
					if (isState(SetCenter)) {
						setCenter(widget, coord);
					}
				}
			}

			virtual void mousePressEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (event->button()==Qt::LeftButton && event->modifiers()==0) {
					setIfStateIsNull(SetCenter);
					if (widget->pick(event->pos())) {
						setCenter(widget, widget->getPickPosition());
					}
				}
			}

			virtual void mouseReleaseEvent(ImageWidget* widget, QMouseEvent* event)
			{
				setState(NullState);
			}

			virtual void wheelEvent(ImageWidget* , QWheelEvent*) {}
			virtual void keyPressEvent(ImageWidget*, QKeyEvent*) {}
			virtual void keyReleaseEvent(ImageWidget*, QKeyEvent*) {}
			virtual void mouseDoubleClickEvent(ImageWidget*, QMouseEvent*) {}

		protected:
			ImageWidgetGroupController::Pointer m_Controller;
			NavigateImageInteractorObserver *m_Observer;

			NavigateImageInteractor(const ImageWidgetGroupController::Pointer& controller):m_Controller(controller), m_Observer(NULL)
			{
			}

			void setCenter(ImageWidget* widget, const pcl::Point3D<double>& center)
			{
				m_Controller->setCenter(ImageWidgetHelper::GetNearestVoxelPhysicalCoordinate(widget, center));
			}
		};

	}
}

#endif