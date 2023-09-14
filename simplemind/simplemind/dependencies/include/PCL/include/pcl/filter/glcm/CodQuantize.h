#ifndef PCL_COD_QUANTIZE
#define PCL_COD_QUANTIZE

#include <pcl/cod/CodBase.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;

		template <class T, class RType>
		class CodQuantize: public CodBase<T::ReferableViaIndex>
		{
		//TODO!!!! TO RETHINK THE RETURN VALUE!!!
		public:
			typedef CodQuantize Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef RType ReturnType;

			static Pointer New(const typename T::Pointer& obj, typename T::ReturnType minval, typename T::ReturnType maxval, typename T::ReturnType binsize)
			{
				return Pointer(new Self(obj, minval, maxval, binsize));
			}
			
			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return getVal(m_Object->get(iter));
			}

			inline ReturnType get(const Point3D<int>& p) const
			{
				return getVal(m_Object->get(p));
			}
			
			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				return getVal(m_Object->get(p,i));
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_Object->getTemplateImage();
			}
			
		protected:
			typename T::Pointer m_Object;
			typename T::ReturnType m_Min, m_Max, m_Bin;
			ReturnType m_MaxReturnVal;

			CodQuantize(const typename T::Pointer& obj, ReturnType minval, ReturnType maxval, ReturnType binsize) 
			{
				m_Object = obj;
				if (obj->isUnbounded()) setRegionInfo();
				else setRegionInfo(obj);
				m_Min = minval;
				m_Max = maxval;
				m_Bin = binsize;
				m_MaxReturnVal = (m_Max-m_Min)/m_Bin;
			}

			inline ReturnType getVal(T::ReturnType val)
			{
				if (val<m_Min) return 0;
				else if (val>m_Max) return m_MaxReturnVal;
				return (val-m_Min)/m_Bin;
			}
		};

	}
}

#endif