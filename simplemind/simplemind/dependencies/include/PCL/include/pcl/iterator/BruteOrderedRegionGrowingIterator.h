#pragma once

#include <boost/container/map.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>
#include <utility>
#include <pcl/image.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/image/ImageAlgorithmObject.h>

namespace pcl
{
	namespace iterator
	{
		namespace details
		{
			struct PointIndexObjectHash
			{
				std::size_t operator()(const pcl::PointIndexObject& p) const
				{
					static boost::hash<long> hasher;
					return hasher(long(p));
				}
			};

			template <class PointType, class DistanceType, class PointHashFunc>
			class PointLookup
			{
			public:
				struct Iterator
				{
					typename boost::unordered_map<PointType, DistanceType, PointHashFunc>::iterator iter;
					bool is_null;

					const PointType& point() const
					{
						return iter->first;
					}

					const DistanceType& distance() const
					{
						return iter->second;
					}
				};

				PointLookup()
				{}

				bool empty()
				{
					return m_Point2DistLookup.empty();
				}

				std::pair<PointType, DistanceType> pop()
				{
					auto d2p_iter = m_Dist2PointLookup.begin();
					auto ret = std::make_pair(d2p_iter->second, d2p_iter->first);
					m_Dist2PointLookup.erase(d2p_iter);
					m_Point2DistLookup.erase(ret.first);
					return ret;
				}

				std::pair<PointType, DistanceType> peek()
				{
					auto d2p_iter = m_Dist2PointLookup.begin();
					return std::make_pair(d2p_iter->second, d2p_iter->first);
				}

				Iterator find(const PointType& p)
				{
					auto p2d_iter = m_Point2DistLookup.find(p);
					Iterator ret;
					if (p2d_iter == m_Point2DistLookup.end()) {
						ret.is_null = true;
					}
					else {
						ret.is_null = false;
						ret.iter = p2d_iter;
					}
					return ret;
				}

				void update(const PointType& p, const DistanceType& d)
				{
					auto iter = find(p);
					if (iter.is_null) {
						m_Point2DistLookup[p] = d;
						m_Dist2PointLookup.insert(std::make_pair(d, p));
					}
					else {
						update(iter, d);
					}
				}

				void update(const Iterator& iter, const DistanceType& d)
				{
					if (iter.is_null) return;
					if (d == iter.distance() || d > iter.distance()) return;
					auto range = m_Dist2PointLookup.equal_range(iter.distance());
					typename boost::container::multimap<DistanceType, PointType>::iterator it;
					for (auto i = range.first; i != range.second; ++i) {
						if (i->second == iter.point()) {
							it = i;
							break;
						}
					}
					m_Dist2PointLookup.erase(it);
					m_Dist2PointLookup.insert(std::make_pair(d, iter.point()));
					iter.iter->second = d;
				}

				void clear()
				{
					m_Dist2PointLookup.clear();
					m_Point2DistLookup.clear();
				}

			protected:
				boost::container::multimap<DistanceType, PointType> m_Dist2PointLookup;
				boost::unordered_map<PointType, DistanceType, PointHashFunc> m_Point2DistLookup;
			};

		}

		template <class DT>
		class BruteOrderedRegionGrowingIterator : private boost::noncopyable
		{
		public:
			typedef DT DistanceImageType;
			typedef boost::unordered_set<pcl::PointIndexObject, details::PointIndexObjectHash> FrontType;
			typedef typename DistanceImageType::IoValueType DistanceType;

			BruteOrderedRegionGrowingIterator() {}

			BruteOrderedRegionGrowingIterator(const typename DistanceImageType::Pointer& image, const ImageNeighborIterator::ConstantOffsetListPointer& offsets = ImageNeighborIterator::CreateConnect6Offset())
			{
				setDistanceImage(image, offsets);
			}

			void setDistanceImage(const typename DistanceImageType::Pointer& image, const ImageNeighborIterator::ConstantOffsetListPointer& offsets = ImageNeighborIterator::CreateConnect6Offset())
			{
				m_DistanceImage = image;
				m_AffectedRegion.reset();
				m_GrowIter.reset(new ImageNeighborIterator(m_DistanceImage, offsets));
				m_UpdateIter.reset(new ImageNeighborIterator(m_DistanceImage, ImageNeighborIterator::CreateConnect26Offset()));
				m_SafeRegion = pcl::Region3D<int>(
					m_DistanceImage->getMinPoint() + 1,
					m_DistanceImage->getMaxPoint() - 1
					);
				m_Dist.reserve(m_UpdateIter->size());
				pcl_ForEach(*(m_UpdateIter->getOffsetList()), item) {
					double x2 = image->getSpacing()[0] * (*item)[0],
						y2 = image->getSpacing()[1] * (*item)[1],
						z2 = image->getSpacing()[2] * (*item)[2];
					m_Dist.push_back(std::sqrt(x2*x2 + y2*y2 + z2*z2));
				}
			}

