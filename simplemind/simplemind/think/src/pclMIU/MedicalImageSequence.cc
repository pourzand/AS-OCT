#include "MedicalImageSequence.h"
#include "Exception.h"
#include <string>

#define PROGNAME "MedicalImageSequence.cc"

extern "C" {
#include "assert.h"  
#include "ucla_v5b_mamo.h"
}

using std::cout;
using std::endl;

MedicalImageSequence::MedicalImageSequence()
  : ImageSequence(), _patient_name(NULL), _patient_id(NULL),
    _patient_birth_date(NULL), _patient_birth_time(NULL), 
    _patient_sex(NULL), _study_date(NULL), _study_time(NULL), 
    _accession_number(NULL), _study_id(NULL),
    _study_instance_uid(NULL), _referring_physicians_name(NULL),
    _study_description(NULL), _patient_age(NULL), _patient_size(NULL), 
    _patient_weight(NULL), _modality(NULL), _series_number(NULL), 
    _series_instance_uid(NULL), _body_part_examined(NULL), 
    _view_position(NULL), _series_date(NULL), _series_time(NULL), 
    _performing_physicians_name(NULL), _protocol_name(NULL), 
    _series_description(NULL), _operators_name(NULL), 
    _patient_position(NULL), _manufacturer(NULL), _institution_name(NULL), 
    _manufacturers_model_name(NULL), _image_plane(NULL),
    _short_desc(0), _long_desc(0), _hu_values_column_major(0)
{

  _short_desc = new char[1];
  _long_desc = new char[1];

  // added by Kelvin
  _init_PACS_SUBHEADER_MAMO();

  _rov = new char[1];
  _rov[0] = '\0';
  _angle_tilted = new char[1];
  _angle_tilted[0] = '\0';
  _inteliWin = 0.0;
  _inteliLevel = 0.0;
  _access_count = 0;
  
};

MedicalImageSequence::MedicalImageSequence(const int num_images, 
					   const int width, const int height,
					   const int bits_per_pixel, 
					   const char** const file_names)
  : ImageSequence(num_images, width, height, bits_per_pixel,
		  file_names), _patient_name(NULL), _patient_id(NULL),
    _patient_birth_date(NULL), _patient_birth_time(NULL), _patient_sex(NULL),
    _study_date(NULL), _study_time(NULL), _accession_number(NULL),
    _study_id(NULL), _study_instance_uid(NULL), 
    _referring_physicians_name(NULL),
    _study_description(NULL), _patient_age(NULL), _patient_size(NULL), 
    _patient_weight(NULL), _modality(NULL), _series_number(NULL), 
    _series_instance_uid(NULL), _body_part_examined(NULL), 
    _view_position(NULL),
    _series_date(NULL), _series_time(NULL), _performing_physicians_name(NULL),
    _protocol_name(NULL), _series_description(NULL), _operators_name(NULL), 
    _patient_position(NULL), _manufacturer(NULL), _institution_name(NULL), 
    _manufacturers_model_name(NULL), _image_plane(NULL),
    _short_desc(0), _long_desc(0), _hu_values_column_major(0)
{
  //cout << "MedicalImageSequence -> Not a default CONSTRUCTOR CALLED!" << endl;
  
  _patient_name = new char[1];
  _patient_name[0] = '\0';
  
  _patient_id = new char[1];
  _patient_id[0] = '\0';
  
  _patient_birth_date = new char[1];
  _patient_birth_date[0] = '\0';
  
  _patient_birth_time = new char[1];
  _patient_birth_time[0] = '\0';
  
  _patient_sex = new char[1];
  _patient_sex[0] = '\0';

  _study_date = new char[1];
  _study_date[0] = '\0';
  
  _study_time = new char[1];
  _study_time[0] = '\0';

  _accession_number = new char[1];
  _accession_number[0] = '\0';
  
  _study_id = new char[1];
  _study_id[0] = '\0';
  
  _study_instance_uid = new char[1];
  _study_instance_uid[0] = '\0';
  
  _patient_age = new char[1];
  _patient_age[0] = '\0';
  
  _patient_size = new char[1];
  _patient_size[0] = '\0';

  _patient_weight = new char[1];
  _patient_weight[0] = '\0';
  
  _referring_physicians_name = new char[1];
  _referring_physicians_name[0] = '\0';
  
  _study_description = new char[1];
  _study_description[0] = '\0';
  
  _modality = new char[1];
  _modality[0] = '\0';
  
  _series_number = new char[1];
  _series_number[0] = '\0';
  
  _series_instance_uid = new char[1];
  _series_instance_uid[0] = '\0';
  
  _body_part_examined = new char[1];
  _body_part_examined[0] = '\0';

  _view_position = new char[1];
  _view_position[0] = '\0';
  
  _series_date = new char[1];
  _series_date[0] = '\0';

  _series_time = new char[1];
  _series_time[0] = '\0';
  
  _performing_physicians_name = new char[1];
  _performing_physicians_name[0] = '\0';
  
  _protocol_name = new char[1];
  _protocol_name[0] = '\0';
  
  _series_description = new char[1];
  _series_description[0] = '\0';
  
  _operators_name = new char[1];
  _operators_name[0] = '\0';

  _patient_position = new char[1];
  _patient_position[0] = '\0';

  _manufacturer = new char[1];
  _manufacturer[0] = '\0';
  
  _institution_name = new char[1];
  _institution_name[0] = '\0';

  _manufacturers_model_name = new char[1];
  _manufacturers_model_name[0] = '\0';
  
  //Added by BW 090997 to init image planes (for checkpoint of dmtk) 
  _init_image_planes();
  
  _short_desc = new char[1];
  _long_desc = new char[1];
  
  _create_descriptions();
  
// Added by Kelvin 100997 to init PACS_SUBHEADER_MAMO
  _init_PACS_SUBHEADER_MAMO();
  _rov = new char[1];
  _rov[0] = '\0';
  _angle_tilted = new char[1];
  _angle_tilted[0] = '\0';
  _inteliWin = 0.0;
  _inteliLevel = 0.0;
  _access_count = 0;
};


