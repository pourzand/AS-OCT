#ifndef PCL_NEAREST_NEIGHBOR_INTERPOLATOR
#define PCL_NEAREST_NEIGHBOR_INTERPOLATOR

#include <pcl/macro.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>

namespace pcl
{
	namespace filter
	{

		template <class ImageType, template<class> class BoundaryHandlerClass>
		class NearestNeighborInterpolatorBase
		{
		public:
			typedef BoundaryHandlerClass<ImageType> BoundaryHandlerType;

			NearestNeighborInterpolatorBase() {}
			NearestNeighborInterpolatorBase(const typename ImageType::ConstantPointer& image)
			{
				setImage(image);
			}
			
			typename ImageType::ConstantPointer getImage()
			{
				return m_BoundaryHandler.getImage();
			}

			void setImage(const typename ImageType::ConstantPointer& image)
			{
				m_BoundaryHandler.setImage(image);
			}

			BoundaryHandlerClass<ImageType>& getBoundaryHandler()
			{
				return m_BoundaryHandler;
			}

			template <class PointType>
			double get(const PointType& p) const
			{
				Point3D<int> temp(pcl_Round(p[0]), pcl_Round(p[1]), pcl_Round(p[2]));
				return m_BoundaryHandler.get(temp);
			}

		protected:
			BoundaryHandlerClass<ImageType> m_BoundaryHandler;
		};


		template <class ImageType>
		class NearestNeighborInterpolator: public NearestNeighborInterpolatorBase<ImageType, RepeatingBoundaryHandler>
		{
		public:
			typedef typename NearestNeighborInterpolatorBase<ImageType, RepeatingBoundaryHandler>::BoundaryHandlerType BoundaryHandlerType;

			NearestNeighborInterpolator() {}
			NearestNeighborInterpolator(const typename ImageType::ConstantPointer& image)
			{
				setImage(image);
			}
		};
	}
}

#endif