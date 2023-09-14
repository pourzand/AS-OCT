#ifndef PCL_IMAGE_REGIONS_ITERATOR
#define PCL_IMAGE_REGIONS_ITERATOR

#include <pcl/image/ImageBase.h>
#include <pcl/iterator/IteratorMacro.h>
#include <pcl/macro.h>
#include <pcl/exception.h>
#include <iostream>

namespace pcl
{

	template <class InfoType=char>
	class ImageRegionsIterator: private boost::noncopyable
	{
	public:
		enum Axis {
			NONE = 0,
			X = 1,
			Y = 2,
			Z = 3,
			RX = -1,
			RY = -2,
			RZ = -3
		};

		ImageRegionsIterator() {}
		ImageRegionsIterator(const ImageBase::ConstantPointer& ptr)
		{
			setImage(ptr);
		}
		ImageRegionsIterator(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE) 
		{
			setImage(ptr, a, b, c);
		}

		void setImage(const ImageBase::ConstantPointer& ptr)
		{
			m_Image = ptr;
			m_Axis[0] = X;
			m_Axis[1] = Y;
			m_Axis[2] = Z;
		}
		void setImage(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE)
		{
			m_Image = ptr;
			m_Axis[0] = a;
			m_Axis[1] = b;
			m_Axis[2] = c;
		}
		
		void clear()
		{
			m_Regions.clear();
			m_Info.clear();
		}
		
		void add(const Region3D<int>& reg, InfoType info=InfoType())
		{
			if (!reg.empty()) {
				m_Regions.push_back(reg);
				m_Info.push_back(info);
			}
		}
		
		template <class ListType>
		void addList(const ListType& list, InfoType info=InfoType())
		{
			pcl_ForEach(list, item) add(*item, info);
		}

		void begin() const
		{
			if (m_Regions.empty()) {
				m_End = true;
				return;
			}
			m_End = false;
			m_RegionIndex = 0;
			regionBegin();
		}

		bool end() const
		{
			return m_End;
		}

		void next() const
		{
			regionNext();
			if (m_RegionEnd) {
				++m_RegionIndex;
				if (m_RegionIndex>=m_Regions.size()) {
					m_End = true;
					return;
				}
				regionBegin();
			}
		}

		long getIndex() const
		{
			return m_Index;
		}
		operator long() const
		{
			return m_Index;
		}
		
		InfoType getInfo() const
		{
			return m_Info[m_RegionIndex];
		}

	protected:
		std::vector<Region3D<int>> m_Regions;
		std::vector<InfoType> m_Info;
		mutable int m_RegionIndex;
		mutable long m_EndSpan[3], m_Index;
		mutable long m_Offset[3], m_Span[3];
		mutable bool m_RegionEnd;
		mutable bool m_End;
		mutable Point3D<int> m_StartPoint;
		mutable long m_StartIndex;
		mutable int m_Num;
		ImageBase::ConstantPointer m_Image;
		int m_Axis[3];

		void addSequence(int axis, Point3D<int>& end_point, int count[]) const
		{
			int val = pcl_Abs(axis)-1;
			if (m_StartPoint[val]==end_point[val]) return;
			//Preparing offset for current axis
			count[m_Num] = end_point[val]-m_StartPoint[val]+1;
			if (axis<0) {
				std::swap(m_StartPoint[val], end_point[val]);
				m_Offset[m_Num] = -m_Image->getOffsetTable()[val];
			} else {
				m_Offset[m_Num] = m_Image->getOffsetTable()[val];
			}
			//Compensating for previous axis
			long total = 0;
			for (int i=0; i<m_Num; i++) {
				total = total*count[i] + count[i]*m_Offset[i];
			}
			m_Span[m_Num] = count[m_Num]*m_Offset[m_Num];
			m_Offset[m_Num] -= total;
			m_Num++;
		}

		void addDummySequence() const //This is to avoid the need for an extra "if" statement in next()
		{
			m_Offset[m_Num] = 1;
			m_Span[m_Num] = 1;
			m_Num++;
		}
		
