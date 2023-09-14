#ifndef DICOMsequence_h
#define DICOMsequence_h

#undef Status
extern "C" {
};


#include "MedicalImageSequence.h"
#include <vector>
#include <pcl/image_io.h>

/// Image sequence corresponding to DICOM file
class DICOMsequence : public MedicalImageSequence {
public:
	typedef pcl::Image<short> ImageType;
	///Constructor.
	DICOMsequence(const std::vector<std::string> &filenames) : MedicalImageSequence()
	{
		if(!filenames.empty()) {
			_num_images = filenames.size();
			_images = new Image*[_num_images];
			_init_image_planes();
			for(int i = 0; i < filenames.size(); i++) {
				try {
					ImageType::Pointer img = pcl::ItkImageReader<ImageType>::ReadImage(filenames[i]);
					auto min_max = pcl::ImageHelper::GetMinMax(img);
					//std::cout << "Range: " << min_max.get<0>() << " " << min_max.get<1>() << std::endl;
					if (i==0) _collect_dicom_nonimage_data(img);
					_collect_dicom_image_data(img, i, filenames[i].c_str());
					//std::cout << "Index: " << i << " " << filenames[i] << std::endl;
				} catch (pcl::Exception& e) {
					std::cerr << e;
				}
			}
		}
	}

	///Destructor.
	~DICOMsequence() {}

private:
	bool _get_image_meta_data_element(std::string& str, const std::string& key, const ImageType::Pointer& image)
	{
		auto iter = image->getMetadata()->find(key);
		if (iter!=image->getMetadata()->end()) {
			str = image->getMetadata()->getValue<std::string>(iter);
			return true;
		}
		return false;
	}

	///Collects non-image data for DICOM images
	void _collect_dicom_nonimage_data(const ImageType::Pointer& image)
	{
		std::string buffer;
		if(!_get_image_meta_data_element(buffer, "0010|0010", image)) {
#ifdef DEBUG
			std::cerr << "apatient_name read failed" << std::endl;
#endif
			patient_name(NULL);
		} else patient_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|0020", image)) {
#ifdef DEBUG
			std::cerr << "apatient_id read failed" << std::endl;
#endif
			patient_id(NULL);
		} else patient_id(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|0030", image)) {
#ifdef DEBUG
			std::cerr << "apatient_birthdate read failed" << std::endl;
#endif
			patient_birth_date(NULL);
		} else patient_birth_date(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|0032", image)) {
#ifdef DEBUG
			std::cerr << "apatient_birthtime read failed" << std::endl;
#endif
			patient_birth_time(NULL);
		} else patient_birth_time(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|0040", image)) {
#ifdef DEBUG
			std::cerr << "apatient_sex read failed" << std::endl;
#endif
			patient_sex(NULL);
		} else patient_sex(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0020", image)) {
#ifdef DEBUG
			std::cerr << "astudy_date read failed" << std::endl;
#endif
			study_date(NULL);
		} else study_date(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0030", image)) {
#ifdef DEBUG
			std::cerr << "astudy_time read failed" << std::endl;
#endif
			study_time(NULL);
		} else study_time(buffer.c_str());


