#ifndef PCL_GUI_IMAGE_SOURCE
#define PCL_GUI_IMAGE_SOURCE

#include <pcl/gui/ImageSourceBase.h>

namespace pcl
{
	namespace gui
	{

		template <class PclImageType>
		class ImageSource: public ImageSourceBase
		{
		public:
			typedef ImageSource Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename pcl::vtk::ItkToVtkHelper::GetConverterType<typename PclImageType::Pointer>::type ImageConverterType;
			typedef PclImageType ImageType;

			static Pointer New(const typename ImageType::Pointer& image, bool* is_alias=NULL)
			{
				Pointer obj(new Self);
				obj->setSource(image, is_alias);
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
				return m_Image->set(p, val);
			}

			virtual vtkImageData* getImageData()
			{
				return m_ImageConverter->GetOutput();
			}

			const typename ImageType::Pointer& getImage()
			{
				return m_Image;
			}

			const typename ImageConverterType::Pointer& getImageConverter()
			{
				return m_ImageConverter;
			}

		protected:
			typename ImageType::Pointer m_Image;
			typename ImageConverterType::Pointer m_ImageConverter;

			void setSource(const typename ImageType::Pointer& image, bool* is_alias=NULL)
			{
				m_Image = image;
				m_ImageConverter = pcl::vtk::ItkToVtkHelper::GetConverter(image, is_alias);
			}
		};

	}
}

#endif
