#ifndef PCL_GEOMETRICAL_MEASUREMENT_HELPER
#define PCL_GEOMETRICAL_MEASUREMENT_HELPER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <limits>

namespace pcl
{
	namespace measurement
	{

		struct LongestAxialHelper 
		{
			enum {
				Diameter,
				PerpendicularDiameter,
				Z,
				AngleDifference,
				StartX,
				StartY,
				EndX,
				EndY,
				PerpendicularStartX,
				PerpendicularStartY,
				PerpendicularEndX,
				PerpendicularEndY,
				_Size
			};
		};

		struct LongestDiameterHelper
		{
			enum {
				Diameter,
				StartX,
				StartY,
				StartZ,
				EndX,
				EndY,
				EndZ,
				_Size
			};
		};


		namespace details
		{
			class LongestAxialDiameterCalculator
			{
			public:
				template <class ImagePointer>
				static std::vector<double> Compute(ImagePointer mask)
				{
					if (mask->getSpacing().x()!=mask->getSpacing().y()) pcl_ThrowException(pcl::Exception(), "In plane resolution is not isotropic!");
					int component_count = 0;
					auto image = pcl::ImageAlgorithm::ConnectedComponentAnalysis<pcl::Image<int,true>>(mask, pcl::iterator::ImageNeighborIterator::CreateConnect8Offset(), &component_count);
					std::vector<int> label_item_lookup(component_count+1, -1);
					auto region = image->getRegion();
					//Searching for the longest diameter
					DiameterResult longest_axial, longest_perp;
					pcl::ImageIteratorWithPoint iter(image, pcl::ImageIterator::X, pcl::ImageIterator::Y);
					pcl::iterator::ImageNeighborIterator n_iter(image, pcl::iterator::ImageNeighborIterator::CreateConnect4Offset());
					for (int z=image->getMinPoint().z(); z<=image->getMaxPoint().z(); ++z) {
						//Extracting coordinates of border points
						region.getMinPoint().z() = region.getMaxPoint().z() = z;
						iter.setRegion(region);
						std::vector<VectorPointer> border_points_list;
						pcl_ForIterator(iter) {
							int val = image->get(iter);
							if (val!=0) {
								bool is_border = false;
								n_iter.setOrigin(iter.getPoint());
								pcl_ForIterator(n_iter) if (!image->contain(n_iter.getPoint()) || image->get(n_iter)==0) {
									is_border = true;
									break;
								}
								if (is_border) {
									int label = label_item_lookup[val];
									if (label==-1) {
										border_points_list.push_back(VectorPointer(new std::vector<Point3D<int>>()));
										label_item_lookup[val] = label = border_points_list.size()-1;
									}
									border_points_list[label]->push_back(iter.getPoint());
								}
							}
						}
						StoreLargest<long, InfoPair> largest_storer;
						pcl_ForEach(border_points_list, border_points) if ((*border_points)->size()>1) {
							auto cur_list = _ComputeLongest(**border_points);
							pcl_ForEach(cur_list, item) {
								largest_storer.add(item->length_sqr, InfoPair(*item, *border_points));
							}
						}
						if (largest_storer.getLargestValue()>=longest_axial.length_sqr) {
							pcl_ForEach(largest_storer.getStorage(), item) {
								auto cur_perp = _ComputeLongestPerpendicular(*(item->border_points), item->diameter);
								bool assign = false;
								if (item->diameter.length_sqr>longest_axial.length_sqr) assign = true;
								else if (item->diameter.length_sqr==longest_axial.length_sqr) {
									if (cur_perp.length_sqr>longest_perp.length_sqr) assign = true;
								}
								if (assign) {
									longest_axial = item->diameter;
									longest_perp = cur_perp;
								}
							}
						}
					}
					return _GetResult(longest_axial, longest_perp, mask->getSpacing());
				}

