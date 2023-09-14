#ifndef PCL_TREE
#define PCL_TREE

#include <pcl/tree/Node.h>
#include <pcl/macro.h>
#include <pcl/exception.h>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <queue>

namespace pcl
{
	namespace tree
	{

		template <class DType>
		class Tree
		{
		public:
			typedef Tree Self;
			typedef DType DataType;
			typedef Node<Self> NodeType;

			Tree()
			{
				m_RootId = -1;
			}

			Tree(const Tree& obj)
			{
				m_RootId = obj.m_RootId;
				m_Relationship = obj.m_Relationship;
				m_Data = obj.m_Data;
			}

			int dataSize() const
			{
				return static_cast<int>(m_Data.size());
			}

			int size() const
			{
				int size = 0;
				for (int i=0; i<m_Relationship.size(); ++i) {
					if (m_Relationship[i].parent==-1 && i!=m_RootId);
					else ++size;
				}
				return size;
			}

			void clear()
			{
				m_RootId = -1;
				m_Relationship.clear();
				m_Data.clear();
			}

			void squeeze()
			{
				if (m_RootId<0) return;

				std::vector<Relationship> relationship = std::move(m_Relationship);
				std::vector<DataType> data = std::move(m_Data);
				int root = m_RootId;

				std::queue<NewParentOldId> queue;
				{
					//Initialize with root
					int parent = add(std::move(data[root]));
					std::vector<int> &children = relationship[root].children;
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				while (!queue.empty()) {
					NewParentOldId info = queue.front();
					queue.pop();
					int parent = add(info.new_parent, std::move(data[info.old_id]));
					std::vector<int> &children = relationship[info.old_id].children;
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
			}

			const DataType& data(int id) const
			{
				return m_Data[id];
			}

			const DataType& data(const NodeType& node) const
			{
				return m_Data[node.id()];
			}

			DataType& data(int id)
			{
				return m_Data[id];
			}

			DataType& data(const NodeType& node)
			{
				return m_Data[node.id()];
			}

			//Assignment

			Tree& operator=(const Tree& obj)
			{
				m_RootId = obj.m_RootId;
				m_Relationship = obj.m_Relationship;
				m_Data = obj.m_Data;
				return *this;
			}

			Tree& operator=(Tree&& obj)
			{
				m_RootId = obj.m_RootId;
				m_Relationship = std::move(obj.m_Relationship);
				m_Data = std::move(obj.m_Data);
				obj.clear();
				return *this;
			}

			//Get operation

			NodeType get(int id) const
			{
				return NodeType(id, this);
			}

			NodeType getRoot() const
			{
				return get(m_RootId);
			}

			int getRootId() const
			{
				return m_RootId;
			}

			std::vector<NodeType> getChildren(int id) const
			{
				std::vector<NodeType> result;
				std::vector<int>& children = m_Relationship[id].children;
				if (!children.empty()) {
					result.reserve(children.size());
					pcl_ForEach(children, item) result.push_back(getNode(*item));
				}
				return std::move(result);
			}

			std::vector<NodeType> getChildren(const NodeType& node) const
			{
				return std::move(getChildren(node.id()));
			}

			std::vector<int> getChildrenId(int id) const
			{
				std::vector<int> result;
				const std::vector<int>& children = m_Relationship[id].children;
				if (!children.empty()) {
					result.reserve(children.size());
					pcl_ForEach(children, item) result.push_back(*item);
				}
				return std::move(result);
			}

			int getChildrenNum(int id) const
			{
				return m_Relationship[id].children.size();
			}

			NodeType getChild(const NodeType& node, int child_index) const
			{
				return getChild(node.id(), child_index);
			}

			NodeType getChild(int id, int child_index) const
			{
				return getNode(m_Relationship[id].children[child_index]);
			}

			NodeType getChildId(const NodeType& node, int child_index) const
			{
				return getChildId(node.id(), child_index);
			}

			NodeType getChildId(int id, int child_index) const
			{
				return m_Relationship[id].children[child_index];
			}

			std::vector<int> getChildrenId(const NodeType& node) const
			{
				return std::move(getChildrenId(node.id()));
			}

			NodeType getParent(int id) const
			{
				if (id==-1 || id>=m_Data.size() || m_Relationship[id].parent==-1) return NodeType();
				return NodeType(m_Relationship[id].parent, this);
			}

			int getParentId(int id) const
			{
				if (id==-1) return -1;
				return m_Relationship[id].parent;
			}

			NodeType getParent(const NodeType& node) const
			{
				return getParent(node.isNull()?-1:node.id());
			}

			int getParentId(const NodeType& node) const
			{
				return getParentId(node.isNull()?-1:node.id());
			}

			//Add operation

			int add(const DataType& root_data)
			{
				clear();
				m_RootId = 0;
				m_Data.push_back(root_data);
				m_Relationship.push_back(Relationship(-1));
				return m_RootId;
			}

			int add(DataType&& root_data)
			{
				clear();
				m_RootId = 0;
				m_Data.push_back(std::move(root_data));
				m_Relationship.push_back(Relationship(-1));
				return m_RootId;
			}

			int add(int parent_id, const DataType& data)
			{
				if (m_RootId==-1 || parent_id==-1) return add(data);
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				m_Data.push_back(data);
				int id = m_Data.size()-1;
				m_Relationship[parent_id].children.push_back(id);
				m_Relationship.push_back(Relationship(parent_id));
				return id;
			}

			int add(int parent_id, DataType&& data)
			{
				if (m_RootId==-1 || parent_id==-1) return add(std::move(data));
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				m_Data.push_back(std::move(data));
				int id = static_cast<int>(m_Data.size())-1;
				m_Relationship[parent_id].children.push_back(id);
				m_Relationship.push_back(Relationship(parent_id));
				return id;
			}

			int add(const NodeType& parent, const DataType& data)
			{
				return add(parent.id(), data);
			}

			int add(const NodeType& parent, DataType&& data)
			{
				return add(parent.id(), std::move(data));
			}

			int add(const Self& data)
			{
				m_Relationship = data.m_Relationship;
				m_Data = data.m_Data;
				m_RootId = data.m_RootId;
				return m_RootId;
			}

			int add(Self&& data)
			{
				m_Relationship = std::move(data.m_Relationship);
				m_Data = std::move(data.m_Data);
				m_RootId = data.m_RootId;
				data.clear();
				return m_RootId;
			}

			int add(int parent_id, const Self& data)
			{
				if (m_RootId==-1 || parent_id==-1) return add(data);
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				std::queue<NewParentOldId> queue;
				queue.add(parent_id, data.getRootId());
				int return_val;
				bool init = true;
				while (!queue.empty()) {
					NewParentAndOldId info = queue.front();
					queue.pop();
					std::vector<int> children = data.getChildrenId(info.old_id);
					int parent = add(info.parent, data.data(info.old_id));
					if (init) {
						return_val = parent;
						init = false;
					}
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				return return_val;
			}

			int add(int parent_id, Self&& data)
			{
				if (m_RootId==-1 || parent_id==-1) return add(std::move(data));
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				std::queue<NewParentOldId> queue;
				queue.add(parent_id, data.getRootId());
				int return_val;
				bool init = true;
				while (!queue.empty()) {
					NewParentAndOldId info = queue.front();
					queue.pop();
					std::vector<int> children = data.getChildrenId(info.old_id);
					int parent = add(info.parent, std::move(data.data(info.old_id)));
					if (init) {
						return_val = parent;
						init = false;
					}
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				data.clear();
				return return_val;
			}

			int add(const NodeType& parent, const Self& data)
			{
				return add(parent, data);
			}

			int add(const NodeType& parent, Self&& data)
			{
				return add(parent, std::move(data));
			}

			// Custom add operation

			void customAdd(int id, int parent_id, const DataType& data)
			{
				if (parent_id==-1 && m_RootId!=-1) pcl_ThrowException(Exception(), "Multiple root detected!");
				resizeDataTo(id);
				if (m_Relationship[id].parent!=-1) pcl_ThrowException(Exception(), "Duplicated ID ("+boost::lexical_cast<std::string>(id)+") detected!");
				if (parent_id==-1) m_RootId = id;
				else m_Relationship[parent_id].children.push_back(id);
				m_Data[id] = data;
				m_Relationship[id].parent = parent_id;
			}

			void customAdd(int id, int parent_id, DataType&& data)
			{
				if (parent_id==-1 && m_RootId!=-1) pcl_ThrowException(Exception(), "Multiple root detected!");
				resizeDataTo(id);
				if (m_Relationship[id].parent!=-1) pcl_ThrowException(Exception(), "Duplicated ID ("+boost::lexical_cast<std::string>(id)+") detected!");
				if (parent_id==-1) m_RootId = id;
				else m_Relationship[parent_id].children.push_back(id);
				m_Data[id] = std::move(data);
				m_Relationship[id].parent = parent_id;
			}

			//Remove operation

			void remove(int id)
			{
				if (id==-1) return;
				if (id==m_RootId) {
					clear();
					return;
				}
				//Managing parent
				{
					int parent = getParentId(id);
					std::vector<int>& children = m_Relationship[parent].children;
					std::vector<int> new_children;
					pcl_ForEach(children, item) if (*item!=id) new_children.push_back(*item);
					children = std::move(new_children);
				}
				//Deleting subtree
				std::queue<int> queue;
				queue.push(id);
				while (!queue.empty()) {
					int cur_id = queue.front();
					queue.pop();
					std::vector<int>& children = m_Relationship[cur_id].children;
					pcl_ForEach(children, item) queue.push(*item);
					m_Relationship[cur_id].clear();
					m_Data[cur_id] = std::move(DataType());
				}
			}

			void remove(const NodeType& node)
			{
				remove(node.id());
			}

			//Subtree operation

			Self detachSubtree(int root_id)
			{
				Self target;
				if (root_id==-1 || m_RootId==-1) return std::move(target);
				if (root_id>=m_Data.size() || root_id<0) return Self();
				std::queue<NewParentOldId> queue;
				queue.add(-1, root_id);
				bool init = true;
				while (!queue.empty()) {
					NewParentAndOldId info = queue.front();
					queue.pop();
					std::vector<int> children = getChildrenId(info.old_id);
					int parent = add(info.parent, std::move(data(info.old_id)));
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				remove(root_id);
				return std::move(target);
			}

			Self extractSubtree(const NodeType& root_node) const
			{
				Self target;
				if (root_id==-1 || m_RootId==-1) return std::move(target);
				if (root_id>=m_Data.size() || root_id<0) return Self();
				std::queue<NewParentOldId> queue;
				queue.add(-1, root_id);
				bool init = true;
				while (!queue.empty()) {
					NewParentAndOldId info = queue.front();
					queue.pop();
					std::vector<int> children = getChildrenId(info.old_id);
					int parent = add(info.parent, data(info.old_id));
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				return std::move(target);
			}

		protected:
			struct NewParentOldId
			{
				int new_parent, old_id;
				NewParentOldId(int n, int o)
				{
					new_parent = n;
					old_id = o;
				}
			};

			struct Relationship
			{
				int parent;
				std::vector<int> children;

				Relationship()
				{
					parent = -1;
				}

				Relationship(int p)
				{
					parent = p;
				}

				void clear()
				{
					parent = -1;
					children.clear();
				}
			};

			std::vector<DataType> m_Data;
			int m_RootId;
			std::vector<Relationship> m_Relationship;

			void resizeDataTo(int id)
			{
				int size = id+1;
				if (m_Data.size()<size) {
					m_Data.resize(size);
					m_Relationship.resize(size);

				}
			}
		};

	}
}

#endif