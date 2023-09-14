#ifndef __MedicalImageSequence_h_
#define __MedicalImageSequence_h_

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     UCLA Department of Radiological Sciences
**     Imaging and Information Systems (c) 1997
** ======================================================================
** This software comprises unpublished confidential information of the
** University of California and may not be used, copied or made
** available to anyone, except with written permission of the
** Department of Radiological Sciences and Regents of the University
** of California.  All rights reserved.
**
** This software program and documentation are copyrighted by The
** Regents of the University of California. The software program and
** documentation are supplied "as is", without any accompanying
** services from The Regents. The Regents does not warrant that the
** operation of the program will be uninterrupted or error-free. The
** end-user understands that the program was developed for research
** purposes and is advised not to rely exclusively on the program for
** any reason.
**
** IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY
** PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
** DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
** SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
** CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE
** UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
** PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
** CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
** UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
**
** ======================================================================
** The UCLA Radiology Information and Picture Archiving and
** Communications System (RIS/PACS) is a division of the Department of
** Radiological Sciences, University of California at Los Angeles.
**
** for more information, or for permission to use this software for
** commercial or non-commercial purposes, please contact:
**
** Daniel J. Valentino, Ph.D.
** Assistant Professor and Faculty Director of Imaging and Information Systems
** Department of Radiological Sciences
** Mail Stop172115
** UCLA Medical Center
** Los Angeles  CA 90024-1721
** 310-794-7131
** http://www.radsci.ucla.edu/
** ======================================================================
-*/

#include "ImageSequence.h"
#include "ucla_v5b_mamo.h"

//extern "C" {
#include <assert.h>
#include <string.h>
//}
#include <iostream>
using std::ifstream;
using std::ofstream;

/// Image sequence corresponding to a medical series.
class MedicalImageSequence : public ImageSequence
{
  /**
   *  Overloaded stream insertion operator to write contents to a file.
   *  Must ensure that file is opened first.
   *  Usage: file << aMedicalImageSequence.
   */
  friend ofstream& operator<<(ofstream& file,
			      const MedicalImageSequence& mis);

  /**
   *  Overloaded stream extraction operator to get data from file.
   *  Must ensure that file is opened, and in proper format.
   *  Usage: file >> Study.
   */
  friend ifstream& operator>>(ifstream& file, MedicalImageSequence& mis);
public:
  /// Default constructor.
  MedicalImageSequence();

  /**
   *  @param num_images Number of images.
   *  @param width Width of images in pixels.
   *  @param height Height of images in pixels.
   *  @param bits_per_pixel Gray scale bit depth of images (out of 16).
   *  @param file_names Array of char array corresponding to one file
   *  name per image.  Zero corresponds to sequence of black images.
   *  @memo Constructor.
   */
  MedicalImageSequence(const int num_images,
		       const int width, const int height,
		       const int bits_per_pixel,
		       const char** const file_names = 0);

  /// Constructor for preallocated images.
  MedicalImageSequence(const int num_images, Image** images);

  /// Copy constructor (Added by BW for dmtk).
  MedicalImageSequence(const MedicalImageSequence& mis);

  /// Destructor.
  ~MedicalImageSequence();

  ///Overloaded assignment operator (Added by BW for dmtk).
  const MedicalImageSequence& operator=(const MedicalImageSequence& Rhs);

  /// Returns patient name (NULL if not set).
  const char* const patient_name() const;

  /// Sets patient name.
  void patient_name(const char* const new_value);

  /// Returns patient ID (NULL if not set).
  const char* const patient_id() const;

  /// Sets patient ID.
  void patient_id(const char* const new_value);

  /// Returns patient birth date (NULL if not set).
  const char* const patient_birth_date() const;

  /// Sets patient birth date.
  void patient_birth_date(const char* const new_value);

  /// Returns patient birth time (-1.0 if not set).
  const char* const patient_birth_time() const;

  /// Sets patient birth time
  void patient_birth_time(const char* const new_value);

  /// Returns patient sex (NULL if not set).
  const char* const patient_sex() const;

  /// Sets patient sex.
  void patient_sex(const char* const new_value);