				template <class ImagePointer>
				static std::vector<double> ComputeOriginalVersion(ImagePointer mask)
				{
					if (mask->getSpacing().x()!=mask->getSpacing().y()) pcl_ThrowException(pcl::Exception(), "In plane resolution is not isotropic!");
					int component_count = 0;
					auto image = pcl::ImageAlgorithm::ConnectedComponentAnalysis<pcl::Image<int,true>>(mask, pcl::iterator::ImageNeighborIterator::CreateConnect8Offset(), &component_count);
					std::vector<int> label_item_lookup(component_count+1, -1);
					auto region = image->getRegion();
					//Searching for the longest diameter
					DiameterResult longest_axial, longest_perp;
					pcl::ImageIteratorWithPoint iter(image, pcl::ImageIterator::X, pcl::ImageIterator::Y);
					pcl::iterator::ImageNeighborIterator n_iter(image, pcl::iterator::ImageNeighborIterator::CreateConnect4Offset());
					for (int z=image->getMinPoint().z(); z<=image->getMaxPoint().z(); ++z) {
						//Extracting coordinates of border points
						region.getMinPoint().z() = region.getMaxPoint().z() = z;
						iter.setRegion(region);
						std::vector<VectorPointer> border_points_list;
						pcl_ForIterator(iter) {
							int val = image->get(iter);
							if (val!=0) {
								bool is_border = false;
								n_iter.setOrigin(iter.getPoint());
								pcl_ForIterator(n_iter) if (!image->contain(n_iter.getPoint()) || image->get(n_iter)==0) {
									is_border = true;
									break;
								}
								if (is_border) {
									int label = label_item_lookup[val];
									if (label==-1) {
										border_points_list.push_back(VectorPointer(new std::vector<pcl::Point3D<int>>()));
										label_item_lookup[val] = label = border_points_list.size()-1;
									}
									border_points_list[label]->push_back(iter.getPoint());
								}
							}
						}
						StoreLargest<long, InfoPair> largest_storer;
						pcl_ForEach(border_points_list, border_points) if ((*border_points)->size()>1) {
							auto cur_list = _ComputeLongest(**border_points);
							pcl_ForEach(cur_list, item) {
								largest_storer.add(item->length_sqr, InfoPair(*item, *border_points));
							}
						}
						if (largest_storer.getLargestValue()>=longest_axial.length_sqr) {
							pcl_ForEach(largest_storer.getStorage(), item) {
								//Find and compute perpendicular diameter
								//Based on the code in QIWS Measurements class
								float poffset = 0.25;
								DiameterResult cur_perp;
								for (int i=1; i<9 && cur_perp.length_sqr==0; i=2*i ) {
									auto cur = _ComputeOriginalLongestPerpendicular(poffset*i, *(item->border_points), item->diameter);
									if (cur.length_sqr>cur_perp.length_sqr) cur_perp = cur;
								}

								bool assign = false;
								if (item->diameter.length_sqr>longest_axial.length_sqr) assign = true;
								else if (item->diameter.length_sqr==longest_axial.length_sqr) {
									if (cur_perp.length_sqr>longest_perp.length_sqr) assign = true;
								}
								if (assign) {
									longest_axial = item->diameter;
									longest_perp = cur_perp;
								}
							}
						}
					}
					return _GetResult(longest_axial, longest_perp, mask->getSpacing());
				}

			protected:
				typedef boost::shared_ptr<std::vector<pcl::Point3D<int>>> VectorPointer;

				struct DiameterResult
				{
					long length_sqr;
					pcl::Point3D<int> start, end;

					DiameterResult()
					{
						length_sqr = 0;
					}

					DiameterResult(const pcl::Point3D<int>& a, const pcl::Point3D<int>& b)
					{
						start = a;
						end = b;
						length_sqr = pcl::square(long(a.x()-b.x())) +
							pcl::square(long(a.y()-b.y()));
					}

					double computeLength(const pcl::Point3D<double>& spacing) const
					{
						return std::sqrt(
							pcl::square((start.x()-end.x())*spacing.x()) +
							pcl::square((start.y()-end.y())*spacing.y())
							);
					}
				};

