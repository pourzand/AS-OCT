#ifndef PCL_WEIGHTED_SMOOTHING_FILTER
#define PCL_WEIGHTED_SMOOTHING_FILTER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>

namespace pcl
{
	namespace terrain_map_filter
	{

		template <class RType>
		class WeightedSmoothingFilter: public boost::noncopyable
		{
		public:
			typedef WeightedSmoothingFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef RType ResultType;

			template <class InputPointerType>
			static Pointer New(InputPointerType& input, double beta=1)
			{
				Pointer obj(new Self);
				obj->setBeta(beta);
				obj->setInput(input);
				return obj;
			}

			void setBeta(double beta)
			{
				m_Beta = beta;
			}

			template <class TerrainType>
			typename TerrainType::Pointer getSourceGradient()
			{
				auto source_gradient = TerrainType::New(m_Source);
				if (m_SafeRegionExist) {
					pcl_ForIterator(m_SafeIterator) {
						double cur_val = m_Source->get(m_SafeIterator);
						double gradient = 0;
						for (int i=0; i<4; i++) {
							double cur_grad = (m_Source->get(m_SafeIterator+m_IndexOffset[i])-cur_val)/m_Norm[i];
							gradient += pcl_Abs(cur_grad);
						}
						gradient /= 4.;
						source_gradient->set(m_SafeIterator, gradient);
					}
				}
				pcl_ForEach(m_UnsafeRegionList, item) {
					m_UnsafeIterator.setRegion(*item);
					Point3D<int> p;
					pcl_ForIterator(m_UnsafeIterator) {
						p = m_UnsafeIterator.getPoint();
						double cur_val = m_Source->get(m_UnsafeIterator);
						double gradient = 0,
							num = 0;
						for (int i=0; i<4; i++) {
							p.x() = m_UnsafeIterator.getPoint().x()+m_PointOffset[i][0];
							p.y() = m_UnsafeIterator.getPoint().y()+m_PointOffset[i][1];
							if (m_Source->contain(p)) {
								double cur_grad = (m_Source->get(m_SafeIterator+m_IndexOffset[i])-cur_val)/m_Norm[i];
								gradient += pcl_Abs(cur_grad);
								num++;
							}
						}
						gradient /= num;
						source_gradient->set(m_SafeIterator, gradient);
					}
				}
				return source_gradient;
			}

			template <class WeightPointer>
			void update(const WeightPointer& weight)
			{
				m_Source.swap(m_Result);
				if (m_SafeRegionExist) {
					pcl_ForIterator(m_SafeIterator) {
						double cur_weight = weight->get(m_SafeIterator);
						double cur_val = m_Source->get(m_SafeIterator);
						if (cur_weight>0) {
							double laplacian_point = 0;
							for (int i=0; i<4; i++) {
								double ratio = cur_weight/(cur_weight+weight->get(m_SafeIterator+m_IndexOffset[i]));
								laplacian_point += ratio*(m_Source->get(m_SafeIterator+m_IndexOffset[i])-cur_val);
							}
							laplacian_point /= 4.;
							m_Result->set(m_SafeIterator, cur_val+m_Beta*laplacian_point);
						} else  m_Result->set(m_SafeIterator, cur_val);
					}
				}
				pcl_ForEach(m_UnsafeRegionList, item) {
					m_UnsafeIterator.setRegion(*item);
					Point3D<int> p;
					pcl_ForIterator(m_UnsafeIterator) {
						double cur_weight = weight->get(m_UnsafeIterator);
						double cur_val = m_Source->get(m_UnsafeIterator);
						if (cur_weight>0) {
							p = m_UnsafeIterator.getPoint();
							double laplacian_point = 0,
								num = 0;
							for (int i=0; i<4; i++) {
								p.x() = m_UnsafeIterator.getPoint().x()+m_PointOffset[i][0];
								p.y() = m_UnsafeIterator.getPoint().y()+m_PointOffset[i][1];
								if (m_Source->contain(p)) {
									double ratio = cur_weight/(cur_weight+weight->get(m_UnsafeIterator+m_IndexOffset[i]));
									laplacian_point += ratio*(m_Source->get(m_UnsafeIterator+m_IndexOffset[i])-cur_val);
									num++;
								}
							}
							laplacian_point /= num;
							m_Result->set(m_UnsafeIterator, cur_val+m_Beta*laplacian_point);
						} else m_Result->set(m_UnsafeIterator, cur_val);
					}
				}
			}

			typename ResultType::Pointer getResult()
			{
				return m_Result;
			}

		protected:
			typename ResultType::Pointer m_Source, m_Result;
			double m_Beta;
			long m_IndexOffset[4];
			int m_PointOffset[4][2];
			double m_Norm[4];
			bool m_SafeRegionExist;
			std::list<Region3D<int> > m_UnsafeRegionList;
			ImageIterator m_SafeIterator;
			ImageIteratorWithPoint m_UnsafeIterator;

			WeightedSmoothingFilter() {}

			template <class InputPointerType>
			void setInput(InputPointerType& input)
			{
				m_Result = ResultType::New(input);
				m_Source = ResultType::New(input);

				ImageHelper::Copy(input, m_Result);

				int x_offset[] = {-1, 1, 0, 0},
					y_offset[] = {0, 0, -1, 1};
				for (int i=0; i<4; i++) {
					m_PointOffset[i][0] = x_offset[i];
					m_PointOffset[i][1] = y_offset[i];
					m_IndexOffset[i] = input->getOffsetTable()[0]*x_offset[i] + input->getOffsetTable()[1]*y_offset[i];
					double x_len = x_offset[i]*input->getSpacing().x(),
						y_len = y_offset[i]*input->getSpacing().y();
					m_Norm[i] = sqrt(x_len*x_len + y_len*y_len);
				}

				misc::SafeUnsafeRegionGenerator region_gen(
					m_Result->getRegion(), 
					m_Result->getRegion(), 
					Region3D<int>(Point3D<int>(-1,-1,0), Point3D<int>(1,1,0))
					);
				m_UnsafeRegionList = region_gen.getUnsafeRegion();

				m_SafeIterator.setImage(m_Result);
				m_SafeIterator.setRegion(region_gen.getSafeRegion());
				if (!region_gen.getSafeRegion().empty()) m_SafeRegionExist = true;
				else m_SafeRegionExist = false;

				m_UnsafeIterator.setImage(m_Result);
			}
		};

	}
}

#endif