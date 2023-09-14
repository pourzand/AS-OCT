#ifndef PCL_IMAGE_NEIGHBOR_ITERATOR_TYPE
#define PCL_IMAGE_NEIGHBOR_ITERATOR_TYPE

#include <pcl/image.h>
#include <pcl/macro.h>
#include <vector>

namespace pcl
{
	namespace iterator
	{

		class ImageNeighborIterator: private boost::noncopyable
		{
		public:
			typedef std::vector<Point3D<int> > OffsetListType;
			typedef boost::shared_ptr<OffsetListType> OffsetListPointer;
			typedef boost::shared_ptr<OffsetListType const> ConstantOffsetListPointer;

			ImageNeighborIterator() 
			{
				m_Initialized = false;
			}
			ImageNeighborIterator(const ImageBase::ConstantPointer& img)
			{
				m_Initialized = false;
				setImage(img);
			}
			ImageNeighborIterator(const ImageBase::ConstantPointer& img, const ConstantOffsetListPointer& ptr)
			{
				m_Initialized = false;
				setImage(img);
				(*this) = ptr;
			}

			//Assignment (setting internal offset list)
			ImageNeighborIterator& operator=(const OffsetListPointer& ptr)
			{
				m_OffsetList = ptr;
				initialize();
				return *this;
			}
			ImageNeighborIterator& operator=(const ConstantOffsetListPointer& ptr)
			{
				m_OffsetList = ptr;
				initialize();
				return *this;
			}

			ImageNeighborIterator& operator=(const ImageNeighborIterator& obj)
			{
				m_OffsetList = obj.m_OffsetList;
				m_IndexOffsetList = obj.m_IndexOffsetList;
				m_Image = obj.m_Image;
				initialize();
				return *this;
			}

			template <class List>
			ImageNeighborIterator& operator=(const List& list)
			{
				OffsetListType *offset_list = new OffsetListType();
				pcl_ForEach(list, iter) offset_list->push_back(*iter);
				m_OffsetList.reset(offset_list);
				initialize();
				return *this;
			}

			//Setting image
			void setImage(const ImageBase::ConstantPointer& img) 
			{
				m_Image = img;
				initialize();
			}

			//Setting origin
			void setOrigin(const Point3D<int>& p)
			{
				setOrigin(p, m_Image->toIndex(p));
			}
			void setOrigin(const Point3D<int>& p, long index)
			{
				m_OriginIndex = index;
				m_OriginPoint = p;
			}
			void setOrigin(long index)
			{
				m_OriginIndex = index;
				m_OriginPoint.set(0,0,0);
			}

			//Iteration related
			void begin() const 
			{
				m_Iter = 0;
				m_End = false;
				next();
			}

			inline bool end() const 
			{ 
				return m_End;
			}

			void next() const 
			{
				if (end()) return;
				if (m_Iter>=(int)m_OffsetList->size()) {
					m_End = true;
					return;
				}
				m_Index = m_OriginIndex + m_IndexOffsetList[m_Iter];
				m_Point = m_OriginPoint; m_Point += (*m_OffsetList)[m_Iter];
				m_Iter++;
			}

			//Iteration data access
			inline operator long() const
			{
				return m_Index;
			}

			inline long getIndex() const
			{
				return m_Index;
			}

			inline const Point3D<int>& getPoint() const
			{
				return m_Point;
			}

			inline const Point3D<int>& getOffset() const
			{
				return (*m_OffsetList)[m_Iter-1];
			}

			inline int getIteration() const
			{
				return m_Iter-1;
			}

			//Misc data access
			inline const Point3D<int>& getOffset(int i) const
			{
				return (*m_OffsetList)[i]; 
			}

			int size() const
			{ 
				return static_cast<int>(m_OffsetList->size()); 
			}
			
			bool initialized() const
			{
				return m_Initialized;
			}

			inline const Region3D<int>& getOffsetRegion() const
			{
				return m_OffsetRegion;
			}

			const ConstantOffsetListPointer& getOffsetList() const
			{
				return m_OffsetList;
			}

			const ImageBase::ConstantPointer& getImage() const
			{
				return m_Image;
			}

		protected:
			mutable long m_Index;
			mutable Point3D<int> m_Point;
			mutable int m_Iter;
			mutable bool m_End;
			Region3D<int> m_OffsetRegion;
			Point3D<int> m_OriginPoint;
			long m_OriginIndex;
			std::vector<long> m_IndexOffsetList;
			ConstantOffsetListPointer m_OffsetList;
			ImageBase::ConstantPointer m_Image;
			bool m_Initialized;

			void initialize()
			{
				if (!m_Image || !m_OffsetList) {
					m_Initialized = false;
					return;
				}
				m_IndexOffsetList.clear();
				m_OffsetRegion.reset();
				long const *offset_table = m_Image->getOffsetTable();
				pcl_ForEach(*m_OffsetList, iter)
				{
					long offset = 0;
					for (int i=0; i<3; i++) {
						offset += offset_table[i]*(*iter)[i];
					}
					m_IndexOffsetList.push_back(offset);
					m_OffsetRegion.add(*iter);
				}
				m_Initialized = true;
			}

		public:
			static void Print(std::ostream& os, const ConstantOffsetListPointer& list)
			{
                for (int i=0; i<list->size(); ++i) os << (*list)[i] << std::endl;
			}
		
