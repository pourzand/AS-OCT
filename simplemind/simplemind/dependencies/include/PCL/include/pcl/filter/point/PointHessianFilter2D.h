#ifndef PCL_POINT_HESSIAN_FILTER_2D
#define PCL_POINT_HESSIAN_FILTER_2D

#include <pcl/filter/point/PointEigenAnalysisFilter2D.h>
#include <pcl/filter/point/PointFiniteDifferenceFilter.h>

namespace pcl
{
	namespace filter
	{

		template <class InputImageType, class DifferenceFilterType=PointFiniteDifferenceFilter<InputImageType> >
		class PointHessianFilter2D: public PointEigenAnalysisFilter2D<DifferenceFilterType>
		{
		public:
			typedef PointHessianFilter2D Self;
			typedef PointEigenAnalysisFilter2D<DifferenceFilterType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename DifferenceFilterType::Pointer InputPointer;


			static Pointer New(const typename InputImageType::ConstantPointer& image, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = DifferenceFilterType::New(image, 2,0,0);
				obj->m_Filter[Self::D_YY] = DifferenceFilterType::New(image, 0,2,0);
				obj->m_Filter[Self::D_XY] = DifferenceFilterType::New(image, 1,1,0);
				obj->m_Image = image;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			static Pointer New(const typename InputImageType::ConstantPointer& image, const InputPointer& dxx, const InputPointer& dyy, const InputPointer& dxy, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = dxx;
				obj->m_Filter[Self::D_YY] = dyy;
				obj->m_Filter[Self::D_XY] = dxy;
				obj->m_Image = image;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			using Parent::apply;
			void apply(const Point3D<int>& point) 
			{
				this->apply(point, m_Image->localToIndex(point));
			}

		protected:
			typename InputImageType::ConstantPointer m_Image;

			PointHessianFilter2D() {}
		};

	}
}

#endif