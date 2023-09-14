#pragma once

#include <pcl/vtk/PclToVtkImport.h>

namespace pcl
{

	class VtkHelper
	{
	public:
		template <class ImagePointerType>
		static vtkImageImport* GetVtkImageImport(const ImagePointerType& org_source, bool* is_alias=NULL)
		{
			if (is_alias) *is_alias = ImagePointerType::element_type::ItkAliasable;
			typedef typename boost::remove_const<typename ImagePointerType::element_type>::type ImageType;
			auto source = boost::const_pointer_cast<ImageType>(org_source);
			if (ImagePointerType::element_type::ItkAliasable) {
				auto ret = PclToVtkImport<ImageType>::New();
				ret->setInput(source);
				return ret;
			} else {
				typedef pcl::Image<typename ImageType::IoValueType, ImageType::UseOrientationMatrix> CopyImageType;
				auto copy = pcl::ImageHelper::GetCopy<CopyImageType>(source);
				auto ret = PclToVtkImport<CopyImageType>::New();
				ret->setInput(copy);
				return ret;
			}
		}
	};
	
}