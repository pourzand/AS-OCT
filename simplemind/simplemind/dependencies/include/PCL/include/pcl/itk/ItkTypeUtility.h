#ifndef ITK_TYPE_UTILITY
#define ITK_TYPE_UTILITY

namespace pcl
{

	template <class PclImageType, class ItkImageType>
	class is_itk_image_convertable
	{
		typedef typename ItkImageType::PixelType ItkValueType;
		typedef typename PclImageType::ValueType PclValueType;

	public:
		enum {value = 
			boost::is_same<PclValueType, ItkValueType>::value &
			boost::is_base_of<itk::Image<ItkValueType,3>, ItkImageType>::value
		};
	};

	template <class ItkPointer, class Enable=void>
	struct deduce_itk_ptr_base_type
	{
		typedef typename ItkPointer::ObjectType type;
	};

	template <class ItkPointer>
	struct deduce_itk_ptr_base_type<ItkPointer, typename boost::enable_if<boost::is_pointer<ItkPointer>>::type>
	{
		typedef typename pcl::base_type<ItkPointer>::type type;
	};

};

#endif