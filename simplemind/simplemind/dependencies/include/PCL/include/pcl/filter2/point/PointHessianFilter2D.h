#ifndef PCL_POINT_HESSIAN_FILTER_2D
#define PCL_POINT_HESSIAN_FILTER_2D

#include <pcl/filter/point/PointEigenAnalysisFilter2D.h>
#include <pcl/filter/point/PointFiniteDifferenceFilter.h>

namespace pcl
{
	namespace filter
	{

		template <class BoundaryType, class DifferenceFilterType=PointFiniteDifferenceFilter<BoundaryType> >
		class PointHessianFilter2D: public PointEigenAnalysisFilter2D<DifferenceFilterType>
		{
		public:
			typedef PointHessianFilter2D Self;
			typedef PointEigenAnalysisFilter2D<DifferenceFilterType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename DifferenceFilterType::Pointer InputPointer;
			typedef BoundaryType BoundaryHandlerType;


			static Pointer New(const BoundaryHandlerType& bound, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = DifferenceFilterType::New(bound, 2,0,0);
				obj->m_Filter[Self::D_YY] = DifferenceFilterType::New(bound, 0,2,0);
				obj->m_Filter[Self::D_XY] = DifferenceFilterType::New(bound, 1,1,0);
				obj->m_BoundaryHandler = bound;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			static Pointer New(const typename BoundaryHandlerType& bound, const InputPointer& dxx, const InputPointer& dyy, const InputPointer& dxy, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = dxx;
				obj->m_Filter[Self::D_YY] = dyy;
				obj->m_Filter[Self::D_XY] = dxy;
				obj->m_BoundaryHandler = bound;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			using Parent::apply;
			void apply(const Point3D<int>& point) 
			{
				this->apply(point, m_BoundaryHandler.getImage()->localToIndex(point));
			}

		protected:
			BoundaryHandlerType m_BoundaryHandler;

			PointHessianFilter2D() {}
		};

	}
}

#endif