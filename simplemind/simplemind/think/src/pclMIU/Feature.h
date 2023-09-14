#ifndef __Feature_h_
#define __Feature_h_

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

#include <math.h>

#include "Attribute.h"
#include "Fuzzy.h"
#include "MedicalImageSequence.h"
#include "ImageRegion.h"

/// An abstract base class for storing features on the blackboard.
class Feature : public Attribute {
public:

	/**
	Constructor.
	The b_flag parameter (0 or 1) indicates whether a _B (bi-directional relationship) was included in the model descriptor.
	The e_flag parameter (0 or 1) indicates whether an _E (essential relationship) was included in the model descriptor.
	*/
	Feature(const Fuzzy& fuzzy, const int b_flag, const int e_flag);

	/// Copy constructor
	Feature(const Feature&);

	/// Destructor
	virtual ~Feature();

	/// Type of attribute
	inline const char* const type() const { return "Feature"; };

	/// Fuzzy membership function
	const Fuzzy& fuzzy() const;

	/**
	If the derived type of the image primitive is appropriate the numeric value of feature in the referenced argument val is set and 1 is returned, otherwise 0 is returned and val is not set.
	The first element of the Darray of image primitives is the primitive for which the value is being computed. Subsequent primitives are from related solution elements.
	*/
	virtual const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val) = 0;

	/**
	Write feature to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Related SolElement: name <newline> Related SolElement: name <newline> Fuzzy: fuzzy_membership <newline> Attribute: End <newline>
	*/
	virtual void write(ostream& s) const;

private:
	/// Fuzzy membership function
	Fuzzy _fuzzy;
};


/**
Maximum planar area feature - maximum area in any single xy-plane of an image primitive.
Expects one primitives as argument.
The area is returned in mm3, based on the pixel spacings of the medical image sequence.
This feature can be computed for ImageRegions.
*/
class Area_maxXY : public Feature {
public:
	/// Constructor
	Area_maxXY(const Fuzzy&);

	/// Copy constructor
	Area_maxXY(const Area_maxXY&);

	/// Destructor
	~Area_maxXY();

	/// Name of the attribute
	const char* const name() const { return "Area_maxXY"; };

	/// Vector must contain one primitive, for which the area is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Area per plane feature - total area of an image primitive in the xy-plane(s) is divided by the total number of planes between the first and last plane in which the primitive appears.
Expects one primitives as argument.
The area is returned in mm3, based on the pixel spacings of the medical image sequence.
This feature can be computed for ImageRegions.
*/
class Area_perXY : public Feature {
public:
	/// Constructor
	Area_perXY(const Fuzzy&);

	/// Copy constructor
	Area_perXY(const Area_perXY&);

	/// Destructor
	~Area_perXY();

	/// Name of the attribute
	const char* const name() const { return "Area_perXY"; };

	/// Vector must contain one primitive, for which the area is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Area feature - total area of an image primitive in the xy-plane(s).
Expects one primitives as argument.
The area is returned in mm3, based on the pixel spacings of the medical image sequence.
This feature can be computed for ImageRegions.
*/
class Area_XY : public Feature {
public:
	/// Constructor
	Area_XY(const Fuzzy&);

	/// Copy constructor
	Area_XY(const Area_XY&);

	/// Destructor
	~Area_XY();

	/// Name of the attribute
	const char* const name() const { return "Area_XY"; };

	/// Vector must contain one primitive, for which the area is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Average Diameter feature for ROIs.
Expects one primitive as arguments.
Longest diameter and its perpendicular are computed in a 2D slice.
The length value is the average of the orthogonal diameters in mm.
This feature can be computed for ImageRegions.
*/
class AvgDiameter : public Feature {
public:
	/// Constructor
	AvgDiameter(const Fuzzy& f);

	/// Copy constructor
	AvgDiameter(const AvgDiameter&);

	/// Destructor
	~AvgDiameter();

	/// Name of the attribute
	const char* const name() const { return "AvgDiameter"; };

