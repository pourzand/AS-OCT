#ifndef __SegParam_h_
#define __SegParam_h_

#include "miniIni.h"

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     Matthew Brown (c) 1995
** ======================================================================
** This software comprises unpublished confidential information of Matthew
** Brown and may not be used, copied or made
** available to anyone, except with written permission of Matthew Brown.
** All rights reserved.
**
** This software program and documentation are copyrighted by Matthew Brown
** The software program and documentation are supplied "as is", without any
**
** accompanying services from Matthew Brown. Matthew Brown does not warrant
** that the operation of the program will be uninterrupted or error-free. The
** end-user understands that the program was developed for research
** purposes and is advised not to rely exclusively on the program for
** any reason.
**
** IN NO EVENT SHALL MATTHEW BROWN BE LIABLE TO ANY
** PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
** DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
** SOFTWARE AND ITS DOCUMENTATION, EVEN IF MATTHEW BROWN
** HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. MATTHEW BROWN
** SPECIFICALLY DISCLAIMS ANY WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
** PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND MATTHEW BROWN
** HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
** UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
**
** ======================================================================
**
** for more information, or for permission to use this software for
** commercial or non-commercial purposes, please contact:
**
** Matthew S. Brown, Ph.D.
** Assistant Professor
** Department of Radiological Sciences
** Mail Stop172115
** UCLA Medical Center
** Los Angeles  CA 90024-1721
** 310-267-1820
** mbrown@mednet.ucla.edu
** ======================================================================
-*/

#include <vector>
#include <string.h>

#include "Attribute.h"

/// An abstract base class for storing segmentation parameters on the blackboard
class SegParam : public Attribute {
public:

	/// Constructor
	SegParam();

	/// Copy constructor
	SegParam(const SegParam&);

	/// Destructor
	virtual ~SegParam();

	/// Type of attribute
	const char* const type() const { return "SegParam"; };

};

/// Use matched image candidates from another solution element or multiple elements as candidates
class AddMatchedCandidates : public SegParam {
public:
	/// Constructor (single related solution element)
	AddMatchedCandidates(const std::string& rel_solel_name, const int rel_solel_ind);

	/// Constructor (multiple related solution elements)
	AddMatchedCandidates(const Darray<std::string*>& rel_solel_name, const Darray<int>& rel_solel_ind);

	/// Copy constructor
	AddMatchedCandidates(const AddMatchedCandidates&);

	/// Destructor
	~AddMatchedCandidates();

	/// Name of the attribute
	const char* const name() const { return "AddMatchedCandidates"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline>  Related SolElement: name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};

/// Use distance map to form candidates
class DistanceMap25DPercMax : public SegParam {
public:
	/// Constructor
	DistanceMap25DPercMax(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold, const float percentage, const int hu_diff_threshold, const float max_z_dist);

	/// Copy constructor
	DistanceMap25DPercMax(const DistanceMap25DPercMax&);

	/// Destructor
	~DistanceMap25DPercMax();

	/// Name of the attribute
	const char* const name() const { return "DistanceMap25DPercMax"; };

	/// Threshold on local maximum if a candidate is to be formed (mm)
	const float dist_threshold() const;

	/// Percentage for thresholding
	const float percentage() const;

	/// Threshold on absolute difference in HU between current voxel and HU-value at local (distance) maximum
	const int hu_diff_threshold() const;

	const float max_z_distance() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Percentage: percentage <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// Threshold on local maximum if a candidate is to be formed (mm)
	float _dist_threshold;

	/// Percentage of local maximum to be used in thresholding
	float _perc;

	/// Threshold on absolute difference in HU between current voxel and HU-value at local (distance) maximum
	int _hu_diff_threshold;

	float _max_z_dist;
};


/// Use distance map to form candidates
class DistanceMapRegionGrowing : public SegParam {
public:
	/// Constructor
	DistanceMapRegionGrowing(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold, const float percentage);

	/// Copy constructor
	DistanceMapRegionGrowing(const DistanceMapRegionGrowing&);

	/// Destructor
	~DistanceMapRegionGrowing();

	/// Name of the attribute
	const char* const name() const { return "DistanceMapRegionGrowing"; };

	/// Threshold on local maximum if a candidate is to be formed (mm)
	const float dist_threshold() const;

	/// Percentage for thresholding
	const float percentage() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Percentage: percentage <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// Threshold on local maximum if a candidate is to be formed (mm)
	float _dist_threshold;

	/// Percentage of local maximum to be used in thresholding
	float _perc;
};


/// Use distance map to form candidates
class DistanceMapWatershed : public SegParam {
public:
	/// Constructor
	DistanceMapWatershed(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_threshold);