  /// Returns study date (NULL if not set).
  const char* const study_date() const;

  /// Sets study date.
  void study_date(const char* const new_value);

  /// Returns study time (NULL if not set).
  const char* const study_time() const;

  /// Sets study time.
  void study_time(const char* const new_value);

  /// Returns the accession number.
  const char* const accession_number() const;

  /// Sets the accession number.
  void accession_number(const char* const new_value);

  /// Returns the study ID.
  const char* const study_id() const;

  /// Sets the study ID.
  void study_id(const char* const new_value);

  /// Returns the study instance UID (NULL if not set).
  const char* const study_instance_uid() const;

  /// Sets the study instance UID.
  void study_instance_uid(const char* const new_value);

  /// Returns name of referring physician (NULL if not set).
  const char* const referring_physicians_name() const;

  /// Sets name of referring physician.
  void referring_physicians_name(const char* const new_value);

  /// Returns study description (NULL if not set).
  const char* const study_description() const;

  /// Sets study description.
  void study_description(const char* const new_value);

  /// Returns the patient age (NULL if not set).
  const char* const patient_age() const;

  /// Sets the patient age.
  void patient_age(const char* const new_value);

  /// Returns the patient size (NULL if not set).
  const char* const patient_size() const;

  /// Sets the patient size.
  void patient_size(const char* const new_value);

  /// Returns the patient weight (NULL if not set).
  const char* const patient_weight() const;

  /// Sets the patient weight.
  void patient_weight(const char* const new_value);

  /**
    Returns imaging modality (NULL if not set).
    CT = computed tomography
    */
  const char* const modality() const;

  /// Sets imaging modality.
  void modality(const char* const new_value);

  /// Returns the series number (NULL if not set).
  const char* const series_number() const;

  /// Sets the series number.
  void series_number(const char* const new_value);

  /// Returns the series instance UID (NULL if not set).
  const char* const series_instance_uid() const;

  /// Sets the series instance UID.
  void series_instance_uid(const char* const new_value);

  /// Returns the body part examined (NULL if not set).
  const char* const body_part_examined() const;

  /// Sets the body part examined.
  void body_part_examined(const char* const new_value);

  /// Returns the view position (NULL if not set).
  const char* const view_position() const;

  /// Sets the view position.
  void view_position(const char* const new_value);

  /// Returns series date (NULL if not set).
  const char* const series_date() const;

  /// Sets series date.
  void series_date(const char* const new_value);

  /// Returns series time (NULL if not set).
  const char* const series_time() const;

  /// Sets series time.
  void series_time(const char* const new_value);

  /// Name of physician performing image acquisition.
  const char* const performing_physicians_name() const;

  /// Sets name of physician performing image acquisition.
  void performing_physicians_name(const char* const new_value);

  /// Returns description of conditions under which series was obtained.
  const char* const protocol_name() const;

  /// Sets description of conditions under which series was obtained.
  void protocol_name(const char* const new_value);

  /// Returns series description (NULL if not set).
  const char* const series_description() const;

  /// Sets series description.
  void series_description(const char* const new_value);

  /// Returns technologist's name (NULL if not set).
  const char* const operators_name() const;

  /// Sets technologist's name.
  void operators_name(const char* const new_value);

  /// Returns patient position.
  const char* const patient_position() const;

  /// Sets patient position.
  void patient_position(const char* const new_value);

  /// Returns imaging device manufacturer (NULL if not set).
  const char* const manufacturer() const;

  /// Sets imaging device manufacturer.
  void manufacturer(const char* const new_value);

  /// Returns institution name where imaging device is located (or NULL).
  const char* const institution_name() const;

  /// Sets name of institution where imaging device is located.
  void institution_name(const char* const new_value);

  /// Returns manufacturer's model name (NULL if not set).
  const char* const manufacturers_model_name() const;

  /// Sets manufacturer's model name.
  void manufacturers_model_name(const char* const new_value);

  /**
    Returns peak kilo voltage of the X-ray generator for the given image number.
    Returns NULL if not set.
    */
  const char* const kvp(const int z) const;

  /// Sets peak kilo voltage of the X-ray generator for the given image number.
  void kvp(const int, const char* const new_value);

