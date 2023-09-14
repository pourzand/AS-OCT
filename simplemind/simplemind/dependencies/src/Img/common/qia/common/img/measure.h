#include <pcl/image.h>
#include <pcl/image_io.h>
#include <pcl/filter2.h>
#include <image.h>
#include <math.h>  
#include <pcl/geometry.h>
#include <AirwayMeasurement.h>

template<class T>
std::vector<pcl::Point3D<double>> _actualAirwayMeasurement(boost::tuple<ImageObject*, pcl::Point3D<double>, double>& param)
{
    auto image = boost::static_pointer_cast<pcl::Image<T,true>>(boost::get<0>(param)->image());
    auto centroid = boost::get<1>(param);
    auto division = boost::get<2>(param);
    auto result = AirWayMeasurement::Compute(image, centroid, division);
	return result;
	
}


std::vector<pcl::Point3D<double>> airWayMeasurement(ImageObject* image, const pcl::Point3D<double>& centroid, double division)
{
    boost::tuple<ImageObject*, pcl::Point3D<double>, double> param(image, centroid, division);
    callFunctionParamTypeWithReturn(image->type(), _actualAirwayMeasurement, param);
}
