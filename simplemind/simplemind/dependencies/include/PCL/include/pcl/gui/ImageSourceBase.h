#ifndef PCL_GUI_IMAGE_SOURCE_BASE
#define PCL_GUI_IMAGE_SOURCE_BASE

#include <pcl/image.h>
#include <pcl/image/DummyImage.h>
#include <pcl/vtk/ItkToVtkHelper.h>
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#include <typeinfo.h>

namespace pcl
{
	namespace gui
	{

		class ImageSourceBase: private boost::noncopyable
		{
		public:
			typedef ImageSourceBase Self;
			typedef boost::shared_ptr<Self> Pointer;

			virtual bool imageIsType(const type_info& info) const=0;
			virtual pcl::ImageBase::Pointer getImageBase()=0;
			virtual pcl::DummyImage::Pointer getDummyImage() const=0;
			virtual pcl::Region3D<double> getPhysicalRegion() const=0;
			virtual boost::tuple<double,double> getMinMax() const=0;
			virtual pcl::Point3D<int> getImageCoordinate(const pcl::Point3D<double>& physical_coord) const=0;
			virtual pcl::Point3D<double> getPhysicalCoordinate(const pcl::Point3D<int>& image_coord) const=0;
			virtual double getImageValue(const pcl::Point3D<int>& p) const=0;
			virtual bool contain(const pcl::Point3D<int>& p) const=0;
			virtual void setImageValue(const pcl::Point3D<int>& p, double val)=0;
			virtual vtkImageData* getImageData()=0;

			void visibilityOn()
			{
				m_Visibility = true;
			}
			void visibilityOff()
			{
				m_Visibility = false;
			}
			bool visibility() const
			{
				return m_Visibility;
			}

			/********************** Lookup table related **********************/

			vtkLookupTable* getLookupTable()
			{
				return m_LookupTable;
			}

			double* getLookupTableRange()
			{
				return m_LookupTable->GetRange();
			}

			void getWindowLevel(double& offset, double& width)
			{
				double* range = getLookupTableRange();
				width = range[1]-range[0];
				offset = range[0]+ width*0.5;
			}

			void setLookupTableRange(double min_val, double max_val)
			{
				m_LookupTable->SetRange(min_val, max_val);
				m_LookupTable->Build();
			}

			void setWindowLevel(double offset, double width)
			{
				if (width<1) width = 1;
				m_LookupTable->SetNumberOfTableValues(width);
				setLookupTableRange(offset-width*0.5, offset+width*0.5);
			}

		protected:
			vtkSmartPointer<vtkLookupTable> m_LookupTable;
			bool m_Visibility;

			ImageSourceBase()
			{
				m_LookupTable = vtkSmartPointer<vtkLookupTable>::New();
				m_Visibility = true;
			}
		};

	}
}

#endif
