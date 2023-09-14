#ifndef PCL_COLOR_IMAGE_GENERATOR
#define PCL_COLOR_IMAGE_GENERATOR

#include <pcl/image/ItkHeader.h>
#include <pcl/image.h>
#include <pcl/filter.h>
#include <pcl/geometry/Cube.h>
#include <itkImageFileWriter.h>
#include <itkImage.h>
#include <itkRGBPixel.h>
#include <itkImageRegionIteratorWithIndex.h>
#ifndef NO_VTK
#include <vtkSmartPointer.h>
#include <vtkLookupTable.h>
#endif

namespace pcl
{
	namespace misc
	{
		typedef pcl::Point3D<unsigned char> RGBColor;

#ifndef NO_VTK
		class LookupTable
		{
		public:
			enum class Ramp 
			{
				Linear=0,
				SCurve=1,
				Sqrt=2
			};
			enum class Scale
			{
				Linear=0,
				Log10=1
			};

			LookupTable()
			{
				m_Lookup = vtkSmartPointer<vtkLookupTable>::New();
			}
			
			vtkLookupTable* getVtkLookupTable()
			{
				return m_Lookup;
			}

			void resize(int size=256)
			{
				m_Lookup->SetNumberOfTableValues(size);
			}
			
			void setIndexedLookup(bool en)
			{
				if (en) m_Lookup->IndexedLookupOn();
				else m_Lookup->IndexedLookupOff();
			}
						
			void setTableRange(double min, double max)
			{
				m_Lookup->SetTableRange(min, max);
			}
			double* getTableRange()
			{
				return m_Lookup->GetTableRange();
			}

			void setValueRange(double min, double max)
			{
				m_Lookup->SetValueRange(min, max);
			}
			double* getValueRange()
			{
				return m_Lookup->GetValueRange();
			}

			void setHueRange(double min, double max)
			{
				m_Lookup->SetHueRange(min, max);
			}
			double* getHueRange()
			{
				return m_Lookup->GetHueRange();
			}

			void setSaturationRange(double min, double max)
			{
				m_Lookup->SetSaturationRange(min, max);
			}
			double* getSaturationRange()
			{
				return m_Lookup->GetSaturationRange();
			}

			void setAlphaRange(double min, double max)
			{
				m_Lookup->SetAlphaRange(min, max);
			}
			double* getAlphaRange()
			{
				return m_Lookup->GetAlphaRange();
			}
			
			void setBelowRangeColor(bool en, double r, double g, double b, double a)
			{
				if (en) {
					m_Lookup->UseBelowRangeColorOn();
					m_Lookup->SetBelowRangeColor(r, g, b, a);
				} else {
					m_Lookup->UseBelowRangeColorOff();
				}
			}
			void setBelowRangeColor(bool en)
			{
				m_Lookup->SetUseBelowRangeColor(en);
			}
			double* getBelowRangeColor()
			{
				if (m_Lookup->GetUseBelowRangeColor() == 0) return NULL;
				return m_Lookup->GetBelowRangeColor();
			}
			
			void setAboveRangeColor(bool en, double r, double g, double b, double a)
			{
				if (en) {
					m_Lookup->UseAboveRangeColorOn();
					m_Lookup->SetAboveRangeColor(r, g, b, a);
				} else {
					m_Lookup->UseAboveRangeColorOff();
				}
			}
			void setAboveRangeColor(bool en)
			{
				m_Lookup->SetUseAboveRangeColor(en);
			}
			double* getAboveRangeColor()
			{
				if (m_Lookup->GetUseAboveRangeColor() == 0) return NULL;
				return m_Lookup->GetAboveRangeColor();
			}
			
			void setNanColor(double r, double g, double b, double a)
			{
				m_Lookup->SetNanColor(r,g,b,a);
			}
			double* getNanColor()
			{
				return m_Lookup->GetNanColor();
			}
			
			void setScale(int i)
			{
				m_Lookup->SetScale(i);
			}
			int getScale()
			{
				return m_Lookup->GetScale();
			}

			void setRamp(int i)
			{
				m_Lookup->SetRamp(i);
			}
			int getRamp()
			{
				return m_Lookup->GetRamp();
			}

			void set(size_t i, double r, double g, double b, double a=1.0)
			{
				m_Lookup->SetTableValue(i, r, g, b, a);
			}
			
			double* get(size_t i, double rgba[4])
			{
				m_Lookup->GetTableValue(i, rgba);
				return rgba;
			}
			
			const unsigned char* mapValue(double x)
			{
				return m_Lookup->MapValue(x);
			}
			
			double getAlpha(double x)
			{
				return m_Lookup->GetOpacity(x);
			}
			
