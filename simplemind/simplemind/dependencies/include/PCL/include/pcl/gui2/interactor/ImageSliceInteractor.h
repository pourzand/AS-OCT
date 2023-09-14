#ifndef PCL_IMAGE_SLICE_INTERACTOR
#define PCL_IMAGE_SLICE_INTERACTOR

#include <pcl/gui2/interactor/Interactor.h>
#include <pcl/gui2/interactor/activator/Activator.h>
#include <pcl/gui2/view/ImageView.h>

namespace pcl
{
	namespace gui
	{

		class ImageSliceInteractor: public DragInteractor
		{
		public:
			typedef ImageSliceInteractor Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(Activator::Pointer activator)
			{
				return Pointer(new Self(activator));
			}

			virtual void wheelEvent(View* view, QWheelEvent* event) 
			{
				if (m_Activator->isActivated(event)) wheel(view, event);
			}

		protected:
			ImageSliceInteractor(Activator::Pointer activator): DragInteractor(activator)
			{}

			virtual void wheel(View* view, QWheelEvent* event)
			{
				if (event->delta()>0) static_cast<pcl::gui::ImageView*>(view)->getImageResliceParam()->moveSlice(1);
				else if (event->delta()<0) static_cast<pcl::gui::ImageView*>(view)->getImageResliceParam()->moveSlice(-1);
				view->render();
				view->updateAllInteractor();
			}

			virtual void drag(View* view, QMouseEvent* event)
			{
				int disp = (event->pos().y()-this->getMousePos().y())/2;
				static_cast<pcl::gui::ImageView*>(view)->getImageResliceParam()->moveSlice(disp);
				this->storeMousePos(event);
				view->render();
			}


			virtual void dragStart(View* view, QMouseEvent* event) 
			{
				this->storeMousePos(event);
			}

			virtual void dragEnd(View* view, QMouseEvent*)
			{
				view->updateAllInteractor();
			}
		};

	}
}

#endif