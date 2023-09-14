#ifndef PCL_IMAGE_HELPER
#define PCL_IMAGE_HELPER

#include <pcl/image/Image.h>
#include <pcl/iterator/ImageIterator.h>
#include <pcl/image/PclToItkConverterImage.h>
#include <pcl/geometry/transformation/FlipTransformation.h>
#include <pcl/geometry/transformation/AffineTransformation.h>
#include <pcl/image/ItkHeader.h>
#include <pcl/type_utility.h>
#include <pcl/exception.h>

#ifndef NO_ITK
#include <pcl/itk/ItkTypeUtility.h>
#endif

#include "boost/tuple/tuple.hpp"


namespace pcl
{
	template <class Type>
	struct equivalent_image_type
	{
		typedef Image<typename Type::ValueType, Type::UseOrientationMatrix> type;
	};
	
	template <class TargetType, class SourceType>
	class can_NewAlias
	{
		typedef char yes[1];
		typedef char no[2];
		template <class T, class S> static yes& check(decltype(T::NewAlias(S::Pointer()))*);
		template <class T, class S> static no& check(...);
	public:
		static const bool value = sizeof(check<TargetType,SourceType>(0))==sizeof(yes);
	};

	class ImageHelper
	{
	public:
		template <class ImagePointerType1, class ImagePointerType2>
		static bool IsMemoryStructureSame(const ImagePointerType1& img1, const ImagePointerType2& img2)
		{
			for (int i=0; i<4; ++i) if (img1->getOffsetTable()[i]!=img2->getOffsetTable()[i]) return false;
			return true;
		}

		/**************************** Coordinate transformation method ****************************/
#ifndef NO_VNL
		template <class SourcePointerType, class TargetPointerType>
		static geometry::AffineTransformation::Pointer GetImageCoordinateTransformation(const SourcePointerType& source, const TargetPointerType& target)
		{
			auto result = geometry::AffineTransformation::New();
			result->addImageToPhysicalTransformation(source);
			result->addPhysicalToImageTransformation(target);
			return result;
		}
#endif
		
		template <class SourcePointerType>
		static auto GetAliasWithOrigin(const SourcePointerType& source, const Point3D<double>& origin) -> decltype(source->getAlias(Point3D<int>(), bool()))
		{
			Point3D<double> image_point = source->toImageVector(source->toPhysicalCoordinate(source->getMinPoint())-origin);
			Point3D<int> minp;
			minp.assignRound(image_point);
			if (!image_point.epsilonEqual(minp)) pcl_ThrowException(pcl::Exception(), "Incompatible origin given, error generated is too large");
			return source->getAlias(minp, true);
		}

		/**************************** Max min method ****************************/
		template <class ImagePointerType, class ValueType>
		static void GetMinMax(const ImagePointerType& source, ValueType& min, ValueType& max) 
		{
			ImageIterator iter(source);
			iter.begin();
			max = min = source->get(iter);
			for (; !iter.end(); iter.next()) {
				ValueType val = source->get(iter);
				if (val>max) max = val;
				else if (val<min) min = val;
			}
		}
		
		template <class ImagePointerType>
		static boost::tuple<
			typename ImagePointerType::element_type::IoValueType,
			typename ImagePointerType::element_type::IoValueType
		> GetMinMax(const ImagePointerType& source) 
		{
			typedef typename ImagePointerType::element_type::IoValueType ValueType;
			boost::tuple<ValueType, ValueType> result;
			ImageIterator iter(source);
			iter.begin();
			result.template get<0>() = result.template get<1>() = source->get(iter);
			for (; !iter.end(); iter.next()) {
				ValueType val = source->get(iter);
				if (val>result.template get<1>()) result.template get<1>() = val;
				else if (val<result.template get<0>()) result.template get<0>() = val;
			}
			return result;
		}

		template <class ImagePointerType, class IterType>
		static boost::tuple<
			typename ImagePointerType::element_type::IoValueType,
			typename ImagePointerType::element_type::IoValueType
		> GetMinMax(const ImagePointerType& source, IterType& iter) 
		{
			typedef typename ImagePointerType::element_type::IoValueType ValueType;
			boost::tuple<ValueType, ValueType> result;
			iter.begin();
			result.template get<0>() = result.template get<1>() = source->get(iter);
			for (; !iter.end(); iter.next()) {
				ValueType val = source->get(iter);
				if (val>result.template get<1>()) result.template get<1>() = val;
				else if (val<result.template get<0>()) result.template get<0>() = val;
			}
			return result;
		}
		
