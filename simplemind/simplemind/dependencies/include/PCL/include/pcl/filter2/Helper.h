#ifndef PCL_FILTER2_HELPER
#define PCL_FILTER2_HELPER

#include <pcl/filter2/interpolator/TrilinearInterpolator.h>
#include <pcl/filter2/interpolator/NearestNeighborInterpolator.h>
#include <pcl/filter2/interpolator/BilinearInterpolator.h>
#include <pcl/filter2/interpolator/WindowedSincInterpolator.h>

#include <pcl/filter2/boundary/FixedValueBoundary.h>
#include <pcl/filter2/boundary/MirroringBoundary.h>
#include <pcl/filter2/boundary/ZeroFluxBoundary.h>
#include <pcl/filter2/boundary/AltSourceBoundary.h>

#define pcl_filter2_GetBaseType(T) pcl::base_type<typename T::element_type>::type

namespace pcl
{
	namespace filter2
	{
		class Helper
		{
		public:

			template <class ImagePointerType, class SecondaryBoundary>
			static AltSourceBoundary<typename pcl_filter2_GetBaseType(ImagePointerType),SecondaryBoundary> GetAltSourceBoundary(
				const ImagePointerType& image,
				const SecondaryBoundary& bound
				)
			{
				return AltSourceBoundary<typename pcl_filter2_GetBaseType(ImagePointerType),SecondaryBoundary>(image, bound);
			}
			
			template <class BoundaryType>
			static AltSourceBoundary<typename BoundaryType::ImageType, BoundaryType> GetAutoAltSourceBoundary(
				const BoundaryType& bound,
				const Region3D<int>& region
				)
			{
				return AltSourceBoundary<typename BoundaryType::ImageType, BoundaryType>(bound.getImage()->getSubImage(region.getMinPoint(), region.getMaxPoint()), bound);
			}

			template <class ImagePointerType, class SecondaryBoundary>
			static PointOnlyAltSourceBoundary<typename pcl_filter2_GetBaseType(ImagePointerType),SecondaryBoundary> GetPointOnlyAltSourceBoundary(
				const ImagePointerType& image,
				const SecondaryBoundary& bound
				)
			{
				return PointOnlyAltSourceBoundary<typename pcl_filter2_GetBaseType(ImagePointerType),SecondaryBoundary>(image, bound);
			}
			//TODO PENDING testing
			template <class ImagePointerType>
			static FixedValueBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)> GetFixedValueBoundary(
				const ImagePointerType& image,
				typename pcl_filter2_GetBaseType(ImagePointerType)::IoValueType val=typename pcl_filter2_GetBaseType(ImagePointerType)::IoValueType()
				)
			{
				return FixedValueBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)>(image, val);
			}

			template <class ImagePointerType>
			static MirroringBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)> GetMirroringBoundary(const ImagePointerType& image)
			{
				return MirroringBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)>(image);
			}

			template <class ImagePointerType>
			static ZeroFluxBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)> GetZeroFluxBoundary(const ImagePointerType& image)
			{
				return ZeroFluxBoundary<typename pcl_filter2_GetBaseType(ImagePointerType)>(image);
			}

			template <class BoundaryType>
			static BilinearInterpolator<BoundaryType> GetBilinearInterpolator(const BoundaryType& bound, int axis1, int axis2)
			{
				return BilinearInterpolator<BoundaryType>(bound, axis1, axis2);
			}

			template <class BoundaryType>
			static NearestNeighborInterpolator<BoundaryType> GetNearestNeighborInterpolator(const BoundaryType& bound)
			{
				return NearestNeighborInterpolator<BoundaryType>(bound);
			}

			template <class BoundaryType>
			static TrilinearInterpolator<BoundaryType> GetTrilinearInterpolator(const BoundaryType& bound)
			{
				return TrilinearInterpolator<BoundaryType>(bound);
			}
			
			template <unsigned int VRadius, class BoundaryType>
			static WindowedSincInterpolator<BoundaryType, LanczosWindowFunction<VRadius>> GetLanczosSincInterpolator(const BoundaryType& bound)
			{
				return WindowedSincInterpolator<BoundaryType, LanczosWindowFunction<VRadius>>(bound);
			}

			template <unsigned int Dim, unsigned int VRadius, class BoundaryType>
			static WindowedSincInterpolator1D<BoundaryType, Dim, LanczosWindowFunction<VRadius>> Get1DLanczosSincInterpolator(const BoundaryType& bound)
			{
				return WindowedSincInterpolator1D<BoundaryType, Dim, LanczosWindowFunction<VRadius>>(bound);
			}
		};
	}
}

#endif