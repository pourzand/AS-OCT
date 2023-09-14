#ifndef PCL_TO_ITK_CONVERTER_IMAGE
#define PCL_TO_ITK_CONVERTER_IMAGE
#ifndef NO_ITK

#include <pcl/image/Image.h>
#include <pcl/image/ItkHeader.h>

namespace itk 
{

	template <class T>
	class PclToItkConverterImage: public Image<typename T::ValueType,3> 
	{
	public:
		typedef T ImageType;
		typedef typename T::ValueType TPixel;
		/** Standard class typedefs */
		typedef PclToItkConverterImage Self;
		typedef Image<TPixel, 3>  Superclass;
		typedef SmartPointer<Self>  Pointer;
		typedef SmartPointer<const Self>  ConstPointer;
		typedef WeakPointer<const Self>  ConstWeakPointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);

		/** Run-time type information (and related methods). */
		itkTypeMacro(CImgImage, Image);

		/** Pixel typedef support. Used to declare pixel type in filters
		* or other operations. */
		typedef TPixel PixelType;

		/** Typedef alias for PixelType */
		typedef TPixel ValueType;

		/** Internal Pixel representation. Used to maintain a uniform API
		* with Image Adaptors and allow to keep a particular internal
		* representation of data while showing a different external
		* representation. */
		typedef TPixel InternalPixelType;

		typedef PixelType IOPixelType;

		/** Accessor type that convert data between internal and external
		*  representations.  */
		typedef DefaultPixelAccessor< PixelType > AccessorType;
		typedef DefaultPixelAccessorFunctor< Self > AccessorFunctorType;

		/** Tyepdef for the functor used to access a neighborhood of pixel pointers.*/
		typedef NeighborhoodAccessorFunctor< Self > 
			NeighborhoodAccessorFunctorType;

		/** Dimension of the image.  This constant is used by functions that are
		* templated over image type (as opposed to being templated over pixel type
		* and dimension) when they need compile time access to the dimension of
		* the image. */
		itkStaticConstMacro(ImageDimension, unsigned int, 3);

		/** Container used to store pixels in the image. */
		typedef ImportImageContainer<unsigned long, PixelType> PixelContainer;

		/** Index typedef support. An index is used to access pixel values. */
		typedef typename Superclass::IndexType  IndexType;

		/** Offset typedef support. An offset is used to access pixel values. */
		typedef typename Superclass::OffsetType OffsetType;

		/** Size typedef support. A size is used to define region bounds. */
		typedef typename Superclass::SizeType  SizeType;

		/** Direction typedef support. A matrix of direction cosines. */
		typedef typename Superclass::DirectionType  DirectionType;

		/** Region typedef support. A region is used to specify a subset of an image. */
		typedef typename Superclass::RegionType  RegionType;

		/** Spacing typedef support.  Spacing holds the size of a pixel.  The
		* spacing is the geometric distance between image samples. */
		typedef typename Superclass::SpacingType SpacingType;

		/** Origin typedef support.  The origin is the geometric coordinates
		* of the index (0,0). */
		typedef typename Superclass::PointType PointType;

		/** A pointer to the pixel container. */
		typedef typename PixelContainer::Pointer PixelContainerPointer;
		typedef typename PixelContainer::ConstPointer PixelContainerConstPointer;

		/** Offset typedef (relative position between indices) */
		typedef typename Superclass::OffsetValueType OffsetValueType;

		using Superclass::Allocate;

		/** Special Allocator for sharing data */
		void Allocate(typename T::Pointer image) 
		{
			auto pcl_buffer = image->getBuffer();
			auto pcl_buffer_coord = image->getMinPoint() - image->toBufferCoordinate(image->getMinPoint());
			auto pcl_region = image->getRegion();
			auto pcl_spacing = image->getSpacing();
			auto pcl_origin = image->getOrigin();
			auto pcl_matrix = image->getOrientationMatrix();
			
			this->SetSpacing(&pcl_spacing[0]);
			this->SetOrigin(&pcl_origin[0]);
			this->SetDirection(DirectionType(pcl_matrix));

			//Setting up the regions
			SizeType buffer_size, region_size;
			IndexType buffer_index, region_index;
			const pcl::Point3D<int>& pcl_buffer_size = pcl_buffer->getSize();
			const pcl::Point3D<int>& pcl_minp = pcl_region.getMinPoint();
			for (int i=0; i<3; i++) {
				buffer_size[i] = pcl_buffer_size[i];
				region_size[i] = pcl_region.getSize((typename pcl::Region3D<int>::Axis)i);
				buffer_index[i] = pcl_buffer_coord[i];
				region_index[i] = pcl_minp[i];
			}
			RegionType buffer_region,
				actual_region;
			buffer_region.SetSize(buffer_size);
			buffer_region.SetIndex(buffer_index);
			actual_region.SetSize(region_size);
			actual_region.SetIndex(region_index);

			this->SetLargestPossibleRegion(buffer_region);
			this->SetBufferedRegion(buffer_region);
			this->SetRequestedRegion(actual_region);

			this->ComputeOffsetTable();
			unsigned long num = this->GetOffsetTable()[3];
			this->GetPixelContainer()->SetImportPointer(pcl_buffer->getPointer(), num, false);
			m_PclImage = image;
		}

	protected:
		PclToItkConverterImage() {}
		virtual ~PclToItkConverterImage() {}
		typename ImageType::Pointer m_PclImage;

	private:
		PclToItkConverterImage(const Self&); //purposely not implemented
		void operator=(const Self&); //purposely not implemented
	};

}

#endif
#endif