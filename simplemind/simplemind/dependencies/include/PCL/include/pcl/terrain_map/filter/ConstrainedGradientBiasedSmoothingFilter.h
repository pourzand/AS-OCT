#ifndef PCL_CONSTRAINED_GRADIENT_BIASED_SMOOTHING_FILTER
#define PCL_CONSTRAINED_GRADIENT_BIASED_SMOOTHING_FILTER

#include <pcl/terrain_map/filter/GradientBiasedSmoothingFilter.h>
#include <pcl/math.h>

namespace pcl
{
	namespace terrain_map_filter
	{

		template <class Type>
		class ConstrainedGradientBiasedSmoothingFilter: public GradientBiasedSmoothingFilter<Type>
		{
		public:
			typedef ConstrainedGradientBiasedSmoothingFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef Type ResultType;

			template <class InputPointerType>
			static Pointer New(InputPointerType& input, double max_ignored_gradient=0, double beta=1)
			{
				Pointer obj(new Self);
				obj->setBeta(beta);
				obj->setMaxIgnoredGradient(max_ignored_gradient);
				obj->setInput(input);
				return obj;
			}

			template <class InputPointerType, class AnchorListType>
			static Pointer New(InputPointerType& input, const AnchorListType& anchor_list, double max_ignored_gradient=0, double beta=1)
			{
				Pointer obj(new Self);
				obj->setBeta(beta);
				obj->setMaxIgnoredGradient(max_ignored_gradient);
				obj->setInput(input);
				pcl_ForEach(anchor_list, item) {
					addAnchor(*item);
				}
				return obj;
			}

			void setMaxIgnoredGradient(double val)
			{
				m_MaxIgnoredGradient = val;
			}

			void addAnchor(const Point3D<int>& p) 
			{
				addAnchor(m_Source->localToIndex(p));
			}
			void addAnchor(long index)
			{
				m_Anchor.push_back(index);
			}

			void update()
			{
				m_Source.swap(m_Result);
				computeGradientFromSource();
				pcl_ForEach(m_Anchor, item) {
					m_Gradient->set(*item, 0); //Prevent anchor points from being moved
				}
				{
					double minv, maxv;
					ImageHelper::GetMinMax(m_Gradient, minv, maxv);
					std::cout << "grad: " << minv << " " << maxv << std::endl;
				}

				double max_displacement = -std::numeric_limits<double>::infinity();

				m_MaxGradient = -std::numeric_limits<double>::infinity();
				if (m_SafeRegionExist) {
					pcl_ForIterator(m_SafeIterator) {
						double cur_grad = m_Gradient->get(m_SafeIterator);
						if (m_MaxGradient<cur_grad) m_MaxGradient = cur_grad;
						double cur_val = m_Source->get(m_SafeIterator);
						if (cur_grad>m_MaxIgnoredGradient) {
							double laplacian_point = 0;
							for (int i=0; i<4; i++) {
								double neighbor_grad = m_Gradient->get(m_SafeIterator+m_IndexOffset[i]);
								if (neighbor_grad>m_MaxIgnoredGradient) {
									double ratio = cur_grad/(cur_grad+neighbor_grad);
									laplacian_point += ratio*(m_Source->get(m_SafeIterator+m_IndexOffset[i])-cur_val);
								} else {
									laplacian_point += m_Source->get(m_SafeIterator+m_IndexOffset[i])-cur_val;
								}
							}
							laplacian_point /= 4.;
							m_Result->set(m_SafeIterator, cur_val+m_Beta*laplacian_point);
							max_displacement = std::max(max_displacement, pcl::abs(m_Beta*laplacian_point));
						} else  m_Result->set(m_SafeIterator, cur_val);
					}
				}
				std::cout << "    " << max_displacement << std::endl;
				pcl_ForEach(m_UnsafeRegionList, item) {
					m_UnsafeIterator.setRegion(*item);
					Point3D<int> p;
					pcl_ForIterator(m_UnsafeIterator) {
						double cur_grad = m_Gradient->get(m_UnsafeIterator);
						if (m_MaxGradient<cur_grad) m_MaxGradient = cur_grad;
						double cur_val = m_Source->get(m_UnsafeIterator);
						if (cur_grad>m_MaxIgnoredGradient) {
							p = m_UnsafeIterator.getPoint();
							double laplacian_point = 0,
								num = 0;
							for (int i=0; i<4; i++) {
								p.x() = m_UnsafeIterator.getPoint().x()+m_PointOffset[i][0];
								p.y() = m_UnsafeIterator.getPoint().y()+m_PointOffset[i][1];
								if (m_Source->contain(p)) {
									double neighbor_grad = m_Gradient->get(m_UnsafeIterator+m_IndexOffset[i]);
									if (neighbor_grad>m_MaxIgnoredGradient) {
										double ratio = cur_grad/(cur_grad+neighbor_grad);
										laplacian_point += ratio*(m_Source->get(m_UnsafeIterator+m_IndexOffset[i])-cur_val);
									} else {
										laplacian_point += m_Source->get(m_UnsafeIterator+m_IndexOffset[i])-cur_val;
									}
									num++;
								}
							}
							laplacian_point /= num;
							m_Result->set(m_UnsafeIterator, cur_val+m_Beta*laplacian_point);
						} else  m_Result->set(m_UnsafeIterator, cur_val);
					}
				}
			}

			double getMaxGradientPreviousUpdate() const
			{
				return m_MaxGradient;
			}

		protected:
			std::list<long> m_Anchor;
			double m_MaxIgnoredGradient;
			double m_MaxGradient;

			ConstrainedGradientBiasedSmoothingFilter() {}
		};

	}
}

#endif