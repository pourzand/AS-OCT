#ifndef PCL_BILINEAR_INTERPOLATOR
#define PCL_BILINEAR_INTERPOLATOR

#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/math.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace filter
	{

		template <class ImageType, int Axis1, int Axis2, template<class> class BoundaryHandlerClass>
		class BilinearInterpolatorBase
		{
		public:
			typedef BoundaryHandlerClass<ImageType> BoundaryHandlerType;

			BilinearInterpolatorBase() {}
			BilinearInterpolatorBase(const typename ImageType::ConstantPointer& image)
			{
				setImage(image);
			}
			
			typename ImageType::ConstantPointer getImage()
			{
				return m_Image;
			}

			void setImage(const typename ImageType::ConstantPointer& image)
			{
				m_Image = image;
				m_SafeRegion = m_Image->getRegion();
				m_SafeRegion.getMaxPoint()[Axis1] -= 1;
				m_SafeRegion.getMaxPoint()[Axis2] -= 1;
				m_SafeRegion.getMaxPoint().max(m_SafeRegion.getMinPoint());
				m_BoundaryHandler.setImage(image);
				for (int i=0; i<3; i++) {
					m_IndexOffset[i] = m_Image->getOffsetTable()[i];
					if (i!=Axis1 && i!=Axis2) m_SkipAxis = i;
				}
			}

			BoundaryHandlerClass<ImageType>& getBoundaryHandler()
			{
				return m_BoundaryHandler;
			}

			template <class PointType>
			typename boost::enable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				//Based on http://en.wikipedia.org/wiki/Bilinear_interpolation
				Point3D<int> p0, p1;
				p0[m_SkipAxis] = pcl_Round(p[m_SkipAxis]);
				p0[Axis1] = floor(p[Axis1]);
				p0[Axis2] = floor(p[Axis2]);
				p1[m_SkipAxis] = pcl_Round(p[m_SkipAxis]);
				p1[Axis1] = p0[Axis1]+1;
				p1[Axis2] = p0[Axis2]+1;
				
				long index = m_Image->localToIndex(p0);
				double f00, f10, f01, f11;
				if (m_SafeRegion.contain(p0)) {
					f00 = m_Image->get(index);
					f10 = m_Image->get(index+m_IndexOffset[Axis1]);
					f01 = m_Image->get(index+m_IndexOffset[Axis2]);
					f11 = m_Image->get(index+m_IndexOffset[Axis1]+m_IndexOffset[Axis2]);
				} else {
					Point3D<int> t;
					t = p0;
					f00 = m_BoundaryHandler.get(t, index);
					t[Axis1] += 1;
					f10 = m_BoundaryHandler.get(t, index+m_IndexOffset[Axis1]);
					t[Axis2] += 1;
					f11 = m_BoundaryHandler.get(t, index+m_IndexOffset[Axis1]+m_IndexOffset[Axis2]);
					t = p0;
					t[Axis2] += 1;
					f01 = m_BoundaryHandler.get(t, index+m_IndexOffset[Axis2]);
				}
				double x = p[Axis1] - p0[Axis1],
					y = p[Axis2] - p0[Axis2];
				return f00 + (f10-f00)*x + (f01-f00)*y  + (f00-f10-f01+f11)*x*y;
			}
			
			template <class PointType>
			typename boost::disable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> temp; temp.assign(p);
				return m_BoundaryHandler.get(p);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;
			Region3D<int> m_SafeRegion;
			BoundaryHandlerClass<ImageType> m_BoundaryHandler;
			long m_IndexOffset[3];
			int m_SkipAxis;
		};


		template <class ImageType, int Axis1, int Axis2>
		class BilinearInterpolator: public BilinearInterpolatorBase<ImageType, Axis1, Axis2, RepeatingBoundaryHandler>
		{
		public:
			typedef typename BilinearInterpolatorBase<ImageType, Axis1, Axis2, RepeatingBoundaryHandler>::BoundaryHandlerType BoundaryHandlerType;

			BilinearInterpolator() {}
			BilinearInterpolator(const typename ImageType::ConstantPointer& image)
			{
				setImage(image);
			}
		};

	}
}

#endif