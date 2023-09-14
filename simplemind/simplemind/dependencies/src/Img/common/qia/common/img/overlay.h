#ifndef __CYTHON_OVERLAY__
#define __CYTHON_OVERLAY__

#include <pcl/misc/ColorImageGenerator.h>
#include <pcl/filter2.h>
#include "image.h"

template <class T>
void _setImage(boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, double, double, bool, bool, double>& param)
{
	auto gen = param.get<0>();
	auto image = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<1>().image());
	double minval = param.get<2>(),
		maxval = param.get<3>();
	bool use_nn = param.get<4>();
	bool fixed_boundary = param.get<5>();
	double boundary_value = param.get<6>();
	if (fixed_boundary) {
		auto bound = pcl::filter2::Helper::GetFixedValueBoundary(image, boundary_value);
		if (use_nn) {
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound);
			gen.setImage(interp, minval, maxval);
		} else {
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound);
			gen.setImage(interp, minval, maxval);
		}
	} else {
		auto bound = pcl::filter2::Helper::GetZeroFluxBoundary(image);
		if (use_nn) {
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound);
			gen.setImage(interp, minval, maxval);
		} else {
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound);
			gen.setImage(interp, minval, maxval);
		}
	}
}
void setImage(pcl::misc::ColorImageGenerator& gen, const ImageObject& obj, double minval, double maxval, bool use_nn, bool fixed_boundary, double boundary_value)
{
	boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, double, double, bool, bool, double> param(gen, obj, minval, maxval, use_nn, fixed_boundary, boundary_value);
	callFunctionParamType(obj.type(), _setImage, param);
}

template <class T>
void _addOverlay(boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, pcl::misc::RGBColor, double, bool, bool, double>& param)
{
	auto gen = param.get<0>();
	auto image = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<1>().image());
	auto color = param.get<2>();
	double alpha = param.get<3>();
	bool use_nn = param.get<4>();
	bool fixed_boundary = param.get<5>();
	double boundary_value = param.get<6>();
	if (fixed_boundary) {
		auto bound = pcl::filter2::Helper::GetFixedValueBoundary(image, boundary_value);
		if (use_nn) {
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound);
			gen.addOverlay(interp, color, alpha);
		} else {
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound);
			gen.addOverlay(interp, color, alpha);
		}
	} else {
		auto bound = pcl::filter2::Helper::GetZeroFluxBoundary(image);
		if (use_nn) {
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound);
			gen.addOverlay(interp, color, alpha);
		} else {
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound);
			gen.addOverlay(interp, color, alpha);
		}
	}
}
void addOverlay(pcl::misc::ColorImageGenerator& gen, const ImageObject& obj, unsigned char r, unsigned char g, unsigned char b, double alpha, bool use_nn, bool fixed_boundary, double boundary_value)
{
	boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, pcl::misc::RGBColor, double, bool, bool, double> param(gen, obj, pcl::misc::RGBColor(r,g,b), alpha, use_nn, fixed_boundary, boundary_value);
	callFunctionParamType(obj.type(), _addOverlay, param);
}

