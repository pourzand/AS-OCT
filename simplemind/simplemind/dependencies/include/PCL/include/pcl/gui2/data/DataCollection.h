#ifndef PCL_DATA_COLLECTION
#define PCL_DATA_COLLECTION

#include <vector>
#include <list>
#include <pcl/gui2/data/Data.h>
#include <pcl/gui2/ModifiableObject.h>
#include <pcl/macro.h>

namespace pcl
{
	namespace gui
	{

		class DataCollection: public ModifiableObject
		{
		public:
			typedef DataCollection Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

			static Pointer New()
			{
				return Pointer(new DataCollection());
			}

			bool add(const Data::Pointer& data)
			{
				resizeCollection(data->type());
				std::list<Data::Pointer>& list = m_Collection[data->type()];
				if (exist(list, data)) return false;
				list.push_back(data);
				this->modified();
				return true;
			}
			
			bool insertBefore(const Data::Pointer& data, const Data::Pointer& anchor)
			{
				if (data->type()!=anchor->type()) return false;
				std::list<Data::Pointer> *list = get(data->type());
				if (list==NULL) return false;
				pcl_ForEach(*list, item) {
					if (*item == anchor) {
						list->insert(item, data);
						this->modified();
						return true;
					}
				}
				return false;
			}

			bool remove(const Data::Pointer& data)
			{
				std::list<Data::Pointer> *list = get(data->type());
				if (list==NULL) return false;
				bool removed = false;
				list->remove_if([&](Data::Pointer& item)->bool {
					if (item==data) {
						removed = true;
						this->modified();
						return true;
					}
					return false;
				});
				return removed;
			}

			std::list<Data::Pointer>* get(Data::DataType type)
			{
				if (type>=m_Collection.size()) return NULL;
				auto res = &m_Collection[type];
				if (res->empty()) return NULL;
				return &m_Collection[type];
			}

			const std::list<Data::Pointer>* get(Data::DataType type) const
			{
				if (type>=m_Collection.size()) return NULL;
				auto res = &m_Collection[type];
				if (res->empty()) return NULL;
				return &m_Collection[type];
			}

			bool contain(const Data::Pointer& data) const
			{
				auto list = get(data->type());
				if (list==NULL) return false;
				return exist(*list, data);
			}

			bool empty() const
			{
				pcl_ForEach(m_Collection, item) {
					if (!item->empty()) return false;
				}
				return true;
			}

			int size() const
			{
				int sz = 0;
				pcl_ForEach(m_Collection, item) sz += item->size();
				return sz;
			}

		protected:
			std::vector<std::list<Data::Pointer>> m_Collection;

			DataCollection()
			{}

			void resizeCollection(int type)
			{
				if (m_Collection.size()<=type) m_Collection.resize(type+1);
			}

			bool exist(const std::list<Data::Pointer>& list, const Data::Pointer& data) const
			{
				pcl_ForEach(list, item) {
					if (*item == data) return true;
				}
				return false;
			}
		};

	}
}

#endif