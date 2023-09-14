#ifndef PCL_TERRAIN_MAP_TENSION
#define PCL_TERRAIN_MAP_TENSION

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/iterator/BruteRegionGrowingIterator.h>
#include <pcl/filter/boundary_handler/RepeatingBoundaryHandler.h>
#include <pcl/misc/VoronoiMapGenerator.h>
#include <boost/numeric/conversion/bounds.hpp>

namespace pcl
{
	namespace terrain_map
	{ 
		using namespace pcl;
		using namespace pcl::iterator;
		using namespace pcl::misc;
		using namespace pcl::filter;

		/*!
			The tensioning function for interpolating a surface based on anchor points as described in "Art of Surface Interpolation" from M. Dressler
		*/
		template <class TerrainMapType, bool UseInternalOptimalPlane=false, class TensionFunctionType=TensionFunctionDegree3>
		class TerrainMapTension: private boost::noncopyable
		{
		public:
			typedef int LabelValueType;
			typedef TerrainMap<LabelValueType> LabelMapType;

			TerrainMapTension() {}
			TerrainMapTension(const typename TerrainMapType::Pointer& map)
			{
				setTerrainMap(map);
			}

			void setTerrainMap(const typename TerrainMapType::Pointer& map)
			{
				m_TerrainMap = map;
				m_BoundaryHandler.setImage(m_TerrainMap);
				m_Anchor.clear();
			}

			void addAnchor(const Point3D<int>& point)
			{
				addAnchor(point, m_TerrainMap->localToIndex(point));
			}
			void addAnchor(const Point3D<int>& point, long index)
			{
				m_Anchor.push_back(PointIndexObject(point,index));
			}

			void update()
			{
				TerrainMap<float>::Pointer euclidean_dist_inverse;
				if (!UseInternalOptimalPlane) euclidean_dist_inverse = computeLabel();
				else euclidean_dist_inverse = computeLabelWithOptimizedBoundaryValues();
				int max_iter = m_MaxChessBoardDist/2. + 2;
				max_iter = pcl_Round(max_iter);
				max_iter = pcl_Max(4, max_iter);
				ImageIteratorWithPoint iter(m_TerrainMap);
				for (int i=max_iter; i>=1; i--) {
					pcl_ForIterator(iter) {
						m_TerrainMap->set(
							iter, 
							computeTension(iter.getPoint().x(), iter.getPoint().y(), euclidean_dist_inverse->get(iter), i, m_Anchor[m_Label->get(iter)])
							);
					}
				}
			}

			double getValue(int x, int y, double x_offset, double y_offset)
			{
				Point3D<int> round_coord(
					pcl_Round(x+x_offset),
					pcl_Round(y+y_offset),
					m_TerrainMap->getMinPoint().z()
					);
				return m_BoundaryHandler.get(round_coord, m_TerrainMap->localToIndex(round_coord));
			}

		protected:
			typename TerrainMapType::Pointer m_TerrainMap;
			LabelMapType::Pointer m_Label;
			std::vector<PointIndexObject> m_Anchor;
			TensionFunctionType m_TensionFunction;
			RepeatingBoundaryHandler<TerrainMapType> m_BoundaryHandler;
			int m_MaxChessBoardDist;

			inline int getChessBoardDistance(int x, int y, const Point3D<int>& anchor_coord)
			{
				int dist_x = x-anchor_coord.x(),
					dist_y = y-anchor_coord.y();
				dist_x = pcl_Abs(dist_x);
				dist_y = pcl_Abs(dist_y);
				return pcl_Max(dist_x, dist_y);
			}

