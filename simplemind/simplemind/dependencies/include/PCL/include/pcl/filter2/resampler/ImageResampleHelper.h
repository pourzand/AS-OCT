#ifndef PCL2_IMAGE_RESAMPLE_HELPER
#define PCL2_IMAGE_RESAMPLE_HELPER

#include <pcl/macro.h>
#include <pcl/type_utility.h>
#include <pcl/exception.h>
#include <math.h>

namespace pcl
{
	namespace filter2
	{

		class ImageResampleHelper
		{
			struct DefaultValueConverter
			{
				inline double operator()(const double& val) const
				{
					return val;
				}
			};
		public:
			/*********************************** Physical space resampling methods ***********************************/
			/**
			If the transformation contains rotation or negative scaling, and UseOrientationMatrix is false, 
			the physical space that the resulting output image resides in will be different than what is defined by the transformation.
			Only the spacing of the output image is guaranteed to be correct.
			**/

			template <class OutputImageType, class InterpolatorType, class TransformType>
			static typename OutputImageType::Pointer Resample(const InterpolatorType& interpolator, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region=Region3D<int>().reset())
			{
				return Resample<OutputImageType>(interpolator, output_index_to_input_mapper, mapper_is_to_physical, output_region, DefaultValueConverter());
			}

			template <class OutputImageType, class InterpolatorType, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer Resample(const InterpolatorType& interpolator, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region, ValueConverter value_converter)
			{
				if (mapper_is_to_physical) {
					auto final_mapper = output_index_to_input_mapper->getCopy();
					final_mapper->addPhysicalToImageTransformation(interpolator.getImage());
					return ActualResample<OutputImageType>(interpolator, final_mapper, output_region, value_converter);
				} else {
					return ActualResample<OutputImageType>(interpolator, output_index_to_input_mapper, output_region, value_converter);
				}
			}
			
			template <class OutputImageType, class InterpolatorType, class TemplatePointerType>
			static typename OutputImageType::Pointer ResampleTo(const InterpolatorType& interpolator, const TemplatePointerType& template_image)
			{
				return ResampleTo<OutputImageType>(interpolator, template_image, DefaultValueConverter());
			}
			
			template <class OutputImageType, class InterpolatorType, class TemplatePointerType, class ValueConverter>
			static typename OutputImageType::Pointer ResampleTo(const InterpolatorType& interpolator, const TemplatePointerType& template_image, ValueConverter value_converter)
			{
				auto transform = pcl::ImageHelper::GetImageCoordinateTransformation(template_image, interpolator.getImage());
				return Resample<OutputImageType>(interpolator, transform, false, template_image->getRegion(), value_converter);
			}

		protected:
			template <class OutputImageType, class InterpolatorType, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer ActualResample(const InterpolatorType& interpolator, const TransformType& output_index_to_input_mapper, Region3D<int> output_region, ValueConverter value_converter)
			{
				if (output_region.empty()) { //Determining output region automatically
					std::list<Point3D<double>> corner_points;
					interpolator.getImage()->getRegion().getCornerPoints(corner_points);
					auto input_to_output_index_mapper = output_index_to_input_mapper->getInverse();
					pcl_ForEach(corner_points, item) {
						*item = input_to_output_index_mapper->template toTransformed<Point3D<double>>(*item);
					}

					pcl_ForEach(corner_points, item) {
						Point3D<int> temp;
						temp.assignCustom(*item, [](double val) {
							return floor(val);
						});
						output_region.add(temp);
						temp.assignCustom(*item, [](double val) {
							return ceil(val);
						});
						output_region.add(temp);
					}
				}

				//Compute origin and spacing
				Point3D<double> origin = output_index_to_input_mapper->template toTransformed< Point3D<double> >(Point3D<int>(0,0,0));
					origin = interpolator.getImage()->toPhysicalCoordinate(origin);
				Point3D<double> spacing;
				typename OutputImageType::OrientationMatrixType orientation_matrix;
				for (int i=0; i<3; i++) {
					Point3D<int> p(0,0,0);
					p[i] = 1;
					// TODO PENDING TESTING.
					Point3D<double> displacement = output_index_to_input_mapper->template toTransformed< Point3D<double> >(p);
						displacement = interpolator.getImage()->toPhysicalCoordinate(displacement);
					displacement -= origin;
					spacing[i] = displacement.getNorm();
					displacement /= spacing[i];
					for (int r=0; r<3; r++) orientation_matrix(r,i) = displacement[r];
				}
				//Creating the image
				typename OutputImageType::Pointer result;
				if (OutputImageType::UseOrientationMatrix) result = OutputImageType::New(output_region.getMinPoint(), output_region.getMaxPoint(), spacing, origin, orientation_matrix);
				else result = OutputImageType::New(output_region.getMinPoint(), output_region.getMaxPoint(), spacing, origin);
				//Actual resampling
				ImageIteratorWithPoint iter(result);
				pcl_ForIterator(iter) {
					result->set(
						iter, 
						value_converter(
							interpolator.get(
								output_index_to_input_mapper->template toTransformed< pcl::Point3D<double> >(iter.getPoint())
							)
						)
					);
				}
				return result;
			}
		};

	}
}

#endif