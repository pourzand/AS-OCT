#ifndef PCL_EUCLIDEAN_DISTANCE_TRANSFORM_FILTER
#define PCL_EUCLIDEAN_DISTANCE_TRANSFORM_FILTER

#include <pcl/filter/image/ImageFilterBase.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/math.h>
#include <boost/math/special_functions/fpclassify.hpp>
#include <boost/numeric/conversion/bounds.hpp>

namespace pcl
{
	namespace filter
	{

		/**
			Note: Only computes for foreground!
		**/
		template <class InputImageType, class OutputImageType, template <class> class BoundaryHandlerClass=RepeatingBoundaryHandler>
		class EuclideanDistanceTransformFilter: public ImageFilterBase
		{
		public:
			typedef BoundaryHandlerClass<InputImageType> BoundaryHandlerType;
			typedef typename OutputImageType::IoValueType OutputValueType;

			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, bool use_square_distance, bool use_spacing, const Region3D<int>& process_region=Region3D<int>().reset())
			{
				EuclideanDistanceTransformFilter filter;
				filter.setProcessRegion(process_region);
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

			void setUseSquareDistance(bool en)
			{
				m_UseSquareDistance = en;
			}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
			}

			BoundaryHandlerType& getBoundaryHandler() 
			{
				return m_BoundaryHandler;
			}