MedicalImageSequence::MedicalImageSequence(const int num_images, 
					   Image** images)
  : ImageSequence(num_images, images), _patient_name(NULL), _patient_id(NULL),
    _patient_birth_date(NULL), _patient_birth_time(NULL), _patient_sex(NULL),
    _study_date(NULL), _study_time(NULL), _accession_number(NULL),
    _study_id(NULL), _study_instance_uid(NULL), 
    _referring_physicians_name(NULL),
    _study_description(NULL), _patient_age(NULL), _patient_size(NULL), 
    _patient_weight(NULL), _modality(NULL), _series_number(NULL), 
    _series_instance_uid(NULL), _body_part_examined(NULL), 
    _view_position(NULL),
    _series_date(NULL), _series_time(NULL), _performing_physicians_name(NULL),
    _protocol_name(NULL), _series_description(NULL), _operators_name(NULL), 
    _patient_position(NULL), _manufacturer(NULL), _institution_name(NULL), 
    _manufacturers_model_name(NULL), _image_plane(NULL),
    _short_desc(0), _long_desc(0), _hu_values_column_major(0)
{
_patient_name = new char[1];
_patient_name[0] = 0;

_patient_id = new char[1];
_patient_id[0] = 0;

_patient_birth_date = new char[1];
_patient_birth_date[0] = 0;

_patient_birth_time = new char[1];
_patient_birth_time[0] = 0;

_patient_sex = new char[1];
_patient_sex[0] = 0;

_study_date = new char[1];
_study_date[0] = 0;

_study_time = new char[1];
_study_time[0] = 0;

_accession_number = new char[1];
_accession_number[0] = 0;

_study_id = new char[1];
_study_id[0] = 0;

_study_instance_uid = new char[1];
_study_instance_uid[0] = 0;

_patient_age = new char[1];
_patient_age[0] = 0;

_patient_size = new char[1];
_patient_size[0] = 0;

_patient_weight = new char[1];
_patient_weight[0] = 0;

_referring_physicians_name = new char[1];
_referring_physicians_name[0] = 0;

_study_description = new char[1];
_study_description[0] = 0;

_modality = new char[1];
_modality[0] = 0;

_series_number = new char[1];
_series_number[0] = 0;

_series_instance_uid = new char[1];
_series_instance_uid[0] = 0;

_body_part_examined = new char[1];
_body_part_examined[0] = 0;

_view_position = new char[1];
_view_position[0] = 0;

_series_date = new char[1];
_series_date[0] = 0;

_series_time = new char[1];
_series_time[0] = 0;

_performing_physicians_name = new char[1];
_performing_physicians_name[0] = 0;

_protocol_name = new char[1];
_protocol_name[0] = 0;

_series_description = new char[1];
_series_description[0] = 0;

_operators_name = new char[1];
_operators_name[0] = 0;

_patient_position = new char[1];
_patient_position[0] = 0;

_manufacturer = new char[1];
_manufacturer[0] = 0;

_institution_name = new char[1];
_institution_name[0] = 0;

_manufacturers_model_name = new char[1];
_manufacturers_model_name[0] = 0;

_init_image_planes();

_short_desc = new char[1];
_long_desc = new char[1];

_create_descriptions();

};


//Added by BW for dmtk
MedicalImageSequence::MedicalImageSequence(const MedicalImageSequence& mis) 
  : _patient_name(NULL), _patient_id(NULL), _patient_birth_date(NULL), 
    _patient_birth_time(NULL), _patient_sex(NULL), _study_date(NULL), 
    _study_time(NULL), _accession_number(NULL), _study_id(NULL), 
    _study_instance_uid(NULL), _referring_physicians_name(NULL),
    _study_description(NULL), _patient_age(NULL), _patient_size(NULL), 
    _patient_weight(NULL), _modality(NULL), _series_number(NULL), 
    _series_instance_uid(NULL), _body_part_examined(NULL), 
    _view_position(NULL), _series_date(NULL), _series_time(NULL), 
    _performing_physicians_name(NULL), _protocol_name(NULL), 
    _series_description(NULL), _operators_name(NULL), 
    _patient_position(NULL), _manufacturer(NULL), _institution_name(NULL), 
    _manufacturers_model_name(NULL), _image_plane(NULL),
    _short_desc(0), _long_desc(0), _hu_values_column_major(0) 
{
  //Initialize ImageSequence variables for a consistent state when using =
  _num_images = 0;
  _images = NULL;
  //Init image planes if image_plane is NULL, otherwise nothing will happen
  _init_image_planes();
  
  // Added by Kelvin 100997 to init PACS_SUBHEADER_MAMO
  _init_PACS_SUBHEADER_MAMO();
  _rov = new char[1];
  _rov[0] = '\0';
  _angle_tilted = new char[1];
  _angle_tilted[0] = '\0';
  _inteliWin = 0.0;
  _inteliLevel = 0.0;
  _access_count = 0;  
  
  *this = mis;
}

MedicalImageSequence::~MedicalImageSequence()
{
  int i, n=num_images();

  delete [] _patient_name;
  delete [] _patient_id;
  delete [] _patient_birth_date;
  delete [] _patient_birth_time;
  delete [] _patient_sex;
  delete [] _study_date;
  delete [] _study_time;
  delete [] _accession_number;
  delete [] _study_id;
  delete [] _study_instance_uid;
  delete [] _referring_physicians_name;
  delete [] _study_description;
  delete [] _patient_age;
  delete [] _patient_size;
  delete [] _patient_weight;
  delete [] _modality;
  delete [] _series_number;
  delete [] _series_instance_uid;
  delete [] _body_part_examined;
  delete [] _view_position;
  delete [] _series_date;
  delete [] _series_time;
  delete [] _performing_physicians_name;
  delete [] _protocol_name;
  delete [] _series_description;
  delete [] _operators_name;
  delete [] _patient_position;
  delete [] _manufacturer;
  delete [] _institution_name;
  delete [] _manufacturers_model_name;

  if (_image_plane) {
    for(i=0; i<n; i++) {
      /*for(i = 0; i < _num_images; i++) {*/
      delete [] _image_plane[i]._kvp;
      delete [] _image_plane[i]._exposure_time;
      delete [] _image_plane[i]._xray_tube_current;
      delete [] _image_plane[i]._exposure;
      delete [] _image_plane[i]._reconstruction_diameter;
    }
  }
  delete [] _image_plane;

  //Added by BW (for Kelvin) 101097
  delete [] _rov;
  delete [] _angle_tilted;
  //End BW

  delete [] _short_desc;
  delete [] _long_desc;
  
  if (_hu_values_column_major) delete [] _hu_values_column_major;
};

