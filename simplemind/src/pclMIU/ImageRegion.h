#ifndef __ImageRegion_h_
#define __ImageRegion_h_

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
#include "MedicalImageSequence.h"
#include "ROItraverser.h"
#include "ImagePrimitive.h"
#include "FPoint.h"
#include "tools_miu.h"

using std::ostream;

/**
An ROI image primitive.
An ROI primitive without any points is not allowed.
This class maintains some internal values derived from the ROI (e.g. centroid). Therefore these values must be updated internally any time the ROI is changed.
*/
class ImageRegion : public ImagePrimitive {
public:

	/**
	Constructor.
	An ROI primitive without any points is not allowed, program exits with error message if this is attempted.
	*/
	ImageRegion(const ROI&, const MedicalImageSequence&);

	/// Copy constructor
	ImageRegion(const ImageRegion&);

	/// Destructor
	~ImageRegion();

	/// Type of primitive
	const char* const type() const { return "ImageRegion"; };

	/// Creates a copy of the region, with separate memory allocated
	ImagePrimitive* create_copy() const;	

	/// Creates a subsampled version of the primitive
	ImagePrimitive* create_subsampled_prim(const MedicalImageSequence& orig_image, const MedicalImageSequence& subsampled_image) const;	

	/// Return the ROI
	inline const ROI& roi() const { return _roi; };

	/// Return the centroid
	inline const FPoint& centroid() const { return _centroid; };

	/// Returns a pointer to the centroid of a given plane z (returns 0 if the plane contains points)
	const FPoint* centroid(const int z) const;

	/// Return the total area in the xy-plane(s) (in mm3)
	inline const float area_xy() const { return _area_xy; };

	/// Return the volume (in mm3)
	inline const float volume() const { return _volume; };

	/**
	Performs a z-coordinate translation of the primitive.
	Intended to make the primitive compliant with ImageInstanceNumber in the dicom header rather than based on images indexed from zero.
	The assumption is that the ImageInstanceNumbers are continuous, if not use the method map_inst_nums.
	Use translate_to_inst_nums where possible since it is faster than map_to_inst_nums.
	*/
	void translate_z(const int offset);
	//void translate_to_inst_nums(const int offset);
	
	/**
	Performs a z-coordinate mapping of the primitive.
	Intended to make the primitive compliant with ImageInstanceNumber in the dicom header rather than based on images indexed from zero.
	The argument array should contain one element per image giving the corresponding instance number when the z-coordinate of the primitive is supplied as index into the array.
	*/
	void map_to_inst_nums(const int* im_inst_nums);

	/**
	Write primitive to an output stream.
	Format is:
	Type_name
	ROI
	_centroid
	FPoint FPoint.... FPoint (centroids on each plane - planar centroids)
	_area_xy _volume
	*/
	void write(ostream& s) const;

	const std::string extension() const
	{
		return ".roi";
	}

	void writeEssentialOnly(ostream& s) const;

protected:
	/**
	Accepts an array of image regions and returns a pointer to a newly allocated region which is a combination of this region and those provided.
	The type of all the supplied primitives must be the same, if this is not the case then program exits with an error message.
	*/
	ImagePrimitive* _combine(Darray<ImagePrimitive*>&, const MedicalImageSequence&) const;

private:
	/*
	/// Default constructor
	ImageRegion() {};
	*/
	
	/*
	Creates a copy of the primitive that is optimized for low memory consumption, but should not be modified - intended to be called within the create_matched_prim method.
	Modifications of the primitive copy may be greatly suboptimal, but querying the copy should be uneffected.
	*/
	/*
	ImagePrimitive* _low_memory_copy() const;
	*/

	/// The ROI
	ROI _roi;

	/// The centroid of the ROI
	FPoint _centroid;

	/// Maximum z-coordinate of points in the ROI
	int _max_z;

	/**
	Array of pointers to centroids computed from points in a given plane.
	The number of elements in the array is _max_z+1.
	The index of each element corresponds to the z-coordinate of the plane from which the centroid was computed.
	Element (pointer) is set to zero if there are no points in the corresponding plane.
	*/
	FPoint** _planar_centroids;

	/// The total area of the ROI in the xy-plane(s) in mm2
	float _area_xy;

	/// The volume of the ROI in mm3
	float _volume;
};


#endif // !__ImageRegion_h_

