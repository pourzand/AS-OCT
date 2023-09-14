#ifndef PCL2_EUCLIDEAN_DISTANCE_TRANSFORM_FILTER
#define PCL2_EUCLIDEAN_DISTANCE_TRANSFORM_FILTER

#include <pcl/filter2/image/ImageFilterBase.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/iterator/ImageRegionsIterator.h>
#include <pcl/math.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/numeric/conversion/bounds.hpp>

namespace pcl
{
	namespace filter2
	{	
		/**
			Reference: C. R. Maurer, Jr., R. Qi, and V. Raghavan, "A Linear Time Algorithm for Computing Exact Euclidean Distance Transforms of Binary Images in Arbitrary Dimensions", IEEE - Transactions on Pattern Analysis and Machine Intelligence, 25(2): 265-270, 2003. 
			Note: Foreground is where input is larger than 0
		**/
		template <class BoundaryType, class OutputImageType>
		class EuclideanDistanceTransformFilter: public ImageFilterBase
		{
		public:
			typedef typename OutputImageType::IoValueType OutputValueType;
			
			static typename OutputImageType::Pointer Compute(const BoundaryType& input, bool is_signed, bool use_square_distance, bool use_spacing, const Region3D<int>& output_region=Region3D<int>().reset())
			{
				EuclideanDistanceTransformFilter filter;
				filter.setOutputRegion(output_region);
				filter.setIsSigned(is_signed);
				filter.setUseSquareDistance(use_square_distance);
				filter.setUseSpacing(use_spacing);
				filter.setInput(input);
				filter.update();
				return filter.getOutput();
			}


			EuclideanDistanceTransformFilter() {}

			void setUseSpacing(bool en)
			{
				m_UseSpacing = en;
			}
			
			void setIsSigned(bool en)
			{
				m_IsSigned = en;
			}

			void setUseSquareDistance(bool en)
			{
				m_UseSquareDistance = en;
			}

			void setInput(const BoundaryType& input)
			{
				m_Input = input;
			}

