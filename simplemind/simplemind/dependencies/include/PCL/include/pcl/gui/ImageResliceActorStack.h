#ifndef PCL_GUI_IMAGE_RESLICE_ACTOR_STACK
#define PCL_GUI_IMAGE_RESLICE_ACTOR_STACK

#include <pcl/gui/ImageResliceActor.h>
#include <pcl/gui/ImageCrossHairs.h>
#include <vtkImageProperty.h>
#include <vtkImageStack.h>
#include <vtkImageSliceCollection.h>
#include <vector>
#include <utility>

namespace pcl
{
	namespace gui
	{

		class ImageResliceActorStack
		{
		public:
			typedef ImageResliceActorStack Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New()
			{
				Pointer obj(new Self);
				return obj;
			}

			/*************** Stack related ***************/

			void addScalarImageResliceActor(const ImageResliceActor::Pointer& img)
			{
				m_Image.push_back(img);
				modified();
			}
			void addSegmentImageResliceActor(const ImageResliceActor::Pointer& img)
			{
				m_Segment.push_back(img);
				modified();
			}

			void remove(const ImageResliceActor::Pointer& img)
			{
				remove(img, m_Image);
				remove(img, m_Segment);
			}

			void clear()
			{
				m_ImageStack->GetImages()->RemoveAllItems();
				m_Image.clear();
				m_Segment.clear();
				modified();
			}

			/*************** Get methods ***************/

			std::vector<ImageResliceActor::Pointer>& getImages()
			{
				return m_Image;
			}

			std::vector<ImageResliceActor::Pointer>& getSegments()
			{
				return m_Segment;
			}

			vtkImageStack* getImageStack()
			{
				return m_ImageStack;
			}

			const ImageResliceParam::Pointer& getResliceParam()
			{
				return m_ResliceParam;
			}

			const pcl::Point3D<double>& getNormalStep() const
			{
				return m_ResliceParam->getNormalStep();
			}

			vtkMatrix4x4* getResliceAxes()
			{
				return m_ResliceAxes;
			}

			const ImageCrossHairs::Pointer& getImageCrossHairs()
			{
				return m_CrossHairs;
			}

			pcl::Point3D<double> getImageResliceOrigin() const
			{
				return pcl::Point3D<double>(m_ResliceAxes->GetElement(0,3),m_ResliceAxes->GetElement(1,3),m_ResliceAxes->GetElement(2,3));
			}

			pcl::Region3D<double> getFullPhysicalRegion() const
			{
				pcl::Region3D<double> overall_region;
				overall_region.reset();
				for (int i=0; i<m_Image.size(); ++i) overall_region.add(m_Image[i]->getImageSource()->getPhysicalRegion());
				for (int i=0; i<m_Segment.size(); ++i) overall_region.add(m_Segment[i]->getImageSource()->getPhysicalRegion());
				return overall_region;
			}

			/*************** Set methods ***************/

			void setImageResliceAxes(const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis, const pcl::Point3D<double>& z_axis)
			{
				m_ResliceAxes->SetElement(0,0, x_axis[0]); m_ResliceAxes->SetElement(0,1, y_axis[0]); m_ResliceAxes->SetElement(0,2, z_axis[0]);
				m_ResliceAxes->SetElement(1,0, x_axis[1]); m_ResliceAxes->SetElement(1,1, y_axis[1]); m_ResliceAxes->SetElement(1,2, z_axis[1]);
				m_ResliceAxes->SetElement(2,0, x_axis[2]); m_ResliceAxes->SetElement(2,1, y_axis[2]); m_ResliceAxes->SetElement(2,2, z_axis[2]);
				buildImageStack();
			}

			void setCameraOrientationMultiplier(double view_direction_multiplier, double view_up_multiplier)
			{
				m_CameraOrientationMultiplier[0] = view_direction_multiplier;
				m_CameraOrientationMultiplier[1] = view_up_multiplier;
			}

