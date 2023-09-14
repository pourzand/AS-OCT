#ifndef __SearchArea_h_
#define __SearchArea_h_

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

#include <string.h>

#include "Attribute.h"
#include "ImageRegion.h"
#include "MedicalImageSequence.h"
#include "ROIdescription.h"

//const double PI_DBL = fabs(atan2(0,-1));

/// An abstract base class for storing knowledge to derive search areas
class SearchArea : public Attribute {
public:

	/// Constructor
	SearchArea(const int e_flag);

	/// Copy constructor
	SearchArea(const SearchArea&);

	/// Destructor
	virtual ~SearchArea();

	/// Type of attribute
	const char* const type() const { return "SearchArea"; };

	/// Sets OR flag to 1
	inline void set_or_flag() { _or_flag=1; };

	/// Returns value of or flag, 1 if OR should be used to combine search area, 0 otherwise
	inline const int or_flag() const { return _or_flag; };

	/**
	If the derived type of the image primitive is appropriate the ROI in the referenced argument val is modified and 1 is returned, otherwise 0 is returned and val is not modified.
	The Darray of image primitives are from related solution elements.
	The ss_factor indicates the x, y and z subsampling factors to be applied to the output ROI - the image data and the primitives supplied as arguments are NOT subsampled.
	*/
	virtual const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi) = 0;

	/**
	Write search area attribute to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Related SolElement: name <newline>  Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:

	/// OR flag is set to 1 if OR should be used to combine search area (0 by default)
	int _or_flag;
};


/**
BetweenX search area is set of voxels formed by including x-intervals that lie between two intervals of the related image primitive.
Take AND with current search area.
*/
class BetweenX : public SearchArea {
public:
	/// Constructor
	BetweenX(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~BetweenX();

	/// Name of the attribute
	const char* const name() const { return "BetweenX"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};



/**
BoxRelCentroid search area is set of voxels in a 3-D box whose center is located relative to the centroid of the related image primitive.
Can take OR or AND with current search area depending on whether or flag is set.
The centroid coordinate offsets are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series). Variations in z-spacing are accounted for. The centroid and box may go outside the image dimensions. If so the spacing between the most first (or last) pair of slices is used.
Ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelCentroid : public SearchArea {
public:
	/// Constructor
	BoxRelCentroid(const std::string& rel_solel_name, const int rel_solel_ind,
	const float x_offset, const float y_offset, const float z_offset,
	const float x_length, const float y_length, const float z_length,
	const int e_flag);

	/// Destructor
	~BoxRelCentroid();

	/// Name of the attribute
	const char* const name() const { return "BoxRelCentroid"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _z_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;

	/// Length of box in z-direction (mm)
	float _z_length;
};


/**
BoxRelPlanarCentroid search area is set of 2-D boxes whose centers are located relative to the planar-centroids of the related image primitive (viz the centroids computed from the voxels in each plane of the related primitive).
Can take OR or AND with current search area depending on whether or flag is set.
The centroid coordinate offsets are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series).
The centroid and box may go outside the image dimensions, but ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelPlanarCentroid : public SearchArea {
public:
	/// Constructor
	BoxRelPlanarCentroid(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float x_offset, const float y_offset,
	const float x_length, const float y_length,
	const int e_flag);

	/// Destructor
	~BoxRelPlanarCentroid();

	/// Name of the attribute
	const char* const name() const { return "BoxRelPlanarCentroid"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;
};


/**
BoxRelPlanarYmin search area is set of 2-D boxes whose centers are located relative to the planar-y-minimum-points of the related image primitive. The y-minimum-point of a region is defined as the mid-point of the first raster interval.
Thus the boxes are only computed in slices where the related image primitive exists.
Can take OR or AND with current search area depending on whether or flag is set.
The coordinate offsets for the box centers are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series).
The centroid and box may go outside the image dimensions, but ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelPlanarYmin : public SearchArea {
public:
	/// Constructor
	BoxRelPlanarYmin(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float x_offset, const float y_offset,
	const float x_length, const float y_length,
	const int e_flag);

	/// Destructor
	~BoxRelPlanarYmin();

	/// Name of the attribute
	const char* const name() const { return "BoxRelPlanarYmin"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;
};



/**
BoxRelPlanarYmax search area is set of 2-D boxes whose centers are located relative to the planar-y-maximum-points of the related image primitive. The y-maximum-point of a region is defined as the mid-point of the last raster interval.
Thus the boxes are only computed in slices where the related image primitive exists.
Take AND with current search area.
The coordinate offsets for the box centers are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series).
The centroid and box may go outside the image dimensions, but ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelPlanarYmax : public SearchArea {
public:
	/// Constructor
	BoxRelPlanarYmax(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float x_offset, const float y_offset,
	const float x_length, const float y_length,
	const int e_flag);

	/// Destructor
	~BoxRelPlanarYmax();

	/// Name of the attribute
	const char* const name() const { return "BoxRelPlanarYmax"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;
};



/**
BoxRelZmin search area is a 3-D box whose center is located relative to the z-minimum-point of the related image primitive. The z-minimum-point of a region is defined as the centroid of the set of points with minimum z-coordinate.
Can take OR or AND with current search area depending on whether or flag is set.
The coordinate offsets for the box center are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series). Variable z-spacings are taken into acccount.
The centroid and box may go outside the image dimensions, but ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelZmin : public SearchArea {
public:
	/// Constructor
	BoxRelZmin(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float x_offset, const float y_offset, const float z_offset,
	const float x_length, const float y_length, const float z_length,
	const int e_flag);

	/// Destructor
	~BoxRelZmin();

	/// Name of the attribute
	const char* const name() const { return "BoxRelZmin"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _z_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;

	/// Length of box in z-direction (mm)
	float _z_length;
};



/**
BoxRelZmax search area is a 3-D box whose center is located relative to the z-maximum-point of the related image primitive. The z-maximum-point of a region is defined as the centroid of the set of points with maximum z-coordinate.
Can take OR or AND with current search area depending on whether or flag is set.
The coordinate offsets for the box center are given in mm (can be +ve or -ve).
The lengths are also in mm. These lengths must be -1 or >=0. If a length is given as -1, then the box occupies the full range of the image in that dimension.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series). Variable z-spacings are taken into acccount.
The centroid and box may go outside the image dimensions, but ultimately the search area is constrained to be inside the image dimensions.
*/
class BoxRelZmax : public SearchArea {
public:
	/// Constructor
	BoxRelZmax(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float x_offset, const float y_offset, const float z_offset,
	const float x_length, const float y_length, const float z_length,
	const int e_flag);

	/// Destructor
	~BoxRelZmax();

	/// Name of the attribute
	const char* const name() const { return "BoxRelZmax"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _x_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _y_offset;

	/// Centroid coordinate offset in mm (can be +ve or -ve)
	float _z_offset;

	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;

	/// Length of box in z-direction (mm)
	float _z_length;
};



/**
BoxSizeTlProp creates a rectangular search area with dimensions in mm of x_length, y_length, z_length in mm. For each the parameter must be > 0.0, if it is <= 0 then the region extends over the entire image along that dimension. The tlx_prop tly_prop tlz_prop parameters indicate the proportion of the difference between the image size and the crop size along a dimension that should be assigned when positioning the top left (tl) corner of the crop box. The prop parameters should be [0.0, 1.0]. For example 0,0,0 positions the crop at the top left, 0.5,0.5,0.5 positions it in the middle of the image. If a prop parameter is outside of the allowed range then the entire image along the corresponding axis is included.
When converting lengths and offsets from mm to pixels, the row and column pixel spacings from the first image are used (i.e. assumed constant throughout series). Variations in z-spacing are accounted for. The centroid and box may go outside the image dimensions. If so the spacing between the most first (or last) pair of slices is used.
Ultimately the search area is constrained to be inside the image dimensions.
Can take OR or AND with current search area depending on whether or flag is set.
*/
class BoxSizeTlProp : public SearchArea {
public:
	/// Constructor
	BoxSizeTlProp(const float x_length, const float y_length, const float z_length, const float tlx_prop, const float tly_prop, const float tlz_prop);

	/// Destructor
	~BoxSizeTlProp();

	/// Name of the attribute
	const char* const name() const { return "BoxSizeTlProp"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:
	/// Length of box in x-direction (mm)
	float _x_length;

	/// Length of box in y-direction (mm)
	float _y_length;

	/// Length of box in z-direction (mm)
	float _z_length;

	/// Proportion of the difference between the image size and the crop size along a dimension that should be assigned to the top left x coordinate.
	float _tlx_prop;

	/// Proportion of the difference between the image size and the crop size along a dimension that should be assigned to the top left y coordinate.
	float _tly_prop;

	/// Proportion of the difference between the image size and the crop size along a dimension that should be assigned to the top left z coordinate.
	float _tlz_prop;
};



/**
Clears the search area (makes it empty).
*/
class Clear : public SearchArea {
public:
	/// Constructor
	Clear();

	/// Destructor
	~Clear();

	/// Name of the attribute
	const char* const name() const { return "Clear"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};


/**
Search area is convex hull formed from boundaries of the related primitive on each slice.
Search area is set of voxels that are enclosed (2D) by the convex hull.
The algorithm requires a simple closed polygon. Therefore to ensure that the boundary doesn't double back on itself, a morphological DILATION is performed (with a small structuring element) prior to computation of the convex hull. Then an EROSION is performed on the resulting convex hull.
The algorithm starts with the largest boundaries, and ignores boundaries that are already inside a convex hull, i.e. ignores boundaries associated with holes inside a region.
Take AND with current search area.
*/
class ConvexHull : public SearchArea {
public:
	/// Constructor
	ConvexHull(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~ConvexHull();

	/// Name of the attribute
	const char* const name() const { return "ConvexHull"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};


/**
Search area is obtained by thresholding a 2D Euclidean distance map of a related image primitive.
The distance map is computed in mm and the threshold is given in the same units.
Values in the distance map must be greater than or equal to the threshold.
Take AND or OR with current search area.
*/
class DistanceMap2D : public SearchArea {
public:
	/// Constructor
	DistanceMap2D(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_thresh, const int e_flag);

	/// Destructor
	~DistanceMap2D();

	/// Name of the attribute
	const char* const name() const { return "DistanceMap2D"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Distance threshold in mm
	float _dist_thresh;
};


/*
Search area is obtained by thresholding a 2D Euclidean distance map of each slice of a related image primitive.
The distance map is computed in mm and the threshold is given as a percentage of the maximum of the 3D-local-maximum.
The 3D-local-maximum for a given voxel is the maximum value of the distance map among all connected non-zero voxels in the distance map.
For this given voxel to be included in the search area its value must be greater than or equal to a percentage of the local maximum as specified in the constructor.
Take AND or OR with current search area.
*/
/*
class DistanceMap25DPercMax : public SearchArea {
public:
	/// Constructor
	DistanceMap25DPercMax(const std::string& rel_solel_name, const int rel_solel_ind, const float percentage, const int e_flag);

	/// Destructor
	~DistanceMap25DPercMax();

	/// Name of the attribute
	const char* const name() const { return "DistanceMap25DPercMax"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Percentage of local maximum to be used in thresholding
	float _perc;
};
*/


/**
ExpandContractPlanar search area is computed by expanding/contracting each slice of the related image primitive toward the centroid in each plane.
Can take OR or AND with current search area depending on whether OR flag is set.
Only one boundary contour is considered per slice - that with the greatest number of points. Centroid is computed as average of those boundary points.
The distance of the expansion/contraction is given in mm (+ve for expansion or -ve for contraction). Boundary points are moved toward or away from the centroid of the region. To ensure that the new boundary is a simple closed contour, every N'th point on the original curve is considered, where N is the specified translation distance divided by the row pixel spacing. This is faster for expanding or contracting by relatively large distances, otherwise a morphological erosion or dilation is probably better.
*/
class ExpandContractPlanar : public SearchArea {
public:
	/// Constructor
	ExpandContractPlanar(const std::string& rel_solel_name,
	const int rel_solel_ind,
	const float distance,
	/*const float max_angle_diff,*/
	const int e_flag);

	/// Destructor
	~ExpandContractPlanar();

	/// Name of the attribute
	const char* const name() const { return "ExpandContractPlanar"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:

	/// Distance in mm for expansion/contraction (can be +ve or -ve)
	float _distance;

	// Maximum allowable angle difference if point is to be included in new contour (in radians)
	//float _max_angle_diff;
};


/**
Found checks if a match was made to the related solution element.
If the yes_flag==1 and no match was made then the current search area is made empty (cleared).
If the yes_flag==0 and a match was made then the current search area is made empty (cleared).
Defined for ImagePrimitives.
*/
class Found : public SearchArea {
public:
	/// Constructor
	Found(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag, const int yes_flag);

	/// Destructor
	~Found();

	/// Name of the attribute
	const char* const name() const { return "Found"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);

private:
	/// Flag indicating whether the related solution element should be matched or not
	int _yes_flag;
};


/**
Inside2D search area is set of voxels that are fully enclosed (2D) by related image primitive.
Take AND with current search area.
*/
class Inside2D : public SearchArea {
public:
	/// Constructor
	Inside2D(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~Inside2D();

	/// Name of the attribute
	const char* const name() const { return "Inside2D"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};


/**
NotPartOf subtracts the related primitive from the current search area.
Defined for ImageRegions.
*/
class NotPartOf : public SearchArea {
public:
	/// Constructor
	NotPartOf(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~NotPartOf();

	/// Name of the attribute
	const char* const name() const { return "NotPartOf"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};


/**
PartOf search area is set of voxels included in the related image primitive.
Can take OR or AND with current search area depending on whether or flag is set.
*/
class PartOf : public SearchArea {
public:
	/// Constructor
	PartOf(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~PartOf();

	/// Name of the attribute
	const char* const name() const { return "PartOf"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};


/*
PartOfOR search area is set of voxels included in the related image primitive.
Take OR with current search area.
class PartOfOR : public SearchArea {
public:
	/// Constructor
	PartOfOR(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag);

	/// Destructor
	~PartOfOR();

	/// Name of the attribute
	const char* const name() const { return "PartOfOR"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};
*/


/**
Abstract base class for morphological operations to generate a search area.
If a related solution element is specified, then the morphological operation is applied to the related primitive and then ANDed or ORed with the current search area, depending on whether or flag is set.
Otherwise the morphological operation is applied directly to the current search area.
The struct_el_descr is used for writing the attribute to a stream.
*/
class Morph : public SearchArea {
public:
	/// Constructor
	Morph(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag);

	/// Constructor
	Morph(const std::string& struct_el_descr, const int e_flag);

	/// Destructor
	~Morph();

	/// Sets ROI argument to be the structuring element (assumed empty) and returns 1 if ROI description is valid, 0 otherwise
	const int struct_el(ROI&, const MedicalImageSequence&) const;

	/**
	Write search area attribute to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Related SolElement: name <newline>  Structuring Element: description <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;

private:
	/// Structuring element
	ROIdescription _struct_el;
};


/**
Morphological erosion to generate a search area.
If a related solution element is specified, then the morphological operation is applied to the related primitive and then ANDed or ORed with the current search area, depending on whether or flag is set.
Otherwise the morphological operation is applied directly to the current search area.
*/
class MorphErode : public Morph {
public:
	/// Constructor
	MorphErode(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag);

	/// Constructor
	MorphErode(const std::string& struct_el_descr, const int e_flag);

	/// Destructor
	~MorphErode();

	/// Name of the attribute
	const char* const name() const { return "MorphErode"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};

/**
Morphological dilation to generate a search area.
If a related solution element is specified, then the morphological operation is applied to the related primitive and then ANDed or ORed with the current search area, depending on whether or flag is set.
Otherwise the morphological operation is applied directly to the current search area.
*/
class MorphDilate : public Morph {
public:
	/// Constructor
	MorphDilate(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag);

	/// Constructor
	MorphDilate(const std::string& struct_el_descr, const int e_flag);

	/// Destructor
	~MorphDilate();

	/// Name of the attribute
	const char* const name() const { return "MorphDilate"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};

/**
Morphological opening to generate a search area.
If a related solution element is specified, then the morphological operation is applied to the related primitive and then ANDed or ORed with the current search area, depending on whether or flag is set.
Otherwise the morphological operation is applied directly to the current search area.
*/
class MorphOpen : public Morph {
public:
	/// Constructor
	MorphOpen(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag);

	/// Destructor
	~MorphOpen();

	/// Name of the attribute
	const char* const name() const { return "MorphOpen"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};

/**
Morphological closing to generate a search area.
If a related solution element is specified, then the morphological operation is applied to the related primitive and then ANDed or ORed with the current search area, depending on whether or flag is set.
Otherwise the morphological operation is applied directly to the current search area.
*/
class MorphClose : public Morph {
public:
	/// Constructor
	MorphClose(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag);
	//MorphClose(const std::string& rel_solel_name, const int rel_solel_ind, const ROI& roi, const int e_flag);

	/// Destructor
	~MorphClose();

	/// Name of the attribute
	const char* const name() const { return "MorphClose"; };

	/// Vector must contain one primitive from the related solution element
	const int search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi);
};

#endif // !__SearchArea_h_

