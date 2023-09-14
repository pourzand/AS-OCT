#ifndef PCL_TERRAIN_MAP_MAX_SETTER
#define PCL_TERRAIN_MAP_MAX_SETTER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/terrain_map/tools/TerrainMapTension.h>

namespace pcl
{
	namespace terrain_map
	{

		/*!
			Setting values/heights in to a terrain map for points with maximum weight
		*/
		template <class TerrainMapType>
		class TerrainMapMaxSetter: private boost::noncopyable
		{
		public:
			typedef float WeightValueType;
			typedef TerrainMap<WeightValueType> WeightMapType;

			TerrainMapMaxSetter() {}
			TerrainMapMaxSetter(const typename TerrainMapType::Pointer& map)
			{
				setTerrainMap(map);
			}

			void setTerrainMap(const typename TerrainMapType::Pointer& map)
			{
				m_TerrainMap = map;
				m_Weight = WeightMapType::New(m_TerrainMap);
				ImageHelper::Fill(m_Weight, -std::numeric_limits<WeightValueType>::infinity());
			}

			template <class PT>
			bool addPoint(const PT& p, WeightValueType w=1)
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
				long index = m_TerrainMap->localToIndex(coord);
				WeightValueType &weight = m_Weight->at(index);
				bool added = false;
				if (w>weight) {
					m_TerrainMap->set(index, p.z());
					weight = w;
					added = true;
				}
				return added;
			}
			
			template <class PT>
			bool addPhysicalPoint(const PT& p, WeightValueType w=1)
			{
				return addPoint(m_TerrainMap->toImageCoordinate(p), w);
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
				m_Weight->set(coord, -std::numeric_limits<WeightValueType>::infinity());
			}

			template <class PT>
			void resetPhysicalPoint(const PT& p)
			{
				resetPoint(m_TerrainMap->toImageCoordinate(p));
			}

			void update()
			{}

			void updateWithTensioning() {
				updateWithTensioning<TerrainMapTension<TerrainMapType> >();
			}
			template <class TensionType>
			void updateWithTensioning()
			{
				update();
				TensionType tension(m_TerrainMap);
				ImageIteratorWithPoint iter(m_TerrainMap);
				pcl_ForIterator(iter) if (m_Weight->get(iter)>-std::numeric_limits<WeightValueType>::infinity()) {
					tension.addAnchor(iter.getPoint(), iter);
				}
				tension.update();
			}

			typename TerrainMapType::Pointer getTerrainMap()
			{
				return m_TerrainMap;
			}

			typename WeightMapType::Pointer getWeightMap()
			{
				return m_Weight;
			}

		protected:
			typename TerrainMapType::Pointer m_TerrainMap;
			WeightMapType::Pointer m_Weight;
		};

	}
}

#endif