  /**
    Returns time of X-ray exposure in msec for the given image number.
    Returns NULL if not set.
    */
  const char* const exposure_time(const int z) const;

  /// Sets time of X-ray exposure in msec for the given image number.
  void exposure_time(const int z, const char* const new_value);

  /**
    Returns X-ray tube current in mA for the given image number.
    Returns NULL if not set.
    */
  const char* const xray_tube_current(const int z) const;

  /// Sets X-ray tube current in mA for the given image number.
  void xray_tube_current(const int z, const char* const new_value);

  /**
    Returns product of exposure time and X-ray tube current in mAs for
    the given image number.  Returns NULL if not set.
    */
  const char* const exposure(const int z) const;

  /**
    Sets product of exposure time and X-ray tube current in mAs for
    the given image number.
    */
  void exposure(const int z, const char* const new_value);

  /**
    Returns CT field of view for the given image number.
    Returns NULL if not set.
    */
  const char* const reconstruction_diameter(const int z) const;

  /// Sets CT field of view for the given image number.
  void reconstruction_diameter(const int z, const char* const new_value);

  /**
    Returns row distance (mm) between centers of pixels for the given
    image number.  Returns -1.0 if not defined.
    */
  const float row_pixel_spacing(const int z) const;

  /// Sets row distance (mm) between centers of pixels for given image number.
  void row_pixel_spacing(const int z, const float new_value);

  /**
    Returns column distance (mm) between centers of pixels for the given
    image number. Returns -1.0 if not defined.
    */
  const float column_pixel_spacing(const int z) const;

  /// Sets column distance (mm) between pixels centers for given image number.
  void column_pixel_spacing(const int z, const float new_value);

  /**
    Returns nominal slice thickness (mm) for the given image number.
    Returns -1.0 if not defined.
    */
  const float slice_thickness(const int z) const;

  /// Sets nominal slice thickness (mm) for the given image number.
  void slice_thickness(const int z, const float new_value);

  /**
    Returns relative position of exposure (mm) for the given image number.
    Returns -1.0 if not defined.
    */
  const float slice_location(const int z) const;

  /// Sets relative position of exposure (mm) for the given image number.
  void slice_location(const int z, const float new_value);

  /**
    Returns Pointer to pixel data (in raster order) for the given image number.
    Returns NULL if not defined.
    */
  const short* const pixel_data(const int z);

	/**
	Returns HU values in column-major order.
	1D array with size xdim*ydim*zdim.
	Will compute the first time it is called (so will be slow the first time).
	*/
	const short* const hu_values_column_major();

  /// Returns x-dimension (number of pixels in x-direction).
  const int xdim() const;

  /// Returns y-dimension (number of pixels in y-direction).
  const int ydim() const;

  /// Returns z-dimension (number of slices).
  const int zdim() const;

  /**
    Returns a pixel value (gray-level).
    Returns 32767 if value undefined.
    */
  const short pix_val(const int x, const int y, const int z=0);

  /**
    THIS IS A FAST AND DANGEROUS METHOD TO USE, BEWARE!!!
    Returns a pixel value (gray-level).
    Does not do any array bounds checking => program may crash if
    (x,y,z) coordinates are invalid.
    Also does not check if data has been loaded from disk into memory for the
    image.  If it hasn't, the program will segmentation fault.
    */
  const short fast_pix_val(const int x, const int y,
				    const int z=0);

  /// Displays imaging parameters on the screen.
  void print_parameters();

  ///
  const char* const short_desc() const;
  ///
  const char* const long_desc() const;


  /* added by Kelvin */
  /* added by Kelvin */
  /* added by Kelvin */

  /// Sets reason of visit.
  void rov(const char* const reason);

  /// Returns reason of visit.
  const char* const rov();

  /// Sets angle tilted.
  void angle_tilt(const char* const angle);

  /// Returns angle tilted.
  const char* const angle_tilt();

  /// Sets intelligent window.
  void inteliWin(double frac_win);

  /// Returns inteliWin value.
  const double inteliWin() const;

  /// Sets intelligent level.
  void inteliLevel(double frac_level);

