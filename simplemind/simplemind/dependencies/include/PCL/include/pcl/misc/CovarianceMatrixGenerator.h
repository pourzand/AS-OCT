#ifndef PCL_COVARIANCE_MATRIX_GENERATOR
#define PCL_COVARIANCE_MATRIX_GENERATOR

#include <vnl/vnl_matrix_fixed.h>

namespace pcl
{
	namespace misc
	{

		template <int Dimension>
		class CovarianceMatrixGenerator
		{
		public:
			CovarianceMatrixGenerator()
			{
				reset();
			}

			void reset()
			{
				for (int r=0; r<Dimension; ++r) {
					m_Mean[r] = 0;
					for (int c=0; c<=r; ++c) {
						m_Var[r][c] = 0;
					}
				}
				m_Weight = 0;
				m_WeightSquare = 0;
				m_Count = 0;
			}

			template <class Type>
			void add(const Type *val)
			{
				for (int r=0; r<Dimension; ++r) {
					m_Mean[r] += val[r];
					for (int c=0; c<=r; ++c) {
						m_Var[r][c] += val[r]*val[c];
					}
				}
				m_Weight++;
				m_WeightSquare++;
				m_Count++;
			}

			template <class Type>
			void add(const Type *val, double weight)
			{
				for (int r=0; r<Dimension; ++r) {
					m_Mean[r] += val[r]*weight;
					for (int c=0; c<=r; ++c) {
						m_Var[r][c] += val[r]*val[c]*weight;
					}
				}
				m_Weight += weight;
				m_WeightSquare += (weight*weight);
				m_Count++;
			}

			template<class Type>
			vnl_matrix_fixed<Type,Dimension,Dimension> getCovarianceMatrix() const
			{
				double norm = 1./(1. - m_WeightSquare/(m_Weight*m_Weight));
				vnl_matrix_fixed<Type,Dimension,Dimension> result;
				for (int r=0; r<Dimension; ++r) {
					for (int c=0; c<=r; ++c) {
						result(r,c) = m_Var[r][c]/m_Weight - (m_Mean[r]/m_Weight)*(m_Mean[c]/m_Weight);
						if (r!=c) result(c,r) = result(r,c);
					}
				}
				return result;
			}

			template<class Type>
			Type getCentroid() const
			{
				Type centroid;
				for (int i=0; i<3; i++) {
					centroid[i] = m_Mean[i]/m_Weight;
				}
				return centroid;
			}

			int getCount() const
			{
				return m_Count;
			}

		protected:
			double m_Mean[Dimension];
			double m_Var[Dimension][Dimension];
			double m_Weight, m_WeightSquare;
			int m_Count;
		};

	}
}

#endif