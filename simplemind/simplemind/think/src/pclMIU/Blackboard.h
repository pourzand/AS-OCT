#ifndef __Blackboard_h_
#define __Blackboard_h_

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

#include <string>

#include "Model.h"
#include "ImageRegion.h"
#include "Attribute.h"
#include "MedicalImageSequence.h"
#include "ROI.h"


/// A candidate image primitive for matching to a model entity.
class ImageCandidate {
public:
	/**
	Constructor - memory for the ImagePrimitive must be allocated outside the constructor, and is then destroyed internal to the class.
	Must specify the number of Attributes of the SolElement to which the ImageCandidate belongs.
	The number of attributes must be >= 0, otherwise program exits with error message.
	*/
	ImageCandidate(ImagePrimitive* prim, const int num_attributes);

	/// Copy constructor - pointer to image primitive is shared (not new copy)
	ImageCandidate(const ImageCandidate&);

	/// Destructor
	~ImageCandidate();

	/// Number of attributes
	inline const int num_attributes() const { return _num_attributes; };

	/**
	Sets a confidence score for an attribute and updates the partial confidence score.
	If the attribute index is invalid, program exits with an error message.
	*/
	void conf_score(const int attribute_index, const float score);

	/**
	Get confidence score of i'th  attribute.
	Program exits if i is invalid.
	*/
	const float conf_score(const int attribute_index) const;

	/**
	Sets a feature value for the image candidate.
	If the feature index is invalid, program exits with an error message.
	*/
	void feature_value(const int feature_index, const float value);

	/**
	Get value of i'th  feature.
	Program exits if i is invalid or if corresponding confidence score is 2.0 => feature value not computed.
	*/
	const float feature_value(const int feature_index) const;

	/// Get image primitive
	inline ImagePrimitive* primitive() const { return _primitive; };

	/// Returns partial confidence score for the image primitive as the minimum of the confidence scores set via the conf_score method
	inline const float partial_confidence() const { return _partial_conf; };

	/**
	Writes to an output file stream.
	Format is:
	ImagePrimitive
	_num_attributes _feature_value _confidence _feature_value _confidence ....  _feature_value _confidence
	_partial_conf
	*/
	void write(ostream& s) const;

private:
	/// Candidate image primitive for matching to a model entity
	ImagePrimitive* _primitive;

	/// Number of attributes
	int _num_attributes;

	/**
	Array of attribute values, one per attribute (each value is only set when corresponding element of _confidence is set).
	Therefore elements should only be accessed if the corresponding element of _confidence is not equal to 2.0 => only set for feature attributes.
	*/
	float* _feature_value;

	/**
	Array of confidence scores, one per attribute (initialized to 2).
	This array only holds confidence scores computed by the ImCandConf knowledge source, i.e. confidences may not be computed for all features (see ImCandConf documentation).
	Elements of this array are initialized to 2.0 => can test whether a feature has been analyzed or not, and if the corresponding attribute is not a Feature it has no effect on the overall confidence.
	Set to zero if no attributes.
	*/
	float* _confidence;

	/**
	Partial confidence score (initialized to 1).
	Keeps track of the minimum of the confidence scores stored in _confidence.
	Updated by the conf_score method.
	*/
	float _partial_conf;
};


/// Blackboard solution element
class SolElement {
public:

	/// Constructor
	SolElement(const std::string& name);

	/// Copy constructor - does not make full copy (only name)
	SolElement(const SolElement&);

	/// Destructor
	~SolElement();

	/// Returns name if solution element
	const std::string& name() const;

	/**
	Add attribute - memory for the Attribute must be allocated outside the constructor, and is then managed (destroyed) internal to the class.
	*/
	void add_attribute(Attribute*);

	/// Number of attributes
	inline const int num_attributes() const { return _attribute.N(); };

	/**
	Get i'th attribute.
	Program exits if i is invalid.
	*/
	const Attribute* const attribute(const int i) const;

	/// Find first attribute with specified name, return 0 if not successful
	const Attribute* const find_attribute(const char* const att_name) const;
	
	/// Find first attribute with specified name, return 0 if not successful
	Attribute* find_update_attribute(const char* const att_name);

	/**
	Add image candidate - memory for the ImagePrimitive must be allocated outside the constructor, and is then destroyed internal to the class.
	*/
	void add_candidate(ImagePrimitive* prim);

	/// Number of candidates
	inline const int num_candidates() const { return _candidate.N(); };

