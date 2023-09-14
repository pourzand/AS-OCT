#ifndef PCL_POINT_EIGEN_ANALYSIS_FILTER
#define PCL_POINT_EIGEN_ANALYSIS_FILTER

#include <pcl/image.h>
#include <pcl/constant.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <math.h>

namespace pcl
{
	namespace filter
	{
	
		namespace binary_operator
		{

			struct DotProductOperator
			{
				template <class T1, class T2>
				inline double operator()(const T1& a, const T2& b)
				{
					return a.getDotProduct(b);
				}
			};
		
		}

		template <class InputFilterType>
		class PointEigenAnalysisFilter: public PointFilterBase
		{
		public:
			typedef PointEigenAnalysisFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename InputFilterType::Pointer InputPointer;

			enum 
			{
				MIN_VAL = 0,
				MID_VAL = 1,
				MAX_VAL = 2,

				MIN_X = 3,
				MIN_Y = 4,
				MIN_Z = 5,

				MID_X = 6,
				MID_Y = 7,
				MID_Z = 8,

				MAX_X = 9,
				MAX_Y = 10,
				MAX_Z = 11
			};

			enum 
			{
				D_XX = 0,
				D_YY = 1,
				D_ZZ = 2,
				D_XY = 3,
				D_XZ = 4,
				D_YZ = 5
			};

