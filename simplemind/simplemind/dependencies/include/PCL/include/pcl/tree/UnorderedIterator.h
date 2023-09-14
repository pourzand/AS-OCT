#ifndef PCL_TREE_UNORDERED_ITERATOR
#define PCL_TREE_UNORDERED_ITERATOR

#include <pcl/iterator/IteratorMacro.h>
#include <pcl/macro.h>
#include <boost/utility.hpp>

namespace pcl
{
	namespace tree
	{

		template <class TreeType>
		class UnorderedIterator: private boost::noncopyable
		{
		public:
			UnorderedIterator(const TreeType& tree)
			{
				m_Tree = &tree;
			}

			void begin() const
			{
				m_Id = -1;
				m_End = false;
				next();
			}

			bool end() const
			{
				return m_End;
			}

			void next() const
			{
				do {
					++m_Id;
					if (m_Id>=m_Tree->dataSize()) {
						m_End = true;
						return;
					}
				} while (m_Tree->getParentId(m_Id)==-1 && m_Tree->getRootId()!=m_Id);
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
			mutable int m_Id;
			mutable bool m_End;
		};

	}
}

#endif