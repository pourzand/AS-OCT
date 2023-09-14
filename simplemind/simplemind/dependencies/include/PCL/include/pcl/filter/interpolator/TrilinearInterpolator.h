#ifndef PCL_TRILINEAR_INTERPOLATOR
#define PCL_TRILINEAR_INTERPOLATOR

#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace filter
	{

		template <class ImageType, template<class> class BoundaryHandlerClass>
		class TrilinearInterpolatorBase
		{
		public:
			typedef BoundaryHandlerClass<ImageType> BoundaryHandlerType;

			TrilinearInterpolatorBase() {}
			TrilinearInterpolatorBase(const typename ImageType::ConstantPointer& image)
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
				m_SafeRegion.getMaxPoint() -= 1;
				m_SafeRegion.getMaxPoint().max(m_SafeRegion.getMinPoint());
				m_BoundaryHandler.setImage(image);
				for (int i=0; i<3; i++) {
					m_IndexOffset[i] = m_Image->getOffsetTable()[i];
				}
			}

			BoundaryHandlerClass<ImageType>& getBoundaryHandler()
			{
				return m_BoundaryHandler;
			}

			template <class PointType>
			typename boost::enable_if<boost::is_floating_point<typename PointType::ValueType>, double>::type get(const PointType& p) const
			{
				Point3D<int> p0(floor(p.x()), floor(p.y()), floor(p.z())),
					p1(p0.x()+1, p0.y()+1, p0.z()+1);
				long index = m_Image->localToIndex(p0);
				double xd = p.x()-p0.x(),
					yd = p.y()-p0.y(),
					zd = p.z()-p0.z();
				double i1, i2, j1, j2;
				if (m_SafeRegion.contain(p0)) {
					i1 = m_Image->get(index)*(1-zd) + m_Image->get(index+m_IndexOffset[2])*zd;
					i2 = m_Image->get(index+m_IndexOffset[1])*(1-zd) + m_Image->get(index+m_IndexOffset[1]+m_IndexOffset[2])*zd;
					j1 = m_Image->get(index+m_IndexOffset[0])*(1-zd) + m_Image->get(index+m_IndexOffset[0]+m_IndexOffset[2])*zd;
					j2 = m_Image->get(index+m_IndexOffset[0]+m_IndexOffset[1])*(1-zd) + m_Image->get(index+m_IndexOffset[0]+m_IndexOffset[1]+m_IndexOffset[2])*zd;
				} else {
					Point3D<int> t1(p0), t2(p0);
					t2.z() += 1;
					i1 = m_BoundaryHandler.get(t1,index)*(1-zd) + m_BoundaryHandler.get(t2,index+m_IndexOffset[2])*zd;
					t1.y() += 1;
					t2.y() += 1;
					i2 = m_BoundaryHandler.get(t1,index+m_IndexOffset[1])*(1-zd) + m_BoundaryHandler.get(t2,index+m_IndexOffset[1]+m_IndexOffset[2])*zd;
					t1 = p0; t2 = p0;
					t1.x() += 1; 
					t2.x() += 1; t2.z() += 1;
					j1 = m_BoundaryHandler.get(t1,index+m_IndexOffset[0])*(1-zd) + m_BoundaryHandler.get(t2,index+m_IndexOffset[0]+m_IndexOffset[2])*zd;
					t1.y() += 1;
					t2.y() += 1;
					j2 = m_BoundaryHandler.get(t1,index+m_IndexOffset[0]+m_IndexOffset[1])*(1-zd) + m_BoundaryHandler.get(t2, index+m_IndexOffset[0]+m_IndexOffset[1]+m_IndexOffset[2])*zd;
				}
				double w1 = i1*(1-yd) + i2*yd,
					w2 = j1*(1-yd) + j2*yd;
				return w1*(1-xd) + w2*xd;
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
		};


		template <class ImageType>
		class TrilinearInterpolator: public TrilinearInterpolatorBase<ImageType, RepeatingBoundaryHandler>
		{
		public:
			typedef typename TrilinearInterpolatorBase<ImageType, RepeatingBoundaryHandler>::BoundaryHandlerType BoundaryHandlerType;

			TrilinearInterpolator() {}
			TrilinearInterpolator(const typename ImageType::ConstantPointer& image)
			{
				setImage(image);
			}
		};

	}
}

#endif