			static Pointer New(const InputPointer& dxx, const InputPointer& dyy, const InputPointer& dzz, const InputPointer& dxy, const InputPointer& dxz, const InputPointer& dyz, bool compute_eigen_vector=true)
			{
				Pointer obj(new Self);
				obj->m_Filter[D_XX] = dxx;
				obj->m_Filter[D_YY] = dyy;
				obj->m_Filter[D_ZZ] = dzz;
				obj->m_Filter[D_XY] = dxy;
				obj->m_Filter[D_XZ] = dxz;
				obj->m_Filter[D_YZ] = dyz;
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
				if (isPreviousIndex(index)) return;
				for (int i=0; i<6; i++) m_Filter[i]->apply(point);
				apply();
			}
			void apply(const Point3D<int>& point, long index) 
			{
				if (isPreviousIndex(index)) return;
				for (int i=0; i<6; i++) m_Filter[i]->apply(point, index);
				apply();
			}
			void apply(long index) 
			{
				if (isPreviousIndex(index)) return;
				for (int i=0; i<6; i++) m_Filter[i]->apply(index);
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

			inline Point3D<double> getEigenVector(int eigen_val_index) const
			{
				int index = 3*(eigen_val_index+1);
				return Point3D<double>(m_Result[index], m_Result[index+1], m_Result[index+2]);
			}

		protected:
			InputPointer m_Filter[6];
			bool m_ComputeEigenVector;
			double m_Result[12];
			double m_M11, m_M22, m_M33, m_M12, m_M13, m_M23;
			bool m_VecIsZero[3];
			
			PointEigenAnalysisFilter() {}

			void apply()
			{
				m_M11 = m_Filter[D_XX]->getResult();
				m_M22 = m_Filter[D_YY]->getResult();
				m_M33 = m_Filter[D_ZZ]->getResult();
				m_M12 = m_Filter[D_XY]->getResult();
				m_M13 = m_Filter[D_XZ]->getResult();
				m_M23 = m_Filter[D_YZ]->getResult();

				computeEigenvalues(m_Result[MIN_VAL], m_Result[MID_VAL], m_Result[MAX_VAL]);
				sortEigenvalues();
				if (m_ComputeEigenVector) {
					computeEigenvector(m_Result[MIN_VAL], MIN_X, m_VecIsZero[MIN_VAL], false);
					computeEigenvector(m_Result[MID_VAL], MID_X, m_VecIsZero[MID_VAL], false);
					if (m_VecIsZero[MIN_VAL] && m_VecIsZero[MID_VAL]) computeEigenvector(m_Result[MAX_VAL], MAX_X, m_VecIsZero[MAX_VAL], true);
					else computeEigenvector(m_Result[MAX_VAL], MAX_X, m_VecIsZero[MAX_VAL], false);

					//Extra computation to solve for zero vectors
					int non_zero_vec[] = {-1,-1,-1};
					int zero_vec[] = {-1,-1,-1};
					int non_zero_count = 0;
					int zero_count = 0;
					for (int i=0; i<3; i++) {
						if (!m_VecIsZero[i]) {
							non_zero_vec[non_zero_count] = i;
							++non_zero_count;
						} else {
							zero_vec[zero_count] = i;
							++zero_count;
						}
					}
					if (zero_count>0) {
						int zero_index;
						if (zero_count==2) {
							Point3D<double> &vec = getEigenVector(non_zero_vec[0]).getPerpendicularVector();
							int index = (zero_vec[0]+1)*3;
							m_Result[index] = vec[0];
							m_Result[index+1] = vec[1];
							m_Result[index+2] = vec[2];
							non_zero_vec[1] = zero_vec[0];
							zero_index = zero_vec[1];
						} else zero_index = zero_vec[0];

						Point3D<double> &vec = getEigenVector(non_zero_vec[0]).setCrossProduct(getEigenVector(non_zero_vec[1]));
						int index = (zero_index+1)*3;
						m_Result[index] = vec[0];
						m_Result[index+1] = vec[1];
						m_Result[index+2] = vec[2];
					}
				}
			}

			void computeEigenvector(double eig_val, int index, bool& vec_is_zero, bool force_non_zero)
			{
				double m11p = m_M11-eig_val,
					m22p = m_M22-eig_val,
					m33p = m_M33-eig_val;
				
				if (m11p==0 && m_M12==0 && m_M13==0) m_Result[index] = 1;
				else m_Result[index] = m_M12*m_M23 - m_M13*m22p;

				if (m22p==0 && m_M12==0 && m_M23==0) m_Result[index+1] = 1;
				else m_Result[index+1] = m_M12*m_M13 - m_M23*m11p;

				if (m33p==0 && m_M23==0 && m_M13==0) m_Result[index+2] = 1;
				else m_Result[index+2] = m11p*m22p - pcl_Square(m_M12);
				
				double norm = 0;
				for (int i=0; i<3; i++) norm += pcl_Square(m_Result[index+i]);
				if (norm <= Epsilon && !force_non_zero) {
					vec_is_zero = true;
				} else {
					vec_is_zero = false;
					norm = sqrt(norm);
					for (int i=0; i<3; i++) m_Result[index+i] /= norm;
				}
			}

			void sortEigenvalues()
			{
				const int num = 3;
				bool is_modified = true;
				while (is_modified) {
					is_modified = false;
					for (int i=0; i<num-1; i++) {
						double a = pcl_Abs(m_Result[i]),
							b = pcl_Abs(m_Result[i+1]);
						if (a > b) {
							pcl_Swap(m_Result[i], m_Result[i+1]);
							is_modified = true;
						} else if (a == b) {
							if (m_Result[i] > m_Result[i+1]) {
								pcl_Swap(m_Result[i], m_Result[i+1]);
								is_modified = true;
							}
						}
					}
				}
			}

			void computeEigenvalues(double &l1, double &l2, double &l3) //Adapted from vnl library
			{
				// Characteristic eqtn |M - xI| = 0
				// x^3 + b x^2 + c x + d = 0
				const double b = -m_M11-m_M22-m_M33;
				const double c = m_M11*m_M22 +m_M11*m_M33 +m_M22*m_M33 -m_M12*m_M12 -m_M13*m_M13 -m_M23*m_M23;
				const double d = m_M11*m_M23*m_M23 +m_M12*m_M12*m_M33 +m_M13*m_M13*m_M22 -2.0*m_M12*m_M13*m_M23 -m_M11*m_M22*m_M33;

				// Using a numerically tweaked version of the real cubic solver http://www.1728.com/cubic2.htm
				const double b_3 = b/3.0;
				const double f = b_3*b_3 -  c/3.0 ;
				const double g = b*c/6.0 - b_3*b_3*b_3 - 0.5*d;

				if (f == 0.0 && g == 0.0) {
					l1 = l2 = l3 = - b_3 ;
					return;
				}

				const double f3 = f*f*f;
				const double g2 = g*g;
				const double sqrt_f = -sqrt(f);

				// deal explicitly with repeated root and treat
				// complex conjugate roots as numerically inaccurate repeated roots.

				// first check we are not too numerically innacurate
				assert((g2 - f3) / sqrt(pcl_Cube(b)) < 1e-8);  

				if (g2 >= f3) {
					if (g < 0.0) {
						l1 = 2.0 * sqrt_f  - b_3;
						l2 = l3 = - sqrt_f - b_3;
					} else {
						l1 = l2 = sqrt_f  - b_3;
						l3 = -2.0 * sqrt_f - b_3;
					}
					return;
				}

				const double sqrt_f3 = sqrt_f * sqrt_f * sqrt_f;
				const double k = acos(g / sqrt_f3) / 3.0;
				const double j = 2.0 * sqrt_f;
				l1 = j * cos(k) - b_3;
				l2 = j * cos(k + pcl::PI * 2.0 / 3.0) - b_3;
				l3 = j * cos(k - pcl::PI * 2.0 / 3.0) - b_3;

				if (l2 < l1) pcl_Swap(l2, l1);
				if (l3 < l2) {
					pcl_Swap(l2, l3);
					if (l2 < l1) pcl_Swap(l2, l1);
				}
			}
		};

	}
}

#endif