			void setImageResliceToAxial(bool flip_z)
			{
				m_ResliceAxes->SetElement(0,0, 1); m_ResliceAxes->SetElement(0,1, 0); m_ResliceAxes->SetElement(0,2, 0);
				m_ResliceAxes->SetElement(1,0, 0); m_ResliceAxes->SetElement(1,1, 1); m_ResliceAxes->SetElement(1,2, 0);
				m_ResliceAxes->SetElement(2,0, 0); m_ResliceAxes->SetElement(2,1, 0); m_ResliceAxes->SetElement(2,2, 1);
				buildImageStack();
				setCameraOrientationMultiplier(1, -1);
				if (flip_z) m_ResliceParam->setNormalStep(-m_ResliceParam->getNormalStep());
			}
			void setImageResliceToCoronal(bool flip_z)
			{
				m_ResliceAxes->SetElement(0,0, 1); m_ResliceAxes->SetElement(0,1, 0); m_ResliceAxes->SetElement(0,2, 0);
				m_ResliceAxes->SetElement(1,0, 0); m_ResliceAxes->SetElement(1,1, 0); m_ResliceAxes->SetElement(1,2, 1);
				m_ResliceAxes->SetElement(2,0, 0); m_ResliceAxes->SetElement(2,1, 1); m_ResliceAxes->SetElement(2,2, 0);
				buildImageStack();
				if (!flip_z) setCameraOrientationMultiplier(-1, -1);
				else setCameraOrientationMultiplier(1, 1);
			}
			void setImageResliceToSagittal(bool flip_z)
			{
				m_ResliceAxes->SetElement(0,0, 0); m_ResliceAxes->SetElement(0,1, 0); m_ResliceAxes->SetElement(0,2, 1);
				m_ResliceAxes->SetElement(1,0, 1); m_ResliceAxes->SetElement(1,1, 0); m_ResliceAxes->SetElement(1,2, 0);
				m_ResliceAxes->SetElement(2,0, 0); m_ResliceAxes->SetElement(2,1, 1); m_ResliceAxes->SetElement(2,2, 0);
				buildImageStack();
				if (!flip_z) setCameraOrientationMultiplier(1, -1);
				else setCameraOrientationMultiplier(-1, 1);
				m_ResliceParam->setNormalStep(-m_ResliceParam->getNormalStep());
			}

			void setVisiblity(bool en)
			{
				m_ImageStack->SetVisibility(en);
				m_CrossHairs->setVisibility(m_ShowCrossHairs && m_ImageStack->GetVisibility());
			}

			pcl::Point3D<double> getCameraDirection()
			{
				return pcl::Point3D<double>(
					m_ResliceAxes->GetElement(0,2)*m_CameraOrientationMultiplier[0],
					m_ResliceAxes->GetElement(1,2)*m_CameraOrientationMultiplier[0],
					m_ResliceAxes->GetElement(2,2)*m_CameraOrientationMultiplier[0]
				);
			}
			pcl::Point3D<double> getCameraViewUp()
			{
				return pcl::Point3D<double>(
					m_ResliceAxes->GetElement(0,1)*m_CameraOrientationMultiplier[1],
					m_ResliceAxes->GetElement(1,1)*m_CameraOrientationMultiplier[1],
					m_ResliceAxes->GetElement(2,1)*m_CameraOrientationMultiplier[1]
				);
			}

			/*************** Misc methods ***************/

			void applyParamChange()
			{
				for (int i=0; i<m_Image.size(); ++i) m_Image[i]->setParam(m_ResliceParam);
				for (int i=0; i<m_Segment.size(); ++i) m_Segment[i]->setParam(m_ResliceParam);
				m_CrossHairs->updateLineGeometry(this->getFullPhysicalRegion());
				modified();
			}

			void showCrossHairs(bool en)
			{
				m_ShowCrossHairs = en;
				m_CrossHairs->setVisibility(m_ShowCrossHairs && m_ImageStack->GetVisibility());
			}

			inline void modified()
			{
				m_Modified = true;
			}

			void update()
			{
				if (m_Modified) {
					computeLayerNumber();
					buildImageStack();
				}
				m_Modified = false;
				for (int i=0; i<m_Image.size(); ++i) m_Image[i]->updateVisiblity();
				for (int i=0; i<m_Segment.size(); ++i) m_Segment[i]->updateVisiblity();
			}

			bool setSliceToContain(const Point3D<double>& p)
			{
				m_CrossHairs->setPoint(p);
				if (m_ResliceParam->moveSliceToContain(p)) {
					for (int i=0; i<m_Image.size(); ++i) m_Image[i]->modified();
					for (int i=0; i<m_Segment.size(); ++i) m_Segment[i]->modified();
					return true;
				}
				return false;
			}

			void setActiveLayer(int layer)
			{
				m_ImageStack->SetActiveLayer(layer);
			}

