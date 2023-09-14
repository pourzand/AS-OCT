#ifndef PCL_NEAREST_NEIGHBOR_TERRAIN_MAP_SETTER
#define PCL_NEAREST_NEIGHBOR_TERRAIN_MAP_SETTER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/terrain_map/tools/TerrainMapTension.h>

namespace pcl
{
	namespace terrain_map
	{

		/*!
			Setting the values/heights in a terrain map based on nearest neighbor rule.
			If more than one physical coordinates of the same priority are to be assigned to a particular point in the terrain map, the one where the final projected coordinates is closest to the point will be assigned.
		*/
		template <class TerrainMapType>
		class NearestNeighborTerrainMapSetter: private boost::noncopyable
		{
		public:
			typedef float DistanceValueType;
			typedef TerrainMap<DistanceValueType> DistanceMapType;

			NearestNeighborTerrainMapSetter() {}
			NearestNeighborTerrainMapSetter(const typename TerrainMapType::Pointer& map)
			{
				setTerrainMap(map);
			}

			void setTerrainMap(const typename TerrainMapType::Pointer& map)
			{
				m_TerrainMap = map;
				m_Dist = DistanceMapType::New(m_TerrainMap);
				ImageHelper::Fill(m_Dist, 0);
			}

			template <class PT>
			bool containPhysicalPoint(const PT& p) const
			{
				auto phy_p = m_TerrainMap->toImageCoordinate(p);
				Point3D<int> coord(
					pcl_Round(p.x()),
					pcl_Round(p.y()),
					m_TerrainMap->getMinPoint().z()
					);
				return m_TerrainMap->contain(coord);
			}

			template <class PT>
			bool addPoint(const PT& p, bool is_priority=false)
			{
				//Negative for priority points, Positive for normal points
				//Priority points will ignore distance and overwrite normal points
				//Note that the distance is offseted with a 1 by default (-1 for priority points)
				Point3D<int> coord(
					pcl_Round(p.x()),
					pcl_Round(p.y()),
					m_TerrainMap->getMinPoint().z()
					);
				if (!m_TerrainMap->contain(coord)) {
					coord.max(m_TerrainMap->getMinPoint());
					coord.min(m_TerrainMap->getMaxPoint());
				}
				long index = m_TerrainMap->localToIndex(coord);
				DistanceValueType cur_dist = getDistance(p, coord);
				DistanceValueType recorded_dist = m_Dist->get(index);
				bool added = false;
				if (recorded_dist==0) {
					m_TerrainMap->set(index, p.z());
					m_Dist->set(index, is_priority?-cur_dist:cur_dist);
					added = true;
				} else {
					if (is_priority && recorded_dist>0) { //Set directly as priority point has priviledge
						m_TerrainMap->set(index, p.z());
						m_Dist->set(index, -cur_dist);
						added = true;
					} else if (!is_priority && recorded_dist<0); //Do nothing as normal point cannot overwrite priority point
					else { //Normal processing
						if (cur_dist<pcl_Abs(recorded_dist)) {
							m_TerrainMap->set(index, p.z());
							m_Dist->set(index, is_priority?-cur_dist:cur_dist);
							added = true;
						}
					}
				}
				return added;
			}
			
			template <class PT>
			bool addPhysicalPoint(const PT& p, bool is_priority=false)
			{
				return addPoint(m_TerrainMap->toImageCoordinate(p), is_priority);
			}

			template <class PT>
			void resetPoint(const PT& p)
			{
				Point3D<int> coord(
					pcl_Round(p.x()),
					pcl_Round(p.y()),
					m_TerrainMap->getMinPoint().z()
					);
				if (!m_TerrainMap->contain(coord)) {
					coord.max(m_TerrainMap->getMinPoint());
					coord.min(m_TerrainMap->getMaxPoint());
				}
				m_Dist->set(coord, 0);
			}

			template <class PT>
			void resetPhysicalPoint(const PT& p)
			{
				resetPoint(m_TerrainMap->toImageCoordinate(p));
			}

			void update() {} //Not needed

			void updateWithTensioning() {
				updateWithTensioning<TerrainMapTension<TerrainMapType> >();
			}
			template <class TensionType>
			void updateWithTensioning()
			{
				update();
				TensionType tension(m_TerrainMap);
				ImageIteratorWithPoint iter(m_TerrainMap);
				pcl_ForIterator(iter) if (m_Dist->get(iter)!=0) {
					tension.addAnchor(iter.getPoint(), iter);
				}
				tension.update();
			}

			typename TerrainMapType::Pointer getTerrainMap()
			{
				return m_TerrainMap;
			}

			DistanceMapType::Pointer getWeightMap()
			{
				return m_Dist;
			}

		protected:
			typename TerrainMapType::Pointer m_TerrainMap;
			DistanceMapType::Pointer m_Dist;

			template <class PT>
			DistanceValueType getDistance(const PT& org_point, const Point3D<int>& actual_point)
			{
				return pcl::square(org_point.x()-actual_point.x()) + pcl::square(org_point.y()-actual_point.y()) + 1;
			}
		};

	}
}

#endif