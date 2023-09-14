#ifndef PCL_SHAPE
#define PCL_SHAPE

#include <pcl/geometry.h>
#include <vector>

namespace pcl
{
	namespace shapemodel
	{

		template <class PType>
		class PointCloud
		{
		public:
			typedef PType PointType;
			typedef typename PointType::ValueType ValueType;
			static const int Dimension = PointType::Dimension;
			typedef std::vector<ValueType> StorageType;
			typedef PointCloud Self;

			PointCloud()
			{}

			PointCloud(const Self& obj)
			{
				operator=(obj);
			}

			StorageType& getData()
			{
				return m_Data;
			}

			const StorageType& getData() const
			{
				return m_Data;
			}

			int size() const
			{
				return m_Data.size();
			}

			void resize(int sz)
			{
				m_Data.resize(sz);
			}

			ValueType& get(int i)
			{
				return m_Data[i];
			}

			ValueType get(int i) const
			{
				return m_Data[i];
			}

			ValueType& at(int i)
			{
				return m_Data[i];
			}

			ValueType at(int i) const
			{
				return m_Data[i];
			}

			ValueType& operator[](int i)
			{
				return m_Data[i];
			}

			ValueType operator[](int i) const
			{
				return m_Data[i];
			}

			//Point related

			int pointSize() const
			{
				return m_Data.size()/Dimension;
			}

			void resizePoint(int sz)
			{
				m_Data.resize(sz*Dimension);
			}

			PointType getPoint(int index) const
			{
				PointType result;
				int p_ind = index*Dimension;
				for (int i=0; i<Dimension; ++i) {
					result[i] = m_Data[p_ind];
					++p_ind;
				}
				return result;
			}

			template <class PT>
			void setPoint(int index, const PT& point)
			{
				int p_ind = index*Dimension;
				for (int i=0; i<Dimension; ++i) {
					m_Data[p_ind] = point[i];
					++p_ind;
				}
			}

			void add(const PointType& point)
			{
				for (int i=0; i<Dimension; ++i) m_Data.push_back(point[i]);
			}

			template <class T>
			void add(const T& point)
			{
				int cur_dim = pcl_Min(Dimension, T::Dimension);
				for (int i=0; i<cur_dim; ++i) m_Data.push_back(point[i]);
				for (int i=cur_dim; i<Dimension; ++i) m_Data.push_back(0);
			}

			template <class ListType>
			void addList(const ListType& list)
			{
				pcl_ForEach(list, item) add(*item);
			}
			
			std::vector<PointType> getVector() const
			{
				std::vector<PointType> result;
				result.reserve(pointSize());
				for (int i=0; i<pointSize(); ++i) result.push_back(getPoint(i));
				return std::move(result);
			}

			//Assigment

			template <class ListType>
			Self& operator=(const ListType& list)
			{
				m_Data.clear();
				pcl_ForEach(list, item) m_Data.push_back(*item);
				return *this;
			}

			Self& operator=(StorageType&& list)
			{
				m_Data = std::move(list);
				return *this;
			}

            Self& operator=(Self& obj)
            {
                m_data = obj.m_data;
                return *this;
            }

			Self& operator=(Self&& obj)
			{
				m_Data = std::move(obj.m_Data);
				return *this;
			}

			//Iterator

			typename StorageType::iterator begin()
			{
				return m_Data.begin();
			}

			typename StorageType::const_iterator begin() const
			{
				return m_Data.begin();
			}

			typename StorageType::iterator end() 
			{
				return m_Data.end();
			}

			typename StorageType::const_iterator end() const
			{
				return m_Data.end();
			}

		protected:
			StorageType m_Data;
		};

	}
}

#endif