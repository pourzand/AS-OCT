#include <pcl/image.h>
#include <pcl/filter2.h>
#include <pcl/image_io.h>
#include <pcl/geometry.h>
#include <math.h>  
#include <pcl/exception.h>
#include <pcl/image/ImagePhysicalLayer.h>
using namespace std;

const double pi=3.14159;
const double radius_step = 0.1;
const double num_point = 200;

class AirWayMeasurement
{
public:
	template <class InputImagePointerType>
	static vector<pcl::Point3D<double>> Compute(const InputImagePointerType& input_image, pcl::Point3D<double> centroid, double num_division=10)
	{
		vector<pcl::Point3D<double>> inner_outer_radius_all;
		auto bound = pcl::filter2::Helper::GetZeroFluxBoundary(input_image);
		auto filter_image = pcl::filter2::ImageGaussianFilter<decltype(bound), pcl::Image<float,true>>::Compute(bound,0.5,0.5,0.5,true,33,0.0001);
		//pcl::Point3D<double> cent = filter_image->toImageCoordinate(centroid);
		//pcl::Point2D<double> center(centroid[0],centroid[1]);
		vector<double> angles;
		for(int i=0; i<360/num_division; i++) angles.push_back(i*num_division);

		for(int i=0;i<angles.size();i++)
		{
			//cout<<i<<" "<<angles[i]<<endl;
			vector<pcl::Point3D<double>> pp;
			pp = candidatePointRay(centroid,angles[i]);
			auto temp = innerOuterRadius(filter_image,i*num_division,pp);;
			//inner_outer_radius_all.push_back(temp);
			for(int i=0;i<temp.size();i++) inner_outer_radius_all.push_back(temp[i]);
		}
		return inner_outer_radius_all;
		
	}

protected:
	static vector<pcl::Point3D<double>> candidatePointRay(pcl::Point3D<double> centroid, double angle)
	{
		vector<pcl::Point3D<double>> cad_list;
		cad_list.push_back(centroid);
		if(abs(angle-90.0)<numeric_limits<double>::epsilon())
		{
			//cout<<"case1"<<endl;
			for(int i=1; i<num_point;i++)
			{
				pcl::Point3D<double> point_on_ray(centroid[0], centroid[1]-i*radius_step,centroid[2]);
				cad_list.push_back(point_on_ray);
			}
		}
		
		else if(abs(angle-270.0)<numeric_limits<double>::epsilon())
		{
			//cout<<"case2"<<endl;
			for(int i=1;i<num_point;i++)
			{
				pcl::Point3D<double> point_on_ray(centroid[0], centroid[1]+i*radius_step,centroid[2]);
				cad_list.push_back(point_on_ray);
			}
		}
		else
		{
			//cout<<"case3"<<endl;
			for(int i=1;i<num_point;i++)
			{
				pcl::Point3D<double> point_on_ray;
				point_on_ray = pointCoordinateCompute(centroid, angle, i*radius_step);
				cad_list.push_back(point_on_ray);
			}
		}
		return cad_list;
	}

	template <class InputImagePointerType>
	static vector<pcl::Point3D<double>> innerOuterRadius(const InputImagePointerType& image,long long num, vector<pcl::Point3D<double>> point_list)
	{
		auto image_interp = pcl::filter2::Helper::GetBilinearInterpolator(pcl::filter2::Helper::GetZeroFluxBoundary(image),0,1);
		vector<double> HU_list;
		vector<double> pp_dis_list;
		vector<double> gradient_list;
		for(int i=0; i<point_list.size();i++)
		{
			pcl::Point3D<double> image_pos = image->toImageCoordinate(point_list[i]);
			double image_pos_value = image_interp.get(image_pos);
			//cout<<image_pos_value<<",";
			HU_list.push_back(image_pos_value);
 			double pp_dis = pow((point_list[i][0]-point_list[0][0]),2.0) + pow((point_list[i][1]-point_list[0][1]),2.0);
			pp_dis = sqrt(pp_dis);
			pp_dis_list.push_back(pp_dis);
			if(i>=1) gradient_list.push_back(HU_list[i]-HU_list[i-1]);

		}
		
		//for(int i=0;i<gradient_list.size();i++) cout<<gradient_list[i]<<" ";
		/*string folder_path = "C:\\Xiaoyong\\general\\airway\\result\\1.2.392.200036.9116.2.6.1.48.1221389715.1454559525.444853_0_128\\";
		string save_path = folder_path + to_string(num)+ ".txt";
		ofstream output_file(save_path);
		ostream_iterator<double> output_iterator(output_file, "\n");
		copy(HU_list.begin(), HU_list.end(), output_iterator)*/;
		vector<double> max_gradient_candidate;
		for (int i=0;i<50;i++) max_gradient_candidate.push_back(gradient_list[i]);
		auto vmaxp = max_element(max_gradient_candidate.begin(),max_gradient_candidate.end());
		double vmaxp_ind = distance(max_gradient_candidate.begin(), vmaxp);
		double vmaxp_position = pp_dis_list[vmaxp_ind];
		vector<double> min_gradient_candidate;
		for (int i=vmaxp_ind+1;i<vmaxp_ind+51;i++) min_gradient_candidate.push_back(gradient_list[i]);
		auto vminp = min_element(min_gradient_candidate.begin(),min_gradient_candidate.end());
		double vminp_ind = distance(min_gradient_candidate.begin(),vminp);
		vminp_ind = vminp_ind + vmaxp_ind+1;
		double vminp_position = pp_dis_list[vminp_ind];
		//cout<<"radius: "<<vmaxp_position<<" "<<vminp_position<<endl;
		double inner_radius,outer_radius;
		inner_radius = (vmaxp_position*2 + radius_step)/2;
		outer_radius = (vminp_position*2 + radius_step)/2;
		pcl::Point3D<double> inner_outer_radius(num, inner_radius,outer_radius);

		// corresponding coordinate of inner and outer radius point (visualize)
		pcl::Point3D<double> vmaxp_coor((point_list[vmaxp_ind]+point_list[vmaxp_ind+1])/2);
		pcl::Point3D<double> vminp_coor((point_list[vminp_ind]+point_list[vminp_ind+1])/2);
		//vector<pcl::Point3D<double>> inner_outer_coor;
		vector<pcl::Point3D<double>> inner_outer_info;
		inner_outer_info.push_back(inner_outer_radius);
		inner_outer_info.push_back(vmaxp_coor);
		inner_outer_info.push_back(vminp_coor);
		return inner_outer_info;
		//return inner_outer_radius;
	}

	static pcl::Point3D<double> pointCoordinateCompute(pcl::Point3D<double> centroid, double angle, double radius)
	{
		double slope = tan(angle*pi/180);
		pcl::Point3D<double> target_point;
		if(angle>90 && angle<270) target_point[0] = centroid[0] - sqrt(pow(radius,2.0)/(1+pow(slope,2.0)));
		else if((angle>=0 && angle<90) || (angle>270 && angle<360)) target_point[0] = centroid[0] + sqrt(pow(radius,2.0)/(1+pow(slope,2.0)));
		target_point[1] = centroid[1] + slope*(target_point[0] - centroid[0]);
		target_point[2] = centroid[2];
		return target_point;
	}

};