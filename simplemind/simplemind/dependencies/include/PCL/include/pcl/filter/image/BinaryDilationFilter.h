#ifndef PCL_BINARY_DILATION_FILTER
#define PCL_BINARY_DILATION_FILTER

#include <pcl/filter/image/ImageFilterBase.h>
#include <pcl/filter/boundary_handler/LocalFixedValueBoundaryHandler.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;
		using namespace pcl::iterator;

		template <class InputImageType, class OutputImageType>
		class BinaryDilationFilter: public ImageFilterBase
		{
		protected:
			typedef typename OutputImageType::IoValueType OutputValueType;

		public:

			template <class ElementImageType>
			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const typename ElementImageType::ConstantPointer& structuring_element_image, const Region3D<int>& process_region=Region3D<int>().reset(), const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0)
			{
				return Compute(input, pcl::iterator::ImageNeighborIterator::CreateOffsetFromImage<ElementImageType>(structuring_element_image), process_region, fore_ground, back_ground);
			}
			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& structuring_element, const Region3D<int>& process_region=Region3D<int>().reset(), const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0)
			{
				BinaryDilationFilter filter;
				filter.setValue(fore_ground, back_ground);
				filter.setProcessRegion(process_region);
				filter.setInput(input);
				filter.setStructuringElement(structuring_element);
				filter.update();
				return filter.getOutput();
			}


			BinaryDilationFilter() 
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
				m_BoundaryHandler.setImage(m_Input);
				if (this->m_ProcessRegion.empty()) setProcessRegion(m_Input->getRegion());
			}

			void setStructuringElement(const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& se)
			{
				m_StructIter = se;
			}

			void update()
			{
				m_BoundaryHandler = 0;
				this->m_ProcessRegion.setIntersect(m_Input->getRegion());
				m_StructIter.setImage(m_Input); //Setting structure iterator to input instead in order to get the offset region (output size is the same as input, so it doesn't matter which one we set iterator to!)
				misc::SafeUnsafeRegionGenerator region_gen(this->m_ProcessRegion, m_Input->getRegion(), m_StructIter.getOffsetRegion());

				//Searching for border voxels
				MarkerImageType::Pointer marker = MarkerImageType::New(m_Input);
				ImageHelper::Fill(marker, this->m_ProcessRegion, 0);
				bool safe_exist, unsafe_exist;
				detectBorderSurface(marker, safe_exist, unsafe_exist, region_gen.getSafeRegion());

				//Initializing result
				m_Output = OutputImageType::New(m_Input);
				{
					ImageIterator iter(m_Input);
					iter.setRegion(this->m_ProcessRegion);
					pcl_ForIterator(iter) {
						if (m_Input->get(iter)) m_Output->set(iter, m_ForeGroundValue);
						else m_Output->set(iter, m_BackGroundValue);
					}
				}

				//Actual processing
				if (safe_exist) {
					ImageIterator iter(m_Output);
					iter.setRegion(region_gen.getSafeRegion());
					Region3D<int> safe_region = region_gen.getSafeRegion();
					pcl_ForIterator(iter) {
						if (marker->get(iter)!=0) {
							m_StructIter.setOrigin(iter);
							pcl_ForIterator(m_StructIter) {
								m_Output->set(m_StructIter, m_ForeGroundValue);
							}
						}
					}
				}
				if (unsafe_exist) {
					ImageIteratorWithPoint iter(m_Input);
					pcl_ForEach(region_gen.getUnsafeRegion(), item) {
						iter.setRegion(*item);
						pcl_ForIterator(iter) {
							if (marker->get(iter)!=0) {
								m_StructIter.setOrigin(iter.getPoint(), iter);
								pcl_ForIterator(m_StructIter) {
									if (m_Output->contain(m_StructIter.getPoint())) m_Output->set(m_StructIter, m_ForeGroundValue);
								}
							}
						}
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typedef Image<char> MarkerImageType;
			typename InputImageType::ConstantPointer m_Input;
			typename OutputImageType::Pointer m_Output;
			ImageNeighborIterator m_StructIter;
			LocalFixedValueBoundaryHandler<InputImageType> m_BoundaryHandler;
			OutputValueType m_ForeGroundValue, m_BackGroundValue;

			void detectBorderSurface(MarkerImageType::Pointer& marker, bool& safe_exist, bool& unsafe_exist, const Region3D<int>& output_safe_region)
			{
				ImageNeighborIterator neighbor_iter(m_Input, ImageNeighborIterator::CreateConnect26Offset());
				misc::SafeUnsafeRegionGenerator region_gen(this->m_ProcessRegion, m_Input->getRegion(), neighbor_iter.getOffsetRegion());

				safe_exist = false;
				unsafe_exist = false;

				if (!region_gen.getSafeRegion().empty()) {
					ImageIteratorWithPoint iter(m_Input);
					iter.setRegion(region_gen.getSafeRegion());
					pcl_ForIterator(iter) if (m_Input->get(iter)) {
						neighbor_iter.setOrigin(iter);
						bool is_border = false;
						pcl_ForIterator(neighbor_iter) {
							if (!m_Input->get(neighbor_iter)) {
								is_border = true;
								break;
							}
						}
						if (is_border) {
							if (!safe_exist && output_safe_region.contain(iter.getPoint())) {
								safe_exist = true;
							} else unsafe_exist = true;
							marker->set(iter, 1);
						}
					}
				}

				{
					ImageIteratorWithPoint iter(m_Input);
					pcl_ForEach(region_gen.getUnsafeRegion(), item) {
						iter.setRegion(*item);
						pcl_ForIterator(iter) if (m_Input->get(iter)) {
							neighbor_iter.setOrigin(iter.getPoint(), iter);
							bool is_border = false;
							pcl_ForIterator(neighbor_iter) {
								if (!m_BoundaryHandler.get(neighbor_iter.getPoint(), neighbor_iter)) {
									is_border = true;
									break;
								}
							}
							if (is_border) {
								if (!safe_exist && output_safe_region.contain(iter.getPoint())) {
									safe_exist = true;
								} else unsafe_exist = true;
								marker->set(iter, 1);
							}
						}
					}
				}
			}
		};

	}
}

#endif