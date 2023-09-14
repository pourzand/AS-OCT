#ifndef PCL_INTERPOLATING_POINT_CONVOLUTION_FILTER
#define PCL_INTERPOLATING_POINT_CONVOLUTION_FILTER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter/point/PointFilterBase.h>
#include <pcl/filter/interpolator/TrilinearInterpolator.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl::iterator;

		template <class InputImageType, class KernelType, class InterpolatorType=TrilinearInterpolator<InputImageType>, class InternalValueType=typename KernelType::ValueType >
		class InterpolatingPointConvolutionFilter: public PointFilterBase
		{
		public:
			typedef InterpolatingPointConvolutionFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef InterpolatorType::BoundaryHandlerType BoundaryHandlerType;

			static Pointer New(const typename InputImageType::ConstantPointer& image, const KernelType& kernel)
			{
				Pointer obj(new Self);
				m_Input = image;
				m_Kernel = kernel;
				m_Interpolator.setImage(m_Input);
				return obj;
			}
			
			BoundaryHandlerType& getBoundaryHandler()
			{
				return m_Interpolator.getBoundarHandler();
			}

			template <class IteratorType>
			void apply(const IteratorType& iter)
			{
				apply(iter.getPoint(), iter.getIndex());
			}

			void apply(const Point3D<int>& point) 
			{
				apply(point, m_Input->localToIndex(point));
			}
			void apply(const Point3D<int>& point, long index) 
			{
				if (isPreviousIndex(index)) return;
				m_Result = 0;
				//TODO!!!





				if (m_SafeRegion.contain(point)) { //Safe version (check not needed)
					m_SafeInputIterator.setWindowOrigin(index);
					for (m_SafeInputIterator.begin(), m_KernelIterator.begin(); !m_KernelIterator.end(); m_SafeInputIterator.next(), m_KernelIterator.next()) {
						m_Result += m_Input->get(m_SafeInputIterator) * m_Kernel->get(m_KernelIterator);
					}
				} else {
					m_InputIterator.setWindowOrigin(point,index);
					for (m_InputIterator.begin(), m_KernelIterator.begin(); !m_KernelIterator.end(); m_InputIterator.next(), m_KernelIterator.next()) {
						m_Result += m_BoundaryHandler.get(m_InputIterator.getPoint(),m_InputIterator) * m_Kernel->get(m_KernelIterator);
					}
				}
			}

			inline double getResult(int i=0)
			{
				return m_Result;
			}

		protected:
			typename InputImageType::ConstantPointer m_Input;
			KernelType m_Kernel;
			InterpolatorType m_Interpolator;

			InternalValueType m_Result;

			InterpolatingPointConvolutionFilter() {}
		};

	}
}

#endif