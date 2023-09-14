#ifndef PCL_GUI_IMAGE_WIDGET
#define PCL_GUI_IMAGE_WIDGET

#include <QVTKWidget.h>
#include <QVTKWidget2.h>

#include <QLabel>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtDebug>
#include <QMenu>

#include <QPaintDevice>
#include <QPainter>

#include <list>

#include <pcl/gui2/ImageResliceActorStack.h>
#include <pcl/gui2/ImageWidgetInteractorBase.h>

#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkPropPicker.h>
#include <vtkGenericOpenGLRenderWindow.h>

#include <qmessagebox.h>

namespace pcl
{
	namespace gui
	{

		class ImageWidget: public QVTKWidget2
		{
			Q_OBJECT

		public:
			ImageWidget(QWidget *parent=0, const QGLWidget* share_widget=0, Qt::WFlags f=0):QVTKWidget2(parent,share_widget,f)
			{
				m_Renderer = vtkRenderer::New();
				m_Renderer->SetBackground(0,0,0);
				m_Picker = vtkPropPicker::New();
				GetRenderWindow()->AddRenderer(m_Renderer);

				m_PopupMenu = new QMenu(this);
				auto action = m_PopupMenu->addAction("Reset view"); connect(action, SIGNAL(triggered()), this, SLOT(resetCamera()));
			}

			void setResliceActorStack(const ImageResliceActorStack::Pointer& actor_stack)
			{
				if (m_ResliceActorStack) {
					m_Renderer->GetViewProps()->RemoveItem(m_ResliceActorStack->getImageStack());
				}
				m_ResliceActorStack = actor_stack;
				m_ResliceActorStack->getImageStack()->PickableOn();
				reset();
			}

			void setResliceActorStack()
			{
				if (m_ResliceActorStack) {
					m_Renderer->GetViewProps()->RemoveItem(m_ResliceActorStack->getImageStack());
					m_Renderer->GetViewProps()->RemoveItem(m_ResliceActorStack->getImageCrossHairs()->getLineActor(0));
					m_Renderer->GetViewProps()->RemoveItem(m_ResliceActorStack->getImageCrossHairs()->getLineActor(1));
				}
				m_ResliceActorStack.reset();
				reset();
			}

			void addImageWidgetInteractor(const ImageWidgetInteractorBase::Pointer& interactor)
			{
				pcl_ForEach(m_InteractorList, item) {
					if (*item==interactor) return;
				}
				m_InteractorList.push_back(interactor);
			}

			void removeImageWidgetInteractor(const ImageWidgetInteractorBase::Pointer& interactor)
			{
				pcl_ForEach(m_InteractorList, item) {
					if (*item==interactor) {
						m_InteractorList.erase(item);
						return;
					}
				}
			}

			void removeAllImageWidgetInteractor()
			{
				m_InteractorList.clear();
			}

			const ImageResliceActorStack::Pointer& getResliceActorStack()
			{
				return m_ResliceActorStack;
			}

			vtkPropPicker* getPicker()
			{
				return m_Picker;
			}

			vtkRenderer* getRenderer()
			{
				return m_Renderer;
			}

			int getFlippedY(int y)
			{
				return this->GetRenderWindow()->GetSize()[1] - y - 1;
			}

			template <class EventType>
			int pick(EventType* event)
			{
				return pick(event.pos());
			}
			int pick(const QPoint& p)
			{
				return pick(p.x(), p.y());
			}
			int pick(int x, int y)
			{
				return m_Picker->Pick(x, getFlippedY(y), 0, m_Renderer);
			}

			double* getPickPosition()
			{
				return m_Picker->GetPickPosition();
			}

			void render(bool adjust_camera=false)
			{
				if (adjust_camera && m_ResliceActorStack) {
					auto camera = m_Renderer->GetActiveCamera();
					auto image_stack = m_ResliceActorStack->getImageStack();
					pcl::Point3D<double> image_origin(image_stack->GetCenter()), image_normal,
						camera_position(camera->GetPosition()), camera_direction(camera->GetDirectionOfProjection());
					auto reslice_axes = m_ResliceActorStack->getResliceAxes();
					for (int i=0; i<3; ++i) {
						image_normal[i] = reslice_axes->GetElement(i, 2);
					}
					double d = ((image_origin-camera_position).getDotProduct(image_normal))/camera_direction.getDotProduct(image_normal);
					pcl::Point3D<double> new_camera_focal_point = camera_direction*d + camera_position,
						new_camera_position = new_camera_focal_point - camera_direction*camera->GetDistance();
					/*qDebug() << "*****";
					qDebug() << "New focal point: " << new_camera_focal_point.toString().c_str();
					qDebug() << "New position: " << new_camera_position.toString().c_str();
					qDebug() << "Before";
					printCameraInfo(camera);*/
					camera->SetPosition(&new_camera_position[0]);
					camera->SetFocalPoint(&new_camera_focal_point[0]);
					m_Renderer->UpdateLightsGeometryToFollowCamera();
					m_Renderer->ResetCameraClippingRange();
					/*qDebug() << "After";
					printCameraInfo(camera);*/

					pcl::Point3D<double> line_center = m_ResliceActorStack->getImageCrossHairs()->getLineActor(0)->GetCenter();
					//qDebug() << "Line center: " << line_center.toString().c_str();
				}
				if (this->isVisible()) this->GetRenderWindow()->Render();
			}

