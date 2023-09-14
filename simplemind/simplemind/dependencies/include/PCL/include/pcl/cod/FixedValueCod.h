#ifndef PCL_FIXED_VALUE_COD
#define PCL_FIXED_VALUE_COD

#include <pcl/cod/CodBase.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class ValueType>
		class FixedValueCod: public CodBase<true>
		{
		public:
			typedef FixedValueCod Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef ValueType ReturnType;

			static Pointer New(const ReturnType& val)
			{
				Pointer obj(new Self);
				obj->setRegionInfo();
				obj->setValue(val);
				return obj;
			}
			
			void setValue(const ReturnType& val)
			{
				m_Value = val;
			}

			template <class IteratorType>
            const ReturnType& get(const IteratorType& /*iter*/) const
			{
				return m_Value;
			}
			
            const ReturnType& get(const Point3D<int>& /*p*/) const
			{
				return m_Value;
			}

            const ReturnType& get(const Point3D<int>& /*p*/, long /*index*/) const
			{
				return m_Value;
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				return DummyImage::Pointer();
			}

		protected:
			ReturnType m_Value;
			FixedValueCod() {}
		};

	}
}

#endif
