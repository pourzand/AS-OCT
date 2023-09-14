#ifndef PCL_GUI_BRIGHTNESS_CONTRAST_IMAGE_INTERACTOR
#define PCL_GUI_BRIGHTNESS_CONTRAST_IMAGE_INTERACTOR

#include <pcl/gui2/ImageWidgetGroupController.h>
#include <pcl/gui2/ImageWidgetInteractorBase.h>
#include <pcl/gui2/image_interactor/BrightnessConstrastImageInteratorObserver.h>

namespace pcl
{
	namespace gui
	{

		class BrightnessContrastImageInteractor: public ImageWidgetInteractorBase
		{
		public:
			typedef BrightnessContrastImageInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { ColorAdjust=1 };

			static Pointer New(const ImageWidgetGroupController::Pointer& controller)
			{
				return Pointer(new Self(controller));
			}

			void setObserver(BrightnessConstrastImageInteractorObserver* observer)
			{
				m_Observer = observer;
			}

			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (isState(ColorAdjust)) {
					QPoint relative_pos = event->pos() - this->m_MousePos;
					auto source = widget->getResliceActorStack()->getActiveImageResliceActor()->getImageSource();
					pcl::gui::ImageSourceHelper::GrayscaleVtkLookupTableWrapper lookup_table(source->getColorLookup<vtkLookupTable>());
					if (!lookup_table.isNull()) {
						double offset = m_OrgWindow[0]+relative_pos.x(), 
							width = m_OrgWindow[1]-relative_pos.y();
						if (width<1) width = 1;
						if (m_Observer) m_Observer->inform(offset, width);
						lookup_table.setWindowLevel(offset, width);
						m_Controller->render();
					}
				}
			}

			virtual void mousePressEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (event->button()==Qt::LeftButton && event->modifiers()==0) {
					setIfStateIsNull(ColorAdjust);
					storeMousePos(event);
					storeRange(widget);
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
			double m_OrgWindow[2];
			BrightnessConstrastImageInteractorObserver *m_Observer;

			BrightnessContrastImageInteractor(const ImageWidgetGroupController::Pointer& controller):m_Controller(controller), m_Observer(NULL)
			{}

			void storeRange(ImageWidget* widget)
			{
				auto source = widget->getResliceActorStack()->getActiveImageResliceActor()->getImageSource();
				pcl::gui::ImageSourceHelper::GrayscaleVtkLookupTableWrapper lookup_table(source->getColorLookup<vtkLookupTable>());
				if (!lookup_table.isNull()) {
					lookup_table.getWindowLevel(m_OrgWindow[0], m_OrgWindow[1]);
				}
			}
		};

	}
}

#endif