const MedicalImageSequence& MedicalImageSequence::
operator=(const MedicalImageSequence & Rhs)
{
  if(&Rhs != this) {
    //Init image planes if image_plane is NULL, otherwise nothing will happen
    _init_image_planes();
    
    //Get rid of old imageplane array and strings
    int i;

    for(i = 0; i < _num_images; i++) {
      delete [] _image_plane[i]._kvp;
      delete [] _image_plane[i]._exposure_time;
      delete [] _image_plane[i]._xray_tube_current;
      delete [] _image_plane[i]._exposure;
      delete [] _image_plane[i]._reconstruction_diameter;
    }
    delete [] _image_plane;

    //In _init_image_plane(), checks if _image_plane is NULL
    _image_plane = NULL; 
    
    //Copy base class info 
    (ImageSequence&) *this = Rhs;

    //Copy MedicalImageSequence attributes
    patient_name(Rhs._patient_name);
    patient_id(Rhs._patient_id);
    patient_birth_date(Rhs._patient_birth_date);
    patient_birth_time(Rhs._patient_birth_time);
    patient_sex(Rhs._patient_sex);
    study_date(Rhs._study_date);
    study_time(Rhs._study_time);
    accession_number(Rhs._accession_number);
    study_id(Rhs._study_id);
    study_instance_uid(Rhs._study_instance_uid);
    referring_physicians_name(Rhs._referring_physicians_name);
    study_description(Rhs._study_description);
    patient_age(Rhs._patient_age);
    patient_size(Rhs._patient_size);
    patient_weight(Rhs._patient_weight);
    modality(Rhs._modality);
    series_number(Rhs._series_number);
    series_instance_uid(Rhs._series_instance_uid);
    body_part_examined(Rhs._body_part_examined);
    view_position(Rhs._view_position);
    series_date(Rhs._series_date);
    series_time(Rhs._series_time);
    performing_physicians_name(Rhs._performing_physicians_name);
    protocol_name(Rhs._protocol_name);
    series_description(Rhs._series_description);
    operators_name(Rhs._operators_name);
    patient_position(Rhs._patient_position);
    manufacturer(Rhs._manufacturer);
    institution_name(Rhs._institution_name);
    manufacturers_model_name(Rhs._manufacturers_model_name);
    
    //Now copy in image plane stuff
    _num_images = Rhs._num_images;
    _init_image_planes();
    for(i = 0; i < _num_images; i++) {
      row_pixel_spacing(i, Rhs._image_plane[i]._row_pixel_spacing);
      column_pixel_spacing(i,  Rhs._image_plane[i]._column_pixel_spacing);
      slice_thickness(i, Rhs._image_plane[i]._slice_thickness);
      slice_location(i, Rhs._image_plane[i]._slice_location);
          
      kvp(i, Rhs._image_plane[i]._kvp);
      exposure_time(i, Rhs._image_plane[i]._exposure_time);
      xray_tube_current(i, Rhs._image_plane[i]._xray_tube_current);
      exposure(i, Rhs._image_plane[i]._exposure);
      reconstruction_diameter(i, Rhs._image_plane[i]._reconstruction_diameter);
    };
  };

  // added by Kelvin 
  // Now copy PACS_SUBHEADER_MAMO stuff
  _init_PACS_SUBHEADER_MAMO();
  int i;
  for (i = 0; i < ASSOC_FILE; i++) 
    strcpy(mamo.assoc_file.name[i], Rhs.mamo.assoc_file.name[i]);
	 
  for ( i = 0; i < MAX_ACCESS; i++) {
    strcpy(mamo.access[i].date, Rhs.mamo.access[i].date);
    strcpy(mamo.access[i].phy_name, Rhs.mamo.access[i].phy_name);
    strcpy(mamo.access[i].phy_id, Rhs.mamo.access[i].phy_id);
    strcpy(mamo.access[i].birad_code, Rhs.mamo.access[i].birad_code);
    for (int j = 0; j < MAX_MARK; j++) {
      mamo.access[i].marks[j].x = Rhs.mamo.access[i].marks[j].x;
      mamo.access[i].marks[j].y = Rhs.mamo.access[i].marks[j].y;
    };
    
    mamo.access[i].marks_count = Rhs.mamo.access[i].marks_count;
    strcpy(mamo.access[i].t_code.file, Rhs.mamo.access[i].t_code.file);
    strcpy(mamo.access[i].t_code.dbname, Rhs.mamo.access[i].t_code.dbname);
    strcpy(mamo.access[i].t_code.dbtable, Rhs.mamo.access[i].t_code.dbtable);
		
  }
	 
  rov(Rhs._rov);
  angle_tilt(Rhs._angle_tilted);
  inteliWin(Rhs._inteliWin);
  inteliLevel(Rhs._inteliLevel);
  _access_count = Rhs._access_count;
	 
  // ..........  ended by Kelvin 
	 
  delete [] _short_desc;
  delete [] _long_desc;
  _create_descriptions();
	 
  return *this;
}

void check_null_and_print(const char* t, const char* s)
{
  cout << t << ": ";
  if (s) { 
    cout << s; 
  }
  else { 
    cout << "undefined"; 
  };
  cout << "\n";
}

void check_minus1_and_print(const char* t, const float f)
{
  cout << t << ": ";
  if (f!=-1.0) {
    cout << f;
  }
  else {
    cout << "undefined";
  };
  cout << "\n";
}

void MedicalImageSequence::print_parameters()
{
  int i,j, n=num_images();

  cout << "IMAGING  PARAMETERS\n\n";
  check_null_and_print("Patient name", _patient_name);
  check_null_and_print("Patient ID", _patient_id);
  check_null_and_print("Patient birth date", _patient_birth_date);
  check_null_and_print("Patient sex", _patient_sex);
  check_null_and_print("Study date", _study_date);
  check_null_and_print("Study time", _study_time);
  check_null_and_print("Referring physician's name", 
		       _referring_physicians_name);
  check_null_and_print("Study description", _study_description);
  check_null_and_print("Modality", _modality);
  check_null_and_print("Series date", _series_date);
  check_null_and_print("Series time", _series_time);
  check_null_and_print("Performing physician's name", 
		       _performing_physicians_name);
  check_null_and_print("Protocol name", _protocol_name);
  check_null_and_print("Series description", _series_description);
  check_null_and_print("Operator's name", _operators_name);
  check_null_and_print("Patient position", _patient_position);
  check_null_and_print("Manufacturer", _manufacturer);
  check_null_and_print("Institution name", _institution_name);
  check_null_and_print("Manufacturers model name", 
		       _manufacturers_model_name);

  cout << "\n";
  if (_image_plane) {
    for(i=0; i<n; i++) {
      cout << "Image #" << i << "\n";
      check_minus1_and_print("Row pixel spacing (mm)", 
			     _image_plane[i]._row_pixel_spacing);
      check_minus1_and_print("Column pixel spacing (mm)", 
			     _image_plane[i]._column_pixel_spacing);
      check_minus1_and_print("Slice thickness (mm)", 
			     _image_plane[i]._slice_thickness);
      check_minus1_and_print("Slice location (mm)", 
			     _image_plane[i]._slice_location);
      check_null_and_print("KVp", _image_plane[i]._kvp);
      check_null_and_print("Exposure time", _image_plane[i]._exposure_time);
      check_null_and_print("X-ray tube current", 
			   _image_plane[i]._xray_tube_current);
      check_null_and_print("Exposure", _image_plane[i]._exposure);
      check_null_and_print("Reconstruction diameter", 
			   _image_plane[i]._reconstruction_diameter);
    }
  }

  cout << "Mamo related\n";

  
  for (i = 0; i < ASSOC_FILE; i++) {
    cout << "(" << i << ")";
    check_null_and_print("assoc files:",  mamo.assoc_file.name[i]);
  }

  for ( i = 0; i < MAX_ACCESS; i++) {
    cout << endl;
    cout <<"(" << i << ") ";
    check_null_and_print("access date", mamo.access[i].date);
    cout <<"(" << i << ") ";
    check_null_and_print("phy_name", mamo.access[i].phy_name);
    cout <<"(" << i << ") ";
    check_null_and_print("phy_id", mamo.access[i].phy_id);
    cout <<"(" << i << ") ";
    check_null_and_print("birad_code", mamo.access[i].birad_code);
    cout <<endl;

    for (j = 0; j < MAX_MARK; j++) {
      cout << "access: " << i << " marker: " << j << " " 
	   << "(x=" << mamo.access[i].marks[j].x 
	   << ", y="<< mamo.access[i].marks[j].y << ")" << endl;
    }

    cout << "access: " << i << " marks_count: " << mamo.access[i].marks_count << endl;
    check_null_and_print("t_code file: ", mamo.access[i].t_code.file);
    check_null_and_print("t_code dbname: ", mamo.access[i].t_code.dbname);
    check_null_and_print("t_code dbtable: ", mamo.access[i].t_code.dbtable);
  }
  
  cout << endl;;
  check_null_and_print("rov: ", _rov);
  check_null_and_print("angle_tilted: ", _angle_tilted);
  cout << "inteliWin =" << _inteliWin << endl;
  cout << "inteliWin =" <<  _inteliWin << endl;
  cout << "access_count =" << _access_count << endl;


};


void char_alloc_and_copy(char*& new_str, const char* const old_str)
{

  delete [] new_str;

  if (old_str) {

    int sl = strlen(old_str);

    for (; sl > 0 && old_str[sl-1] == ' '; sl--);

    if (sl > 0) {
      new_str = new char [sl+1];
      strncpy(new_str, old_str, sl);
      new_str[sl] = '\0';
    }
    else {
      //cout << "String length must be 0" << endl;
      new_str = new char[1];
      new_str[0] = '\0';
    };
  }
  else {
    new_str = new char[1];
    new_str[0] = '\0';
  };
}

