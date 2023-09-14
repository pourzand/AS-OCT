#ifndef PCL_HESSIAN_EIGEN_VECTOR_COD_CONVERTER
#define PCL_HESSIAN_EIGEN_VECTOR_COD_CONVERTER

#include <pcl/cod/CodBase.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;
		
		namespace binary_operator
		{
		
			struct DotProductOperator
			{
				template <class PT1, class PT2>
				double operator()(const PT1& p1, const PT2& p2) const
				{
					return p1.getDotProduct(p2);
				}
			};
		
		}

		template <class FType, bool Flag=false>
		class HessianEigenVectorCodConverter: public CodBase<Flag>
		{
		public:
			typedef HessianEigenVectorCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef FType FilterType;
			typedef Point3D<double> ReturnType;

			static Pointer New(const typename FilterType::Pointer& filter, int result_index)
			{
				if (!filter) {
					pcl_ThrowException(IncompatibleCodException(), "Empty filter object provided");
				}
				Pointer obj(new Self);
				obj->m_Filter = filter;
				obj->m_ResultIndex = result_index;
				obj->setRegionInfo();
				return obj;
			}

			template<class ImagePointerType>
			static Pointer New(const typename FilterType::Pointer& filter, const ImagePointerType& image, int result_index)
			{
				if (!filter) {
#ifndef NO_WARNING
					std::cout << "Warning at PointFilterToCodConverter::New(): Empty filter object provided, a NULL pointer is returned!" << std::endl;
#endif
					return Pointer();
				}
				Pointer obj(new Self);
				obj->m_Filter = filter;
				obj->m_ResultIndex = result_index;
				if (image) obj->setRegionInfo(image);
				return obj;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return get(iter.getPoint(), iter);
			}
			
			inline ReturnType get(const Point3D<int>& p) const
			{
				m_Filter->apply(p);
				return m_Filter->getEigenVector(m_ResultIndex);
			}

			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				m_Filter->apply(p, i);
				return m_Filter->getEigenVector(m_ResultIndex);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_TemplateImage;
			}
			
		protected:
			int m_ResultIndex;
			typename FilterType::Pointer m_Filter;
			DummyImage::ConstantPointer m_TemplateImage;

			HessianEigenVectorCodConverter() {}

			using CodBase<Flag>::setRegionInfo;

			template <class ImagePointerType>
			void setRegionInfo(const ImagePointerType& image) 
			{
				setRegionInfo(image->toIndex(image->getMinPoint()), image->getRegion());
				if (boost::is_same<typename pcl::ptr_base_type<ImagePointerType>::type, DummyImage>::value) m_TemplateImage = image;
				else m_TemplateImage = DummyImage::New(image);
			}
		};

	}
}

#endif