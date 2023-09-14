#ifndef PCL_GUI_IMAGE_WIDGET_HELPER
#define PCL_GUI_IMAGE_WIDGET_HELPER

#include <pcl/gui/ImageWidget.h>
#include <vtkInteractorObserver.h>

namespace pcl
{
	namespace gui
	{

		class ImageWidgetHelper
		{
		public:
			static pcl::Point3D<double> GetNearestVoxelPhysicalCoordinate(ImageWidget* widget, const pcl::Point3D<double>& p)
			{
				auto image_source = widget->getResliceActorStack()->getActiveImageResliceActor()->getImageSource();
				return image_source->getPhysicalCoordinate(image_source->getImageCoordinate(p));
			}

			/************* Move slice related *************/

			static void MoveSlice(ImageWidget* widget, double offset)
			{
				auto pos = widget->getResliceActorStack()->getNormalStep() * offset;
				pos += widget->getResliceActorStack()->getImageResliceOrigin();
				widget->getResliceActorStack()->setSliceToContain(pos);
				widget->render(true);
				//qDebug() << pos.toString().c_str();
			}

			static void MoveSlice(ImageWidget* widget, const Point3D<double>& pos)
			{
				widget->getResliceActorStack()->setSliceToContain(pos);
				widget->render(true);
			}

			static pcl::Point3D<double> getMoveSlicePos(ImageWidget* widget, double offset, const Point3D<double>& org_pos)
			{
				auto pos = widget->getResliceActorStack()->getNormalStep() * offset;
				pos += org_pos;
				return pos;
			}

			/************* Camera panning related *************/

			static void PanCamera(ImageWidget* widget, const QPoint& new_mouse_point, const QPoint& old_mouse_point)
			{
				double viewFocus[4], focalDepth, viewPoint[3];
				double newPickPoint[4], oldPickPoint[4], motionVector[3];
				vtkRenderer* renderer = widget->getRenderer();

				// Below are codes adapted from vtkInteractorStyleTrackballCamera class

				// Calculate the focal depth since we'll be using it a lot

				vtkCamera *camera = renderer->GetActiveCamera();
				camera->GetFocalPoint(viewFocus);
				vtkInteractorObserver::ComputeWorldToDisplay(renderer, viewFocus[0], viewFocus[1], viewFocus[2], 
					viewFocus);
				focalDepth = viewFocus[2];

				vtkInteractorObserver::ComputeDisplayToWorld(renderer, new_mouse_point.x(), widget->getFlippedY(new_mouse_point.y()),
					focalDepth, 
					newPickPoint);

				// Has to recalc old mouse point since the viewport has moved,
				// so can't move it outside the loop

				vtkInteractorObserver::ComputeDisplayToWorld(renderer, old_mouse_point.x(), widget->getFlippedY(old_mouse_point.y()),
					focalDepth, 
					oldPickPoint);

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

				renderer->UpdateLightsGeometryToFollowCamera();

				widget->render();
			}

			/************* Camera zooming related *************/

			static void ZoomCamera(ImageWidget* widget, double magnification) //Negative to zoom out and positive to zoom in
			{
				if (magnification<=0) magnification = 0.001;
				vtkRenderer* renderer = widget->getRenderer();
				double parallel_scale = renderer->GetActiveCamera()->GetParallelScale();
				parallel_scale /= magnification;
				renderer->GetActiveCamera()->SetParallelScale(parallel_scale);
				renderer->UpdateLightsGeometryToFollowCamera();
				widget->render();
			}

			static void ZoomCamera(ImageWidget* widget, double magnification, const QPoint& fixed_mouse_point)
			{
				if (magnification<=0) magnification = 0.001;
				vtkRenderer* renderer = widget->getRenderer();
				vtkCamera* camera = renderer->GetActiveCamera();
				double fixed_point[4], view_focus[4], focal_depth;
				//Getting focal depth
				camera->GetFocalPoint(view_focus);
				vtkInteractorObserver::ComputeWorldToDisplay(renderer, view_focus[0], view_focus[1], view_focus[2], 
					view_focus);
				focal_depth = view_focus[2];
				//Getting world coordinate of the fixed point
				vtkInteractorObserver::ComputeDisplayToWorld(renderer, fixed_mouse_point.x(), widget->getFlippedY(fixed_mouse_point.y()), focal_depth,
					fixed_point);
				//Zooming the camera
				double parallel_scale = renderer->GetActiveCamera()->GetParallelScale();
				parallel_scale /= magnification;
				renderer->GetActiveCamera()->SetParallelScale(parallel_scale);
				//Getting new world coordinate of the fixed point
				double pre_fixed_point[4];
				vtkInteractorObserver::ComputeDisplayToWorld(renderer, fixed_mouse_point.x(), widget->getFlippedY(fixed_mouse_point.y()), focal_depth,
					pre_fixed_point);
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
				renderer->UpdateLightsGeometryToFollowCamera();
				widget->render();
			}
		};

	}
}

#endif