		/****************************Get region method ****************************/
		
		template <class ImagePointerType, class Cond>
		static Region3D<int> GetRegion(const ImagePointerType& source, Cond cond)
		{
			return GetRegion(source, cond, source->getRegion());
		}
		template <class ImagePointerType, class Cond>
		static Region3D<int> GetRegion(const ImagePointerType& source, Cond cond, const Region3D<int>& region)
		{
			ImageIteratorWithPoint iter(source);
			iter.setRegion(region.getIntersect(source->getRegion()));
			Region3D<int> result;
			result.reset();
			for (iter.begin(); !iter.end(); iter.next()) {
				if (cond(source->get(iter))) result.add(iter.getPoint());
			}
			return result;
		}

		/**************************** Fill methods ****************************/
		template <class ImagePointerType>
		static void Fill(const ImagePointerType& source, const typename ImagePointerType::element_type::IoValueType& val=typename ImagePointerType::element_type::IoValueType()) 
		{
			ImageIterator iter(source);
			for (iter.begin(); !iter.end(); iter.next()) source->set(iter, val);
		}

		template <class ImagePointerType>
		static void Fill(const ImagePointerType& source, const Region3D<int>& region, const typename ImagePointerType::element_type::IoValueType& val=typename ImagePointerType::element_type::IoValueType()) 
		{
			Fill(source, region.getMinPoint(), region.getMaxPoint(), val);
		}
		template <class ImagePointerType>
		static void Fill(const ImagePointerType& source, const Point3D<int>& minp, const Point3D<int>& maxp, const typename ImagePointerType::element_type::IoValueType& val=typename ImagePointerType::element_type::IoValueType()) 
		{
			Region3D<int> reg(minp, maxp);
			if (!source->getRegion().contain(reg)) {
				reg.setIntersect(source->getRegion());
				if (reg.empty()) return;
			}

			ImageIterator iter(source);
			iter.setRegion(reg);
			for (iter.begin(); !iter.end(); iter.next()) source->set(iter, val);
		}

		/**************************** Copy methods ****************************/
		template <class SourceImagePointerType, class DestinationImagePointerType>
		static void Copy(const SourceImagePointerType& source, const DestinationImagePointerType& dest)
		{
			Region3D<int> reg(source->getRegion());
			reg.setIntersect(dest->getRegion());
			if (reg.empty()) return;

			ImageIterator source_iter(source);
			ImageIterator dest_iter(dest);
			source_iter.setRegion(reg);
			dest_iter.setRegion(reg);
			for (source_iter.begin(), dest_iter.begin(); !source_iter.end() && !dest_iter.end(); source_iter.next(), dest_iter.next()) {
				dest->set(dest_iter, source->get(source_iter));
			}
		}

		template <class SourceImagePointerType, class DestinationImagePointerType>
		static void Copy(const SourceImagePointerType& source, const DestinationImagePointerType& dest, const Region3D<int>& region)
		{
			Copy(source, dest, region.getMinPoint(), region.getMaxPoint());
		}
		template <class SourceImagePointerType, class DestinationImagePointerType>
		static void Copy(const SourceImagePointerType& source, const DestinationImagePointerType& dest, const Point3D<int>& minp, const Point3D<int>& maxp)
		{
			Region3D<int> reg(minp, maxp);
			const Region3D<int> &source_reg = source->getRegion(),
				&dest_reg = dest->getRegion();
			if (!source_reg.contain(reg) || !dest_reg.contain(reg)) {
				reg.setIntersect(source_reg);
				reg.setIntersect(dest_reg);
			}
			if (reg.empty()) return;

			ImageIterator source_iter(source);
			ImageIterator dest_iter(dest);
			source_iter.setRegion(reg);
			dest_iter.setRegion(reg);
			for (source_iter.begin(), dest_iter.begin(); !source_iter.end(); source_iter.next(), dest_iter.next()) {
				dest->set(dest_iter, source->get(source_iter));
			}
		}

