#ifndef PCL_FEATURE_COD_WRAPPER
#define PCL_FEATURE_COD_WRAPPER
#include <pcl/cod.h>
#include <pcl/machine_learning/feature/FeatureBase.h>
#include <pcl/exception.h>

namespace pcl
{
	namespace feature
	{

		template <class T, class CType>
		class CodWrapper: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef typename CType::Pointer CodPointer;

			static Pointer New(const CodPointer& cod_ptr, const std::string& name)
			{
				return Pointer(new CodWrapper<T,CType>(cod_ptr, name));
			}

			virtual void forceNextCompute()
			{
				m_PrevIndex = -1;
			}

			virtual void compute(const Point3D<int>& p, long i)
			{
				if (m_PrevIndex==i) return;
				m_PrevIndex = i;
				m_Value = m_Cod->get(p,i);
			}

			virtual T getResult(size_t i=0) const
			{
				return m_Value;
			}

			virtual void populateResult(std::vector<T>& result) const
			{
				result.push_back(m_Value);
			}

			virtual void populateResult(std::vector<T>& result, size_t offset) const
			{
				result[offset] = m_Value;
			}

			virtual std::string getFeatureName(size_t i=0) const
			{
				return m_Name;
			}

		protected:
			CodPointer m_Cod;
			std::string m_Name;
			ValueType m_Value;
			long m_PrevIndex;

			CodWrapper(const CodPointer& cod_ptr, const std::string& name)
			{
				m_Cod = cod_ptr;
				m_Name = name;
				this->m_Size = 1;
				m_PrevIndex = -1;
			}
		};


		class CodWrapperHelper
		{
		public:
			template <class T, class CType>
			static typename CodWrapper<T,CType>::Pointer New(const CodEncapsulator<CType>& cod, const std::string& name="")
			{
				return CodWrapper<T,CType>::New(cod.get(), name);
			}
		};

	}
}

#endif