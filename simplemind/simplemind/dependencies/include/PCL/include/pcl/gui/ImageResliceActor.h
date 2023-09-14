#ifndef PCL_GUI_IMAGE_RESLICE_ACTOR
#define PCL_GUI_IMAGE_RESLICE_ACTOR

#include <pcl/exception.h>
#include <pcl/gui/ImageSourceBase.h>

#include <vtkSmartPointer.h>
#include <vtkImageReslice.h>
#include <vtkImageMapToColors.h>
#include <vtkImageActor.h>
#include <vtkMatrix4x4.h>
#include <vtkCellPicker.h>
#include <numeric>

#include <pcl/misc/ArrayToString.h>

namespace pcl
{
	namespace gui
	{

		class ImageResliceParam
		{
		public:
			typedef ImageResliceParam Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New()
			{
				return Pointer(new ImageResliceParam);
			}

			/******************************** Get methods ********************************/

			const pcl::Point3D<double>& getNormalStep() const
			{
				return m_NormalStep;
			}

			const pcl::Point3D<double>& getOrigin() const
			{
				return m_Origin;
			}

			const pcl::Point3D<double>& getXAxis() const
			{
				return m_XAxis;
			}

			const pcl::Point3D<double>& getYAxis() const
			{
				return m_YAxis;
			}

			const double* getSpacing() const
			{
				return m_Spacing;
			}

			const int* getExtent() const
			{
				return m_Extent;
			}

			vtkMatrix4x4* getAxes()
			{
				return m_Axes;
			}

			/******************************** Misc methods ********************************/

			void copy(const Pointer& obj)
			{
				m_NormalStep = obj->m_NormalStep;
				m_XAxis = obj->m_XAxis;
				m_YAxis = obj->m_YAxis;
				m_Origin = obj->m_Origin;
				for (int i=0; i<4; ++i) m_Extent[i] = obj->m_Extent[i];
				for (int i=0; i<2; ++i) m_Spacing[i] = obj->m_Spacing[i];
				m_Axes = obj->m_Axes;
			}

			void setOrigin(const pcl::Point3D<double>& origin)
			{
				m_Origin = origin;
			}

			void setNormalStep(const pcl::Point3D<double>& normal_step)
			{
				m_NormalStep = normal_step;
				m_MinChangeOffset = m_NormalStep.getNorm()*0.1;
			}

			void setXAxis(const pcl::Point3D<double>& x_axis, double spacing)
			{
				m_XAxis = x_axis;
				m_Spacing[0] = spacing;
			}
			void setXAxis(const pcl::Point3D<double>& x_axis)
			{
				m_Spacing[0] = x_axis.getNorm();
				m_XAxis = x_axis;
				m_XAxis /= m_Spacing[0];
			}

			void setYAxis(const pcl::Point3D<double>& y_axis, double spacing)
			{
				m_YAxis = y_axis;
				m_Spacing[1] = spacing;
			}
			void setYAxis(const pcl::Point3D<double>& y_axis)
			{
				m_Spacing[1] = y_axis.getNorm();
				m_YAxis = y_axis;
				m_YAxis /= m_Spacing[1];
			}
		
			void setUsing(const ImageSourceBase::Pointer& image_source, vtkMatrix4x4* mat)
			{
				m_Axes = mat;
				auto dummy = image_source->getDummyImage();
				//Collecting information from image source
				for (int i=0; i<3; ++i) {
					m_XAxis[i] = m_Axes->GetElement(i, 0);
					m_YAxis[i] = m_Axes->GetElement(i, 1);
					m_NormalStep[i] = m_Axes->GetElement(i, 2);
				}
				m_Origin = dummy->getOrigin();
				//Scale axis according to voxel spacing
				m_XAxis = computeOptimalSamplingVector(m_XAxis, dummy->getSpacing());
				m_YAxis = computeOptimalSamplingVector(m_YAxis, dummy->getSpacing());
				m_NormalStep = computeOptimalSamplingVector(m_NormalStep, dummy->getSpacing());
				m_MinChangeOffset = m_NormalStep.getNorm()*0.1;
				m_Spacing[0] = m_XAxis.getNorm(); m_XAxis /= m_Spacing[0];
				m_Spacing[1] = m_YAxis.getNorm(); m_YAxis /= m_Spacing[1];
				//Computing extent of slice
				setExtentToCover(dummy->getPhysicalRegion());
			}

