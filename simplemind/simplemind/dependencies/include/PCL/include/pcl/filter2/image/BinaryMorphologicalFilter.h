#ifndef PCL2_BINARY_DILATION_FILTER
#define PCL2_BINARY_DILATION_FILTER

#include <pcl/iterator.h>
#include <pcl/image/ImageAlgorithm.h>
#include <pcl/filter2/image/ImageFilterBase.h>

namespace pcl
{
	namespace filter2
	{
		using namespace pcl;
		using namespace pcl::iterator;
		
		class Dilation
		{
		public:
			Dilation() {}
			
			template <class T>
			T getForeGround(T fg, T bg)
			{
				return fg;
			}
			
			template <class T>
			T getBackGround(T fg, T bg)
			{
				return bg;
			}
			
			template <class T>
			bool operator()(T val)
			{
				return val>0;
			}
		};
		
		class Erosion
		{
		public:
			Erosion() {}
			
			template <class T>
			T getForeGround(T fg, T bg)
			{
				return bg;
			}
			
			template <class T>
			T getBackGround(T fg, T bg)
			{
				return fg;
			}
			
			template <class T>
			bool operator()(T val)
			{
				return val<=0;
			}			
		};


		//Based on "AN EFFICIENT ALGORITHM FOR 3D BINARY MORPHOLOGICAL TRANSFORMATIONS WITH 3D STRUCTURING ELEMENTS OF ARBITRARY SIZE AND SHAPE" 
		//by N. Nikopoulos and I. Pitas
		template <class OperationType, class BoundaryHandler, class OutputImageType, class InternalCountImageType=pcl::Image<int>>
		class BinaryMorphologicalFilter: public ImageFilterBase
		{
		protected:
			typedef typename OutputImageType::IoValueType OutputValueType;

		public:

			template <class T>
			static typename boost::enable_if<boost::is_base_of<pcl::ImageBase, typename ptr_base_type<T>::type>, typename OutputImageType::Pointer>::type Compute(const BoundaryHandler& input, const T& structuring_element_image, const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				BinaryMorphologicalFilter filter;
				filter.setOutputRegion(output_region);
				filter.setValue(fore_ground, back_ground);
				filter.setInput(input);
				filter.setStructuringElementImage(structuring_element_image);
				filter.update();
				return filter.getOutput();
			}
			template <class T>
			static typename boost::disable_if<boost::is_base_of<pcl::ImageBase, typename ptr_base_type<T>::type>, typename OutputImageType::Pointer>::type Compute(const BoundaryHandler& input, const T& structuring_element, const OutputValueType& fore_ground=1, const OutputValueType& back_ground=0, pcl::Region3D<int>& output_region=pcl::Region3D<int>().reset())
			{
				BinaryMorphologicalFilter filter;
				filter.setOutputRegion(output_region);
				filter.setValue(fore_ground, back_ground);
				filter.setInput(input);
				filter.setStructuringElement(structuring_element);
				filter.update();
				return filter.getOutput();
			}

			BinaryMorphologicalFilter() 
			{
				setValue(1,0);
				m_OutputRegion.reset();
			}

			void setValue(const OutputValueType& fore_ground, const OutputValueType& back_ground)
			{
				m_ForeGroundValue = m_Op.getForeGround(fore_ground, back_ground);
				m_BackGroundValue = m_Op.getBackGround(fore_ground, back_ground);
			}

			void setInput(const BoundaryHandler& input)
			{
				m_Input = input;
			}

			void setStructuringElement(const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& se)
			{
				pcl::Region3D<int> region; region.reset();
				pcl_ForEach(*se, item) region.add(*item);
				auto image = pcl::Image<char>::New(region.getMinPoint(), region.getMaxPoint());
				pcl::ImageHelper::Fill(image);
				pcl_ForEach(*se, item) image->set(*item, 1);
				setStructuringElementImage(image);
			}

