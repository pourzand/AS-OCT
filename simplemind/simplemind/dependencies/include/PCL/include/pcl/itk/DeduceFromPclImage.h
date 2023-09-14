#ifndef DEDUCE_FROM_PCL_IMAGE
#define DEDUCE_FROM_PCL_IMAGE
#ifndef NO_ITK

#include <pcl/image.h>

namespace pcl
{

	template <class PclImageType>
	struct DeduceFromPclImage
	{
		typedef typename identity<decltype(ImageHelper::GetItkImage(typename PclImageType::Pointer(), false))>::type::ObjectType ItkImageType;
		enum {Aliasable = PclImageType::ItkAliasable };
		
		operator bool() const
		{
			return Aliasable;
		}
	};

}

#endif
#endif