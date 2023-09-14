#ifndef PCL_QUANTIZE_HELPER
#define PCL_QUANTIZE_HELPER

#include <pcl/cod/CodHelper.h>
#include <pcl/filter/glcm/CodQuantize.h>

namespace pcl
{
	namespace filter
	{
		using namespace pcl;
		using namespace pcl::cod;

		class QuantizeHelper
		{
		public:
			template <class ReturnType, class CodPointerType>
			static CodEncapsulator<CodQuantize<typename CodPointerType::element_type, ReturnType>> Create(const CodPointerType& cod, typename CodPointerType::element_type::ReturnType minval, typename CodPointerType::element_type::::ReturnType maxval, typename CodPointerType::element_type::::ReturnType binsize)
			{
				return CodEncapsulatorCodEncapsulator<CodQuantize<typename CodPointerType::element_type, ReturnType>>(
					CodQuantize<CodPointerType,ReturnType>::New(cod, minval, maxval, binsize)
					);
			}

			template <class ReturnType, class ImagePointerType>
			static CodEncapsulator<CodQuantize<ImageToCodConverter<typename ImagePointerType::element_type>, ReturnType>> CreateFromImage(const ImagePointerType& image, typename ImagePointerType::element_type::ValueType minval, typename ImagePointerType::element_type::ValueType maxval, typename ImagePointerType::element_type::ValueType binsize)
			{
				return Create<ReturnType>(CodHelper::CreateFromImage(image), minval, maxval, binsize);
			}

			template <class T>
			static T GetSize(T minval, T maxval, T binsize)
			{
				return ((maxval-minval)/binsize)+1;
			}

			static double GetBinSize(double minval, double maxval, long bin_num)
			{
				return (maxval-minval)/(bin_num-1);
			}
		};

	}
}

#endif