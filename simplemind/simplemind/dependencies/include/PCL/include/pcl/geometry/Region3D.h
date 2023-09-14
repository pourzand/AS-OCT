#ifndef PCL_REGION_3D
#define PCL_REGION_3D

#include <pcl/geometry/Point.h>
#include <pcl/math.h>
#include <boost/numeric/conversion/bounds.hpp>
#include <iostream>
#include <vector>
#include <utility>

namespace pcl
{

	template<class Type>
	class Region3D
	{
	public:
		typedef Region3D Self;
		typedef Type ValueType;
		typedef Point3D<ValueType> PointType;

		enum Axis {
			X = 0,
			Y = 1,
			Z = 2
		};

		Region3D() {}
		template <class PT>
		Region3D(const PT& minp, const PT& maxp)
		{
			m_MinPoint.assign(minp);
			m_MaxPoint.assign(maxp);
		}
		template <class RT>
		Region3D(const RT& obj)
		{
			m_MinPoint.assign(obj.getMinPoint());
			m_MaxPoint.assign(obj.getMaxPoint());
		}

		inline bool empty() const
		{
			return m_MinPoint.x()>m_MaxPoint.x();
		}

		inline bool valid() const
		{
			for (int i=0; i<3; ++i) {
				if (m_MinPoint[i]>m_MaxPoint[i]) return false;
			}
			return true;
		}

		inline Region3D& reset()
		{
			for (int i=0; i<3; i++) {
				m_MinPoint[i] = boost::numeric::bounds<ValueType>::highest();
				m_MaxPoint[i] = boost::numeric::bounds<ValueType>::lowest();
			}
			return *this;
		}

		inline Point3D<int> getSize() const
		{
			return Point3D<int>(getSize(X), getSize(Y), getSize(Z));
		}
		inline int getSize(int i) const
		{
			if (empty()) return 0;
			return round(m_MaxPoint[i]-m_MinPoint[i]+1);
		}
		
		inline Point3D<Type> getDiff() const
		{
			return m_MaxPoint-m_MinPoint;
		}
		inline Type getDiff(int i) const
		{
			if (empty()) return 0;
			return m_MaxPoint[i]-m_MinPoint[i];
		}

		const Point3D<Type>& getMinPoint() const
		{
			return m_MinPoint;
		}
		const Point3D<Type>& getMaxPoint() const
		{
			return m_MaxPoint;
		}

		Point3D<Type>& getMinPoint()
		{
			return m_MinPoint;
		}
		Point3D<Type>& getMaxPoint()
		{
			return m_MaxPoint;
		}

		void setMinPoint(const Point3D<Type>& p)
		{
			m_MinPoint = p;
		}
		void setMinPoint(int i, int val)
		{
			m_MinPoint[i] = val;
		}

		void setMaxPoint(const Point3D<Type>& p)
		{
			m_MaxPoint = p;
		}
		void setMaxPoint(int i, int val)
		{
			m_MaxPoint[i] = val;
		}

		void set(const Point3D<Type>& minp, const Point3D<Type>& maxp)
		{
			m_MinPoint = minp;
			m_MaxPoint = maxp;
		}

		void setSize(const Point3D<int>& sz)
		{
			m_MaxPoint = m_MinPoint + sz -1;
		}
		void setSize(int i, int val)
		{
			m_MaxPoint[i] = m_MinPoint[i]+val-1;
		}

		template <class PT>
		inline bool contain(const PT& p) const
		{
			return contain(p[0], p[1], p[2]);
		}
		template <class T>
		inline bool contain(T x, T y, T z) const
		{
			if (x<this->m_MinPoint.x() || x>this->m_MaxPoint.x()) return false;
			if (y<this->m_MinPoint.y() || y>this->m_MaxPoint.y()) return false;
			if (z<this->m_MinPoint.z() || z>this->m_MaxPoint.z()) return false;
			return true;
		}
		template <class T>
		inline bool contain(const Region3D<T>& obj) const
		{
			const typename Region3D<T>::PointType &minp = obj.getMinPoint(),
				&maxp = obj.getMaxPoint();
			for (int i=0; i<3; i++) {
				if (this->m_MinPoint[i]>minp[i] || this->m_MaxPoint[i]<maxp[i]) return false;
			}
			return true;
		}
		
