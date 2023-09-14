#ifndef PCL2_IMAGE_HELPER
#define PCL2_IMAGE_HELPER

#include <pcl/filter2/Helper.h>
#include <pcl/filter2/image/BinaryMorphologicalFilter.h>
#include <pcl/filter2/image/EuclideanDistanceTransformFilter.h>

namespace pcl
{
	namespace filter2
	{
		
		class BinaryMorphology
		{
		public:
			template <class OutputImageType, class InputType, class StructElemType>
			static typename boost::enable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Dilate(const InputType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset()) 
			{
				auto bound = Helper::GetFixedValueBoundary(input);
				return BinaryMorphologicalFilter<Dilation, decltype(bound), OutputImageType>::Compute(bound, struct_elem, 1, 0, output_region);
			}

			template <class OutputImageType, class InputType, class StructElemType>
			static typename boost::disable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Dilate(const InputType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				return BinaryMorphologicalFilter<Dilation, InputType, OutputImageType>::Compute(input, struct_elem, 1, 0, output_region);
			}



			template <class OutputImageType, class InputType, class StructElemType>
			static typename boost::enable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Erode(const InputType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset()) 
			{
				auto bound = Helper::GetFixedValueBoundary(input);
				return BinaryMorphologicalFilter<Erosion, decltype(bound), OutputImageType>::Compute(bound, struct_elem, 1, 0, output_region);
			}

			template <class OutputImageType, class InputType, class StructElemType>
			static typename boost::disable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Erode(const InputType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset()) 
			{
				return BinaryMorphologicalFilter<Erosion, InputType, OutputImageType>::Compute(input, struct_elem, 1, 0, output_region);
			}



			template <class OutputImageType, class InputImagePointerType, class StructElemType>
			static typename OutputImageType::Pointer Close(const InputImagePointerType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset()) 
			{
				pcl::Region3D<int> region; region.reset();
				pcl::ImageIteratorWithPoint iter(input);
				if (!output_region.empty()) iter.setRegion(output_region);
				pcl_ForIterator(iter) {
					if (input->get(iter)>0) region.add(iter.getPoint());
				}
				auto struct_region = GetStructElemRegion(struct_elem);
				region.getMinPoint() += struct_region.getMinPoint();
				region.getMaxPoint() += struct_region.getMaxPoint();
				
				typename OutputImageType::Pointer result;
				{
					auto bound = Helper::GetFixedValueBoundary(input);
					result = BinaryMorphologicalFilter<Dilation, decltype(bound), OutputImageType>::Compute(bound, struct_elem, 1, 0, region);
				}
				{
					auto bound = Helper::GetFixedValueBoundary(result);
					result = BinaryMorphologicalFilter<Erosion, decltype(bound), OutputImageType>::Compute(bound, struct_elem, 1, 0, output_region);
				}
				return result;
			}

			template <class OutputImageType, class InputImagePointerType, class StructElemType>
			static typename OutputImageType::Pointer Open(const InputImagePointerType& input, const StructElemType& struct_elem, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset()) 
			{
				auto bound = Helper::GetFixedValueBoundary(input);
				auto result = BinaryMorphologicalFilter<Erosion, decltype(bound), OutputImageType>::Compute(bound, struct_elem, 1, 0, output_region);
				auto bound2 = Helper::GetFixedValueBoundary(result);
				result = BinaryMorphologicalFilter<Dilation, decltype(bound2), OutputImageType>::Compute(bound2, struct_elem, 1, 0, output_region);
				return result;
			}

		protected:
			template <class T>
			static typename boost::enable_if<boost::is_base_of<pcl::ImageBase, typename ptr_base_type<T>::type>, pcl::Region3D<int>>::type GetStructElemRegion(const T& struct_elem)
			{
				return struct_elem->getRegion();
			}

			template <class T>
			static typename boost::disable_if<boost::is_base_of<pcl::ImageBase, typename ptr_base_type<T>::type>, pcl::Region3D<int>>::type GetStructElemRegion(const T& struct_elem)
			{
				pcl::Region3D<int> region;
				region.reset();
				pcl_ForEach(*struct_elem, item) region.add(*item);
				return region;
			}
		};


		class DistanceTransform
		{
		public:
			template <class OutputImageType, class InputType>
			static typename boost::enable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Compute(const InputType& input, bool is_signed, bool use_square_distance=false, bool use_spacing=true, const Region3D<int>& output_region=Region3D<int>().reset())
			{
				auto bound = Helper::GetFixedValueBoundary(input);
				return EuclideanDistanceTransformFilter<decltype(bound), OutputImageType>::Compute(bound, is_signed, use_square_distance, use_spacing, output_region);
			}

			template <class OutputImageType, class InputType>
			static typename boost::disable_if<
				boost::is_base_of<
					pcl::ImageBase, 
					typename ptr_base_type<InputType>::type
				>, 
				typename OutputImageType::Pointer
			>::type Compute(const InputType& input, bool is_signed, bool use_square_distance=false, bool use_spacing=true, const Region3D<int>& output_region=Region3D<int>().reset())
			{
				return EuclideanDistanceTransformFilter<InputType, OutputImageType>::Compute(input, is_signed, use_square_distance, use_spacing, output_region);
			}
		};
		
	}
}

#endif