	/// Copy constructor
	DistanceMapWatershed(const DistanceMapWatershed&);

	/// Destructor
	~DistanceMapWatershed();

	/// Name of the attribute
	const char* const name() const { return "DistanceMapWatershed"; };

	/// Threshold on local maximum if a candidate is to be formed (mm)
	const float dist_threshold() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Percentage: percentage <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// Threshold on local maximum if a candidate is to be formed (mm)
	float _dist_threshold;
};


/// Perform region growing of part solid nodules
class GrowPartSolid : public SegParam {
public:
	/// Constructor
	GrowPartSolid(const std::string& rel_solel_name, const int rel_solel_ind, const float percentage);

	/// Copy constructor
	GrowPartSolid(const GrowPartSolid&);

	/// Destructor
	~GrowPartSolid();

	/// Name of the attribute
	const char* const name() const { return "GrowPartSolid"; };

	/// Percentage for thresholding
	const float percentage() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Percentage: percentage <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// Percentage of local maximum to be used in thresholding
	float _perc;
};



/// Include all voxels in the search area in a single candidate, respecting HUrange and 2D segmentation constraints where applicable
class IncludeAllVoxels : public SegParam {
public:
	/// Constructor
	IncludeAllVoxels();

	/// Copy constructor
	IncludeAllVoxels(const IncludeAllVoxels&);

	/// Destructor
	~IncludeAllVoxels();

	/// Name of the attribute
	const char* const name() const { return "IncludeAllVoxels"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};


/// Use distance map to form candidates
class IntensitySubregion : public SegParam {
public:
	/// Constructor
	IntensitySubregion(const std::string& rel_solel_name, const int rel_solel_ind);

	/// Copy constructor
	IntensitySubregion(const IntensitySubregion&);

	/// Destructor
	~IntensitySubregion();

	/// Name of the attribute
	const char* const name() const { return "IntensitySubregion"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Percentage: percentage <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};


/// LineToDots tracks a smooth curve (provided by the search area) at intervals specified by a spacing (and searching ahead up to check_distance if there are discontinuities), starting in the axis direction specified. Axis is "x" or "y". Spacing and check_distance are in mm. If the direction is x, the leftmost point is used as the start, and if the direction is y, the top point is used. The algorithm operates in 2D and processes each slice independently, finding one curve per slice.
class LineToDots : public SegParam {
public:
	/// Constructor
	LineToDots(const std::string& direction, const float spacing, const float check_distance);

	/// Copy constructor
	LineToDots(const LineToDots&);

	/// Destructor
	~LineToDots();

	/// Name of the attribute
	const char* const name() const { return "LineToDots"; };

	/// Direction ("x" or "y")
	const std::string& direction() const { return _direction; };

	/// spacing
	const float spacing() const;

	/// check_distance
	const float check_distance() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> direction: direction <newline> spacing: spacing <newline> check_distance: check_distance <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// direction
	std::string _direction;
	
	/// spacing
	float _spacing;
	
	/// check_distance
	float _check_distance;
};


/// Use 2D maximum cost path algorithm to generate candidates
class MaxCostPath : public SegParam {
public:
	/// Constructor
	MaxCostPath();

	/// Copy constructor
	MaxCostPath(const MaxCostPath&);

	/// Destructor
	~MaxCostPath();

	/// Name of the attribute
	const char* const name() const { return "MaxCostPath"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};

/// Regions should have the specified minimum number of voxels before forming a candidate
class MinNumVoxels : public SegParam {
public:
	/// Constructor
	MinNumVoxels(const int minNum);

	/// Copy constructor
	MinNumVoxels(const MinNumVoxels&);

	/// Destructor
	~MinNumVoxels();

	/// Name of the attribute
	const char* const name() const { return "MinNumVoxels"; };

	/// Minimum number of voxels
	const int minNumVoxels() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Min_num_voxels: minNumVoxels <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// Minimum number of voxels
	int _min_num_voxels;
};

/// Indicates that a CNN model should be called using Keras with input image dimensions as specified 
class NeuralNetKeras : public SegParam {
public:
	/// Constructor
	NeuralNetKeras(const std::string& cnn_arch, const std::string& weights_path, const int image_rows, const int image_cols);

	//youngwon edited
	/// Constructor
	NeuralNetKeras(const std::string& cnn_arch, const std::string& weights_path, const int image_rows, const int image_cols, const int image_slices);

	/// Constructor
	NeuralNetKeras(const float learning_rate);

	/// Copy constructor
	NeuralNetKeras(const NeuralNetKeras&);