				static std::vector<double> _GetResult(const DiameterResult& longest_axial, const DiameterResult& longest_perp, const pcl::Point3D<double>& spacing)
				{
					std::vector<double> retval(LongestAxialHelper::_Size, 0);
					if (longest_axial.length_sqr==0) return retval;
					retval[LongestAxialHelper::Diameter] = longest_axial.computeLength(spacing);
					retval[LongestAxialHelper::Z] = longest_axial.start.z();
					retval[LongestAxialHelper::StartX] = longest_axial.start.x();
					retval[LongestAxialHelper::StartY] = longest_axial.start.y();
					retval[LongestAxialHelper::EndX] = longest_axial.end.x();
					retval[LongestAxialHelper::EndY] = longest_axial.end.y();

					if (longest_perp.length_sqr==0) return retval;
					retval[LongestAxialHelper::PerpendicularDiameter] = longest_perp.computeLength(spacing);
					retval[LongestAxialHelper::PerpendicularStartX] = longest_perp.start.x();
					retval[LongestAxialHelper::PerpendicularStartY] = longest_perp.start.y();
					retval[LongestAxialHelper::PerpendicularEndX] = longest_perp.end.x();
					retval[LongestAxialHelper::PerpendicularEndY] = longest_perp.end.y();

					auto longest_vec = pcl::Point3D<double>(longest_axial.start-longest_axial.end);
					auto perp_vec = pcl::Point3D<double>(longest_perp.start-longest_perp.end);
					longest_vec.normalize();
					perp_vec.normalize();
					double abs_dot_prod = pcl::abs(perp_vec.getDotProduct(longest_vec));
					retval[LongestAxialHelper::AngleDifference] = std::acos(abs_dot_prod)*180/pcl::PI;
					return retval;
				}

				struct InfoPair
				{
					DiameterResult diameter;
					VectorPointer border_points;

					InfoPair(const DiameterResult& d, const VectorPointer& b)
					{
						diameter = d;
						border_points = b;
					}
				};

				template <class T, class D>
				class StoreLargest
				{
				public:
					StoreLargest()
					{}

					void add(const T& val, const D& data)
					{
						if (m_LargestValue==val || m_Storage.empty()) {
							m_LargestValue = val;
							m_Storage.push_back(data);
						} else if (val>m_LargestValue) {
							m_Storage.clear();
							m_LargestValue = val;
							m_Storage.push_back(data);
						}
					}

					const T& getLargestValue() const
					{
						return m_LargestValue;
					}

					const std::vector<D>& getStorage() const
					{
						return m_Storage;
					}

				protected:
					T m_LargestValue;
					std::vector<D> m_Storage;
				};

				static std::vector<DiameterResult> _ComputeLongest(const std::vector<pcl::Point3D<int>>& border_points)
				{
					StoreLargest<long, DiameterResult> storer;
					for (int i=0; i<border_points.size(); ++i) for (int j=i+1; j<border_points.size(); ++j) {
						DiameterResult cur(border_points[i], border_points[j]);
						storer.add(cur.length_sqr, cur);
					}
					return storer.getStorage();
				}

				static bool _DoubleEqual(double a, double b, double epsilon)
				{
					return pcl::abs(a - b) < epsilon;
				}

				//Tolerance of "perpendicular" angle is set to approximately 45 degree by default
				static DiameterResult _ComputeLongestPerpendicular(const std::vector<pcl::Point3D<int>>& border_points, const DiameterResult& longest_axial, double max_cosine=0.707, double epsilon=std::numeric_limits<double>::epsilon())
				{
					auto longest_vec = pcl::Point3D<double>(longest_axial.start-longest_axial.end);
					longest_vec.normalize();
					//Getting longest and most perpendicular diameter
					DiameterResult result;
					double min_abs_dot_prod;
					for (int i=0; i<border_points.size(); ++i) for (int j=i+1; j<border_points.size(); ++j) {
						auto vec = pcl::Point3D<double>(border_points[i] - border_points[j]);
						vec.normalize();
						double abs_dot_prod = pcl::abs(vec.getDotProduct(longest_vec));
						if (abs_dot_prod<max_cosine) {
							DiameterResult cur(border_points[i], border_points[j]);
							bool assign = false;
							if (result.length_sqr==0) assign = true;
							else if (_DoubleEqual(abs_dot_prod, min_abs_dot_prod, epsilon)) {
								if (cur.length_sqr>result.length_sqr) assign = true;
							} else if (abs_dot_prod<min_abs_dot_prod) assign = true;

							if (assign) {
								result = cur;
								min_abs_dot_prod = abs_dot_prod;
							}
						}
					}
					return result;
				}

