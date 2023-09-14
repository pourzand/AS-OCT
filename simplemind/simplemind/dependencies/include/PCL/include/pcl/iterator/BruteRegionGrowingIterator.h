#ifndef PCL_BRUTE_REGION_GROWING_ITERATOR
#define PCL_BRUTE_REGION_GROWING_ITERATOR

#include <pcl/image/ImageAlgorithmObject.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <queue>

namespace pcl
{
	namespace iterator
	{

		class BruteRegionGrowingIterator: private boost::noncopyable
		{
		public:
			BruteRegionGrowingIterator() {}
			
			void setNeighborIterator(const ImageNeighborIterator& iter, const Region3D<int>& region)
			{
				m_Iter.reset(new ImageNeighborIterator);
				*m_Iter = iter;
				m_Region = region;
				misc::SafeUnsafeRegionGenerator safe_region_gen(m_Region, m_Region, m_Iter->getOffsetRegion());
				m_SafeRegion = safe_region_gen.getSafeRegion();
			}
			
			void setNeighborIterator(const boost::shared_ptr<ImageNeighborIterator>& iter, const Region3D<int>& region)
			{
				m_Iter = iter;
				m_Region = region;
				misc::SafeUnsafeRegionGenerator safe_region_gen(m_Region, m_Region, m_Iter->getOffsetRegion());
				m_SafeRegion = safe_region_gen.getSafeRegion();
			}

			void setNeighborIterator(const ImageNeighborIterator::ConstantOffsetListPointer& ptr, const ImageBase::ConstantPointer& img)
			{
				m_Iter.reset(new ImageNeighborIterator);
				*m_Iter = ptr;
				m_Iter->setImage(img);
				m_Region = img->getRegion();
				misc::SafeUnsafeRegionGenerator safe_region_gen(m_Region, m_Region, m_Iter->getOffsetRegion());
				m_SafeRegion = safe_region_gen.getSafeRegion();
			}

			//Methods for getting internal data
			const Region3D<int>& getRegion() const
			{
				return m_Region;
			}

			ImageNeighborIterator& getNeighborIterator()
			{
				return *m_Iter;
			}
			
			boost::shared_ptr<ImageNeighborIterator> getNeighborIteratorPointer()
			{
				return m_Iter;
			}

			//Methods for adding seeds
			void addSeed(const Point3D<int>& p) 
			{
				addSeed(p, m_Iter->getImage()->toIndex(p));
			}
			void addSeed(const Point3D<int>& p, long index) 
			{
				m_SourceQueue.push(PointIndexObject(p,index));
			}

			//Iteration methods
			inline void begin() 
			{
				m_End = false;
				next();
			}

			void next() {
				if (m_End) return;
				if (m_TargetQueue.empty()) {
					while (!m_SourceQueue.empty() && m_TargetQueue.empty()) {
						addTargetFromSource(m_SourceQueue.front());
						m_SourceQueue.pop();
					}
				}
				if (m_SourceQueue.empty() && m_TargetQueue.empty()) m_End = true;
				else {
					m_Target = m_TargetQueue.front();
					m_TargetQueue.pop();
				}
				m_Accepted = false;
			}

			inline bool end() 
			{ 
				return m_End; 
			}

			void accept() {
				if (m_Accepted) return;
				m_Accepted = true;
				m_SourceQueue.push(m_Target);
			}

			Point3D<int> getOffset() const
			{
				return m_Target.point - m_Source.point;
			}

			//Accessing target or current point
			inline const Point3D<int>& getPoint() const
			{ 
				return m_Target.point; 
			}
			inline long getIndex() const
			{
				return m_Target.index;
			}
			inline operator long() const
			{
				return m_Target.index;
			}
			const PointIndexObject& getTarget() const
			{
				return m_Target;
			}

			//Accessing source point
			const Point3D<int>& getSourcePoint() const
			{ 
				return m_Source.point; 
			}
			long getSourceIndex() const
			{ 
				return m_Source.index;
			}
			const PointIndexObject& getSource() const
			{
				return m_Source;
			}

		private:
			Region3D<int> m_Region, m_SafeRegion;
			boost::shared_ptr<ImageNeighborIterator> m_Iter;
			bool m_Accepted, m_End;
			std::queue<PointIndexObject> m_SourceQueue, m_TargetQueue;
			PointIndexObject m_Target, m_Source;

			void addTargetFromSource(const PointIndexObject& source) 
			{
				m_Iter->setOrigin(source.point, source.index);
				if (m_SafeRegion.contain(source.point)) {
					pcl_ForIterator(*m_Iter) {
						m_TargetQueue.push(PointIndexObject(m_Iter->getPoint(), *m_Iter));
					}
				} else {
					pcl_ForIterator(*m_Iter) {
						if (m_Region.contain(m_Iter->getPoint())) {
							m_TargetQueue.push(PointIndexObject(m_Iter->getPoint(), *m_Iter));
						}
					}
				}
				m_Source = source;
			}
		};

	}
}

#endif