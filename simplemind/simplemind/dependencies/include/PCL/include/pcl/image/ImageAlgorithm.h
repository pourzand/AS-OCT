#ifndef PCL_IMAGE_ALGORITHM
#define PCL_IMAGE_ALGORITHM

#include <pcl/image/ImageHelper.h>
#include <pcl/image/ImageAlgorithmObject.h>
#include <pcl/iterator/ImageNeighborIterator.h>
#include <pcl/iterator/RegionGrowingIterator.h>
#include <pcl/misc/SafeUnsafeRegionGenerator.h>
#include <pcl/iterator/BruteRegionGrowingIterator.h>
#include <boost/numeric/conversion/bounds.hpp>
#include <pcl/exception.h>

namespace pcl
{

	class ImageAlgorithm
	{
	public:
		/************** Loop and retain value **************/
		template <class ValueType, class IteratorType, class DecisionFuncType>
		static typename boost::enable_if<getPoint_exists<IteratorType>, PointIndexObject>::type LoopAndRetain(IteratorType& iter, const ValueType& init_value, DecisionFuncType& dec_func, ValueType* return_val=NULL)
		{
			PointIndexObject result;
			result.index = -1;
			ValueType value = init_value;
			pcl_ForIterator(iter) {
				if (dec_func(iter, value)) result = iter;
			}
			if (return_val) *return_val = value;
			return result;
		}
		template <class ValueType, class IteratorType, class DecisionFuncType>
		static typename boost::disable_if<getPoint_exists<IteratorType>, long>::type LoopAndRetain(IteratorType& iter, const ValueType& init_value, DecisionFuncType& dec_func, ValueType* return_val=NULL)
		{
			long result = -1;
			ValueType value = init_value;
			pcl_ForIterator(iter) {
				if (dec_func(iter, value)) result = iter;
			}
			if (return_val) *return_val = value;
			return result;
		}

		template <class ValueType, class IteratorType, class InitialFuncType, class DecisionFuncType>
		static typename boost::enable_if<getPoint_exists<IteratorType>, PointIndexObject>::type LoopAndRetain(IteratorType& iter, InitialFuncType& init_func, DecisionFuncType& dec_func, ValueType* return_val=NULL)
		{
			PointIndexObject result;
			result.index = -1;
			ValueType value;
			for (iter.begin(); !iter.end(); iter.next()) {
				if (init_func(iter, value)) {
					result = iter;
					iter.next();
					break;
				}
			}
			for (; !iter.end(); iter.next()) {
				if (dec_func(iter, value)) result = iter;
			}
			if (return_val) *return_val = value;
			return result;
		}
		template <class ValueType, class IteratorType, class InitialFuncType, class DecisionFuncType>
		static typename boost::disable_if<getPoint_exists<IteratorType>, long>::type LoopAndRetain(IteratorType& iter, InitialFuncType& init_func, DecisionFuncType& dec_func, ValueType* return_val=NULL)
		{
			long result = -1;
			ValueType value;
			for (iter.begin(); !iter.end(); iter.next()) {
				if (init_func(iter, value)) {
					result = iter;
					iter.next();
					break;
				}
			}
			for (; !iter.end(); iter.next()) {
				if (dec_func(iter, value)) result = iter;
			}
			if (return_val) *return_val = value;
			return result;
		}

