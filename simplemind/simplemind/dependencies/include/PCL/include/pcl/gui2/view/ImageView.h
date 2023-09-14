#ifndef PCL_GUI_IMAGEVIEW
#define PCL_GUI_IMAGEVIEW

#include <pcl/gui2/view/View.h>
#include <pcl/gui2/view/misc/ImageResliceParam.h>
#include <pcl/gui2/view/misc/ImageResliceActor.h>
#include <boost/unordered_map.hpp>
#include <vtkImageStack.h>
#include <vtkImageProperty.h>
#include <vtkImageSliceCollection.h>

#include <vtkRenderWindow.h>

namespace pcl
{
	namespace gui
	{

		class ImageView: public View
		{
		public:
			enum SlicePosition {
				FIRST,
				MIDDLE,
				FIRST_ON_INIT,
				MIDDLE_ON_INIT,
				NO_CHANGE
			};

			ImageView(QWidget *parent=0, const QGLWidget* share_widget=0, Qt::WFlags f=0): View(parent, share_widget, f)
			{
				m_Param = ImageResliceParam::New();
				m_ImageStack = vtkSmartPointer<vtkImageStack>::New();
				this->m_Renderer->GetActiveCamera()->ParallelProjectionOn();
				this->m_Renderer->AddActor(m_ImageStack);
			}

			virtual void setDataCollection(const DataCollection::Pointer& collection)
			{
				setDataCollection(collection, FIRST_ON_INIT);
			}

			void setDataCollection(const DataCollection::Pointer& collection, SlicePosition slice_pos_state)
			{
				std::cout << "Set data collection" << std::endl;
				View::setDataCollection(collection);
				std::cout << "Prepare " << m_DataCollection->size() << std::endl;
				prepare(slice_pos_state);
			}

			const ImageResliceParam::Pointer& getImageResliceParam()
			{
				return m_Param;
			}

			vtkImageStack* getImageStack()
			{
				return m_ImageStack;
			}

		public slots:
			virtual void resetCamera()
			{
				std::cout << "Reset" << std::endl;
				auto camera = m_Renderer->GetActiveCamera();
				pcl::Point3D<double> center = m_Param->getRegion().getMinPoint() + (m_Param->getRegion().getMaxPoint()-m_Param->getRegion().getMinPoint())/2;
				pcl::Point3D<double> camera_position(camera->GetPosition()),
					camera_focal(camera->GetFocalPoint());
				pcl::Point3D<double> normal(m_Param->getAxes()->GetElement(0,2), m_Param->getAxes()->GetElement(1,2), m_Param->getAxes()->GetElement(2,2));
				camera_position = normal*(camera_position-center).getDotProduct(normal) + center;
				camera->SetPosition(camera_position);
				camera->SetFocalPoint(center);
				camera->SetViewUp(m_Param->getYAxis());

				const double *spacing = m_Param->getSpacing();
				const int *extent = m_Param->getExtent();
				camera->SetParallelScale((extent[3]-extent[2]+1)*spacing[1]*0.5);
				render();
			}

			virtual void render()
			{
				if (this->isVisible()) {
					pcl_ForEach(m_DataMap, item) {
						item->second->update();
					}
					m_Renderer->ResetCameraClippingRange();
					this->GetRenderWindow()->Render();
					emit renderCalled();
				}
			}

		protected:
			ImageResliceParam::Pointer m_Param;
			boost::unordered_map<ImageDataBase*, ImageResliceActor::Pointer> m_DataMap;
			vtkSmartPointer<vtkImageStack> m_ImageStack;