		template <class OutputImageType, class InputImagePointerType>
		static typename OutputImageType::Pointer GetCopy(const InputImagePointerType& source)
		{
			typename OutputImageType::Pointer result = OutputImageType::New(source);
			ImageHelper::Copy(source, result);
			return result;
		}

		template <class InputImagePointerType>
		static typename equivalent_image_type<typename ptr_base_type<InputImagePointerType>::type>::type::Pointer GetCopyAuto(const InputImagePointerType& source)
		{
			typedef typename equivalent_image_type<typename ptr_base_type<InputImagePointerType>::type>::type OutputImageType;
			typename OutputImageType::Pointer result = OutputImageType::New(source);
			ImageHelper::Copy(source, result);
			return result;
		}
		
		template <class ImagePointerType, class InputPointerType>
		static typename boost::enable_if_c<
			can_NewAlias<typename pcl::ptr_base_type<ImagePointerType>::type, typename pcl::ptr_base_type<InputPointerType>::type>::value &&
			!(!boost::is_const<typename ImagePointerType::element_type>::value && boost::is_const<typename InputPointerType::element_type>::value),
			ImagePointerType
		>::type GetCopyIfNeeded(const InputPointerType& input, bool* is_copy=NULL)
		{
			if (is_copy) *is_copy = false;
			return pcl::ptr_base_type<ImagePointerType>::type::NewAlias(input, false);
		}
		template <class ImagePointerType, class InputPointerType>
		static typename boost::disable_if_c<
			can_NewAlias<typename pcl::ptr_base_type<ImagePointerType>::type, typename pcl::ptr_base_type<InputPointerType>::type>::value &&
			!(!boost::is_const<typename ImagePointerType::element_type>::value && boost::is_const<typename InputPointerType::element_type>::value),
			ImagePointerType
		>::type GetCopyIfNeeded(const InputPointerType& input, bool* is_copy=NULL)
		{
			if (is_copy) *is_copy = true;
			return GetCopy<typename pcl::ptr_base_type<ImagePointerType>::type>(input);
		}

		/**************************** Flip methods ****************************/
		template <class OutputImageType, class InputImagePointerType>
		static typename OutputImageType::Pointer GetFlipped(const InputImagePointerType& source, int axis) 
		{
			geometry::FlipTransformation::Pointer trans = geometry::FlipTransformation::New();
			trans->flipRegion((geometry::FlipTransformation::Axis)axis, source->getRegion());
			return GetFlipped<OutputImageType>(source, trans);
		}

		template <class OutputImageType, class InputImagePointerType>
		static typename OutputImageType::Pointer GetFlipped(const InputImagePointerType& source, const geometry::FlipTransformation::ConstantPointer& trans) 
		{
			typename OutputImageType::Pointer result = OutputImageType::New(source);
			Point3D<int> minp = trans->toTransformed(source->getMinPoint()),
				maxp = trans->toTransformed(source->getMaxPoint());
			for (int i=0; i<3; i++) {
				if (minp[i]>maxp[i]) pcl_Swap(minp[i], maxp[i]);
			}
			result = result->getAlias(minp);
			ImageIteratorWithPoint iter(source);
			pcl_ForIterator(iter) {
				result->set(trans->toTransformed(iter.getPoint()), source->get(iter));
			}
			return result;
		}

		/**************************** Crop methods ****************************/
		template <class OutputImageType, class InputImagePointerType>
		static typename OutputImageType::Pointer GetCropped(const InputImagePointerType& source, const Region3D<int>& region, const typename OutputImageType::IoValueType& val=typename OutputImageType::IoValueType()) 
		{
			return GetCropped<OutputImageType>(source, region.getMinPoint(), region.getMaxPoint(), val);
		}
		
		template <class OutputImageType, class InputImagePointerType>
		static typename OutputImageType::Pointer GetCropped(const InputImagePointerType& source, const Point3D<int>& minp, const Point3D<int>& maxp, const typename OutputImageType::IoValueType& val=typename OutputImageType::IoValueType()) 
		{
			typename OutputImageType::Pointer result;
			if (OutputImageType::UseOrientationMatrix) result = OutputImageType::New(minp, maxp, source->getSpacing(), source->getOrigin(), source->getOrientationMatrix());
			else result = OutputImageType::New(minp, maxp, source->getSpacing(), source->getOrigin());
			CropCopy(source, result, minp, maxp, val);
			return result;
		}
		
