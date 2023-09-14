#ifndef __Model_h_
#define __Model_h_

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
#include <fstream>
#include "AnatPathEntity.h"
#include "tools_miu.h"

/// Stores anatomical/pathological/imaging model
class Model {
public:

	/// Constructor
	Model(const char* const file);

	/// Constructor
	Model(const char* const file, const char* const chromosome);
	//Model(const char* const file, const char* const roi_directory, const char* const chromosome);

	/// Copy constructor
	Model(const Model&);

	/// Destructor
	virtual ~Model();

	/// Returns the name of the model
	const std::string& file() const { return _file; };

	/**
	Returns the directory from which ROIs are to be read and used to pre-populate match primitives.
	May have no characters.
	*/
	const std::string& roi_directory() const { return _roi_directory; };

	/**
	Returns the chromosome of the model.
	May have no characters (bits).
	*/
	const std::string& chromosome() const { return _chromosome; };

	/**
	Adds an entity to the Model.
	Returns 1 if successful.
	If entity with same name already exists, then nothing is done and 0 is returned.
	*/
	const int add_entity(const AnatPathEntity&);

	/**
	Adds an entity to the Model (entity is prepended to array).
	Returns 1 if successful.
	If entity with same name already exists, then nothing is done and 0 is returned.
	*/
	const int add_entity_first(const AnatPathEntity&);
	
	/**
	Read from file.
	*/
	virtual void read();

	/**
	Writes model files within a specified directory.
	The path should be the full path (INCLUDING a / at the end), since the files are written directly into the given directory (with no allowance for version number etc.).
	*/
	virtual void write(const char* const full_path) const;

	/// Returns number of entities in the Model
	const int n() const;

	/**
	Returns the i'th entity in the Model.
	Program exits if i is invalid.
	*/
	AnatPathEntity& entity(const unsigned int i) const;

	/**
	Returns the index of the named entity in the Model.
	Returns -1 if entity with given name cannot be found.
	*/
	const int entity_index(const char* const name) const;

	// Output stream operator
	friend ostream& operator<<(ostream& s, const Model& a);

protected:
	/**
	Parses a main model input file and appends the names of the entities listed in the file to the array of strings.
	The filename should be fully specified, i.e. full path.
	*/
	void _parse_model_file(const std::string& model_filename, Darray<std::string>& entity_names) const;

private:

	/// Name of the model
	std::string _file;

	/// Directory from which ROIs will be read and used to set the matched primitive
  	std::string _roi_directory;

	/// Chromosome represented as string of bits
  	std::string _chromosome;

  	/// Vector of flags to indicate whether each chromosome bit has been used
  	//std::vector<bool> _bit_used;

	/// Vector of anatomically or pathologically-based entities
  //Darray<AnatPathEntity> _entity;
  std::vector<AnatPathEntity> _entity;
};

ostream& operator<<(ostream& s, const Model& m);


///**
//Stores patient-specific model.
//An identifier (e.g. patient ID) is prepended to all filenames associated with the patient-specific model to distinguish it from models associated with other patients.
//*/
//class PSmodel : public Model {
//public:
//
//	/// Constructor
//	PSmodel(const char* const model_name, const char* const identifier, const int version_num=1);
//
//	/// Constructor that initializes a patient-specific model from a generic model
//	PSmodel(const Model& m, const char* const identifier);
//
//	/// Copy constructor
//	PSmodel(const PSmodel&);
//
//	/// Destructor
//	~PSmodel();
//
//	/**
//	Read from file.
//	It is assumed that within the directory specified by path (e.g. /home/mbrown/model), there will be a directory called model_name.version_num, e.g. kidney.1, which in turn contains a subdirectory called PSmodels.
//	Within this directory the main model file should be called identifier.model_name, and the files for the AnatPathEntities should also be in this directory, each of the form identifier.entity_name.
//	Program exits with message to cerr if any file cannot be opened.
//	The format of the main model file is as follows: model_name <newline> version_number <newline> AnatPathEntity Filename <newline> AnatPathEntity Filename <newline> AnatPathEntity Filename <newline>.
//	The entity filenames in the main model file should not include the prepended identifier.
//	*/
//	void read(const char* const path);
//
//	/**
//	Write patient-specific model to files.
//	It is assumed that within the directory specified by path (e.g. /home/mbrown/model), there will be a directory called model_name.version_num, e.g. kidney.1, which in turn contains a subdirectory called PSmodels.
//	The resulting files will be written to this directory. The main model file will be called identifier.model_name, and the files for the AnatPathEntities will have names of the form identifier.entity_name.
//	Program exits with message to cerr if any file cannot be opened.
//	The format of the main model file is as follows: model_name <newline> version_number <newline> AnatPathEntity Filename <newline> AnatPathEntity Filename <newline> AnatPathEntity Filename <newline>.
//	The entity filenames in the main model file will not include the prepended identifier.
//	*/
//	void write(const char* const path) const;
//
//private:
//
//	/// An identifier is prepended to all filenames associated with the patient-specific model to distinguish it from models associated with other patients.
//  std::string _identifier;
//};

#endif // !__Model_h_
