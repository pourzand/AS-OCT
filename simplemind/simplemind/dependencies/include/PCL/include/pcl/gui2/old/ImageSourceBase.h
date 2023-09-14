#ifndef PCL_GUI_IMAGE_SOURCE_BASE
#define PCL_GUI_IMAGE_SOURCE_BASE

#include <pcl/image.h>
#include <pcl/image/DummyImage.h>
#include <pcl/vtk/macro.h>
#include <pcl/vtk/ItkToVtkHelper.h>
#include <vtkScalarsToColors.h>
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

			void setColorLookup(vtkScalarsToColors* lookup)
			{
				m_ColorLookup = lookup;
			}

			vtkScalarsToColors* getColorLookup()
			{
				return m_ColorLookup;
			}

			template <class T>
			T* getColorLookup()
			{
				return dynamic_cast<T*>(m_ColorLookup.GetPointer());
			}

		protected:
			vtk_Pointer(vtkScalarsToColors, m_ColorLookup);
			bool m_Visibility;

			ImageSourceBase()
			{
				m_Visibility = true;
			}
		};

	}
}

#endif
