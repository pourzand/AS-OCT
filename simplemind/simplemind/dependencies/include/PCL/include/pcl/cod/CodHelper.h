#ifndef PCL_COD_HELPER
#define PCL_COD_HELPER

#include <pcl/cod/ImageToCodConverter.h>
#include <pcl/cod/PointFilterToCodConverter.h>
#include <pcl/cod/FixedValueCod.h>
#include <pcl/cod/CodBinaryOperator.h>
#include <pcl/cod/CodUnaryOperator.h>
#include <pcl/cod/CodIfElse.h>
#include <pcl/cod/CodToImageConverter.h>
#include <pcl/cod/BufferedCodToImageConverter.h>
#include <pcl/cod/ImageToBoundaryHandledCod.h>
#include <pcl/cod/LimitlessIndexlessCodConverter.h>
#include <pcl/cod/HessianEigenVectorCodConverter.h>
#include <pcl/cod/LazyImageCod.h>
#include <pcl/type_utility.h>

namespace pcl
{
	using namespace pcl::cod;

	class CodHelper;

	namespace cod
	{
		class CodEncapsulatorBase //Meant for type checking and virtual access of values
		{
		public:
			virtual double getValue(const Point3D<int>& p) const = 0;
			virtual double getValue(const Point3D<int>& p, long index) const = 0;
			
			template <class IteratorType>
			inline double getValue(const IteratorType& iter) const
			{
				return getValue(iter.getPoint(), iter);
			}

		protected:
			CodEncapsulatorBase() {}
			pcl::image_base_details::ToDouble m_DoubleValueConverter;
		};
		
		template <class CodPointer>
		class CodEncapsulator;
	}

	
	
	class CodHelper
	{
	public:
		template <class T>
		struct determine_cod_type
		{
			typedef typename boost::remove_reference<decltype(CreateIfNeeded(boost::declval<T>()))>::type EncapsulatorType;
			typedef typename EncapsulatorType::CodType type;
		};

		template <class CodPointer>
		static CodEncapsulator<typename CodPointer::element_type> Encapsulate(const CodPointer& cod)
		{
			return CodEncapsulator<typename CodPointer::element_type>(cod);
		}
		
		template <class ImageType, class CodType>
		static typename ImageType::Pointer GetImageCopy(CodType& obj)
		{
            auto template_image = obj->getTemplateImage();
			if (!template_image) pcl_ThrowException(Exception(), "Template image is NULL!");
			auto result = ImageType::New(template_image);
			pcl::ImageIteratorWithPoint iter(result);
			pcl_ForIterator(iter) {
                result->set(iter, obj->get(iter));
			}
			return result;
		}
		
		template <class ImageType, class FunctionType, class TemplateImagePointerType>
		static typename CodToImageConverter<LazyImageCod<ImageType>, true>::ConstantPointer GetLazyImage(FunctionType& func, TemplateImagePointerType& template_image, const pcl::Region3D<int>& region=pcl::Region3D<int>().reset())
		{
			auto cod = LazyImageCod<ImageType>::New(func, template_image, region);
			return CodToImageConverter<LazyImageCod<ImageType>, true>::New(cod, cod->getTemplateImage());
		}
		
		template <class ImageType, class CodType, class ImagePointer>
		static typename ImageType::Pointer GetImageCopy(CodType& obj, const ImagePointer& template_image)
		{
			auto result = ImageType::New(template_image);
			pcl::ImageIteratorWithPoint iter(result);
			pcl_ForIterator(iter) {
                result->set(iter, obj->get(iter));
			}
			return result;
		}

		/********************************** Create from image **********************************/
		template <class ImagePointerType>
		static CodEncapsulator<ImageToCodConverter<typename ImagePointerType::element_type>> CreateFromImage(const ImagePointerType& image)
		{
			typedef ImageToCodConverter<typename ImagePointerType::element_type> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(image));
		}