			TerrainMap<float>::Pointer computeLabel()
			{
				VoronoiMapGenerator<LabelMapType> map_gen(m_TerrainMap);
				map_gen.setUseSpacing(false);
				for (int i=0; i<m_Anchor.size(); ++i) map_gen.addSeed(m_Anchor[i]);
				map_gen.update();
				m_Label = map_gen.getOutput();

				TerrainMap<float>::Pointer euclidean_dist_inverse = TerrainMap<float>::New(m_TerrainMap);
				m_MaxChessBoardDist = 0;
				ImageIteratorWithPoint iter(m_Label);
				pcl_ForIterator(iter) {
					PointIndexObject &anchor = m_Anchor[m_Label->get(iter)];
					double euclidean_dist = anchor.point.getEuclideanDistance(iter.getPoint());
					euclidean_dist_inverse->set(iter, 1/euclidean_dist);
					m_TerrainMap->set(iter, m_TerrainMap->get(anchor.index));
					m_MaxChessBoardDist = std::max(m_MaxChessBoardDist, getChessBoardDistance(iter.getPoint().x(), iter.getPoint().y(), anchor.point));
				}
				m_TensionFunction.set(m_MaxChessBoardDist);
				return euclidean_dist_inverse;
			}

			template <class BasisMatrixType>
			Point3D<double> getPoint(int column, const BasisMatrixType& mat)
			{
				return Point3D<double>(mat(0,column), mat(1,column), mat(2,column)).normalize();
			}
			TerrainMap<float>::Pointer computeLabelWithOptimizedBoundaryValues()
			{
				VoronoiMapGenerator<LabelMapType> map_gen(m_TerrainMap);
				map_gen.setUseSpacing(false);
				for (int i=0; i<m_Anchor.size(); ++i) map_gen.addSeed(m_Anchor[i]);
				map_gen.update();
				m_Label = map_gen.getOutput();

				//Computing the optimal plane of representation
				pcl::geometry::Plane3D<double> optimal_plane;
				CovarianceMatrixGenerator<3> cov_gen;
				pcl_ForEach(m_Anchor, item) {
					cov_gen.add(&(item->getPoint())[0]);
				}
				try {
					VnlSymmetricEigenSystemWrapper<double> eigen(cov_gen.getCovarianceMatrix<double>());
					int min_index = 0;
					for (int i=1; i<3; i++) {
						if (pcl_Abs(eigen->get_eigenvalue(min_index))>pcl_Abs(eigen->get_eigenvalue(i))) min_index = i;
					}
					optimal_plane.setNormal(getPoint(min_index, eigen->V));
					optimal_plane.setOrigin(cov_gen.getCentroid<pcl::Point3D<double>>());
				} catch (const pcl::InvalidValueException&) {
					pcl_ThrowException(pcl::Exception(), "Error encountered when trying to compute the following covariance matrix: " + boost::lexical_cast<std::string>(cov_gen.getCovarianceMatrix<double>()));
				}

				TerrainMap<float>::Pointer euclidean_dist_inverse = TerrainMap<float>::New(m_TerrainMap);
				Point3D<double> terrain_normal = m_TerrainMap->getOrientationMatrixColumn(2);
				m_MaxChessBoardDist = 0;
				ImageIteratorWithPoint iter(m_Label);
				pcl_ForIterator(iter) {
					PointIndexObject &anchor = m_Anchor[m_Label->get(iter)];
					double euclidean_dist = anchor.point.getEuclideanDistance(iter.getPoint());
					euclidean_dist_inverse->set(iter, 1/euclidean_dist);

					//Getting the displacement in the optimal plane
					Point3D<double> anchor_physical_coord = m_TerrainMap->toPhysicalCoordinate( m_TerrainMap->getCoordinate<Point3D<double>>(anchor.point, anchor.index) );
					pcl::geometry::Line3D<double> anchor_line(
						anchor_physical_coord,
						terrain_normal
						);
					pcl::geometry::Line3D<double> target_line(
						m_TerrainMap->toPhysicalCoordinate(iter.getPoint(), 0),
						terrain_normal
						);
					Point3D<double> optimal_anchor_point(optimal_plane.getIntersection(anchor_line)),
						optimal_target_point(optimal_plane.getIntersection(target_line));
					Point3D<double> displace_vector = optimal_target_point - optimal_anchor_point;
					Point3D<double> target_physical_coord = anchor_physical_coord+displace_vector;

					Point3D<double> target_image_coord = m_TerrainMap->toImageCoordinate(target_physical_coord);

					m_TerrainMap->set(iter, m_TerrainMap->get(anchor.index));
					m_MaxChessBoardDist = std::max(m_MaxChessBoardDist, getChessBoardDistance(iter.getPoint().x(), iter.getPoint().y(), anchor.point));
				}
				m_TensionFunction.set(m_MaxChessBoardDist);
				return euclidean_dist_inverse;
			}