	/// Destructor
	~NeuralNetKeras();

	/// Name of the attribute
	const char* const name() const { return "NeuralNetKeras"; };

	/// Set CNN architecture descriptor
	void set_cnn_arch(const std::string& cnn_arch);

	/// CNN architecture descriptor
	const std::string& cnn_arch() const { return _cnn_arch; };

	/// Set path to the default (no chromosome) CNN weights file
	void set_weights_path(const std::string& weights_path);

	/// Path to the default (no chromosome) CNN weights file
	const std::string& weights_path() const { return _weights_path; };

	/// Set image rows and columns
	void set_rows_cols(const int rows, const int cols);

	/// Input image row dimension
	const int image_rows() const;

	/// Input image column dimension
	const int image_cols() const;

	//youngwon edited
	/// Input image slice dimension
	const int image_slices() const;

	/// Set number of input channels and normalizations
	//void set_channels(const int num_channels, const std::string* intensity_normalization);

	/** Adds processing to the channel with specified index.
	The format of the string should be a list of '_' separated processing steps with their parameters (also _ separated).
	If there is no channel at the index position then it is created, if needed intervening channels are created with a string of "empty".
	If the channel at the index is "empty" then the provided string replaces it, otherwise it is appended to the string with a "_" separator.
	*/
	void add_to_channel(const int channel_index, const std::string& intensity_normalization);

	/// Number of input channels (some may be "empty", which is equivalent to "nochannel" when used)
	const int num_channels() const;

	/**
	Returns intensity_normalization for jth channel (validity of j is checked only by assert).
	See add_to_channel for expected format of the normalization string.
	*/
	const std::string& intensity_normalization(const int j) const;
	
	/// For each provided bit that is True, set it to True in the member (otherwise do nothing to the member).
	void set_normalization_bits_used(const std::vector<bool> bits);

	/// Returns true if none have been set as used
	const bool no_normalization_bits_used() const;

	/**
	Returns whether chromosome index was used for normalization (if outside range then returns false).
	*/
	inline const bool normalization_bit_used(const int index) const { return (index>=0)&&(index<_normalization_bits_used.size())&&_normalization_bits_used[index]; };


	/** Adds augmentation to the stack with specified index.
	The format of the string should be a list of '_' separated augmentation steps with their parameters (also _ separated).
	If there is no stack at the index position then it is created, if needed intervening stacks are created with a string of "empty".
	If the stack at the index is "empty" then the provided string replaces it, otherwise it is appended to the string with a "_" separator.
	*/
	void add_to_augmentation(const int stack_index, const std::string& augmentation);

	/// Number of input augmentation (some may be "empty", which is equivalent to "noaugmentation" when used)
	const int num_augmentations() const;

	/**
	Returns augmentation for jth augmentation (validity of j is checked only by assert).
	See add_to_augmentation for expected format of the augmentation string.
	*/
	const std::string& augmentation(const int j) const;
	
	/// For each provided bit that is True, set it to True in the member (otherwise do nothing to the member).
	void set_augmentation_bits_used(const std::vector<bool> bits);

	/// Returns true if none have been set as used
	const bool no_augmentation_bits_used() const;

	/**
	Returns whether chromosome index was used for augmentation (if outside range then returns false).
	*/
	inline const bool augmentation_bit_used(const int index) const { return (index>=0)&&(index<_augmentation_bits_used.size())&&_augmentation_bits_used[index]; };




	/// Set learning rate
	void set_learning_rate(const float learning_rate);

	/// Learning rate
	const float learning_rate() const;

	/// Set (add or update) parameter
	void set_parameter(const std::string group, const std::string name, const std::string value);

	/// Appends a parameter value (with a comma and space separator if a value already exits
	void append_parameter(const std::string group, const std::string name, const std::string value);

	/**
	Writes parameters in an ini file format.
	output_filename is the output file.
	default_param_filename reads ini file if it exists and adds default parameters if they do not exist in the data structure.
	*/
	const void write_ini(const std::string output_filename, const std::string default_param_filename) const;

	///	Overwrites parameters in the provided ini file.
	const void set_ini(mINI::INIStructure& ini) const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> cnn_arch: <newline> image_rows: <newline> image_cols: <newline> 
	num_channels: <newline> intensity_normalization: list of each <newline> learning_rate: <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// CNN architecture descriptor
	std::string _cnn_arch;

	/// Path to the CNN weights file
	std::string _weights_path;
	
	/// Input image row dimension
	int _image_rows;

	/// Input image column dimension
	int _image_cols;

