#ifndef PCL_IMAGE_RESAMPLE_HELPER
#define PCL_IMAGE_RESAMPLE_HELPER

#include <pcl/macro.h>
#include <pcl/type_utility.h>
#include <pcl/exception.h>
#include <math.h>

namespace pcl
{
	namespace filter
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

			template <class OutputImageType, class InterpolatorType, class InputImagePointer, class TransformType>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const InterpolatorType& interp, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical)
			{
				return Resample<OutputImageType>(input, interp, output_index_to_input_mapper, mapper_is_to_physical, DefaultValueConverter());
			}
			template <class OutputImageType, class InterpolatorType, class InputImagePointer, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const InterpolatorType& interp, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, ValueConverter value_converter)
			{
				std::list<Point3D<double>> corner_points;
				if (mapper_is_to_physical) {
					Region3D<double> &physical_region = input->getPhysicalRegion();
					physical_region.getCornerPoints(corner_points);
					auto physical_to_output_index_mapper = output_index_to_input_mapper->getInverse();
					pcl_ForEach(corner_points, item) {
						*item = physical_to_output_index_mapper->template toTransformed<Point3D<double>>(*item);
					}
				} else {
					input->getRegion().getCornerPoints(corner_points);
					auto input_to_output_index_mapper = output_index_to_input_mapper->getInverse();
					pcl_ForEach(corner_points, item) {
						*item = input_to_output_index_mapper->template toTransformed<Point3D<double>>(*item);
					}
				}
				Region3D<int> output_region;
				output_region.reset();
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
				return Resample<OutputImageType>(input, interp, output_index_to_input_mapper, mapper_is_to_physical, output_region, value_converter);
			}

			template <class OutputImageType, class InterpolatorType, class InputImagePointer, class TransformType>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const InterpolatorType& interp, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region)
			{
				return Resample<OutputImageType>(input, interp, output_index_to_input_mapper, mapper_is_to_physical, output_region, DefaultValueConverter());
			}
			template <class OutputImageType, class InterpolatorType, class InputImagePointer, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const InterpolatorType& interpolator, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region, ValueConverter value_converter)
			{
				//Compute origin and spacing
				Point3D<double> origin = output_index_to_input_mapper->template toTransformed< Point3D<double> >(Point3D<int>(0,0,0));
				if (!mapper_is_to_physical) origin = input->toPhysicalCoordinate(origin);

                // Determine orientation matrix by converting a unit vector along each of the axes (in image space) to physical space
				Point3D<double> spacing;
				typename OutputImageType::OrientationMatrixType orientation_matrix;
				for (int i=0; i<3; i++) {
					Point3D<int> p(0,0,0);
					p[i] = 1;
					Point3D<double>& displacement = output_index_to_input_mapper->template toTransformed< Point3D<double> >(p);
					if (!mapper_is_to_physical) displacement = input->toPhysicalCoordinate(displacement);
					displacement -= origin;
					spacing[i] = displacement.getNorm();
					displacement /= spacing[i];
					for (int r=0; r<3; r++) orientation_matrix(r,i) = displacement[r];
				}

				//Creating the image
				typename OutputImageType::Pointer result;
				if (OutputImageType::UseOrientationMatrix) result = OutputImageType::New(output_region.getMinPoint(), output_region.getMaxPoint(), spacing, origin, orientation_matrix);
				else result = OutputImageType::New(output_region.getMinPoint(), output_region.getMaxPoint(), spacing, origin);
				if (!mapper_is_to_physical) {
					ImageIteratorWithPoint iter(result);
					pcl_ForIterator(iter) {
						result->set(iter, value_converter(interpolator.get(output_index_to_input_mapper->template toTransformed< Point3D<double> >(iter.getPoint()))));
					}
				} else {
					populateImage(result, input, interpolator, output_index_to_input_mapper, value_converter);
				}
				return result;
			}


			template <class OutputImageType, template<class> class Interpolator, class InputImagePointer, class TransformType>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical)
			{
				Interpolator<typename ptr_base_type<InputImagePointer>::type> interpolator(input);
				return Resample<OutputImageType>(input, interpolator, output_index_to_input_mapper, mapper_is_to_physical);
			}
			template <class OutputImageType, template<class> class Interpolator, class InputImagePointer, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, ValueConverter value_converter)
			{
				Interpolator<typename ptr_base_type<InputImagePointer>::type> interpolator(input);
				return Resample<OutputImageType>(input, interpolator, output_index_to_input_mapper, mapper_is_to_physical, value_converter);
			}

			template <class OutputImageType, template<class> class Interpolator, class InputImagePointer, class TransformType>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region)
			{
				Interpolator<typename ptr_base_type<InputImagePointer>::type> interpolator(input);
				return Resample<OutputImageType>(input, interpolator, output_index_to_input_mapper, mapper_is_to_physical, output_region);
			}
			template <class OutputImageType, template<class> class Interpolator, class InputImagePointer, class TransformType, class ValueConverter>
			static typename OutputImageType::Pointer Resample(const InputImagePointer& input, const TransformType& output_index_to_input_mapper, bool mapper_is_to_physical, const Region3D<int>& output_region, ValueConverter value_converter)
			{
				Interpolator<typename ptr_base_type<InputImagePointer>::type> interpolator(input);
				return Resample<OutputImageType>(input, interpolator, output_index_to_input_mapper, mapper_is_to_physical, output_region, value_converter);
			}

		protected:
			template <class T, class IM>
			class addPhysicalToImageTransformation_exists
			{
				typedef char yes;
				typedef short no;

				template <class C> static yes test(decltype(boost::declval<C>()->addPhysicalToImageTransformation(IM()))*);
				template <class C> static no test(...);
			public:
				enum { value = sizeof(test<T>(0)) == sizeof(yes) };
			};

			template <class ResultImagePointerType, class InputImagePointerType, class InterpolatorType, class TransformType, class ValueConverter>
			static typename boost::enable_if<
				addPhysicalToImageTransformation_exists<TransformType, InputImagePointerType>, void
			>::type populateImage(ResultImagePointerType& result, InputImagePointerType& input, InterpolatorType& interpolator, TransformType& transform, ValueConverter& value_converter)
			{
				auto final_mapper = transform->getCopy();
				final_mapper->addPhysicalToImageTransformation(input);
				ImageIteratorWithPoint iter(result);
				pcl_ForIterator(iter) {
					result->set(iter, value_converter(interpolator.get(final_mapper->template toTransformed< Point3D<double> >(iter.getPoint()))));
				}
			}

			template <class ResultImagePointerType, class InputImagePointerType, class InterpolatorType, class TransformType, class ValueConverter>
			static typename boost::disable_if<
				addPhysicalToImageTransformation_exists<TransformType, InputImagePointerType>, void
			>::type populateImage(ResultImagePointerType& result, InputImagePointerType& input, InterpolatorType& interpolator, TransformType& transform, ValueConverter& value_converter)
			{
				ImageIteratorWithPoint iter(result);
				pcl_ForIterator(iter) {
					result->set(iter, value_converter(interpolator.get(
						input->toImageCoordinate(
							transform->template toTransformed< Point3D<double> >(iter.getPoint())
						)
					)));
				}
			}
		};

	}
}

#endif