			void printCameraInfo(vtkCamera* camera)
			{
				qDebug() << "Camera position: " << pcl::Point3D<double>(camera->GetPosition()).toString().c_str() << "\n";
				qDebug() << "Camera focus: " << pcl::Point3D<double>(camera->GetFocalPoint()).toString().c_str() << "\n";
				qDebug() << "Camera dist:" << QString::number(camera->GetDistance()).toStdString().c_str() << "\n";
			}

		public slots:
			virtual void setVisible(bool visible) 
			{
				QVTKWidget2::setVisible(visible);
				if (visible) render();
			}

			void resliceAxesChanged()
			{
				if (!m_ResliceActorStack) {
					this->GetRenderWindow()->Render();
					return;
				}
				auto image_stack = m_ResliceActorStack->getImageStack();
				auto camera = m_Renderer->GetActiveCamera();
				auto reslice_axes = m_ResliceActorStack->getResliceAxes();
				pcl::Point3D<double> view_up = m_ResliceActorStack->getCameraViewUp(), 
					camera_direction = m_ResliceActorStack->getCameraDirection();
				pcl::Point3D<double> center(image_stack->GetCenter());
				pcl::Point3D<double> camera_pos = center - camera_direction*camera->GetDistance();
				
				camera->SetPosition(&camera_pos[0]);
				camera->SetViewUp(&view_up[0]);
				m_Renderer->UpdateLightsGeometryToFollowCamera();
				if (this->isVisible()) this->GetRenderWindow()->Render();
			}

			void resetCamera()
			{
				if (!m_ResliceActorStack) {
					this->GetRenderWindow()->Render();
					return;
				}
				auto image_stack = m_ResliceActorStack->getImageStack();
				auto camera = m_Renderer->GetActiveCamera();
				auto reslice_axes = m_ResliceActorStack->getResliceAxes();
				pcl::Point3D<double> view_up = m_ResliceActorStack->getCameraViewUp(), 
					camera_direction = m_ResliceActorStack->getCameraDirection();
				pcl::Point3D<double> center(image_stack->GetCenter());
				pcl::Point3D<double> camera_pos = center - camera_direction*camera->GetDistance();

				camera->SetPosition(&camera_pos[0]);
				camera->SetFocalPoint(&center[0]);
				camera->SetViewUp(&view_up[0]);

				camera->ParallelProjectionOn();
				m_Renderer->ResetCameraClippingRange();
				const double *spacing = m_ResliceActorStack->getResliceParam()->getSpacing();
				const int *extent = m_ResliceActorStack->getResliceParam()->getExtent();
				camera->SetParallelScale((extent[3]-extent[2]+1)*spacing[1]*0.5);
				m_Renderer->UpdateLightsGeometryToFollowCamera();
				if (this->isVisible()) this->GetRenderWindow()->Render();
			}

		protected:
			virtual void contextMenuEvent(QContextMenuEvent * event)
			{
				m_PopupMenu->popup(event->globalPos());
			}

			virtual void wheelEvent(QWheelEvent *event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->wheelEvent(this, event);
			}

			virtual void mouseMoveEvent(QMouseEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseMoveEvent(this, event);
			}

			virtual void mousePressEvent(QMouseEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mousePressEvent(this, event);
			}

			virtual void mouseReleaseEvent(QMouseEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseReleaseEvent(this, event);
			}

			virtual void keyPressEvent(QKeyEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->keyPressEvent(this, event);
			}

			virtual void keyReleaseEvent(QKeyEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->keyReleaseEvent(this, event);
			}

			virtual void mouseDoubleClickEvent(QMouseEvent* event)
			{
				if (!m_ResliceActorStack || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseDoubleClickEvent(this, event);
			}

		protected:
			ImageResliceActorStack::Pointer m_ResliceActorStack;
			vtk_Pointer(vtkRenderer, m_Renderer);
			vtk_Pointer(vtkPropPicker, m_Picker);
			std::list<ImageWidgetInteractorBase::Pointer> m_InteractorList;
			QMenu *m_PopupMenu;

			void reset()
			{
				if (m_ResliceActorStack) {
					m_Renderer->AddViewProp(m_ResliceActorStack->getImageStack());
					m_Renderer->AddViewProp(m_ResliceActorStack->getImageCrossHairs()->getLineActor(0));
					m_Renderer->AddViewProp(m_ResliceActorStack->getImageCrossHairs()->getLineActor(1));
				}
				resetCamera();
			}
		};

	}
}

#endif