const char* const MedicalImageSequence::patient_name() const
{
  return _patient_name;
}


void MedicalImageSequence::patient_name(const char* const new_value)
{
  char_alloc_and_copy(_patient_name, new_value);
}


const char* const MedicalImageSequence::patient_id() const
{
  return _patient_id;
}

void MedicalImageSequence::patient_id(const char* const new_value)
{
  char_alloc_and_copy(_patient_id, new_value);
}


const char* const MedicalImageSequence::patient_birth_date() const
{
  return _patient_birth_date;
}

void MedicalImageSequence::patient_birth_date(const char* const new_value)
{
  char_alloc_and_copy(_patient_birth_date, new_value);
}

const char* const MedicalImageSequence::patient_birth_time() const 
{
  return _patient_birth_time;
}

void MedicalImageSequence::patient_birth_time(const char* const new_value)
{
  char_alloc_and_copy(_patient_birth_time, new_value);
}

const char* const MedicalImageSequence::patient_sex() const
{
  return _patient_sex;
}

void MedicalImageSequence::patient_sex(const char* const new_value)
{
  char_alloc_and_copy(_patient_sex, new_value);
}


const char* const MedicalImageSequence::study_date() const
{
  return _study_date;
}

void MedicalImageSequence::study_date(const char* const new_value)
{
  char_alloc_and_copy(_study_date, new_value);
}


const char* const MedicalImageSequence::study_time() const
{
  return _study_time;
}

void MedicalImageSequence::study_time(const char* const new_value)
{
  char_alloc_and_copy(_study_time, new_value);
}

const char* const MedicalImageSequence::accession_number() const
{
  return _accession_number;
}

void MedicalImageSequence::accession_number(const char* const new_value)
{
  char_alloc_and_copy(_accession_number, new_value);
}

const char* const MedicalImageSequence::study_id() const
{
  return _study_id;
}

void MedicalImageSequence::study_id(const char* const new_value) 
{
  char_alloc_and_copy(_study_id, new_value);
}

const char* const MedicalImageSequence::study_instance_uid() const
{
  return _study_instance_uid;
}

void MedicalImageSequence::study_instance_uid(const char* const new_value)
{
  char_alloc_and_copy(_study_instance_uid, new_value);
}

const char* const MedicalImageSequence::referring_physicians_name() const
{
  return _referring_physicians_name;
}

void 
MedicalImageSequence::referring_physicians_name(const char* const new_value)
{
  char_alloc_and_copy(_referring_physicians_name, new_value);
}

const char* const MedicalImageSequence::study_description() const
{
  return _study_description;
}

void MedicalImageSequence::study_description(const char* const new_value)
{
  char_alloc_and_copy(_study_description, new_value);
}

const char* const MedicalImageSequence::patient_age() const
{
  return _patient_age;
}

void MedicalImageSequence::patient_age(const char* const new_value)
{
  char_alloc_and_copy(_patient_age, new_value);
}

const char* const MedicalImageSequence::patient_size() const
{
  return _patient_size;
}

void MedicalImageSequence::patient_size(const char* const new_value)
{
  char_alloc_and_copy(_patient_size, new_value);
}

const char* const MedicalImageSequence::patient_weight() const
{
  return _patient_weight;
}

void MedicalImageSequence::patient_weight(const char* const new_value)
{
  char_alloc_and_copy(_patient_weight, new_value);
}

const char* const MedicalImageSequence::modality() const
{
  return _modality;
}

void MedicalImageSequence::modality(const char* const new_value)
{
  char_alloc_and_copy(_modality, new_value);
}

const char* const MedicalImageSequence::series_number() const
{
  return _series_number;
}

void MedicalImageSequence::series_number(const char* const new_value)
{
  char_alloc_and_copy(_series_number, new_value);
}

const char* const MedicalImageSequence::series_instance_uid() const
{
  return _series_instance_uid;
}

void MedicalImageSequence::series_instance_uid(const char* const new_value)
{
  char_alloc_and_copy(_series_instance_uid, new_value);
}

const char* const MedicalImageSequence::body_part_examined() const
{
  return _body_part_examined;
}

void MedicalImageSequence::body_part_examined(const char* const new_value)
{
  char_alloc_and_copy(_body_part_examined, new_value);
}

const char* const MedicalImageSequence::view_position() const
{
  return _view_position;
}

void MedicalImageSequence::view_position(const char* const new_value)
{
  char_alloc_and_copy(_view_position, new_value);
}

const char* const MedicalImageSequence::series_date() const
{
  return _series_date;
}

void MedicalImageSequence::series_date(const char* const new_value)
{
  char_alloc_and_copy(_series_date, new_value);
}


const char* const MedicalImageSequence::series_time() const
{
  return _series_time;
}

void MedicalImageSequence::series_time(const char* const new_value)
{
  char_alloc_and_copy(_series_time, new_value);
}

const char* const MedicalImageSequence::performing_physicians_name() const
{
  return _performing_physicians_name;
}

void MedicalImageSequence::performing_physicians_name(const char* 
						      const new_value)
{
  char_alloc_and_copy(_performing_physicians_name, new_value);
}


const char* const MedicalImageSequence::protocol_name() const
{
  return _protocol_name;
}

void MedicalImageSequence::protocol_name(const char* const new_value)
{
  char_alloc_and_copy(_protocol_name, new_value);
}


const char* const MedicalImageSequence::series_description() const
{
  return _series_description;
}

void MedicalImageSequence::series_description(const char* const new_value)
{
  char_alloc_and_copy(_series_description, new_value);
}


const char* const MedicalImageSequence::operators_name() const
{
  return _operators_name;
}

void MedicalImageSequence::operators_name(const char* const new_value)
{
  char_alloc_and_copy(_operators_name, new_value);
}

const char* const MedicalImageSequence::patient_position() const
{
  return _patient_position;
}

void MedicalImageSequence::patient_position(const char* const new_value)
{
  char_alloc_and_copy(_patient_position, new_value);
}


const char* const MedicalImageSequence::manufacturer() const
{
  return _manufacturer;
}

void MedicalImageSequence::manufacturer(const char* const new_value)
{
  char_alloc_and_copy(_manufacturer, new_value);
}

const char* const MedicalImageSequence::institution_name() const
{
  return _institution_name;
}

void MedicalImageSequence::institution_name(const char* const new_value)
{
  char_alloc_and_copy(_institution_name, new_value);
}


const char* const MedicalImageSequence::manufacturers_model_name() const
{
  return _manufacturers_model_name;
}

void MedicalImageSequence::manufacturers_model_name(const char* const 
						    new_value)
{
  char_alloc_and_copy(_manufacturers_model_name, new_value);
}

const char* const MedicalImageSequence::kvp(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._kvp;
  }
  else {
    return NULL;
  };
}

void MedicalImageSequence::kvp(const int z, const char* const new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    char_alloc_and_copy(_image_plane[z]._kvp, new_value);
  };
};


const char* const MedicalImageSequence::exposure_time(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._exposure_time; 
  }
  else {
    return NULL;
  };
};

void MedicalImageSequence::exposure_time(const int z, 
					 const char* const new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    char_alloc_and_copy(_image_plane[z]._exposure_time, new_value);
  };
};

const char* const MedicalImageSequence::xray_tube_current(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._xray_tube_current;
  }
  else {
    return NULL;
  };
};

void MedicalImageSequence::xray_tube_current(const int z, 
					     const char* const new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    char_alloc_and_copy(_image_plane[z]._xray_tube_current, new_value);
  };
};

