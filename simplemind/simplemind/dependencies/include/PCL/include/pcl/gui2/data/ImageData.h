#ifndef PCL_GUI_IMAGE_DATA
#define PCL_GUI_IMAGE_DATA

#include <pcl/image.h>
#include <pcl/image/DummyImage.h>
#include <pcl/gui2/data/Data.h>
#include <pcl/vtk/ItkToVtkHelper.h>
#include <pcl/gui2/data/misc/DiscreteLookupTableWrapper.h>
#include <pcl/gui2/data/misc/WindowLevelLookupTableWrapper.h>
#include <vtkMatrix4x4.h>
#include <pcl/exception.h>
#include <vtkScalarsToColors.h>
#include <vtkSmartPointer.h>
#include <typeinfo.h>

namespace pcl
{
	namespace gui
	{

		class ImageDataBase: public Data
		{
		public:
			typedef ImageDataBase Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

			virtual DataType type() const
			{
				return m_Type;
			}

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
			virtual vtkSmartPointer<vtkMatrix4x4> getLocalMatrixFromWorld(vtkMatrix4x4* matrix)=0;

			vtkScalarsToColors* getColorLookup()
			{
				return m_ColorLookup;
			}
			template <class T>
			T* getColorLookup()
			{
				return dynamic_cast<T*>(getColorLookup());
			}

			void setVisibility(bool en)
			{
				if (en!=m_Visibility) {
					m_Visibility = en;
					this->modified();
				}
			}
			bool visibility() const
			{
				return m_Visibility;
			}

		protected:
			vtkSmartPointer<vtkScalarsToColors> m_ColorLookup;
			bool m_Visibility;
			const DataType m_Type;

			ImageDataBase(DataType type):m_Type(type)
			{
				m_Visibility = true;
			}
		};



		template <class PclImageType>
		class ImageData: public ImageDataBase
		{
		public:
			typedef ImageData Self;
			typedef PclImageType ImageType;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;
			typedef typename pcl::vtk::ItkToVtkHelper::GetConverterType<typename ImageType::Pointer>::type ImageConverterType;

			static Pointer New(DataType type, const typename ImageType::Pointer& image, bool* is_alias=NULL)
			{
				Pointer obj(new Self(type, image, is_alias));
				return obj;
			}

			virtual bool imageIsType(const type_info& info) const
			{
				return info==typeid(ImageType);
			}

			virtual pcl::ImageBase::Pointer getImageBase()
			{
				return m_Image;
			}

			virtual pcl::DummyImage::Pointer getDummyImage() const
			{
				return pcl::DummyImage::New(m_Image);
			}

			virtual pcl::Region3D<double> getPhysicalRegion() const
			{
				return m_Image->getPhysicalRegion();
			}

			virtual boost::tuple<double,double> getMinMax() const
			{
				boost::tuple<double,double> result;
				pcl::ImageHelper::GetMinMax(m_Image, result.get<0>(), result.get<1>());
				return result;
			}

			virtual pcl::Point3D<int> getImageCoordinate(const pcl::Point3D<double>& physical_coord) const
			{
				return pcl::Point3D<int>().assignRound(m_Image->toImageCoordinate(physical_coord));
			}

			virtual pcl::Point3D<double> getPhysicalCoordinate(const pcl::Point3D<int>& image_coord) const
			{
				return m_Image->toPhysicalCoordinate(image_coord);
			}

			virtual double getImageValue(const pcl::Point3D<int>& p) const
			{
				return m_Image->get(p);
			}

			virtual bool contain(const pcl::Point3D<int>& p) const
			{
				return m_Image->contain(p);
			}

			virtual void setImageValue(const pcl::Point3D<int>& p, double val) 
			{
				m_Image->set(p, val);
				m_ImageModified = true;
			}

			virtual vtkImageData* getImageData()
			{
				return m_ImageConverter->GetOutput();
			}

			virtual vtkSmartPointer<vtkMatrix4x4> getLocalMatrixFromWorld(vtkMatrix4x4* matrix)
			{
				if (m_OrientationMatrix.GetPointer()!=NULL) {
					auto result = vtkSmartPointer<vtkMatrix4x4>::New();
					vtkMatrix4x4::Multiply4x4(m_InverseOrientationMatrix, matrix, result);
					return result;
				} else return vtkSmartPointer<vtkMatrix4x4>(matrix);
			}

			const typename ImageType::Pointer& getImage()
			{
				return m_Image;
			}

			const typename ImageConverterType::Pointer& getImageConverter()
			{
				return m_ImageConverter;
			}

			virtual void modified()
			{
				ModifiableObject::modified();
				if (m_ImageModified) {
					getImageData()->Modified();
					m_ImageModified = false;
				}
			}

		protected:
			vtkSmartPointer<vtkMatrix4x4> m_OrientationMatrix, m_InverseOrientationMatrix;
			typename ImageType::Pointer m_Image;
			typename ImageConverterType::Pointer m_ImageConverter;
			bool m_ImageModified;

			ImageData(DataType type, const typename ImageType::Pointer& image, bool* is_alias=NULL): ImageDataBase(type)
			{
				m_Image = image;
				m_ImageConverter = pcl::vtk::ItkToVtkHelper::GetConveter(image, is_alias);
				this->m_ColorLookup = vtkSmartPointer<vtkLookupTable>::New();

				auto minmax = getMinMax();
				if (type==Data::IMAGE) {
					WindowLevelLookupTableWrapper wrapper(this->m_ColorLookup);
					wrapper.reset(minmax.get<0>(), minmax.get<1>());
				} else if (type==Data::OVERLAY) {
					DiscreteLookupTableWrapper wrapper(this->m_ColorLookup);
					wrapper.setRandomColor(minmax.get<0>(), minmax.get<1>(), 0.5);
				} else {
					pcl_ThrowSimpleException(pcl::Exception(), "Error: Invalid type provided!");
				}
				m_ImageModified = false;

				std::cout << "Is identity " << m_Image->getOrientationMatrix().is_identity() << std::endl;
				if (!m_Image->getOrientationMatrix().is_identity()) {
					auto orientation = m_Image->getOrientationMatrix();
					auto origin = m_Image->getOrigin() - pcl::geometry::MatrixPointHelper::MultiplyMatrixWithPoint<pcl::Point3D<double>>(m_Image->getOrientationMatrix(),m_Image->getOrigin());
					m_OrientationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
					m_OrientationMatrix->Identity();
					for(int r=0; r<3; r++) {
						for (int c=0; c<3; c++) {
							m_OrientationMatrix->SetElement(r,c, orientation(r,c));
						}
						m_OrientationMatrix->SetElement(r,3, origin[r]);
					}

					m_InverseOrientationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
					vtkMatrix4x4::Invert(m_OrientationMatrix, m_InverseOrientationMatrix);
				}
			}
		};

	}
}

#endif