	/**
	Get i'th candidate.
	Program exits if i is invalid.
	*/
	ImageCandidate* candidate(const int i);

	/**
	Appends the index of a matched candidate to the array of indices.
	If index is invalid program exits with error message.
	*/
	void append_matched_cand_index(const int index);

	/**
	Returns 1 if primitive has been matched to this solution element (0 otherwise), and sets argument to be the number of candidates used to generate this matched primitive. If candidates have been freed, then the argument will be set to zero, even if matching has been done (and 1 is returned).
	*/
	const int num_matched_cands(int& num) const;

	/**
	Get the index of the i'th matched candidate.
	Program exits with error message if i is invalid.
	*/
	const int matched_cand_index(const int i) const;

	/**
	Creates copy of ImagePrimitive which has been matched to the solution element.
	If multiple candidates matched then this primitive is a combination of all those matched.
	*/
	void create_matched_prim(const MedicalImageSequence&);

	/**
	Returns pointer to copy of ImagePrimitive which has been matched to the solution element.
	If multiple candidates matched then this primitive is a combination of all those matched.
	Pointer is 0 if create_matched_prim has not been called or if no candidates matched.
	*/
	inline ImagePrimitive* matched_prim() { return _matched_prim; };

	/// Frees candidates
	void free_candidates();

	/**
	Write solution element to a stream.
	Format:
	SolElement: Name
	Attribute
	Attribute....
	Attribute
	Num_Cands
	ImageCandidate
	ImageCandidate....
	ImageCandidate
	num_matched_cand_indices index index ... index
	1 or 0 (depending on whether matched primitive exists)
	ImagePrimitive (if previous element was 1)
	End: SolElement:
	*/
	void write(ostream& s) const;

	/// Output stream operator
	friend ostream& operator<<(ostream& s, const SolElement& e);

private:
	/// Name of solution element
  std::string _name;

	/**
	Vector of pointers to attributes.
	Memory for each Attribute is allocated outside class but destroyed by the class.
	*/
	Darray<Attribute*> _attribute;

	/**
	Vector of pointers to image candidates.
	Memory for each ImageCandidate is managed internal to the SolElement class, except for the ImagePrimitive associated with the candidate which is allocated outside the class but destroyed internally.
	*/
	Darray<ImageCandidate*> _candidate;

	/// Array of indices of image candidates matched to the solution element
	Darray<int> _matched_cand_index;

	/**
	Copy of ImagePrimitive which has been matched to the solution element.
	Memory for this copy is allocated separate to the image candidates.
	If multiple candidates matched then this primitive is a combination of all those matched.
	Pointer is 0 if no candidates matched.
	*/
	ImagePrimitive* _matched_prim;

	/*
	/// Number of candidates matched to this solution element - only set when num_matched_cands method is called (if _matched_cand_index darray has more than 0 elements)
	int _num_matched_cands;
	*/
};

ostream& operator<<(ostream& s, const SolElement& e);

/// A group of candidate image primitives for matching to a group of solution elements
class GroupCandidate {
public:
	/**
	Constructor.
	The number of solution elements in the group must be specified.
	*/
	GroupCandidate(const int num_solels);

	/// Copy constructor
	GroupCandidate(const GroupCandidate&);

	/// Destructor
	~GroupCandidate();

	/// Number of solution elements involved in the group candidate
	inline const int num_sol_els() const { return _num_solels; };

	/**
	Set index of image candidate for i'th  solution element of the group.
	Program exits with error message if i is invalid.
	*/
	void im_cand_index(const int i, const int im_cand_ind);

	/**
	Get index (within the appropriate solution element) of image candidate for i'th  solution element of the group.
	Image candidate index is returned as -1 if no image candidate has been assigned.
	Program exits with error message if i is invalid.
	*/
	const int im_cand_index(const int i) const;

	/// Set overall confidence score of group
	inline void conf_score(const float score) { _confidence = score; };

	/// Get overall confidence score of group
	inline const float conf_score() const	{ return _confidence; };

private:
	/// Number of solution elements in the group
	int _num_solels;

	/**
	Indices (in the respective solution elements) of image candidates of the group.
	There should be one candidate per solution element in the group.
	The array values are initialized to -1 => no image candidate found.
	*/
	//int* _im_cand_index;
	std::vector<int> _im_cand_index;

	/// Overall confidence score of group (initialized to 1)
	float _confidence;
};