const char* const MedicalImageSequence::exposure(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._exposure;
  }
  else {
    return NULL;
  };
};

void MedicalImageSequence::exposure(const int z, const char* const new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    char_alloc_and_copy(_image_plane[z]._exposure, new_value);
  };
};

const char* const 
MedicalImageSequence::reconstruction_diameter(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._reconstruction_diameter;
  }
  else {
    return NULL;
  };
};

void MedicalImageSequence::reconstruction_diameter(const int z, 
						   const char* const
						   new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    char_alloc_and_copy(_image_plane[z]._reconstruction_diameter, new_value);
  };
};

const float MedicalImageSequence::row_pixel_spacing(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._row_pixel_spacing;
  }
  else {
    return -1.0;
  };
};

void MedicalImageSequence::row_pixel_spacing(const int z, 
					     const float new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    _image_plane[z]._row_pixel_spacing = new_value;
  };
};

const float MedicalImageSequence::column_pixel_spacing(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._column_pixel_spacing;
  }
  else {
    return -1.0;
  };
};

void MedicalImageSequence::column_pixel_spacing(const int z, 
						const float new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    _image_plane[z]._column_pixel_spacing = new_value;
  };
};

const float MedicalImageSequence::slice_thickness(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._slice_thickness;
  }
  else {
    return -1.0;
  };
};

void MedicalImageSequence::slice_thickness(const int z, 
					   const float new_value)
{ 
  if (_image_plane && (z>=0) && (z<_num_images)) {
    _image_plane[z]._slice_thickness = new_value;
  };
};

const float MedicalImageSequence::slice_location(const int z) const
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    return _image_plane[z]._slice_location;
  }
  else {
    return -1.0;
  };
};

void MedicalImageSequence::slice_location(const int z, 
					  const float new_value)
{
  if (_image_plane && (z>=0) && (z<_num_images)) {
    _image_plane[z]._slice_location = new_value;
  };
};


const short* const 
MedicalImageSequence::pixel_data(const int z)
{
  if (_image_plane && (z>=0) && (z<num_images())) {
    return image(z).pixel_data();
  }
  else {
    return NULL;
  };
};


const int MedicalImageSequence::xdim() const
{
  return width();
}


const int MedicalImageSequence::ydim() const
{
  return height();
}


const int MedicalImageSequence::zdim() const
{
  return num_images();
}


const short* const MedicalImageSequence::hu_values_column_major() {
	if (!_hu_values_column_major) {
		register int xdim = this->xdim();
		register int ydim = this->ydim();
		register int zdim = this->zdim();
		_hu_values_column_major = new short [xdim*ydim*zdim];
		for (int z = 0; z < zdim; z++) {
			const Image& im = image_const(z);
			register int rs = im.rescale_slope();
			register int ri = im.rescale_intercept();
			for (int y = 0; y < ydim; y++)
				for (int x = 0; x < xdim; x++)
					_hu_values_column_major[y + x * ydim + z * ydim * xdim] = (short) (fast_pix_val(x, y, z) * rs + ri);
		}
	}
	return _hu_values_column_major;
}


const short MedicalImageSequence::pix_val(const int x, 
						   const int y, 
						   const int z)
{
  if ((x>=0) && (x<xdim())&& (y>=0) && (y<ydim()) 
      && (z>=0) && (z<zdim()) && _image_plane) {
    return image(z).pixel_data()[y*xdim() + x];
  }
  else {
    return 32767;
  };
};

void MedicalImageSequence::_init_image_planes() 
{
  if(_image_plane == NULL) {
    //cerr << "INIT IMAGE PLANES CALLED" << endl;
    int i;
    _image_plane = new _ImagePlane [_num_images];
    assert(_image_plane != NULL);
    for(i=0; i<_num_images; i++) {
      _image_plane[i]._row_pixel_spacing = -1.0;
      _image_plane[i]._column_pixel_spacing = -1.0;
      _image_plane[i]._slice_thickness = -1.0;
      _image_plane[i]._slice_location = -1.0;
      
      _image_plane[i]._kvp = new char[1];
      assert(_image_plane[i]._kvp != 0);
      strcpy(_image_plane[i]._kvp, "");
      
      _image_plane[i]._exposure_time = new char[1];
      assert(_image_plane[i]._exposure_time != 0);
      strcpy(_image_plane[i]._exposure_time, "");
      
      _image_plane[i]._xray_tube_current = new char[1];
      assert(_image_plane[i]._xray_tube_current != 0);
      strcpy(_image_plane[i]._xray_tube_current, "");
      
      _image_plane[i]._exposure = new char[1];
      assert(_image_plane[i]._exposure != 0);
      strcpy(_image_plane[i]._exposure, "");
      
      _image_plane[i]._reconstruction_diameter = new char[1];
      assert(_image_plane[i]._reconstruction_diameter != 0);
      strcpy(_image_plane[i]._reconstruction_diameter, "");
    }
  }
}

