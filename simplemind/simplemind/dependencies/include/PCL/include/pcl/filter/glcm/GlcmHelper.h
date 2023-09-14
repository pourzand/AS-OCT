#ifndef PCL_GLCM_HELPER
#define PCL_GLCM_HELPER

#include <pcl/filter/glcm/GrayLevelCooccurrenceMatrix.h>
#include <pcl/image.h>

namespace pcl
{
	namespace filter
	{
        
		class GlcmHelper
		{
		public:
			template <class ImagePointerType>
			static PointIndexObject GetOffset(const Point3D<int>& p, const ImagePointerType& image)
			{
				long const *offset_table = image->getOffsetTable();
				long index = 0;
				for (int i=0; i<3; ++i) {
					index += offset_table[i]*p[i];
				}
				return PointIndexObject(p, index);
			}

			template <class ImagePointerType, class IteratorType>
			static GrayLevelCooccurrenceMatrix& ComputeFromIterator(GrayLevelCooccurrenceMatrix& glcm, const ImagePointerType& image, IteratorType& iter, const PointIndexObject& offset)
			{
				PointIndexObject neighbor;
				pcl_ForIterator(iter) if (image->contain(iter.getPoint())) {
					neighbor.point = iter.getPoint()+offset.point;
					neighbor.index = iter.getIndex()+offset.index;
					if (image->contain(neighbor.point)) {
						glcm.add(pcl::round(image->get(iter)), pcl::round(image->get(neighbor)));
					}
				}
				return glcm;
			}

			template <class ImagePointerType, class IteratorType, class AcceptPoint, class AcceptNeighbor>
			static GrayLevelCooccurrenceMatrix& ComputeFromIterator(GrayLevelCooccurrenceMatrix& glcm, const ImagePointerType& image, IteratorType& iter, const PointIndexObject& offset, AcceptPoint& accp, AcceptNeighbor& accn)
			{
				PointIndexObject neighbor;
				pcl_ForIterator(iter) if (image->contain(iter.getPoint()) && accp(iter.getPoint(), iter.getIndex())) {
					neighbor.point = iter.getPoint()+offset.point;
					neighbor.index = iter.getIndex()+offset.index;
					if (image->contain(neighbor.point) && accn(neighbor.point, neighbor.index)) {
						glcm.add(pcl::round(image->get(iter)), pcl::round(image->get(neighbor)));
					}
				}
				return glcm;
			}
			
		    template <class ImagePointerType, class IteratorType>
			static GrayLevelCooccurrenceMatrix& ComputeFromIteratorWithDynamicQuantization(GrayLevelCooccurrenceMatrix& glcm, const ImagePointerType& image, IteratorType& iter, const PointIndexObject& offset)
            {
                return ComputeFromIteratorWithDynamicQuantization(glcm, image, iter, offset, [](const pcl::Point3D<int>&, long)->bool {return true;});
            }
		

