#include "image.h"
#include <pcl/filter2/image/ImageGaussianFilter.h>
#include <itkN4BiasFieldCorrectionImageFilter.h>
#include <itkMedianImageFilter.h>
//#include <pcl/filter2/image/ProjectionDecompositionFilter3D.h>

#define callTypedFunctionParamWithReturn(type,t,func,param) \
	if (type==typeid(long)) return func<long,t>(param); \
	else if (type==typeid(int)) return func<int,t>(param); \
	else if (type==typeid(short)) return func<short,t>(param); \
	else if (type==typeid(char)) return func<char,t>(param); \
	else if (type==typeid(float)) return func<float,t>(param); \
	else if (type==typeid(double)) return func<double,t>(param); \
	else if (type==typeid(unsigned char)) return func<unsigned char,t>(param); \
	else if (type==typeid(unsigned short)) return func<unsigned short,t>(param); \
	else if (type==typeid(unsigned int)) return func<unsigned int,t>(param); \
	else if (type==typeid(unsigned long)) return func<unsigned long,t>(param); \
	else { \
		std::cout << "Invalid type encountered!" << std::endl; \
		pcl_ThrowException(pcl::Exception(), "Invalid type encountered!"); \
	}

template <class T, class O>
ImageObject* _actualConnectedComponentsFilter(boost::tuple<ImageObject*, pcl::iterator::ImageNeighborIterator::OffsetListPointer>& param)
{
	auto input = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<0>()->image());
	auto result = pcl::ImageAlgorithm::ConnectedComponentAnalysis<pcl::Image<O,true>>(input, param.get<1>(), [&](const pcl::Point3D<int>& p, long i){
		return input->get(i)>0;
	});
	return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(O));
}


ImageObject* connectedComponentsFilter(ImageObject* input, const pcl::iterator::ImageNeighborIterator::OffsetListPointer& offset, const std::string& outtype)
{
	boost::tuple<ImageObject*, pcl::iterator::ImageNeighborIterator::OffsetListPointer> param(input, offset);
	if (outtype.compare("char")==0 || outtype.compare("uchar")==0) {
		callTypedFunctionParamWithReturn(input->type(), unsigned char, _actualConnectedComponentsFilter, param);
	} else if (outtype.compare("short")==0 || outtype.compare("ushort")==0) {
		callTypedFunctionParamWithReturn(input->type(), unsigned short, _actualConnectedComponentsFilter, param);
	} else if (outtype.compare("int")==0 || outtype.compare("uint")==0) {
		callTypedFunctionParamWithReturn(input->type(), unsigned int, _actualConnectedComponentsFilter, param);
	} else if (outtype.compare("long") == 0 || outtype.compare("ulong") == 0) {
		callTypedFunctionParamWithReturn(input->type(), unsigned long long, _actualConnectedComponentsFilter, param);
	} else {
		pcl_ThrowException(pcl::Exception(), "Type \""+outtype+"\" is not supported!");
	}
}

template <class T>
ImageObject* _actualGaussianFilter(boost::tuple<ImageObject*, double, double, double, bool, int, double, double>& param)
{
	auto input = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<0>()->image());
	auto sigmax = param.get<1>(), 
		sigmay = param.get<2>(), 
		sigmaz = param.get<3>();
	auto use_image_spacing = param.get<4>();
	auto max_kernel_width = param.get<5>();
	auto kernel_cutoff = param.get<6>();
	auto outval = param.get<7>();
	pcl::Image<float,true>::Pointer result;
	if (std::isfinite(outval)) {
		auto bound = pcl::filter2::Helper::GetZeroFluxBoundary(input);
		result = pcl::filter2::ImageGaussianFilter<decltype(bound), pcl::Image<float,true>>::Compute(bound, sigmax, sigmay, sigmaz, use_image_spacing, max_kernel_width, kernel_cutoff);
	} else {
		auto bound = pcl::filter2::Helper::GetFixedValueBoundary(input, outval);
		result = pcl::filter2::ImageGaussianFilter<decltype(bound), pcl::Image<float,true>>::Compute(bound, sigmax, sigmay, sigmaz, use_image_spacing, max_kernel_width, kernel_cutoff);
	}
	return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(float));
}

ImageObject* gaussianFilter(ImageObject* input, double sigmax, double sigmay, double sigmaz, bool use_image_spacing=true, int max_kernel_width=33, double kernel_cutoff=0.0001, double outval=std::numeric_limits<double>::infinity())
{
	boost::tuple<ImageObject*, double, double, double, bool, int, double, double> param(
		input, sigmax, sigmay, sigmaz, use_image_spacing, max_kernel_width, kernel_cutoff, outval
	);
	callFunctionParamTypeWithReturn(input->type(), _actualGaussianFilter, param);
}

template <class T>
ImageObject* _actualMedianFilter(boost::tuple<ImageObject*, double, double, double>& param)
{
	typedef itk::Image<T,3> ItkImageType;
	typedef itk::MedianImageFilter<ItkImageType,ItkImageType> FilterType;
	auto input = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<0>()->image());
	auto radx = param.get<1>(), 
		rady = param.get<2>(), 
		radz = param.get<3>();
	auto itk_input = pcl::ImageHelper::GetItkImage(input);
	typename FilterType::Pointer median_filter = FilterType::New();
	typename FilterType::InputSizeType radius;
	radius[0] = radx;
	radius[1] = rady;
	radius[2] = radz;
	median_filter->SetRadius(radius);
	median_filter->SetInput(itk_input);
	median_filter->Update();
	auto result = pcl::ImageHelper::CreateFromItkImage<pcl::Image<T,true>>(median_filter->GetOutput(), true);
	return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
}