/// Bi-directionally related group of solution elements
class SEgroup {
public:

	/// Constructor
	SEgroup();

	/// Copy constructor
	SEgroup(const SEgroup&);

	/// Destructor
	~SEgroup();

	/// Set priority used for determining when a group should be identified [0.0, 1.0] or -1.0 after group has been processed
	void priority(const float);

	/// Get priority used for determining when a group should be identified [0.0, 1.0] or -1.0 after group has been processed
	const float priority() const;

	/**
	Add solution element.
	There is no checking as to whether the solution element has already been assigned to another group.
	This method can only be called when the number of group candidates is zero.
	If this condition is violated the program exits with an error message.
	*/
	void add_sol_el(const int bb_index);

	/// Number of solution elements
	inline const int num_sol_els() const { return _sol_el_index.N(); };

	/**
	Get index (in blackboard) of i'th solution element of the group.
	Program exits if i is invalid.
	*/
	const int sol_el_index(const int i) const;

	/// Finds the index (within the group) of the solution element with the given blackboard index (returns -1 if not successful)
	const int find_solel_grp_ind(const int solel_index_bb) const;

	/// Returns 1 if solution element with the given index (blackboard) is in the group, and 0 otherwise
	const int in_group(const int sol_el_index) const;

	/// Add a group candidate
	void add_group_cand(const GroupCandidate&);

	/// Number of group candidates
	inline const int num_group_cands() const { return _candidate.N(); };

	/**
	Access i'th group candidate.
	Program exits with error message if i is invalid.
	*/
	GroupCandidate& group_cand(const int i);

	/// Frees candidates
	void free_candidates();

private:
	/// Priority used for determining when a group should be identified (0.0, 1.0) or -1.0 after group has been processed. Initialized to 0.0 when group is constructed.
	float _priority;

	/// Array of (blackboard) indices of solution elements in the group.
	Darray<int> _sol_el_index;

	/// Array of pointers to group candidates.
	Darray<GroupCandidate> _candidate;

};

//ostream& operator<<(ostream& s, const SEgroup& g);

/// Record of knowledge sources activated
class ActivationRecord {
public:
	/**
	Constructor.
	Requires name and type of the knowledge source.
	*/
	ActivationRecord(const std::string& name, const std::string& type);

	/// Copy constructor
	ActivationRecord(const ActivationRecord&);

	/// Destructor
	~ActivationRecord();

	/// Returns the name of the knowledge source
	const std::string& ks_name() const;

	/// Returns the type of the knowledge source
	const std::string& ks_type() const;

	/// Record a message
	void message(const std::string& mess);

	/// Number of messages
	const int num_messages() const;

	/**
	Read message number i.
	Program exits if i is invalid.
	*/
	const std::string& message(const int i) const;

	/**
	Returns index of first message matching the given string.
	Returns -1 if no matching message found.
	*/
	const int find_message(const std::string&) const;

	/**
	Returns index of first message starting with the given string.
	Returns -1 if no matching message found.
	*/
	const int find_message_starting_with(const std::string&) const;

	/// Output stream operator
	friend ostream& operator<<(ostream& s, const ActivationRecord& a);

private:
	/// Name of the knowledge source
  std::string _name;

	/// Type of the knowledge source
  std::string _type;

	/// Messages
	Darray<std::string> _message;
};

ostream& operator<<(ostream& s, const ActivationRecord& a);

/// Blackboard
class Blackboard {
public:
	/**
	Constructor.
	Overall search area set to entire image sequence.
	*/
	Blackboard(MedicalImageSequence&, Model&, const char* const image_path, const char* const exec_path, const char* const temp_file_path);