			template <class ImagePointerType>
			void setStructuringElementImage(const ImagePointerType& image)
			{
				auto size = image->getSize();
				for (int i=0; i<3; ++i) {
					if (size[i]>1) m_CheckOffset[i] = 1;
					else m_CheckOffset[i] = 0;
				}
				m_NeighborOffset.reset(new pcl::iterator::ImageNeighborIterator::OffsetListType());
				m_NeighborOffset->reserve(26);
				for (int z=-m_CheckOffset[2]; z<=m_CheckOffset[2]; ++z)
					for (int y=-m_CheckOffset[1]; y<=m_CheckOffset[1]; ++y)
						for (int x=-m_CheckOffset[0]; x<=m_CheckOffset[0]; ++x) 
							if (!(x==0 && y==0 && z==0)) 
								m_NeighborOffset->push_back(pcl::Point3D<int>(x,y,z));

				m_ElemPoints.clear();
				m_ElemConnPoints.clear();
				m_ElemSurfD.clear();
				m_ElemSurfD.resize(m_NeighborOffset->size());

				pcl::iterator::ImageNeighborIterator n_iter(image, m_NeighborOffset);
				pcl::ImageIteratorWithPoint iter(image);
				pcl_ForIterator(iter) if (image->get(iter)>0) {
					m_ElemPoints.push_back(iter.getPoint());

					n_iter.setOrigin(iter.getPoint(), iter);
					pcl_ForIterator(n_iter) {
						if (!image->contain(n_iter.getPoint()) || image->get(n_iter)<=0) {
							m_ElemSurfD[n_iter.getIteration()].push_back(iter.getPoint());
						}
					}
				}

				std::vector<pcl::ComponentInfo> info;
				auto count_image = pcl::ImageAlgorithm::ConnectedComponentAnalysis<InternalCountImageType>(image, m_NeighborOffset, info);
				for (int i=1; i<info.size(); ++i) {
					iter.setRegion(info[i].region);
					pcl_ForIterator(iter) if (count_image->get(iter)==i) {
						m_ElemConnPoints.push_back(iter.getPoint());
						break;
					}
				}
			}

