#ifndef PCL_MODIFIABLE_OBJECT
#define PCL_MODIFIABLE_OBJECT

#include <time.h>
#include <boost/smart_ptr.hpp>

namespace pcl
{
	namespace gui
	{

		class ModifiableObject
		{
		public:
			typedef ModifiableObject Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

			ModifiableObject()
			{
				modified();
			}

			virtual void modified()
			{
				m_ModifiedTime = clock();
			}

			clock_t getModifiedTime() const
			{
				return m_ModifiedTime;
			}

		protected:
			clock_t m_ModifiedTime;
		};

	}
}

#endif