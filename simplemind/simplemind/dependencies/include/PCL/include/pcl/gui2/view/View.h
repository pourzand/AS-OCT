#ifndef PCL_GUI_IMAGE_WIDGET
#define PCL_GUI_IMAGE_WIDGET

#include <pcl/gui2/data/DataCollection.h>
#include <pcl/gui2/interactor/Interactor.h>
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

#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkMatrix4x4.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkPropPicker.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkAbstractPicker.h>

#include <qmessagebox.h>

namespace pcl
{
	namespace gui
	{

		class View: public QVTKWidget2
		{
			Q_OBJECT

		public:
			View(QWidget *parent=0, const QGLWidget* share_widget=0, Qt::WFlags f=0):QVTKWidget2(parent,share_widget,f)
			{
				m_Renderer = vtkSmartPointer<vtkRenderer>::New();
				m_Renderer->SetBackground(0,0,0);
				GetRenderWindow()->AddRenderer(m_Renderer);
				m_PopupMenu = NULL;
			}

			virtual void setDataCollection(const DataCollection::Pointer& collection)
			{
				m_DataCollection = collection;
			}

			void setPopupMenu(QMenu* popup)
			{
				m_PopupMenu = popup;
			}

			QMenu* getPopupMenu()
			{
				return m_PopupMenu;
			}

			const DataCollection::Pointer& getDataCollection()
			{
				return m_DataCollection;
			}

			void addProp()
			{
				//TODO
			}

			void removeProp()
			{
				//TODO
			}

			void addInteractor(const Interactor::Pointer& interactor)
			{
				m_InteractorList.push_back(interactor);
			}

			void removeInteractor(const Interactor::Pointer& interactor)
			{
				m_InteractorList.remove(interactor);
			}

			void removeAllInteractor()
			{
				m_InteractorList.clear();
			}

			virtual void updateAllInteractor()
			{
				pcl_ForEach(m_InteractorList, item) (*item)->update(this);
			}

			vtkRenderer* getRenderer()
			{
				return m_Renderer;
			}

			int pick(const QPoint& p, vtkAbstractPicker* picker)
			{
				return pick(p.x(), p.y(), picker);
			}

			int pick(int x, int y, vtkAbstractPicker* picker)
			{
				return picker->Pick(x, getVtkY(y), 0, m_Renderer);
			}

			double getDisplayFocalDepth()
			{
				double view_focus[4];
				m_Renderer->GetActiveCamera()->GetFocalPoint(view_focus);
				vtkInteractorObserver::ComputeWorldToDisplay(m_Renderer, view_focus[0], view_focus[1], view_focus[2], 
					view_focus);
				return view_focus[2];
			}

			void getWorldCoordinate(double *coord, const QPoint& p)
			{
				return getWorldCoordinate(coord, p.x(), p.y());
			}

			void getWorldCoordinate(double *coord, int x, int y)
			{
				getWorldCoordinate(coord, x, y, getDisplayFocalDepth());
			}

			void getWorldCoordinate(double *coord, const QPoint& p, double depth)
			{
				getWorldCoordinate(coord, p.x(), p.y(), depth);
			}

			void getWorldCoordinate(double *coord, int x, int y, double depth)
			{
				vtkInteractorObserver::ComputeDisplayToWorld(m_Renderer, x, getVtkY(y), depth, coord);
			}

		signals:
			void renderCalled();

		public slots:
			virtual void setVisible(bool visible) 
			{
				QVTKWidget2::setVisible(visible);
				if (visible) render();
			}

			virtual void resetCamera()=0;
			virtual void render()=0;

		protected:
			virtual void contextMenuEvent(QContextMenuEvent * event)
			{
				if (m_PopupMenu!=NULL) m_PopupMenu->popup(event->globalPos());
			}

			virtual void wheelEvent(QWheelEvent *event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->wheelEvent(this, event);
			}

			virtual void mouseMoveEvent(QMouseEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseMoveEvent(this, event);
			}

			virtual void mousePressEvent(QMouseEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mousePressEvent(this, event);
			}

			virtual void mouseReleaseEvent(QMouseEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseReleaseEvent(this, event);
			}

			virtual void keyPressEvent(QKeyEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->keyPressEvent(this, event);
			}

			virtual void keyReleaseEvent(QKeyEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->keyReleaseEvent(this, event);
			}

			virtual void mouseDoubleClickEvent(QMouseEvent* event)
			{
				if (!m_DataCollection || m_InteractorList.empty()) return;
				pcl_ForEach(m_InteractorList, item) if ((*item)->isEnabled()) (*item)->mouseDoubleClickEvent(this, event);
			}

		protected:
			DataCollection::Pointer m_DataCollection;
			vtkSmartPointer<vtkRenderer> m_Renderer;
			std::list<Interactor::Pointer> m_InteractorList;
			QMenu *m_PopupMenu;

			int getVtkY(int y)
			{
				return this->GetRenderWindow()->GetSize()[1] - y - 1;
			}
		};

	}
}

#endif