  /// Returns inteliLevel value.
  const double inteliLevel() const;

  /// Sets diagonstic physician name.
  void diag_phy_name(unsigned int access_index, const char* const name);

  /// Returns diagonstic physician name.
  const char* const diag_phy_name(unsigned int access_index);

  /// Sets diagonstic physician id.
  void diag_phy_id(unsigned int access_index, const char* const name);

  /// Returns diagonstic physician id.
  const char* const diag_phy_id(unsigned int access_index);

  /// Sets diagonstic date.
  void diag_date(unsigned int access_index, const char* const name);

  /// Returns diagonstic date.
  const char* const diag_date(unsigned int index);

  /// Sets BIRAD code.
  void birad(unsigned int access_index, const char* const name);

  /// Returns BIRAD code.
  const char* const birad(unsigned int access_index);

  /// Sets MARKs.
  void mark(unsigned int access_index, unsigned int mark_index, MARK* m);

  /// Returns MARKs.
  MARK mark(unsigned int access_index, unsigned int mark_index);

  /// delete MARKS.
  void delete_mark(unsigned int access_index, unsigned int mark_index);

  /// Sets associated files.
  void assoc_file(unsigned int file_num, const char* const filename);

  /// Returns associated files.
  const char* const assoc_file(unsigned int file_num);

  /// Sets Transcript code file.
  void tcode_file(unsigned int access_index, const char* const filename);

  /// Returns Transcript code file.
  const char* const tcode_file(unsigned int index);

  /// Sets Transcript code database name.
  void tcode_db(unsigned int access_index, const char* const dbname);

  /// Returns Transcript code database name.
  const char* const tcode_db(unsigned int access_index);

  /// Sets Transcript code database table name.
  void tcode_dbtable(unsigned int access_index, const char* const dbtable);

  /// Returns Transcript code database table name.
  const char* const tcode_dbtable(unsigned int index);

  /// histogram peak position - Limin.
  int peakposition;

  /// start position - Limin.
  int startposition;

  /// end position - Limin.
  int endposition;

  /// pixel sum - Limin.
  int pixsum;

  /// get access count.
  int get_access_count();

  /// increase access count.
  void add_access_count();

  /// get max\_mark\_count.
  unsigned int get_mark_count(int access_index);

  /* Ended by Kelvin */
  /* Ended by Kelvin */
  /* Ended by Kelvin */

protected:

  /**
    Allocates storage and initializes image planes.
    \_num\_images must be set.
    */
  void _init_image_planes();

  /// Generate the descriptions based on the header.
  // change to virtual function by Kelvin 10/21/97
  virtual void _create_descriptions();

  /**
  Patient name.
  DICOM: Patient Module.
  Undefined: NULL
  */
  char* _patient_name;

  /**
  Patient ID.
  DICOM: Patient Module.
  Undefined: NULL
  */
  char* _patient_id;

  /**
  Patient birth date.
  DICOM: Patient Module.
  Undefined: NULL
  */
  char* _patient_birth_date;

  /**  Added by BW 082797
  Patient birth time.
  DICOM: General Patient Module.
  Undefined: NULL
  */
  char* _patient_birth_time;

  /**
  Patient sex.
  DICOM: Patient Module.
  Undefined: NULL
  */
  char* _patient_sex;

  /**
  Study date.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _study_date;

  /**
  Study time.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _study_time;

  /**  Added by BW 082797
  Accession Number.
  DICOM: General Study Relationship Module.
  Undefined: NULL
  */
  char* _accession_number;

  /** Added by BW 082797
  Study ID.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _study_id;

  /** Added by BW 082707
  Study Instace UID.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _study_instance_uid;

  /**
  Referring physician's name.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _referring_physicians_name;

  /**
  Study description.
  DICOM: General Study Module.
  Undefined: NULL
  */
  char* _study_description;

  /** Added by BW 082797
  Patient Age.
  DICOM: Patient Study Module.
  Undefined: NULL
  */
  char* _patient_age;

  /** Added by BW 082797
  Patient Size.
  DICOM: Patient Study Module.
  Undefined: NULL
  */
  char* _patient_size;