	/**
	Constructor.
	2nd last argument is overall search area.
	Last argument is subsampled version of the image data.
	*/
	Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, const char* const temp_file_path/*, MedicalImageSequence* ss_ms=0*/);

	/**
	Constructor.
	3rd last argument is overall search area.
	2nd last argument is input directory for ROIs.
	Last argument is input directory for EDMs.
	*/
	Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, const char* const temp_file_path, const char* const roi_directory, const char* const edm_directory, const char* const stop_at_node);

	/**
	Constructor.
	6th last argument is overall search area.
	5th last argument is input directory for ROIs.
	4th last argument is input directory for EDMs.
	3rd last argument is user resource configuration directory for CNNs.
	2nd last argument is switch to turn off / on generating png normalized input image for CNN.
	Last argument is switch to turn off / on generating tensorboard log for CNN.
	*/
	Blackboard(MedicalImageSequence& ms, Model& mod , const ROI& s_area, const char* const image_path, const char* const exec_path, 
	            const char* const temp_file_path, const char* const roi_directory, const char* const edm_directory, 
				const char* const stop_at_node, const char* const user_resource_directory, const char* const condor_job_directory,
				const bool skip_normalized_image_png, const bool skip_normalized_image_png_training, const bool skip_tensorboard_logging, const bool predict_cpu_only);

	/// Destructor
	~Blackboard();

	/// Returns model
	const Model& model() const;

	/// Returns MedicalImageSequence
	inline MedicalImageSequence& med_im_seq() { return _mis; };

	/**
	Returns HU values in column-major order.
	1D array with size xdim*ydim*zdim.
	Will compute from MedicalImageSequence the first time it is called (so will be slow the first time).
	*/
	//const short* const hu_values_column_major();

	/// Returns path to the input image files.
	const char* const image_path() const;

	/// Returns path for storing temporary files needed during computation.
	const char* const temp_file_path() const;

	/**
	Returns path for input ROIs.
	String has length zero if roi input directory not defined.
	*/
	const std::string roi_directory() const;

	/**
	Returns path for miu executable.
	*/
	const std::string exec_directory() const;

	/**
	Returns path for input EDMs.
	String has length zero if edm input directory not defined.
	*/
	const std::string edm_directory() const;

	/**
	Returns name of node at which processing should stop.
	String has length zero if node not specified.
	*/
	const std::string stop_at_node() const;

	/**
	Returns path for user resource configuration directory.
	String has length zero if user resource configuration directory not defined.
	*/
	const std::string user_resource_directory() const;

	/**
	Returns path for condor job and resource configuration directory.
	String has length zero if condor job and resource configuration directory not defined.
	*/
	const std::string condor_job_directory() const;
	
	/**
	Returns boolean for predict_cpu_only
	*/
	const bool predict_cpu_only() const;

	/**
	Returns boolean for skipping png image
	*/
	const bool skip_normalized_image_png() const;

	/**
	Returns boolean for skipping png image for training phase
	*/
	const bool skip_normalized_image_png_training() const;

	/**
	Returns boolean for skipping png image
	*/
	const bool skip_tensorboard_logging() const;

	/*
	Returns pointer to subsampled MedicalImageSequence (0 if not defined)
	inline MedicalImageSequence* ss_med_im_seq() { return _ss_mis; };
	*/

	/// Add a solution element to the blackboard
	void add_sol_element(const std::string& name);

	/// Returns number of solution elements
	const int num_sol_elements() const;

	/**
	Access a solution element.
	Program exits with error message if i is invalid.
	*/
	SolElement& sol_element(const int i);

	/**
	Access a solution element with a given name (using simple forward search).
	Returns 0 if element cannot be found.
	*/
	SolElement* sol_element(const std::string& name);

	/**
	Get index of a solution element with a given name (using simple forward search).
	Returns -1 if element cannot be found.
	*/
	const int sol_element_index(const std::string& name) const;

	/**
	Access the last solution element added to the blackboard.
	Program exits with error if there are no activation records.
	*/
	SolElement& last_sol_element();

	
	/**
	Adds indices of Solution Elements on which the element with the given element depends.
	The indices will be unique, but no ordering is applied.
	*/
	void add_ancestors(const SolElement& se, std::vector<int>& anc);

	/// Returns number of groups
	const int num_groups() const;

	/**
	Access the i'th group.
	Program exits with error if index is invalid.
	*/
	SEgroup& group(const int i);

	/**
	Provides access to the i'th group as a constant.
	Program exits with error if index is invalid.
	*/
	const SEgroup& group_const(const int i) const;

	/**
	Provides access to the i'th group as a constant.
	Program exits with error if index is invalid.
	*/
	const SEgroup& get_group(const int i) const;

	/**
	Get index of group containing solution element with index solel_ind.
	Returns -1 if group cannot be found.
	*/
	const int find_group(const int solel_ind) const;

	/**
	Sets the index of the next group to be processed (determined by the scheduler).
	Program exits with error message if index is invalid.
	*/
	void next_group(const int index);

	/**
	Returns the index of the next group to be processed (determined by the scheduler).
	-1 if undefined.
	*/
	inline const int next_group() const { return _next_group; };

	/**
	Sets the index of the next solution element to be processed (determined by the scheduler).
	Program exits with error message if index is invalid.
	*/
	void next_solel(const int index);

	/**
	Returns the index of the next solution element to be processed (determined by the scheduler).
	-1 if undefined.
	*/
	inline const int next_solel() const { return _next_solel; };

	/**
	Append a new activation record.
	Requires name and type of the knowledge source.
	*/
	void append_act_rec(const std::string& name, const std::string& type);

	/**
	Adds a message to the last activation record.
	Program exits with error if there are no activation records.
	*/
	void add_message_to_last_act_rec(const std::string& mess);

	/// Returns number of activation records
	const int num_act_recs() const;

	/**
	Get i'th activation record.
	Program exits with error message if i is invalid.
	*/
	const ActivationRecord& act_rec(const int i);

	/**
	Returns a pointer to a particular activation record.
	The name, type and message within the activation record should be provided.
	A record may have multiple messages and only one of them has to match the argument.
	The list of activation records is searched starting from the most recent and the first match is returned.
	Returns 0 if there are no matching activation records.
	*/
	const ActivationRecord* const find_act_rec(const std::string& name, const std::string& type, const std::string& message) const;

	/**
	Returns a pointer to the last activation record.
	Returns 0 if there are no activation records.
	*/
	const ActivationRecord* const last_act_rec() const;

	/**
	Write solution elements to an output stream operator.
	Format of output is: Number of SolElements: XX <newline> followed by solution elements in their ouptut format - using their write method as opposed to output stream operator.
	*/
	void write_sol_elements(ostream& s);

	/**
	Write groups to an output stream operator.
	Format of output is: Group: group# <newline> name of member (solution element) <newline> name of member (solution element) End: Group: group#.
	*/
	void write_groups(ostream& s);

	/**
	Write activation records to an output stream operator.
	Format of output is: Number of ActivationRecords: XX <newline> followed by solution elementsin their ouptut format.
	*/
	void write_act_recs(ostream& s);

	/// Set overall search area
	void overall_search_area(const ROI&);

	/// Get overall search area
	const ROI& overall_search_area() const;

	/// Form groups of solution elements
	friend void GroupFormerA(Blackboard&);

	// Output stream operator (for writing to a file)
	//friend ostream& operator<<(ostream& s, const Blackboard& bb);

