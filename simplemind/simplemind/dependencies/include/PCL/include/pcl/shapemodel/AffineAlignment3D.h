#ifndef PCL_AFFINE_ALIGNMENT_3D
#define PCL_AFFINE_ALIGNMENT_3D

#include <pcl/geometry.h>
#include <pcl/exception.h>
#include <pcl/shapemodel/PointCloud.h>

namespace pcl
{
	namespace shapemodel
	{

		class AffineAlignment3D
		{
		public:
			template <class T>
			static Point3D<double> GetCentroid(const PointCloud<T>& shape)
			{
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");
				Point3D<double> centroid(0,0,0);
				for (int i=0; i<shape.pointSize(); ++i) centroid += shape.getPoint(i);
				centroid /= shape.pointSize();
				return centroid;
			}

			template <class T>
			static Point3D<double> GetStandardDeviation(const PointCloud<T>& shape, const Point3D<double>& centroid)
			{
				Point3D<double> variance(0,0,0);
				for (int i=0; i<shape.pointSize(); ++i) {
					Point3D<double> diff = shape.getPoint(i)-centroid;
					variance += (diff*diff);
				}
				variance /= shape.pointSize();
				return Point3D<double>(std::sqrt(variance.x()), std::sqrt(variance.y()), std::sqrt(variance.z()));
			}

			template <class T>
			static PointCloud<T> GetStandardizedShape(const PointCloud<T>& shape, const Point3D<double>& centroid, const Point3D<double>& stddev)
			{
				PointCloud<T> result;
				result.resize(shape.size());
				for (int i=0; i<result.pointSize(); ++i) {
					result.setPoint(i, (shape.getPoint(i)-centroid)/stddev);
				}
				return std::move(result);
			}

			template <class T>
			static pcl::geometry::AffineTransformation::Pointer GetAlignment(const PointCloud<T>& fixed, const PointCloud<T>& moving)
			{
				return GetAlignment(fixed, GetCentroid(fixed), moving, GetCentroid(moving));
			}

			/*
             For construction of affine matrix terms
             see "Statistical Models of Appearance for Computer Vision" Cootes, Taylor 2004
             appendix B
             http://www.face-rec.org/algorithms/AAM/app_models.pdf
             */
			template <class T>
			static pcl::geometry::AffineTransformation::Pointer GetAlignment(const PointCloud<T>& fixed, const Point3D<double>& fixed_centroid, const PointCloud<T>& moving, const Point3D<double>& moving_centroid)
			{
				if (fixed.pointSize()!=moving.pointSize()) pcl_ThrowException(pcl::Exception(), "Inconsistent shapes provided!");
				if (PointCloud<T>::Dimension!=3) pcl_ThrowException(pcl::Exception(), "Point cloud dimension not valid!");

				pcl::geometry::AffineTransformation::Pointer result = pcl::geometry::AffineTransformation::New();
				result->addTranslation(-moving_centroid);

				vnl_matrix_fixed<double,3,3> left_term, right_term;
				for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) {
					left_term(r,c) = 0;
					right_term(r,c) = 0;
				}
				for (int i=0; i<fixed.pointSize(); ++i) {
					auto fixed_point = fixed.getPoint(i)-fixed_centroid,
						moving_point = moving.getPoint(i)-moving_centroid;
					for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) {
						left_term(r,c) += moving_point[r]*moving_point[c];
						right_term(r,c) += fixed_point[r]*moving_point[c];
					}
				}
				if (left_term(2,2)==0) {
					left_term(2,2) = 1;
					right_term(2,2) = 1;
				}
				result->addTransformation(right_term*vnl_inverse(left_term));
				
				result->addTranslation(fixed_centroid);
				return result;
			}

			template <class T>
			static PointCloud<T> ApplyTransformation(const PointCloud<T>& shape, pcl::geometry::AffineTransformation::ConstantPointer transformation)
			{
				PointCloud<T> result;
				result.resizePoint(shape.pointSize());
				for (int i=0; i<shape.pointSize(); ++i) {
					result.setPoint(i, transformation->toTransformed<PointCloud<T>::PointType>(shape.getPoint(i)));
				}
				return std::move(result);
			}
		};

	}
}

#endif