#ifndef PCL_TERRAIN_MAP_TRIANGLE_INFO_FILTER
#define PCL_TERRAIN_MAP_TRIANGLE_INFO_FILTER

#include <pcl/terrain_map/TerrainMap.h>
#include <pcl/iterator/BruteRegionGrowingIterator.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/cod.h>
#include <memory>

namespace pcl
{
	namespace terrain_map
	{

		namespace details
		{

			template <class Type>
			class TriangleInfo 
			{
			public:
				typedef TriangleInfo Self;
				typedef std::unique_ptr<Self> Pointer;
				typedef std::unique_ptr<const Self> ConstantPointer;
				typedef Point3D<Type> PointType;
				typedef Region3D<Type> RegionType;

				static Pointer New(const PointType* points)
				{
					return Pointer(new Self(points));
				}

				inline bool isLine() const
				{ 
					return m_IsLine; 
				}

				inline bool contain(const PointType& p) const 
				{
					for (int i=0; i<3; ++i) {
						double val = (p-m_LineOrigin[i]).getDotProduct(m_LineNormal[i]);
						if (val<0 && m_Sign[i]>0) return false;
						else if (val>0 && m_Sign[i]<0) return false;
					}
					return true;
				}

				inline double getOffset(const PointType& p) const
				{ 
					return (p-m_LineOrigin[0]).getDotProduct(m_Normal); 
				}

				std::vector<PointType> getPlaneIntersectPoints(double slice, int axis) const //Returns the number of intersecting points (Should normally expect 2 points!)
				{
					if (!intersectPlane(slice, axis)) return std::vector<PointType>();
					std::vector<PointType> result;
					int intersect_count = 0;
					for (int i=0; i<3; i++) {
						PointType p;
						if (pcl::abs(m_LineDirection[i][axis])>pcl::Epsilon) {
							double l = (slice-m_LineOrigin[i][axis])/m_LineDirection[i][axis];
							if (l>=-1 && l<=1) { //Check whether the point is within edge
								p = m_LineDirection[i];
								p *= l;
								p += m_LineOrigin[i];
								result.push_back(p);
								intersect_count++;
								if (intersect_count==2) break;
							}
						}
					}
					return std::move(result);
				}

				inline const PointType* getPoints() const
				{ 
					return m_LineOrigin; 
				}

				inline const PointType& getNormal() const
				{ 
					return m_Normal; 
				}

				inline const PointType& getCentroid() const
				{
					return m_Centroid;
				}

				inline const RegionType& getRegion() const
				{
					return m_Region;
				}

			protected:
				static int m_NextPoint[3];
				PointType m_LineOrigin[3], 
					m_LineDirection[3],
					m_LineNormal[3],
					m_Normal,
					m_Centroid;
				RegionType m_Region;
				char m_Sign[3];
				bool m_IsLine;

				TriangleInfo(const PointType* points) 
				{
					PointType dir1, 
						dir2;
					dir1 = points[0]-points[1]; dir1.normalize();
					dir2 = points[0]-points[2]; dir2.normalize();
					m_Normal = dir1; m_Normal.setCrossProduct(dir2);
					if ((m_Normal[0]+m_Normal[1]+m_Normal[2])<=pcl::Epsilon) { //Test to see whether the points are on a line
						m_IsLine = true;
					} else {
						m_IsLine = false;
						m_Normal.normalize();
						for (int i=0; i<3; ++i) m_LineOrigin[i] = points[i];
						for (int i=0; i<3; ++i) {
							PointType temp = points[m_NextPoint[i]]-points[i];
							m_LineDirection[i] = temp;
							temp.normalize();
							m_LineNormal[i] = m_Normal; m_LineNormal[i].setCrossProduct(temp);
							m_LineNormal[i].normalize();
						}
						//Computing the inside sign for the lines
						PointType centroid = (points[0] + points[1] + points[2])/3.; //Compute centroid
						for (int i=0; i<3; ++i) if ((centroid-m_LineOrigin[i]).getDotProduct(m_LineNormal[i])<0) m_Sign[i] = -1;
						else m_Sign[i] = 1;
					}
					//Getting centroid and bounding box
					m_Region.reset();
					m_Centroid.set(0,0,0);
					for (int i=0; i<3; ++i) {
						m_Region.add(points[i]);
						m_Centroid += points[i];
					}
					m_Centroid /= 3;
				}