		if(!_get_image_meta_data_element(buffer, "0008|0050", image)) {
#ifdef DEBUG
			std::cerr << "Aaccession_number read failed" << std::endl;
#endif
			accession_number(NULL);
		} else accession_number(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0020|0010", image)) {
#ifdef DEBUG
			std::cerr << "astudy_id read failed" << std::endl;
#endif
			study_id(NULL);
		} else study_id(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0020|000d", image)) {
#ifdef DEBUG
			std::cerr << "astudy_instance_uid read failed" << std::endl;
#endif
			study_instance_uid(NULL);
		} else study_instance_uid(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|1030", image)) {
#ifdef DEBUG
			std::cerr << "astudy_description read failed" << std::endl;
#endif
			study_description(NULL);
		} else study_description(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|1010", image)) {
#ifdef DEBUG
			std::cerr << "apatient_age read failed" << std::endl;
#endif
			patient_age(NULL);
		} else patient_age(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|1010", image)) {
#ifdef DEBUG
			std::cerr << "apatient_age read failed" << std::endl;
#endif
			patient_age(NULL);
		} else patient_age(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|1020", image)) {
#ifdef DEBUG
			std::cerr << "apatient_size read failed" << std::endl;
#endif
			patient_size(NULL);
		} else patient_size(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0010|1030", image)) {
#ifdef DEBUG
			std::cerr << "apatient_weight read failed" << std::endl;
#endif
			patient_weight(NULL);
		} else patient_weight(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0060", image)) {
#ifdef DEBUG
			std::cerr << "amodality read failed" << std::endl;
#endif
			modality(NULL);
		} else modality(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0020|0011", image)) {
#ifdef DEBUG
			std::cerr << "aseries_number read failed" << std::endl;
#endif
			series_number(NULL);
		} else series_number(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0020|000e", image)) {
#ifdef DEBUG
			std::cerr << "aseries_instance_uid read failed" << std::endl;
#endif
			series_instance_uid(NULL);
		} else series_instance_uid(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0018|0015", image)) {
#ifdef DEBUG
			std::cerr << "abody_part_examined read failed" << std::endl;
#endif
			body_part_examined(NULL);
		} else body_part_examined(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0018|5101", image)) {
#ifdef DEBUG
			std::cerr << "aview_position read failed" << std::endl;
#endif
			view_position(NULL);
		} else view_position(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0021", image)) {
#ifdef DEBUG
			std::cerr << "aseries_date read failed" << std::endl;
#endif
			series_date(NULL);
		} else series_date(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0031", image)) {
#ifdef DEBUG
			std::cerr << "aseries_time read failed" << std::endl;
#endif
			series_time(NULL);
		} else series_time(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0090", image)) {
#ifdef DEBUG
			std::cerr << "areferring_physicians_name read failed" << std::endl;
#endif
			referring_physicians_name(NULL);
		} else referring_physicians_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|1050", image)) {
#ifdef DEBUG
			std::cerr << "aperforming_physicians_name read failed" << std::endl;
#endif
			performing_physicians_name(NULL);
		} else performing_physicians_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0018|1030", image)) {
#ifdef DEBUG
			std::cerr << "aprotocol_name read failed" << std::endl;
#endif
			protocol_name(NULL);
		} else protocol_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|103e", image)) {
#ifdef DEBUG
			std::cerr << "aseries_description read failed" << std::endl;
#endif
			series_description(NULL);
		} else series_description(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|1070", image)) {
#ifdef DEBUG
			std::cerr << "aoperators_name read failed" << std::endl;
#endif
			operators_name(NULL);
		} else operators_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0018|5100", image)) {
#ifdef DEBUG
			std::cerr << "apatient_position read failed" << std::endl;
