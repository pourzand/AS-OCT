#ifndef __ImageContour_h_
#define __ImageContour_h_

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

#include <ostream>
#include "Contour.h"
#include "ImagePrimitive.h"

using std::ostream;

/// A Contour image primitives
class ImageContour : public ImagePrimitive {
public:

	/// Constructor
	ImageContour(const Contour&);

	/// Copy constructor
	ImageContour(const ImageContour&);

	/// Destructor
	~ImageContour();

	/// Type of primitive
	const char* const type() const { return "ImageContour"; };	

	/// Creates a copy of the contour, with separate memory allocated
	ImagePrimitive* create_copy() const;	

	/// Return the Contour
	const Contour& contour() const;

	/// NOT WRITTEN: Creates a subsampled version of the contour
	ImagePrimitive* create_subsampled_prim(const MedicalImageSequence& orig_image, const MedicalImageSequence& subsampled_image) const;

	/// NOT WRITTEN
	void translate_z(const int offset);
	//void translate_to_inst_nums(const int offset);

	/// NOT WRITTEN
	void map_to_inst_nums(const int* im_inst_nums);

	/// NOT WRITTEN: Write primitive to an output stream
	void write(ostream& s) const;

	virtual const std::string extension() const
	{
		std::cerr << "ERROR: ImageContour: extension() method not implemented" << std::endl;
		exit(1);
		return "";
	}

	virtual void writeEssentialOnly(ostream& s) const
	{
		std::cerr << "ERROR: ImageContour: writeEssentialOnly() method not implemented" << std::endl;
		exit(1);
	}

protected:
	/**
	METHOD NOT WRITTEN.
	Accepts an array of image contours and returns a pointer to a newly allocated contour which is a combination of this contour and those provided.
	The type of all the supplied primitives must be the same, if this is not the case then program exits with an error message.
	*/
	ImagePrimitive* _combine(Darray<ImagePrimitive*>&, const MedicalImageSequence&) const;


private:
	/*
	METHOD NOT WRITTEN.
	Creates a copy of the primitive that is optimized for low memory consumption, but should not be modified - intended to be called within the create_matched_prim method.
	Modifications of the primitive copy may be greatly suboptimal, but querying the copy should be uneffected.
	*/
	/*
	ImagePrimitive* _low_memory_copy() const;
	*/

	Contour _cont;
};


#endif // !__ImageContour_h_

