#ifndef PCL_OCTAVE_SUBSAMPLER
#define PCL_OCTAVE_SUBSAMPLER

#include <pcl/image.h>
#include <pcl/iterator.h>

namespace pcl
{
	namespace filter
	{

		using namespace pcl;
		using namespace pcl::iterator;

		template <class InputImage, class OutputImage>
		class OctaveSubsampler
		{
		public:
			static typename OutputImage::Pointer Compute(const typename InputImage::ConstantPointer& input)
			{
				OctaveSubsampler sampler(input);
				sampler.update();
				return sampler.getOutput();
			}


			OctaveSubsampler() {}
			OctaveSubsampler(const typename InputImage::ConstantPointer& input) 
			{
				setInput(input);
			}

			void setInput(const typename InputImage::ConstantPointer& input)
			{
				m_Input = input;
			}

			void update()
			{
				Point3D<int> new_size(
					ceil(m_Input->getSize().x()/2.0),
					ceil(m_Input->getSize().y()/2.0),
					ceil(m_Input->getSize().z()/2.0)
					);
				Point3D<double> new_spacing(m_Input->getSpacing());
				Region3D<int> window(Point3D<int>(0,0,0), Point3D<int>(1,1,1));
				for (int i=0; i<3; i++) {
					if (m_Input->getSize()[i]==1) window.getMaxPoint()[i] = 0;
				}
				new_spacing *= 2;
				m_Output = OutputImage::New(m_Input->getMinPoint(), m_Input->getMinPoint()+new_size-1, new_spacing, m_Input->getOrigin());
				m_BoundaryHandler.setImage(m_Input);
				Region3D<int> safe_region(m_Output->getMinPoint(), m_Output->getMaxPoint()-Point3D<int>(
					m_Input->getSize().x()%2,
					m_Input->getSize().y()%2,
					m_Input->getSize().z()%2
					));

				ImageIteratorWithPoint output_iter(m_Output);
				ImageWindowIteratorWithPoint input_iter(m_Input, window);
				Point3D<int> input_coord;
				pcl_ForIterator(output_iter) {
					input_coord = output_iter.getPoint()-m_Output->getMinPoint();
					input_coord *= 2;
					input_coord += m_Input->getMinPoint();
					input_iter.setWindowOrigin(input_coord, m_Input->localToIndex(input_coord));
					double val = 0;
					if (safe_region.contain(output_iter.getPoint())) {
						pcl_ForIterator(input_iter) {
							val += m_Input->get(input_iter);
						}
					} else {
						pcl_ForIterator(input_iter) {
							val += m_BoundaryHandler.get(input_iter.getPoint(), input_iter);
						}
					}
					val /= 8;
					m_Output->set(output_iter, val);
				}
			}

			typename OutputImage::Pointer getOutput()
			{
				return m_Output;
			}

		protected:
			typename InputImage::ConstantPointer m_Input;
			typename OutputImage::Pointer m_Output;
			RepeatingBoundaryHandler<InputImage> m_BoundaryHandler;
		};

	}
}

#endif