			float computeTension(int x, int y, double local_norm, int dist, const PointIndexObject& point_index) {
				double x_offset = x - point_index.point.x(),
					y_offset = y - point_index.point.y();
				double norm = dist*local_norm;
				if (norm<1) {
					x_offset *= norm;
					y_offset *= norm;
				}
				return m_TensionFunction(dist, x, y, x_offset, y_offset, *this);
			}
		};

		/********************************************************************/

		class TensionFunctionDegree0
		{
		public:
			TensionFunctionDegree0() {}
			void set(double max_chess_board_dist)
			{
				m_L = 0.7/((0.107*max_chess_board_dist-0.714)*max_chess_board_dist);
				m_MaxChessBoardDist = max_chess_board_dist;
			}
			template <class T>
			inline double operator()(int k, int x, int y, double x_offset, double y_offset, T& obj)
			{
				double q = m_L*SQUARES(m_NNGen->GetMaxChessBoardDist()-k);
				return ( 
					q*(obj.getValue(x,y,x_offset,y_offset) + obj.getValue(x,y,-x_offset,-y_offset)) +
					(obj.getValue(x,y,-x_offset,y_offset) + obj.getValue(x,y,x_offset,-y_offset))
					) / (2*q+2);
			}
		protected:
			double m_L;
			double m_MaxChessBoardDist;
		};

		class TensionFunctionDegree1
		{
		public:
			TensionFunctionDegree1() {}
			void set(double max_chess_board_dist)
			{
				m_L = 1/((0.107*max_chess_board_dist-0.714)*max_chess_board_dist);
				m_MaxChessBoardDist = max_chess_board_dist;
			}
			template <class T>
			inline double operator()(int k, int x, int y, double x_offset, double y_offset, T& obj)
			{
				double q = m_L*SQUARES(m_NNGen->GetMaxChessBoardDist()-k);
				return ( 
					q*(obj.getValue(x,y,x_offset,y_offset) + obj.getValue(x,y,-x_offset,-y_offset)) +
					(obj.getValue(x,y,-x_offset,y_offset) + obj.getValue(x,y,x_offset,-y_offset))
					) / (2*q+2);
			}
		protected:
			double m_L;
			double m_MaxChessBoardDist;
		};

		class TensionFunctionDegree2
		{
		public:
			TensionFunctionDegree2() {}
			void set(double max_chess_board_dist)
			{
				m_L = 1/(0.0360625*max_chess_board_dist+0.192);
				m_MaxChessBoardDist = max_chess_board_dist;
			}
			template <class T>
			inline double operator()(int k, int x, int y, double x_offset, double y_offset, T& obj)
			{
				double q = m_L*(m_NNGen->GetMaxChessBoardDist()-k);
				return ( 
					q*(obj.getValue(x,y,x_offset,y_offset) + obj.getValue(x,y,-x_offset,-y_offset)) +
					(obj.getValue(x,y,-x_offset,y_offset) + obj.getValue(x,y,x_offset,-y_offset))
					) / (2*q+2);
			}
		protected:
			double m_L;
			double m_MaxChessBoardDist;
		};

		class TensionFunctionDegree3
		{
		public:
			TensionFunctionDegree3() {}
			void set(double /*max_chess_board_dist*/) {}
			template <class T>
			inline double operator()(int k, int x, int y, double x_offset, double y_offset, T& obj)
			{
				return (obj.getValue(x,y,x_offset,y_offset) + obj.getValue(x,y,-x_offset,-y_offset))/2;
			}
		};

	}
}

#endif