			double* getColor(double x, double rgb[3]) 
			{
				m_Lookup->GetColor(x, rgb);
				return rgb;
			}
			
			double* getRGBA(double x, double rgba[4])
			{
				m_Lookup->GetColor(x, rgba);
				rgba[3] = m_Lookup->GetOpacity(x);
				return rgba;
			}

			void build()
			{
				m_Lookup->Build();
			}

			size_t size() const
			{
				return m_Lookup->GetNumberOfTableValues();
			}

		protected:
			vtkSmartPointer<vtkLookupTable> m_Lookup;
		};
#endif

		class ColorImageGenerator
		{
		public:
			typedef itk::RGBPixel<unsigned char> RGBPixelType;
			typedef itk::Image<RGBPixelType,2> RGBImageType;
			
			template <class ImagePointer>
			ColorImageGenerator(const pcl::Point3D<double>& point, ImagePointer image, const pcl::Region3D<double>& region, double spacing_mm, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis, double tol=-1)
			{
				if (tol < 0) {
					tol = image->getSpacing()[0];
					for (int i = 1; i < 3; ++i) tol = std::min(tol, image->getSpacing()[i]);
					tol /= 100;
				}
				pcl::geometry::Cube<double> cube(region, [&](const pcl::Point3D<double>& p)->pcl::Point3D<double>{
					return image->toPhysicalCoordinate(p);
				}, tol);
				auto info = computeOriginAndSize(cube, point, x_axis, y_axis);
				set(info.get<0>(), info.get<1>(), info.get<2>(), spacing_mm, info.get<3>(), info.get<4>());
			}

			ColorImageGenerator(const pcl::Point3D<double>& point, const pcl::Region3D<double>& region, double spacing_mm, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis)
			{
				pcl::geometry::Cube<double> cube(region);
				auto info = computeOriginAndSize(cube, point, x_axis, y_axis);
				set(info.get<0>(), info.get<1>(), info.get<2>(), spacing_mm, info.get<3>(), info.get<4>());
			}
			
			ColorImageGenerator(const pcl::Point3D<double>& origin, double width_mm, double height_mm, double spacing_mm, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis)
			{
				set(origin, width_mm, height_mm, spacing_mm, x_axis, y_axis);
			}

			template <class InterpolatorType>
			void setImage(InterpolatorType& interpolator, double min_val, double max_val)
			{
				RGBPixelType pixel_val;
				double diff = max_val - min_val;
				itk::ImageRegionIteratorWithIndex<RGBImageType> iter(m_Image, m_Image->GetLargestPossibleRegion());
				for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter) {
					auto index = iter.GetIndex();
					pcl::Point3D<double> pos = m_Origin;
					pos += (m_XAxis * (index[0]*m_Spacing));
					pos += (m_YAxis * (index[1]*m_Spacing));
					pos = interpolator.getImage()->toImageCoordinate(pos);
					double val = interpolator.get(pos);
					val = (val-min_val)/diff;

					unsigned char real_val;
					if (val>1) real_val = 255;
					else if (val<0) real_val = 0;
					else real_val = static_cast<unsigned char>(pcl::round(val*255));
					pixel_val[0] = real_val;
					pixel_val[1] = real_val;
					pixel_val[2] = real_val;
					iter.Set(pixel_val);
				}
			}
			