		template <class InputImagePointerType>
		static typename equivalent_image_type<typename ptr_base_type<InputImagePointerType>::type>::type::Pointer GetCroppedAuto(const InputImagePointerType& source, const Region3D<int>& region, const typename InputImagePointerType::element_type::IoValueType& val=typename InputImagePointerType::element_type::IoValueType()) 
		{
			typedef typename equivalent_image_type<typename ptr_base_type<InputImagePointerType>::type>::type OutputImageType;
			return GetCropped<OutputImageType>(source, region, val);
		}

	protected:
		template <class SourceImagePointerType, class OutputImagePointerType, class ValueType>
		static void CropCopy(SourceImagePointerType source, OutputImagePointerType result, const Point3D<int>& minp, const Point3D<int>& maxp, const ValueType& val)
		{
			Region3D<int> result_reg(minp, maxp);
			Region3D<int> overlap_reg(source->getRegion());
			overlap_reg.setIntersect(result_reg);
			std::vector<Region3D<int> > fill_reg;
			fill_reg.reserve(6);
			result_reg.getRegionsAfterSubtractionBy(overlap_reg, fill_reg);

			ImageIterator source_iter(source);
			ImageIterator result_iter(result);

			//Copying values from source
			source_iter.setRegion(overlap_reg);
			result_iter.setRegion(overlap_reg);
			for (source_iter.begin(), result_iter.begin(); !source_iter.end(); source_iter.next(), result_iter.next()) {
				result->set(result_iter, source->get(source_iter));
			}

			//Filling with value
			for (int i=0; i<fill_reg.size(); i++) {
				Region3D<int> &cur_reg = fill_reg[i];
				result_iter.setRegion(cur_reg);
				for (result_iter.begin(); !result_iter.end(); result_iter.next()) result->set(result_iter, val);
			}
		}

#ifndef NO_ITK
		/**************************** itk to pcl conversion methods ****************************/
	public:

		template<class PclImageType, class ItkImagePointerType>
		static typename boost::enable_if<
			is_itk_image_convertable<PclImageType, typename deduce_itk_ptr_base_type<ItkImagePointerType>::type>,
			typename PclImageType::Pointer
		>::type CreateFromItkImage(ItkImagePointerType itk_image, bool grab_ownership, bool* is_alias=NULL)
		{
			typedef typename deduce_itk_ptr_base_type<ItkImagePointerType>::type ItkImageType;
			if (is_alias) *is_alias = true;
			return CreateAliasOfItkImage<PclImageType>(itk_image, grab_ownership);
		}

		template<class PclImageType, class ItkImagePointerType>
		static typename boost::disable_if<
			is_itk_image_convertable<PclImageType, typename deduce_itk_ptr_base_type<ItkImagePointerType>::type>,
			typename PclImageType::Pointer
		>::type CreateFromItkImage(ItkImagePointerType itk_image, bool grab_ownership, bool* is_alias=NULL)
		{
			typedef typename deduce_itk_ptr_base_type<ItkImagePointerType>::type ItkImageType;
			if (ItkImageType::ImageDimension>3) {
				std::stringstream ss;
				ss << "Invalid dimension (" << ItkImageType::ImageDimension << ") provided! The maximum supported is 3.";
				pcl_ThrowException(pcl::Exception(), ss.str());
			}
			if (is_alias) *is_alias = false;
			return CreateCopyOfItkImage<PclImageType>(itk_image);
		}

