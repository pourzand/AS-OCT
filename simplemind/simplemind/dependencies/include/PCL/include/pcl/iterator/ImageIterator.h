#ifndef PCL_IMAGE_ITERATOR2
#define PCL_IMAGE_ITERATOR2

#include <pcl/image/ImageBase.h>
#include <pcl/iterator/IteratorMacro.h>
#include <pcl/macro.h>
#include <pcl/exception.h>
#include <iostream>

namespace pcl
{

	class ImageIterator: private boost::noncopyable
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

		ImageIterator() {}
		ImageIterator(const ImageBase::ConstantPointer& ptr)
		{
			setImage(ptr);
		}
		ImageIterator(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE) 
		{
			setImage(ptr, a, b, c);
		}

		void setImage(const ImageBase::ConstantPointer& ptr)
		{
			m_Image = ptr;
			m_Axis[0] = X;
			m_Axis[1] = Y;
			m_Axis[2] = Z;
			setRegion(m_Image->getRegion());
		}
		void setImage(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE)
		{
			m_Image = ptr;
			m_Axis[0] = a;
			m_Axis[1] = b;
			m_Axis[2] = c;
			setRegion(m_Image->getRegion());
		}

		void setRegion(const Point3D<int>& minp, const Point3D<int>& maxp)
		{
			setRegion(Region3D<int>(minp, maxp));
		}
		void setRegion(const Region3D<int>& reg)
		{
			if (reg.empty()) {
				m_StartIndex = -1;
				return;
			}
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

		void begin() const
		{
			if (m_StartIndex<0) {
				m_End = true;
				return;
			}
			m_Index = m_StartIndex;
			for (int i=0; i<m_Num; ++i) {
				m_EndSpan[i] = m_Index + m_Span[i];
			}
			m_End = false;
		}

		bool end() const
		{
			return m_End;
		}

		void next() const
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
			m_End = true;
		}

		long getIndex() const
		{
			return m_Index;
		}
		operator long() const
		{
			return m_Index;
		}

	protected:
		mutable long m_EndSpan[3], m_Index;
		long m_Offset[3], m_Span[3];
		mutable bool m_End;
		ImageBase::ConstantPointer m_Image;
		int m_Axis[3];
		Point3D<int> m_StartPoint;
		long m_StartIndex;
		int m_Num;

		void addSequence(int axis, Point3D<int>& end_point, int count[])
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

		void addDummySequence() //This is to avoid the need for an extra "if" statement in next()
		{
			m_Offset[m_Num] = 1;
			m_Span[m_Num] = 1;
			m_Num++;
		}
	};



	class ImageIteratorWithPoint: public ImageIterator
	{
	public:
		ImageIteratorWithPoint() {}
		ImageIteratorWithPoint(const ImageBase::ConstantPointer& ptr) 
		{
			setImage(ptr);
		}
		ImageIteratorWithPoint(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE)
		{
			setImage(ptr, a, b, c);
		} 

		void setImage(const ImageBase::ConstantPointer& ptr)
		{
			ImageIterator::setImage(ptr);
			setPointOffset(ptr->getRegion());
		}
		void setImage(const ImageBase::ConstantPointer& ptr, Axis a, Axis b=NONE, Axis c=NONE)
		{
			ImageIterator::setImage(ptr, a, b, c);
			setPointOffset(ptr->getRegion());
		}

		void setRegion(const Point3D<int>& minp, const Point3D<int>& maxp)
		{
			setRegion(Region3D<int>(minp, maxp));
		}
		void setRegion(const Region3D<int>& reg)
		{
			ImageIterator::setRegion(reg);
			setPointOffset(reg);
		}

		void begin() const
		{
			ImageIterator::begin();
			m_Point = this->m_StartPoint;
		}

		void next() const
		{
			m_Index += m_Offset[0];
			if (m_Index!=m_EndSpan[0]) {
				m_Point[m_PointIndex[0]] += m_PointOffset[0];
				return;
			} else m_Point[m_PointIndex[0]] = m_StartPoint[m_PointIndex[0]];
			for (int i=1; i<m_Num; ++i) {
				m_Index += m_Offset[i];
				if (m_Index!=m_EndSpan[i]) {
					m_Point[m_PointIndex[i]] += m_PointOffset[i];
					for (int j=0; j<i; ++j)
						m_EndSpan[j] = m_Index + m_Span[j];
					return;
				} else m_Point[m_PointIndex[i]] = m_StartPoint[m_PointIndex[i]];
			}
			m_End = true;
		}

		const Point3D<int>& getPoint() const 
		{ 
			return m_Point; 
		}

	protected:
		int m_PointOffset[3];
		int m_PointIndex[3];
		mutable Point3D<int> m_Point;

		void setPointOffset(const Region3D<int>& reg)
		{
			int num = 0;
			for (int i=0; i<3; ++i) {
				if (m_Axis[i]==0) break;
				int val = pcl_Abs(m_Axis[i])-1;
				if (reg.getMinPoint()[val]!=reg.getMaxPoint()[val]) {
					if (m_Axis[i]<0) m_PointOffset[num] = -1;
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
	};

}

#endif