#define declareLookupFunc(func_name, gen_func) \
template <class T> \
void _ ## func_name ## WithLookup(boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, pcl::misc::LookupTable*, bool, bool, double>& param) \
{ \
	auto gen = param.get<0>(); \
	auto image = boost::static_pointer_cast<pcl::Image<T,true>>(param.get<1>().image()); \
	auto lookup = param.get<2>(); \
	bool use_nn = param.get<3>(); \
	bool fixed_boundary = param.get<4>(); \
	double boundary_value = param.get<5>(); \
	if (fixed_boundary) { \
		auto bound = pcl::filter2::Helper::GetFixedValueBoundary(image, boundary_value); \
		if (use_nn) { \
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound); \
			gen.gen_func(interp, *lookup); \
		} else { \
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound); \
			gen.gen_func(interp, *lookup); \
		} \
	} else { \
		auto bound = pcl::filter2::Helper::GetZeroFluxBoundary(image); \
		if (use_nn) { \
			auto interp = pcl::filter2::Helper::GetNearestNeighborInterpolator(bound); \
			gen.gen_func(interp, *lookup); \
		} else { \
			auto interp = pcl::filter2::Helper::GetTrilinearInterpolator(bound); \
			gen.gen_func(interp, *lookup); \
		} \
	} \
} \
void func_name(pcl::misc::ColorImageGenerator& gen, const ImageObject& obj, pcl::misc::LookupTable* lookup, bool use_nn, bool fixed_boundary, double boundary_value) \
{ \
	boost::tuple<pcl::misc::ColorImageGenerator&, const ImageObject&, pcl::misc::LookupTable*, bool, bool, double> param(gen, obj, lookup, use_nn, fixed_boundary, boundary_value); \
	callFunctionParamType(obj.type(), _ ## func_name ## WithLookup, param); \
}

declareLookupFunc(setImage, setImage)
declareLookupFunc(addImage, addImage)

pcl::misc::ColorImageGenerator* autoNew(ImageObject* obj, const pcl::Point3D<double>& point, const pcl::Region3D<double>& region, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis, double spacing_mm=-1)
{
	pcl::Region3D<double> actual_region = region;
	if (actual_region.empty()) {
		actual_region.getMinPoint() = obj->image()->getMinPoint();
		actual_region.getMaxPoint() = obj->image()->getMaxPoint();
	}
	if (spacing_mm<=0) {
		spacing_mm = std::numeric_limits<double>::infinity();
		for (int i=0; i<3; ++i) spacing_mm = std::min(obj->image()->getSpacing()[i], spacing_mm);
	}
	return new pcl::misc::ColorImageGenerator(point, obj->image(), actual_region, spacing_mm, x_axis, y_axis);
}

pcl::misc::ColorImageGenerator* autoNew(const pcl::Point3D<double>& point, const pcl::Region3D<double>& region, const pcl::Point3D<double>& x_axis, const pcl::Point3D<double>& y_axis, double spacing_mm)
{
	return new pcl::misc::ColorImageGenerator(point, region, spacing_mm, x_axis, y_axis);
}

template <class T>
boost::tuple<pcl::Point3D<double>, pcl::Point3D<double>> _actualGetExtent(boost::tuple<ImageObject*, pcl::Point3D<double>>& info)
{
	 auto image = boost::static_pointer_cast<pcl::Image<T,true>>(info.get<0>()->image());
	 auto normal = info.get<1>();
	 pcl::ImageIteratorWithPoint iter(image);
	 double min_val = std::numeric_limits<double>::infinity(), 
		 max_val = -std::numeric_limits<double>::infinity();
	 pcl_ForIterator(iter) if (image->get(iter)>0) {
		 double d = image->toPhysicalCoordinate(iter.getPoint()).getDotProduct(normal);
		 min_val = std::min(d, min_val);
		 max_val = std::max(d, max_val);
	 }
	 return boost::tuple<pcl::Point3D<double>, pcl::Point3D<double>>(normal*min_val, normal*max_val);
}

static boost::tuple<pcl::Point3D<double>, pcl::Point3D<double>> getExtent(ImageObject* obj, const pcl::Point3D<double>& normal)
{
	boost::tuple<ImageObject*, pcl::Point3D<double>> param(obj, normal);
	callFunctionParamTypeWithReturn(obj->type(), _actualGetExtent, param);
}


template <class T>
pcl::Point3D<double> _actualGetCentroid(ImageObject* obj)
{
	 auto image = boost::static_pointer_cast<pcl::Image<T,true>>(obj->image());
	 pcl::Point3D<double> centroid(0,0,0);
	 double num = 0;
	 pcl::ImageIteratorWithPoint iter(image);
	 pcl_ForIterator(iter) if (image->get(iter)>0) {
		 centroid += image->toPhysicalCoordinate(iter.getPoint());
		 num += 1;
	 }
	 return centroid/num;
}

static pcl::Point3D<double> getCentroid(ImageObject* obj)
{
	callFunctionParamTypeWithReturn(obj->type(), _actualGetCentroid, obj);
}

#endif