		template<class PclImageType, class ItkImagePointerType>
		static typename PclImageType::Pointer CreateAliasOfItkImage(ItkImagePointerType itk_image, bool grab_ownership)
		{
			typedef typename deduce_itk_ptr_base_type<ItkImagePointerType>::type ItkImageType;
			if (ItkImageType::ImageDimension>3) {
				std::stringstream ss;
				ss << "Invalid dimension (" << ItkImageType::ImageDimension << ") provided! The maximum supported is 3.";
				pcl_ThrowException(pcl::Exception(), ss.str());
			}

			typedef typename PclImageType::BufferType PclImageBufferType;

			//Setting up the buffer
			const typename ItkImageType::RegionType &buffer_region = itk_image->GetBufferedRegion();
			const typename ItkImageType::SizeType &buffer_size = buffer_region.GetSize();
			typename PclImageBufferType::Pointer pcl_buffer = PclImageBufferType::New(itk_image->GetBufferPointer(), buffer_size[0], buffer_size[1], buffer_size[2], grab_ownership);
			itk_image->GetPixelContainer()->SetContainerManageMemory(!grab_ownership);

			const typename ItkImageType::RegionType::IndexType& buffer_index = buffer_region.GetIndex();
			Point3D<int> buffer_minp;
			for (int i=0; i<ItkImageType::ImageDimension; i++) buffer_minp[i] = buffer_index[i];

			//Computing the actual region
			const typename ItkImageType::RegionType &actual_region = itk_image->GetRequestedRegion();
			const typename ItkImageType::RegionType::IndexType &actual_index = actual_region.GetIndex();
			const typename ItkImageType::RegionType::SizeType &actual_size = actual_region.GetSize();
			Point3D<int> actual_minp,
				actual_maxp;
			for (int i=0; i<ItkImageType::ImageDimension; i++) {
				actual_minp[i] = actual_index[i];
				actual_maxp[i] = actual_size[i]+actual_index[i]-1;
			}

			//Buffer coordinate corresponding to min point of actual region
			const Point3D<int>& buffer_coord = actual_minp - buffer_minp;

			//Creating the actual image
			const typename ItkImageType::SpacingType &itk_spacing = itk_image->GetSpacing();
			const typename ItkImageType::PointType &itk_origin = itk_image->GetOrigin();
			Point3D<double> spacing,
				origin;
			for (int i=0; i<ItkImageType::ImageDimension; i++) {
				spacing[i] = itk_spacing[i];
				origin[i] = itk_origin[i];
			}

			typename PclImageType::Pointer pcl_image;
			if (PclImageType::UseOrientationMatrix) pcl_image = PclImageType::New(pcl_buffer, buffer_coord, actual_minp, actual_maxp, spacing, origin, itk_image->GetDirection().GetVnlMatrix());
			else pcl_image = PclImageType::New(pcl_buffer, buffer_coord, actual_minp, actual_maxp, spacing, origin);

			if (itk_image->GetMetaDataDictionary().Begin()!=itk_image->GetMetaDataDictionary().End()) {
				pcl_image->setMetadata(Metadata::New(itk_image->GetMetaDataDictionary()));
			}

			return pcl_image;
		}

		template<class PclImageType, class ItkImagePointerType>
		static typename PclImageType::Pointer CreateCopyOfItkImage(ItkImagePointerType itk_image) {
			typedef typename deduce_itk_ptr_base_type<ItkImagePointerType>::type ItkImageType;
			if (ItkImageType::ImageDimension>3) {
				std::stringstream ss;
				ss << "Invalid dimension (" << ItkImageType::ImageDimension << ") provided! The maximum supported is 3.";
				pcl_ThrowException(pcl::Exception(), ss.str());
			}

			//Extracting image information
			const typename ItkImageType::RegionType &actual_region = itk_image->GetRequestedRegion();
			const typename ItkImageType::RegionType::IndexType &actual_index = actual_region.GetIndex();
			const typename ItkImageType::RegionType::SizeType &actual_size = actual_region.GetSize();
			Point3D<int> actual_minp,
				actual_maxp;
			for (int i=0; i<ItkImageType::ImageDimension; i++) {
				actual_minp[i] = actual_index[i];
				actual_maxp[i] = actual_size[i]+actual_index[i]-1;
			}

			const typename ItkImageType::SpacingType &itk_spacing = itk_image->GetSpacing();
			const typename ItkImageType::PointType &itk_origin = itk_image->GetOrigin();
			Point3D<double> spacing,
				origin;
			for (int i=0; i<ItkImageType::ImageDimension; i++) {
				spacing[i] = itk_spacing[i];
				origin[i] = itk_origin[i];
			}

			typename PclImageType::Pointer result;
			if (PclImageType::UseOrientationMatrix) result = PclImageType::New(actual_minp, actual_maxp, spacing, origin, itk_image->GetDirection().GetVnlMatrix());
			else result = PclImageType::New(actual_minp, actual_maxp, spacing, origin);

			//Copying values
			ImageIterator pcl_iter(result);
			itk::ImageRegionConstIterator<ItkImageType> itk_iter(itk_image, itk_image->GetRequestedRegion());

			for (itk_iter.GoToBegin(), pcl_iter.begin(); !itk_iter.IsAtEnd(); ++itk_iter, pcl_iter.next()) {
				result->set(pcl_iter, itk_iter.Get());
			}

			if (itk_image->GetMetaDataDictionary().Begin()!=itk_image->GetMetaDataDictionary().End()) {
				result->setMetadata(Metadata::New(itk_image->GetMetaDataDictionary()));
			}

			return result;
		}