		template <template<class> class BoundaryHandlerClass, class ImagePointerType>
		static CodEncapsulator<ImageToBoundaryHandledCod<typename ImagePointerType::element_type, BoundaryHandlerClass>> CreateFromImage(const ImagePointerType& image)
		{
			typedef ImageToBoundaryHandledCod<typename ImagePointerType::element_type, BoundaryHandlerClass> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(image));
		}

		/********************************** Create from point filter **********************************/
		template <class PointFilterPointerType>
		static CodEncapsulator<PointFilterToCodConverter<typename PointFilterPointerType::element_type>> CreateFromPointFilter(const PointFilterPointerType& filter, int index=0)
		{
			typedef PointFilterToCodConverter<typename PointFilterPointerType::element_type> ReturnType;
			return PointFilterToCodConverter<typename PointFilterPointerType::element_type>::New(filter, index);
		}
		template <bool ReferableViaIndex, class PointFilterPointerType>
		static CodEncapsulator<PointFilterToCodConverter<typename PointFilterPointerType::element_type, ReferableViaIndex>> CreateFromPointFilter(const PointFilterPointerType& filter, int index=0)
		{
			typedef PointFilterToCodConverter<typename PointFilterPointerType::element_type, ReferableViaIndex> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(filter, index));
		}

		template <class PointFilterPointerType, class ImagePointerType>
		static CodEncapsulator<PointFilterToCodConverter<typename PointFilterPointerType::element_type>> CreateFromPointFilter(const PointFilterPointerType& filter, const ImagePointerType& image_template, int index)
		{
			typedef PointFilterToCodConverter<typename PointFilterPointerType::element_type> ReturnType;
			return PointFilterToCodConverter<typename PointFilterPointerType::element_type>::New(filter, image_template, index);
		}
		template <bool ReferableViaIndex, class PointFilterPointerType, class ImagePointerType>
		static CodEncapsulator<PointFilterToCodConverter<typename PointFilterPointerType::element_type, ReferableViaIndex>> CreateFromPointFilter(const PointFilterPointerType& filter, const ImagePointerType& image_template, int index)
		{
			typedef PointFilterToCodConverter<typename PointFilterPointerType::element_type, ReferableViaIndex> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(filter, image_template, index));
		}
		
		/********************************** Create from value **********************************/
		template <class ValueType>
		static CodEncapsulator<FixedValueCod<ValueType>> CreateFromValue(const ValueType& val)
		{
			typedef FixedValueCod<ValueType> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(val));
		}
		
		/********************************** Create from eigen vector cod **********************************/
		template <class FilterPointerType>
		static CodEncapsulator<HessianEigenVectorCodConverter<typename FilterPointerType::element_type>> CreateHessianEigenVectorCod(const FilterPointerType& filter, int index)
		{
			typedef HessianEigenVectorCodConverter<typename FilterPointerType::element_type> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(filter, index));
		}

		/********************************** Create if else condition cod **********************************/
		template <class CondType, class IfType, class ElseType>
		static auto CreateIfElse(const CondType& cond, const IfType& if_obj, const ElseType& else_obj) -> CodEncapsulator<CodIfElse< 
			typename determine_cod_type<CondType>::type, 
			typename determine_cod_type<IfType>::type, 
			typename determine_cod_type<ElseType>::type,
			decltype( 
				boost::declval<typename determine_cod_type<IfType>::type::ReturnType>() + 
				boost::declval<typename determine_cod_type<ElseType>::type::ReturnType>() 
			)
		>>
		{
			typedef CodIfElse< 
				typename determine_cod_type<CondType>::type, 
				typename determine_cod_type<IfType>::type, 
				typename determine_cod_type<ElseType>::type,
				decltype( 
				boost::declval<typename determine_cod_type<IfType>::type::ReturnType>() + 
				boost::declval<typename determine_cod_type<ElseType>::type::ReturnType>() 
				)
			> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(CreateIfNeeded(cond), CreateIfNeeded(if_obj), CreateIfNeeded(else_obj)));
		}

		template <class ReturnValueType, class CondType, class IfType, class ElseType>
		static CodEncapsulator<CodIfElse< 
			typename determine_cod_type<CondType>::type, 
			typename determine_cod_type<IfType>::type, 
			typename determine_cod_type<ElseType>::type,
			ReturnValueType
		>> CreateIfElse(const CondType& cond, const IfType& if_obj, const ElseType& else_obj)
		{
			typedef CodIfElse< 
				typename determine_cod_type<CondType>::type, 
				typename determine_cod_type<IfType>::type, 
				typename determine_cod_type<ElseType>::type,
				ReturnValueType
			> ReturnType;
			return CodEncapsulator<ReturnType>(ReturnType::New(CreateIfNeeded(cond), CreateIfNeeded(if_obj), CreateIfNeeded(else_obj)));
		}

		/********************************** Create if needed methods **********************************/
		template <class Type>
		static typename boost::enable_if_c<
			!boost::is_base_of<CodEncapsulatorBase, Type>::value && !details::is_base_of_CodBase<Type>::value,
			CodEncapsulator<FixedValueCod<Type>>
		>::type CreateIfNeeded(const Type& obj)
		{
			return CreateFromValue(obj);
		}

		template <class Type>
		static typename boost::enable_if<boost::is_base_of<CodEncapsulatorBase, Type>, const Type&>::type CreateIfNeeded(const Type& obj)
		{
			return obj;
		}

		template <class Type>
		static typename boost::enable_if<
			details::is_base_of_CodBase<Type>,
			CodEncapsulator<typename Type::element_type>
		>::type CreateIfNeeded(const Type& obj)
		{
			return CodEncapsulator<typename Type::element_type>(obj);
		}
	};


	namespace cod
	{	
		/**
			Encapsulate object from CodBase
			The main purpose of this object is to enable the use of overloaded operators
		**/
		template <class CType>
		class CodEncapsulator: public CodEncapsulatorBase
		{
		public:
			typedef CodEncapsulator Self;
			typedef CType CodType;
			typedef typename CodType::Pointer CodPointer;

			CodEncapsulator() {}
			CodEncapsulator(const CodPointer& ptr)
			{
				m_Pointer = ptr;
			}
			CodEncapsulator(const CodEncapsulator& obj)
			{
				m_Pointer = obj.m_Pointer;
			}

			CodEncapsulator& operator=(const CodEncapsulator& obj)
			{
				m_Pointer = obj.m_Pointer;
				return *this;
			}

			typename CodPointer::element_type* operator->() const
			{
				return m_Pointer.get();
			}

			const CodPointer get() const
			{
				return m_Pointer;
			}

			operator const CodPointer&() const
			{
				return m_Pointer;
			}
			
			/********************************** Virtual methods from base **********************************/
			using CodEncapsulatorBase::getValue;
			
			virtual inline double getValue(const Point3D<int>& p) const
			{
				return m_DoubleValueConverter(m_Pointer->get(p));
			}
			virtual inline double getValue(const Point3D<int>& p, long index) const
			{
				return m_DoubleValueConverter(m_Pointer->get(p, index));
			}

			/********************************** Image conversion methods **********************************/
			typename CodToImageConverter<CodType,false>::ConstantPointer getImage()
			{
				return getImage<false>();
			}
			template <bool UseOrientationMatrix>
			typename CodToImageConverter<CodType,UseOrientationMatrix>::ConstantPointer getImage()
			{
				auto template_image = m_Pointer->getTemplateImage();
				if (!template_image) pcl_ThrowException(Exception(), "Template image is NULL!");
				return CodToImageConverter<CodType,UseOrientationMatrix>::New(m_Pointer, template_image);
			}

			template <class ImagePointer>
			typename CodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix>::ConstantPointer getImage(const ImagePointer& template_image)
			{
				return CodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix>::New(m_Pointer, template_image);
			}

			template <bool UseOrientationMatrix>
			typename BufferedCodToImageConverter<CodType,UseOrientationMatrix>::ConstantPointer getBufferedImage()
			{
				auto template_image = m_Pointer->getTemplateImage();
				if (!template_image) pcl_ThrowException(Exception(), "Template image is NULL!");
				return BufferedCodToImageConverter<CodType,UseOrientationMatrix>::New(m_Pointer, template_image);
			}

			template <bool UseOrientationMatrix, class BufferType>
			typename BufferedCodToImageConverter<CodType, UseOrientationMatrix, BufferType>::ConstantPointer getBufferedImage()
			{
				auto template_image = m_Pointer->getTemplateImage();
				if (!template_image) pcl_ThrowException(Exception(), "Template image is NULL!");
				return BufferedCodToImageConverter<CodType, UseOrientationMatrix, BufferType>::New(m_Pointer, template_image);
			}

			template <class ImagePointer>
			typename BufferedCodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix>::ConstantPointer getBufferedImage(const ImagePointer& template_image)
			{
				return BufferedCodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix>::New(m_Pointer, template_image);
			}
			
			template <class BufferType, class ImagePointer>
			typename BufferedCodToImageConverter<CodType, ImagePointer::element_type::UseOrientationMatrix, BufferType>::ConstantPointer getBufferedImage(const ImagePointer& template_image)
			{
				return BufferedCodToImageConverter<CodType, ImagePointer::element_type::UseOrientationMatrix, BufferType>::New(m_Pointer, template_image);
			}

			template <class ImagePointer>
			typename BufferedCodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix,typename ImagePointer::element_type::BufferType>::ConstantPointer getBufferedImageUsing(const ImagePointer& template_image)
			{
				return BufferedCodToImageConverter<CodType,ImagePointer::element_type::UseOrientationMatrix,typename ImagePointer::element_type::BufferType>::NewWithTemplateImageAsBuffer(m_Pointer, template_image);
			}

			/********************************** Arithmetic operators **********************************/

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::AdditionOperator>> operator+(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::AdditionOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::AdditionOperator>> operator+(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) + obj2;
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::SubtractionOperator>> operator-(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::SubtractionOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::SubtractionOperator>> operator-(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) - obj2;
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MultiplicationOperator>> operator*(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MultiplicationOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::MultiplicationOperator>> operator*(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) * obj2;
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::DivisionOperator>> operator/(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::DivisionOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::DivisionOperator>> operator/(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) / obj2;
			}

			/********************************** Comparison operators **********************************/

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::EqOperator>> operator==(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::EqOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::EqOperator>> operator==(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) == obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::NeOperator>> operator!=(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::NeOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::NeOperator>> operator!=(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) != obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::GtOperator>> operator>(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::GtOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::GtOperator>> operator>(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) > obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::GeOperator>> operator>=(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::GeOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::GeOperator>> operator>=(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) >= obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::LtOperator>> operator<(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::LtOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::LtOperator>> operator<(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) < obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::LeOperator>> operator<=(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::LeOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::LeOperator>> operator<=(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) <= obj2;
			}

			/********************************** Logical operators **********************************/

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::AndOperator>> operator&&(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::AndOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::AndOperator>> operator&&(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) && obj2;
			}

			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::OrOperator>> operator||(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::OrOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::OrOperator>> operator||(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1) || obj2;
			}

			/********************************** Unary operators **********************************/

			CodEncapsulator<CodUnaryOperator<CodType, unary_operator::NegateOperator> > operator-()
			{
				typedef CodUnaryOperator<CodType, unary_operator::NegateOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer)
				);
			}

			CodEncapsulator<CodUnaryOperator<CodType, unary_operator::NotOperator> > operator!()
			{
				typedef CodUnaryOperator<CodType, unary_operator::NotOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer)
				);
			}

			/********************************** Misc **********************************/

			CodEncapsulator<CodUnaryOperator<CodType, unary_operator::AbsOperator>> abs()
			{
				typedef CodUnaryOperator<CodType, unary_operator::AbsOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer)
				);
			}
			
			CodEncapsulator<CodUnaryOperator<CodType, unary_operator::SqrtOperator>> sqrt()
			{
				typedef CodUnaryOperator<CodType, unary_operator::SqrtOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer)
				);
			}

			CodEncapsulator<LimitlessIndexlessCodConverter<CodType>> getLimitlessIndexless()
			{
				typedef LimitlessIndexlessCodConverter<CodType> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer)
				);
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MaxOperator>> max(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MaxOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MinOperator>> min(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::MinOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::PowerOperator>> pow(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::PowerOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			
			template <class Target>
			CodEncapsulator<CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::DotProductOperator>> dotProduct(const Target& obj)
			{
				typedef CodBinaryOperator<CodType, typename CodHelper::determine_cod_type<Target>::type, binary_operator::DotProductOperator> ReturnType;
				return CodEncapsulator<ReturnType>(
					ReturnType::New(m_Pointer, CodHelper::CreateIfNeeded(obj))
				);
			}
			template <class Target>
			friend CodEncapsulator<CodBinaryOperator<typename CodHelper::determine_cod_type<Target>::type, CodType, binary_operator::DotProductOperator>> dotProduct(const Target& obj1, const Self& obj2)
			{
				return CodHelper::CreateIfNeeded(obj1).dotProduct(obj2);
			}

		protected:
			CodPointer m_Pointer;
		};
	}

}

#endif