			void update()
			{
				//Setting up environment
				m_Output = this->createImage<OutputImageType>(m_Input.getImage());
				pcl::ImageHelper::Fill(m_Output, boost::numeric::bounds<OutputValueType>::highest());

				//Initialization
				if (m_IsSigned) borderInitialize();
				else initialize();

				//Actual computation
				const int dimension = 3;
				for (int d=0; d<dimension; ++d) {
					int cur_dim[dimension-1];
					for (int c=0, i=0; i<dimension; ++i) if (i!=d) {
						cur_dim[c] = i+1;
						++c;
					}
					ImageIterator iter(m_Output, ImageIterator::Axis(cur_dim[0]), ImageIterator::Axis(cur_dim[1]));
					pcl_ForIterator(iter) {
						voronoi(d, iter);
					}
				}

				//Finalize values
				if (!m_IsSigned) {
					if (!m_UseSquareDistance) {
						ImageIterator iter(m_Output);
						pcl_ForIterator(iter) {
							if (m_Output->get(iter)>0) m_Output->set(iter, sqrt(m_Output->get(iter)));
						}
					}
				} else {
					pcl::Region3D<int> safe_region(m_Output->getRegion());
					safe_region.setIntersect(m_Input.getImage()->getRegion());
					auto unsafe_region = m_Output->getRegion().getRegionsAfterSubtractionBy(safe_region);

					ImageRegionsIterator<> out_iter(m_Output);
					ImageRegionsIteratorWithPoint<> in_iter(m_Input.getImage());
					out_iter.add(safe_region, 1); in_iter.add(safe_region, 1);
					out_iter.addList(unsafe_region, 0); in_iter.addList(unsafe_region, 0);
					if (m_UseSquareDistance) {
						pcl_ForIterator2(out_iter, in_iter) {
							bool is_inside;
							if (in_iter.getInfo()) is_inside = m_Input.getImage()->get(in_iter)>0;
							else is_inside = m_Input.get(in_iter)>0;
							if (!is_inside) m_Output->set(out_iter, -m_Output->get(out_iter));
						}
					} else {
						pcl_ForIterator2(out_iter, in_iter) {
							bool is_inside;
							if (in_iter.getInfo()) is_inside = m_Input.getImage()->get(in_iter)>0;
							else is_inside = m_Input.get(in_iter)>0;
							if (is_inside) m_Output->set(out_iter, sqrt(m_Output->get(out_iter)));
							else m_Output->set(out_iter, -sqrt(m_Output->get(out_iter)));
						}
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			BoundaryType m_Input;
			typename OutputImageType::Pointer m_Output;
			bool m_UseSpacing;
			bool m_UseSquareDistance;
			bool m_IsSigned;

			inline bool remove( OutputValueType d1, OutputValueType d2, OutputValueType df, OutputValueType x1, OutputValueType x2, OutputValueType xf )
			{
				OutputValueType a = x2 - x1;
				OutputValueType b = xf - x2;
				OutputValueType c = xf - x1;
				return ( c*abs(d2) - b*abs(d1) - a*abs(df) - a*b*c ) > 0;
			}

			void voronoi(int d, long index)
			{
				int size = m_Output->getRegion().getSize()[d];
				std::vector<OutputValueType> g(size, 0);
				std::vector<OutputValueType> h(size, 0);
				long offset = m_Output->getOffsetTable()[d];
				int l = -1;
				long x = index - offset;
				double i_offset = 1;
				if (m_UseSpacing) i_offset = m_Output->getSpacing()[d];
				double i = -i_offset;
				for (int i_count=0; i_count<size; i_count++) {
					x += offset;
					i += i_offset;
					OutputValueType f = m_Output->get(x);
					if (f != boost::numeric::bounds<OutputValueType>::highest()) {
						if (l<1) {
							++l;
							h[l] = i;
							g[l] = f;
						} else {
							while (l>=1 && remove(g[l-1], g[l], f, h[l-1], h[l], i)) {
								--l;
							}
							++l;
							h[l] = i;
							g[l] = f;
						}
					}
				}
				if (l==-1) return;
				int ns = l;
				l = 0;
				i = -i_offset;
				x = index - offset;
				for (int i_count=0; i_count<size; i_count++) {
					x += offset;
					i += i_offset;
					while (l<ns && ( g[l]+square(h[l]-i) ) > ( g[l+1] + square(h[l+1]-i) )) {
						++l;
					}
					m_Output->set(x, g[l] + square(h[l]-i));
				}
			}

			void initialize()
			{
				pcl::Region3D<int> safe_region(m_Output->getRegion());
				safe_region.setIntersect(m_Input.getImage()->getRegion());
				auto unsafe_regions = m_Output->getRegion().getRegionsAfterSubtractionBy(safe_region);
				
				ImageRegionsIterator<> out_iter(m_Output);
				ImageRegionsIteratorWithPoint<> in_iter(m_Input.getImage());
				out_iter.add(safe_region, 1); in_iter.add(safe_region, 1);
				out_iter.addList(unsafe_regions, 0); in_iter.addList(unsafe_regions, 0);
				pcl_ForIterator2(out_iter, in_iter) if (in_iter.getInfo()) {
					if (m_Input.getImage()->get(in_iter)<=0) m_Output->set(out_iter, 0);
				} else {
					if (m_Input.get(in_iter)<=0) m_Output->set(out_iter, 0);
				}
			}
			
			void borderInitialize()
			{
				auto offset_list = pcl::iterator::ImageNeighborIterator::CreateConnect26Offset();
				pcl::iterator::ImageNeighborIterator::FilterOffsetList(offset_list, m_Output->getRegion().getSize()); //Removing extra dimensions
				pcl::iterator::ImageNeighborIterator n_iter(m_Input.getImage(), offset_list);
				
				misc::SafeUnsafeRegionGenerator rgn_gen(m_Output->getRegion(), m_Input.getImage()->getRegion(), n_iter.getOffsetRegion());
				ImageRegionsIterator<> out_iter(m_Output);
				ImageRegionsIteratorWithPoint<> in_iter(m_Input.getImage());
				out_iter.add(rgn_gen.getSafeRegion(), 1); in_iter.add(rgn_gen.getSafeRegion(), 1);
				out_iter.addList(rgn_gen.getUnsafeRegion(), 0); in_iter.addList(rgn_gen.getUnsafeRegion(), 0);
				pcl_ForIterator2(out_iter, in_iter) if (m_Input.get(in_iter)<=0) {
					bool is_border = false;
					if (in_iter.getInfo()) {
						n_iter.setOrigin(in_iter.getPoint(), in_iter);
						pcl_ForIterator(n_iter) {
							if (m_Input.getImage()->get(n_iter)>0) {
								is_border = true;
								break;
							}
						}
					} else {
						n_iter.setOrigin(in_iter.getPoint());
						pcl_ForIterator(n_iter) {
							if (m_Input.get(n_iter.getPoint())>0) {
								is_border = true;
								break;
							}
						}
					}
					if (is_border) m_Output->set(out_iter, 0);
				}
			}
		};

	}
}

#endif