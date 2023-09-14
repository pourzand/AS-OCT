#ifndef PCL_GUI_PAINT_IMAGE_INTERACTOR
#define PCL_GUI_PAINT_IMAGE_INTERACTOR

#include <pcl/gui2/ImageWidgetGroupController.h>
#include <pcl/gui2/ImageWidgetInteractorBase.h>
#include <pcl/gui2/image_interactor/PaintImageInteractorObserver.h>

namespace pcl
{
	namespace gui
	{

		class PaintImageInteractor: public ImageWidgetInteractorBase
		{
		public:
			typedef PaintImageInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { PaintEvent=1 };

			static Pointer New(const ImageWidgetGroupController::Pointer& controller, const std::vector<ImageResliceActor::Pointer>& reslice_vector, double fval=1, double bval=0)
			{
				return Pointer(new Self(controller, reslice_vector, fval, bval));
			}

			void setObserver(PaintImageInteractorObserver* observer)
			{
				m_Observer = observer;
			}

			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (isState(PaintEvent)) {
					if (event->modifiers()==Qt::ALT) paintImage(widget, event, 0);
					else if (event->modifiers()==0) paintImage(widget, event, 1);
				}
			}

			virtual void mousePressEvent(ImageWidget* widget, QMouseEvent* event)
			{
				if (event->button()==Qt::LeftButton) {
					if (event->modifiers()==Qt::ALT) {
						setIfStateIsNull(PaintEvent);
						paintImage(widget, event, m_BackGround);
					} else if (event->modifiers()==0) {
						setIfStateIsNull(PaintEvent);
						paintImage(widget, event, m_ForeGround);
					}
				}
			}

			virtual void mouseReleaseEvent(ImageWidget* widget, QMouseEvent* event)
			{
				setState(NullState);
			}

			virtual void wheelEvent(ImageWidget* widget, QWheelEvent*) {}
			virtual void keyPressEvent(ImageWidget*, QKeyEvent*) {}
			virtual void keyReleaseEvent(ImageWidget*, QKeyEvent*) {}
			virtual void mouseDoubleClickEvent(ImageWidget*, QMouseEvent*) {}

		protected:
			ImageWidgetGroupController::Pointer m_Controller;
			std::vector<ImageResliceActor::Pointer> m_ResliceVector;
			PaintImageInteractorObserver *m_Observer;
			double m_ForeGround, m_BackGround;

			PaintImageInteractor(const ImageWidgetGroupController::Pointer& controller, const std::vector<ImageResliceActor::Pointer>& reslice_vector, double fval, double bval):
			m_Controller(controller), m_ResliceVector(reslice_vector), m_Observer(NULL), m_ForeGround(fval), m_BackGround(bval)
			{
			}

			void paintImage(ImageWidget* widget, QMouseEvent* event, double val)
			{
				widget->pick(event->pos().x(), event->pos().y());
				if (widget->pick(event->pos().x(), event->pos().y())!=0) {
					auto image_source = m_ResliceVector[0]->getImageSource();
					pcl::Point3D<double> pos = widget->getPickPosition();
					pcl::Point3D<int> coord;
					coord.assignRound(image_source->getImageCoordinate(pos));
					if (image_source->contain(coord) && image_source->getImageValue(coord)!=val) {
						if (m_Observer) m_Observer->inform(image_source->getPhysicalCoordinate(coord), val==m_ForeGround);
						image_source->setImageValue(coord, val);
						pcl_ForEach(m_ResliceVector, item) (*item)->modified();
						m_Controller->render();
					}
				}
			}
		};

	}
}

#endif