			void prepare(SlicePosition slice_pos_state)
			{
				std::cout << "Preparing" << std::endl;
				std::vector<std::list<Data::Pointer>*> list_vector(2);
				if (this->m_DataCollection) {
					list_vector[0] = this->m_DataCollection->get(Data::IMAGE);
					list_vector[1] = this->m_DataCollection->get(Data::OVERLAY);
				}

				if (list_vector[0]!=NULL || list_vector[1]!=NULL) {
					std::cout << "Setting up" << std::endl;
					//Setting up image reslice param
					bool init = true;
					iterateImageDataList(list_vector, [&](ImageDataBase::Pointer image_data) {
						m_Param->add(image_data, init);
						init = false;
					});
					std::cout << m_Param->getRegion() << std::endl;

					std::cout << "Adjusting camera" << std::endl;
					//Adjusting camera
					auto corner_points = m_Param->getRegion().getCornerPoints();
					pcl::Point3D<double> normal(m_Param->getAxes()->GetElement(0,2), m_Param->getAxes()->GetElement(1,2), m_Param->getAxes()->GetElement(2,2));
					pcl::Point3D<double> center = m_Param->getRegion().getMinPoint() + (m_Param->getRegion().getMaxPoint()-m_Param->getRegion().getMinPoint())/2;
					double max_disp = -std::numeric_limits<double>::infinity(),
						min_disp = std::numeric_limits<double>::infinity();
					pcl_ForEach(corner_points, item) {
						pcl::Point3D<double> vec = *item - center;
						double disp = vec.getDotProduct(normal);
						if (disp>max_disp) max_disp = disp;
						if (disp<min_disp) min_disp = disp;
					}
					auto camera = m_Renderer->GetActiveCamera();
					pcl::Point3D<double> camera_pos;
					if (m_DataMap.empty()) { //Initialize camera to center of the image by default
						std::cout << "DataMap empty route" << std::endl;
						camera_pos = (normal*(min_disp-10)) + center;
						camera->SetViewUp(m_Param->getYAxis());
						const double *spacing = m_Param->getSpacing();
						const int *extent = m_Param->getExtent();
						camera->SetParallelScale((extent[3]-extent[2]+1)*spacing[1]*0.5);
						std::cout << (extent[3]-extent[2]+1)*spacing[1]*0.5 << std::endl;
					} else {
						pcl::Point3D<double> cur_pos(camera->GetPosition());
						double disp = min_disp - (cur_pos-center).getDotProduct(normal);
						camera_pos = (normal*(disp-10)) + cur_pos;
					}
					camera->SetPosition(camera_pos);
					camera->SetFocalPoint(center);
				pcl::Point3D<double> temp;
				camera->GetDirectionOfProjection(temp);
				std::cout << "Direction: " << temp << std::endl;
				std::cout << "Position: " << camera_pos << std::endl;
					m_Renderer->UpdateLightsGeometryToFollowCamera();

					//Updating slice position
					switch (slice_pos_state) {
					case NO_CHANGE:
					case FIRST_ON_INIT:
						if (!m_DataMap.empty()) break;
					case FIRST:
						m_Param->moveSliceToContain(center + normal*min_disp);
						break;
					case MIDDLE_ON_INIT:
						if (!m_DataMap.empty()) break;
					case MIDDLE:
						m_Param->moveSliceToContain(center);
						break;
					};
					std::cout << "Center " << center << std::endl;

					//Setting up the image actors (recyling any if possible)
					boost::unordered_map<ImageDataBase*, ImageResliceActor::Pointer> new_map;
					int layer = 0;
					iterateImageDataList(list_vector, [&](ImageDataBase::Pointer image_data) {
						auto iter = m_DataMap.find(image_data.get());
						ImageResliceActor::Pointer reslice_actor;
						if (iter==m_DataMap.end()) {
							reslice_actor = ImageResliceActor::New(image_data, m_Param);
						} else {
							reslice_actor = iter->second;
						}
						reslice_actor->getImageActor()->GetProperty()->SetLayerNumber(layer);
						++layer;
						new_map[image_data.get()] = reslice_actor;
					});
					m_DataMap = new_map;

					//Building image stack
					m_ImageStack->GetImages()->RemoveAllItems();
					pcl_ForEach(m_DataMap, item) {
						item->second->update();
						m_ImageStack->AddImage(item->second->getImageActor());
					}

				double *bound = m_ImageStack->GetBounds();
				std::cout << "Actor bound: (" << bound[0] << "," << bound[2] << "," << bound[4] 
					<< ") (" << bound[1] << "," << bound[3] << "," << bound[5] << ")" << std::endl;

	/*//Setting up VTK environment
	auto renderer = vtkSmartPointer<vtkRenderer>::New();
	auto renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renderer->SetBackground(1,1,1);
	renWin->AddRenderer( renderer );
	renWin->SetSize( 700, 700 );
	auto iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	auto istyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	iren->SetInteractorStyle(istyle);
	iren->SetRenderWindow(renWin);
	//Setting up renderer
	renderer->AddActor(m_ImageStack);
    renWin->Render();
	// Start interactive control
    iren->Start();*/

				} else {
					m_DataMap.clear();
					m_ImageStack->GetImages()->RemoveAllItems();
				}
			}

			template <class Func>
			void iterateImageDataList(std::vector<std::list<Data::Pointer>*> list_vector, Func func)
			{
				pcl_ForEach(list_vector, list) {
					if (*list!=NULL) pcl_ForEach(**list, item) func(boost::static_pointer_cast<ImageDataBase>(*item));
				}
			}
		};

	}
}

#endif