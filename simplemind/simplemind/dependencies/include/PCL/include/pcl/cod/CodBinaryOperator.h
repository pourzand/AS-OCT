#ifndef PCL_COD_BINARY_OPERATOR
#define PCL_COD_BINARY_OPERATOR

#include <pcl/cod/CodBase.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		namespace cod_binary_operator_details
		{
			pcl_CreateMethodTester(canBeShortCircuited_exists, canBeShortCircuited);
		}

		template <class T1, class T2, class OperatorType>//, bool Enable=details::is_base_of_CodBase<T1>::value && details::is_base_of_CodBase<T2>::value>
		class CodBinaryOperator: public CodBase<T1::ReferableViaIndex && T2::ReferableViaIndex>
		{
		public:
			typedef CodBinaryOperator Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef decltype(boost::declval<OperatorType>().operator()(
				boost::declval<typename T1::ReturnType>(),
				boost::declval<typename T2::ReturnType>()
				)) ReturnType;

			static Pointer New(const typename T1::Pointer& obj1, const typename T2::Pointer& obj2)
			{
				if (!obj1->isCompatible(obj2)) pcl_ThrowException(IncompatibleCodException(), "Incompatible COD objects provided");
				Pointer result(new Self);
				result->m_Object1 = obj1;
				result->m_Object2 = obj2;
				if (obj1->isUnbounded() && obj2->isUnbounded()) result->setRegionInfo();
				else if (!obj1->isUnbounded()) result->setRegionInfo(obj1);
				else result->setRegionInfo(obj2);
				return result;
			}
			
			OperatorType& getOperator()
			{
				return m_Operator;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return getResult<OperatorType>(iter);
			}
			
			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				return getResult<OperatorType>(p,i);
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				DummyImage::ConstantPointer template_image;
				//Prioritized bounded object first
				if (!m_Object1->isUnbounded()) {
					template_image = m_Object1->getTemplateImage();
				} else if (!m_Object2->isUnbounded()) {
					template_image = m_Object2->getTemplateImage();
				}
				if (template_image) return template_image;
				//Prioritized object that are referable via index
				if (T1::ReferableViaIndex) {
					template_image = m_Object1->getTemplateImage();
				} else if (T2::ReferableViaIndex) {
					template_image = m_Object2->getTemplateImage();
				}
				if (template_image) return template_image;
				//Last resort
				template_image = m_Object1->getTemplateImage();
				if (template_image) return template_image;
				template_image = m_Object2->getTemplateImage();
				return template_image;
			}
			
		protected:
			typename T1::Pointer m_Object1;
			typename T2::Pointer m_Object2;
			OperatorType m_Operator;

			template <class T, class Iter>
			inline typename boost::enable_if<
				cod_binary_operator_details::canBeShortCircuited_exists<T,bool(T::*)(const typename T1::ReturnType&)const>,
				ReturnType
			>::type getResult(const Iter& iter) const
			{
				auto val = m_Object1->get(iter);
				if (m_Operator.canBeShortCircuited(val)) return val;
				else return m_Operator(val, m_Object2->get(iter)); 
			}
			template <class T, class Iter>
			inline typename boost::disable_if<
				cod_binary_operator_details::canBeShortCircuited_exists<T,bool(T::*)(const typename T1::ReturnType&)const>,
				ReturnType
			>::type getResult(const Iter& iter) const
			{
				return m_Operator(m_Object1->get(iter), m_Object2->get(iter)); 
			}

			template <class T>
			inline typename boost::enable_if<
				cod_binary_operator_details::canBeShortCircuited_exists<T,bool(T::*)(const typename T1::ReturnType&)const>,
				ReturnType
			>::type getResult(const Point3D<int>& p, long i) const
			{
				auto val = m_Object1->get(p,i);
				if (m_Operator.canBeShortCircuited(val)) return val;
				else return m_Operator(val, m_Object2->get(p,i));
			}
			template <class T>
			inline typename boost::disable_if<
				cod_binary_operator_details::canBeShortCircuited_exists<T,bool(T::*)(const typename T1::ReturnType&)const>,
				ReturnType
			>::type getResult(const Point3D<int>& p, long i) const
			{
				return m_Operator(m_Object1->get(p,i), m_Object2->get(p,i));
			}

			CodBinaryOperator() {}
		};
		

		namespace binary_operator
		{

			/******************************************* Arithmetic operators *******************************************/
			struct AdditionOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a+b)>::type
				{
					return a+b;
				}
			};

			struct SubtractionOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a-b)>::type
				{
					return a-b;
				}
			};

			struct MultiplicationOperator
			{
				template <class T>
				inline typename boost::enable_if<boost::is_convertible<T,int>, bool>::type canBeShortCircuited(const T& a) const
				{
					if (a==0) return true;
					return false;
				}

				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a*b)>::type
				{
					return a*b;
				}
			};

			struct DivisionOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a/b)>::type
				{
					return a/b;
				}
			};
			
			struct PowerOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a*b)>::type
				{
					typedef typename boost::remove_cv<decltype(a*b)>::type CastType;
					return std::pow(static_cast<CastType>(a), static_cast<CastType>(b));
				}
			};

			/******************************************* Comparison operators *******************************************/
			struct EqOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a==b)>::type
				{
					typedef decltype(a*b) IntermediateType;
					return static_cast<IntermediateType>(a)==static_cast<IntermediateType>(b);
				}
			};

			struct NeOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a!=b)>::type
				{
					typedef decltype(a*b) IntermediateType;
					return static_cast<IntermediateType>(a)!=static_cast<IntermediateType>(b);
				}
			};

			struct GeOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a>=b)>::type
				{
					return a>=b;
				}
			};

			struct GtOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a>b)>::type
				{
					return a>b;
				}
			};

			struct LeOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a<=b)>::type
				{
					return a<=b;
				}
			};

			struct LtOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a<b)>::type
				{
					return a<b;
				}
			};

			/******************************************* Logical operators *******************************************/
			struct AndOperator
			{
				template <class T>
				inline typename boost::enable_if<boost::is_convertible<T,bool>, bool>::type canBeShortCircuited(const T& a) const
				{
					if (a) return false;
					return true;
				}

				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a&&b)>::type
				{
					return a&&b;
				}
			};

			struct OrOperator
			{
				template <class T>
				inline typename boost::enable_if<boost::is_convertible<T,bool>, bool>::type canBeShortCircuited(const T& a) const
				{
					if (a) return true;
					return false;
				}

				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a||b)>::type
				{
					return a||b;
				}
			};
			
			/******************************************* Min and max operators *******************************************/
			struct MinOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a+b)>::type
				{
					return a<b?a:b;
				}
			};
			
			struct MaxOperator
			{
				template <class T1, class T2>
				inline auto operator()(const T1& a, const T2& b) const -> typename boost::remove_cv<decltype(a+b)>::type
				{
					return a>b?a:b;
				}
			};

		}
	}
}

#endif