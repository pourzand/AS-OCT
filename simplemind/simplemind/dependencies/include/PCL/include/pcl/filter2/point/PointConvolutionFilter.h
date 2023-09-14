#ifndef PCL_POINT_CONVOLUTION_FILTER
#define PCL_POINT_CONVOLUTION_FILTER

#include <pcl/image.h>
#include <pcl/iterator.h>
#include <pcl/filter2/point/PointFilterBase.h>

namespace pcl
{
	namespace filter2
	{
		using namespace pcl::iterator;

		template <class BoundaryType, class KernelImageType, class InternalValueType=typename KernelImageType::IoValueType >
		class PointConvolutionFilter: public PointFilterBase
		{
		public:
			typedef PointConvolutionFilter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef BoundaryType BoundaryHandlerType;

			static Pointer New(const BoundaryHandlerType& bound, const typename KernelImageType::ConstantPointer& kernel)
			{
				Pointer obj(new Self);
				obj->setInput(bound);
				obj->setKernel(kernel);
				obj->update();
				return obj;
			}
			
			BoundaryHandlerType& getBoundaryHandler()
			{
				return m_BoundaryHandler;
			}

			template <class IteratorType>
			void apply(const IteratorType& iter)
			{
				apply(iter.getPoint(), iter.getIndex());
			}

			void apply(const Point3D<int>& point) 
			{
				apply(point, m_BoundaryHandler.getImage()->localToIndex(point));
			}
			void apply(const Point3D<int>& point, long index) 
			{
				if (isPreviousIndex(index)) return;
				m_Result = 0;
				if (m_SafeRegion.contain(point)) { //Safe version (check not needed)
					m_SafeInputIterator.setWindowOrigin(index);
					for (m_SafeInputIterator.begin(), m_KernelIterator.begin(); !m_KernelIterator.end(); m_SafeInputIterator.next(), m_KernelIterator.next()) {
						m_Result += m_BoundaryHandler.getImage()->get(m_SafeInputIterator) * m_Kernel->get(m_KernelIterator);
					}
				} else {
					m_InputIterator.setWindowOrigin(point,index);
					for (m_InputIterator.begin(), m_KernelIterator.begin(); !m_KernelIterator.end(); m_InputIterator.next(), m_KernelIterator.next()) {
						m_Result += m_BoundaryHandler.get(m_InputIterator.getPoint(),m_InputIterator) * m_Kernel->get(m_KernelIterator);
					}
				}
			}

			void apply(long index) //Implicitly safe
			{
				if (isPreviousIndex(index)) return;
				m_Result = 0;
				m_SafeInputIterator.setWindowOrigin(index);
				for (m_SafeInputIterator.begin(), m_KernelIterator.begin(); !m_KernelIterator.end(); m_SafeInputIterator.next(), m_KernelIterator.next()) {
					m_Result += m_BoundaryHandler.getImage()->get(m_SafeInputIterator) * m_Kernel->get(m_KernelIterator);
				}
			}

			inline double getResult(int i=0)
			{
				return m_Result;
			}

		protected:
			typename KernelImageType::ConstantPointer m_Kernel;
			BoundaryHandlerType m_BoundaryHandler;

			ImageIterator m_KernelIterator;
			ImageWindowIterator m_SafeInputIterator;
			ImageWindowIteratorWithPoint m_InputIterator;
			Region3D<int> m_SafeRegion;

			InternalValueType m_Result;

			PointConvolutionFilter() {}

			void setInput(const typename BoundaryHandlerType& bound)
			{
				m_BoundaryHandler = bound;
			}

			void setKernel(const typename KernelImageType::ConstantPointer& kernel)
			{
				m_Kernel = kernel;
			}

			void update()
			{
				m_KernelIterator.setImage(m_Kernel, ImageWindowIterator::RX, ImageWindowIterator::RY, ImageWindowIterator::RZ);
				m_InputIterator.setImage(m_BoundaryHandler->getImage(), m_Kernel->getRegion());

				bool is_kernel_smaller = true;
				for (int i=0; i<3; i++) {
					if (m_Kernel->getSize()[i] > m_BoundaryHandler.getImage()->getSize()[i]) {
						is_kernel_smaller = false;
						break;
					}
				}
				if (is_kernel_smaller) {
					m_SafeRegion.set(
						m_Input->getMinPoint() - m_Kernel->getMinPoint(),
						m_Input->getMaxPoint() - m_Kernel->getMaxPoint()
						);
					m_SafeInputIterator.setImage(m_BoundaryHandler.getImage(), m_Kernel->getRegion());
				} else m_SafeRegion.reset();
			}
		};

	}
}

#endif