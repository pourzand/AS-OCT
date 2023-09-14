#ifndef PCL_WINDOW_ITERATOR
#define PCL_WINDOW_ITERATOR

#include <pcl/iterator/ImageIterator.h>

namespace pcl
{
	namespace iterator
	{

		class ImageWindowIterator: public pcl::ImageIterator
		{
		public:
			ImageWindowIterator() {}
			ImageWindowIterator(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window)
			{
				setImage(ptr, window);
			}
			ImageWindowIterator(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window, Axis a, Axis b=NONE, Axis c=NONE)
			{
				setImage(ptr, window, a, b, c);
			}

			void setImage(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window)
			{
				setImage(ptr, window, X, Y, Z);
			}
			void setImage(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window, Axis a, Axis b=NONE, Axis c=NONE)
			{
				m_Image = ptr;
				m_Axis[0] = a;
				m_Axis[1] = b;
				m_Axis[2] = c;
				m_StartPoint = window.getMinPoint();
				Point3D<int> end_point = window.getMaxPoint();
				int count[3];
				m_Num = 0;
				for (int i=0; i<3; i++) {
					if (m_Axis[i]==0) break;
					addSequence(m_Axis[i], end_point, count);
				}
				if (m_Num==0) addDummySequence();

				m_StartIndexOffset = 0;
				for (int i=0; i<3; i++)
					m_StartIndexOffset += m_StartPoint[i]*(m_Image->getOffsetTable())[i];

                m_window = window;
			}

			void setWindowOrigin(const Point3D<int>& p) 
			{
				setWindowOrigin(m_Image->toIndex(p));

                m_region.set(p + m_window.getMinPoint(), p + m_window.getMaxPoint());
			}
			void setWindowOrigin(long index) 
			{
				m_StartIndex = index;
			}

			void begin() const
			{
				m_Index = m_StartIndex + m_StartIndexOffset;
				for (int i=0; i<m_Num; ++i) {
					m_EndSpan[i] = m_Index + m_Span[i];
				}
				m_End = false;
			}

            const Region3D<int> &getRegion() const
            {
                return m_region;
            }


		protected:
			long m_StartIndexOffset;
            Region3D<int> m_window;         // "Window" space i.e. (-3, -3, -3) (3, 3, 3)
            Region3D<int> m_region;         // Image space

		private:
			void setRegion() {};
		};



		class ImageWindowIteratorWithPoint: public ImageWindowIterator
		{
		public:
			ImageWindowIteratorWithPoint() {}
			ImageWindowIteratorWithPoint(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window)
			{
				setImage(ptr, window);
			}
			ImageWindowIteratorWithPoint(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window, Axis a, Axis b=NONE, Axis c=NONE)
			{
				setImage(ptr, window, a, b, c);
			}
			
			void setImage(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window)
			{
				setImage(ptr, window, X, Y, Z);
			}
			void setImage(const ImageBase::ConstantPointer& ptr, const Region3D<int>& window, Axis a, Axis b=NONE, Axis c=NONE)
			{
				ImageWindowIterator::setImage(ptr, window, a,b,c);
				int num = 0;
				for (int i=0; i<3; ++i) {
					if (m_Axis[i]==0) break;
					int val = pcl_Abs(m_Axis[i])-1;
					if (window.getMinPoint()[val]!=window.getMaxPoint()[val]) {
						if (m_Axis[i]<0) m_PointOffset[num] = -1;
						else m_PointOffset[num] = 1;
						m_PointIndex[num] = val;
						++num;
					}
				}
			}

			void setWindowOrigin(const Point3D<int>& p) 
			{
				setWindowOrigin(p, m_Image->toIndex(p));
			}
			void setWindowOrigin(const Point3D<int>& p, long index) 
			{
				m_StartIndex = index;
				m_ActualStartPoint = p;
				m_ActualStartPoint += m_StartPoint;
                m_region.set(p + m_window.getMinPoint(), p + m_window.getMaxPoint());
			}

			void begin() const
			{
				ImageWindowIterator::begin();
				m_Point = m_ActualStartPoint;
			}

			void next() const
			{
				m_Index += m_Offset[0];
				if (m_Index!=m_EndSpan[0]) {
					m_Point[m_PointIndex[0]] += m_PointOffset[0];
					return;
				} else m_Point[m_PointIndex[0]] = m_ActualStartPoint[m_PointIndex[0]];
				for (int i=1; i<m_Num; ++i) {
					m_Index += m_Offset[i];
					if (m_Index!=m_EndSpan[i]) {
						m_Point[m_PointIndex[i]] += m_PointOffset[i];
						for (int j=0; j<i; ++j)
							m_EndSpan[j] = m_Index + m_Span[j];
						return;
					} else m_Point[m_PointIndex[i]] = m_ActualStartPoint[m_PointIndex[i]];
				}
				m_End = true;
			}

			const Point3D<int>& getPoint() const 
			{ 
				return m_Point; 
			}

		protected:
			Point3D<int> m_ActualStartPoint;
			int m_PointIndex[3];
			int m_PointOffset[3];
			mutable Point3D<int> m_Point;
		};

	}
}

#endif