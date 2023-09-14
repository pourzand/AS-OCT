#ifndef PCL_MODIFICATION_TRACKER
#define PCL_MODIFICATION_TRACKER

#include <pcl/gui2/ModifiableObject.h>

namespace pcl
{
	namespace gui
	{

		class ModificationTracker
		{
		public:
			ModificationTracker()
			{}

			ModificationTracker(const ModificationTracker& obj)
			{
				m_Object = obj.m_Object;
				m_RecordedModifiedTime = obj.m_RecordedModifiedTime;
			}

			ModificationTracker(const ModifiableObject::ConstantPointer& obj)
			{
				reset(obj);
			}

			ModificationTracker& operator=(const ModificationTracker& obj)
			{
				m_Object = obj.m_Object;
				m_RecordedModifiedTime = obj.m_RecordedModifiedTime;
				return *this;
			}

			void reset()
			{
				m_Object.reset();
			}

			void reset(const ModifiableObject::ConstantPointer& obj)
			{
				if (m_Object==obj) return;
				m_Object = obj;
				if (m_Object) m_RecordedModifiedTime = m_Object->getModifiedTime()-1; //Tracker is not updated when first set
			}

			bool isModified() const
			{
				if (m_Object) return m_RecordedModifiedTime<m_Object->getModifiedTime();
				return false;
			}

			void updated()
			{
				if (m_Object) m_RecordedModifiedTime = m_Object->getModifiedTime();
			}

		protected:
			ModifiableObject::ConstantPointer m_Object;
			clock_t m_RecordedModifiedTime;
		};

	}
}

#endif