			template <class ImagePointerType>
			static OffsetListPointer FilterOffsetList(const OffsetListPointer& list, const ImagePointerType& img)
			{
				return FilterOffsetList(list, img->getSize());
			}
			static OffsetListPointer FilterOffsetList(const OffsetListPointer& list, const Point3D<int>& sz)
			{
				Point3D<int> size = sz;
				size -= 1;
				OffsetListType temp;
				temp.reserve(list->size());
				pcl_ForEach(*list, item) {
					bool accept = true;
					for (int d=0; d<3; ++d) if (abs((*item)[d])>size[d]) accept = false;
					if (accept) temp.push_back(*item);
				}
				*list = std::move(temp);
				return list;
			}
		
			//Helper functions for creating typical offset list
			template<class ImagePointerType>
			static OffsetListPointer CreateOffsetFromImage(const ImagePointerType& img)
			{
				OffsetListPointer list(new OffsetListType());
				ImageIteratorWithPoint iter(img);
				pcl_ForIterator(iter) {
					if (img->get(iter)) list->push_back(iter.getPoint());
				}
				return list;
			}
			
			static OffsetListPointer CreateOffsetFromRegion(const pcl::Region3D<int>& region, bool skip_origin=false)
			{
				OffsetListPointer list(new OffsetListType());
				pcl_ForXYZ(x,y,z, region.getMinPoint(), region.getMaxPoint()) { 
					if (skip_origin) {
						if (x==0 && y==0 && z==0) continue;
					}
					list->push_back(Point3D<int>(x,y,z));
				}
				return list;
			}
			
			static OffsetListPointer CreateSphereOffset(int r, bool skip_origin=false)
			{
				return CreateEllipsoidOffset(r,r,r, skip_origin);
			}
			static OffsetListPointer CreateEllipsoidOffset(int rx, int ry, int rz, bool skip_origin=false)
			{
				OffsetListPointer list(new OffsetListType());
				Point3D<int> minp(-rx,-ry,-rz), maxp(rx,ry,rz);
				pcl_ForXYZ(x,y,z, minp, maxp) {
					double ax = double(x)/double(rx),
						ay = double(y)/double(ry),
						az = double(z)/double(rz);
					if (rx<=0) ax = 0;
					if (ry<=0) ay = 0;
					if (rz<=0) az = 0;
					if (skip_origin) {
						if (x==0 && y==0 && z==0) continue;
					}
					if (pcl_Square(ax) + pcl_Square(ay) + pcl_Square(az) < 1.0005) { //A number slightly more than 1 is used to prevent round up errors
						list->push_back(Point3D<int>(x,y,z));
					}
				}
				return list;
			}

			static OffsetListPointer CreateConnect6Offset(bool skip_origin=true) 
			{
				OffsetListPointer list(new OffsetListType());
				list->reserve(6);
				list->push_back(Point3D<int>(1,0,0));
				list->push_back(Point3D<int>(-1,0,0));
				list->push_back(Point3D<int>(0,1,0));
				list->push_back(Point3D<int>(0,-1,0));
				list->push_back(Point3D<int>(0,0,1));
				list->push_back(Point3D<int>(0,0,-1));
				if (!skip_origin) list->push_back(Point3D<int>(0,0,0));
				return list;
			}

			static OffsetListPointer CreateConnect26Offset(bool skip_origin=true) 
			{
				OffsetListPointer list(new OffsetListType());
				list->reserve(26);
				for (int x=-1; x<=1; x++) for (int y=-1; y<=1; y++) for (int z=-1; z<=1; z++) {
					if (x==0 && y==0 && z==0) continue;
					list->push_back(Point3D<int>(x,y,z));
				}
				if (!skip_origin) list->push_back(Point3D<int>(0,0,0));
				return list;
			}
			
			static OffsetListPointer CreateConnect18Offset(bool skip_origin=true)
			{
				OffsetListPointer list(new OffsetListType());
				list->reserve(18);
				for (int x=-1; x<=1; x++) for (int y=-1; y<=1; y++) for (int z=-1; z<=1; z++) {
					if (x==0 && y==0 && z==0) continue;
					if (pcl_Abs(x)+pcl_Abs(y)+pcl_Abs(z)==3) continue;
					list->push_back(Point3D<int>(x,y,z));
				}
				if (!skip_origin) list->push_back(Point3D<int>(0,0,0));
				return list;
			}

			static OffsetListPointer CreateConnect8Offset(bool skip_origin=true) 
			{
				OffsetListPointer list(new OffsetListType());
				list->reserve(8);
				for (int x=-1; x<=1; x++) for (int y=-1; y<=1; y++) {
					if (x==0 && y==0) continue;
					list->push_back(Point3D<int>(x,y,0));
				}
				if (!skip_origin) list->push_back(Point3D<int>(0,0,0));
				return list;
			}

			static OffsetListPointer CreateConnect4Offset(bool skip_origin=true) 
			{
				OffsetListPointer list(new OffsetListType());
				list->reserve(4);
				list->push_back(Point3D<int>(1,0,0));
				list->push_back(Point3D<int>(-1,0,0));
				list->push_back(Point3D<int>(0,1,0));
				list->push_back(Point3D<int>(0,-1,0));
				if (!skip_origin) list->push_back(Point3D<int>(0,0,0));
				return list;
			}
		};

	}
}

#endif