private:
	/// Reference to Image
	MedicalImageSequence& _mis;

	/**
	HU values derived from _mis.
	Initialized to zero.
	*/
	//short* _hu_values_column_major;

	/// Reference to Model
	Model& _model;

	/*
	Pointer to Subsampled Image (0 if not defined)
	MedicalImageSequence* _ss_mis;
	*/

	/// Solution Elements
	Darray<SolElement> _solel;

	/// Records of knowledge sources activated
	Darray<ActivationRecord> _actrec;

	/// Solution Element groups;
	Darray<SEgroup> _group;

	/// Overall search area for segmentation
	ROI _overall_search_area;

	/**
	Index of next group to be processed (as determined by the Scheduler).
	Initialized to -1 => not defined.
	*/
	int _next_group;

	/**
	Index of next solution element to be processed (as determined by the Scheduler).
	Initialized to -1 => not defined.
	*/
	int _next_solel;

	/**
	Path for storage of temporary files needed during computation.
	*/
	char* _temp_file_path;

	/**
	Path for execution directory (where miu executable is located).
	*/
	std::string _exec_directory;

	/**
	Path for input ROI directory.
	*/
	std::string _roi_directory;

	/**
	Path for input EDM directory.
	*/
	std::string _edm_directory;
	
	/**
	Path for user resource configuration directory.
	*/
	std::string _user_resource_directory;
	
	/**
	Path for condor job and resource configuration directory.
	*/
	std::string _condor_job_directory;
	
	/**
	Boolean for predict_cpu_only
	*/
	bool _predict_cpu_only;

	/**
	Boolean for skipping png image
	*/
	bool _skip_normalized_image_png;

	/**
	Boolean for skipping png image for training phase
	*/
	bool _skip_normalized_image_png_training;

	/**
	Returns boolean for skipping png image
	*/
	bool _skip_tensorboard_logging;

	/**
	Name of node at which to stop processing.
	*/
	std::string _stop_at_node;

	/**
	Path for input image.
	*/
	char* _image_path;
};

ostream& operator<<(ostream& s, const Blackboard& bb);

#endif // !__Blackboard_h_