            /**
             * Creates a graylevel co-occurrence matrix with dynamic quantization, based on the given parameters.
             * Dynamic quantization involves mapping the graylevel scale of the original image to the specified number of graylevel bins.
             * The formula is
             *   binNumber = (value - minValue) / (maxValue - minValue + 1) * numGraylevels
             * where minValue and maxValue are determined from the given image and iterator
             *
             * Parameters:
             *   - glcm -- Reference to appropriately-sized GLCM object; number of graylevel bins is determined from GLCM size
             *   - image -- Pointer to image from which to create GLCM
             *   - iter -- The iterator object that specifies the region over which to compute the GLCM
             *   - offset -- The offset (step distance and direction) to use in computing the GLCM
             *   - check_func -- A lambda function that evaluates a point to determine whether it is within bounds for computing GLCM
             *
             * Returns a GrayLevelCooccurrenceMatrix object, which can then be passed to the GlcmFeatures class to compute features.
             */
			template <class ImagePointerType, class IteratorType, class CheckFunc>
			static GrayLevelCooccurrenceMatrix& ComputeFromIteratorWithDynamicQuantization(GrayLevelCooccurrenceMatrix& glcm, const ImagePointerType& image, IteratorType& iter, const PointIndexObject& offset, CheckFunc& check_func, bool useOldBinCounting = false)
			{
//clock.tic();
				PointIndexObject neighbor;
				double min_val=std::numeric_limits<double>::infinity(), max_val=-std::numeric_limits<double>::infinity();
				unsigned int glcm_size = glcm.size();
                double step;
//elapsed1 += clock.toc().getClock();

                // Determine min and max value within image/region
//clock.tic();
				pcl_ForIterator(iter) if (image->contain(iter.getPoint())) {
					double val = image->get(iter);
                    Point3D<int> point = iter.getPoint();
					if (min_val>val) min_val = val;
					if (max_val<val) max_val = val;
				}
//elapsed2 += clock.toc().getClock();

//clock.tic();
                if (useOldBinCounting)
                {
                    step = (max_val - min_val) / glcm_size;         // Old (incorrect) version
                }
                else
                {
                    step = (max_val - min_val + 1) / glcm_size;     // Fixed by Danny 2015-Jun-18
                }
//elapsed3 += clock.toc().getClock();

                // There are superfluous checks being made in the below loop
                // In particular, image->contain(neighbor.point) and check_func(neighbor.point, neighbor.index) probably have some redundancy
                
                // Loop through image iterator
//clock.tic();
                pcl_ForIterator(iter) if (image->contain(iter.getPoint())) {
                    // Determine neighbor of current point, as defined by the specified offset
					neighbor.point = iter.getPoint()+offset.point;
					neighbor.index = iter.getIndex()+offset.index;

                    // Check whether the neighbor is contained within the image/acceptable region
					if (image->contain(neighbor.point)) {
                        if (check_func(neighbor.point, neighbor.index))
                        {
                            // Compute bin number of current point
						    unsigned im_val = static_cast<unsigned>(std::floor((image->get(iter)-min_val)/step));
						    im_val = im_val<0?0:im_val;
						    im_val = im_val>=glcm_size?glcm_size-1:im_val;

                            // Compute bin number of neighbor
						    unsigned ng_val = static_cast<unsigned>(std::floor((image->get(neighbor)-min_val)/step));
						    ng_val = ng_val<0?0:ng_val;
						    ng_val = ng_val>=glcm_size?glcm_size-1:ng_val;

                            // Increment GLCM corresponding to current and neighbor bin numbers
						    glcm.add(im_val, ng_val);
                        }
					}
				}
//elapsed4 += clock.toc().getClock();
				return glcm;
			}
			template <class ImagePointerType, class IteratorType, class AcceptPoint, class AcceptNeighbor>
			static GrayLevelCooccurrenceMatrix& ComputeFromIteratorWithDynamicQuantization(GrayLevelCooccurrenceMatrix& glcm, const ImagePointerType& image, IteratorType& iter, const PointIndexObject& offset, AcceptPoint& accp, AcceptNeighbor& accn)
			{
				PointIndexObject neighbor;
				double min_val=std::numeric_limits<double>::infinity(), max_val=-std::numeric_limits<double>::infinity();
				unsigned int glcm_size = glcm.size();
				pcl_ForIterator(iter) if (image->contain(iter.getPoint()) && accp(iter.getPoint(), iter.getIndex())) {
					double val = image->get(iter);
					if (min_val>val) min_val = val;
					if (max_val<val) max_val = val;
				}
				double step = (max_val - min_val + 1) / glcm_size;
				pcl_ForIterator(iter) if (image->contain(iter.getPoint()) && accp(iter.getPoint(), iter.getIndex())) {
					neighbor.point = iter.getPoint()+offset.point;
					neighbor.index = iter.getIndex()+offset.index;
					if (image->contain(neighbor.point) && accn(neighbor.point, neighbor.index)) {
						// Compute bin number of current point
						unsigned im_val = static_cast<unsigned>(std::floor((image->get(iter)-min_val)/step));
						im_val = im_val<0?0:im_val;
						im_val = im_val>=glcm_size?glcm_size-1:im_val;

						// Compute bin number of neighbor
						unsigned ng_val = static_cast<unsigned>(std::floor((image->get(neighbor)-min_val)/step));
						ng_val = ng_val<0?0:ng_val;
						ng_val = ng_val>=glcm_size?glcm_size-1:ng_val;

						// Increment GLCM corresponding to current and neighbor bin numbers
						glcm.add(im_val, ng_val);
					}
				}
				return glcm;
			}
		};

	}
}

#endif