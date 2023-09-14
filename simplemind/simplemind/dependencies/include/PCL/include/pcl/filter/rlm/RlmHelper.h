#ifndef PCL_RLM_HELPER
#define PCL_RLM_HELPER

#include <pcl/filter/rlm/RunLengthMatrix.h>
#include <pcl/image.h>

namespace pcl
{
	namespace filter
	{
		class RlmHelper
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

            /**
             * Creates a run length matrix with dynamic quantization, based on the given parameters.
             * Dynamic quantization involves mapping the graylevel scale of the original image to the specified number of graylevel bins.
             * The formula is
             *   binNumber = (value - minValue) / (maxValue - minValue + 1) * numGraylevels
             * where minValue and maxValue are determined from the given image and iterator
             *
             * Parameters:
             *   - image -- Pointer to image from which to create RLM
             *   - numGraylevels -- The number of graylevel bins to use for dynamic quantization
             *   - iter -- The iterator object that specifies the region over which to compute the RLM
             *   - offset -- The offset (step distance and direction) to use in computing the RLM
             *   - checkFunc -- A lambda function that evaluates a point to determine whether it is within bounds for computing RLM
             *
             * Returns a RunLengthMatrix object, which can then be passed to the RlmFeatures class to compute features.
             */
			template <class ImagePointerType, class IteratorType, class CheckFunc>
			static RunLengthMatrix ComputeFromIteratorWithDynamicQuantization(const ImagePointerType& image, int numGraylevels, IteratorType& iter, const PointIndexObject& offset, CheckFunc& checkFunc, bool useOldBinCounting = false)
			{
                RunLengthMatrix rlm;
                std::vector<BinRunLengthStruct> runLengthList;
                int maxRunLength = std::numeric_limits<int>::min();

                Image<bool, true>::Pointer visited;
				PointIndexObject neighbor;
				double minValue =  std::numeric_limits<double>::infinity();
                double maxValue = -std::numeric_limits<double>::infinity();
                Point3D<int> minp, maxp;

                // Determine min and max value/point for the image and region
                minp = image->getMaxPoint();
                maxp = image->getMinPoint();
				pcl_ForIterator(iter) if (image->contain(iter.getPoint())) {
					double val = image->get(iter);
                    Point3D<int> point = iter.getPoint();

					if (minValue > val) minValue = val;
					if (maxValue < val) maxValue = val;

                    if (point.x() < minp.x()) minp.x() = point.x();
                    if (point.y() < minp.y()) minp.y() = point.y();
                    if (point.z() < minp.z()) minp.z() = point.z();

                    if (point.x() > maxp.x()) maxp.x() = point.x();
                    if (point.y() > maxp.y()) maxp.y() = point.y();
                    if (point.z() > maxp.z()) maxp.z() = point.z();
                }

                // Build visited matrix
                visited = Image<bool, true>::New(minp, maxp);
                ImageHelper::Fill(visited, false);

                // Loop through image iterator
                pcl_ForIterator(iter)
                {
                    // Variables named iter refer to the base point from walking through the iterator
                    // Variables named next refer to the points visited by stepping forwards and backwards from the base point
                    Point3D<int> iterPoint = iter.getPoint();
                    PointIndexObject nextPoint;
                    double iterValue, nextValue;
                    int iterBin, nextBin;
                    int runLength;

                    // If the current voxel has not already been visited and it falls within the image
                    if (!visited->get(iterPoint) && image->contain(iterPoint))
                    {
                        // Mark the voxel as having been visited
                        visited->set(iterPoint, true);

                        // Determine value/bin of current voxel
                        iterValue = image->get(iterPoint);
                        iterBin = getBinNumber(iterValue, minValue, maxValue, numGraylevels, useOldBinCounting);

                        // Count runlength starting at 1
                        runLength = 1;

                        // Step forwards
                        nextPoint.point = iter.getPoint() + offset.getPoint();
                        nextPoint.index = iter.getIndex() + offset.getIndex();
                        while (checkFunc(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            nextBin = getBinNumber(nextValue, minValue, maxValue, numGraylevels, useOldBinCounting);

                            if (nextBin != iterBin) break;                          // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point += offset.point;                        // Step next
                            nextPoint.index += offset.index;
                        }

                        // Step backwards
                        nextPoint.point = iter.getPoint() - offset.getPoint();
                        nextPoint.index = iter.getIndex() - offset.getIndex();
                        while (checkFunc(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            nextBin = getBinNumber(nextValue, minValue, maxValue, numGraylevels, useOldBinCounting);

                            if (nextBin != iterBin) break;                          // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point -= offset.point;                        // Step next
                            nextPoint.index -= offset.index;
                        }

                        // Add (bin number, runlength) to the running total
                        runLengthList.push_back(BinRunLengthStruct(iterBin, runLength));

                        // Keep track of maximum runlength encountered
                        if (runLength > maxRunLength) maxRunLength = runLength;
                    }
                }

                // Create an empty RLM based on number of bins and max runlength
                rlm.setSize(numGraylevels, maxRunLength);

                // Populate RLM
                for (auto i = runLengthList.begin(); i != runLengthList.end(); i++)
                    rlm.add(i->binNumber, i->runLength);

                return rlm;
			}
			
			template <class ImagePointerType, class IteratorType, class AcceptPoint, class AcceptNeighbor>
			static RunLengthMatrix ComputeFromIteratorWithDynamicQuantization(const ImagePointerType& image, int numGraylevels, IteratorType& iter, const PointIndexObject& offset, AcceptPoint& accp, AcceptNeighbor& accn)			
			{
                RunLengthMatrix rlm;
                std::vector<BinRunLengthStruct> runLengthList;
                int maxRunLength = std::numeric_limits<int>::min();

                Image<bool, true>::Pointer visited;
				PointIndexObject neighbor;
				double minValue =  std::numeric_limits<double>::infinity();
                double maxValue = -std::numeric_limits<double>::infinity();
                Point3D<int> minp, maxp;

                // Determine min and max value/point for the image and region
                minp = image->getMaxPoint();
                maxp = image->getMinPoint();
				pcl_ForIterator(iter) if (image->contain(iter.getPoint())) {
					double val = image->get(iter);
                    Point3D<int> point = iter.getPoint();

					if (minValue > val) minValue = val;
					if (maxValue < val) maxValue = val;

                    if (point.x() < minp.x()) minp.x() = point.x();
                    if (point.y() < minp.y()) minp.y() = point.y();
                    if (point.z() < minp.z()) minp.z() = point.z();

                    if (point.x() > maxp.x()) maxp.x() = point.x();
                    if (point.y() > maxp.y()) maxp.y() = point.y();
                    if (point.z() > maxp.z()) maxp.z() = point.z();
                }

                // Build visited matrix
                visited = Image<bool, true>::New(minp, maxp);
                ImageHelper::Fill(visited, false);

                // Loop through image iterator
                pcl_ForIterator(iter)
                {
                    // Variables named iter refer to the base point from walking through the iterator
                    // Variables named next refer to the points visited by stepping forwards and backwards from the base point
                    Point3D<int> iterPoint = iter.getPoint();
                    PointIndexObject nextPoint;
                    double iterValue, nextValue;
                    int iterBin, nextBin;
                    int runLength;

                    // If the current voxel has not already been visited and it falls within the image
                    if (!visited->get(iterPoint) && image->contain(iterPoint)&& accp(iter.getPoint(), iter))
                    {
                        // Mark the voxel as having been visited
                        visited->set(iterPoint, true);

                        // Determine value/bin of current voxel
                        iterValue = image->get(iterPoint);
                        iterBin = getBinNumber(iterValue, minValue, maxValue, numGraylevels, false);

                        // Count runlength starting at 1
                        runLength = 1;

                        // Step forwards
                        nextPoint.point = iter.getPoint() + offset.getPoint();
                        nextPoint.index = iter.getIndex() + offset.getIndex();
                        while (accn(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            nextBin = getBinNumber(nextValue, minValue, maxValue, numGraylevels, false);

                            if (nextBin != iterBin) break;                          // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point += offset.point;                        // Step next
                            nextPoint.index += offset.index;
                        }

                        // Step backwards
                        nextPoint.point = iter.getPoint() - offset.getPoint();
                        nextPoint.index = iter.getIndex() - offset.getIndex();
                        while (accn(nextPoint.point, nextPoint.index))         // Is it necessary to also check whether nextPoint falls within image bounds? Safety vs speed
                        {
                            nextValue = image->get(nextPoint.point);                // Determine value/bin of next point. Is it faster/better to pass nextPoint.index instead?
                            nextBin = getBinNumber(nextValue, minValue, maxValue, numGraylevels, false);

                            if (nextBin != iterBin) break;                          // If bin numbers no longer match, end the run

                            visited->set(nextPoint.point, true);                    // Visit voxel (is it faster/better to pass nextPoint.index instead?)
                            runLength++;                                            // Increment runlength
                            
                            nextPoint.point -= offset.point;                        // Step next
                            nextPoint.index -= offset.index;
                        }

                        // Add (bin number, runlength) to the running total
                        runLengthList.push_back(BinRunLengthStruct(iterBin, runLength));

                        // Keep track of maximum runlength encountered
                        if (runLength > maxRunLength) maxRunLength = runLength;
                    }
                }

                // Create an empty RLM based on number of bins and max runlength
                rlm.setSize(numGraylevels, maxRunLength);

                // Populate RLM
                for (auto i = runLengthList.begin(); i != runLengthList.end(); i++)
                    rlm.add(i->binNumber, i->runLength);
                return rlm;
			}

        protected:
            struct BinRunLengthStruct
            {
                int binNumber;
                int runLength;
                
                BinRunLengthStruct(int b, int r)
                {
                    binNumber = b;
                    runLength = r;
                }
                
                friend std::ostream &operator<<(std::ostream &out, const BinRunLengthStruct &obj)
                {
                    out << "(" << obj.binNumber << ", " << obj.runLength << ")";
                    return out;
                }
            };

            static inline unsigned int getBinNumber(double value, double minValue, double maxValue, int numBins, bool useOldBinCounting = false)
            {
                unsigned int binNumber;

                if (!useOldBinCounting)
                {
                    double stepSize = (maxValue - minValue + 1) / numBins;          // Fixed by Danny 2015-Jun-18
                    binNumber = static_cast<unsigned int>(std::floor((value - minValue) / stepSize));
                }
                else
                {
                    double stepSize = (maxValue - minValue) / numBins;          // Old (incorrect) version

                    binNumber = static_cast<unsigned int>(std::floor((value - minValue) / stepSize));                
                    if (binNumber < 0) binNumber = 0;
                    if (binNumber >= numBins) binNumber = numBins - 1;
                }

                return binNumber;
            }
		};

	}
}

#endif