	/// Vector must contain one primitive, for which the max diameter is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Average Diameter feature for ROIs.
Expects one primitive as arguments.
Longest diameter and its perpendicular are computed in a 2D slice.
The length value is the average of the orthogonal diameters in mm divided by the slice thicknesses, i.e., the average diameter in slice thicknesses.
This feature can be computed for ImageRegions.
*/
class AvgDiameterInSliceThicknesses : public Feature {
public:
	/// Constructor
	AvgDiameterInSliceThicknesses(const Fuzzy& f);

	/// Copy constructor
	AvgDiameterInSliceThicknesses(const AvgDiameterInSliceThicknesses&);

	/// Destructor
	~AvgDiameterInSliceThicknesses();

	/// Name of the attribute
	const char* const name() const { return "AvgDiameterInSliceThicknesses"; };

	/// Vector must contain one primitive, for which the max diameter is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};



/**
Centroid distance feature.
Expects two primitives as arguments.
Distance between centroid of primitive in question and centroid of related image primitive.
The difference is returned in mm, assuming equal pixel spacings within all slices, but taking into account variations in slice spacings.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidDistance : public Feature {
public:
	/// Constructor
	CentroidDistance(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidDistance(const CentroidDistance&);

	/// Destructor
	~CentroidDistance();

	/// Name of the attribute
	const char* const name() const { return "CentroidDistance"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Centroid offset feature for Z-coordinate.
Expects two primitives as arguments.
Difference between z-coordinate of centroid of primitive in question and the maximum z-coordinate of the related image primitive.
The difference is returned in mm, taking into account variations in slice spacings.
The convention is that superior is a positive difference and inferior is a negative difference. So if the z-coordinate of centroid of the entity in question is greater than the z-coordinate of the tip of the related entity then the distance value will be negative.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidOffsetMaxZ : public Feature {
public:
	/// Constructor
	CentroidOffsetMaxZ(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidOffsetMaxZ(const CentroidOffsetMaxZ&);

	/// Destructor
	~CentroidOffsetMaxZ();

	/// Name of the attribute
	const char* const name() const { return "CentroidOffsetMaxZ"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Centroid offset feature for X-coordinate.
Expects two primitives as arguments.
Difference between x-coordinate of centroid of primitive in question and x-coordinate for related image primitive.
The difference is returned in mm, based on the pixel spacings of the slice of the centroid of the primitive in question.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidOffsetX : public Feature {
public:
	/// Constructor
	CentroidOffsetX(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidOffsetX(const CentroidOffsetX&);

	/// Destructor
	~CentroidOffsetX();

	/// Name of the attribute
	const char* const name() const { return "CentroidOffsetX"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Centroid offset feature for Y-coordinate.
Expects two primitives as arguments.
Difference between y-coordinate of centroid of primitive in question and x-coordinate for related image primitive.
The difference is returned in mm, based on the pixel spacings of the slice of the centroid of the primitive in question.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidOffsetY : public Feature {
public:
	/// Constructor
	CentroidOffsetY(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidOffsetY(const CentroidOffsetY&);

	/// Destructor
	~CentroidOffsetY();

	/// Name of the attribute
	const char* const name() const { return "CentroidOffsetY"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Centroid offset feature for Z-coordinate.
Expects two primitives as arguments.
Difference between z-coordinate of centroid of primitive in question and z-coordinate for related image primitive.
The difference is returned in mm, taking into account variations in slice spacings.
The convention is that superior is a positive difference and inferior is a negative difference. So if the z-coordinate of centroid of the entity in question is greater than the z-coordinate of the related entity then the distance value will be negative.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidOffsetZ : public Feature {
public:
	/// Constructor
	CentroidOffsetZ(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidOffsetZ(const CentroidOffsetZ&);

	/// Destructor
	~CentroidOffsetZ();

	/// Name of the attribute
	const char* const name() const { return "CentroidOffsetZ"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
CentroidInside feature.
Expects two primitives as arguments.
Returns 1 if the centroid of the primitive in question is inside the ROI of the related image primitive.
Returns 0 otherwise.
This feature can be computed for ImageRegions, which it treats as 3-D when calculating the centroid.
*/
class CentroidInside : public Feature {
public:
	/// Constructor
	CentroidInside(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	CentroidInside(const CentroidInside&);

	/// Destructor
	~CentroidInside();

	/// Name of the attribute
	const char* const name() const { return "CentroidInside"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Circularity feature - % of area of bounding circle occupied by region.
Circularity is computed on the plane with maximum diameter.
Expects one primitive (ImageRegion) as argument.
Radius of bounding circle is the maximum distance (in mm) from planar centroid to any point in the region.
Circularity = 100*(area of region)/(area of bounding circle).
This feature can be computed for ImageRegions.
*/

class CircularityAtMaxDia : public Feature {
public:
	/// Constructor
	CircularityAtMaxDia(const Fuzzy&);

	/// Copy constructor
	CircularityAtMaxDia(const CircularityAtMaxDia&);

	/// Destructor
	~CircularityAtMaxDia();

	/// Name of the attribute
	const char* const name() const { return "CircularityAtMaxDia"; };

	/// Vector must contain one primitive, for which the circularity is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Circularity feature - % of area of bounding circle occupied by region.
Circularity is computed on a plane-by-plane basis, with the final result being the minimum circularity from any plane.
Expects one primitive (ImageRegion) as argument.
Radius of bounding circle is the maximum distance (in mm) from planar centroid to any point in the region.
Circularity = 100*(area of region)/(area of bounding circle).
This feature can be computed for ImageRegions.
*/

class CircularityMin : public Feature {
public:
	/// Constructor
	CircularityMin(const Fuzzy&);

	/// Copy constructor
	CircularityMin(const CircularityMin&);

	/// Destructor
	~CircularityMin();

	/// Name of the attribute
	const char* const name() const { return "CircularityMin"; };

	/// Vector must contain one primitive, for which the sphericity is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};



/**
Contacts feature.
Expects two primitives as arguments.
This feature can be computed for 2D or 3D ImageRegions.
Sets the value to be 1.0 if the regions are in contact, 0.0 otherwise.
If the threed_flag is 1, then in contact means that they overlap or contain one or more adjacent pixels (by 26-point connectivity).
Otherwise connectivity is checked in 2D using 8-point connectivity.
*/
class Contacts : public Feature {
public:
	/// Constructor
	Contacts(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag, const int threed_flag);

	/// Copy constructor
	Contacts(const Contacts&);

	/// Destructor
	~Contacts();

	/// Name of the attribute
	const char* const name() const { return "Contacts"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);

private:
	/// Flag indicating whether to apply 2D or 3D connectivity
	int _threed;
};


/**
Length feature for ROIs.
Expects one primitive as arguments.
The distance between the top and bottom slices is computed based on DICOM slice location (difference between first and last slice).
The returned value is the distance in mm divided by the slice thicknesses, i.e., the longitudinal dimension in slice thicknesses.
This feature can be computed for ImageRegions.
*/
class LengthInSliceThicknesses : public Feature {
public:
	/// Constructor
	LengthInSliceThicknesses(const Fuzzy& f);

	/// Copy constructor
	LengthInSliceThicknesses(const LengthInSliceThicknesses&);

	/// Destructor
	~LengthInSliceThicknesses();

	/// Name of the attribute
	const char* const name() const { return "LengthInSliceThicknesses"; };

	/// Vector must contain one primitive, for which the short axis is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Max Diameter feature for ROIs.
Expects one primitive as arguments.
Longest diameter and its perpendicular are computed in a 2D slice.
The length value is the longest diameter in mm divided by the slice thicknesses, i.e., the maximum diameter in slice thicknesses.
This feature can be computed for ImageRegions.
*/
class MaxDiameterInSliceThicknesses : public Feature {
public:
	/// Constructor
	MaxDiameterInSliceThicknesses(const Fuzzy& f);

	/// Copy constructor
	MaxDiameterInSliceThicknesses(const MaxDiameterInSliceThicknesses&);

	/// Destructor
	~MaxDiameterInSliceThicknesses();

	/// Name of the attribute
	const char* const name() const { return "MaxDiameterInSliceThicknesses"; };

	/// Vector must contain one primitive, for which the max diameter is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Max bounding box length feature for ROIs.
Expects one primitive as arguments.
Lengths along each axis of the 3D bounding box are computed.
The maximum bounding box length value is returned in mm.
This feature can be computed for ImageRegions.
*/
class MaxBBoxLength : public Feature {
public:
	/// Constructor
	MaxBBoxLength(const Fuzzy& f);

	/// Copy constructor
	MaxBBoxLength(const MaxBBoxLength&);

	/// Destructor
	~MaxBBoxLength();

	/// Name of the attribute
	const char* const name() const { return "MaxBBoxLength"; };

	/// Vector must contain one primitive, for which the max diameter is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Median gray level feature.
Expects one primitive as argument.
This feature can be computed for ImageRegions.
*/
class MedianHU : public Feature {
public:
	/// Constructor
	MedianHU(const Fuzzy& f);

	/// Copy constructor
	MedianHU(const MedianHU&);

	/// Destructor
	~MedianHU();

	/// Name of the attribute
	const char* const name() const { return "MedianHU"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Overlap feature.
Expects two primitives as arguments.
This feature can be computed for 3D ImageRegions.
Sets the value to be 1.0 if the regions overlap, 0.0 otherwise.
*/
class Overlaps : public Feature {
public:
	/// Constructor
	Overlaps(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	Overlaps(const Overlaps&);

	/// Destructor
	~Overlaps();

	/// Name of the attribute
	const char* const name() const { return "Overlaps"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Perpendicular Diameter feature for ROIs.
Expects one primitive as arguments.
Longest diameter and its perpendicular are computed in a 2D slice.
The length value is the perpendicular diameter in mm divided by the slice thicknesses, i.e., the short axis length in slice thicknesses.
This feature can be computed for ImageRegions.
*/
class PerpDiameterInSliceThicknesses : public Feature {
public:
	/// Constructor
	PerpDiameterInSliceThicknesses(const Fuzzy& f);

	/// Copy constructor
	PerpDiameterInSliceThicknesses(const PerpDiameterInSliceThicknesses&);

	/// Destructor
	~PerpDiameterInSliceThicknesses();

	/// Name of the attribute
	const char* const name() const { return "PerpDiameterInSliceThicknesses"; };

	/// Vector must contain one primitive, for which the perpendicular diameter is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Local centroid offset feature for X-coordinate.
Expects two primitives as arguments.
Computes centroid (x1, y1, z1) of primitive in question, and then the planar centroid of the related primitive.
The planar centroid considers only those points in the related primitive that lie in plane z1 or in the plane nearest to z1 that has points.
Difference between x-coordinate of centroid of primitive in question and x-coordinate for related image primitive.
The difference is returned in mm, based on the pixel spacings of the slice of the centroid of the primitive in question.
This feature can be computed for ImageRegions.
*/
class PlanarCentroidOffsetX : public Feature {
public:
	/// Constructor
	PlanarCentroidOffsetX(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	PlanarCentroidOffsetX(const PlanarCentroidOffsetX&);

	/// Destructor
	~PlanarCentroidOffsetX();

	/// Name of the attribute
	const char* const name() const { return "PlanarCentroidOffsetX"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Local centroid offset feature for Y-coordinate.
Expects two primitives as arguments.
Computes centroid (x1, y1, z1) of primitive in question, and then the planar centroid of the related primitive.
The planar centroid considers only those points in the related primitive that lie in plane z1 or in the plane nearest to z1 that has points.
Difference between y-coordinate of centroid of primitive in question and y-coordinate for related image primitive.
The difference is returned in mm, based on the pixel spacings of the slice of the centroid of the primitive in question.
This feature can be computed for ImageRegions.
*/
class PlanarCentroidOffsetY : public Feature {
public:
	/// Constructor
	PlanarCentroidOffsetY(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	PlanarCentroidOffsetY(const PlanarCentroidOffsetY&);

	/// Destructor
	~PlanarCentroidOffsetY();

	/// Name of the attribute
	const char* const name() const { return "PlanarCentroidOffsetY"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};

/**
RadLogicsNoduleClassification feature.
This feature value is computed by running the RadLogics nodule classifier.
nodeToBeClassified is an enumerated type.
*/
/*
class RadLogicsNoduleClassification : public Feature {
public:
	/// Constructor
	RadLogicsNoduleClassification(const Fuzzy& f, const int node_to_be_classified);

	/// Copy constructor
	RadLogicsNoduleClassification(const RadLogicsNoduleClassification&);

	/// Destructor
	~RadLogicsNoduleClassification();

	/// Name of the attribute
	const char* const name() const { return "RadLogicsNoduleClassification"; };

	/// Vector must contain one primitive, for which the sphericity is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);

private:
	/// Enumerated type indicating which node is being classified
	int _node_to_be_classified;
};
*/

/**
Short axis feature for ROIs.
Expects one primitive as arguments.
Longest diameter and its perpendicular are computed in a 2D slice. The perpendicular is returned as the short axis value.
This feature can be computed for ImageRegions, for 3-D ROIs all slices are checked to find the longest diameter and the perpendicular (short axis) is computed on the same slice.
*/
class ShortAxisInPixels : public Feature {
public:
	/// Constructor
	ShortAxisInPixels(const Fuzzy& f);

	/// Copy constructor
	ShortAxisInPixels(const ShortAxisInPixels&);

	/// Destructor
	~ShortAxisInPixels();

	/// Name of the attribute
	const char* const name() const { return "ShortAxisInPixels"; };

	/// Vector must contain one primitive, for which the short axis is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};



/**
Sphericity feature - % of volume of bounding sphere occupied by region.
Expects one primitive (ImageRegion) as argument.
Radius of bounding sphere is the maximum distance (in mm) from centroid to any point in the region.
Sphericity = 100*(volume of region)/(volume of bounding sphere).
This feature can be computed for ImageRegions.
*/
class Sphericity : public Feature {
public:
	/// Constructor
	Sphericity(const Fuzzy&);

	/// Copy constructor
	Sphericity(const Sphericity&);

	/// Destructor
	~Sphericity();

	/// Name of the attribute
	const char* const name() const { return "Sphericity"; };

	/// Vector must contain one primitive, for which the sphericity is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);

//const int temp_value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
SurfaceContactPercentage feature.
Expects two primitives as arguments.
This feature can be computed for 3D ImageRegions.
Determines the percentage of the surfaces of the two regions that are in contact.
Values range from 0.0 to 100.0.
Percentage contact is determined by dilating one region by a single voxel and taking the number of voxels in this dilated band as being the denominator and the numerator as the number of these voxels which overlap with the other region.
*/
class SurfaceContactPercentage : public Feature {
public:
	/// Constructor
	SurfaceContactPercentage(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag);

	/// Copy constructor
	SurfaceContactPercentage(const SurfaceContactPercentage&);

	/// Destructor
	~SurfaceContactPercentage();

	/// Name of the attribute
	const char* const name() const { return "SurfaceContactPercentage"; };

	/// Vector must contain two primitives
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


/**
Volume feature - total volume of an image primitive.
Expects one primitives as argument.
The volume is returned in mm3, based on the voxel spacings of the medical image sequence.
This feature can be computed for ImageRegions.
*/

class Volume : public Feature {
public:
	/// Constructor
	Volume(const Fuzzy&);

	/// Copy constructor
	Volume(const Volume&);

	/// Destructor
	~Volume();

	/// Name of the attribute
	const char* const name() const { return "Volume"; };

	/// Vector must contain one primitive, for which the volume is being calculated
	const int value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val);
};


#endif // !__Feature_h_

