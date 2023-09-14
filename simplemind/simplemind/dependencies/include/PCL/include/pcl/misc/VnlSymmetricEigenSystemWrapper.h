#ifndef PCL_VNL_SYMMETRIC_EIGEN_SYSTEM_WRAPPER
#define PCL_VNL_SYMMETRIC_EIGEN_SYSTEM_WRAPPER

#ifndef NO_VNL

#include <vnl/algo/vnl_symmetric_eigensystem.h>
#include <pcl/exception.h>
#include <boost/smart_ptr.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

namespace pcl
{
	namespace misc
	{

		template <class Type>
		class VnlSymmetricEigenSystemWrapper
		{
		public:
			VnlSymmetricEigenSystemWrapper(const vnl_matrix<Type>& mat)
			{
				int n = mat.rows();
				for (int r=0; r<n; ++r) for (int c=0; c<n; ++c) if (!boost::math::isfinite<Type>(mat(r,c))) {
					pcl_ThrowException(pcl::InvalidValueException(), "Matrix contains none finite value(s)");
				}

				m_Pointer.reset(new vnl_symmetric_eigensystem<Type>(mat));
			}

			vnl_symmetric_eigensystem<Type>* operator->() const
			{
				return m_Pointer.get();
			}

		protected:
			boost::scoped_ptr<vnl_symmetric_eigensystem<Type>> m_Pointer;
		};

	}
}

#endif

#endif