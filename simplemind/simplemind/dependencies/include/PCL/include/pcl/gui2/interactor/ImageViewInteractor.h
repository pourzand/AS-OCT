#ifndef PCL_IMAGE_VIEW_INTERACTOR
#define PCL_IMAGE_VIEW_INTERACTOR

#include <pcl/gui2/interactor/Interactor.h>
#include <pcl/gui2/interactor/activator/Activator.h>
#include <pcl/gui2/view/ImageView.h>
#include <pcl/exception.h>

namespace pcl
{
	namespace gui
	{

		class ImageViewInteractor: public Interactor
		{
		public:
			typedef ImageViewInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			enum { PAN=1, ZOOM=2 };

			static Pointer New(Activator::Pointer pan, Activator::Pointer zoom)
			{
				return Pointer(new Self(pan, zoom));
			}

			virtual void mouseMoveEvent(View* view, QMouseEvent* event)
			{
				if (isState(PAN)) pan(view, event);
				if (isState(ZOOM)) zoom(view, event);
			}

			virtual void mousePressEvent(View* view, QMouseEvent* event)
			{
				if (getState()==NULLSTATE) {
					if (m_ZoomActivator->isActivated(Activator::PRESS, event)) {
						zoomStart(view, event);
						setState(ZOOM);
					}
					if (m_PanActivator->isActivated(Activator::PRESS, event)) {
						panStart(view, event);
						setState(PAN);
					}
				} else {
					if (getState()==ZOOM && m_ZoomActivator->isDeactivated(Activator::PRESS, event)) setState(NULLSTATE);
					else if (getState()==PAN && m_PanActivator->isDeactivated(Activator::PRESS, event)) setState(NULLSTATE);
				}
			}

			virtual void mouseReleaseEvent(View* view, QMouseEvent* event)
			{
				if (getState()==NULLSTATE) {
					if (m_ZoomActivator->isActivated(Activator::RELEASE, event)) {
						zoomStart(view, event);
						setState(ZOOM);
					}
					if (m_PanActivator->isActivated(Activator::RELEASE, event)) {
						panStart(view, event);
						setState(PAN);
					}
				} else {
					if (getState()==ZOOM && m_ZoomActivator->isDeactivated(Activator::RELEASE, event)) setState(NULLSTATE);
					else if (getState()==PAN && m_PanActivator->isDeactivated(Activator::RELEASE, event)) setState(NULLSTATE);
				}
			}

			virtual void mouseDoubleClickEvent(View* view, QMouseEvent* event) 
			{
				if (getState()==NULLSTATE) {
					if (m_ZoomActivator->isActivated(Activator::DOUBLECLICK, event)) {
						zoomStart(view, event);
						setState(ZOOM);
					}
					if (m_PanActivator->isActivated(Activator::DOUBLECLICK, event)) {
						panStart(view, event);
						setState(PAN);
					}
				} else {
					if (getState()==ZOOM && m_ZoomActivator->isDeactivated(Activator::DOUBLECLICK, event)) setState(NULLSTATE);
					else if (getState()==PAN && m_PanActivator->isDeactivated(Activator::DOUBLECLICK, event)) setState(NULLSTATE);
				}
			}

			virtual void wheelEvent(View* view, QWheelEvent* event) 
			{
				if (m_ZoomActivator->isActivated(event)) zoom(view, event);
				if (m_PanActivator->isActivated(event)) pan(view, event);
			}

		protected:
			Activator::Pointer m_PanActivator, m_ZoomActivator;

			ImageViewInteractor(Activator::Pointer pan, Activator::Pointer zoom): m_PanActivator(pan), m_ZoomActivator(zoom)
			{}

			virtual void zoomStart(View* view, QMouseEvent* event) 
			{
				pcl_ThrowException(pcl::Exception(), "Not supported!");
			}

			virtual void panStart(View* view, QMouseEvent* event) 
			{
				this->storeMousePos(event);
			}

			virtual void zoom(View* view, QMouseEvent* event) 
			{
				pcl_ThrowException(pcl::Exception(), "Not supported!");
			}
			
			virtual void pan(View* view, QMouseEvent* event) 
			{
				panCamera(view, event->pos(), this->getMousePos());
				this->storeMousePos(event);
			}

