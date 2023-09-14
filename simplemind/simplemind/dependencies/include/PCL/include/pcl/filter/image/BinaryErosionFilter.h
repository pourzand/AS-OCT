#ifndef PCL_BINARY_EROSION_FILTER
#define PCL_BINARY_EROSION_FILTER

#include <pcl/filter/image/BinaryDilationFilter.h>
#include <pcl/cod.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;
		using namespace pcl::iterator;

		template <class InputImageType, class OutputImageType>
		class BinaryErosionFilter: public ImageFilterBase
		{
		public:
			typedef typename OutputImageType::IoValueType OutputValueType;

			template <class ElementImageType>
			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const typename ElementImageType::ConstantPointer& structuring_element_image, const Region3D<int>& process_region=Region3D<int>().reset(), const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0)
			{
				return Compute(input, pcl::iterator::ImageNeighborIterator::CreateOffsetFromImage<ElementImageType>(structuring_element_image), process_region, fore_ground, back_ground);
			}
			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& structuring_element, const Region3D<int>& process_region=Region3D<int>().reset(), const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0)
			{
				BinaryErosionFilter filter;
				filter.setValue(fore_ground, back_ground);
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setStructuringElement(structuring_element);
				filter.update();
				return filter.getOutput();
			}


			BinaryErosionFilter() 
			{
				m_ForeGroundValue = 1;
				m_BackGroundValue = 0;
			}

			void setValue(const OutputValueType& fore_ground, const OutputValueType& back_ground)
			{
				m_ForeGroundValue = fore_ground;
				m_BackGroundValue = back_ground;
			}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
			}

			void setStructuringElement(const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& se)
			{
				m_StructOffsetList = se;
			}

			void update()
			{
				Point3D<int> minp((*m_StructOffsetList)[0]), maxp((*m_StructOffsetList)[0]);
				pcl_ForEach(*m_StructOffsetList, item) {
					minp.min(*item);
					maxp.max(*item);
				}
				Region3D<int> touched_region(this->m_ProcessRegion);
				touched_region.getMinPoint() += minp;
				touched_region.getMaxPoint() += maxp;
				touched_region.setIntersect(m_Input->getRegion());
				auto temp_input = (!CodHelper::CreateFromImage(m_Input)).template getImage<InputImageType::UseOrientationMatrix>();
				m_Output = BinaryDilationFilter<typename pcl::ptr_base_type<decltype(temp_input)>::type, OutputImageType>::Compute(temp_input, m_StructOffsetList, touched_region, m_BackGroundValue, m_ForeGroundValue);
				// TODO: pending testing.
				//m_Output = BinaryDilationFilter<InputImageType, OutputImageType>::Compute(temp_input, m_StructOffsetList, touched_region, m_BackGroundValue, m_ForeGroundValue);
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typename InputImageType::ConstantPointer m_Input;
			typename OutputImageType::Pointer m_Output;
			pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer m_StructOffsetList;
			OutputValueType m_ForeGroundValue, m_BackGroundValue;
		};

	}
}

#endif