			void setExtentToCover(const pcl::Region3D<double>& region)
			{
				auto corner_points = region.getCornerPoints();
				bool init = true;
				pcl_ForEach(corner_points, point) {
					pcl::Point3D<double> vec = *point-m_Origin;
					double x = vec.getDotProduct(m_XAxis)/m_Spacing[0], 
						y = vec.getDotProduct(m_YAxis)/m_Spacing[1];
					if (init) {
						m_Extent[0] = std::floor(x);
						m_Extent[1] = std::ceil(x);
						m_Extent[2] = std::floor(y);
						m_Extent[3] = std::ceil(y);
						init = false;
					} else {
						m_Extent[0] = std::min<int>(std::floor(x), m_Extent[0]);
						m_Extent[1] = std::max<int>(std::ceil(x), m_Extent[1]);
						m_Extent[2] = std::min<int>(std::floor(y), m_Extent[2]);
						m_Extent[3] = std::max<int>(std::ceil(y), m_Extent[3]);
					}
				}
			}

			bool moveSliceToContain(const pcl::Point3D<double>& p)
			{
				pcl::Point3D<double> slice_origin = m_Origin;
				pcl::Point3D<double> normal(m_Axes->GetElement(0,2), m_Axes->GetElement(1,2), m_Axes->GetElement(2,2));
				double old_offset = normal.getDotProduct(pcl::Point3D<double>(m_Axes->GetElement(0,3), m_Axes->GetElement(1,3), m_Axes->GetElement(2,3))-m_Origin);
				double offset = normal.getDotProduct(p-m_Origin);
				if (pcl::abs(offset-old_offset)<m_MinChangeOffset) return false;
				slice_origin += normal*offset;
				double max_change = 0;
				for (int i=0; i<3; ++i) {
					m_Axes->SetElement(i,3, slice_origin[i]);
				}
				return true;;
			}

		protected:
			pcl::Point3D<double> m_NormalStep;
			double m_MinChangeOffset;
			pcl::Point3D<double> m_XAxis, m_YAxis;
			pcl::Point3D<double> m_Origin;
			int m_Extent[4];
			double m_Spacing[2];
			vtkSmartPointer<vtkMatrix4x4> m_Axes;

			ImageResliceParam() {}

			pcl::Point3D<double> computeOptimalSamplingVector(const pcl::Point3D<double>& vec, const pcl::Point3D<double>& spc) const
			{
				pcl::Point3D<double> result;
				double result_norm_sqr = 0;
				for (int i=0; i<3; ++i) {
					pcl::Point3D<double> v = vec*spc[i];
					bool valid = true;
					for (int j=0; j<3; ++j) if (v[j]>spc[j]) {
						valid = false;
						break;
					}
					if (valid) {
						double norm_sqr = v.getNormSqr();
						if (norm_sqr>result_norm_sqr) {
							result = v;
							result_norm_sqr = norm_sqr;
						}
					}
				}
				if (result_norm_sqr==0) pcl_ThrowException(pcl::Exception(), "Norm of resulting vector is zero");
				return result;
			}
		};


