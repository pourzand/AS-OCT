#ifndef __ImagePrimitive_h_
#define __ImagePrimitive_h_

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

#include "Darray.h"
#include "MedicalImageSequence.h"

#include <ostream>
using std::ostream;


/// A virtual base class for image primitives.
class ImagePrimitive {
public:

	/// Constructor
	ImagePrimitive();

	/// Copy constructor
	ImagePrimitive(const ImagePrimitive&);

	/// Destructor
	virtual ~ImagePrimitive();

	/// Type of the primitive (ImageRegion, ImageContour, etc)
	virtual const char* const type() const = 0;

	/**
	Creates a copy of a primitive, with separate memory allocated.
	"Formed from" information is not copied.
	*/
	virtual ImagePrimitive* create_copy() const = 0;

	/// Creates a subsampled version of the primitive
	virtual ImagePrimitive* create_subsampled_prim(const MedicalImageSequence& orig_image, const MedicalImageSequence& subsampled_image) const = 0;

	/**
	Creates the matched primitive for a solution element.
	Accepts a pointer to an array of image primitives and returns a pointer to a newly allocated primitive which is a combination of this primitive and those provided.
	The type of all the supplied primitives must be the same, if this is not the case then program exits with an error message.
	*/
	ImagePrimitive* create_matched_prim(Darray<ImagePrimitive*>&, const MedicalImageSequence&) const;

	/**
	Performs a z-coordinate translation of the primitive.
	Intended to make the primitive compliant with ImageInstanceNumber in the dicom header rather than based on images indexed from zero.
	The assumption is that the ImageInstanceNumbers are continuous, if not use the method map_inst_nums.
	Use translate_to_inst_nums where possible since it is faster than map_to_inst_nums.
	*/
	virtual void translate_z(const int offset) = 0;
	//virtual void translate_to_inst_nums(const int offset) = 0;

	/**
	Performs a z-coordinate mapping of the primitive.
	Intended to make the primitive compliant with ImageInstanceNumber in the dicom header rather than based on images indexed from zero.
	The argument array should contain one element per image giving the corresponding instance number when the z-coordinate of the primitive is supplied as index into the array.
	*/
	virtual void map_to_inst_nums(const int* im_inst_nums) = 0;

	/// Write primitive to an output stream
	virtual void write(ostream& s) const = 0;

	virtual const std::string extension() const = 0;

	virtual void writeEssentialOnly(ostream& s) const = 0;


protected:
	/**
	Accepts an array of image primitives and returns a pointer to a newly allocated primitive which is a combination of this primitive and those provided.
	The type of all the supplied primitives must be the same, if this is not the case then program exits with an error message.
	*/
	virtual ImagePrimitive* _combine(Darray<ImagePrimitive*>&, const MedicalImageSequence&) const = 0;

private:
	/*
	Creates a copy of the primitive that is optimized for low memory consumption, but should not be modified - intended to be called within the create_matched_prim method.
	Modifications of the primitive copy may be greatly suboptimal, but querying the copy should be uneffected.
	*/
	/*
	virtual ImagePrimitive* _low_memory_copy() const = 0;
	*/

	/*
	/// Pointers to primitives from which this primitive was formed (only set for the matched primitive of a solution element)
	Darray<ImagePrimitive*> _formed_from;
	*/
};


#endif // !__ImagePrimitive_h_

