#ifndef PCL_TREE_NODE
#define PCL_TREE_NODE

#include <vector>

namespace pcl
{
	namespace tree
	{

		template <class TreeType>
		class Node
		{
		public:
			Node()
			{
				m_Tree = NULL:
			}

			Node(int id, const TreeType* tree)
			{
				m_Id = id;
				m_Tree = const_cast<TreeType*>(tree);
			}

			Node(const Node& node)
			{
				m_Id = node.m_Id;
				m_Tree = node.m_Tree;
			}

			bool isNull() const
			{
				return m_Tree==NULL;
			}

			Node& operator=(const Node& node)
			{
				m_Id = node.m_Id;
				m_Tree = node.m_Tree;
				return *this;
			}

			int id() const
			{
				return m_Id;
			}

			typename TreeType::DataType& data()
			{
				return m_Tree->data(m_Id);
			}

			const typename TreeType::DataType& data() const
			{
				return m_Tree->data(m_Id);
			}

			Node parent() const
			{
				return m_Tree->getParent(*this);
			}
			
			int parentId() const
			{
				return m_Tree->getParentId(*this);
			}

			std::vector<Node> children() const
			{
				return m_Tree->getChildren(*this);
			}

			std::vector<int> childrenId() const
			{
				return m_Tree->getChildrenId(*this);
			}
			
			int childrenNum() const
			{
				return m_Tree->getChildrenNum(m_Id);
			}
			
			int childId(int index) const
			{
				return m_Tree->getChildId(m_Id, index);
			}
			
			Node child(int index) const
			{
				return m_Tree->getChild(m_Id, index);
			}

		private:
			int m_Id;
			TreeType* m_Tree;
		};

	}
}

#endif