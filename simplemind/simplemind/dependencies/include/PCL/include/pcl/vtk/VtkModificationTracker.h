#ifndef PCL_VTK_MODIFICATION_TRACKER
#define PCL_VTK_MODIFICATION_TRACKER

#include <vtkObject.h>

namespace pcl
{
	namespace vtk
	{

		class VtkModificationTracker
		{
		public:
			VtkModificationTracker()
			{}

			VtkModificationTracker(const VtkModificationTracker& obj)
			{
				m_Object = obj.m_Object;
				m_RecordedModifiedTime = obj.m_RecordedModifiedTime;
			}

			VtkModificationTracker(vtkObject* obj)
			{
				reset(obj);
			}

			VtkModificationTracker& operator=(const VtkModificationTracker& obj)
			{
				m_Object = obj.m_Object;
				m_RecordedModifiedTime = obj.m_RecordedModifiedTime;
				return *this;
			}

			void reset()
			{
				m_Object = NULL;
			}

			void reset(vtkObject* obj)
			{
				if (m_Object==obj) return;
				m_Object = obj;
				if (m_Object) m_RecordedModifiedTime = m_Object->GetMTime()-1; //Tracker is not updated when first set
			}

			bool isModified() const
			{
				if (m_Object) return m_RecordedModifiedTime<m_Object->GetMTime();
				return false;
			}

			void updated()
			{
				if (m_Object) m_RecordedModifiedTime = m_Object->GetMTime();
			}

		protected:
			vtkObject* m_Object;
			clock_t m_RecordedModifiedTime;
		};

	}
}

#endif