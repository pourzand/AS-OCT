#ifndef PCL_POINT_FILTER_BASE
#define PCL_POINT_FILTER_BASE

#include <boost/numeric/conversion/bounds.hpp>

namespace pcl
{
	namespace filter
	{

		class PointFilterBase: private boost::noncopyable
		{
		public:
			void forceNextApply()
			{
				m_PrevIndex = boost::numeric::bounds<long>::lowest();
			}
		
		protected:
			PointFilterBase() 
			{
				m_PrevIndex = boost::numeric::bounds<long>::lowest();
			}

			bool isPreviousIndex(long index)
			{
				if (m_PrevIndex==index) return true;
				m_PrevIndex = index;
				return false;
			}

		private:
			long m_PrevIndex;
		};

	}
}

#endif