	//youngwon edited
	/// Input image slices dimension
	int _image_slices;

	/// Number of input channels
	//int _num_channels;

	/// Intensity_normalization descriptors (number of elements = _num_channels)
	std::vector<std::string> _intensity_normalization;
	//std::string* _intensity_normalization;

	/** 
	Indicates which chromosome bit numbers were used in setting normalization parameters.
	The vector is NOT guaranteed to be the length of the chromosome, any bits not included in the vector are assumed to be False, 
	i.e., if the vector is empty we assume no chromosome bits were used.
	*/
	std::vector<bool> _normalization_bits_used;

	/// Learning rate
	float _learning_rate;
	
	/// Augmentation descriptors (number of elements = _num_stacks)
	std::vector<std::string> _augmentation;
	/** 
	Indicates which chromosome bit numbers were used in setting augmentation parameters.
	The vector is NOT guaranteed to be the length of the chromosome, any bits not included in the vector are assumed to be False, 
	i.e., if the vector is empty we assume no chromosome bits were used.
	*/
	std::vector<bool> _augmentation_bits_used;

	/**
	Neural network parameters.
	For details of permitted values see the NeuralNet_Parameter model attribute.
	*/
	mINI::INIStructure _parameters;

	void _update_intensity_norm_ini_param();
	void _update_img_shape_ini_param();
	void _update_augmentation_ini_param();
};

/// Threshold plateness values to perform segmentation and region growing
class PlatenessThreshRegGrow : public SegParam {
public:
	/// Constructor
	PlatenessThreshRegGrow(const float low, const float high);

	/// Copy constructor
	PlatenessThreshRegGrow(const PlatenessThreshRegGrow&);

	/// Destructor
	~PlatenessThreshRegGrow();

	/// Name of the attribute
	const char* const name() const { return "PlatenessThreshRegGrow"; };

	/// Low value for thresholding
	const float low() const;

	/// High value for thresholding
	const float high() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Range (low high): low high <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// Low value for thresholding
	float _low;

	/// High value for thresholding
	float _high;
};

/// Use same candidates as another solution element or multiple elements
class SameCandidatesAs : public SegParam {
public:
	/// Constructor (single related solution element)
	SameCandidatesAs(const std::string& rel_solel_name, const int rel_solel_ind);

	/// Constructor (multiple related solution elements)
	SameCandidatesAs(const Darray<std::string*>& rel_solel_name, const Darray<int>& rel_solel_ind);

	/// Copy constructor
	SameCandidatesAs(const SameCandidatesAs&);

	/// Destructor
	~SameCandidatesAs();

	/// Name of the attribute
	const char* const name() const { return "SameCandidatesAs"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline>  Related SolElement: name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};


/// Perform 2D segmentation if available
class SegmentIn2D : public SegParam {
public:
	/// Constructor
	SegmentIn2D();

	/// Copy constructor
	SegmentIn2D(const SegmentIn2D&);

	/// Destructor
	~SegmentIn2D();

	/// Name of the attribute
	const char* const name() const { return "SegmentIn2D"; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};


/// Attenuation threshold range in gray levels (GL)
class ThreshRangeGL : public SegParam {
public:
	/// Constructor
	ThreshRangeGL(const int lowGL, const int highGL);

	/// Copy constructor
	ThreshRangeGL(const ThreshRangeGL&);

	/// Destructor
	~ThreshRangeGL();

	/// Name of the attribute
	const char* const name() const { return "ThreshRangeGL"; };

	/// Low GL threshold limit
	const float lowGL() const;

	/// High GL threshold limit
	const float highGL() const;

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> GL_range: low_GL to high_GL <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// Low GL threshold limit
	float _low_GL;

	/// High GL threshold limit
	float _high_GL;
};


/// Use subsampled image for segmentation if available
class UseSubSampledImage : public SegParam {
public:
	/// Constructor
	UseSubSampledImage(const int x_step, const int y_step, const int z_step);

	/// Copy constructor
	UseSubSampledImage(const UseSubSampledImage&);

	/// Destructor
	~UseSubSampledImage();

	/// Name of the attribute
	const char* const name() const { return "UseSubSampledImage"; };

	/// Return x-step for image subsampling
	const int x_step() const { return _x_step; };

	/// Return y-step for image subsampling
	const int y_step() const { return _y_step; };

	/// Return z-step for image subsampling
	const int z_step() const { return _z_step; };

	/**
	Write segmentation parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// X-step for image subsampling
	int _x_step;

	/// Y-step for image subsampling
	int _y_step;

	/// Z-step for image subsampling
	int _z_step;
};

#endif // !__SegParam_h_