		/************** Scan voxel neighborhood **************/
		template <class ImagePointer, class Cond, class Process>
		static void ScanVoxelNeighborhood(const ImagePointer& image, const iterator::ImageNeighborIterator::ConstantOffsetListPointer& list,
			Cond cond, Process process) 
		{
			ImageIteratorWithPoint iter(image);
			iterator::ImageNeighborIterator neighbor_iter(image, list);
			misc::SafeUnsafeRegionGenerator rgn_gen(image->getRegion(), image->getRegion(), neighbor_iter.getOffsetRegion());
			if (!rgn_gen.getSafeRegion().empty()) {
				iter.setRegion(rgn_gen.getSafeRegion());
				pcl_ForIterator(iter) if (cond(iter)) {
					neighbor_iter.setOrigin(iter.getPoint());
					pcl_ForIterator(neighbor_iter) {
						if (process(neighbor_iter)) break;
					}
				}
			}
			pcl_ForEach(rgn_gen.getUnsafeRegion(), item) {
				iter.setRegion(*item);
				pcl_ForIterator(iter) if (cond(iter)) {
					neighbor_iter.setOrigin(iter.getPoint());
					pcl_ForIterator(neighbor_iter) if (image->contain(neighbor_iter.getPoint()) && cond(iter)) {
						if (process(neighbor_iter)) break;
					}
				}
			}
		}
		
		/************** Connected component analysis **************/
		template <class CountImageType, class ImagePointer>
		static typename CountImageType::Pointer ConnectedComponentAnalysis(const ImagePointer& image, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& list, typename CountImageType::ValueType *component_count=NULL)
		{
			return ConnectedComponentAnalysis<CountImageType>(image, list, [&](const pcl::Point3D<int>& p, long i){
				return image->get(i)!=0;
			}, component_count);
		}

		template <class CountImageType, class ImagePointer, class DecisionFunction>
		static typename CountImageType::Pointer ConnectedComponentAnalysis(const ImagePointer& image, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& list, DecisionFunction decision, typename CountImageType::ValueType *component_count=NULL)
		{
			typedef typename CountImageType::ValueType CountType;
			auto count_image = CountImageType::New(image);
			ImageHelper::Fill(count_image, static_cast<CountType>(0));
			CountType count = static_cast<CountType>(0);
			ImageIteratorWithPoint iter(image);
			iterator::BruteRegionGrowingIterator rgn_iter;
			rgn_iter.setNeighborIterator(list, image);
			pcl_ForIterator(iter) {
				if (count_image->get(iter)==static_cast<CountType>(0)) {
					if (decision(iter.getPoint(), iter)) {
						if (count==boost::numeric::bounds<CountType>::highest()) {
							pcl_ThrowException(pcl::Exception(), "Count overloaded!");
						}
						++count;
						count_image->set(iter, count);
						rgn_iter.addSeed(iter.getPoint(), iter);
						pcl_ForIterator(rgn_iter) if (count_image->get(rgn_iter)==static_cast<CountType>(0)) {
							if (decision(rgn_iter.getPoint(), rgn_iter)) {
								count_image->set(rgn_iter, count);
								rgn_iter.accept();
							}
						}
					}
				}
			}
			if (component_count!=NULL) *component_count = count;
			return count_image;
		}

		template <class CountImageType, class ImagePointer>
		static typename CountImageType::Pointer ConnectedComponentAnalysis(const ImagePointer& image, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& list, std::vector<ComponentInfo>& info)
		{
			return ConnectedComponentAnalysis<CountImageType>(image, list, info, [&](const pcl::Point3D<int>& p, long i){
				return image->get(i)!=0;
			});
		}
		
		template <class CountImageType, class ImagePointer, class DecisionFunction>
		static typename CountImageType::Pointer ConnectedComponentAnalysis(const ImagePointer& image, const pcl::iterator::ImageNeighborIterator::ConstantOffsetListPointer& list, std::vector<ComponentInfo>& info, DecisionFunction decision)
		{
			info.clear();
			info.reserve(10);
			info.push_back(ComponentInfo());
			typedef typename CountImageType::ValueType CountType;
			auto count_image = CountImageType::New(image);
			ImageHelper::Fill(count_image, static_cast<CountType>(0));
			CountType count = static_cast<CountType>(0);
			ImageIteratorWithPoint iter(image);
			iterator::BruteRegionGrowingIterator rgn_iter;
			rgn_iter.setNeighborIterator(list, image);
			pcl_ForIterator(iter) {
				if (count_image->get(iter)==static_cast<CountType>(0)) {
					if (decision(iter.getPoint(), iter)) {
						if (count==boost::numeric::bounds<CountType>::highest()) {
							pcl_ThrowException(pcl::Exception(), "Count overloaded!");
						}
						++count;
						info.push_back(ComponentInfo());
						info[count].add(iter.getPoint());
						count_image->set(iter, count);
						rgn_iter.addSeed(iter.getPoint(), iter);
						pcl_ForIterator(rgn_iter) if (count_image->get(rgn_iter)==static_cast<CountType>(0)) {
							if (decision(rgn_iter.getPoint(), rgn_iter)) {
								info[count].add(rgn_iter.getPoint());
								count_image->set(rgn_iter, count);
								rgn_iter.accept();
							}
						}
					} else {
						info[0].add(iter.getPoint());
					}
				}
			}
			return count_image;
		}