			void update()
			{
				//Setting up environment
				m_BoundaryHandler.setImage(m_Input);
				this->m_ProcessRegion.setIntersect(m_Input->getRegion()); //Making sure that process region is within the input
				m_Output = OutputImageType::New(m_Input); //Output is created based on the input as template (same buffer size!)
				ImageHelper::Fill(m_Output, m_ProcessRegion, boost::numeric::bounds<OutputValueType>::highest());

				//Initialization
				initialize();

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
				ImageIterator iter(m_Input);
				iter.setRegion(m_ProcessRegion);
				if (m_UseSquareDistance) {
					pcl_ForIterator(iter) {
						if (!m_Input->get(iter)) m_Output->set(iter, 0);
						else {
							if (m_Output->get(iter)<0) {
								m_Output->set(iter, getImageDistance(m_Output->get(iter)));
							}
						}
					}
				} else {
					pcl_ForIterator(iter) {
						if (m_Input->get(iter)) {
							m_Output->set(iter, sqrt(getImageDistance(m_Output->get(iter))));
						} else m_Output->set(iter, 0);
//if (m_Output->get(iter)==boost::numeric::bounds<OutputValueType>::highest()) m_Output->set(iter, 0);
//else if (!m_Input->get(iter)) m_Output->set(iter, sqrt(m_Output->get(iter)));
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typename InputImageType::ConstantPointer m_Input;
			typename OutputImageType::Pointer m_Output;
			BoundaryHandlerType m_BoundaryHandler;
			bool m_UseSpacing;
			bool m_UseSquareDistance;
			std::vector<Point3D<int>> m_OffsetList;
			Point3D<double> m_SubSpacing;

			inline bool remove( OutputValueType d1, OutputValueType d2, OutputValueType df, OutputValueType x1, OutputValueType x2, OutputValueType xf )
			{
				OutputValueType a = x2 - x1;
				OutputValueType b = xf - x2;
				OutputValueType c = xf - x1;
				return ( c*abs(d2) - b*abs(d1) - a*abs(df) - a*b*c ) > 0;
			}

			void voronoi(int d, long index)
			{
				int size = m_ProcessRegion.getSize()[d];
				std::vector<OutputValueType> g(size, 0);
				std::vector<OutputValueType> h(size, 0);
				long offset = m_Input->getOffsetTable()[d];
				int l = -1;
				long x = index - offset;
				double i_offset = 1;
				if (m_UseSpacing) i_offset = m_Input->getSpacing()[d];
				double i = -i_offset;
				for (int i_count=0; i_count<size; i_count++) {
					x += offset;
					i += i_offset;
					OutputValueType f = m_Output->get(x);
					if (f != boost::numeric::bounds<OutputValueType>::highest() && m_Input->get(x)) {
						if (l<1) {
							++l;
							h[l] = static_cast<OutputValueType>(i);
							g[l] = f;
						} else {
							while (l>=1 && remove(getImageDistance(g[l-1]), getImageDistance(g[l]), getImageDistance(f), h[l-1], h[l], static_cast<OutputValueType>(i))) {
								--l;
							}
							++l;
							h[l] = static_cast<OutputValueType>(i);
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
					while (l<ns && ( getImageDistance(d,g[l],h[l]-i) ) > ( getImageDistance(d,g[l+1],h[l+1]-i) )) {
						++l;
					}
					double dist = getImageDistance(d,g[l],h[l]-i);
					if (getImageDistance(m_Output->get(x))>dist) m_Output->set(x, dist);
				}
			}
			inline double getImageDistance(int d, OutputValueType val, double h)
			{
				if (val>=0) return val + square(h);
				double result = 0;
				auto &p = m_OffsetList[static_cast<int>(-val)-1];
				for (int i=0; i<3; ++i) {
					if (i==d) result += square(p[i]*m_SubSpacing[i] + abs(h));
					else result += square(p[i]*m_SubSpacing[i]);
				}
				return result;
			}
			inline double getImageDistance(OutputValueType val) {
				if (val>=0) return val;
				auto &p = m_OffsetList[static_cast<int>((-val)-1)];
				return square(p[0]*m_SubSpacing[0]) + square(p[1]*m_SubSpacing[1]) + square(p[2]*m_SubSpacing[2]);
			}

			template<class ImageIteratorType, class InitIteratorType, class BorderTest>
			void initialize_details(const Point3D<double>& sub_spacing, ImageIteratorType& image_iter, InitIteratorType& init_iter, BorderTest border_test)
			{
				pcl_ForIterator(image_iter) if (m_Input->get(image_iter)) {
					init_iter.setOrigin(image_iter.getPoint(), image_iter);
					bool is_border = false;
					pcl_ForIterator(init_iter) {
						if (border_test(init_iter)) {
							is_border = true;
							break;
						}
					}
					if (is_border) {
						double min_dist = std::numeric_limits<double>::infinity();
						int iteration_count;
						Point3D<double> source;
						pcl_ForIterator(init_iter) {
							if (border_test(init_iter)) {
								auto& offset = init_iter.getOffset();
								double dist = 0;
								for (int i=0; i<3; ++i) dist += pcl::square(offset[i]*sub_spacing[i]);
								if (dist<min_dist) {
									min_dist = dist;
									iteration_count = init_iter.getIteration();
								}
							}
						}
						m_Output->set(image_iter, -(iteration_count+1));
					}
				}
			}

			void initialize()
			{
				pcl::iterator::ImageNeighborIterator init_iter(m_Input);
				{
					auto offset_list = pcl::iterator::ImageNeighborIterator::CreateConnect26Offset();
					pcl::iterator::ImageNeighborIterator::FilterOffsetList(offset_list, m_ProcessRegion.getSize());
					init_iter = offset_list;
					m_OffsetList.clear();
					pcl_ForEach(*offset_list, item)
						m_OffsetList.push_back(Point3D<int>(abs(item->x()), abs(item->y()), abs(item->z())));
				}
				const auto zero = static_cast<typename InputImageType::IoValueType>(0);
				m_SubSpacing.set(0.5,0.5,0.5);
				if (m_UseSpacing) m_SubSpacing = m_Input->getSpacing()*0.5;
				misc::SafeUnsafeRegionGenerator rgn_gen(m_ProcessRegion, m_Input->getRegion(), init_iter.getOffsetRegion());
				ImageIteratorWithPoint iter(m_Input);
				if (!rgn_gen.getSafeRegion().empty()) {
					iter.setRegion(rgn_gen.getSafeRegion());
					initialize_details(m_SubSpacing, iter, init_iter, [&](pcl::iterator::ImageNeighborIterator& it)->bool {
						return m_Input->get(it)==zero;
					});
				}
				pcl_ForEach(rgn_gen.getUnsafeRegion(), item) {
					iter.setRegion(*item);
					initialize_details(m_SubSpacing, iter, init_iter, [&](pcl::iterator::ImageNeighborIterator& it)->bool {
						return m_BoundaryHandler.get(it)==zero;
					});
				}
			}
		};

	}
}

#endif