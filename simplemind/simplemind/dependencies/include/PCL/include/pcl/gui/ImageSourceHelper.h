#ifndef PCL_GUI_IMAGE_SOURCE_HELPER
#define PCL_GUI_IMAGE_SOURCE_HELPER

#include <pcl/gui/ImageSource.h>

namespace pcl
{
	namespace gui
	{

		class ImageSourceHelper
		{
		public:
			template <class ImagePointerType>
			static typename ImageSource<typename pcl::ptr_base_type<ImagePointerType>::type>::Pointer Create(const ImagePointerType& image, bool* is_alias=NULL)
			{
				return ImageSource<typename pcl::ptr_base_type<ImagePointerType>::type>::New(image, is_alias);
			}

			static void ResetLookupTable(const ImageSourceBase::Pointer& image_source)
			{
				auto range = image_source->getMinMax();
				image_source->getLookupTable()->SetRange(range.get<0>(), range.get<1>());
				image_source->getLookupTable()->SetValueRange(0, 1);
				image_source->getLookupTable()->SetSaturationRange(0,0);
				image_source->getLookupTable()->SetRampToLinear();
				image_source->getLookupTable()->Build();
			}

			static void SetRandomColorTable(const ImageSourceBase::Pointer& image_source)
			{
				auto range = image_source->getMinMax();
				int label_num = ceil(range.get<1>()-range.get<0>())+1;
				if (label_num>500) label_num = 500;
				SetRandomColorTable(image_source, range, label_num);
			}

			static void SetRandomColorTable(const ImageSourceBase::Pointer& image_source, int label_num)
			{
				auto range = image_source->getMinMax();
				SetRandomColorTable(image_source, range, label_num);
			}

		protected:
			static void SetRandomColorTable(const ImageSourceBase::Pointer& image_source, const boost::tuple<double,double>& range, int label_num)
			{
				int zero_ind = -1;
				if (floor(range.get<0>())<0) {
					//TODO!!!!
				}
				image_source->getLookupTable()->SetNumberOfTableValues(label_num);
				image_source->getLookupTable()->SetRange(range.get<0>(), range.get<1>());
				//image_source->getLookupTable()->SetTab
			}
		};

	}
}

#endif