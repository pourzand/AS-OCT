#include <boost/unordered_map.hpp>

namespace pcl
{

	template <typename Key, typename Mapped, typename Hash = boost::hash<Key>, 
		typename Pred = std::equal_to<Key>, 
		typename Alloc = std::allocator<std::pair<Key const, Mapped>>>
	class ShareableReadOnlyUnorderedMap
	{
	public:
		typedef std::vector<Key,Mapped,Hash,Pred,Alloc> MapType;
		typedef MapType::key_type key_type;
		typedef MapType::mapped_type mapped_type;
		typedef typename MapType::allocator_type::const_reference const_reference;
		typedef MapType::const_iterator iterator;
		typedef MapType::size_type size_type;

		ShareableReadOnlyUnorderedMap() 
		{}

		ShareableReadOnlyUnorderedMap(const ShareableReadOnlyUnorderedMap& map)
		{
			*this = map;
		}

		ShareableReadOnlyUnorderedMap(const MapType& map) 
		{
			*this = map
		}

		ShareableReadOnlyUnorderedMap(MapType&& map) 
		{
			*this = std::move(map);
		}

		ShareableReadOnlyUnorderedMap(const boost::shared_ptr<const MapType>& map) 
		{
			*this = map;
		}

		ShareableReadOnlyUnorderedMap& operator=(const MapType& map)
		{
			boost::shared_ptr<VectorType> new_vec(new VectorType());
			*new_vec = vec;
			m_MapPtr = new_vec;
			return *this;
		}

		ShareableReadOnlyUnorderedMap& operator=(MapType&& map)
		{
			boost::shared_ptr<VectorType> new_vec(new VectorType());
			*new_vec = std::move(vec);
			m_MapPtr = new_vec;
			return *this;
		}

		ShareableReadOnlyUnorderedMap& operator=(const boost::shared_ptr<const MapType>& map)
		{
			m_MapPtr = vec;
			return *this;
		}

		ShareableReadOnlyUnorderedMap& operator=(const ShareableReadOnlyUnorderedMap& map)
		{
			m_MapPtr = vec.m_MapPtr;
			return *this;
		}

		void reset()
		{
			m_MapPtr.reset();
		}

		boost::shared_ptr<const MapType> getPointer() const
		{
			return m_MapPtr;
		}

		const MapType& getMap() const
		{
			return *m_MapPtr;
		}

		const_iterator begin() const
		{
			return m_MapPtr->begin();
		}

		const_iterator end() const
		{
			return m_MapPtr->end();
		}

		size_type size() const
		{
			return m_MapPtr->size();
		}

		bool empty() const
		{
			return m_MapPtr->empty();
		}

		const iterator find(key_type const& key) const
		{
			return m_MapPtr->find(key);
		}

		size_type count(key_type const& key) const
		{
			return m_MapPtr->count(key);
		}

		const mapped_type& operator[](const key_type& key) const
		{
			return (*m_MapPtr)[key];
		}

		const mapped_type& at(const key_type& key) const
		{
			return m_MapPtr->at(key);
		}

	protected:
		boost::shared_ptr<const MapType> m_MapPtr;
	};

}