				static DiameterResult _ComputeOriginalLongestPerpendicular(float perp_offset, const std::vector<pcl::Point3D<int>>& border_points, const DiameterResult& longest_axial)
				{
					//Based on the code in QIWS Measurements class
					const pcl::Point3D<int> &ml_pt1 = longest_axial.start,
						&ml_pt2 = longest_axial.end;
					bool ml_vert=false;
					float ml_g=0;
					if (ml_pt1.x()!=ml_pt2.x()) ml_g = (float)(ml_pt2.y()-ml_pt1.y())/(ml_pt2.x()-ml_pt1.x());
					else ml_vert = true;
					DiameterResult result;
					float d=0, g=0;
					bool vert, ok;
					for (int i=0; i<border_points.size(); ++i) for (int j=0; j<border_points.size(); ++j) {
						int dx = border_points[i].x() - border_points[j].x(),
							dy = border_points[i].y() - border_points[j].y();
						if (dx!=0) {
							vert=false;
							g = (float)dy/dx;
						} else vert = true;

						if (ml_vert) ok = !vert && (g<=perp_offset) && (g>=(0-perp_offset));
						else if (vert) ok = (ml_g<=perp_offset) && (ml_g>=(0-perp_offset));
						else ok = (ml_g*g>=(-1-perp_offset)) && (ml_g*g<=(-1+perp_offset));

						if (ok) {
							auto cur = DiameterResult(border_points[i], border_points[j]);
							if (cur.length_sqr>result.length_sqr) result = cur;
						}
					}
					return result;
				}
			};
		}
	
		class GeometricalMeasurementHelper 
		{
		public:
			template <class ImagePointer>
			static double ComputeVolume(ImagePointer mask)
			{
				pcl::ImageIteratorWithPoint iter(mask);
				double count = 0;
				pcl_ForIterator(iter) if (mask->get(iter)!=0) count+=1;
				return count*mask->getSpacing()[0]*mask->getSpacing()[1]*mask->getSpacing()[2];
			}

			template <class ImagePointer>
			static double ComputeVoxelCount(ImagePointer mask)
			{
				pcl::ImageIteratorWithPoint iter(mask);
				double count = 0;
				pcl_ForIterator(iter) if (mask->get(iter)!=0) count+=1;
				return count;
			}

			template <class ImagePointer>
			static std::vector<double> ComputeOriginalLongestAxialDiameter(ImagePointer mask)
			{
				return details::LongestAxialDiameterCalculator::ComputeOriginalVersion(mask);
			}
			
			template <class ImagePointer>
			static std::vector<double> ComputeLongestAxialDiameter(ImagePointer mask)
			{
				return details::LongestAxialDiameterCalculator::Compute(mask);
			}
			
			template <class ImagePointer>
			static std::vector<double> ComputeLongestDiameter(ImagePointer mask)
			{
				std::vector<pcl::Point3D<double>> border_points;
				pcl::ImageIteratorWithPoint iter(mask);
				pcl::Point3D<int> start_p, end_p;
				pcl::iterator::ImageNeighborIterator n_iter(mask, pcl::iterator::ImageNeighborIterator::CreateConnect6Offset());
				pcl_ForIterator(iter) if (mask->get(iter)!=0) {
					bool is_border = false;
					n_iter.setOrigin(iter.getPoint());
					pcl_ForIterator(n_iter) if (!mask->contain(n_iter.getPoint()) || mask->get(n_iter)==0) {
						is_border = true;
						break;
					}
					if (is_border) border_points.push_back(mask->toPhysicalCoordinate(iter.getPoint()));
				}
				double max_length_sqr = 0;
				if (!border_points.empty()) {
					for (int i=0; i<border_points.size(); ++i) for (int j=i+1; j<border_points.size(); ++j) {
						double l = border_points[i].getEuclideanDistanceSqr(border_points[j]);
						if (l>max_length_sqr) {
							max_length_sqr = l;
							start_p.assignRound(mask->toImageCoordinate(border_points[i]));
							end_p.assignRound(mask->toImageCoordinate(border_points[j]));
						}
					}
				}
				std::vector<double> retval(LongestDiameterHelper::_Size, 0);
				if (max_length_sqr>0) {
					retval[LongestDiameterHelper::Diameter] = sqrt(max_length_sqr);
					retval[LongestDiameterHelper::StartX] = start_p.x();
					retval[LongestDiameterHelper::StartY] = start_p.y();
					retval[LongestDiameterHelper::StartZ] = start_p.z();
					retval[LongestDiameterHelper::EndX] = end_p.x();
					retval[LongestDiameterHelper::EndY] = end_p.y();
					retval[LongestDiameterHelper::EndZ] = end_p.z();
				}
				return retval;
			}
		};
		
	}
}

#endif