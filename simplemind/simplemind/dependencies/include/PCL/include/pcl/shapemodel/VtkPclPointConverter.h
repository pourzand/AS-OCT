#ifndef VTK_PCL_POINT_CONVERTER
#define VTK_PCL_POINT_CONVERTER

#include <pcl/shapemodel/PointCloud.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

namespace pcl
{
	namespace shapemodel
	{

		class VtkPclPointConverter
		{
		public:
			template <class T>
			static PointCloud<T> ToPointCloud(vtkPoints* vp)
			{
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");
				PointCloud<T> result;
				result.resizePoint(vp->GetNumberOfPoints());
				double buffer[3];
				for (int i=0; i<vp->GetNumberOfPoints(); ++i) {
					vp->GetPoint(i, buffer);
					int index = i*3;
					for (int j=0; j<3; ++j) result[index+j] = buffer[j];
				}
				return std::move(result);
			}

			template <class T>
			static std::vector<T> ToPointVector(vtkPoints* vp)
			{
				if (T::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point dimension not valid!");
				std::vector<T> result;
				result.resize(vp->GetNumberOfPoints());
				T buffer;
				for (int i=0; i<vp->GetNumberOfPoints(); ++i) {
					vp->GetPoint(i, &buffer[0]);
					result[i] = buffer;
				}
				return std::move(result);
			}

			template <class T>
			static void AssignToVtkPoints(vtkPoints* vp, const PointCloud<T>& p)
			{
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");
				if (p.pointSize()!=vp->GetNumberOfPoints()) pcl_ThrowException(pcl::Exception(), "Inputs are of different sizes!");
				for (int i=0; i<vp->GetNumberOfPoints(); ++i) {
					int index = i*3;
					vp->SetPoint(i, p[index], p[index+1], p[index+2]); 
				}
			}

			template <class T>
			static vtkSmartPointer<vtkPoints> GetVtkPoints(const PointCloud<T>& p)
			{
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");
				vtkSmartPointer<vtkPoints> result = vtkSmartPointer<vtkPoints>::New();
				result->SetNumberOfPoints(p.pointSize());
				for (int i=0; i<result->GetNumberOfPoints(); ++i) {
					int index = i*3;
					result->SetPoint(i, p[index], p[index+1], p[index+2]); 
				}
				return result;
			}

			template <class T>
			static vtkSmartPointer<vtkPoints> GetVtkPoints(const std::vector<T>& p)
			{
				if (T::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point dimension not valid!");
				vtkSmartPointer<vtkPoints> result = vtkSmartPointer<vtkPoints>::New();
				result->SetNumberOfPoints(p.size());
				for (int i=0; i<result->GetNumberOfPoints(); ++i) {
					const T& temp = p[i];
					result->SetPoint(i, temp.x(), temp.y(), temp.z()); 
				}
				return result;
			}

		};

	}
}

#endif