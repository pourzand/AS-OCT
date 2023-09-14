#ifndef PCL_ITK_TO_VTK_HELPER
#define PCL_ITK_TO_VTK_HELPER
#ifndef NO_ITK

#include <pcl/vtk/itkImageToVTKImageFilter.h>
#include <pcl/image.h>
#include <pcl/type_utility.h>

namespace pcl
{
    namespace vtk
	{
        using namespace pcl;
	
		class ItkToVtkHelper
		{
		public:
			template <class PclImagePointerType>
			class GetConverterType
			{
				typedef typename identity<decltype(ImageHelper::GetItkImage(PclImagePointerType(),false))>::type::ObjectType itk_image_type;
			public:
				typedef itk::ImageToVTKImageFilter<itk_image_type> type;
			};
		
			template <class PclImagePointerType>
			static typename GetConverterType<PclImagePointerType>::type::Pointer GetConverter(const PclImagePointerType& image, bool* alias=NULL)
			{
				typedef typename GetConverterType<PclImagePointerType>::type FilterType;
				FilterType::Pointer filter = FilterType::New();
                filter->SetInput(ImageHelper::GetItkImage(image,alias));
				filter->Update();
				return filter;
			}
		};

	}
}

#endif
#endif