ofstream& operator<<(ofstream& file, const MedicalImageSequence& mis) {
  assert(file != 0);
  
  //Store base class stuff to file first
  file << (const ImageSequence&)mis; 
  
  //Write MedicalImageSequence attributes to file 
  file << strlen(mis._patient_name) << endl << mis._patient_name << endl;
  file << strlen(mis._patient_id) << endl << mis._patient_id << endl;
  file << strlen(mis._patient_birth_date) << endl 
       << mis._patient_birth_date << endl;
  file << strlen(mis._patient_birth_time) << endl 
       << mis._patient_birth_time << endl;
  file << strlen(mis._patient_sex) << endl << mis._patient_sex << endl;
  file << strlen(mis._study_date) << endl << mis._study_date << endl;
  file << strlen(mis._study_time) << endl << mis._study_time << endl;
  file << strlen(mis._accession_number) << endl 
       << mis._accession_number << endl;
  file << strlen(mis._study_id) << endl << mis._study_id << endl;
  file << strlen(mis._study_instance_uid) << endl 
       << mis._study_instance_uid << endl;
  file << strlen(mis._referring_physicians_name) << endl 
       << mis._referring_physicians_name << endl;
  file << strlen(mis._study_description) << endl 
       << mis._study_description << endl;
  file << strlen(mis._patient_age) << endl << mis._patient_age << endl;
  file << strlen(mis._patient_size) << endl << mis._patient_size << endl;
  file << strlen(mis._patient_weight) << endl << mis._patient_weight << endl;
  file << strlen(mis._modality) << endl << mis._modality << endl;
  file << strlen(mis._series_number) << endl << mis._series_number << endl;
  file << strlen(mis._series_instance_uid) << endl 
       << mis._series_instance_uid << endl;
  file << strlen(mis._body_part_examined) << endl 
       << mis._body_part_examined << endl;
  file << strlen(mis._view_position) << endl << mis._view_position << endl;
  file << strlen(mis._series_date) << endl << mis._series_date << endl;
  file << strlen(mis._series_time) << endl << mis._series_time << endl;
  file << strlen(mis._performing_physicians_name) << endl 
       << mis._performing_physicians_name << endl;
  file << strlen(mis._protocol_name) << endl << mis._protocol_name << endl;
  file << strlen(mis._series_description) << endl 
       << mis._series_description << endl;
  file << strlen(mis._operators_name) << endl << mis._operators_name << endl;
  file << strlen(mis._patient_position) << endl 
       << mis._patient_position << endl;
  file << strlen(mis._manufacturer) << endl << mis._manufacturer << endl;
  file << strlen(mis._institution_name) << endl 
       << mis._institution_name << endl;
  file << strlen(mis._manufacturers_model_name) << endl 
       << mis._manufacturers_model_name << endl;
  
  //Write imageplane stuff to file
  for(int i = 0; i < mis._num_images; i++) {
    file << mis._image_plane[i]._row_pixel_spacing << endl;
    file << mis._image_plane[i]._column_pixel_spacing << endl;
    file << mis._image_plane[i]._slice_thickness << endl;
    file << mis._image_plane[i]._slice_location << endl;
    file << strlen(mis._image_plane[i]._kvp) << endl 
	 << mis._image_plane[i]._kvp << endl;
    file << strlen(mis._image_plane[i]._exposure_time) << endl 
	 << mis._image_plane[i]._exposure_time << endl;
    file << strlen(mis._image_plane[i]._xray_tube_current) << endl 
	 << mis._image_plane[i]._xray_tube_current << endl;
    file << strlen(mis._image_plane[i]._exposure) << endl 
	 << mis._image_plane[i]._exposure << endl;
    file << strlen(mis._image_plane[i]._reconstruction_diameter) << endl
	 << mis._image_plane[i]._reconstruction_diameter << endl;
  };

  /*
    file << strlen(mis._short_desc) << endl 
    << mis._short_desc << endl;
    file << strlen(mis._long_desc) << endl 
    << mis._long_desc << endl;
    */

  // added by Kelvin
  file << strlen(mis._rov) << endl << mis._rov << endl;
  file << strlen(mis._angle_tilted) << endl << mis._angle_tilted << endl;
  file << mis._inteliWin << endl;
  file << mis._inteliLevel << endl;
  file << mis._access_count << endl;
  
  for (int i = 0; i < ASSOC_FILE; i++) {
    file << strlen(mis.mamo.assoc_file.name[i]) << endl
	 << mis.mamo.assoc_file.name[i] << endl;
  }
  for (int  i = 0; i < MAX_ACCESS; i++) {
    file << strlen(mis.mamo.access[i].date) << endl
	 << mis.mamo.access[i].date << endl;
    file << strlen(mis.mamo.access[i].phy_name) << endl
	 << mis.mamo.access[i].phy_name << endl;
    file << strlen(mis.mamo.access[i].phy_id) << endl
	 << mis.mamo.access[i].phy_id << endl;
    file << strlen(mis.mamo.access[i].birad_code) << endl
	 << mis.mamo.access[i].birad_code << endl;
	 
    for (int j = 0; j < MAX_MARK; j++) {
      file << mis.mamo.access[i].marks[j].x << endl;
      file << mis.mamo.access[i].marks[j].y << endl;
    }
    file << mis.mamo.access[i].marks_count << endl;

    file << strlen(mis.mamo.access[i].t_code.file) << endl
	 << mis.mamo.access[i].t_code.file << endl;
    file << strlen(mis.mamo.access[i].t_code.dbname) << endl
	 << mis.mamo.access[i].t_code.dbname << endl;
    file << strlen(mis.mamo.access[i].t_code.dbtable) << endl
	 << mis.mamo.access[i].t_code.dbtable << endl;
  }


  
  return file;
}