ImageObject* medianFilter(ImageObject* input, double radx, double rady, double radz)
{
	boost::tuple<ImageObject*, double, double, double> param(input, radx, rady, radz);
	callFunctionParamTypeWithReturn(input->type(), _actualMedianFilter, param);
}

template <class T>
ImageObject* _actualn4BiasFieldCorrFilter(boost::tuple<ImageObject*, ImageObject*>& param)
{
	
	typedef itk::Image<T,3> ItkImageType;
	typedef itk::Image<T,3> MaskImageType;
	typedef itk::N4BiasFieldCorrectionImageFilter<ItkImageType, MaskImageType, ItkImageType> FilterType;
	//typedef itk::N4BiasFieldCorrectionImageFilter<ItkImageType, ItkImageType> FilterType;
	auto input = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<0>()->image());
	auto input_mask = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<1>()->image());

	auto itk_input = pcl::ImageHelper::GetItkImage(input);
	auto itk_mask = pcl::ImageHelper::GetItkImage(input_mask);

	typename FilterType::Pointer n4_bias_field_corr_filter = FilterType::New();
	n4_bias_field_corr_filter->SetInput(itk_input);
	n4_bias_field_corr_filter->SetMaskImage(itk_mask);
	
	n4_bias_field_corr_filter->Update();
	auto result = pcl::ImageHelper::CreateFromItkImage<pcl::Image<T,true>>(n4_bias_field_corr_filter->GetOutput(), true);

	return new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result), typeid(T));
}

ImageObject* n4BiasFieldCorrFilter(ImageObject* input, ImageObject* input_mask)
{
	boost::tuple<ImageObject*, ImageObject*> param(input, input_mask);
	callFunctionParamTypeWithReturn(input->type(), _actualn4BiasFieldCorrFilter, param);
}

/*
template <class T, class O>
std::vector<ImageObject*> _actualProjectionDecompositionFilter(boost::tuple<ImageObject*, double, double, double, int, double, const std::vector<double>&, double, pcl::Region3D<int>, bool>& param)
{
	auto input = boost::static_pointer_cast<pcl::Image<T, true>>(param.get<0>()->image());
	double delta = param.get<1>();
	double epsilon = param.get<2>();
	double lambda = param.get<3>();
	int max = param.get<4>();
	double mu = param.get<5>();
	const std::vector<double> &tau = param.get<6>();
	double outval = param.get<7>();
	pcl::Region3D<int> region = param.get<8>();
	bool use_3d = param.get<9>();

	typedef pcl::Image<O, true> OutImageType;

	std::vector<typename OutImageType::Pointer> result;
	if (boost::math::isnan(outval)) {
		auto boundary = pcl::filter2::Helper::GetZeroFluxBoundary(input);
		if (use_3d) result = pcl::filter2::ProjectionDecompositionFilter3D<decltype(boundary), OutImageType>::Compute(boundary, delta, epsilon, lambda, max, mu, tau, region);
		else result = pcl::filter2::ProjectionDecompositionFilter<decltype(boundary), OutImageType>::Compute(boundary, delta, epsilon, lambda, max, mu, tau, region);
	} else {
		auto boundary = pcl::filter2::Helper::GetFixedValueBoundary(input, outval);
		if (use_3d) result = pcl::filter2::ProjectionDecompositionFilter3D<decltype(boundary), OutImageType>::Compute(boundary, delta, epsilon, lambda, max, mu, tau, region);
		else result = pcl::filter2::ProjectionDecompositionFilter<decltype(boundary), OutImageType>::Compute(boundary, delta, epsilon, lambda, max, mu, tau, region);
	}

	std::vector<ImageObject*> ret(3);
	for (int i = 0; i < 3; ++i) ret[i] = new ImageObject(boost::static_pointer_cast<pcl::ImagePhysicalLayer<true>>(result[i]), typeid(O));
	return ret;
}

std::vector<ImageObject*> projectionDecompositionFilter(ImageObject* input, double delta, double epsilon, double lambda, int max, double mu, const std::vector<double>& tau, double outval, const pcl::Region3D<int>& region, const std::string& outtype, bool use_3d)
{
	boost::tuple<ImageObject*, double, double, double, int, double, const std::vector<double>&, double, pcl::Region3D<int>, bool> param(input, delta, epsilon, lambda, max, mu, tau, outval, region, use_3d);
	if (outtype.compare("double") == 0) {
		callTypedFunctionParamWithReturn(input->type(), double, _actualProjectionDecompositionFilter, param);
	} else if (outtype.compare("float") == 0) {
		callTypedFunctionParamWithReturn(input->type(), float, _actualProjectionDecompositionFilter, param);
	} else {
		pcl_ThrowException(pcl::Exception(), "Type \"" + outtype + "\" is not supported!");
	}
}
*/