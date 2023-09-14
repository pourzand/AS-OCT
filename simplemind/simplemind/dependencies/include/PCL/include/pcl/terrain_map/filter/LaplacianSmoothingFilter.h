#ifndef PCL_LAPLACIAN_SMOOTHING_FILTER
#define PCL_LAPLACIAN_SMOOTHING_FILTER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>

namespace pcl
{
	namespace terrain_map_filter
	{

		template <class Type>
		class LaplacianSmoothingFilter: public boost::noncopyable
		{
		public:
			typedef LaplacianSmoothingFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef Type ResultType;

			template <class InputPointertType>
			static Pointer New(typename InputPointertType& input, double beta=0.5)
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

			void update()
			{
				m_Source.swap(m_Result);
				if (m_SafeRegionExist) {
					pcl_ForIterator(m_SafeIterator) {
						double cur_val = m_Source->get(m_SafeIterator);
						double laplacian_point = 0;
						for (int i=0; i<4; i++) {
							laplacian_point += m_Source->get(m_SafeIterator+m_IndexOffset[i]);
						}
						laplacian_point /= 4.;
						m_Result->set(m_SafeIterator, cur_val+m_Beta*(laplacian_point-cur_val));
					}
				}
				pcl_ForEach(m_UnsafeRegionList, item) {
					m_UnsafeIterator.setRegion(*item);
					Point3D<int> p;
					pcl_ForIterator(m_UnsafeIterator) {
						p = m_UnsafeIterator.getPoint();
						double cur_val = m_Source->get(m_UnsafeIterator);
						double laplacian_point = 0,
							num = 0;
						for (int i=0; i<4; i++) {
							p.x() = m_UnsafeIterator.getPoint().x()+m_PointOffset[i][0];
							p.y() = m_UnsafeIterator.getPoint().y()+m_PointOffset[i][1];
							if (m_Source->contain(p)) {
								laplacian_point += m_Source->get(m_UnsafeIterator+m_IndexOffset[i]);
								num++;
							}
						}
						laplacian_point /= num;
						m_Result->set(m_UnsafeIterator, cur_val+m_Beta*(laplacian_point-cur_val));
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
			std::list<Region3D<int> > m_UnsafeRegionList;
			bool m_SafeRegionExist;
			ImageIterator m_SafeIterator;
			ImageIteratorWithPoint m_UnsafeIterator;

			LaplacianSmoothingFilter() {}

			template <class InputPointertType>
			void setInput(typename InputPointertType& input)
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