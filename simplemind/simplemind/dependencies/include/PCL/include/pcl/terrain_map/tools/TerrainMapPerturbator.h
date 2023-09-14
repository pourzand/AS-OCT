#ifndef PCL_TERRAIN_MAP_PERTURBATOR
#define PCL_TERRAIN_MAP_PERTURBATOR

#include <pcl/terrain_map/TerrainMap.h>

namespace pcl
{
	namespace terrain_map
	{
		using namespace pcl;

		/*!
			A class to facilitate pertubation of a terrain map via an iterator interface that is provided for each point on the terrain map.
		*/
		template <class TerrainMapType, class ImageType>
		class TerrainMapPerturbator: private boost::noncopyable
		{
		public:
			class Iterator
			{
				friend TerrainMapPerturbator<TerrainMapType, ImageType>;
			public:
			
				Iterator() {}
			
				void begin() const {}
				void next() const
				{
					m_PhysicalPoint += m_Direction;
					m_TotalDisplacement += m_DirectionNorm;
				}
				bool end() const
				{
					return !m_PhysicalPoint.isFinite() || !m_PhysicalRegion.contain(m_PhysicalPoint);
				}

				void set()
				{
					m_TerrainMap->set(m_TerrainMapIndex, m_TerrainMap->get(m_TerrainMapIndex) + m_TotalDisplacement*m_TerrainMap->getOneOverSpacing().z());
				}
				void set(double displacement)
				{
					m_TerrainMap->set(m_TerrainMapIndex, m_TerrainMap->get(m_TerrainMapIndex) + displacement*m_TerrainMap->getOneOverSpacing().z());
				}

				double getTotalDisplacement() const
				{
					return m_TotalDisplacement;
				}

				operator const Point3D<double>&() const
				{
					return m_PhysicalPoint;
				}
				const Point3D<double>& getPhysicalPoint() const
				{
					return m_PhysicalPoint;
				}

				inline void getImagePointIndex(Point3D<int>& point, long& index)
				{
					Point3D<double> &img_pos = m_Image->toImageCoordinate(m_PhysicalPoint);
					point.set(pcl_Round(img_pos.x()), pcl_Round(img_pos.y()), pcl_Round(img_pos.z()));
					index = m_Image->localToIndex(point);
				}
				inline PointIndexObject getImagePointIndex()
				{
					PointIndexObject obj;
					getImagePointIndex(obj.point, obj.index);
					return obj;
				}
				
				Iterator& operator=(const Iterator& iter)
				{
					m_TerrainMap = iter.m_TerrainMap;
					m_Image = iter.m_Image;
					m_Direction = iter.m_Direction;
					m_DirectionNorm = iter.m_DirectionNorm;
					m_PhysicalPoint = iter.m_PhysicalPoint;
					m_TotalDisplacement = iter.m_TotalDisplacement;
					m_TerrainMapIndex = iter.m_TerrainMapIndex;
					m_PhysicalRegion = iter.m_PhysicalRegion;
					return *this;
				}

				//Temp
				typename ImageType::ConstantPointer getImage()
				{
					return m_Image;
				}

			protected:
				typename TerrainMapType::Pointer m_TerrainMap;
				typename ImageType::ConstantPointer m_Image;
				Point3D<double> m_Direction;
				double m_DirectionNorm;
				mutable Point3D<double> m_PhysicalPoint;
				mutable double m_TotalDisplacement;
				long m_TerrainMapIndex;
				Region3D<double> m_PhysicalRegion;

				Iterator(const typename TerrainMapType::Pointer& map, const typename ImageType::ConstantPointer& image, const Point3D<double>& pos, const Point3D<double>& direction, double direction_norm, long index) 
				{
					m_TerrainMap = map;
					m_Image = image;
					m_PhysicalPoint = pos;
					m_Direction = direction;
					m_DirectionNorm = direction_norm;
					m_TerrainMapIndex = index;
					m_TotalDisplacement = 0;
					m_PhysicalRegion = m_Image->getPhysicalRegion();
				}
			};

			TerrainMapPerturbator() {}
			TerrainMapPerturbator(const typename TerrainMapType::Pointer& terrain_map, const typename ImageType::ConstantPointer& image)
			{
				setInput(terrain_map, image);
			}

			void setInput(const typename TerrainMapType::Pointer& terrain_map, const typename ImageType::ConstantPointer& image)
			{
				m_TerrainMap = terrain_map;
				m_Image = image;

				m_Direction = m_TerrainMap->getOrientationMatrixColumn(2);
				m_Direction *= m_Image->getOneOverSpacing(); //Converting to voxel direction
				int max_direction_index = 0;
				for (int i=1; i<3; i++) if (m_Direction[i]>m_Direction[max_direction_index]) max_direction_index = i;
				m_Direction /= m_Direction[max_direction_index]; //Guarantee to increase only 1 voxel space at a time
				m_Direction *= m_Image->getSpacing(); //Converting back to physical direction
				m_DirectionNorm = m_Direction.getNorm();
			}

			Iterator getIterator(int x, int y, long index)
			{
				return Iterator(m_TerrainMap, m_Image, m_TerrainMap->toPhysicalCoordinate(x,y,m_TerrainMap->get(index)), m_Direction, m_DirectionNorm, index);
			}

			Iterator getReverseIterator(int x, int y, long index)
			{
				return Iterator(m_TerrainMap, m_Image, m_TerrainMap->toPhysicalCoordinate(x,y,m_TerrainMap->get(index)), -m_Direction, -m_DirectionNorm, index);
			}

			double getStepSize() const
			{
				return m_DirectionNorm*m_TerrainMap->getOneOverSpacing().z();
			}

		protected:
			typename TerrainMapType::Pointer m_TerrainMap;
			typename ImageType::ConstantPointer m_Image;
			Point3D<double> m_Direction;
			double m_DirectionNorm;
		};

	}
}

#endif