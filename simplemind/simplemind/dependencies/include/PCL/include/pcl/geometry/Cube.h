#ifndef PCL_CUBE
#define PCL_CUBE

#include <pcl/geometry/Line3D.h>
#include <pcl/geometry/Plane3D.h>
#include <pcl/math.h>
#include <vector>
#include <queue>

namespace pcl
{
	namespace geometry
	{

		template <class VT>
		class Cube
		{
		public:
			template <class T, class F>
			static typename std::vector<Plane3D<VT>> GetPlanes(const pcl::Region3D<T>& input_region,const F& transform, double tol = std::numeric_limits<double>::epsilon())
			{
				pcl::Region3D<T> region = input_region;
				for (int i=0; i<3; ++i) {
					if (pcl::approxZero(region.getMaxPoint()[i]-region.getMinPoint()[i], tol)) {
						T val = region.getMinPoint()[i];
						region.getMaxPoint()[i] = val+1;
						region.getMinPoint()[i] = val-1;
					}
				}
				auto center = transform(pcl::Point3D<double>(
					region.getMinPoint().x()+ (double(region.getMaxPoint().x()-region.getMinPoint().x())/2),
					region.getMinPoint().y()+ (double(region.getMaxPoint().y()-region.getMinPoint().y())/2),
					region.getMinPoint().z()+ (double(region.getMaxPoint().z()-region.getMinPoint().z())/2)
				));
				std::vector<Plane3D<VT>> result;
				for (int normal_axis=0; normal_axis<3; ++normal_axis) {
					for (int is_min=0; is_min<2; ++is_min) {
						pcl::Point3D<double> p[3]; //Max, Min and a third point
						for (int i=0, j=0; i<3; ++i) {
							if (normal_axis==i) {
								if (is_min==0) for (int x=0; x<3; ++x) p[x][i] = region.getMaxPoint()[i];
								else for (int x=0; x<3; ++x) p[x][i] = region.getMinPoint()[i];
							} else {
								p[0][i] = region.getMinPoint()[i];
								p[1][i] = region.getMaxPoint()[i];
								if (j==0) p[2][i] = region.getMinPoint()[i];
								else p[2][i] = region.getMaxPoint()[i];
								++j;
							}
						}
						Plane3D<VT> plane(transform(p[0]), transform(p[1]), transform(p[2]), tol);
						if (plane.sign(center)>0) plane.setNormal(-plane.getNormal());
						result.push_back(plane);
					}
				}
				return result;
			}
		
			// Assumes the normal of all the given planes are pointing outwards
			Cube(const std::vector<Plane3D<VT>>& planes)
			{
				set(planes);
			}
			// https://stackoverflow.com/search?q=cannot+bind+non-const+lvalue+reference+of+type+gcc
			template <class T>
			Cube(const pcl::Region3D<T>& region, double tol = std::numeric_limits<double>::epsilon())
			{
				set(GetPlanes(
					region, 
					[](const pcl::Point3D<T>& p)->pcl::Point3D<T> { return p; }, 
					tol
				));
			}

			template <class T, class F>
			Cube(const pcl::Region3D<T>& region,const F& transform, double tol = std::numeric_limits<double>::epsilon())
			{
				set(GetPlanes(region, transform, tol));
			}

			Cube(const Cube& obj)
			{
				m_Planes = obj.m_Planes;
			}

			Cube(Cube&& obj)
			{
				m_Planes = std::move(obj.m_Planes);
			}

			Cube operator=(const Cube& obj)
			{
				m_Planes = obj.m_Planes;
				return *this;
			}

			Cube operator=(Cube&& obj)
			{
				m_Planes = std::move(obj.m_Planes);
				return *this;
			}

			double getTolerance() const
			{
				return m_Planes[0].plane.getTolerance();
			}
			
