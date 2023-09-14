#ifndef PCL_INDEXED_DATA_SORTED
#define PCL_INDEXED_DATA_SORTED

#include <vector>
#include <boost/utility.hpp>

namespace pcl
{
	namespace misc
	{

		template <class DataType, bool Ascending, class IndexType=int>
		class IndexedDataSorter
		{
		public:
			IndexedDataSorter() 
			{}

			void add(const DataType& data) 
			{
				m_Storage.push_back(Pair());
				Pair &elem = m_Storage.back();
				elem.data = data;
				elem.index = static_cast<IndexType>(m_Storage.size()-1);
			}

			void clear() 
			{ 
				m_Storage.clear(); 
			}

			void sort() 
			{
				std::sort(m_Storage.begin(), m_Storage.end());
			}

			void unique() 
			{
				typename std::vector<Element>::iterator new_end = std::unique(m_Storage.begin(), m_Storage.end());
				m_Storage.resize(new_end-m_Storage.begin());
			}

			int size() 
			{ 
				return m_Storage.size(); 
			}

			const DataType& get(int i) const
			{ 
				return m_Storage[i].data; 
			}

			const IndexType& getIndex(int i) const
			{ 
				return m_Storage[i].index; 
			}

		protected:
			struct Pair 
			{
				DataType data;
				IndexType index;

				Pair() 
				{}

				bool operator<(const Pair& b) const
				{
					return compare<Ascending>(b);
				}

				bool operator==(const Pair& b) const
				{
					if (data==b.data) return true;
					return false;
				}

			protected:
				template <bool Asc>
				typename boost::enable_if_c<Asc, bool>::type compare(const Pair& b) const
				{
					return data<b.data;
				}

				template <bool Asc>
				typename boost::disable_if_c<Asc, bool>::type compare(const Pair& b) const
				{
					return data>b.data;
				}
			};

			std::vector<Pair> m_Storage;
		};

	}
}

#endif