		/**************************** pcl to itk conversion methods ****************************/
		template <class ImagePointerType>
		static typename boost::enable_if_c<
			ImagePointerType::element_type::ItkAliasable,
			typename itk::Image<typename ImagePointerType::element_type::ValueType,3>::Pointer
		>::type GetItkImage(const ImagePointerType& source, bool* is_alias=NULL)
		{
			if (is_alias) *is_alias = true;
			typename itk::Image<typename ImagePointerType::element_type::ValueType,3>::Pointer result =
				static_cast<itk::Image<typename ImagePointerType::element_type::ValueType,3>*>(GetItkImageAlias(source).GetPointer());
			return result;
		}

		template <class ImagePointerType>
		static typename boost::disable_if_c<
			ImagePointerType::element_type::ItkAliasable,
			typename itk::Image<typename ImagePointerType::element_type::IoValueType,3>::Pointer
		>::type GetItkImage(const ImagePointerType& source, bool* is_alias=NULL)
		{
			if (is_alias) *is_alias = false;
			typename itk::Image<typename ImagePointerType::element_type::IoValueType,3>::Pointer result = 
				static_cast<itk::Image<typename ImagePointerType::element_type::IoValueType,3>*>(GetItkImageCopy(source).GetPointer());
			return result;
		}

		template <class ImagePointerType>
		static typename itk::PclToItkConverterImage<typename boost::remove_const<typename ImagePointerType::element_type>::type>::Pointer 
			GetItkImageAlias(const ImagePointerType& org_source)
		{
			typedef typename boost::remove_const<typename ImagePointerType::element_type>::type ImageType;
			auto source = boost::const_pointer_cast<ImageType>(org_source);
			auto result = itk::PclToItkConverterImage<ImageType>::New();
			result->Allocate(source);

			if (source->getMetadata()) {
				source->getMetadata()->populate(result->GetMetaDataDictionary());
			}
			return result;
		}

		template <class ImagePointerType>
		static typename itk::Image<typename ImagePointerType::element_type::IoValueType,3>::Pointer GetItkImageCopy(const ImagePointerType& source)
		{
			typedef typename ImagePointerType::element_type ImageType;
			typedef itk::Image<typename ImageType::IoValueType,3> ItkImageType;
			typename ItkImageType::Pointer result = itk::Image<typename ImageType::IoValueType,3>::New();

			//Setting up the region
			typename ItkImageType::SizeType region_size;
			typename ItkImageType::IndexType region_index;
			const Point3D<int>& pcl_size = source->getSize();
			const Point3D<int>& pcl_minp = source->getMinPoint();
			for (int i=0; i<3; i++) {
				region_size[i] = pcl_size[i];
				region_index[i] = pcl_minp[i];
			}
			typename ItkImageType::RegionType itk_region;
			itk_region.SetSize(region_size);
			itk_region.SetIndex(region_index);

			//Setting up itk image
			result->SetRegions(itk_region);
			result->Allocate();
			result->SetSpacing(&(source->getSpacing()[0]));
			Point3D<double> origin = source->getOrigin();
			result->SetOrigin(&origin[0]);
			result->SetDirection(typename ItkImageType::DirectionType(source->getOrientationMatrix()));

			//Copying values
			ImageIterator pcl_iter(source);
			itk::ImageRegionIterator<ItkImageType> itk_iter(result, result->GetRequestedRegion());

			for (itk_iter.Begin(), pcl_iter.begin(); !itk_iter.IsAtEnd(); ++itk_iter, pcl_iter.next()) {
				itk_iter.Set(source->get(pcl_iter));
			}

			if (source->getMetadata()) {
				source->getMetadata()->populate(result->GetMetaDataDictionary());
			}

			return result;
		}
#endif
	};
}

#endif