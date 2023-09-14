#ifndef PCL_TREE_REVERSE_ITERATOR
#define PCL_TREE_REVERSE_ITERATOR

#include <pcl/iterator/IteratorMacro.h>
#include <pcl/macro.h>
#include <boost/utility.hpp>

namespace pcl
{
	namespace tree
	{

		template <class TreeType>
		class ReverseIterator: private boost::noncopyable
		{
		public:
			ReverseIterator(const TreeType& tree)
			{
				m_Tree = &tree;
				setStart(m_Tree->getRootId());
			}

			ReverseIterator(const TreeType& tree, const typename TreeType::NodeType& start)
			{
				m_Tree = &tree;
				setStart(start);
			}

			ReverseIterator(const TreeType& tree, int start)
			{
				m_Tree = &tree;
				setStart(start);
			}

			void setStart(int id)
			{
				m_StartId = id;
			}

			void setStart(const typename TreeType::NodeType& node) 
			{
				m_StartId = node.id();
			}

			void begin() const
			{
				if (m_StartId>=m_Tree->dataSize()) m_Id = -1;
				else m_Id = m_StartId;
			}

			bool end() const
			{
				return m_Id<0;
			}

			void next() const
			{
				m_Id = m_Tree->getParentId(m_Id);
			}

			typename TreeType::NodeType get() const
			{
				return m_Tree->get(m_Id);
			}

			int getId() const
			{
				return m_Id;
			}

		protected:
			TreeType const *m_Tree;
			int m_StartId;
			mutable int m_Id;
		};

	}
}

#endif