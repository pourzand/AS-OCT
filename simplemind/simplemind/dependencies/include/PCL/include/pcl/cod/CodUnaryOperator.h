#ifndef PCL_COD_UNARY_OPERATOR
#define PCL_COD_UNARY_OPERATOR

#include <pcl/cod/CodBase.h>
#include <boost/utility/enable_if.hpp>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class T, class OperatorType, bool ParamEnable=boost::is_same<typename OperatorType::ParamType,void>::value>//, bool Enable=details::is_base_of_CodBase<T>::value>
		class CodUnaryOperator: public CodBase<T::ReferableViaIndex>
		{
		public:
			typedef CodUnaryOperator Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef decltype(boost::declval<OperatorType>().operator()(
				boost::declval<typename T::ReturnType>()
				)) ReturnType;

			static Pointer New(const typename T::Pointer& obj)
			{
				Pointer result(new Self);
				result->m_Object = obj;
				if (obj->isUnbounded()) result->setRegionInfo();
				else result->setRegionInfo(obj);
				return result;
			}
			
			OperatorType& getOperator()
			{
				return m_Operator;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return m_Operator(m_Object->get(iter));
			}

			inline ReturnType get(const Point3D<int>& p) const
			{
				return m_Operator(m_Object->get(p));
			}
			
			inline ReturnType get(const Point3D<int>& p, long i) const
			{
				return m_Operator(m_Object->get(p,i));
			}
			
			DummyImage::ConstantPointer getTemplateImage() const
			{
				return m_Object->getTemplateImage();
			}
			
		protected:
			typename T::Pointer m_Object;
			OperatorType m_Operator;

			CodUnaryOperator() {}
		};


		template <class T, class OperatorType>
		class CodUnaryOperator<T,OperatorType,false>: public CodBase<T::ReferableViaIndex>
		{
		protected:
			typename T::Pointer m_Object;
			OperatorType m_Operator;

		public:
			typedef CodUnaryOperator Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(const typename T::Pointer& obj, const typename OperatorType::ParamType& param)
			{
				Pointer result(new Self);
				result->m_Object = obj;
				if (obj->isUnbounded()) result->setRegionInfo();
				else result->setRegionInfo(obj->getMinIndex(), obj->getRegion());
				obj->m_Operator.param = param;
				return result;
			}

			template <class IteratorType>
			auto get(const IteratorType& iter) -> decltype( m_Operator(m_Object->get(iter)) )
			{
				return m_Operator(m_Object->get(iter));
			}

			auto get(const Point3D<int>& p) const -> decltype( m_Operator(m_Object->get(p)) )
			{
				return m_Operator(m_Object->get(p));
			}
			
			auto get(const Point3D<int>& p, long i) const -> decltype( m_Operator(m_Object->get(p,i)) )
			{
				return m_Operator(m_Object->get(p,i));
			}

		protected:
			CodUnaryOperator() {}
		};


		/*template <class T, class OperatorType, bool ParamEnable>
		class CodUnaryOperator<T, OperatorType, ParamEnable, false>
		{
		public:
			typedef void Pointer;
		};*/


		namespace unary_operator
		{

			struct AbsOperator
			{
				typedef void ParamType;
				template <class T>
				inline auto operator()(const T& a) const -> typename boost::remove_cv<decltype(-a)>::type
				{
					return pcl_Abs(a);
				}
			};
			
			struct SqrtOperator
			{
				typedef void ParamType;
				template <class T>
				inline auto operator()(const T& a) const -> typename boost::remove_cv<decltype(-a)>::type
				{
					return std::sqrt(a);
				}
			};

			struct NegateOperator
			{
				typedef void ParamType;
				template <class T>
				inline auto operator()(const T& a) const -> typename boost::remove_cv<decltype(-a)>::type
				{
					return -a;
				}
			};

			struct NotOperator
			{
				typedef void ParamType;
				template <class T>
				inline auto operator()(const T& a) const -> typename boost::remove_cv<decltype(!a)>::type
				{
					return !a;
				}
			};

			template <class InternalType=double>
			struct InvertOperator
			{
				typedef void ParamType;
				template <class T>
				inline auto operator()(const T& a) const -> typename boost::remove_cv<decltype((InternalType)1/a)>::type
				{
					return static_cast<InternalType>(1/a);
				}
			};

		}
	}
}

#endif