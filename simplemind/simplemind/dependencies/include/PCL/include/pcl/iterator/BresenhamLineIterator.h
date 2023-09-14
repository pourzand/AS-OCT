#ifndef PCL_BRESENHAM_LINE_ITERATOR
#define PCL_BRESENHAM_LINE_ITERATOR

#include <pcl/image.h>

namespace pcl
{
	namespace iterator
	{

		class BresenhamLineIterator {
		public:
			BresenhamLineIterator() {}
			BresenhamLineIterator(const ImageBase::ConstantPointer& image)
			{
				setImage(image);
			}

			void setImage(const ImageBase::ConstantPointer& image)
			{
				m_Image = image;
			}

			void setOrigin(const Point3D<int>& origin)
			{
				setOrigin(origin, m_Image->toIndex(origin));
			}
			void setOrigin(const Point3D<int>& origin, long index)
			{
				m_Origin = origin;
				m_OriginIndex = index;
			}

			void setDirection(const Point3D<double>& direction, bool direction_is_in_voxel)
			{
				Point3D<double> actual_direction(direction);
				if (!direction_is_in_voxel) {
					//Recomputing direction based on voxel size
					actual_direction /= m_Image->getSpacing();
				}
				//Getting the axis with maximum change
				double abs_dir[] = {pcl_Abs(actual_direction[0]), pcl_Abs(actual_direction[1]), pcl_Abs(actual_direction[2])};
				m_MaxAxis = 0;
				for (int i=1; i<3; i++) if (abs_dir[m_MaxAxis]<abs_dir[i]) m_MaxAxis = i;
				//Computing delta error for the other axis
				for (int i=0; i<3; i++) {
					if (i!=m_MaxAxis) m_DeltaError[i] = abs_dir[i]/abs_dir[m_MaxAxis];
					else m_DeltaError[i] = 0;
					if (actual_direction[i]<0) {
						m_Increment[i] = -1;
						m_IndexIncrement[i] = -m_Image->getOffsetTable()[i];
					} else {
						m_Increment[i] = 1;
						m_IndexIncrement[i] = m_Image->getOffsetTable()[i];
					}
				}
			}

			void begin() const
			{
				m_Point = m_Origin;
				m_Index = m_OriginIndex;
				m_Error.set(0,0,0);
				m_End = false;
			}

			bool end() const
			{
				return m_End;
			}

			void next() const
			{
				m_Point[m_MaxAxis] += m_Increment[m_MaxAxis];
				m_Index += m_IndexIncrement[m_MaxAxis];
				m_Error += m_DeltaError;
				for (int i=0; i<3; ++i) if (i!=m_MaxAxis) {
					if (m_Error[i]>=0.5) {
						m_Point[i] += m_Increment[i];
						m_Index += m_IndexIncrement[i];
						m_Error[i] -= 1;
					}
				}
				if (!m_Image->contain(m_Point)) m_End = true;
			}

			const Point3D<int>& getPoint() const
			{
				return m_Point;
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
			ImageBase::ConstantPointer m_Image;
			Point3D<int> m_Origin, m_Increment;
			Point3D<long> m_IndexIncrement;
			long m_OriginIndex;
			Point3D<double> m_DeltaError;
			int m_MaxAxis;
			mutable Point3D<int> m_Point;
			mutable Point3D<double> m_Error;
			mutable long m_Index;
			mutable bool m_End;
		};



		class BresenhamLineIteratorWithEndPoint: protected BresenhamLineIterator
		{
		public:
			BresenhamLineIteratorWithEndPoint() {}
			BresenhamLineIteratorWithEndPoint(const ImageBase::ConstantPointer& image): BresenhamLineIterator(image)
			{}

			void setImage(const ImageBase::ConstantPointer& image)
			{
				this->m_Image = image;
			}

			void setLine(const Point3D<int>& start_point, const Point3D<int>& end_point)
			{
				this->setOrigin(start_point);
				this->setDirection(end_point-start_point, true);
				m_EndIndex = this->m_Image->toIndex(end_point);
			}

			void begin() const
			{
				BresenhamLineIterator::begin();
			}

			bool end() const
			{
				return this->m_End;
			}

			void next() const
			{
				if (m_Index==m_EndIndex) this->m_End = true;
				else BresenhamLineIterator::next();
			}

			const Point3D<int>& getPoint() const
			{
				return this->m_Point;
			}

			long getIndex() const
			{
				return this->m_Index;
			}
			operator long() const
			{
				return this->m_Index;
			}

		protected:
			long m_EndIndex;
		};

	}
}

#endif