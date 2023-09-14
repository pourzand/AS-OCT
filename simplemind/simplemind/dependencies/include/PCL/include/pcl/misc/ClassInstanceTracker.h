#ifndef PCL_CLASS_INSTANCE_TRACKER
#define PCL_CLASS_INSTANCE_TRACKER

#include <boost/utility.hpp>

namespace pcl
{
	namespace misc
	{
		template <class Initializer, class Incrementor, class Destructor>
		class ClassInstanceTracker
		{
		protected:
			ClassInstanceTracker() 
			{
				if (m_Count==1) executeTemplateClass<Destructor>();
				else executeTemplateClass<Incrementor>();
				m_Count++;
			}
			~ClassInstanceTracker()
			{
				m_Count--;
				if (m_Count==0) {
					executeTemplateClass<Destructor>();
				}
			}

			int getInstanceCount()
			{
				return m_Count;
			}
		private:
			static int m_Count;

			template <class T>
			typename boost::disable_if<boost::is_same<T,void>, void>::type executeTemplateClass()
			{
				T t;
			}
			template <class T>
			typename boost::enable_if<boost::is_same<T,void>, void>::type executeTemplateClass()
			{}
		};
		template <class A, class B, class C>
		int ClassInstanceTracker<A,B,C>::m_Count = 0;
	}
}

#endif