  /** Added by BW 082797
  Patient Weight.
  DICOM: Patient Study Module.
  Undefined: NULL
  */
  char* _patient_weight;

  /**
  Imaging modality.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _modality;

  /** Added by BW 082797
  Series Number.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _series_number;

  /** Added by BW 082797
  Series Instance UID.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _series_instance_uid;

  /** Added by BW 082797
  Body Part Examined.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _body_part_examined;

  /** Added by BW 082797
  View Position.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _view_position;

  /**
  Series date.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _series_date;

  /**
  Series time.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _series_time;

  /**
  Name of physician performing image acquisition.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _performing_physicians_name;

  /**
  User-defined description of conditions under which series was obtained.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _protocol_name;

  /**
  Series description.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _series_description;

  /**
  Operator's name.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _operators_name;

  /**
  Patient position.
  DICOM: General Series Module.
  Undefined: NULL
  */
  char* _patient_position;

  /**
  Imaging device manufacturer.
  DICOM: General Equipment Module.
  Undefined: NULL
  */
  char* _manufacturer;

  /**
  Institution where imaging device is located.
  DICOM: General Equipment Module.
  Undefined: NULL
  */
  char* _institution_name;

  /**
  Manufacturer's model name.
  DICOM: General Equipment Module.
  Undefined: NULL
  */
  char* _manufacturers_model_name;

  /// Data associated with an imaging plane.
  struct _ImagePlane {

    /**
    Row distance (mm) between centers of pixels.
    DICOM: Image Plane Module.
    Undefined: -1.0
   */
    float _row_pixel_spacing;

    /**
    Column distance (mm) between centers of pixels.
    DICOM: Image Plane Module.
    Undefined: -1.0
    */
    float _column_pixel_spacing;

    /**
    Nominal slice thickness (mm).
    DICOM: Image Plane Module.
    Undefined: -1.0
    */
    float _slice_thickness;

    /**
    Relative position of exposure (mm).
    DICOM: Image Plane Module.
    Undefined: -1.0
    */
    float _slice_location;

    /**
    Peak kilo voltage of the x-ray generator.
    DICOM: CR Image Module, CT Image Module.
    Undefined: NULL
    */
    char* _kvp;

    /**
    Time of X-ray exposure in msec.
    DICOM: CR Image Module, CT Image Module.
    Undefined: NULL
    */
    char* _exposure_time;

    /**
    X-ray tube current in mA.
    DICOM: CR Image Module, CT Image Module.
    Undefined: NULL
    */
    char* _xray_tube_current;

    /**
    Product of exposure time and X-ray tube current in mAs.
    DICOM: CR Image Module, CT Image Module.
    Undefined: NULL
    */
    char* _exposure;

    /**
    CT reconstruction diameter (mm).
    DICOM: CT Image Module.
    Undefined: NULL
    */
    char* _reconstruction_diameter;
  };

  /// Image planes.
  _ImagePlane* _image_plane;

  /// Short description of the series (title).
  char* _short_desc;

  /// Long description of the series (formatted header information).
  char* _long_desc;


  /* added by Kelvin */
  /* added by Kelvin */
  /* added by Kelvin */

  /// Allocates storage and initializes PACS\_SUBHDR\_MAMO.
  virtual void _init_PACS_SUBHEADER_MAMO();

  /// PACS mammography subheader.
  PACS_SUBHEADER_MAMO mamo;

  /// reason of visit.
  char* _rov;

  /// angle tilted.
  char* _angle_tilted;

  /// inteliWin value.
  double _inteliWin;

  /// inteliLevel value.
  double _inteliLevel;

  ///
  unsigned int _access_count;

  /* Ended by Kelvin */
  /* Ended by Kelvin */
  /* Ended by Kelvin */

	/**
	HU values derived from _mis.
	Initialized to zero.
	*/
	short* _hu_values_column_major;
};

///
inline
const short MedicalImageSequence::fast_pix_val(const int x,
							const int y,
							const int z)
{
  return image(z).unsafe_pixel_data()[y*width() + x];
}


#endif /* !__MedicalImageSequence_h_ */
