#ifndef PCL_LIMITLESS_INDEXLESS_COD_CONVERTER
#define PCL_LIMITLESS_INDEXLESS_COD_CONVERTER

#include <pcl/cod/CodBase.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class T>
		class LimitlessIndexlessCodConverter: public CodBase<false>
		{
		public:
			typedef LimitlessIndexlessCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef typename T::ReturnType ReturnType;

			static Pointer New(const typename T::Pointer& obj)
			{
				if (!obj->isUnbounded()) {
					pcl_ThrowException(IncompatibleCodException(), "Cannot convert from bounded COD object");
				}
				Pointer result(new Self);
				result->m_Object = obj;
				result->setRegionInfo();
				return result;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return m_Object->get(iter.getPoint());
			}

			inline ReturnType get(const Point3D<int>& p) const
			{
				return m_Object->get(p);
			}
			
			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				return m_Object->get(p);
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_Object->getTemplateImage();
			}

		protected:
			typename T::Pointer m_Object;

			LimitlessIndexlessCodConverter() {}
		};

	}
}

#endif