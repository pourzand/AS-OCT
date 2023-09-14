#ifndef PCLsequence_h
#define PCLsequence_h

#include "MedicalImageSequence.h"
#include <pcl/image_io.h>

class PCLsequence : public MedicalImageSequence {
public:
	typedef pcl::Image<short> ImageType;
	///Constructor.
	PCLsequence(const std::string& file) : MedicalImageSequence()
	{
		auto image = pcl::ImageIoHelper::Read<ImageType>(file);
		_collect_dicom_nonimage_data();
		_num_images = image->getSize()[2];
		_images = new Image*[_num_images];
		_init_image_planes();

		auto minp = image->getMinPoint(),
			maxp = image->getMaxPoint();
		int i=0;
		for (int z=minp.z(); z<=maxp.z(); ++z) {
			auto region = image->getRegion();
			region.getMinPoint()[2] = z;
			region.getMaxPoint()[2] = z;
			auto slice = pcl::ImageHelper::GetCroppedAuto(image, region);
			_collect_dicom_image_data(slice, i, z);
			++i;
		}
	}

	///Destructor.
	~PCLsequence() {}

private:
	std::string m_File;

	///Collects non-image data for DICOM images
	void _collect_dicom_nonimage_data()
	{
		patient_name("dummy");
		patient_id("dummy");
		patient_birth_date("dummy");
		patient_birth_time("dummy");
		patient_sex("dummy");
		study_date("dummy");
		study_time("dummy");
		accession_number("dummy");
		study_id("dummy");
		study_instance_uid("dummy");
		study_description("dummy");
		patient_age("dummy");
		patient_age("dummy");
		patient_size("dummy");
		patient_weight("dummy");
		modality("CT");
		series_number("dummy");
		series_instance_uid("dummy");
		body_part_examined("dummy");
		view_position("dummy");
		series_date("dummy");
		series_time("dummy");
		referring_physicians_name("dummy");
		performing_physicians_name("dummy");
		protocol_name("dummy");
		series_description("dummy");
		operators_name("dummy");
		patient_position("dummy");
		manufacturer("dummy");
		institution_name("dummy");
		manufacturers_model_name("dummy");
	}

	///Collects specific image data for DICOM image
	void _collect_dicom_image_data(ImageType::Pointer& image, int index, int z)
	{
		int bits_per_pixel = sizeof(short)*8;
		float arow_pixel_spacing = float(image->getSpacing()[0]);
		float acolumn_pixel_spacing = float(image->getSpacing()[1]);
		int width = image->getSize()[0];
		int height = image->getSize()[1];

		float aslice_thickness = image->getSpacing()[2];
		float aslice_location = image->toPhysicalCoordinate(image->getMinPoint()).z();

		int instance_number = z;

		//Creating the image
		_images[index] = new Image(instance_number, width, height, bits_per_pixel);
		_images[index]->unsafe_pixel_data() = image->getBuffer()->getPointer();
		image->getBuffer()->dropOwnership();

		/*
		std::cout << arow_pixel_spacing 
			<< " " << acolumn_pixel_spacing 
			<< " " << aslice_thickness 
			<< " " << aslice_location 
			<< " " << width
			<< " " << height
			<< " " << index
			<< " " << instance_number
			<< std::endl;
		*/

		//Set MedicalImageSequence variables
		row_pixel_spacing(index, arow_pixel_spacing);
		column_pixel_spacing(index, acolumn_pixel_spacing);
		slice_thickness(index, aslice_thickness);
		slice_location(index, aslice_location);
		kvp(index, "dummy");
		exposure_time(index, "dummy");
		xray_tube_current(index, "dummy");
		exposure(index, "dummy");
		reconstruction_diameter(index, "dummy");
	}
};

#endif