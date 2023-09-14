#ifndef PCL_BINARY_TREE
#define PCL_BINARY_TREE

#include <pcl/exception.h>
#include <pcl/tree/Node.h>
#include <vector>
#include <queue>

namespace pcl
{
	namespace tree
	{

		struct BinaryTreeException: public pcl::Exception
		{};

		template <class DType>
		class BinaryTree
		{
		public:
			typedef BinaryTree Self;
			typedef DType DataType;
			typedef Node<Self> NodeType;

			BinaryTree()
			{
				m_RootId = -1;
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

				struct NewParentAndOldId
				{
					int new_parent, old_id;
					NewParentAndOldId(int n, int o)
					{
						new_parent = n;
						old_id = o;
					}
				};
				std::queue<NewParentAndOldId> queue;
				{
					//Initialize with root
					int parent = add(std::move(data[root]));
					int *children = relationship[root].children;
					for (int i=0; i<2; ++i) queue.push(NewParentAndOldId(parent, children[i]));
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
				std::vector<int> result;
				Relationship &relationship = m_Relationship[id];
				for (int i=0; i<2; ++i) if (relationship[i]>=0) result.push_back(getNode(relationship[i]));
				return std::move(result);
			}

			std::vector<NodeType> getChildren(const NodeType& node) const
			{
				return std::move(getChildren(node.id()));
			}

			std::vector<int> getChildrenId(int id) const
			{
				std::vector<int> result;
				const Relationship &relationship = m_Relationship[id];
				for (int i=0; i<2; ++i) if (relationship.children[i]>=0) result.push_back(relationship.children[i]);
				return std::move(result);
			}

			std::vector<int> getChildrenId(const NodeType& node) const
			{
				return std::move(getChildrenId(node.id()));
			}

			int getChildrenNum(int id) const
			{
				const int* children = m_Relationship[id].children;
				return (children[0]>=0?1:0) + (children[1]>=0?1:0);
			}

			NodeType getChild(const NodeType& node, int child_index) const
			{
				return getChild(node.id(), child_index);
			}

			NodeType getChild(int id, int child_index) const
			{
				return get(m_Relationship[id].children[child_index]);
			}

			NodeType getChildId(const NodeType& node, int child_index) const
			{
				return getChildId(node.id(), child_index);
			}

			NodeType getChildId(int id, int child_index) const
			{
				return m_Relationship[id].children[child_index];
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

			int add(int parent_id, const DataType& data, bool nothrow=true)
			{
				if (m_RootId==-1 || parent_id==-1) return add(data);
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				int id = static_cast<int>(m_Data.size());
				if (!addChildrenTo(parent_id, id, nothrow)) return std::numeric_limits<int>::lowest();
				m_Data.push_back(data);
				m_Relationship.push_back(Relationship(parent_id));
				return id;
			}

			int add(int parent_id, DataType&& data, bool nothrow=true)
			{
				if (m_RootId==-1 || parent_id==-1) return add(std::move(data));
				if (parent_id>=m_Data.size() || parent_id<0) return std::numeric_limits<int>::lowest();
				int id = static_cast<int>(m_Data.size());
				if (!addChildrenTo(parent_id, id, nothrow)) return std::numeric_limits<int>::lowest();
				m_Data.push_back(std::move(data));
				m_Relationship.push_back(Relationship(parent_id));
				return id;
			}

			int add(const NodeType& parent, const DataType& data, bool nothrow=true)
			{
				return add(parent.id(), data, nothrow);
			}

			int add(const NodeType& parent, DataType&& data, bool nothrow=true)
			{
				return add(parent.id(), std::move(data), nothrow);
			}

			int add(const Self& data)
			{
				m_Relationship = data.m_Relationship;
				m_Data = data.m_Data;
				m_RootId = data.m_RootId;
			}

			int add(Self&& data)
			{
				m_Relationship = std::move(data.m_Relationship);
				m_Data = std::move(data.m_Data);
				m_RootId = data.m_RootId;
				data.clear();
			}

			int add(int parent_id, const Self& data, bool nothrow=true)
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
					int parent = add(info.parent, data.data(info.old_id), nothrow);
					if (parent<0) return std::numeric_limits<int>::lowest();
					if (init) {
						return_val = parent;
						init = false;
					}
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				return return_val;
			}

			int add(int parent_id, Self&& data, bool nothrow=true)
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
					int parent = add(info.parent, std::move(data.data(info.old_id)), nothrow);
					if (parent<0) return std::numeric_limits<int>::lowest();
					if (init) {
						return_val = parent;
						init = false;
					}
					pcl_ForEach(children, item) queue.push(NewParentOldId(parent,*item));
				}
				data.clear();
				return return_val;
			}

			int add(const NodeType& parent, const Self& data, bool nothrow=true)
			{
				return add(parent, data, nothrow);
			}

			int add(const NodeType& parent, Self&& data, bool nothrow=true)
			{
				return add(parent, std::move(data), nothrow);
			}

			// Custom add operation

			void customAdd(int id, int parent_id, const DataType& data)
			{
				if (parent_id==-1 && m_RootId!=-1) pcl_ThrowException(Exception(), "Multiple root detected!");
				resizeDataTo(id);
				if (m_Relationship[id].parent!=-1) pcl_ThrowException(Exception(), "Duplicated ID ("+boost::lexical_cast<std::string>(id)+") detected!");
				if (parent_id==-1) m_RootId = id;
				else addChildrenTo(parent_id, id, false);
				m_Data[id] = data;
				m_Relationship[id].parent = parent_id;
			}

			void customAdd(int id, int parent_id, DataType&& data)
			{
				if (parent_id==-1 && m_RootId!=-1) pcl_ThrowException(Exception(), "Multiple root detected!");
				resizeDataTo(id);
				if (m_Relationship[id].parent!=-1) pcl_ThrowException(Exception(), "Duplicated ID ("+boost::lexical_cast<std::string>(id)+") detected!");
				if (parent_id==-1) m_RootId = id;
				else addChildrenTo(parent_id, id, false);
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
					removeChildrenIn(parent, id);
				}
				//Deleting subtree
				std::queue queue;
				queue.push(id);
				while (!queue.empty()) {
					int cur_id = queue.front();
					queue.pop();
					Relationship &relationship = m_Relationship[cur_id];
					for (int i=0; i<2; ++i) queue.push(relationship.children[i]);
					relationship.clear();
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
			struct Relationship
			{
				int parent;
				int children[2];

				Relationship()
				{
					parent = -1;
					children[0] = -1;
					children[1] = -1;
				}

				Relationship(int p)
				{
					parent = p;
					children[0] = -1;
					children[1] = -1;
				}

				void clear()
				{
					parent = -1;
					children[0] = -1;
					children[1] = -1;
				}
			};

			std::vector<DataType> m_Data;
			int m_RootId;
			std::vector<Relationship> m_Relationship;

			bool addChildrenTo(int target_id, int child_id, bool nothrow)
			{
				Relationship &relationship = m_Relationship[target_id];
				bool added = false;
				for (int i=0; i<2; ++i) if (relationship.children[i]==-1) {
					relationship.children[i] = child_id;
					added = true;
					break;
				}
				if (added) return true;
				else {
					if (nothrow) return false;
					else pcl_ThrowException(BinaryTreeException(), "Attempt to add children to a full node!");
				}
			}

			void removeChildrenIn(int traget_id, int child_id)
			{
				Relationship &relationship = m_relationship[target_id];
				for (int i=0; i<2; ++i) if (relationship[i]==child_id) {
					relationship[i] = -1;
					break;
				}
			}

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