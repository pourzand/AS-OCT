#ifndef PCL_TPS_INTERPOLATOR
#define PCL_TPS_INTERPOLATOR

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/terrain_map/TerrainMapHelper.h>
#include <pcl/iterator.h>
#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <f2c.h>
#include <clapack.h>

namespace pcl
{
	namespace terrain_map
	{
		using namespace pcl;

		template <class TerrainMapType>
		class TpsInterpolator 
		{
		public:
			TpsInterpolator()
			{}

			template <class PointList>
			void setInput(const Region3D<double>& region, const Point3D<double>& spacing, const PointList& control)
			{
				setInput(region, spacing, control, TerrainMapHelper::DummyNormalAlignmentFunctor());
			}

			template <class PointList, class NormalAlignmentFunctor>
			void setInput(const Region3D<double>& region, const Point3D<double>& spacing, const PointList& control, NormalAlignmentFunctor align_normal)
			{
				if (control.size()<3) pcl_ThrowException(Exception(), "Invalid number (<3) of control points provided!");
				setInput(TerrainMapHelper::Create<TerrainMapType>(region, control, spacing, align_normal), control, true);
			}

			template <class PointList>
			void setInput(const typename TerrainMapType::Pointer& result, const PointList& control, bool control_is_physical=true)
			{
				if (control.size()<3) pcl_ThrowException(Exception(), "Invalid number (<3) of control points provided!");
				m_Result = result;
				m_Control.clear();
				m_Control.reserve(control.size());
				if (!control_is_physical) {
					pcl_ForEach(control, item) m_Control.push_back(*item);
				} else {
					pcl_ForEach(control, item) m_Control.push_back(m_Result->toImageCoordinate(*item));
				}
			}

			void update(double regularization=0)
			{
				//Based on implementation from http://elonen.iki.fi/code/tpsdemo/
				unsigned p = m_Control.size();
				vnl_matrix<double> mtx_l(p+3, p+3),
					mtx_v(p+3, 1),
					mtx_orig_k(p, p);

				// Fill K (p x p, upper left of L) and calculate
				// mean edge length from control points
				//
				// K is symmetrical so we really have to
				// calculate only about half of the coefficients.
				double a = 0.0;
				for ( unsigned i=0; i<p; ++i ) {
					for ( unsigned j=i+1; j<p; ++j ) {
						pcl::Point3D<double> pt_i = m_Control[i],
							pt_j = m_Control[j];
						pt_i.z() = pt_j.z() = 0;
						double elen = pt_i.getEuclideanDistance(pt_j);
						mtx_l(i,j) = mtx_l(j,i) =
							mtx_orig_k(i,j) = mtx_orig_k(j,i) =
							basisFunction(elen);
						a += elen * 2; // same for upper & lower tri
					}
				}
				a /= (double)(p*p);

				// Fill the rest of L
				for ( unsigned i=0; i<p; ++i )
				{
					// diagonal: reqularization parameters (lambda * a^2)
					mtx_l(i,i) = mtx_orig_k(i,i) =
						regularization * (a*a);

					// P (p x 3, upper right)
					mtx_l(i, p+0) = 1.0;
					mtx_l(i, p+1) = m_Control[i].x();
					mtx_l(i, p+2) = m_Control[i].y();

					// P transposed (3 x p, bottom left)
					mtx_l(p+0, i) = 1.0;
					mtx_l(p+1, i) = m_Control[i].x();
					mtx_l(p+2, i) = m_Control[i].y();
				}
				// O (3 x 3, lower right)
				for ( unsigned i=p; i<p+3; ++i )
					for ( unsigned j=p; j<p+3; ++j )
						mtx_l(i,j) = 0.0;

				// Fill the right hand vector V
				for ( unsigned i=0; i<p; ++i )
					mtx_v(i,0) = m_Control[i].z();
				mtx_v(p+0, 0) = mtx_v(p+1, 0) = mtx_v(p+2, 0) = 0.0;

				//Solve the linear system
				//auto tps_weight = vnl_matrix_inverse<double>(mtx_l)*mtx_v;
				auto tps_weight = inverse(mtx_l)*mtx_v;

				// Interpolate grid heights
				pcl::ImageIteratorWithPoint iter(m_Result);
				pcl_ForIterator(iter) {
					double h = tps_weight(p+0, 0) + tps_weight(p+1, 0)*iter.getPoint().x() + tps_weight(p+2, 0)*iter.getPoint().y();
					Point3D<double> pt_i, pt_cur(iter.getPoint());
					pt_cur.z() = 0;
					for ( unsigned i=0; i<p; ++i ) {
						pt_i = m_Control[i];
						pt_i.z() = 0;
						h += tps_weight(i,0) * basisFunction( pt_i.getEuclideanDistance(pt_cur) );
					}
					m_Result->set(iter, h);
				}
			}

