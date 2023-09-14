#ifndef PCL_IMAGE_TYPEDEF
#define PCL_IMAGE_TYPEDEF

#include <pcl/image/Image.h>

namespace pcl
{
	typedef Image<Bit> BitImage;
	typedef Image<bool> BoolImage;
	typedef Image<char> CharImage;
	typedef Image<int> IntImage;
	typedef Image<float> FloatImage;
	typedef Image<double> DoubleImage;
}

#endif