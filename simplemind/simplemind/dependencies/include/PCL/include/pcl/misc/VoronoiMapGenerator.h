#ifndef PCL_VORONOI_MAP_GENERATOR
#define PCL_VORONOI_MAP_GENERATOR

#include <pcl/image.h>
#include <pcl/math.h>
#include <pcl/image/ImageAlgorithm.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/numeric/conversion/bounds.hpp>

namespace pcl
{
	namespace misc
	{

		template <class OutputImageType>
		class VoronoiMapGenerator
		{
		public:
			typedef typename OutputImageType::IoValueType OutputValueType;

			VoronoiMapGenerator() 
			{
				setUseSpacing(true);
			}
			template <class ImagePointer>
			VoronoiMapGenerator(const ImagePointer& template_image)
			{
				setOutputBasedOn(template_image);
				setUseSpacing(true);
			}

			template <class ImagePointer>
			void setOutputBasedOn(const ImagePointer& template_image)
			{
				m_Output = OutputImageType::New(template_image);
				ImageHelper::Fill(m_Output, boost::numeric::bounds<OutputValueType>::highest());
			}
						
			void setUseSpacing(bool en)
			{
				m_UseSpacing = en;
			}

			template <class IterType>
			typename boost::enable_if<getPoint_exists<IterType>, bool>::type addSeed(const IterType& seed) 
			{
				if (m_Output->get(seed)!=boost::numeric::bounds<OutputValueType>::highest()) return false;
				m_Seed.push_back(seed.getPoint());
				m_Output->set(seed, m_Seed.size()-1);
				return true;
			}

			bool addSeed(const Point3D<int>& seed) 
			{
				if (m_Output->get(seed)!=boost::numeric::bounds<OutputValueType>::highest()) return false;
				m_Seed.push_back(seed);
				m_Output->set(seed, m_Seed.size()-1);
				return true;
			}

			bool addSeed(const Point3D<int>& seed, long index)
			{
				if (m_Output->get(index)!=boost::numeric::bounds<OutputValueType>::highest()) return false;
				m_Seed.push_back(seed);
				m_Output->set(index, m_Seed.size()-1);
				return true;
			}

			void update()
			{
				if (m_UseSpacing) m_Spacing = m_Output->getSpacing();
				else m_Spacing.set(1,1,1);
				const int dimension = 3;
				for (int d=0; d<dimension; ++d) {
					int cur_dim[dimension-1];
					for (int c=0, i=0; i<dimension; ++i) if (i!=d) {
						cur_dim[c] = i+1;
						++c;
					}
					ImageIteratorWithPoint iter(m_Output, ImageIterator::Axis(cur_dim[0]), ImageIterator::Axis(cur_dim[1]));
					pcl_ForIterator(iter) {
						voronoi(d, iter.getPoint(), iter);
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

			const std::vector<Point3D<int>>& getSeed() const
			{
				return m_Seed;
			}

		protected:
			typename OutputImageType::Pointer m_Output;
			std::vector<Point3D<int>> m_Seed;
			Point3D<double> m_Spacing;
			bool m_UseSpacing;

			inline double deltaD(const Point3D<int>& u, const Point3D<int>& r, int d)
			{
				double result = 0;
				for (int i=0; i<3; ++i) if (i!=d) result += pcl::square((u[i]-r[i])*m_Spacing[i]);
				return result;
			}

			inline bool remove( OutputValueType u_label, OutputValueType v_label, OutputValueType w_label, int d, const Point3D<int>& point)
			{
				const Point3D<int> &u = m_Seed[u_label],
					&v = m_Seed[v_label],
					&w = m_Seed[w_label];
				OutputValueType a = (v[d] - u[d])*m_Spacing[d];
				OutputValueType b = (w[d] - v[d])*m_Spacing[d];
				OutputValueType c = a + b;
				return ( c*deltaD(v,point,d) - b*deltaD(u,point,d) - a*deltaD(w,point,d) - a*b*c ) > 0;
			}

			void voronoi(int d, const Point3D<int>& point, long index)
			{
				int size = m_Output->getSize()[d];
				long offset = m_Output->getOffsetTable()[d];
				std::vector<OutputValueType> g(size, 0);
				int l = -1;
				long x = index - offset;
				for (int i=0; i<size; i++) {
					x += offset;
					OutputValueType f = m_Output->get(x);
					if (f != boost::numeric::bounds<OutputValueType>::highest()) {
						if (l<1) {
							++l;
							g[l] = f;
						} else {
							while (l>=1 && remove(g[l-1], g[l], f, d, point)) {
								--l;
							}
							++l;
							g[l] = f;
						}
					}
				}
				//std::cout << std::endl;
				if (l==-1) return;
				int ns = l;
				l = 0;
				x = index - offset;
				Point3D<int> x_p = point;
				--x_p[d];
				for (int i=0; i<size; i++) {
					x += offset;
					++x_p[d];
					while (l<ns && ( m_Seed[g[l]].getEuclideanDistanceSqr(x_p, m_Spacing) > m_Seed[g[l+1]].getEuclideanDistanceSqr(x_p, m_Spacing) )) {
						++l;
					}
					m_Output->set(x, g[l]);
				}
			}
		};

	}
}

#endif