		template <class PT>
		inline bool containEpsilon(const PT& p, double epsilon) const
		{
			return containEpsilon(p[0], p[1], p[2], epsilon);
		}
		template <class T>
		inline bool containEpsilon(T x, T y, T z, double epsilon) const
		{
			if (x-this->m_MinPoint.x()<-epsilon || x-this->m_MaxPoint.x()>epsilon) return false;
			if (y-this->m_MinPoint.y()<-epsilon || y-this->m_MaxPoint.y()>epsilon) return false;
			if (z-this->m_MinPoint.z()<-epsilon || z-this->m_MaxPoint.z()>epsilon) return false;
			return true;
		}
		template <class T>
		inline bool containEpsilon(const Region3D<T>& obj, double epsilon) const
		{
			const typename Region3D<T>::PointType &minp = obj.getMinPoint(),
				&maxp = obj.getMaxPoint();
			for (int i=0; i<3; i++) {
				if (this->m_MinPoint[i]-minp[i]>epsilon || this->m_MaxPoint[i]-maxp[i]<-epsilon) return false;
			}
			return true;
		}

		template <class T>
		inline bool intersect(const Region3D<T>& obj) const
		{
			if (empty()) return false;
			for (int i=0; i<3; i++) {
				if (obj.m_MinPoint[i]>m_MaxPoint[i] || obj.m_MaxPoint[i]<m_MinPoint[i]) return false;
			}
			return true;
		}

		template <class RT>
		Region3D& operator=(const RT& obj)
		{
			m_MinPoint.assign(obj.getMinPoint());
			m_MaxPoint.assign(obj.getMaxPoint());
			return *this;
		}

		Region3D& add(const Point3D<Type>& p)
		{
			m_MinPoint.min(p);
			m_MaxPoint.max(p);
			return *this;
		}
		Region3D& add(const Region3D& r)
		{
            if (r.empty()) return *this;
			m_MinPoint.min(r.m_MinPoint);
			m_MaxPoint.max(r.m_MaxPoint);
			return *this;
		}

		Region3D& setIntersect(const Region3D& r)
		{
			if (!intersect(r)) {
				reset();
				return *this;
			}
			m_MinPoint.max(r.m_MinPoint);
			m_MaxPoint.min(r.m_MaxPoint);
			return *this;
		}

		Region3D getIntersect(const Region3D& r) const
		{
			return Region3D(*this).setIntersect(r);
		}

		Region3D getRegionLessThanOrEqualTo(Axis axis, int val) const
		{
			if (empty() || val<m_MinPoint[axis] || val>m_MaxPoint[axis]) {
				return Region3D().reset();
			}
			Point3D<int> temp(m_MaxPoint);
			temp[axis] = val;
			return Region3D(m_MinPoint, temp);
		}
		Region3D getRegionMoreThanOrEqualTo(Axis axis, int val) const
		{
			if (empty() || val<m_MinPoint[axis] || val>m_MaxPoint[axis]) {
				return Region3D().reset();
			}
			Point3D<int> temp(m_MinPoint);
			temp[axis] = val;
			return Region3D(temp, m_MaxPoint);
		}

		template <class List>
		void getRegionsAfterSubtractionBy(const Region3D& r, List& result) const
		{
			if (empty()) return;
			else if (r.empty()) {
				result.push_back(*this);
				return ;
			}
			Region3D remaining_region(*this);
			const Point3D<int> &minp = r.getMinPoint(), 
				maxp = r.getMaxPoint();
			for (int i=0; i<3; i++) {
				const Region3D& less_than = remaining_region.getRegionLessThanOrEqualTo((Axis)i, minp[i]-1);
				const Region3D& more_than = remaining_region.getRegionMoreThanOrEqualTo((Axis)i, maxp[i]+1);
				if (!less_than.empty()) {
					result.push_back(less_than);
					remaining_region.m_MinPoint[i] = minp[i];
				}
				if (!more_than.empty()) {
					result.push_back(more_than);
					remaining_region.m_MaxPoint[i] = maxp[i];
				}
			}
		}
		std::vector<Region3D> getRegionsAfterSubtractionBy(const Region3D& r) const
		{
			std::vector<Region3D> result;
			getRegionsAfterSubtractionBy(r, result);
			return std::move(result);
		}
		
		template <class List>
		void getCornerPoints(List& result) const
		{
			const Point3D<Type> *p[2] = {&m_MinPoint, &m_MaxPoint};
			int x_count = m_MaxPoint[0]>m_MinPoint[0]?2:1,
				y_count = m_MaxPoint[1]>m_MinPoint[1]?2:1,
				z_count = m_MaxPoint[2]>m_MinPoint[2]?2:1;
			for (int x=0; x<x_count; x++) for (int y=0; y<y_count; y++) for (int z=0; z<z_count; z++) result.push_back(
				Point3D<Type>(
					(*(p[x]))[0],
					(*(p[y]))[1],
					(*(p[z]))[2]
				)
			);
		}
		std::vector<Point3D<Type>> getCornerPoints() const
		{
			std::vector<Point3D<Type>> result;
			getCornerPoints(result);
			return std::move(result);
		}

