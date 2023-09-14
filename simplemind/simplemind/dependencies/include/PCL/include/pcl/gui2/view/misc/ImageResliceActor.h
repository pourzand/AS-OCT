#ifndef PCL_GUI_IMAGE_RESLICE_ACTOR
#define PCL_GUI_IMAGE_RESLICE_ACTOR

#include <pcl/exception.h>
#include <pcl/gui2/ModificationTracker.h>
#include <pcl/gui2/data/ImageData.h>

#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkTransform.h>
#include <vtkImageActor.h>
#include <vtkCellPicker.h>
#include <numeric>

#include <pcl/misc/ArrayToString.h>

namespace pcl
{
	namespace gui
	{

		class ImageResliceActor
		{
		public:
			typedef ImageResliceActor Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(const ImageDataBase::Pointer& image, const ImageResliceParam::Pointer& param)
			{
				return Pointer(new Self(image, param));
			}

			/******************************** Get methods ********************************/

			vtkImageReslice* getImageReslice()
			{
				return m_ImageReslice;
			}

			vtkImageActor* getImageActor()
			{
				return m_ImageActor;
			}

			const ImageDataBase::Pointer& getImageData()
			{
				return m_ImageData;
			}

			const ImageResliceParam::Pointer& getParam()
			{
				return m_Param;
			}

			void update()
			{
				if (m_ImageDataTracker.isModified()) {
					m_ImageActor->SetVisibility(m_ImageData->visibility());
					m_ImageDataTracker.updated();
				}
				if (m_ParamTracker.isModified()) {
					m_ImageReslice->SetResliceAxes(m_ImageData->getLocalMatrixFromWorld(m_Param->getAxes()));
					//m_ImageReslice->SetResliceAxes(m_Param->getAxes());
					m_ImageReslice->SetOutputExtent(m_Param->getExtent()[0], m_Param->getExtent()[1], m_Param->getExtent()[2], m_Param->getExtent()[3], 0, 0);
					m_ImageReslice->SetOutputSpacing(m_Param->getSpacing()[0], m_Param->getSpacing()[1], 0);
					m_ImageReslice->SetOutputDimensionality(2);
					//m_ImageActor->SetUserMatrix(m_Param->getAxes());
					m_ImageActor->SetUserMatrix(m_Param->getAxes());
					m_ParamTracker.updated();
				}
			}

		protected:
			ModificationTracker m_ImageDataTracker, m_ParamTracker;
			ImageDataBase::Pointer m_ImageData;
			ImageResliceParam::Pointer m_Param;
			vtkSmartPointer<vtkImageReslice> m_ImageReslice;
			vtkSmartPointer<vtkImageActor> m_ImageActor;
	
			ImageResliceActor(const ImageDataBase::Pointer& image, const ImageResliceParam::Pointer& param)
			{
				m_Param = param;
				m_ParamTracker.reset(m_Param);
				m_ImageData = image;
				m_ImageDataTracker.reset(m_ImageData);

				m_ImageReslice = vtkSmartPointer<vtkImageReslice>::New();
				m_ImageReslice->SetInterpolationModeToNearestNeighbor();
				m_ImageReslice->SetInput(m_ImageData->getImageData());
				m_ImageReslice->SetResliceAxes(m_ImageData->getLocalMatrixFromWorld(m_Param->getAxes()));
				//m_ImageReslice->SetResliceAxes(m_Param->getAxes());
				m_ImageReslice->SetOutputExtent(m_Param->getExtent()[0], m_Param->getExtent()[1], m_Param->getExtent()[2], m_Param->getExtent()[3], 0, 0);
				m_ImageReslice->SetOutputSpacing(m_Param->getSpacing()[0], m_Param->getSpacing()[1], 0);
				m_ImageReslice->SetOutputDimensionality(2);
				m_ImageReslice->SetBackgroundLevel(m_ImageData->getMinMax().get<0>());

				auto color = vtkSmartPointer<vtkImageMapToColors>::New();
				color->SetLookupTable(m_ImageData->getColorLookup());
				color->SetInput(m_ImageReslice->GetOutput());

				m_ImageActor = vtkSmartPointer<vtkImageActor>::New();
				m_ImageActor->SetInput(color->GetOutput());
				m_ImageActor->SetVisibility(m_ImageData->visibility());
				m_ImageActor->InterpolateOff();
				//m_ImageActor->SetUserMatrix(m_Param->getAxes());
				m_ImageActor->SetUserMatrix(m_Param->getAxes());

				double *bound = m_ImageActor->GetBounds();
				std::cout << "#Actor bound: (" << bound[0] << "," << bound[2] << "," << bound[4] 
					<< ") (" << bound[1] << "," << bound[3] << "," << bound[5] << ")" << std::endl;
			}
		};

	}
}

#endif