				bool intersectPlane(double slice, int axis) const
				{
					if (m_Region.getMinPoint()[axis]>slice || m_Region.getMaxPoint()[axis]<slice) return false;
					return true;
				}
			};
			template <class Type>
			int TriangleInfo<Type>::m_NextPoint[3] = {1, 2, 0};

		}


		using namespace pcl::terrain_map::details;

		class TerrainMapTriangleInfo {
		public:
			typedef TriangleInfo<double> TriangleInfoType;

			TerrainMapTriangleInfoGenerator() {}
			template <class TerrainMapPointer>
			TerrainMapTriangleInfoGenerator(const TerrainMapPointer& terrain) 
			{
				setTerrainMap(terrain);
				m_RegionSplitNum = Point3D<int>(20,20,20);
			}

			template <class TerrainMapPointer>
			void setTerrainMap(const TerrainMapPointer& terrain)
			{
				m_Storage.clear(); m_Storage.reserve(1000);
				//Compute offset
				long offset_xy = terrain->getOffsetTable()[0]+terrain->getOffsetTable()[1],
					offset_x = terrain->getOffsetTable()[0],
					offset_y = terrain->getOffsetTable()[1];
				//Constructing triangle info of terrain
				Point3D<int> max_point(terrain->getMaxPoint());
				max_point -= 1;
				max_point.max(terrain->getMinPoint());
				ImageIteratorWithPoint iter(terrain);
				iter.setRegion(terrain->getMinPoint(), max_point);
				Point3D<double> p[3];
				pcl_ForIterator(iter) {
					p[0] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x(), iter.getPoint().y(), iter);
					p[1] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x()+1, iter.getPoint().y(), static_cast<long>(iter)+offset_x);
					p[2] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x()+1, iter.getPoint().y()+1, static_cast<long>(iter)+offset_xy);
					addTriangle(p);

