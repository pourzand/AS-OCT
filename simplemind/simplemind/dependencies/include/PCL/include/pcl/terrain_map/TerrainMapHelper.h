#ifndef PCL_TERRAIN_MAP_HELPER
#define PCL_TERRAIN_MAP_HELPER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/geometry.h>
#include <pcl/misc/CovarianceMatrixGenerator.h>
#include <pcl/constant.h>
#include <pcl/type_utility.h>
#include <pcl/misc/VnlSymmetricEigenSystemWrapper.h>
#include <pcl/math.h>
#include <pcl/filter/interpolator/BilinearInterpolator.h>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	using namespace pcl::misc;

	namespace terrain_map_helper_details
	{
		static const int m_CheckIndex[2][4]= { 
			{1,3,2,0}, 
			{0,2,1,3} 
		};
		static const int m_XOffset[4] = {-1, 0, 1, 0},
			m_YOffset[4] = {0, 1, 0, -1};
	}

	class TerrainMapHelper
	{
	public:		
		/****************************************** Misc functions ******************************************/
		template <class TerrainMapPointerType>
		static Region3D<double> GetTerrainMapPhysicalRegion(const TerrainMapPointerType& map)
		{
			ImageIteratorWithPoint iter(map);
			Region3D<double> region;
			region.reset();
			pcl_ForIterator(iter) {
				region.add(map->toPhysicalCoordinate(map->getCoordinate(iter)));
			}
			return region;
		}

		template <class TerrainMapPointerType>
		static Point3D<double> GetPhysicalNormalAt(int x, int y, const TerrainMapPointerType& map) 
		{
			return GetPhysicalNormalAt(x,y, map->localToIndex(x,y,map->getMinPoint().z()), map);
		}
		template <class TerrainMapPointerType>
		static Point3D<double> GetPhysicalNormalAt(int x, int y, long index, const TerrainMapPointerType& map)
		{
			Point3D<double> &origin = map->toPhysicalCoordinate(x,y,0); //Note: The computation is independent of the height of the origin
			int z = map->getMinPoint().z();
			//Obtaining neighbors
			Point3D<double> neighbor[4];
			const long* offset_table = map->getOffsetTable();
			for (int i=0; i<4; i++) {
				int cur_x = x+terrain_map_helper_details::m_XOffset[i],
					cur_y = y+terrain_map_helper_details::m_YOffset[i];
				if (map->contain(cur_x, cur_y, z)) {
					neighbor[i] = map->toPhysicalCoordinate(getCoordinate(cur_x, cur_y, index + terrain_map_helper_details::m_XOffset[i]*offset_table[0] + terrain_map_helper_details::m_YOffset[i]*offset_table[1]));
					neighbor[i] -= origin;
				} else {
					neighbor[i].x() = std::numeric_limits<double>::infinity();
				}
			}
			//Computing the normal
			Point3D<double> normal(0,0,0);
			for (int i=0; i<4; i++) {
				Point3D<double> &p1 = neighbor[terrain_map_helper_details::m_CheckIndex[0][i]],
					&p2 = neighbor[terrain_map_helper_details::m_CheckIndex[1][i]];
				if (p1.x()!=std::numeric_limits<double>::infinity() && p2.x()!=std::numeric_limits<double>::infinity()) {
					normal += p1.getCrossProduct(p2);
				}
			}
			return normal.normalize();
		}

		/****************************************** Image related ******************************************/
	public:

		template <class ImageType, class TerrainMapPointerType>
		static typename ImageType::Pointer CreateImageFromTerrain(const TerrainMapPointerType& map, int border_size=1, Point3D<double> spacing=Point3D<double>(0,0,0))
		{
			if (spacing.x()==0) {
				double min_spacing = map->getSpacing()[0];
				for (int i=1; i<3; i++) min_spacing = std::min(min_spacing, map->getSpacing()[i]);
				spacing.set(min_spacing, min_spacing, min_spacing);
			}
			Region3D<double> &physical_region = GetTerrainMapPhysicalRegion(map);
			physical_region.add(physical_region.getMinPoint()-(spacing*border_size));
			physical_region.add(physical_region.getMaxPoint()+(spacing*border_size));
			//std::cout << physical_region << std::endl;
			Point3D<int> size(
				std::ceil(physical_region.getSize(physical_region.X)/spacing.x()),
				std::ceil(physical_region.getSize(physical_region.Y)/spacing.y()),
				std::ceil(physical_region.getSize(physical_region.Z)/spacing.z())
				);
			//std::cout << size << std::endl;
			auto result = ImageType::New(size, spacing, physical_region.getMinPoint());
			ImageHelper::Fill(result, 0);
			ImageIteratorWithPoint iter(map);
			Point3D<int> pos;
			pcl_ForIterator(iter) {
				pos.assignRound(result->toImageCoordinate(map->toPhysicalCoordinate(getCoordinate(iter))));
				result->set(pos, 1);
			}
			return result;
		}

		template <class ImagePointerType, class TerrainMapPointerType>
		static void FillImageWithTerrain(const ImagePointerType& image, const TerrainMapPointerType& map, const typename ptr_base_type<ImagePointerType>::type::IoValueType& val)
		{
			ImageIteratorWithPoint iter(map);
			Point3D<int> pos;
			pcl_ForIterator(iter) {
				pos.assignRound(image->toImageCoordinate(map->toPhysicalCoordinate(map->getCoordinate<pcl::Point3D<double>>(iter.getPoint(), iter))));
				if (image->contain(pos)) image->set(pos, val);
			}
		}

		template <class ImagePointerType, class TerrainMapPointerType, class MaskPointerType>
		static void SeparateImageWithTerrain(const ImagePointerType& image, const TerrainMapPointerType& map, const MaskPointerType& mask, 
			const typename ptr_base_type<ImagePointerType>::type::IoValueType& val_positive=1, const typename ptr_base_type<ImagePointerType>::type::IoValueType& val_negative=2) 
		{
			auto transformation = ImageHelper::GetImageCoordinateTransformation(image, map);
			ImageIteratorWithPoint iter(image);
			pcl::filter::BilinearInterpolator<typename ptr_base_type<TerrainMapPointerType>::type,0,1> interpolator(map);
			int z = map->getMinPoint().z();
			pcl_ForIterator(iter) if (mask->get(iter)) {
				auto& map_p = transformation->toTransformed<Point3D<double>>(iter.getPoint());
				double disp = map_p.z();
				map_p.z() = z;
				if (interpolator.get(map_p)<disp) image->set(iter, val_negative);
				else image->set(iter, val_positive);
				
				/*
				if (0<disp) image->set(iter, val_negative);
				else image->set(iter, val_positive);
				*/
			}
		}

		/****************************************** Creation related ******************************************/
		template <class TerrainMapType>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const Point3D<double>& x_vector, const Point3D<double>& y_vector, const Point3D<double>& z_vector, const Point3D<double>& spacing, const Point3D<double>& origin)
		{
			return Create<TerrainMapType>(
				region, 
				TerrainMapType::ConstructBasisMatrixFromUnitVectors(x_vector, y_vector, z_vector),
				spacing,
				origin
				);
		}

		struct DummyNormalAlignmentFunctor
		{
			void operator()(Point3D<double>& /*normal*/) {}
		};

		template <class TerrainMapType, class PointList>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const PointList& points, const Point3D<double>& spacing)
		{
			return Create<TerrainMapType>(region, points, spacing, DummyNormalAlignmentFunctor());
		}
		template <class TerrainMapType, class PointList, class NormalAlignmentFunctor>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const PointList& points, const Point3D<double>& spacing, NormalAlignmentFunctor align_normal)
		{
			CovarianceMatrixGenerator<3> cov_gen;
			pcl_ForEach(points, item) {
				cov_gen.add(&(*item)[0]);
			}
			return Create<TerrainMapType>(region, points, cov_gen, spacing, align_normal);
		}

		template <class TerrainMapType, class PointList>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const PointList& points, const CovarianceMatrixGenerator<3>& cov_gen, const Point3D<double>& spacing)
		{
			return Create<TerrainMapType>(region, points, cov_gen, spacing, DummyNormalAlignmentFunctor());
		}
		template <class TerrainMapType, class PointList, class NormalAlignmentFunctor>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const PointList& points, const CovarianceMatrixGenerator<3>& cov_gen, const Point3D<double>& spacing, NormalAlignmentFunctor align_normal)
		{
			vnl_matrix_fixed<double,3,3> cov = cov_gen.getCovarianceMatrix<double>();
			//std::cout << "Covariance\n" << cov << std::endl;
			//std::cout << "Centroid: " << cov_gen.getCentroid<Point3D<double> >().toString() << std::endl;

			//std::cout << cov << std::endl;
			try {
				VnlSymmetricEigenSystemWrapper<double> eigen(cov);
				//std::cout << "Eigen\n";
				//std::cout << eigen.D << std::endl;
				//std::cout << eigen.V << std::endl;
				//Computing terrain map info
				int min_index = 0;
				for (int i=1; i<3; i++) {
					if (pcl_Abs(eigen->get_eigenvalue(min_index))>pcl_Abs(eigen->get_eigenvalue(i))) min_index = i;
				}
				Point3D<double> normal, plane_axis;
				normal = getPoint(min_index, eigen->V);
				align_normal(normal);
				for (int i=0; i<3; i++) if (i!=min_index) {
					plane_axis = getAligned(normal, getPoint(i, eigen->V));
					break;
				}
				//std::cout << "Normal: " << normal.toString() << std::endl;
				//std::cout << "Plane axis: " << plane_axis.toString() << std::endl;
				typename TerrainMapType::OrientationMatrixType basis_matrix = TerrainMapType::ConstructOrientationMatrixFromUnitVectors(plane_axis, normal.getCrossProduct(plane_axis), normal);
				Point3D<double> origin = cov_gen.getCentroid<Point3D<double> >();
				//std::cout << "region " << region << std::endl;
				//std::cout << " origin" << origin << std::endl;
				double min_height, max_height;
				getMinMaxHeight(min_height, max_height, points, basis_matrix, origin);
				return Create<TerrainMapType>(
					region, 
					basis_matrix,
					spacing,
					origin,
					min_height,
					max_height
					);
			} catch (const pcl::InvalidValueException&) {
				pcl_ThrowException(pcl::Exception(), "Error encountered when trying to compute the following covariance matrix: " + boost::lexical_cast<std::string>(cov));
			}
		}

		template <class TerrainMapType>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const Point3D<double>& normal, const Point3D<double>& spacing, const Point3D<double>& origin, bool full_coverage=false)
		{
			Point3D<double> plane_axis = normal.getPerpendicularVector();
			plane_axis = getAligned(normal, plane_axis);
			auto basis_matrix = TerrainMapType::ConstructOrientationMatrixFromUnitVectors(plane_axis, normal.getCrossProduct(plane_axis), normal);
			double min_height = 0, 
				max_height = 0;
			if (full_coverage) {
				auto points = region.getCornerPoints();
				getMinMaxHeight(min_height, max_height, points, basis_matrix, origin);
			}
			return Create<TerrainMapType>(
				region, 
				basis_matrix,
				spacing,
				origin,
				min_height,
				max_height
				);
		}

		template <class TerrainMapType>
		static typename TerrainMapType::Pointer Create(const Region3D<double>& region, const typename TerrainMapType::OrientationMatrixType& basis_matrix, const Point3D<double>& spacing, const Point3D<double>& origin, double min_height=0, double max_height=0)
		{
			Region3D<double> terrain_region;
			terrain_region.reset();
			double min_spacing = spacing[0];
			for (int i=1; i<3; i++) min_spacing = std::min(min_spacing, spacing[i]);
			terrain_region.add(getTerrainMapMinMax(min_height, region, basis_matrix, origin, min_spacing/2));
			if (min_height!=max_height) terrain_region.add(getTerrainMapMinMax(max_height, region, basis_matrix, origin, min_spacing/2));
			auto corner_points = region.getCornerPoints();
			pcl_ForEach(corner_points, item) {
				Point3D<double> terrain_coord = *item;
				convertToTerrainMapCoordinate(terrain_coord, basis_matrix, origin);
				if (terrain_coord.z()>min_height && terrain_coord.z()<max_height) terrain_region.add(getTerrainMapMinMax(terrain_coord.z(), region, basis_matrix, origin, min_spacing/2));
			}
			Point3D<double> terrain_origin = origin;
			convertToTerrainMapCoordinate(terrain_origin, basis_matrix, origin);
			Point3D<int> minp, maxp;
			for (int i=0; i<3; i++) {
				minp[i] = floor((terrain_region.getMinPoint()[i]-terrain_origin[i])/spacing[i]);
				maxp[i] = ceil((terrain_region.getMaxPoint()[i]-terrain_origin[i])/spacing[i]);
			}
			//Setting up the terrain map
			typename TerrainMapType::Pointer result = TerrainMapType::New(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, basis_matrix, minp, spacing, origin);
			result = result->getAlias(Point3D<int>(0,0,0), true); //Set min point to (0,0,0) by default
			return result;
		}

	protected:
		template <class BasisMatrixType>
		static Point3D<double> getPoint(int column, const BasisMatrixType& mat)
		{
			return Point3D<double>(mat(0,column), mat(1,column), mat(2,column)).normalize();
		}

		static Point3D<double> getAligned(const Point3D<double>& axis, const Point3D<double>& vec)
		{
			double max_val = -1;
			int max_axis = 0;
			for (int i=0; i<3; ++i) {
				Point3D<double> major_axis(0,0,0);
				major_axis[i] = 1;
				double test_val = pcl::abs(axis.getDotProduct(major_axis));
				if (test_val>max_val) {
					max_val = test_val;
					max_axis = i;
				}
			}
			Point3D<double> major_axis(0,0,0);
			for (int i=0; i<3; ++i) if (i!=max_axis) {
				major_axis[i] = 1;
				break;
			}
			geometry::VectorAlignment alignment(axis, vec);
			alignment.compute(major_axis);
			return alignment.getAlignedVector();
		}

		template <class BasisMatrixType>
		static void convertToTerrainMapCoordinate(Point3D<double>& p, const BasisMatrixType& basis_matrix, const Point3D<double>& origin)
		{
			Point3D<double> temp(p.x(), p.y(), p.z());
			temp -= origin;
			p.set(
				temp[0]*basis_matrix.get(0,0) + temp[1]*basis_matrix.get(1,0) + temp[2]*basis_matrix.get(2,0),
				temp[0]*basis_matrix.get(0,1) + temp[1]*basis_matrix.get(1,1) + temp[2]*basis_matrix.get(2,1),
				temp[0]*basis_matrix.get(0,2) + temp[1]*basis_matrix.get(1,2) + temp[2]*basis_matrix.get(2,2)
				);
		}
		template <class PointList, class BasisMatrixType>
		static void getMinMaxHeight(double &min, double &max, const PointList& points, const BasisMatrixType& basis_matrix, const Point3D<double>& origin)
		{
			Point3D<double> p; p.assign(*(points.begin()));
			convertToTerrainMapCoordinate(p, basis_matrix, origin);
			min = p.z(); max = p.z();
			pcl_ForEach(points, item) {
				p = *item;
				convertToTerrainMapCoordinate(p, basis_matrix, origin);
				if (min>p.z()) min = p.z();
				if (max<p.z()) max = p.z();
			}
		}

		static bool getPlaneIntersection(Point3D<double> &intersect, const Point3D<double>& n1, const Point3D<double>& x1, const Point3D<double>& n2, const Point3D<double>& x2, const Point3D<double>& n3, const Point3D<double>& x3)
		{
			vnl_matrix_fixed<double,3,3> mat;
			mat.set_column(0, &n1[0]);
			mat.set_column(1, &n2[0]);
			mat.set_column(2, &n3[0]);
			double det = vnl_determinant<double>(mat);
			if (det<=pcl::Epsilon) return false;
			det = 1/det;
			intersect = n2.getCrossProduct(n3)*(x1.getDotProduct(n1));
			intersect += n3.getCrossProduct(n1)*(x2.getDotProduct(n2));
			intersect += n1.getCrossProduct(n2)*(x3.getDotProduct(n3));
			intersect *= det;
			return true;
		}
		template <class BasisMatrixType>
		static Region3D<double> getTerrainMapMinMax(double height, const Region3D<double>& region, const BasisMatrixType& basis_matrix, const Point3D<double>& origin, double epsilon)
		{
			Region3D<double> return_region;
			return_region.reset();
			const Point3D<double> normal = getPoint(2, basis_matrix);
			Point3D<double> region_n[3];
			region_n[0].set(1,0,0); 
			region_n[1].set(0,1,0);
			region_n[2].set(0,0,1);
			Point3D<double> range[] = { region.getMinPoint(), region.getMaxPoint() };

			Point3D<double> terrain_x(origin);
			terrain_x += normal*height;
			for (int i=0; i<2; i++) for (int j=0; j<3; j++) {
				Point3D<double> plane1_x(0,0,0);
				plane1_x[j] = range[i][j];
				for (int m=0; m<2; m++) for (int n=0; n<3; n++) {
					Point3D<double> plane2_x(0,0,0);
					plane2_x[n] = range[m][n];
					Point3D<double> p;
					if (getPlaneIntersection(p, normal, terrain_x, region_n[j], plane1_x, region_n[n], plane2_x)) {
						if (region.containEpsilon(p, epsilon)) {
							convertToTerrainMapCoordinate(p, basis_matrix, origin);
							return_region.add(p);
						}
					}
				}
			}

			return return_region;
		}
	};



}

#endif