#ifndef PCL_POINT_HESSIAN_FILTER
#define PCL_POINT_HESSIAN_FILTER

#include <pcl/filter/point/PointEigenAnalysisFilter.h>
#include <pcl/filter/point/PointFiniteDifferenceFilter.h>

namespace pcl
{
	namespace filter
	{

		template <class InputImageType, class DifferenceFilterType=PointFiniteDifferenceFilter<InputImageType> >
		class PointHessianFilter: public PointEigenAnalysisFilter<DifferenceFilterType>
		{
		public:
			typedef PointHessianFilter Self;
			typedef PointEigenAnalysisFilter<DifferenceFilterType> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename DifferenceFilterType::Pointer InputPointer;


			static Pointer New(const typename InputImageType::ConstantPointer& image, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = DifferenceFilterType::New(image, 2,0,0);
				obj->m_Filter[Self::D_YY] = DifferenceFilterType::New(image, 0,2,0);
				obj->m_Filter[Self::D_ZZ] = DifferenceFilterType::New(image, 0,0,2);
				obj->m_Filter[Self::D_XY] = DifferenceFilterType::New(image, 1,1,0);
				obj->m_Filter[Self::D_XZ] = DifferenceFilterType::New(image, 1,0,1);
				obj->m_Filter[Self::D_YZ] = DifferenceFilterType::New(image, 0,1,1);
				obj->m_Image = image;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			static Pointer New(const typename InputImageType::ConstantPointer& image, const InputPointer& dxx, const InputPointer& dyy, const InputPointer& dzz, const InputPointer& dxy, const InputPointer& dxz, const InputPointer& dyz, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[Self::D_XX] = dxx;
				obj->m_Filter[Self::D_YY] = dyy;
				obj->m_Filter[Self::D_ZZ] = dzz;
				obj->m_Filter[Self::D_XY] = dxy;
				obj->m_Filter[Self::D_XZ] = dxz;
				obj->m_Filter[Self::D_YZ] = dyz;
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

			PointHessianFilter() {}
		};

	}
}

#endif