ifstream& operator>>(ifstream& file, MedicalImageSequence& mis) {
  assert(file != 0);
  int length;  //Used to store string lengths

  //First get rid of old imageplane array and strings
  int i;
  for(i = 0; i < mis._num_images; i++) {
    delete [] mis._image_plane[i]._kvp;
    delete [] mis._image_plane[i]._exposure_time;
    delete [] mis._image_plane[i]._xray_tube_current;
    delete [] mis._image_plane[i]._exposure;
    delete [] mis._image_plane[i]._reconstruction_diameter;
  }
  delete [] mis._image_plane;
  
  //In _init_image_plane(), checks if _image_plane is NULL
  mis._image_plane = NULL; 

  //Get base class info
  file >> (ImageSequence&)mis;

  //Read in MedicalImageSequence attributes, cleaning up allocations too
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_name;
  mis._patient_name = new char[length + 1];
  assert(mis._patient_name != 0);
  file.getline(mis._patient_name, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_id;
  mis._patient_id = new char[length + 1];
  assert(mis._patient_id != 0);
  file.getline(mis._patient_id, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_birth_date;
  mis._patient_birth_date = new char[length + 1];
  assert(mis._patient_birth_date != 0);
  file.getline(mis._patient_birth_date, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_birth_time;
  mis._patient_birth_time = new char[length + 1];
  assert(mis._patient_birth_time != 0);
  file.getline(mis._patient_birth_time, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_sex;
  mis._patient_sex = new char[length + 1];
  assert(mis._patient_sex != 0);
  file.getline(mis._patient_sex, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._study_date;
  mis._study_date = new char[length + 1];
  assert(mis._study_date != 0);
  file.getline(mis._study_date, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._study_time;
  mis._study_time = new char[length + 1];
  assert(mis._study_time != 0);
  file.getline(mis._study_time, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._accession_number;
  mis._accession_number = new char[length + 1];
  assert(mis._accession_number != 0);
  file.getline(mis._accession_number, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._study_id;
  mis._study_id = new char[length + 1];
  assert(mis._study_id != 0);
  file.getline(mis._study_id, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._study_instance_uid;
  mis._study_instance_uid = new char[length + 1];
  assert(mis._study_instance_uid != 0);
  file.getline(mis._study_instance_uid, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._referring_physicians_name;
  mis._referring_physicians_name = new char[length + 1];
  assert(mis._referring_physicians_name != 0);
  file.getline(mis._referring_physicians_name, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._study_description;
  mis._study_description = new char[length + 1];
  assert(mis._study_description != 0);
  file.getline(mis._study_description, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_age;
  mis._patient_age = new char[length + 1];
  assert(mis._patient_age != 0);
  file.getline(mis._patient_age, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_size;
  mis._patient_size = new char[length + 1];
  assert(mis._patient_size != 0);
  file.getline(mis._patient_size, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_weight;
  mis._patient_weight = new char[length + 1];
  assert(mis._patient_weight != 0);
  file.getline(mis._patient_weight, length + 1);

  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._modality;
  mis._modality = new char[length + 1];
  assert(mis._modality != 0);
  file.getline(mis._modality, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._series_number;
  mis._series_number = new char[length + 1];
  assert(mis._series_number != 0);
  file.getline(mis._series_number, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._series_instance_uid;
  mis._series_instance_uid = new char[length + 1];
  assert(mis._series_instance_uid != 0);
  file.getline(mis._series_instance_uid, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._body_part_examined;
  mis._body_part_examined = new char[length + 1];
  assert(mis._body_part_examined != 0);
  file.getline(mis._body_part_examined, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._view_position;
  mis._view_position = new char[length + 1];
  assert(mis._view_position != 0);
  file.getline(mis._view_position, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._series_date;
  mis._series_date = new char[length + 1];
  assert(mis._series_date != 0);
  file.getline(mis._series_date, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._series_time;
  mis._series_time = new char[length + 1];
  assert(mis._series_time != 0);
  file.getline(mis._series_time, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._performing_physicians_name;
  mis._performing_physicians_name = new char[length + 1];
  assert(mis._performing_physicians_name != 0);
  file.getline(mis._performing_physicians_name, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._protocol_name;
  mis._protocol_name = new char[length + 1];
  assert(mis._protocol_name != 0);
  file.getline(mis._protocol_name, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._series_description;
  mis._series_description = new char[length + 1];
  assert(mis._series_description != 0);
  file.getline(mis._series_description, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._operators_name;
  mis._operators_name = new char[length + 1];
  assert(mis._operators_name != 0);
  file.getline(mis._operators_name, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._patient_position;
  mis._patient_position = new char[length + 1];
  assert(mis._patient_position != 0);
  file.getline(mis._patient_position, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._manufacturer;
  mis._manufacturer = new char[length + 1];
  assert(mis._manufacturer != 0);
  file.getline(mis._manufacturer, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._institution_name;
  mis._institution_name = new char[length + 1];
  assert(mis._institution_name != 0);
  file.getline(mis._institution_name, length + 1);
  
  file >> length;
  file.get();  //Get rid of newline
  delete [] mis._manufacturers_model_name;
  mis._manufacturers_model_name = new char[length + 1];
  assert(mis._manufacturers_model_name != 0);
  file.getline(mis._manufacturers_model_name, length + 1);
  
  //Now read in image plane stuff
  mis._init_image_planes();
  for(i = 0; i < mis._num_images; i++) {
    file >> mis._image_plane[i]._row_pixel_spacing;
    file >> mis._image_plane[i]._column_pixel_spacing;
    file >> mis._image_plane[i]._slice_thickness;
    file >> mis._image_plane[i]._slice_location;
    
    file >> length;
    file.get();  //Get rid of newline
    delete [] mis._image_plane[i]._kvp;
    mis._image_plane[i]._kvp = new char[length + 1];
    assert(mis._image_plane[i]._kvp != 0);
    file.getline(mis._image_plane[i]._kvp, length + 1);
  
    file >> length;
    file.get();  //Get rid of newline
    delete [] mis._image_plane[i]._exposure_time;
    mis._image_plane[i]._exposure_time = new char[length + 1];
    assert(mis._image_plane[i]._exposure_time != 0);
    file.getline(mis._image_plane[i]._exposure_time, length + 1);
  
    file >> length;
    file.get();  //Get rid of newline
    delete [] mis._image_plane[i]._xray_tube_current;
    mis._image_plane[i]._xray_tube_current = new char[length + 1];
    assert(mis._image_plane[i]._xray_tube_current != 0);
    file.getline(mis._image_plane[i]._xray_tube_current, length + 1);
  
    file >> length;
    file.get();  //Get rid of newline
    delete [] mis._image_plane[i]._exposure;
    mis._image_plane[i]._exposure = new char[length + 1];
    assert(mis._image_plane[i]._exposure != 0);
    file.getline(mis._image_plane[i]._exposure, length + 1);
  
    file >> length;
    file.get();  //Get rid of newline
    delete [] mis._image_plane[i]._reconstruction_diameter;
    mis._image_plane[i]._reconstruction_diameter = new char[length + 1];
    assert(mis._image_plane[i]._reconstruction_diameter != 0);
    file.getline(mis._image_plane[i]._reconstruction_diameter, length + 1);
  };
  mis._create_descriptions();


  // added by Kelvin
  // read in Mammo related stuffs
  file >> length;
  file.get();
  delete [] mis._rov;
  mis._rov = new char[length+1];
  assert(mis._rov != 0);
  file.getline(mis._rov, length+1);

  file >> length;
  file.get();
  delete [] mis._angle_tilted;
  mis._angle_tilted = new char[length+1];
  assert(mis._angle_tilted != 0);
  file.getline(mis._angle_tilted, length+1);

  file >> mis._inteliWin;
  file >> mis._inteliLevel;
  file >> mis._access_count;

  mis._init_PACS_SUBHEADER_MAMO();
  
  for (i = 0; i < ASSOC_FILE; i++) {
    file >> length;
    file.get();
    file.getline(mis.mamo.assoc_file.name[i], length+1);
  }


  for ( i = 0; i < MAX_ACCESS; i++) {
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].date, length+1);
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].phy_name, length+1);
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].phy_id, length+1);
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].birad_code, length+1);
	 
    for (int j = 0; j < MAX_MARK; j++) {
      file >> mis.mamo.access[i].marks[j].x;
      file >> mis.mamo.access[i].marks[j].y;
    }

    file >> mis.mamo.access[i].marks_count;
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].t_code.file, length+1);
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].t_code.dbname, length+1);
    file >> length;
    file.get();
    file.getline(mis.mamo.access[i].t_code.dbtable, length+1);
  }

  return file;
};

const char* const MedicalImageSequence::short_desc() const
{
  return _short_desc;
};

const char* const MedicalImageSequence::long_desc() const
{
  return _long_desc;
};



void MedicalImageSequence::_create_descriptions()
{
  // kelvin
  const int sl = 30;             // short description length
  const int ll = 400;            // long description length

  delete [] _short_desc;
  _short_desc = new char[sl+1];
  memset(_short_desc, 0, sl+1);

  strncat(_short_desc, _study_date,         sl - strlen(_short_desc));
  strncat(_short_desc, " ",                 sl - strlen(_short_desc));
  strncat(_short_desc, _modality,           sl - strlen(_short_desc));
  strncat(_short_desc, " ",                 sl - strlen(_short_desc));  
  // kelvin
  strncat(_short_desc, _body_part_examined,   sl - strlen(_short_desc));
  strncat(_short_desc, " ",                 sl - strlen(_short_desc));  
  strncat(_short_desc, _view_position,   sl - strlen(_short_desc));

  delete [] _long_desc;
  _long_desc = new char[ll+1];
  memset(_long_desc, 0, ll+1);

  strncat(_long_desc, "---PATIENT---\n",              ll - strlen(_long_desc));
  strncat(_long_desc, _patient_name,                  ll - strlen(_long_desc));
  strncat(_long_desc, "\n",                           ll - strlen(_long_desc));
  strncat(_long_desc, _patient_id,                    ll - strlen(_long_desc));
  strncat(_long_desc, "\nDOB: ",                      ll - strlen(_long_desc));
  strncat(_long_desc, _patient_birth_date,            ll - strlen(_long_desc));
  strncat(_long_desc, "\nSEX: ",                      ll - strlen(_long_desc));
  strncat(_long_desc, _patient_sex,                   ll - strlen(_long_desc));
  strncat(_long_desc, "\n\n---STUDY---\nMODALITY: ",  ll - strlen(_long_desc));
  strncat(_long_desc, _modality,                      ll - strlen(_long_desc));
  strncat(_long_desc, "\nDESC: ",                     ll - strlen(_long_desc));
  strncat(_long_desc, _study_description,             ll - strlen(_long_desc));
  strncat(_long_desc, "\nDATE: ",                     ll - strlen(_long_desc));
  strncat(_long_desc, _study_date,                    ll - strlen(_long_desc));
  strncat(_long_desc, "\nTIME: ",                     ll - strlen(_long_desc));
  strncat(_long_desc, _study_time,                    ll - strlen(_long_desc));
  strncat(_long_desc, "\nPERF. PHYS.: ",              ll - strlen(_long_desc));
  strncat(_long_desc, _performing_physicians_name,    ll - strlen(_long_desc));
  strncat(_long_desc, "\nREF. PHYS.: ",               ll - strlen(_long_desc));
  strncat(_long_desc, _referring_physicians_name,     ll - strlen(_long_desc));
  strncat(_long_desc, "\nPROTOCOL: ",                 ll - strlen(_long_desc));
  strncat(_long_desc, _protocol_name,                 ll - strlen(_long_desc));
};


//    added by Kelvin     
void MedicalImageSequence::rov(const char* const reason) 
{
  assert(reason != NULL);
  char_alloc_and_copy(_rov, reason);
};

const char* const MedicalImageSequence::rov()
{
  return(_rov);
};

void MedicalImageSequence::angle_tilt(const char* const angle)
{
  assert(angle != NULL);
  char_alloc_and_copy(_angle_tilted, angle);
};


const char* const MedicalImageSequence::angle_tilt()
{
  return(_angle_tilted);
};

void MedicalImageSequence::inteliWin(double frac_win)
{
  if (frac_win > 1.0) {
    frac_win = 1.0;
  };
  if (frac_win < 0.0) {
    frac_win = 0.0;
  };
  _inteliWin = frac_win;
};

const double MedicalImageSequence::inteliWin() const
{
  return(_inteliWin);
};

void MedicalImageSequence::inteliLevel(double frac_level)
{
  if (frac_level > 1.0) {
    frac_level = 1.0;
  };
  if (frac_level < 0.0) {
    frac_level = 0.0;
  };
  _inteliLevel = frac_level;
};

const double MedicalImageSequence::inteliLevel() const
{
  return(_inteliLevel);
};

void MedicalImageSequence::diag_phy_name(unsigned int access_index, 
					 const char* const name)
{

  assert(name != NULL);
  assert((access_index >= 0) && (access_index < MAX_ACCESS));

  _access_count++;
  assert(_access_count <= MAX_ACCESS);

  strncpy(mamo.access[access_index].phy_name, name, NAME_LEN-1);
}

const char* const MedicalImageSequence::diag_phy_name(unsigned int 
						      access_index)
{
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  return(mamo.access[access_index].phy_name);
};

void MedicalImageSequence::diag_phy_id(unsigned int access_index, 
				       const char* const id)
{
  assert(id != NULL);
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  assert(_access_count <= MAX_ACCESS);
  strncpy(mamo.access[access_index].phy_id, id, ID_LEN-1);
}

const char* const MedicalImageSequence::diag_phy_id(unsigned int 
						    access_index)
{
  assert((access_index > 0) && (access_index < MAX_ACCESS));
  return(mamo.access[access_index].phy_id);
};



void MedicalImageSequence::diag_date(unsigned int access_index, 
				     const char* const date)
{
  assert(date != NULL);
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  assert(_access_count <= MAX_ACCESS);
  strncpy(mamo.access[access_index].date, date, DATE_LEN-1);
}

const char* const MedicalImageSequence::diag_date(unsigned int 
						  access_index)
{
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  return(mamo.access[access_index].date);
};



void MedicalImageSequence::birad(unsigned int access_index, 
				 const char* const code)
{
  assert(code != NULL);
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  assert(_access_count <= MAX_ACCESS);

  strncpy(mamo.access[access_index].birad_code, code, CODE_LEN-1);
}

const char* const MedicalImageSequence::birad(unsigned int 
					      access_index)
{
  assert((access_index >= 0) && (access_index < MAX_ACCESS));
  return(mamo.access[access_index].birad_code);
};

void MedicalImageSequence::mark(unsigned int aindex, 
				unsigned int index, 
				MARK* m)
{
  assert((index >= 0) && (index < MAX_MARK));
  assert((aindex >= 0) && (aindex < MAX_ACCESS));
  // heck
  assert((m->x >= 0) && (m->x < 4800));
  assert((m->y >= 0) && (m->y < 6400));
  assert(_access_count <= MAX_ACCESS);

  mamo.access[aindex].marks_count++;
  mamo.access[aindex].marks[index].x = m->x;
  mamo.access[aindex].marks[index].y = m->y;
  // cout << aindex << " " << index << " " << m->x << " " << m->y << endl;
  

};


void MedicalImageSequence::delete_mark(unsigned int aindex, 
				       unsigned int index)
{
  assert((index >= 0) && (index < MAX_MARK));
  assert((aindex >= 0) && (aindex < MAX_ACCESS));
  assert(_access_count <= MAX_ACCESS);

  mamo.access[aindex].marks_count--;
  mamo.access[aindex].marks[index].x = 0;
  mamo.access[aindex].marks[index].y = 0;
};



MARK MedicalImageSequence::mark(unsigned int aindex, 
				unsigned int index)
{
  assert(index >= 0 && index < MAX_MARK);
  assert(aindex >= 0 && aindex < MAX_ACCESS);
  
  return(mamo.access[aindex].marks[index]);
  
};


void MedicalImageSequence::tcode_file(unsigned int index, const char* const filename)
{
  if (filename != NULL)
    strcpy(mamo.access[index].t_code.file, filename);
  else 
    strcpy(mamo.access[index].t_code.file, "N/A");
};

const char* const MedicalImageSequence::tcode_file(unsigned int index)
{
  assert(index >= 0 && index < MAX_ACCESS);  
  return(mamo.access[index].t_code.file);
};



const char* const MedicalImageSequence::tcode_db(unsigned int index)
{
  assert(index >= 0 && index < MAX_ACCESS);  
  return(mamo.access[index].t_code.dbname);
};


void MedicalImageSequence::tcode_db(unsigned int index, 
				    const char* const dbname)
{
  assert(index >= 0 && index < MAX_ACCESS);  
  if (dbname != NULL)
    strcpy(mamo.access[index].t_code.dbname, dbname);
  else
    strcpy(mamo.access[index].t_code.dbname, "N/A");
};

const char* const MedicalImageSequence::tcode_dbtable(unsigned int 
						      index)
{
  assert(index >= 0 && index < MAX_ACCESS);  
  return(mamo.access[index].t_code.dbtable);
};

void MedicalImageSequence::tcode_dbtable(unsigned int index, 
					 const char* const dbtable)
{
  assert(index >= 0 && index < MAX_ACCESS);  
  if (dbtable != NULL)
    strcpy(mamo.access[index].t_code.dbtable, dbtable);
  else
    strcpy(mamo.access[index].t_code.dbtable, "N/A");
};

void MedicalImageSequence::assoc_file(unsigned int file_num, const char* const filename)
{
  assert((file_num >= 0) && (file_num < ASSOC_FILE));

  if (filename != NULL) {
    strcpy(mamo.assoc_file.name[file_num], filename);
  }
  else {
    strcpy(mamo.assoc_file.name[file_num], "");
  };
};

const char* const MedicalImageSequence::assoc_file(unsigned int file_num)
{
  assert((file_num >= 0) && (file_num < ASSOC_FILE));
  return(mamo.assoc_file.name[file_num]);
};

void MedicalImageSequence::_init_PACS_SUBHEADER_MAMO()
{
  for (int i = 0; i < MAX_ACCESS; i++) {
    strcpy(mamo.access[i].date, "");
    strcpy(mamo.access[i].phy_name, "");
    strcpy(mamo.access[i].phy_id, "");
    strcpy(mamo.access[i].birad_code,"");
    for (int j = 0; j < MAX_MARK; j++) {
      mamo.access[i].marks[j].x = 0;
      mamo.access[i].marks[j].y = 0;
    }
    mamo.access[i].marks_count = 0;
    strcpy(mamo.access[i].t_code.file, "");
    strcpy(mamo.access[i].t_code.dbname, "");
    strcpy(mamo.access[i].t_code.dbtable, " ");
  }
  
  for (int k = 0; k < ASSOC_FILE; k++) 
    strcpy(mamo.assoc_file.name[k], "");
  
};

int MedicalImageSequence::get_access_count()
{
  return(_access_count);
};

void MedicalImageSequence::add_access_count()
{
  _access_count++;
};

unsigned int MedicalImageSequence::get_mark_count(int index)
{
  assert(index >= 0);
  assert(index < MAX_ACCESS);
  return(mamo.access[index].marks_count);
};

