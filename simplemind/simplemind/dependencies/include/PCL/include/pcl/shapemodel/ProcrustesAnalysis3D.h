#ifndef PCL_PROCRUSTES_ANALYSIS_3D
#define PCL_PROCRUSTES_ANALYSIS_3D

#include <pcl/shapemodel/PointCloud.h>
#include <pcl/shapemodel/AffineAlignment3D.h>
#include <vector>

namespace pcl
{
	namespace shapemodel
	{

		class ProcrustesAnalysis3D
		{
		public:
			template <class T>
			static PointCloud<T> GetMeanShape(const std::vector<PointCloud<T>>& shape_list, double significant_change=0.000001)
			{
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");
				double significant_change_sqr = significant_change*significant_change;
				std::vector<Point3D<double>> shape_centroid_list(shape_list.size());
				for (int i=0; i<shape_list.size(); ++i) shape_centroid_list[i] = AffineAlignment3D::GetCentroid(shape_list[i]);

				PointCloud<T> mean_shape = AffineAlignment3D::GetStandardizedShape(
					shape_list[0], 
					shape_centroid_list[0], 
					AffineAlignment3D::GetStandardDeviation(shape_list[0], shape_centroid_list[0])
					);

				int count = 0;
				bool done = false;
				while (!done) {
					PointCloud<T> sum;
					sum.resize(mean_shape.size());
					pcl_ForEach(sum, item) *item = 0;
					for (int i=0; i<shape_list.size(); ++i) {
						PointCloud<T> cur_shape = AffineAlignment3D::ApplyTransformation(shape_list[i], 
							AffineAlignment3D::GetAlignment(mean_shape, Point3D<double>(0,0,0), shape_list[i], shape_centroid_list[i])
							);
						for (int j=0; j<sum.size(); ++j) {
							sum[j] += cur_shape[j];
						}
					}
					pcl_ForEach(sum, item) *item /= shape_list.size();
					sum = AffineAlignment3D::ApplyTransformation(sum,
						AffineAlignment3D::GetAlignment(shape_list[0], shape_centroid_list[0], sum, AffineAlignment3D::GetCentroid(sum))
						);
					auto sum_centroid = AffineAlignment3D::GetCentroid(sum);
					sum = AffineAlignment3D::GetStandardizedShape(
						sum,
						sum_centroid,
						AffineAlignment3D::GetStandardDeviation(sum, sum_centroid)
					);

					done = true;
					double max_diff = -1;;
					for (int i=0; i<sum.pointSize(); ++i) {
						double diff = sum.getPoint(i).getEuclideanDistanceSqr(mean_shape.getPoint(i));
						if (diff >= significant_change_sqr) done = false;
						if (max_diff<diff) max_diff = diff;
					}
					Region3D<double> region; region.reset();
					for (int i=0; i<sum.pointSize(); ++i) region.add(sum.getPoint(i));
					//std::cout << region.getMaxPoint()-region.getMinPoint() << " | " << max_diff << "(" << significant_change_sqr << ")" << std::endl;
					mean_shape = sum;
					++count;
				}
				return std::move(mean_shape);
			}
		};

	}
}

#endif