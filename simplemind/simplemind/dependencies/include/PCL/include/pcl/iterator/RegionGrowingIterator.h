#ifndef PCL_REGION_GROWING_ITERATOR
#define PCL_REGION_GROWING_ITERATOR

#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <queue>

namespace pcl
{
	namespace iterator
	{

		template <class MarkerImageType>
		class RegionGrowingIterator: private boost::noncopyable
		{
		public:
			typedef typename MarkerImageType::IoValueType MarkerValueType;

			RegionGrowingIterator(MarkerValueType label=1) 
			{
				setLabel(label);
			}

			void setLabel(MarkerValueType label)
			{
				m_Label = label;
			}

			//Methods for setting up buffer
			template <class ImagePointer>
			void setMarkerBasedOn(const ImagePointer& image) 
			{ 
				m_Marker = MarkerImageType::New(image);
				ImageHelper::Fill(m_Marker, 0);
			}
			
			void setMarkerImage(const typename MarkerImageType::Pointer &marker_image) 
			{
				m_Marker = marker_image;
			}

			//Method for setting up neighbor iterator
			void setNeighborIterator(const ImageNeighborIterator::ConstantOffsetListPointer& ptr)
			{
				m_Iter.reset(new ImageNeighborIterator);
				*m_Iter = ptr;
			}
			
			void setNeighborIterator(const boost::shared_ptr<ImageNeighborIterator>& ptr)
			{
				m_Iter = ptr;
			}


			//Methods for getting internal data
			typename MarkerImageType::Pointer getMarker()
			{
				return m_Marker;
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
			bool addSeed(const Point3D<int>& p, bool force_add=false) 
			{
				return addSeed(p, m_Marker->toIndex(p), force_add);
			}
			bool addSeed(const Point3D<int>& p, long index, bool force_add=false) 
			{
				if (m_Marker->get(index)==m_Label && !force_add) return false;
				m_Marker->set(index, m_Label);
				m_SourceQueue.push(PointIndexObject(p,index));
				return true;
			}

			//Iteration methods
			inline void begin() 
			{
				if (!m_Iter->initialized()) m_Iter->setImage(m_Marker);
				misc::SafeUnsafeRegionGenerator safe_region_gen(m_Marker->getRegion(), m_Marker->getRegion(), m_Iter->getOffsetRegion());
				m_SafeRegion = safe_region_gen.getSafeRegion();
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
				m_Marker->set(m_Target.index, m_Label);
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
			const Point3D<int>& getSourcePoint() 
			{ 
				return m_Source.point; 
			}
			long getSourceIndex() 
			{ 
				return m_Source.index;
			}
			const PointIndexObject& getSource() const
			{
				return m_Source;
			}

		private:
			MarkerValueType m_Label;
			typename MarkerImageType::Pointer m_Marker;
			boost::shared_ptr<ImageNeighborIterator> m_Iter;
			bool m_Accepted, m_End;
			std::queue<PointIndexObject> m_SourceQueue, m_TargetQueue;
			PointIndexObject m_Target, m_Source;
			Region3D<int> m_SafeRegion;

			void addTargetFromSource(const PointIndexObject& source) 
			{
				m_Iter->setOrigin(source.point, source.index);
				if (m_SafeRegion.contain(source.point)) {
					pcl_ForIterator(*m_Iter) {
						if (m_Marker->get(*m_Iter)!=m_Label) {
							m_TargetQueue.push(PointIndexObject(m_Iter->getPoint(), *m_Iter));
						}
					}
				} else {
					pcl_ForIterator(*m_Iter) {
						if (m_Marker->contain(m_Iter->getPoint()) && m_Marker->get(*m_Iter)!=m_Label) {
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