			void update()
			{
				//Computing the surface
				auto computation_region = this->getOutputRegion(m_Input.getImage());

				computation_region.getMinPoint() -= m_CheckOffset;
				computation_region.getMaxPoint() += m_CheckOffset;
				auto surf_image = this->createImage<pcl::Image<char>>(m_Input.getImage(), computation_region);
				{
					pcl::Region3D<int> safe_region(surf_image->getRegion());
					safe_region.setIntersect(m_Input.getImage()->getRegion());
					safe_region.getMinPoint() += m_CheckOffset;
					safe_region.getMaxPoint() -= m_CheckOffset;
					auto unsafe_regions = surf_image->getRegion().getRegionsAfterSubtractionBy(safe_region);
					
					pcl::ImageRegionsIteratorWithPoint<> surf_iter(surf_image), in_iter(m_Input.getImage());
					surf_iter.add(safe_region, 1); in_iter.add(safe_region, 1);
					surf_iter.addList(unsafe_regions, 0); in_iter.addList(unsafe_regions, 0);
					pcl::iterator::ImageNeighborIterator n_iter(m_Input.getImage(), m_NeighborOffset);
					pcl_ForIterator2(surf_iter, in_iter) {
						bool is_border = false;
						if (in_iter.getInfo()==1) {
							if (m_Op(m_Input.getImage()->get(in_iter))) {
								n_iter.setOrigin(in_iter.getPoint(), in_iter);
								pcl_ForIterator(n_iter) if (!m_Op(m_Input.getImage()->get(n_iter))) {
									is_border = true;
									break;
								}
							}
						} else {
							if (m_Op(m_Input.get(in_iter))) {
								n_iter.setOrigin(in_iter.getPoint(), in_iter);
								pcl_ForIterator(n_iter) if (!m_Op(m_Input.get(n_iter.getPoint()))) {
									is_border = true;
									break;
								}
							}
						}
						if (is_border) surf_image->set(surf_iter, 1);
						else surf_image->set(surf_iter, 0);
					}
				}

				//Getting the connected surfaces
				auto surf_count_image = pcl::ImageAlgorithm::ConnectedComponentAnalysis<InternalCountImageType>(surf_image, m_NeighborOffset);
				surf_image.reset(); //Clearing surface image as it is no longer needed

				//Generating lookup for offsets
				auto offset_lookup = pcl::Image<char>::New(Point3D<int>(-1,-1,-1), Point3D<int>(1,1,1));
				for (int i=0; i<m_NeighborOffset->size(); ++i) {
					offset_lookup->set((*m_NeighborOffset)[i], i);
				}
				//Constructing links
				std::vector<std::vector<LinkInfo>> link;
				{
					pcl::ImageIteratorWithPoint iter(surf_count_image);
					pcl::iterator::ImageNeighborIterator n_iter(surf_count_image, m_NeighborOffset);
					pcl_ForIterator(iter) if (surf_count_image->get(iter)>0) {
						std::vector<LinkInfo> cur_link;
						pcl::iterator::BruteRegionGrowingIterator r_iter;
						r_iter.setNeighborIterator(n_iter, surf_count_image->getRegion());
						r_iter.addSeed(iter.getPoint(), iter);
						cur_link.push_back(LinkInfo(iter.getPoint()));
						surf_count_image->set(iter, -cur_link.size());
						pcl_ForIterator(r_iter) if (surf_count_image->get(r_iter)>0) {
							r_iter.accept();
							//Creating new link
							cur_link.push_back(LinkInfo(r_iter.getPoint()));
							surf_count_image->set(r_iter, -cur_link.size());
							//Updating parent link
							int source_index = -(surf_count_image->get(r_iter.getSource()))-1;
							cur_link[source_index].link.push_back(offset_lookup->get(r_iter.getOffset()));
						}
						link.push_back(std::move(cur_link));
					}
				}
				surf_count_image.reset(); //Clearing surface count image as it is no longer needed

				//Generating resulting image
				m_Output = this->createImage<OutputImageType>(m_Input.getImage());
				pcl::ImageHelper::Fill(m_Output, m_BackGroundValue);
				//Compute the first part of the equation: U_p_in_X U_i_in_[1;n(B)] (p + p_i(B))
				pcl::Region3D<int> covered_region; covered_region.reset();
				{
					pcl::ImageIteratorWithPoint iter(m_Input.getImage());
					pcl_ForIterator(iter) if (m_Op(m_Input.getImage()->get(iter))) {
						pcl_ForEach(m_ElemConnPoints, item) {
							auto p = iter.getPoint()+*item;
							if (m_Output->contain(p)) {
								covered_region.add(p);
								m_Output->set(p, m_ForeGroundValue);
							}
						}
					}
				}
				{ //Compensating for places not covered
					pcl::ImageRegionsIteratorWithPoint<> iter(m_Output);
					iter.addList(m_Output->getRegion().getRegionsAfterSubtractionBy(covered_region));
					pcl_ForEach(m_ElemConnPoints, item) {
						pcl_ForIterator(iter) {
							auto p = iter.getPoint()-*item; //The direction now is inversed
							if (m_Op(m_Input.get(p))) m_Output->set(iter, m_ForeGroundValue);
						}
					}
				}
				//Compute the second part of the equation that starts with U_i_in_[1;n(X)]
				pcl_ForEach(link, link_item) {
					//First part: U_p_in_A(B) (p_s_(i,1) + p)
					auto psi1 = (*link_item)[0];
					pcl_ForEach(m_ElemPoints, item) {
						auto p = psi1.point + *item;
						if (m_Output->contain(p)) m_Output->set(p, m_ForeGroundValue);
					}
					//Second part: U_j_in_[1;N_Si] U_l_in_[1;l(ps_(i,j)] U_p_in_Adl(PSi,j)(B) (p_si,j + udl(ps_(i,j)) + p)
					pcl_ForEach(*link_item, cur_link) {
						pcl_ForEach(cur_link->link, cur_link_ind) {
							auto udl = (*m_NeighborOffset)[*cur_link_ind];
							pcl_ForEach(m_ElemSurfD[*cur_link_ind], item) {
								auto p = cur_link->point + udl + *item;
								if (m_Output->contain(p)) m_Output->set(p, m_ForeGroundValue);
							}
						}
					}
				}
				if (m_Output->getRegion()!=this->getOutputRegion(m_Input.getImage())) {
					m_Output = pcl::ImageHelper::GetCroppedAuto(m_Output, this->getOutputRegion(m_Input.getImage()));
				}
			}

			typename OutputImageType::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			OperationType m_Op;
			pcl::iterator::ImageNeighborIterator::OffsetListPointer m_NeighborOffset;
			pcl::Point3D<int> m_CheckOffset;
			std::vector<Point3D<int>> m_ElemPoints, m_ElemConnPoints;
			std::vector<std::vector<Point3D<int>>> m_ElemSurfD;

			struct LinkInfo {
				Point3D<int> point;
				std::vector<char> link;

				LinkInfo(const Point3D<int>& p)
				{
					point = p;
				}
			};

			BoundaryHandler m_Input;
			typename OutputImageType::Pointer m_Output;
			OutputValueType m_ForeGroundValue, m_BackGroundValue;
		};

	}
}

#endif