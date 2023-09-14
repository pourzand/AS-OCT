#ifndef PCL_TREE_DEPTH_FIRST_ITERATOR
#define PCL_TREE_DEPTH_FIRST_ITERATOR

#include <pcl/iterator/IteratorMacro.h>
#include <pcl/macro.h>
#include <boost/utility.hpp>
#include <vector>
#include <stack>

namespace pcl
{
	namespace tree
	{

		template <class TreeType>
		class DepthFirstIterator: private boost::noncopyable
		{
		public:
			DepthFirstIterator(const TreeType& tree)
			{
				m_Tree = &tree;
				setStart(m_Tree->getRootId());
			}

			void setStart(int id, int start_gen=0)
			{
				m_StartId = id;
				m_StartGen = start_gen;
			}

			void setStart(const typename TreeType::NodeType& node, int start_gen=0) 
			{
				m_StartId = node.id();
				m_StartGen = start_gen;
			}

			void begin() const
			{
				if (m_StartId==-1) {
					m_End = true;
					return;
				}
				m_Id = m_StartId;
				m_Generation = m_StartGen;
				addChildrenToQueue(m_Id, m_Generation);
				m_End = false;
			}

			bool end() const
			{
				return m_End;
			}

			void next() const
			{
				if (m_Queue.empty()) {
					m_End = true;
					return;
				}
				IdGen info = m_Queue.top();
				m_Queue.pop();
				m_Id = info.id;
				m_Generation = info.gen;
				addChildrenToQueue(m_Id, m_Generation);
			}

			typename TreeType::NodeType get() const
			{
				return m_Tree->get(m_Id);
			}

			int getId() const
			{
				return m_Id;
			}

			int generation() const
			{
				return m_Generation;
			}

		protected:
			struct IdGen
			{
				int id;
				int gen;
				IdGen(int i, int g)
				{
					id = i;
					gen = g;
				}
			};

			TreeType const *m_Tree;
			int m_StartId, m_StartGen;
			mutable bool m_End;
			mutable int m_Id;
			mutable int m_Generation;
			mutable std::stack<IdGen> m_Queue;

			void addChildrenToQueue(int id, int gen) const
			{
				int cur_gen = gen+1;
				std::vector<int> children = m_Tree->getChildrenId(id);
				pcl_ForEach(children, item) m_Queue.push(IdGen(*item, cur_gen));
			}
		};

	}
}

#endif