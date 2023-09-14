#ifndef PCL2_WATERSHED
#define PCL2_WATERSHED

#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/iterator/BruteRegionGrowingIterator.h>

namespace pcl
{
	namespace filter2
	{
		using namespace pcl;
		using namespace pcl::iterator;

		template <class InputImageType, class OutputImageType>
		class Watershed
		{
		public:
			static typename OutputImageType::Pointer Compute(const typename InputImageType::ConstantPointer& input, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& neighborhood)
			{
				Watershed filter;
				filter.setInput(input);
				filter.setNeighborhood(neighborhood);
				filter.update();
				return filter.getOutput();
			}


			Watershed() {}

			void setInput(const typename InputImageType::ConstantPointer& input)
			{
				m_Input = input;
			}

			void setNeighborhood(const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& neighbor)
			{
				m_NeighborIter = neighbor;
			}

			void update()
			{
				//Setting up environment
				m_NeighborIter.setImage(m_Input);
				m_OffsetIndexLookup = InternalImageType::New(m_NeighborIter.getOffsetRegion().getMinPoint(), m_NeighborIter.getOffsetRegion().getMaxPoint());
				for (int i=0; i<m_NeighborIter.size(); i++) {
					m_OffsetIndexLookup->set(m_NeighborIter.getOffset(i), i);
				}

				//Precompute the delta of the offset for gradient computation
				m_Delta.reset(new double[m_NeighborIter.size()]);
				for (int i=0; i<m_NeighborIter.size(); i++) {
					const Point3D<int>& offset = m_NeighborIter.getOffset(i);
					m_Delta[i] = offset.x()*m_Input->getSpacing().x() 
						+ offset.y()*m_Input->getSpacing().y()
						+ offset.z()*m_Input->getSpacing().z();
					m_Delta[i] = pcl_Abs(m_Delta[i]);
				}

				//Computing flow image
				InternalImageType::Pointer flow_image = computeFlowImage();
				correctFlowImage(flow_image);

				//Generating output
				m_Output = OutputImageType::New(m_Input);
				ImageHelper::Fill(m_Output, 0);
				ImageIteratorWithPoint image_iter(flow_image);
				typename OutputImageType::IoValueType sink_count = 0;
				pcl_ForIterator(image_iter) if (flow_image->get(image_iter)==-1) {
					sink_count++;
					m_Output->set(image_iter, sink_count);
					BruteRegionGrowingIterator grow_iter;
					grow_iter.setNeighborIterator(m_NeighborIter, m_Input->getRegion());
					grow_iter.addSeed(image_iter.getPoint(), image_iter);
					pcl_ForIterator(grow_iter) if (m_Output->get(grow_iter)==0) {
						Point3D<int> offset(grow_iter.getSourcePoint());
						offset -= grow_iter.getPoint();
						if (flow_image->get(grow_iter)==toOffsetIndex(offset)) {
							m_Output->set(grow_iter, sink_count);
							grow_iter.accept();
						}
					}
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typedef Image<int> InternalImageType;
			typename InputImageType::ConstantPointer m_Input;
			typename OutputImageType::Pointer m_Output;
			ImageNeighborIterator m_NeighborIter;
			InternalImageType::Pointer m_OffsetIndexLookup;
			boost::scoped_array<double> m_Delta;

			int toOffsetIndex(const Point3D<int>& p)
			{
				return m_OffsetIndexLookup->get(p);
			}
			const Point3D<int>& toOffsetPoint(int index)
			{
				return m_NeighborIter.getOffset(index);
			}

			//**************** Flow computation related
			InternalImageType::Pointer computeFlowImage()
			{
				InternalImageType::Pointer flow_image = InternalImageType::New(m_Input);
				misc::SafeUnsafeRegionGenerator region_gen(m_Input->getRegion(), m_Input->getRegion(), m_NeighborIter.getOffsetRegion());
				ImageRegionsIteratorWithPoint<> iter(m_Input);
				iter.add(region_gen.getSafeRegion(), 1);
				iter.addList(region_gen.getUnsafeRegion(), 0);
				//Computing the flow and identifying sink
				pcl_ForIterator(iter) {
					double steepest_flow = 0;
					int steepest_direction = -1;
					m_NeighborIter.setOrigin(iter);
					pcl_ForIterator(m_NeighborIter) if (iter.getInfo() || m_Input->contain(m_NeighborIter.getPoint())) {
						double gradient = (m_Input->get(iter)-m_Input->get(m_NeighborIter))/m_Delta[m_NeighborIter.getIteration()];
						if (gradient>0 && gradient>steepest_flow) {
							steepest_flow = gradient;
							steepest_direction = m_NeighborIter.getIteration();
						}
					}
					if (steepest_direction>=0) {
						flow_image->set(iter, steepest_direction);
					} else {
						flow_image->set(iter, -1);
					}
				}
				return flow_image;
			}

			//**************** Flow correction related
			int findDecendingNeighbor(InternalImageType::Pointer& flow_image, typename InputImageType::ConstantValueType origin_val) 
			{
				int selected_flow =-1;
				double selected_grad = 0;
				pcl_ForIterator(m_NeighborIter) if (m_Input->contain(m_NeighborIter.getPoint())) {
					int cur_flow = flow_image->get(m_NeighborIter);
					if (cur_flow!=-1) {
						Point3D<int> neighbor_neighbor_point(m_NeighborIter.getPoint());
						neighbor_neighbor_point += toOffsetPoint(cur_flow);
						auto neighbor_neighbor_val = m_Input->get(neighbor_neighbor_point);
						if (origin_val>neighbor_neighbor_val) { //This is to prevent the neighbor from pointing to the source
							double grad = (m_Input->get(m_NeighborIter)-neighbor_neighbor_val)/m_Delta[cur_flow];
							if (grad>selected_grad) {
								selected_flow = m_NeighborIter.getIteration();
								selected_grad = grad;
							}
						}
					}
				}
				return selected_flow;
			}

			void correctFlowImage(InternalImageType::Pointer& flow_image)
			{
				typedef std::list<boost::shared_ptr<BruteRegionGrowingIterator> > RegionGrowingListType;
				RegionGrowingListType region_growing_list;
				//Creating the region growing list for overcoming plateau
				ImageIteratorWithPoint image_iter(flow_image);
				pcl_ForIterator(image_iter) if (flow_image->get(image_iter)==-1) {
					m_NeighborIter.setOrigin(image_iter.getPoint(), image_iter);
					int decending_flow = findDecendingNeighbor(flow_image, m_Input->get(image_iter));
					if (decending_flow!=-1) {
						flow_image->set(image_iter, decending_flow);

						boost::shared_ptr<BruteRegionGrowingIterator> reggrow_ptr(new BruteRegionGrowingIterator);
						reggrow_ptr->setNeighborIterator(m_NeighborIter, m_Input->getRegion());
						reggrow_ptr->addSeed(image_iter.getPoint(), image_iter);
						reggrow_ptr->begin();
						region_growing_list.push_back(reggrow_ptr);
					}
				}
				//Overwriting flow according to the region growing list
				bool done = false;
				while (!done) {
					done = true;
					pcl_ForEach(region_growing_list, item) if (*item) {
						BruteRegionGrowingIterator &iter = **item;
						for(; !iter.end(); iter.next()) if (flow_image->get(iter)==-1) {
							Point3D<int> offset(iter.getSourcePoint());
							offset -= iter.getPoint();
							flow_image->set(iter, toOffsetIndex(offset));
							done = false;
							iter.accept();
							iter.next();
							break;
						}
						if (iter.end()) {
							item->reset();
						}
					}
				}
				//Merging persisting group of sink
				pcl_ForIterator(image_iter) if (flow_image->get(image_iter)==-1) {
					BruteRegionGrowingIterator iter;
					iter.setNeighborIterator(m_NeighborIter, m_Input->getRegion());
					iter.addSeed(image_iter.getPoint(), image_iter);
					pcl_ForIterator(iter) if (iter.getIndex()!=image_iter.getIndex() && flow_image->get(iter)==-1) {
						Point3D<int> offset(iter.getSourcePoint());
						offset -= iter.getPoint();
						flow_image->set(iter, toOffsetIndex(offset));
						iter.accept();
					}
				}
			}
		};

	}
}

#endif