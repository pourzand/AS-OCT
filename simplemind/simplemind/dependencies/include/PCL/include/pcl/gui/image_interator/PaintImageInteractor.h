#ifndef PCL_GUI_PAINT_IMAGE_INTERACTOR
#define PCL_GUI_PAINT_IMAGE_INTERACTOR

#include <pcl/gui/ImageWidgetGroupController.h>
#include <pcl/gui/ImageWidgetInteractorBase.h>
#include <pcl/gui/image_interator/PaintImageInteractorObserver.h>
#include <pcl/gui/ImageSquareWidget.h>
#include <pcl/iterator.h>
#include <QCursor>

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

			static Pointer New(const ImageWidgetGroupController::Pointer& controller, const std::vector<ImageResliceActor::Pointer>& reslice_vector, double fval=1, double bval=0, double square_widget_size=20)
			{
				return Pointer(new Self(controller, reslice_vector, fval, bval, square_widget_size));
			}

			void setObserver(PaintImageInteractorObserver* observer)
			{
				m_Observer = observer;
			}

			void setSquareWidgetSize(double val)
			{
				m_SquareWidgetSize = val;
			}

			double getSquareWidgetSize()
			{
				return m_SquareWidgetSize;
			}

			virtual void mouseMoveEvent(ImageWidget* widget, QMouseEvent* event)
			{
				bool render_needed = false;
				m_Rendered = false;
				if (event->modifiers()==Qt::ALT) {
					if (widget->pick(event->pos().x(), event->pos().y())!=0) {
						if (!m_EraseRegionWidget) m_EraseRegionWidget = ImageSquareWidget::New(widget,m_SquareWidgetSize,0,1,1);
						else if (!m_EraseRegionWidget->belongTo(widget)) m_EraseRegionWidget = ImageSquareWidget::New(widget,m_SquareWidgetSize,0,1,1);
						pcl::Point3D<double> pos = widget->getPickPosition();
						m_EraseRegionWidget->setVisibility(true);
						m_EraseRegionWidget->setPoint(pos);
						render_needed = true;
					}
				} else {
					if (m_EraseRegionWidget) {
						m_EraseRegionWidget.reset();
					}
				}
				if (isState(PaintEvent)) {
					if (event->modifiers()==Qt::ALT) paintImage(widget, event, 0);
					else if (event->modifiers()==0) paintImage(widget, event, 1);
				}
				if (render_needed && !m_Rendered) widget->render();
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
			virtual void keyPressEvent(ImageWidget* widget, QKeyEvent* event) 
			{
				bool create_sqr_widget = false;
				if ((QApplication::keyboardModifiers() & Qt::AltModifier)!=0) {
					if (!m_EraseRegionWidget) create_sqr_widget = true;
					else if (!m_EraseRegionWidget->belongTo(widget)) create_sqr_widget = true;
				}
				if (create_sqr_widget) {
					auto pos = widget->mapFromGlobal(QCursor::pos());
					if (widget->pick(pos.x(), pos.y())!=0) {
						m_EraseRegionWidget = ImageSquareWidget::New(widget,m_SquareWidgetSize,0,1,1);
						pcl::Point3D<double> pos = widget->getPickPosition();
						m_EraseRegionWidget->setVisibility(true);
						m_EraseRegionWidget->setPoint(pos);
						widget->render();
					}
				}
			}
			virtual void keyReleaseEvent(ImageWidget* widget, QKeyEvent* event) 
			{
				if ((QApplication::keyboardModifiers() & Qt::AltModifier)==0) {
					m_EraseRegionWidget.reset();
				}
			}
			virtual void mouseDoubleClickEvent(ImageWidget*, QMouseEvent*) {}

		protected:
			ImageWidgetGroupController::Pointer m_Controller;
			std::vector<ImageResliceActor::Pointer> m_ResliceVector;
			PaintImageInteractorObserver *m_Observer;
			double m_ForeGround, m_BackGround;
			ImageSquareWidget::Pointer m_EraseRegionWidget;
			bool m_Rendered;
			double m_SquareWidgetSize;

			PaintImageInteractor(const ImageWidgetGroupController::Pointer& controller, const std::vector<ImageResliceActor::Pointer>& reslice_vector, double fval, double bval, double square_widget_size):
			m_Controller(controller), m_ResliceVector(reslice_vector), m_Observer(NULL), m_ForeGround(fval), m_BackGround(bval), m_SquareWidgetSize(square_widget_size)
			{}

			void paintImage(ImageWidget* widget, QMouseEvent* event, double val)
			{
				//widget->pick(event->pos().x(), event->pos().y());
				if (widget->pick(event->pos().x(), event->pos().y())!=0) {
					auto image_source = m_ResliceVector[0]->getImageSource();
					pcl::Point3D<double> pos = widget->getPickPosition();
					if (!m_EraseRegionWidget) {
						pcl::Point3D<int> coord;
						coord.assignRound(image_source->getImageCoordinate(pos));
						if (image_source->contain(coord) && image_source->getImageValue(coord)!=val) {
							if (m_Observer) m_Observer->inform(image_source->getPhysicalCoordinate(coord), val==m_ForeGround);
							image_source->setImageValue(coord, val);
							pcl_ForEach(m_ResliceVector, item) (*item)->modified();
							m_Controller->render();
							m_Rendered = true;
						}
					} else {
						pcl::Region3D<int> region; region.reset();
						auto reslice_axes = widget->getResliceActorStack()->getResliceAxes();
						for (int a=0; a<2; ++a) {
							pcl::Point3D<double> axis(
								reslice_axes->GetElement(0, a),
								reslice_axes->GetElement(1, a),
								reslice_axes->GetElement(2, a)
								);
							pcl::Point3D<int> coord;
							coord.assignRound(image_source->getImageCoordinate(pos+axis*(m_SquareWidgetSize/2)));
							region.add(coord);
							coord.assignRound(image_source->getImageCoordinate(pos-axis*(m_SquareWidgetSize/2)));
							region.add(coord);
						}
						region = region.getIntersect(image_source->getImageBase()->getRegion());
						if (!region.empty()) {
							auto image_base = image_source->getImageBase();
							pcl::ImageIteratorWithPoint iter(image_base);
							iter.setRegion(region);
							bool value_changed = false;
							pcl_ForIterator(iter) if (image_base->getValue(iter)!=val) {
								if (m_Observer) m_Observer->inform(image_source->getPhysicalCoordinate(iter.getPoint()), val==m_ForeGround);
								image_base->setValue(iter, val);
								value_changed = true;
							}
							if (value_changed) {
								pcl_ForEach(m_ResliceVector, item) (*item)->modified();
								m_Controller->render();
								m_Rendered = true;
							}
						}
					}
				}
			}
		};

	}
}

#endif