			void update(const std::vector<double>& regularization)
			{
				if (m_Control.size()!=regularization.size()) pcl_ThrowException(Exception(), "Regularization list size is invalid!");

				//Based on implementation from http://elonen.iki.fi/code/tpsdemo/
				unsigned p = m_Control.size();
				vnl_matrix<double> mtx_l(p+3, p+3),
					mtx_v(p+3, 1),
					mtx_orig_k(p, p);

				// Fill K (p x p, upper left of L) and calculate
				// mean edge length from control points
				//
				// K is symmetrical so we really have to
				// calculate only about half of the coefficients.
				double a = 0.0;
				for ( unsigned i=0; i<p; ++i ) {
					for ( unsigned j=i+1; j<p; ++j ) {
						pcl::Point3D<double> pt_i = m_Control[i],
							pt_j = m_Control[j];
						pt_i.z() = pt_j.z() = 0;
						double elen = pt_i.getEuclideanDistance(pt_j);
						mtx_l(i,j) = mtx_l(j,i) =
							mtx_orig_k(i,j) = mtx_orig_k(j,i) =
							basisFunction(elen);
						a += elen * 2; // same for upper & lower tri
					}
				}
				a /= (double)(p*p);

				// Fill the rest of L
				for ( unsigned i=0; i<p; ++i )
				{
					// diagonal: reqularization parameters (lambda * a^2)
					mtx_l(i,i) = mtx_orig_k(i,i) =
						regularization[i] * (a*a);

					// P (p x 3, upper right)
					mtx_l(i, p+0) = 1.0;
					mtx_l(i, p+1) = m_Control[i].x();
					mtx_l(i, p+2) = m_Control[i].y();

					// P transposed (3 x p, bottom left)
					mtx_l(p+0, i) = 1.0;
					mtx_l(p+1, i) = m_Control[i].x();
					mtx_l(p+2, i) = m_Control[i].y();
				}
				// O (3 x 3, lower right)
				for ( unsigned i=p; i<p+3; ++i )
					for ( unsigned j=p; j<p+3; ++j )
						mtx_l(i,j) = 0.0;

				// Fill the right hand vector V
				for ( unsigned i=0; i<p; ++i )
					mtx_v(i,0) = m_Control[i].z();
				mtx_v(p+0, 0) = mtx_v(p+1, 0) = mtx_v(p+2, 0) = 0.0;

				//Solve the linear system
				auto tps_weight = vnl_matrix_inverse<double>(mtx_l)*mtx_v;

				// Interpolate grid heights
				pcl::ImageIteratorWithPoint iter(m_Result);
				pcl_ForIterator(iter) {
					double h = tps_weight(p+0, 0) + tps_weight(p+1, 0)*iter.getPoint().x() + tps_weight(p+2, 0)*iter.getPoint().y();
					Point3D<double> pt_i, pt_cur(iter.getPoint());
					pt_cur.z() = 0;
					for ( unsigned i=0; i<p; ++i ) {
						pt_i = m_Control[i];
						pt_i.z() = 0;
						h += tps_weight(i,0) * basisFunction( pt_i.getEuclideanDistance(pt_cur) );
					}
					m_Result->set(iter, h);
				}
			}

			const typename TerrainMapType::Pointer& getOutput()
			{
				return m_Result;
			}

		protected:
			std::vector<Point3D<double>> m_Control;
			typename TerrainMapType::Pointer m_Result;

			double basisFunction(double r)
			{
				if (r==0) return 0;
				else return r*r*log(r);
			}

			vnl_matrix<double>& inverse(vnl_matrix<double>& matrix)
			{
				int N = matrix.rows();
				double *A = new double[N*N];
				matrix.copy_out(A);
				inverse(A, N);
				matrix.copy_in(A);
				delete A;
				return matrix;
			}

			void inverse(double* A, integer N) 
			{
				integer *IPIV = new integer[N+1];
				integer LWORK = N*N;
				double *WORK = new double[LWORK];
				integer INFO;

				dgetrf_(&N,&N,A,&N,IPIV,&INFO);
				dgetri_(&N,A,&N,IPIV,WORK,&LWORK,&INFO);

				delete IPIV;
				delete WORK;
			}

		};

	}
}
#endif