			virtual void zoom(View* view, QWheelEvent* event) 
			{
				if (event->delta()>0) zoomCamera(view, 1.5, event->pos());
				else if (event->delta()<0) zoomCamera(view, 1/1.5, event->pos());
			}
			
			virtual void pan(View*, QWheelEvent*) 
			{
				pcl_ThrowException(pcl::Exception(), "Not supported!");
			}

			void updateView(View* view)
			{
				view->getRenderer()->UpdateLightsGeometryToFollowCamera();
				view->render();
			}

			void panCamera(View* view, const QPoint& new_mouse_point, const QPoint& old_mouse_point)
			{
				double viewFocus[4], focalDepth, viewPoint[3];
				double newPickPoint[4], oldPickPoint[4], motionVector[3];
				vtkRenderer* renderer = view->getRenderer();

				// Below are codes adapted from vtkInteractorStyleTrackballCamera class

				// Calculate the focal depth since we'll be using it a lot

				vtkCamera *camera = renderer->GetActiveCamera();
				camera->GetFocalPoint(viewFocus);
				vtkInteractorObserver::ComputeWorldToDisplay(renderer, viewFocus[0], viewFocus[1], viewFocus[2], 
					viewFocus);
				focalDepth = viewFocus[2];

				view->getWorldCoordinate(newPickPoint, new_mouse_point, focalDepth);

				// Has to recalc old mouse point since the viewport has moved,
				// so can't move it outside the loop

				view->getWorldCoordinate(oldPickPoint, old_mouse_point, focalDepth);
				
				// Camera motion is reversed

				motionVector[0] = oldPickPoint[0] - newPickPoint[0];
				motionVector[1] = oldPickPoint[1] - newPickPoint[1];
				motionVector[2] = oldPickPoint[2] - newPickPoint[2];

				camera->GetFocalPoint(viewFocus);
				camera->GetPosition(viewPoint);
				camera->SetFocalPoint(motionVector[0] + viewFocus[0],
					motionVector[1] + viewFocus[1],
					motionVector[2] + viewFocus[2]);

				camera->SetPosition(motionVector[0] + viewPoint[0],
					motionVector[1] + viewPoint[1],
					motionVector[2] + viewPoint[2]);

				updateView(view);
			}

			void zoomCamera(View* view, double magnification)
			{
				double parallel_scale = view->getRenderer()->GetActiveCamera()->GetParallelScale();
				parallel_scale /= magnification;
				view->getRenderer()->GetActiveCamera()->SetParallelScale(parallel_scale);
				updateView(view);
			}

			void zoomCamera(View* view, double magnification, const QPoint& fixed_mouse_point)
			{
				vtkRenderer* renderer = view->getRenderer();
				vtkCamera* camera = renderer->GetActiveCamera();
				double fixed_point[4], view_focus[4], focal_depth;
				//Getting focal depth
				camera->GetFocalPoint(view_focus);
				vtkInteractorObserver::ComputeWorldToDisplay(renderer, view_focus[0], view_focus[1], view_focus[2], 
					view_focus);
				focal_depth = view_focus[2];
				//Getting world coordinate of the fixed point
				view->getWorldCoordinate(fixed_point, fixed_mouse_point, focal_depth);
				//Zooming the camera
				double parallel_scale = renderer->GetActiveCamera()->GetParallelScale();
				parallel_scale /= magnification;
				renderer->GetActiveCamera()->SetParallelScale(parallel_scale);
				//Getting new world coordinate of the fixed point
				double pre_fixed_point[4];
				view->getWorldCoordinate(pre_fixed_point, fixed_mouse_point, focal_depth);
				//Correcting camera position
				double motion_vector[3];
				motion_vector[0] = fixed_point[0] - pre_fixed_point[0];
				motion_vector[1] = fixed_point[1] - pre_fixed_point[1];
				motion_vector[2] = fixed_point[2] - pre_fixed_point[2];
				double view_point[3];
				camera->GetPosition(view_point);
				camera->GetFocalPoint(view_focus);
				camera->SetFocalPoint(motion_vector[0] + view_focus[0],
					motion_vector[1] + view_focus[1],
					motion_vector[2] + view_focus[2]);
				camera->SetPosition(motion_vector[0] + view_point[0],
					motion_vector[1] + view_point[1],
					motion_vector[2] + view_point[2]);
				updateView(view);
			}
		};

	}
}

#endif