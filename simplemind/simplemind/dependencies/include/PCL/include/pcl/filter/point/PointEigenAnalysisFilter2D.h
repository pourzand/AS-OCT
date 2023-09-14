#ifndef PCL_POINT_EIGEN_ANALYSIS_FILTER_2D
#define PCL_POINT_EIGEN_ANALYSIS_FILTER_2D

#include <pcl/image.h>
#include <pcl/constant.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <math.h>

namespace pcl
{
	namespace filter
	{
	
		template <class InputFilterType>
		class PointEigenAnalysisFilter2D: public PointFilterBase
		{
		public:
			typedef PointEigenAnalysisFilter2D Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename InputFilterType::Pointer InputPointer;

			enum 
			{
				MIN_VAL = 0,
				MAX_VAL = 1,

				MIN_X = 2,
				MIN_Y = 3,

				MAX_X = 4,
				MAX_Y = 5,
			};

			enum 
			{
				D_XX = 0,
				D_YY = 1,
				D_XY = 2
			};

			static Pointer New(const InputPointer& dxx, const InputPointer& dyy, const InputPointer& dxy, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[D_XX] = dxx;
				obj->m_Filter[D_YY] = dyy;
				obj->m_Filter[D_XY] = dxy;
				obj->m_ComputeEigenVector = compute_eigen_vector;
				return obj;
			}

			template <class IteratorType>
			void apply(const IteratorType& iter)
			{
				apply(iter.getPoint(), iter.getIndex());
			}

			void apply(const Point3D<int>& point) 
			{
				//TODO pending testing
				//if (isPreviousIndex(index)) return;
				for (int i=0; i<3; i++) m_Filter[i]->apply(point);
				apply();
			}
			void apply(const Point3D<int>& point, long index) 
			{
				if (isPreviousIndex(index)) return;
				for (int i=0; i<3; i++) m_Filter[i]->apply(point, index);
				apply();
			}
			void apply(long index) 
			{
				if (isPreviousIndex(index)) return;
				for (int i=0; i<3; i++) m_Filter[i]->apply(index);
				apply();
			}

			inline double getResult(int i=0) const
			{
				return m_Result[i];
			}

			inline InputPointer getFilter(int i)
			{
				return m_Filter[i];
			}

			inline Point2D<double> getEigenVector(int eigen_val_index) const
			{
				int index = 2*(eigen_val_index+1);
				return Point2D<double>(m_Result[index], m_Result[index+1]);
			}

		protected:
			InputPointer m_Filter[3];
			bool m_ComputeEigenVector;
			double m_Result[6];
			double m_M11, m_M22, m_M12;
			bool m_VecIsZero[2];
			
			PointEigenAnalysisFilter2D() {}

			void apply()
			{
				m_M11 = m_Filter[D_XX]->getResult();
				m_M22 = m_Filter[D_YY]->getResult();
				m_M12 = m_Filter[D_XY]->getResult();

				//According to description given in http://web.mit.edu/18.06/www/Course-Info/Mfiles/eigen2.m
				computeEigenvalues(m_Result[MIN_VAL], m_Result[MAX_VAL]);
				
				if (pcl::abs(m_M12)>Epsilon) {
					m_Result[MIN_X] = m_M12; m_Result[MIN_Y] = m_Result[MIN_VAL]-m_M11;
					m_Result[MAX_X] = m_M12; m_Result[MAX_Y] = m_Result[MAX_VAL]-m_M11;
				} else {
					m_Result[MIN_X] = 1; m_Result[MIN_Y] = 0;
					m_Result[MAX_X] = 0; m_Result[MAX_Y] = 1;
				}
			}

			void computeEigenvalues(double &l1, double &l2)
			{
				// Characteristic eqtn |M - xI| = 0
				// x^2 + a x + b = 0
				const double a = m_M11+m_M22;
				const double b = m_M11*m_M22 - m_M12*m_M12;

				const double s = sqrt(a*a - 4*b);
				l1 = (-a + s)/2;
				l2 = (-a - s)/2;

				if (pcl::abs(l1)>pcl::abs(l2)) {
					double temp = l2;
					l2 = l1;
					l1 = temp;
				}
			}
		};

	}
}

#endif