		void setRegion(const Region3D<int>& reg) const
		{
			m_StartPoint = reg.getMinPoint();
			Point3D<int> end_point = reg.getMaxPoint();
			int count[3];
			m_Num = 0;
			for (int i=0; i<3; i++) {
				if (m_Axis[i]==0) break;
				addSequence(m_Axis[i], end_point, count);
			}
			if (m_Num==0) addDummySequence();
			m_StartIndex = m_Image->toIndex(m_StartPoint);
		}
		
		void regionBegin() const
		{
			setRegion(m_Regions[m_RegionIndex]);
			m_Index = m_StartIndex;
			for (int i=0; i<m_Num; ++i) {
				m_EndSpan[i] = m_Index + m_Span[i];
			}
			m_RegionEnd = false;
		}
		
		void regionNext() const
		{
			m_Index += m_Offset[0];
			if (m_Index!=m_EndSpan[0]) return;
			for (int i=1; i<m_Num; ++i) {
				m_Index += m_Offset[i];
				if (m_Index!=m_EndSpan[i]) {
					for (int j=0; j<i; ++j)
						m_EndSpan[j] = m_Index + m_Span[j];
					return;
				}
			}
			m_RegionEnd = true;
		}
	};


	template <class InfoType=char>
	class ImageRegionsIteratorWithPoint: public ImageRegionsIterator<InfoType>
	{
	public:
	
		enum Axis {
			NONE = 0,
			X = 1,
			Y = 2,
			Z = 3,
			RX = -1,
			RY = -2,
			RZ = -3
		};
		ImageRegionsIteratorWithPoint() {}
		ImageRegionsIteratorWithPoint(const ImageBase::ConstantPointer& ptr):ImageRegionsIterator<InfoType>(ptr)
		{}
		ImageRegionsIteratorWithPoint(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE):ImageRegionsIterator<InfoType>(ptr,a,b,c)
		{}

		void begin() const
		{
			if (this->m_Regions.empty()) {
				this->m_End = true;
				return;
			}
			this->m_End = false;
			this->m_RegionIndex = 0;
			regionBegin();
		}

		void next() const
		{
			regionNext();
			if (this->m_RegionEnd) {
				++this->m_RegionIndex;
				if (this->m_RegionIndex>=this->m_Regions.size()) {
					this->m_End = true;
					return;
				}
				regionBegin();
			}
		}

		const Point3D<int>& getPoint() const 
		{ 
			return m_Point; 
		}

	protected:
		mutable int m_PointOffset[3];
		mutable int m_PointIndex[3];
		mutable Point3D<int> m_Point;

		void setPointOffset(const Region3D<int>& reg) const
		{
			int num = 0;
			for (int i=0; i<3; ++i) {
				if (this->m_Axis[i]==0) break;
				int val = pcl_Abs(this->m_Axis[i])-1;
				if (reg.getMinPoint()[val]!=reg.getMaxPoint()[val]) {
					if (this->m_Axis[i]<0) m_PointOffset[num] = -1;
					else m_PointOffset[num] = 1;
					m_PointIndex[num] = val;
					++num;
				}
			}
			if (num==0) {
				m_PointOffset[0] = 1;
				m_PointIndex[0] = 0;
			}
		}
		
		void regionBegin() const
		{
			ImageRegionsIterator<InfoType>::regionBegin();
			setPointOffset(this->m_Regions[this->m_RegionIndex]);
			m_Point = this->m_StartPoint;
		}
		
		void regionNext() const
		{
			this->m_Index += this->m_Offset[0];
			if (this->m_Index!=this->m_EndSpan[0]) {
				m_Point[m_PointIndex[0]] += m_PointOffset[0];
				return;
			} else m_Point[m_PointIndex[0]] = this->m_StartPoint[m_PointIndex[0]];
			for (int i=1; i<this->m_Num; ++i) {
				this->m_Index += this->m_Offset[i];
				if (this->m_Index!=this->m_EndSpan[i]) {
					m_Point[m_PointIndex[i]] += m_PointOffset[i];
					for (int j=0; j<i; ++j)
						this->m_EndSpan[j] = this->m_Index + this->m_Span[j];
					return;
				} else m_Point[m_PointIndex[i]] = this->m_StartPoint[m_PointIndex[i]];
			}
			this->m_RegionEnd = true;
		}
	};

}

#endif