					p[0] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x()+1, iter.getPoint().y()+1, static_cast<long>(iter)+offset_xy);
					p[1] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x(), iter.getPoint().y()+1, static_cast<long>(iter)+offset_y);
					p[2] = terrain->getTerrainPhysicalCoordinate(iter.getPoint().x(), iter.getPoint().y(), iter);
					addTriangle(p);
				}
			}

			void setRegionSplitNum(int x, int y, int z)
			{
				m_RegionSplitNum.set(x,y,z);
			}

			/********** Image related **********/

			//template <class ImagePointer, class MaskPointer>
			//void fillImage(const ImagePointer& image, const MaskPointer& mask, const typename ImagePointer::element_type::IoValueType& val_positive=1, const typename ImagePointer::element_type::IoValueType& val_negative=2) const
			//{
			//	std::vector<Region3D<int>> regions = image->getRegion().getSplitRegion(m_RegionSplitNum[0], m_RegionSplitNum[1], m_RegionSplitNum[2]); //Spliting the image into sub regions
			//	pcl_ForEach(regions, sub_region) {
			//		Region3D<double> physical_region = image->getPhysicalRegion(*sub_region);
			//		std::vector<const TriangleInfoType*> triangle_info = getTrianglesWithinRegion(physical_region);
			//		fillImage(image, mask, *sub_region, triangle_info, val_positive, val_negative);
			//	}
			//}
			/*template <class ImagePointer, class MaskPointer>
			void fillImage(const ImagePointer& image, const MaskPointer& mask, const typename ImagePointer::element_type::IoValueType& val_positive=1, const typename ImagePointer::element_type::IoValueType& val_negative=2) const
			{
				enum { POSITIVE=1, NEGATIVE=2, MIXED=3 };
				auto map = pcl::Image<int>::New(m_RegionSplitNum);
				auto marker = pcl::Image<char>::New(m_RegionSplitNum);
				ImageHelper::Fill(marker, 0);
				std::vector<Region3D<int>> sub_region = image->getRegion().getSplitRegionWithMap(map); //Spliting the image into sub regions
				//Setting up region growing iterators
				auto offset_list = iterator::ImageNeighborIterator::CreateConnect26Offset();
				iterator::BruteRegionGrowingIterator mix_rgn_iter, pure_rgn_iter;
				mix_rgn_iter.setNeighborIterator(offset_list, map);
				pure_rgn_iter.setNeighborIterator(offset_list, map);
				//Finding the first region where the surface crosses as seed
				{
					ImageIteratorWithPoint map_iter(map);
					pcl_ForIterator(map_iter) if (map->get(map_iter)>=0) {
						const auto& region = sub_region[map->get(map_iter)];
						const auto& physical_region = image->getPhysicalRegion(region);
						std::vector<const TriangleInfoType*> triangle_info = getTrianglesWithinRegion(physical_region);
						if (!triangle_info.empty()) {
							fillImage(image, mask, region, triangle_info, val_positive, val_negative);
							marker->set(map_iter, MIXED);
							mix_rgn_iter.addSeed(map_iter.getPoint(), map_iter);
							break;
						}
					}
				}
				//Use region growing to search for mixed region (region where surface crosses)
				pcl_ForIterator(mix_rgn_iter) if (map->get(mix_rgn_iter)>=0 && marker->get(mix_rgn_iter)==0) {
					const auto& region = sub_region[map->get(mix_rgn_iter)];
					const auto& physical_region = image->getPhysicalRegion(region);
					std::vector<const TriangleInfoType*> triangle_info = getTrianglesWithinRegion(physical_region);
					if (triangle_info.empty()) {
						auto offset = mix_rgn_iter.getOffset();
						if (pcl_Round(offset.x())+pcl_Round(offset.y())+pcl_Round(offset.z())==1) { //Only performs test on immediate neighbors (will have errors if this is not enforced!)
							const auto& source_region = sub_region[map->get(mix_rgn_iter.getSourceIndex())];
							const auto& source_physical_region = image->getPhysicalRegion(source_region);
							std::vector<const TriangleInfoType*> source_triangle_info = getTrianglesWithinRegion(source_physical_region);
							bool is_positive = centroidBasedFillImage(image, mask, region, source_triangle_info, val_positive, val_negative);
							if (is_positive) {
								marker->set(mix_rgn_iter, POSITIVE);
							} else {
								marker->set(mix_rgn_iter, NEGATIVE);
							}
							pure_rgn_iter.addSeed(mix_rgn_iter.getPoint(), mix_rgn_iter);
						}
					} else {
						fillImage(image, mask, region, triangle_info, val_positive, val_negative);
						marker->set(mix_rgn_iter, MIXED);
						mix_rgn_iter.accept();
					}
				} else if (map->get(mix_rgn_iter)>=0 && marker->get(mix_rgn_iter)!=MIXED) {
					auto offset = mix_rgn_iter.getOffset();
					if (pcl_Round(offset.x())+pcl_Round(offset.y())+pcl_Round(offset.z())==1) {
						const auto& region = sub_region[map->get(mix_rgn_iter)];
						const auto& source_region = sub_region[map->get(mix_rgn_iter.getSourceIndex())];
						const auto& source_physical_region = image->getPhysicalRegion(source_region);
						std::vector<const TriangleInfoType*> source_triangle_info = getTrianglesWithinRegion(source_physical_region);
						bool is_positive = centroidBasedFillImage(image, mask, region, source_triangle_info, val_positive, val_negative);
						if ((is_positive?POSITIVE:NEGATIVE) != marker->get(mix_rgn_iter)) {
							std::cout << "Error sign changed occured at " << region << std::endl;
							exit(1);
						}
					}
				}
				//Propagating the pure regions to the rest of the other regions
				ImageIterator image_iter(image);
				pcl_ForIterator(pure_rgn_iter) if (map->get(pure_rgn_iter)>=0 && marker->get(pure_rgn_iter)==0) {
					char source_label = marker->get(pure_rgn_iter.getSourceIndex());
					image_iter.setRegion(sub_region[map->get(pure_rgn_iter)]);
					if (source_label==POSITIVE) {
						pcl_ForIterator(image_iter) if (mask->get(image_iter)) image->set(image_iter, val_positive);
					} else {
						pcl_ForIterator(image_iter) if (mask->get(image_iter)) image->set(image_iter, val_negative);
					}
					marker->set(pure_rgn_iter, source_label);
					pure_rgn_iter.accept();
				}
			}
			template <class ImagePointer, class MaskPointer>
			void fillImage(const ImagePointer& image, const MaskPointer& mask, const Region3D<int>& requested_region, const std::vector<const TriangleInfoType*>& triangle_info, const typename ImagePointer::element_type::IoValueType& val_positive=1, const typename ImagePointer::element_type::IoValueType& val_negative=2) const
			{
				if (triangle_info.empty()) {
					//Making decision based on centroid of the region, as it does not intersect the terrain surface
					Region3D<double> physical_region = image->getPhysicalRegion(requested_region);
					Point3D<double> centroid(physical_region.getMaxPoint());
					centroid -= physical_region.getMinPoint();
					centroid *= 0.5;
					centroid += physical_region.getMinPoint();
					int sign = getSign(centroid, m_Storage);
					const typename ImagePointer::element_type::IoValueType *fill_val = &val_positive;
					if (sign<0) fill_val = &val_negative;
					ImageIterator iter(image);
					iter.setRegion(requested_region);
					pcl_ForIterator(iter) if (mask->get(iter)) image->set(iter, *fill_val);
				} else {
					ImageIteratorWithPoint iter(image);
					iter.setRegion(requested_region);
					pcl_ForIterator(iter) if (mask->get(iter)) {
						int sign = getSign(image->toPhysicalCoordinate(iter.getPoint()), triangle_info);
						if (sign<0) image->set(iter, val_negative);
						else image->set(iter, val_positive);
					}
				}
			}
			template <class ImagePointer, class MaskPointer>
			bool centroidBasedFillImage(const ImagePointer& image, const MaskPointer& mask, const Region3D<int>& requested_region, const std::vector<const TriangleInfoType*>& triangle_info, const typename ImagePointer::element_type::IoValueType& val_positive=1, const typename ImagePointer::element_type::IoValueType& val_negative=2) const
			{
				if (triangle_info.empty()) pcl_ThrowException(pcl::Exception(), "Triangle info is empty");
				Region3D<double> physical_region = image->getPhysicalRegion(requested_region);
				Point3D<double> centroid(physical_region.getMaxPoint());
				centroid -= physical_region.getMinPoint();
				centroid *= 0.5;
				centroid += physical_region.getMinPoint();
				int sign = getSign(centroid, triangle_info);
				const typename ImagePointer::element_type::IoValueType *fill_val = &val_positive;
				if (sign<0) fill_val = &val_negative;
				ImageIterator iter(image);
				iter.setRegion(requested_region);
				pcl_ForIterator(iter) if (mask->get(iter)) image->set(iter, *fill_val);
				return sign>=0;
			}*/

			// template <class ImagePointer>
			// void drawSurface(const ImagePointer& image, const typename ImagePointer::element_type::IoValueType& val=1, std::vector<long>* drawn_index=NULL) const
			// {
				// std::vector<Region3D<int>> regions = image->getRegion().getSplitRegion(m_RegionSplitNum[0], m_RegionSplitNum[1], m_RegionSplitNum[2]); //Spliting the image into sub regions
				// pcl_ForEach(regions, sub_region) {
					// Region3D<double> physical_region = image->getPhysicalRegion(requested_region);
					// std::vector<const TriangleInfo*> triangle_info = getTrianglesWithinRegion(physical_region);
					// drawSurface(image, *sub_region, triangle_info, val, drawn_index);
				// }
			// }
			// template <class ImagePointer>
			// void drawSurface(const ImagePointer& image, const Region3D<int>& requested_region, const std::vector<const TriangleInfoType*>& triangle_info, const typename ImagePointer::element_type::IoValueType& val=1, std::vector<long>* drawn_index=NULL) const
			// {
				// if (triangle_info.empty()) return;
				// Point3D<double> sub_spacing = image->getSpacing()*0.5;
				// ImageIteratorWithPoint iter(image);
				// pcl_ForEach(triangle_info, item) {
					// const auto& triangle_region = item->getRegion();
					// const auto& intermediate_image_region = item->getImageRegionFromPhysical(triangle_region);
					// Region3D<int> image_region;
					// image_region.getMinPoint().assignCustom(intermediate_image_region.getMinPoint(), [](double val)->int { 
						// return static_cast<int>(floor(val)); 
					// });
					// image_region.getMaxPoint().assignCustom(intermediate_image_region.getMaxPoint(), [](double val)->int { 
						// return static_cast<int>(ceil(val)); 
					// });
					// image_region.setIntersect(requested_region);
					// iter.setRegion(image_region);
					// pcl_ForIterator(iter) if (image->get(iter)!=val) {
						// const auto& physical_coord = image->toPhysicalCoordinate(iter.getPoint());
						// if ((*item)->contain(physical_coord)) {
							// if (isWithinVoxel(image, (*item)->getOffset(phsycial_coord), (*item)->getNormal(), sub_spacing)) {
								// image->set(iter, val);
								// if (drawn_index) drawn_index->push_back(iter);
							// }
						// }
					// }
				// }
			// }

			/********** Miscellaneous helpers **********/

			std::vector<const TriangleInfoType*> getTrianglesWithinRegion(const Region3D<double>& region) const
			{
				std::vector<const TriangleInfoType*> result;
				pcl_ForEach(m_Storage, item) if ((*item)->getRegion().intersect(region)) {
					result.push_back(item->get());
				}
				return std::move(result);
			}
			
			template <class ImagePointer>
			inline typename boost::disable_if_c<ImagePointer::element_type::UseOrientationMatrix, bool>::type isWithinVoxel(const ImagePointer&, double offset, const Point3D<double>& normal, const Point3D<double>& sub_spacing) const
			{
				for (int i=0; i<3; ++i) if (pcl::abs(normal[i]*offset) > sub_spacing[i]) return false;
				return true;
			}
			template <class ImagePointer>
			inline typename boost::enable_if_c<ImagePointer::element_type::UseOrientationMatrix, bool>::type isWithinVoxel(const ImagePointer& image, double offset, const Point3D<double>& normal, const Point3D<double>&) const
			{
				auto disp = normal;
				disp *= offset;
				disp = image->toImageVector(disp);
				for (int i=0; i<3; ++i) if (pcl::abs(disp[i])>0.5) return false;
				return true;
			}

			// template <class TriangleVectorType>
			// int getSign(const Point3D<double>& p, const TriangleVectorType& vec, double epsilon=pcl::Epsilon) const
			// {
				// std::vector<int> nearest_triangle_offset;
				// double min_dist_square = std::numeric_limits<double>::infinity();
				// pcl_ForEach(vec, item) {
					// double dist_square = p.getEuclideanDistanceSqr((*item)->getCentroid());
					// if (doubleEqual(dist_square, min_dist_square, epsilon)) {
						// nearest_triangle_offset.push_back((*item)->getOffset(p)<0?-1:1);
						// min_dist_square = (min_dist_square+dist_square)*0.5; //Setting the min distance to the average
					// } else if (dist_square<min_dist_square) {
						// nearest_triangle_offset.clear();
						// nearest_triangle_offset.push_back((*item)->getOffset(p)<0?-1:1);
						// min_dist_square = dist_square;
					// }
				// }
				// int result = 0;
				// pcl_ForEach(nearest_triangle_offset, item) {
					// result += *item;
				// }
				// return result<0?-1:1;
			// }

		protected:
			std::vector<TriangleInfoType::ConstantPointer> m_Storage;
			Point3D<int> m_RegionSplitNum;

			void addTriangle(const Point3D<double>* points) 
			{
				auto info = TriangleInfoType::New(points);
				if (!info->isLine()) m_Storage.push_back(std::move(info));
			}
		}; 

	}
}

#endif