			const ImageResliceActor::Pointer& getActiveImageResliceActor()
			{
				int layer = m_ImageStack->GetActiveLayer();
				if (layer<m_Image.size()) return m_Image[layer];
				return m_Segment[layer-m_Image.size()];
			}

		protected:
			std::vector<ImageResliceActor::Pointer> m_Image, m_Segment;
			double m_CameraOrientationMultiplier[2];
			ImageResliceParam::Pointer m_ResliceParam;
			vtkSmartPointer<vtkMatrix4x4> m_ResliceAxes;
			vtkSmartPointer<vtkImageStack> m_ImageStack;
			ImageCrossHairs::Pointer m_CrossHairs;
			bool m_ShowCrossHairs;
			bool m_Modified;

			ImageResliceActorStack()
			{
				m_ResliceParam = ImageResliceParam::New();
				m_ResliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
				m_ResliceAxes->Identity();
				m_ImageStack = vtkSmartPointer<vtkImageStack>::New();
				m_CrossHairs = ImageCrossHairs::New(m_ResliceAxes, m_CameraOrientationMultiplier);
				m_ShowCrossHairs = true;
				m_Modified = true;
				setCameraOrientationMultiplier(-1,1);
			}

			void remove(const ImageResliceActor::Pointer &img, std::vector<ImageResliceActor::Pointer>& vector)
			{
				std::vector<ImageResliceActor::Pointer> temp;
				temp.reserve(vector.size());
				for (int i=0; i<vector.size(); ++i) {
					if (vector[i]!=img) temp.push_back(vector[i]);
					else {
						modified();
					}
				}
				vector = std::move(temp);
			}

			void buildImageStack()
			{
				m_ImageStack->GetImages()->RemoveAllItems();
				//Collecting individual param
				std::vector<ImageResliceParam::Pointer> param_set;
				pcl::Region3D<double> overall_region;
				overall_region.reset();
				for (int i=0; i<m_Image.size(); ++i) {
					auto param = ImageResliceParam::New();
					param->setUsing(m_Image[i]->getImageSource(), m_ResliceAxes);
					overall_region.add(m_Image[i]->getImageSource()->getPhysicalRegion());
					param_set.push_back(param);
				}
				for (int i=0; i<m_Segment.size(); ++i) {
					auto param = ImageResliceParam::New();
					param->setUsing(m_Segment[i]->getImageSource(), m_ResliceAxes);
					overall_region.add(m_Segment[i]->getImageSource()->getPhysicalRegion());
					param_set.push_back(param);
				}
				if (param_set.empty()) return;
				//Generating overall param
				m_ResliceParam->copy(param_set[0]);
				pcl::Point3D<double> normal_step = m_ResliceParam->getNormalStep(), 
					x_axis = m_ResliceParam->getXAxis()*m_ResliceParam->getSpacing()[0], 
					y_axis = m_ResliceParam->getYAxis()*m_ResliceParam->getSpacing()[1];
				pcl_ForEach(param_set, item) {
					normal_step.min((*item)->getNormalStep());
					x_axis.min((*item)->getXAxis()*(*item)->getSpacing()[0]);
					y_axis.min((*item)->getYAxis()*(*item)->getSpacing()[1]);
				}
				m_ResliceParam->setNormalStep(normal_step);
				m_ResliceParam->setXAxis(x_axis);
				m_ResliceParam->setYAxis(y_axis);
				m_ResliceParam->setExtentToCover(overall_region);
				//Updating image stack
				applyParamChange();
				for (int i=0; i<m_Image.size(); ++i) m_ImageStack->AddImage(m_Image[i]->getImageActor());
				for (int i=0; i<m_Segment.size(); ++i) m_ImageStack->AddImage(m_Segment[i]->getImageActor());
			}

			void computeLayerNumber()
			{
				int layer = 0;
				for (int i=0; i<m_Image.size(); ++i) {
					m_Image[i]->getImageActor()->GetProperty()->SetLayerNumber(layer);
					if (layer==0) m_Image[i]->getImageActor()->GetProperty()->BackingOn();
					else m_Image[i]->getImageActor()->GetProperty()->BackingOff();
					++layer;
				}
				for (int i=0; i<m_Segment.size(); ++i) {
					m_Segment[i]->getImageActor()->GetProperty()->SetLayerNumber(layer);
					if (layer==0) m_Segment[i]->getImageActor()->GetProperty()->BackingOn();
					else m_Segment[i]->getImageActor()->GetProperty()->BackingOff();
					++layer;
				}
			}
		};

	}
}

#endif
