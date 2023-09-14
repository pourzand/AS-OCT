#ifndef DEDUCE_FROM_ITK_IMAGE
#define DEDUCE_FROM_ITK_IMAGE
#ifndef NO_ITK

#include <pcl/image.h>

namespace pcl
{

	template <class ItkImageType, bool UseOrientationMatrix=true>
	struct DeduceFromItkImage
	{
		typedef Image<typename ItkImageType::PixelType, UseOrientationMatrix> PclImageType;
		enum {Aliasable = is_itk_image_convertable<PclImageType, ItkImageType>::value };
		
		operator bool() const
		{
			return Aliasable;
		}
	};

}
	
#endif
#endif