#endif
			patient_position(NULL);
		} else patient_position(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0070", image)) {
#ifdef DEBUG
			std::cerr << "amanufacturer read failed" << std::endl;
#endif
			manufacturer(NULL);
		} else manufacturer(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|0080", image)) {
#ifdef DEBUG
			std::cerr << "ainstitution_name read failed" << std::endl;
#endif
			institution_name(NULL);
		} else institution_name(buffer.c_str());

		if(!_get_image_meta_data_element(buffer, "0008|1090", image)) {
#ifdef DEBUG
			std::cerr << "amanufacturers_model_name read failed" << std::endl;
#endif
			manufacturers_model_name(NULL);
		} else manufacturers_model_name(buffer.c_str());
	}

	///Collects specific image data for DICOM image
	void _collect_dicom_image_data(const ImageType::Pointer& image, int index, const char* filename)
	{
		float arow_pixel_spacing = 0, acolumn_pixel_spacing = 0, aslice_thickness = 0,
			aslice_location = 0;
		double intercept = 0, slope = 1;
		std::string akvp, aexposure_time, axray_tube_current, aexposure,
			areconstruction_diameter, instance_number;
		int width, height, bits_per_pixel = sizeof(short)*8;

		//gets row and width pixel spacing
		arow_pixel_spacing = float(image->getSpacing()[0]);
		acolumn_pixel_spacing = float(image->getSpacing()[1]);
		width = image->getSize()[0];
		height = image->getSize()[1];

		std::string buffer;
		//get slice thickness
		if(!_get_image_meta_data_element(buffer, "0018|0050", image)) {
#ifdef DEBUG
			std::cerr << "aslice_thickness read failed" << std::endl;
#endif
		} else aslice_thickness = atof(buffer.c_str()); 
		//get slice location
		if(!_get_image_meta_data_element(buffer, "0020|1041", image)) {
#ifdef DEBUG
			std::cerr << "aslice_location read failed" << std::endl;
#endif
		} else aslice_location = atof(buffer.c_str());

/* The following part is not needed as it is already handled internally by the ITK based dicom reader
		//get intercept
		if(!_get_image_meta_data_element(buffer, "0028|1052", image)) {
#ifdef DEBUG
			std::cerr << "intercept read failed" << std::endl;
#endif
		} else intercept = atof(buffer.c_str());
		//get slope
		if(!_get_image_meta_data_element(buffer, "0028|1053", image)) {
#ifdef DEBUG
			std::cerr << "slope read failed" << std::endl;
#endif
		} else slope = atof(buffer.c_str());
		//get bits per pixel
		if(!_get_image_meta_data_element(buffer, "0028|0101", image)) {
#ifdef DEBUG
			std::cerr << "bits_per_pixel read failed" << std::endl;
#endif
		} else bits_per_pixel = atoi(buffer.c_str());
*/
		//get kvp
		if(!_get_image_meta_data_element(buffer, "0018|0060", image)) {
#ifdef DEBUG
			std::cerr << "akvp read failed" << std::endl;
#endif
		} else akvp = buffer;
		//get xray tube current
		if(!_get_image_meta_data_element(buffer, "0018|1151", image)) {
#ifdef DEBUG
			std::cerr << "axray_tube_current read failed" << std::endl;
#endif
		} else axray_tube_current = buffer;
		//get exposure
		if(!_get_image_meta_data_element(buffer, "0018|1152", image)) {
#ifdef DEBUG
			std::cerr << "aexposure read failed" << std::endl;
#endif
		} else aexposure = buffer;
		// get exposure time
		if(!_get_image_meta_data_element(buffer, "0018|1150", image)) {
#ifdef DEBUG
			std::cerr << "aexposure_time read failed" << std::endl;
#endif
		} else aexposure_time = buffer;
		//get reconstruction diameter
		if(!_get_image_meta_data_element(buffer, "0018|1100", image)) {
#ifdef DEBUG
			std::cerr << "areconstruction_diameter read failed" << std::endl;
#endif
		} else areconstruction_diameter = buffer;
		//get instance number
		if(!_get_image_meta_data_element(buffer, "0020|0013", image)) {
#ifdef DEBUG
			std::cerr << "instance_number read failed" << std::endl;
#endif
		} else instance_number = buffer;

		//Creating the image
		_images[index] = new Image(atoi(instance_number.c_str()), width, height, bits_per_pixel, filename, 0, (float)slope, (float)intercept);
		_images[index]->unsafe_pixel_data() = image->getBuffer()->getPointer();
		image->getBuffer()->dropOwnership(); //dropping ownership of image in the temporary buffer

		//Set MedicalImageSequence variables
		row_pixel_spacing(index, arow_pixel_spacing);
		column_pixel_spacing(index, acolumn_pixel_spacing);
		slice_thickness(index, aslice_thickness);
		slice_location(index, aslice_location);
		kvp(index, akvp.c_str());
		exposure_time(index, aexposure_time.c_str());
		xray_tube_current(index, axray_tube_current.c_str());
		exposure(index, aexposure.c_str());
		reconstruction_diameter(index, areconstruction_diameter.c_str());

		// Make sure images are ordered by instance number
		// Added by Matt on 6/24/03
		/*
		if (index>0) {
			Image* image_hold = _images[index];
			int j = index-1;
			while ((j>=0) && (image_hold->instance_number()<_images[j]->instance_number())) {
				_images[j+1] = _images[j];
				row_pixel_spacing(j+1, row_pixel_spacing(j));
				column_pixel_spacing(j+1, column_pixel_spacing(j));
				slice_thickness(j+1, slice_thickness(j));
				slice_location(j+1, slice_location(j));
				kvp(j+1, kvp(j));
				exposure_time(j+1, exposure_time(j));
				xray_tube_current(j+1, xray_tube_current(j));
				exposure(j+1, exposure(j));
				reconstruction_diameter(j+1, reconstruction_diameter(j));
				j--;
			}
			_images[j+1] = image_hold;
			row_pixel_spacing(j+1, arow_pixel_spacing);
			column_pixel_spacing(j+1, acolumn_pixel_spacing);
			slice_thickness(j+1, aslice_thickness);
			slice_location(j+1, aslice_location);
			kvp(j+1, akvp.c_str());
			exposure_time(j+1, aexposure_time.c_str());
			xray_tube_current(j+1, axray_tube_current.c_str());
			exposure(j+1, aexposure.c_str());
			reconstruction_diameter(j+1, areconstruction_diameter.c_str());
		}
		*/
	}
};

#endif