		template <class List>
		void getSplitRegion(List& result, int num, Point3D<int>* region_num=NULL) const
		{
			num = std::max(num,1);
			getSplitRegion(result, num, num, num, region_num);
		}
		template <class List>
		void getSplitRegion(List& result, int num_x, int num_y, int num_z) const //This operation is only valid for integer type region
		{
			auto size = getSize();
			Point3D<int> step_num(num_x, num_y, num_z);
			Point3D<int> step_size;
			for (int i=0; i<3; ++i) {
				step_size[i] = static_cast<int>(pcl::round(static_cast<double>(size[i])/std::max(step_num[i],1)));
				step_size[i] = std::max(step_size[i],1);
				step_num[i] = static_cast<int>(pcl::round(
					std::min(static_cast<double>(step_num[i]), static_cast<double>(size[i])/step_size[i])
					));
			}
			Point3D<int> p;
			for (p.z()=0; p.z()<step_num.z(); ++p.z()) for (p.y()=0; p.y()<step_num.y(); ++p.y()) for (p.x()=0; p.x()<step_num.x(); ++p.x()) {
				Point3D<int> minp = getMinPoint() + (step_size*p), 
					maxp;
				for (int i=0; i<3; i++) {
					if (p[i]==step_num[i]-1) maxp[i] = getMaxPoint()[i];
					else maxp[i] = minp[i] + step_size[i] - 1;
				}
				Region3D<int> reg(minp, maxp);
				if (reg.valid() && contain(reg)) {
					result.push_back(reg);
				}
			}
		}
		std::vector<Region3D> getSplitRegion(int num) const
		{
			return getSplitRegion(num, num, num);
		}
		std::vector<Region3D> getSplitRegion(int num_x, int num_y, int num_z) const
		{
			std::vector<Region3D> result;
			getSplitRegion(result, num_x, num_y, num_z);
			return std::move(result);
		}

		/*template <class ImagePointer>
		std::vector<Region3D> getSplitRegionWithMap(ImagePointer& map) const
		{
			ImageHelper::Fill(map, -1);
			if (map->getMinPoint()!=Point3D<int>(0,0,0)) map = map->getAlias(Point3D<int>(0,0,0), true); //Make sure map starts at (0,0,0)
			auto size = getSize();
			Point3D<int> step_num(map->getSize());
			Point3D<int> step_size;
			for (int i=0; i<3; ++i) {
				step_size[i] = static_cast<int>(pcl::round(static_cast<double>(size[i])/std::max(step_num[i],1)));
				step_size[i] = std::max(step_size[i],1);
				step_num[i] = static_cast<int>(pcl::round(
					std::min(static_cast<double>(step_num[i]), static_cast<double>(size[i])/step_size[i])
					));
			}
			std::vector<Region3D> result;
			Point3D<int> p;
			for (p.z()=0; p.z()<step_num.z(); ++p.z()) for (p.y()=0; p.y()<step_num.y(); ++p.y()) for (p.x()=0; p.x()<step_num.x(); ++p.x()) {
				Point3D<int> minp = getMinPoint() + (step_size*p), 
					maxp;
				for (int i=0; i<3; i++) {
					if (p[i]==step_num[i]-1) maxp[i] = getMaxPoint()[i];
					else maxp[i] = minp[i] + step_size[i] - 1;
				}
				Region3D<int> reg(minp, maxp);
				if (reg.valid() && contain(reg)) {
					result.push_back(reg);
					map->set(p, result.size()-1);
				}
			}
			return std::move(result);
		}*/

		inline bool operator==(const Self& obj) const
		{
			if (getMinPoint()==obj.getMinPoint() && getMaxPoint()==obj.getMaxPoint()) return true;
			return false;
		}
		
		inline bool operator!=(const Self& obj) const 
		{
			return !(*this == obj);
		}
		
		inline bool epsilonEqual(const Self& obj, double epsilon=pcl::Epsilon) const 
		{
			if (getMinPoint().epsilonEqual(obj.getMinPoint(), epsilon) && getMaxPoint().epsilonEqual(obj.getMaxPoint(), epsilon)) return true;
			return false;
		}

		std::string toString() const
		{
			if (empty()) return "Empty";
			return m_MinPoint.toString() + " " + m_MaxPoint.toString();
		}

		std::ostream& print(std::ostream& os) const
		{
			os << m_MinPoint << " " << m_MaxPoint;
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const Region3D& obj)
		{
			return obj.print(os);
		}

	protected:
		Point3D<Type> m_MinPoint, m_MaxPoint;
	};

}

#endif
