#ifndef PCL_GUI_DATA
#define PCL_GUI_DATA

#include <pcl/gui2/ModifiableObject.h>

namespace pcl
{
	namespace gui
	{

		class Data: public ModifiableObject
		{
		public:
			typedef Data Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

			enum DataType 
			{
				IMAGE,
				OVERLAY,
				TOTALNUM
			};

			virtual DataType type() const=0;

		protected:
			Data()
			{}
		};

	}
}

#endif