			template <class InterpolatorType>
			void addOverlay(InterpolatorType& interpolator, const RGBColor& color, double alpha)
			{
				RGBPixelType pixel_val;
				itk::ImageRegionIteratorWithIndex<RGBImageType> iter(m_Image, m_Image->GetLargestPossibleRegion());
				for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter) {
					auto index = iter.GetIndex();
					pcl::Point3D<double> pos = m_Origin;
					pos += (m_XAxis * (index[0]*m_Spacing));
					pos += (m_YAxis * (index[1]*m_Spacing));
					pos = interpolator.getImage()->toImageCoordinate(pos);
					double val = interpolator.get(pos);

					double real_alpha = alpha;
					if (val>0) {
						if (val<1) real_alpha = alpha*val;
						auto img_val = iter.Get();
						for (int i=0; i<3; ++i) {
							pixel_val[i] = static_cast<unsigned char>(std::min(255., img_val[i]*(1-real_alpha) + color[i]*real_alpha));
						}
						iter.Set(pixel_val);
					}
				}
			}
			
#ifndef NO_VTK
			template <class InterpolatorType>
			void setImage(InterpolatorType& interpolator, LookupTable& lookup)
			{
				RGBPixelType pixel_val;
				double rgb[3];
				itk::ImageRegionIteratorWithIndex<RGBImageType> iter(m_Image, m_Image->GetLargestPossibleRegion());
				for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter) {
					auto index = iter.GetIndex();
					pcl::Point3D<double> pos = m_Origin;
					pos += (m_XAxis * (index[0]*m_Spacing));
					pos += (m_YAxis * (index[1]*m_Spacing));
					pos = interpolator.getImage()->toImageCoordinate(pos);
					double val = interpolator.get(pos);
					lookup.getColor(val, rgb);

					//std::cout << val << std::endl;
					for (int i = 0; i < 3; ++i) {
						pixel_val[i] = static_cast<unsigned char>(rgb[i] * 255);
						//std::cout << rgb[i] << " " << pixel_val[i] << std::endl;
					}

					iter.Set(pixel_val);
				}
			}
			
			template <class InterpolatorType>
			void addImage(InterpolatorType& interpolator, LookupTable& lookup)
			{
				RGBPixelType pixel_val;
				double rgba[4];
				itk::ImageRegionIteratorWithIndex<RGBImageType> iter(m_Image, m_Image->GetLargestPossibleRegion());
				for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter) {
					auto index = iter.GetIndex();
					pcl::Point3D<double> pos = m_Origin;
					pos += (m_XAxis * (index[0]*m_Spacing));
					pos += (m_YAxis * (index[1]*m_Spacing));
					pos = interpolator.getImage()->toImageCoordinate(pos);
					double val = interpolator.get(pos);
					lookup.getRGBA(val, rgba);
					
					auto img_val = iter.Get();
					for (int i=0; i<3; ++i) {
						pixel_val[i] = static_cast<unsigned char>(std::min(255., img_val[i]*(1-rgba[3]) + rgba[i]*rgba[3]));
					}
					iter.Set(pixel_val);
				}
			}
#endif

			RGBImageType::Pointer getImage()
			{
				return m_Image;
			}

			void save(const std::string& filename)
			{
				RegisterImageIoFactory();
				auto writer = itk::ImageFileWriter<RGBImageType>::New();
				writer->SetInput(m_Image);
				writer->SetFileName(filename);
				writer->Update();
			}

		protected:
			RGBImageType::Pointer m_Image;
			pcl::Point3D<double> m_Origin; 
			double m_Spacing;
			pcl::Point3D<double> m_XAxis, m_YAxis;
			
			void set(const pcl::Point3D<double>& origin, double width_mm, double height_mm, double spacing_mm, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis)
			{
				RGBImageType::SizeType region_size;
				region_size[0] = std::ceil(width_mm/spacing_mm);
				region_size[1] = std::ceil(height_mm/spacing_mm);

				RGBImageType::IndexType region_index;
				region_index[0] = 0;
				region_index[1] = 0;

				RGBImageType::RegionType itk_region;
				itk_region.SetSize(region_size);
				itk_region.SetIndex(region_index);

				m_Image = RGBImageType::New();
				m_Image->SetRegions(itk_region);
				m_Image->Allocate();

				m_Origin = origin;
				m_Spacing = spacing_mm;
				m_XAxis = x_axis;
				m_YAxis = y_axis;
			}

			boost::tuple<pcl::Point3D<double>, double, double, pcl::Point3D<double>, pcl::Point3D<double>> computeOriginAndSize(const pcl::geometry::Cube<double>& cube, const pcl::Point3D<double>& point, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis)
			{
				auto z_axis = x_axis.getCrossProduct(y_axis);
				pcl::geometry::Plane3D<double> plane(point, z_axis, cube.getTolerance());
				auto point_list = cube.getIntersection(plane);
				double min_x = std::numeric_limits<double>::infinity(),
					max_x = -std::numeric_limits<double>::infinity(),
					min_y = std::numeric_limits<double>::infinity(),
					max_y = -std::numeric_limits<double>::infinity(),
					min_z = std::numeric_limits<double>::infinity();
				pcl_ForEach(point_list, item) {
					max_x = std::max(max_x, x_axis.getDotProduct(*item));
					max_y = std::max(max_y, y_axis.getDotProduct(*item));
					min_x = std::min(min_x, x_axis.getDotProduct(*item));
					min_y = std::min(min_y, y_axis.getDotProduct(*item));
					min_z = std::min(min_z, z_axis.getDotProduct(*item));
				}
				pcl::Point3D<double> origin = (x_axis*min_x) + (y_axis*min_y) + (z_axis*min_z);
				double width_mm = max_x-min_x,
					height_mm = max_y-min_y;
				return boost::tuple<pcl::Point3D<double>, double, double, pcl::Point3D<double>, pcl::Point3D<double>>(origin, width_mm, height_mm, x_axis, y_axis);
			}
		};

	}
}

#endif