			template <class T>
			std::vector<pcl::Point3D<double>> getIntersection(const Plane3D<T>& plane) const
			{
				std::vector<pcl::Point3D<double>> result;
				//Test whether plane coincides with any of planes of the cube
				pcl_ForEach(m_Planes, item) {
					if (item->plane.coincide(plane)) {
						std::queue<int> q;
						for (int i=0; i<item->lines.size(); ++i) q.push(i);
						int index = q.front();
						q.pop();
						int first_index = index;
						while (!q.empty()) {
							int next = -1;
							while (next==-1) {
								next = q.front();
								q.pop();
								if (!item->lines[index].intersect(item->lines[next])) {
									q.push(next);
									next = -1;
								}
							}
							result.push_back(item->lines[index].getIntersection(item->lines[next]));
							index = next;
						}
						result.push_back(item->lines[first_index].getIntersection(item->lines[index]));
						return result;
					}
				}
				//Find intersection
				std::vector<int> used_planes;
				bool done = false;
				int index = getNextPlane([&](const Plane3D<VT>& p)->bool {
					return plane.intersect(p);
				}, used_planes);
				if (index<0) return result;
				int first_index = index;
				while (!done) {
					used_planes.push_back(index);
					const PlaneInfo &info = m_Planes[index];
					//Compute the line where input plane and selected plane intersects
					auto line = plane.getIntersection(info.plane);
					int offset = 0;
					while (true) {
						//Find a plane (which bound the current selected plane) that intersects with the line
						int next = getNextPlane([&](const Plane3D<VT>& p)->bool {
							return line.intersect(p);
						}, used_planes, info.indexes, offset);
						if (next==-1) {
							done = true;
							break;
						} else {
							auto point = m_Planes[next].plane.template getIntersection<Point3D<double>>(line);
							if (containPoint(index, point)) {
								result.push_back(point);
								index = next;
								break;
							} else {
								for (int i=0; i<info.indexes.size(); ++i) if (info.indexes[i]==next) {
									offset = i+1;
									break;
								}
							}
						}
					}
				}
				//Completing the loop
				{
					auto line = plane.getIntersection(m_Planes[index].plane);
					if (line.intersect(m_Planes[first_index].plane)) {
						result.push_back(m_Planes[first_index].plane.template getIntersection<Point3D<double>>(line));
					}
				}
				return result;
			}

			std::vector<Plane3D<VT>> getPlanes() const
			{
				std::vector<Plane3D<VT>> result;
				pcl_ForEach(m_Planes, item) result.push_back(item->plane);
				return result;
			}
			
		protected:
			struct PlaneInfo {
				Plane3D<VT> plane;
				std::vector<Line3D<double>> lines;
				std::vector<int> indexes;
				
				PlaneInfo()
				{}
				
				PlaneInfo(const Plane3D<VT>& p, const std::vector<Line3D<VT>>& v, const std::vector<int>& i)
				{
					plane = p;
					lines = v;
					indexes = i;
				}
				
				PlaneInfo(const PlaneInfo& obj) 
				{
					plane = obj.plane;
					lines = obj.lines;
					indexes = obj.indexes;
				}
				
				PlaneInfo(PlaneInfo&& obj) 
				{
					plane = obj.plane;
					lines = std::move(obj.lines);
					indexes = std::move(obj.indexes);
				}
				
				PlaneInfo& operator=(const PlaneInfo& obj) 
				{
					plane = obj.plane;
					lines = obj.lines;
					indexes = obj.indexes;
					return *this;
				}
				
				PlaneInfo& operator=(PlaneInfo&& obj) 
				{
					plane = obj.plane;
					lines = std::move(obj.lines);
					indexes = std::move(obj.indexes);
					return *this;
				}
			};
			std::vector<PlaneInfo> m_Planes;

			void set(const typename std::vector<Plane3D<VT>>& planes)
			{
				pcl_ForEach(planes, cur_plane) {
					std::vector<Line3D<double>> lines;
					std::vector<int> indexes;
					pcl_ForEach(planes, item) if (cur_plane->intersect(*item)) {
						lines.push_back(cur_plane->getIntersection(*item));
						indexes.push_back(static_cast<int>(item-planes.begin()));
					}
					m_Planes.push_back(PlaneInfo(*cur_plane, lines, indexes));
				}
			}
			
			template <class T>
			bool containPoint(int index, const Point3D<T>& point) const
			{
				const PlaneInfo &info = m_Planes[index];
				pcl_ForEach(info.indexes, item) {
					if (m_Planes[*item].plane.sign(point)>0) {
						return false;
					}
				}
				return true;
			}
			
			bool isIn(const std::vector<int>& vec, int val) const
			{
				pcl_ForEach(vec, item) if (val==*item) return true;
				return false;
			}
			
			template <class TestFunc>
			int getNextPlane(const TestFunc& test, const std::vector<int>& used_planes, const std::vector<int>& avail_planes=std::vector<int>(), int offset=0) const
			{
				if (avail_planes.empty()) {
					for (int i=0; i<m_Planes.size(); ++i) {
						if (!isIn(used_planes, i))
							if (test(m_Planes[i].plane)) 
								return i;
					}
				} else {
					for (int i=offset; i<avail_planes.size(); ++i) {
						int index = avail_planes[i];
						if (!isIn(used_planes, index)) {
							if (test(m_Planes[index].plane))
								return index;
						}
					}
				}
				return -1;
			}
		};
		
	}
}

#endif