			//Methods for adding seeds
			void addSeed(const Point3D<int>& p)
			{
				addSeed(p, m_DistanceImage->localToIndex(p));
			}

			void addSeed(const Point3D<int>& p, long i)
			{
				m_Seed.insert(pcl::PointIndexObject(p, i));
			}

			void clearSeed()
			{
				m_Seed.clear();
			}

			const FrontType& getSeed()
			{
				return m_Seed;
			}

			//Iteration methods
			inline void begin()
			{
				m_End = false;
				m_Accepted = true;
				m_Front.clear();
				m_Candidate.clear();
				init();
				next();
			}

			void next() {
				if (m_End) return;
				if (m_Candidate.empty()) {
					m_End = true;
					return;
				}
				if (!m_Accepted) {
					updateFrontWithRejectPoint(m_Target, m_DistanceImage->get(m_Target));
					m_DistanceImage->set(m_Target, std::numeric_limits<DistanceType>::infinity());
				}
				m_Target = m_Candidate.pop().first;
				m_Accepted = false;
			}

			inline bool end()
			{
				return m_End;
			}

			void accept() {
				if (m_Accepted) return;
				m_Accepted = true;
				addFront(m_Target, m_DistanceImage->get(m_Target));
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
			const pcl::PointIndexObject& getTarget() const
			{
				return m_Target;
			}

			bool canPeek()
			{
				return !m_Candidate.empty();
			}

			pcl::PointIndexObject peek()
			{
				return m_Candidate.peek().first;
			}

			//Other
			typename DistanceImageType::ConstantPointer getDistanceImage() const
			{
				return m_DistanceImage;
			}

			const pcl::Region3D<int>& getAffectedRegion() const
			{
				return m_AffectedRegion;
			}

			const FrontType& getFront()
			{
				return m_Front;
			}

			const pcl::Point3D<double> getFrontCentroid() const
			{
				return m_FrontSum / m_Front.size();
			}

			DistanceType getFrontStep() const
			{
				return m_FrontStep;
			}

			pcl::iterator::ImageNeighborIterator& getGrowIterator()
			{
				return *m_GrowIter;
			}

			void reset()
			{
				m_Front.clear();
				m_Candidate.clear();
				pcl::ImageIterator iter(m_DistanceImage);
				iter.setRegion(m_AffectedRegion);
				pcl_ForIterator(iter) m_DistanceImage->set(iter, std::numeric_limits<DistanceType>::infinity());
				m_AffectedRegion.reset();
			}

			void resetDistanceImage()
			{
				pcl::ImageIterator iter(m_DistanceImage);
				iter.setRegion(m_AffectedRegion);
				pcl_ForIterator(iter) m_DistanceImage->set(iter, std::numeric_limits<DistanceType>::infinity());
			}

		private:
			//typedef typename DistanceImageType::IoValueType DistanceType;
			typename DistanceImageType::Pointer m_DistanceImage;
			pcl::Region3D<int> m_SafeRegion, m_AffectedRegion;
			boost::shared_ptr<pcl::iterator::ImageNeighborIterator> m_UpdateIter, m_GrowIter;
			std::vector<double> m_Dist;
			FrontType m_Seed;
			FrontType m_Front;
			details::PointLookup<pcl::PointIndexObject, DistanceType, details::PointIndexObjectHash> m_Candidate;
			bool m_Accepted, m_End;
			pcl::PointIndexObject m_Target;
			pcl::Point3D<double> m_FrontSum;
			DistanceType m_FrontStep;

			void init()
			{
				m_FrontSum.set(0, 0, 0);
				pcl::iterator::ImageNeighborIterator iter;
				iter = *m_GrowIter;
				pcl_ForEach(m_Seed, item) m_DistanceImage->set(*item, 0);
				pcl_ForEach(m_Seed, item) {
					iter.setOrigin(*item);
					bool is_enclosed = true;
					pcl_ForIterator(iter) {
						if (m_DistanceImage->get(iter)!=0) {
							is_enclosed = false;
							break;
						}
					}
					if (!is_enclosed) addFront(pcl::PointIndexObject(*item), 0, false /*No need to check and remove front as this is done by enclosing check*/);
				}
			}

#define UPDATE_SURROUNDING_DISTANCE \
	DistanceType dist = distance+m_Dist[m_UpdateIter->getIteration()]; \
	if (dist<m_DistanceImage->get(*m_UpdateIter)) { \
		if (m_DistanceImage->get(*m_UpdateIter)>dist) { \
			m_DistanceImage->set(*m_UpdateIter, dist); \
			m_AffectedRegion.add(m_UpdateIter->getPoint()); \
			auto candidate_item = m_Candidate.find(pcl::PointIndexObject(m_UpdateIter->getPoint(), m_UpdateIter->getIndex())); \
			if (!candidate_item.is_null) m_Candidate.update(candidate_item, dist); \
		} \
	}

#define ADD_NEW_CANDIDATE_AND_MARK_FRONT \
	auto pi = pcl::PointIndexObject(m_GrowIter->getPoint(), m_GrowIter->getIndex()); \
	if (distance<m_DistanceImage->get(*m_GrowIter)) { \
		m_Candidate.update(pi, m_DistanceImage->get(*m_GrowIter)); \
	} else if (check_and_remove_front) { \
		auto item = m_Front.find(pi); \
		if (m_Front.end()!=item) front_to_check.push_back(item); \
	}

#define FRONT_REMOVAL \
	auto temp = m_Candidate.find(pcl::PointIndexObject(*m_GrowIter)); \
	if (!temp.is_null) { \
		is_enclosed = false; \
		break; \
	}

			void addFront(const pcl::PointIndexObject& point, DistanceType distance, bool check_and_remove_front = true)
			{
				m_Front.insert(point);
				m_FrontSum += m_DistanceImage->toPhysicalCoordinate(point.point);
				m_FrontStep = distance;
				//Update surrounding distance
				m_UpdateIter->setOrigin(point.point, point.index);
				if (m_SafeRegion.contain(point.point)) {
					pcl_ForIterator(*m_UpdateIter) {
						UPDATE_SURROUNDING_DISTANCE;
					}
				} else {
					pcl_ForIterator(*m_UpdateIter) if (m_DistanceImage->contain(m_UpdateIter->getPoint())) {
						UPDATE_SURROUNDING_DISTANCE;
					}
				}
				//Adding new candidates and marking front for checking
				std::vector<FrontType::iterator> front_to_check;
				m_GrowIter->setOrigin(point.point, point.index);
				front_to_check.reserve(m_GrowIter->size());
				if (m_SafeRegion.contain(point.point)) {
					pcl_ForIterator(*m_GrowIter) {
						ADD_NEW_CANDIDATE_AND_MARK_FRONT;
					}
				} else {
					pcl_ForIterator(*m_GrowIter) if (m_DistanceImage->contain(m_GrowIter->getPoint())) {
						ADD_NEW_CANDIDATE_AND_MARK_FRONT;
					}
				}
				//Checking and removing front if needed
				pcl_ForEach(front_to_check, item) {
					m_GrowIter->setOrigin((*item)->point, (*item)->index);
					bool is_enclosed = true;
					if (m_SafeRegion.contain(point.point)) {
						pcl_ForIterator(*m_GrowIter) {
							FRONT_REMOVAL;
						}
					} else {
						pcl_ForIterator(*m_GrowIter) if (m_DistanceImage->contain(m_GrowIter->getPoint())) {
							FRONT_REMOVAL;
						}
					}
					if (is_enclosed) {
						m_FrontSum -= m_DistanceImage->toPhysicalCoordinate((*item)->point);
						m_Front.erase(*item);
					}
				}
			}

#define MARK_FRONT_FOR_CHECKING \
	auto pi = pcl::PointIndexObject(m_GrowIter->getPoint(), m_GrowIter->getIndex()); \
	if (m_DistanceImage->get(*m_GrowIter)<=distance) { \
		auto item = m_Front.find(pi); \
		if (m_Front.end()!=item) front_to_check.push_back(item); \
	}
			void updateFrontWithRejectPoint(const pcl::PointIndexObject& point, DistanceType distance)
			{
				//Adding new candidates and marking front for checking
				std::vector<FrontType::iterator> front_to_check;
				m_GrowIter->setOrigin(point.point, point.index);
				front_to_check.reserve(m_GrowIter->size());
				if (m_SafeRegion.contain(point.point)) {
					pcl_ForIterator(*m_GrowIter) {
						MARK_FRONT_FOR_CHECKING;
					}
				}
				else {
					pcl_ForIterator(*m_GrowIter) if (m_DistanceImage->contain(m_GrowIter->getPoint())) {
						MARK_FRONT_FOR_CHECKING;
					}
				}
				//Checking and removing front if needed
				pcl_ForEach(front_to_check, item) {
					m_GrowIter->setOrigin((*item)->point, (*item)->index);
					bool is_enclosed = true;
					if (m_SafeRegion.contain(point.point)) {
						pcl_ForIterator(*m_GrowIter) {
							FRONT_REMOVAL;
						}
					}
					else {
						pcl_ForIterator(*m_GrowIter) if (m_DistanceImage->contain(m_GrowIter->getPoint())) {
							FRONT_REMOVAL;
						}
					}
					if (is_enclosed) {
						m_FrontSum -= m_DistanceImage->toPhysicalCoordinate((*item)->point);
						m_Front.erase(*item);
					}
				}
			}
		};

	}
}