		/************** Extract Border **************/
		enum {
			MIN_X_BORDER = 1,
			MAX_X_BORDER = 2,
			MIN_Y_BORDER = 4,
			MAX_Y_BORDER = 8,
			MIN_Z_BORDER = 16,
			MAX_Z_BORDER = 32,
			BORDER_3D = 63,
			BORDER_2D = 15
		};

		template <class ResultImageType, class InputImagePointer>
		static typename ResultImageType::Pointer ExtractBorder(const InputImagePointer& input, unsigned int border_type=BORDER_3D)
		{
			return ExtractBorder<ResultImageType>(input, [&](const pcl::Point3D<int>& p, long i){
				return input->get(i)!=0;
			}, border_type);
		}

		template <class ResultImageType, class InputImagePointer, class DecisionFunction>
		static typename ResultImageType::Pointer ExtractBorder(const InputImagePointer& input, DecisionFunction func, unsigned int border_type=BORDER_3D)
		{
			auto result = ResultImageType::New(input);
			pcl::ImageHelper::Fill(result, 0);

			pcl::iterator::RegionGrowingIterator<ResultImageType> rgn_iter;
			rgn_iter.setNeighborIterator(pcl::iterator::ImageNeighborIterator::CreateConnect6Offset());
			rgn_iter.setMarkerImage(result);

			pcl::ImageIteratorWithPoint iter(input);
			unsigned int check = 1;
			for (int axis=0; axis<3; ++axis) {
				pcl::Region3D<int> region = input->getRegion();
				int range[] = {
					input->getMinPoint()[axis],
					input->getMaxPoint()[axis]
				};
				for (int m=0; m<2; ++m) {
					if (check&border_type) {
						region.getMinPoint()[axis] = region.getMaxPoint()[axis] = range[m];
						iter.setRegion(region);
						pcl_ForIterator(iter) if (func(iter.getPoint(), iter)) rgn_iter.addSeed(iter.getPoint(), iter);
					}
					check <<= 1;
				}
			}

			pcl_ForIterator(rgn_iter) if (func(rgn_iter.getPoint(), rgn_iter)) rgn_iter.accept();			
			return result;
		}

		/************** Hole filler **************/

		template <class ResultImageType, class InputImagePointer>
		static typename ResultImageType::Pointer FillHoles(const InputImagePointer& input, unsigned int border_type=BORDER_3D)
		{
			return FillHoles<ResultImageType>(input, [&](const pcl::Point3D<int>& p, long i){
				return input->get(i)!=0;
			}, border_type);
		}

		template <class ResultImageType, class InputImagePointer, class DecisionFunction>
		static typename ResultImageType::Pointer FillHoles(const InputImagePointer& input, DecisionFunction func, unsigned int border_type=BORDER_3D)
		{
			auto result = ExtractBorder<ResultImageType>(input, [&](const pcl::Point3D<int>& p, long i) {
				return !func(p,i);
			}, border_type);
			pcl::ImageIterator iter(result);
			pcl_ForIterator(iter) if (result->get(iter)!=0) result->set(iter, 0);
			else result->set(iter, 1);
			return result;
		}
	};

}

#endif