		class ImageResliceActor: private boost::noncopyable
		{
		public:
			typedef ImageResliceActor Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(const ImageSourceBase::Pointer& image_source)
			{
				return Pointer(new Self(image_source));
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

			const ImageSourceBase::Pointer& getImageSource()
			{
				return m_ImageSource;
			}

			const double* getImageResliceOrigin() const
			{
				return m_ImageReslice->GetResliceAxesOrigin();
			}

			//pcl::Point3D<double> getPhysicalCoordFromCellPicker(vtkCellPicker* cell_picker) const
			//{
			//	pcl::Point3D<double> coord(cell_picker->GetCellIJK()); 
			//	/*double *pcoords = cell_picker->GetPCoords();
			//	for (int i=0; i<3; ++i) coord[i] += pcoords[i];*/

			//	pcl::Point3D<double> physical_coord(getImageResliceOrigin());
			//	physical_coord += m_Param->getXAxis()*coord[0];
			//	physical_coord += m_Param->getYAxis()*coord[1];

			//	pcl::Point3D<double> vtk_coord(cell_picker->GetPickPosition());
			//	qDebug() << coord.toString().c_str() << " " << physical_coord.toString().c_str() << " " << vtk_coord.toString().c_str();
			//	return physical_coord;
			//}

			const ImageResliceParam::Pointer& getParam()
			{
				return m_Param;
			}

			void setParam(const ImageResliceParam::Pointer& param)
			{
				m_Param = param;
				m_ImageReslice->SetResliceAxes(m_Param->getAxes());
				m_ImageReslice->SetOutputExtent(m_Param->getExtent()[0], m_Param->getExtent()[1], m_Param->getExtent()[2], m_Param->getExtent()[3], 0, 0);
				m_ImageReslice->SetOutputSpacing(m_Param->getSpacing()[0], m_Param->getSpacing()[1], 0);
				m_ImageReslice->SetOutputDimensionality(2);
				m_ImageActor->SetUserMatrix(m_Param->getAxes());
				modified();
			}

			void modified()
			{
				//m_ImageReslice->SetOutputOrigin(m_Param->getAxes()->GetElement(0,3), m_Param->getAxes()->GetElement(1,3), m_Param->getAxes()->GetElement(2,3));
				m_ImageReslice->Modified();
				m_ImageActor->Modified();
				//qDebug() << "Actor: " << pcl::Point3D<double>(m_ImageActor->GetCenter()).toString().c_str();
				updateVisiblity();
			}

			void updateVisiblity()
			{
				if (m_ImageSource->visibility()) m_ImageActor->VisibilityOn();
				else m_ImageActor->VisibilityOff();
			}

		protected:
			ImageSourceBase::Pointer m_ImageSource;
			ImageResliceParam::Pointer m_Param;
			vtkSmartPointer<vtkImageReslice> m_ImageReslice;
			vtkSmartPointer<vtkImageActor> m_ImageActor;
	
			ImageResliceActor(const ImageSourceBase::Pointer& image_source)
			{
				m_Param = ImageResliceParam::New();
				auto axes = vtkSmartPointer<vtkMatrix4x4>::New();
				m_Param->setUsing(image_source, axes);

				m_ImageSource = image_source;
				m_ImageReslice = vtkImageReslice::New();
				m_ImageReslice->SetInterpolationModeToNearestNeighbor();
				m_ImageReslice->SetInput(m_ImageSource->getImageData());
				m_ImageReslice->SetResliceAxes(m_Param->getAxes());
				m_ImageReslice->SetOutputExtent(m_Param->getExtent()[0], m_Param->getExtent()[1], m_Param->getExtent()[2], m_Param->getExtent()[3], 0, 0);
				m_ImageReslice->SetOutputSpacing(m_Param->getSpacing()[0], m_Param->getSpacing()[1], 0);
				m_ImageReslice->SetOutputDimensionality(2);
				m_ImageReslice->SetBackgroundLevel(m_ImageSource->getMinMax().get<0>());

				auto color = vtkSmartPointer<vtkImageMapToColors>::New();
				color->SetLookupTable(m_ImageSource->getLookupTable());
				color->SetInputConnection(m_ImageReslice->GetOutputPort());
				m_ImageActor = vtkImageActor::New();
				m_ImageActor->SetInput(color->GetOutput());
				m_ImageActor->InterpolateOff();
			}
		};

	}
}

#endif
