#ifndef PCL_GUI_BASIC_IMAGE_INTERACTOR
#define PCL_GUI_BASIC_IMAGE_INTERACTOR

#include <pcl/gui/ImageWidgetGroupController.h>
#include <pcl/gui/ImageWidgetInteractorBase.h>

namespace pcl
{
	namespace gui
	{

		class BasicImageInteractor: public ImageWidgetInteractorBase
		{
		public:
			typedef BasicImageInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { PanEvent=1 };

			static Pointer New(const ImageWidgetGroupController::Pointer& controller)
			{
				return Pointer(new Self(controller));
			}

			virtual void wheelEvent(ImageWidget* widget, QWheelEvent* event)
			{
				switch (event->modifiers()) {
				case Qt::CTRL:
					if (event->delta()>0) ImageWidgetHelper::ZoomCamera(widget, 1.5, event->pos());
					else if (event->delta()<0) ImageWidgetHelper::ZoomCamera(widget, 1/1.5, event->pos());
					break;
				case 0:
					pcl::Point3D<double> center;
					if (event->delta()>0) center = ImageWidgetHelper::getMoveSlicePos(widget, 1, m_Controller->getCenter());
					else if (event->delta()<0) center = ImageWidgetHelper::getMoveSlicePos(widget, -1, m_Controller->getCenter());
					this->setCenter(widget, center);
				}
			}

			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (isState(PanEvent)) {
					ImageWidgetHelper::PanCamera(widget, event->pos(), this->m_MousePos);
					this->storeMousePos(event);
				}
			}

			virtual void mousePressEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (event->button()==Qt::LeftButton && event->modifiers()==Qt::CTRL) {
					this->storeMousePos(event);
					setIfStateIsNull(PanEvent);
				}
			}

			virtual void mouseReleaseEvent(ImageWidget* widget, QMouseEvent* event)
			{
				setState(NullState);
			}

			virtual void keyPressEvent(ImageWidget*, QKeyEvent*) {}
			virtual void keyReleaseEvent(ImageWidget*, QKeyEvent*) {}
			virtual void mouseDoubleClickEvent(ImageWidget*, QMouseEvent*) {}

		protected:
			ImageWidgetGroupController::Pointer m_Controller;

			BasicImageInteractor(const ImageWidgetGroupController::Pointer& controller):m_Controller(controller)
			{
			}

			void setCenter(ImageWidget* widget, const pcl::Point3D<double>& center)
			{
				auto actual_center = ImageWidgetHelper::GetNearestVoxelPhysicalCoordinate(widget, center);
				if (widget->getResliceActorStack()->getFullPhysicalRegion().contain(actual_center)) m_Controller->setCenter(actual_center);
			}
		};

	}
}

#endif