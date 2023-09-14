#ifndef __ROI_h_
#define __ROI_h_

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     UCLA Department of Radiological Sciences (c) 1997
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

#include <assert.h>
#include <math.h>
#include <iostream>
#include <ostream>

#include "Darray.h"
#include "Plane.h"
#include "Point.h"
#include "Contour.h"
#include "ROIworkspace.h"

using std::cout;
using std::endl;
using std::ostream;

const double PI_DBL = fabs(atan2(0.,-1));

/// A 3-D (or 2-D) region of interest
class ROI {
  	/**
   	Overloaded stream insertion operator to write contents to a file.
   	Must ensure that file is opened first.
   	Usage: file << anROI.
   	*/
  	friend ostream& operator<<(ostream& file, const ROI& r);
  	/**
   	Overloaded stream insertion operator to write contents to a file.
   	Must ensure that file is opened first.
   	Usage: file << anROI.
  	friend ofstream& operator<<(ofstream& file, const ROI& r);
	**/
  	/**
  	Overloaded stream extraction operator to get data from file.
   	Must ensure that file is opened, and in proper format.
   	Usage: file >> anROI.
   	*/
  	friend istream& operator>>(istream& file, ROI& is);

public:
	/// Default constructor
	ROI();

	/// Copy constructor
	ROI(const ROI&);

	/// Single-plane copy constructor
	ROI(const ROI&, const int z);
//ROI(const ROI&, const int z, const int dummy);

	/// Destructor
	~ROI() {};

	/** @name Adding to an ROI */
	//@{

	/// Adds a single Point to the ROI
	void add_point(const Point&);

	/**
	Adds a 3-D (or 2-D) box to the ROI.
	Program exits with error message if tl and br are invalid relative to each other.
	@param	tl	top left vertex of the box
	@param	br	bottom right vertex of the box
	*/
	void add_box(const Point& tl, const Point& br);

	/**
	Adds points from r that are contiguous (in 2D) with start.
	The argument sub should be set to 1 if the added points are to be subtracted from the argument r.
	*/
	void add_contig(ROI& r, const Point &start, const int sub=0);

	/**
	Adds points from r that are contiguous (in 3D) with start
	The argument sub should be set to 1 if the added points are to be subtracted from the argument r.
	*/
	void add_contig_3d(ROI& r, const Point &start, const int sub=0);

	/// The arguments are the center Point and x and y lengths.
	void add_planar_rect(const Point&, const int , const int);

	/**
	Adds a planar (2-D) polygon to the ROI.
	Vertices of the polygon are specified and may, or may not, be continuous.
	It is assumed that vertices have same z-coordinate (default 0).
	The Points should be ordered such that the inside of the region is on the left of the boundary.
	@param	poly	array of vertices (Points)
	@param	n	number of Points in the array
	*/
	void add_planar_polygon(const Point* const poly, const int n);

	/**
	Adds a planar (2-D) polygon to the ROI.
	Vertices of the polygon are specified and may, or may not, be continuous.
	It is assumed that vertices have same z-coordinate (default 0).
	The Points should be ordered such that the inside of the region is on the left of the boundary.
	If ordering is incorrect then no points are added to the ROI.
	@param	poly	Darray of vertices (Points)
	*/
	void add_planar_polygon(const Darray<Point>& poly);

	/**
	Adds a planar (2-D) polygon to the ROI.
	Vertices of the polygon are specified and may, or may not, be continuous.
	It is assumed that vertices have same z-coordinate (default 0).
	The Points should be ordered such that the inside of the region is on the left of the boundary.
	If ordering is incorrect then no points are added to the ROI.
	@param	poly	Darray of vertices (Points)
	*/
	void add_planar_polygon(const Contour&);

	/**
	Adds a continuous interval to the ROI. Does nothing if x1>x2.
	@param	x1	x-coordinate of first endpoint of the interval
	@param	x2	x-coordinate of second endpoint of the interval
	@param	y	y-coordinate of the interval
	@param	z	z-coordinate of the interval (default 0)
	If x1>x2 then the interval is invalid and no change is made to the ROI.
	*/
	void add_interval(const int x1, const int x2, const int y, const int z=0);
	//void add_interval_temp(const int x1, const int x2, const int y, const int z=0);

	/// Adds a circle with center and radius as given
	void add_circle(const int r, const int xc, const int yc, const int zc=0);

	/**
	A fast method for appending a point to an ROI in raster order.
	This method is fast if: p.x>X, p.y>=Y, p.z>=Z, for all points (X, Y, Z) currently within the ROI.
	*/
	void append_point(const Point& p);

	/**
	A fast method for appending intervals to an ROI in raster order.
	This method is fast if: x1>X, y>=Y, z>=Z, for all points (X, Y, Z) currently within the ROI.
	Otherwise, the regular add_interval method is called.
	@param	x1	x-coordinate of first endpoint of the interval
	@param	x2	x-coordinate of second endpoint of the interval
	@param	y	y-coordinate of the interval
	@param	z	z-coordinate of the interval (default 0)
	If x1>x2 then the interval is invalid and no change is made to the ROI.
	*/
	void append_interval(const int x1, const int x2, const int y, const int z=0);

	//@}


	/** @name Clearing ROI */
	//@{

	/// Removes all points from an ROI
	void clear();

	/// Clears a single plane of an ROI, returning 1 if plane had any points in it to begin with, 0 otherwise
	const int clear(const int z);

	/** Sets the arguments according to the coordinates of the first interval in the ROI, removes the interval and returns 1.
	If the ROI does not have any intervals 0 is returned and nothing is set.
	*/
	const int pop_first_interval(int& x1, int& x2, int& y, int& z);

	//@}


	/** @name Transformations */
	//@{

	/**
	Computes the convex hull in 2D of each slice of the ROI.
	The convex hull formed from boundaries of the ROI on each slice.
	The result to be added is the set of voxels that are enclosed (2D) by the convex hull on each slice.
	The algorithm requires a simple closed polygon. Therefore to ensure that the boundary doesn't double back on itself, a morphological DILATION is performed (with a small structuring element) prior to computation of the convex hull. Then an EROSION is performed on the resulting convex hull.
	The algorithm starts with the largest boundaries, and ignores boundaries that are already inside a convex hull, i.e. ignores boundaries associated with holes inside a region.
	*/
	void convex_hull();

	/**
	Crops an ROI in 3D.
	Top left and bottom right points of crop box are given.
	*/
	void crop(const Point &tl, const Point &br);

	/// Translates the ROI
	void translate(	const int x_shift,
			const int y_shift,
			const int z_shift=0);

	/**
	Maps the ROI z coordinates.
	*/	
	bool inverse_map_z(const int* map, const int map_n);

	/// Fills any holes (2D) in the ROI at a given slice
	void fill_holes_2D(const int z=0);

	/*
	Creates a copy of an ROI that is optimized for low memory consumption, but should not be modified.
	Modifications of the ROI copy may be greatly suboptimal, but querying the copy should be uneffected.
	*/
	/*
	ROI* low_memory_copy(const ROI&) const;
	*/

	/**
	Generates a (separate) subsampled version of the current ROI.
	For "conservative" subsampling an erosion is performed with a structuring element that is cube with lengths along each axis [-step/2 to step/2].
	Following this erosion every step'th voxel is retained and their coordinates rescaled to the lower resolution.
	For "expansive" subsampling, a dilation instead of an erosion is performed.
	[out]	new_roi
			Result: subsampled ROI.
			The argument ROI is cleared prior to subsampling.
			The original ROI (this) is left unchanged.
	[in]	xyz-step
			The subsampling factors in each of the axes.
	[in]	conservative
			Flag that indicates whether the subsampled ROI should be a conservative (or expansive) rounding of the original ROI.
	*/
	void subsample(ROI& new_roi, const int x_step, const int y_step, const int z_step, const bool conservative) const;

	/**
	// Generates a (separate) upsampled version of the current ROI.
	//
	// Each point in the original ROI is scaled to the higher resolution.
	// For "conservative" upsampling a dilation is performed with a structuring element that is cube with lengths along each axis [-step/2 to step/2].
	// For "expansive" upsampling, the dilation is performed with a structuring element that is cube with lengths along each axis [-step+1 to step-1].
	//
	// [out]	new_roi
	//			Result: subsampled ROI.
	//			The argument ROI is cleared prior to subsampling.
	//			The original ROI (this) is left unchanged.
	// [in]		xyz-step
	//			The upsampling factors in each of the axes.
	//			Must be >= 1.
	// [in]		conservative
	//			Flag that indicates whether the upsampled ROI should be a conservative (or expansive) rounding of the original ROI.
	*/
	void upsample(ROI& new_roi, const int x_step, const int y_step, const int z_step, const bool conservative) const;

	//@}


	/** @name Modification by another ROI */
	//@{

	/// Clears the ROI and makes a copy of the argument
	void copy(const ROI&);

	/**
	 Clears the specified slice of an ROI and makes a copy of the same slice of the argument.
	Other slices are uneffected.
	*/
	void copy(const ROI&b, const int z);

	/// Takes the logical "or" with the argument
	void OR(const ROI&);

	/// Takes the logical "and" with the argument
	void AND(const ROI& r);

	/// Removes points which are in common with r
	void subtract(const ROI& r);

	/// Morphological erosion
	void erode(const ROI& struct_element);

	/// Morphological dilation
	void dilate(const ROI& struct_element);

	//@}


	/** @name Queries */
	//@{

	/// Returns 1 if the Point is in the ROI, 0 otherwise
	const int in_roi(const Point&) const;

	/// Returns the number of pixels in the ROI
	const int num_pix() const;

	/// Returns the number of pixels in the image slice with the specified z-coordinate.
	const int num_pix(const int) const;

	/// Returns 1 if the number of pixels in the ROI is greater than or equal to the specified number (0 otherwise).
	const int num_pix_grequal(const int) const;

	/// Returns 1 if the given ROI overlaps this ROI, 0 otherwise
	const int overlaps(const ROI& r) const;

	/**
	Returns the (closed) boundaries of the ROI at a given slice.
	The boundary points are ordered such that points that are "inside" the
	ROI are kept on the "left".
	If the ROI has no points at the given slice (z), then n is set to zero and zero is returned.
	@param	zv	the z-coordinate of the slice to be considered
	@param	n	the number of closed boundary contours being returned
	*/
	Contour* boundaries(int& n, const int zv=0) const;

	/**
	Determines the bounding box for a given slice of the ROI.
	@param	z	z-coordinate of slice to be considered
	@param	ul	upper left vertex of the bounding box
	@param	br	bottom right vertex of the bounding box
	@return	1 if ROI defined at slice z (ul, br set), otherwise 0 (ul, br not set)
	*/
	const int bounding_box(Point& ul, Point& br, const int z=0) const;

	/**
	Determines the bounding cube for a 3-D (or 2-D) ROI.
	@param	tl	top left vertex of the bounding cube
	@param	br	bottom right vertex of the bounding cube
	@return	1 if ROI defined (tl, br set), otherwise 0 (tl, br not set)
	*/
	const int bounding_cube(Point& tl, Point& br) const;

	/**
	Determines the centroid for a given slice of the ROI.
	@param	z	z-coordinate of slice to be considered
	@param	c	centroid (x, y and z coords will be set)
	@return	1 if ROI defined at slice z (c set), otherwise 0 (c not set)
	*/
	const int centroid(Point& c, const int z) const;

	/// Returns 1 if ROI contains no points, 0 otherwise
	const int empty() const;

	/// Returns 1 if given slice of ROI contains no points, 0 otherwise
	const int empty(const int z) const;

	/**
	Sets argument to first point (raster order) in ROI.
	Returns 0 if ROI contains no points and does not set argument.
	Returns 1 otherwise.
	*/
	const int first_point(Point&) const;

	/**
	Sets argument to last point (raster order) in ROI.
	Returns 0 if ROI contains no points and does not set argument.
	Returns 1 otherwise.
	*/
	const int last_point(Point&) const;

	//@}

	/** @name Printing to the screen */
	//@{

	/// Prints the coordinates of all points in the ROI to the screen
	void print_all_points() const;

	//@}
	void getPixelCoordColumnMajor(int[], const int) const;

protected:
	/// Constructor
	ROI(const long plane_mod_fact, const long line_mod_fact, const long interval_mod_fact);

	/// Darray of Planes (a Plane only exists if it has at least 1 Line).
	Darray<Plane> _pl;

	/// Modulation factor for Line Darrays.
	long _ln_mod;

	/// Modulation factor for Interval Darrays.
	long _ivl_mod;

	/** @name ROIworkspace methods */
	//@{

	/**
	Sets the Workspace to the coordinates specified.
	@param	w	Workspace to be set
	@param	z	z-coordinate
	@param	y	y-coordinate, defaults to first point in the z-plane
	@param	x	x-coordinate, defaults to first point in plane, z, line, y
	@return OK if set successfully (x,y,z valid), NOT_FOUND otherwise
	*/
	const WkspaceStatus _set_workspace(ROIworkspace& w, const int z, const int y=-1, const int x=-1) const;

	/// Returns ROI_STAT_OK if Workspace refers to a valid location in the ROI, otherwise returns NOT_FOUND
	const WkspaceStatus _valid_wkspace(const ROIworkspace& w) const;

	/**
	UNSAFE: sets the coordinates of the Point (p) according to the location within the ROI referred to by the Workspace (w).
	Does not check the initial validity of w.
	*/
	void _current_point(ROIworkspace& w, Point& p) const;

	/**
	UNSAFE: advances the Workspace (w) to the next point in the ROI.
	Does not check the initial validity of w.
	The status of w is returned after advancement.
	*/
	const WkspaceStatus _next_point(ROIworkspace& w) const;

	/** @name Inter-plane */
	//@{

	/**
	DANGEROUS Plane dereferencing operator.
	Index into the array is supplied as an argument.
	There is no checking of the validity of the index (except by assert).
	Reference may be modified.
	*/
	Plane& operator[] (const long n) { assert((n>=0)&&(n<_pl.N())); return _pl(n); };

	/**
	Checks the upper limit on pi and returns either OK or END_ROI.
	The lower limit is only checked by assert.
	*/
	const WkspaceStatus _plane_status(const ROIworkspace& w) const;

	/**
	DANGEROUS function which returns a reference to a Plane.
	The referenced Plane may be modified.
	The only bounds checking on w is by assert.
	*/
	//Plane& _plane(const ROIworkspace& w) const { assert(_plane_status(w)==ROI_STAT_OK); return _pl(w.pi); };
	Plane& _plane(const ROIworkspace& w) { assert(_plane_status(w)==ROI_STAT_OK); return _pl(w.pi); };
	const Plane& _plane_const(const ROIworkspace& w) const { assert(_plane_status(w)==ROI_STAT_OK); return _pl[w.pi]; };

	/**
	Increments w to the next Plane (increments pi).
	If w._pi is valid, sets the other indices to the start of the Plane and returns NEW_PLANE.
	Otherwise return END_ROI.
	*/
	const WkspaceStatus _next_plane(ROIworkspace& w) const;

	/**
	Sets w to the Plane with the appropriate z-coord (sets pi).
	If pi is valid, sets the other indices to the start of the Plane and returns NEW_PLANE.
	Otherwise does not modify pi, and returns NOT_FOUND.
	*/
	const WkspaceStatus _set_plane(ROIworkspace& w, const int z) const;

	/**
	Increments w._pi to the Plane with the appropriate z-coord.
	If Plane is found, sets the other indices to the start of the Plane and returns NEW_PLANE.
	Otherwise, w._pi indicates where the plane should be inserted, NOT_FOUND is returned.
	*/
	const WkspaceStatus _find_fwd_plane(ROIworkspace& w, const int z) const;

	/**
	Advances w to the last point in the ROI.
	Returns NOT_FOUND if ROI contains no points, and OK if successful.
	*/
	const WkspaceStatus _end_of_roi(ROIworkspace& w) const;

	/**
	Sets w to the start of the first Plane.
	Validity of pi checked by assert.
	*/
	void _first_plane(ROIworkspace& w) const { w.pi=0; assert(_plane_status(w)==ROI_STAT_OK); _first_line(w);};

	//@}

	/** @name Inter-line */
	//@{

	/**
	Checks the upper limit on li and returns either OK or NEW_PLANE.
	The lower limit and _plane_status are only checked by assert.
	*/
	const WkspaceStatus _line_status(const ROIworkspace& w) const;

	/**
	DANGEROUS function which returns a reference to a Line.
	The referenced Line may be modified.
	The only bounds checking on w is by assert.
	*/
	//Line& _line(const ROIworkspace& w) const { assert(_line_status(w)==ROI_STAT_OK); return _pl(w.pi).ln(w.li); };
	Line& _line(const ROIworkspace& w) { assert(_line_status(w)==ROI_STAT_OK); return _pl(w.pi).ln(w.li); };
	const Line& _line_const(const ROIworkspace& w) const { assert(_line_status(w)==ROI_STAT_OK); return _pl[w.pi].ln[w.li]; };

	/**
	Increments w to the next Line (increments li).
	If w._li is valid, sets the other indices to the start of the Line and returns NEW_LINE.
	Otherwise calls _next_plane.
	*/
	const WkspaceStatus _next_line(ROIworkspace& w) const;

	/**
	Sets w to the Line with the appropriate y-coord (sets li).
	If w._li is valid, sets the other indices to the start of the Line and returns NEW_LINE.
	Otherwise return NOT_FOUND.
	*/
	const WkspaceStatus _set_line(ROIworkspace& w, const int y) const;

	/**
	Increments pi to the Line with the appropriate y-coord.
	If Line is found, sets the other indices to the start of the Line and returns NEW_LINE.
	Otherwise, li indicates where the Line should be inserted, NOT_FOUND is returned.
	*/
	const WkspaceStatus _find_fwd_line(ROIworkspace& w, const int y) const;

	/**
	Sets w to the start of the first Line (in the current Plane).
	Validity of w._li checked by assert.
	*/
	void _first_line(ROIworkspace& w) const { assert((w.pi>0) && (_plane_status(w)==ROI_STAT_OK)); w.li=0; _first_interval(w);};

	/**
	Advances w to the last point in the current Plane.
	Initial validity of w is checked only by assert.
	*/
	void _end_of_plane(ROIworkspace& w) const;

	//@}

	/** @name Inter-interval */
	//@{

	/**
	Checks the upper limit on ii and returns either OK or NEW_LINE.
	The lower limit is only checked by assert.
	*/
	const WkspaceStatus _interval_status(const ROIworkspace& w) const;

	/**
	UNSAFE: Sets the coordinates of the Points, p1 and p2, according to the interval within the ROI referred to by the Workspace (w).
	Does not check the initial validity of w.
	*/
	void _current_interval(ROIworkspace& w, Point& p1, Point& p2) const;

	/**
	DANGEROUS function which returns a reference to an Interval.
	The referenced Interval may be modified.
	The only bounds checking on w is by assert.
	*/
	//Interval& _interval(const ROIworkspace& w) const { assert(_interval_status(w)==ROI_STAT_OK); return _pl(w.pi).ln(w.li).ivl[w.ii]; };
	Interval& _interval(const ROIworkspace& w) { assert(_interval_status(w)==ROI_STAT_OK); return _pl(w.pi).ln(w.li).ivl[w.ii]; };
	const Interval& _interval_const(const ROIworkspace& w) const { assert(_interval_status(w)==ROI_STAT_OK); return _pl[w.pi].ln[w.li].ivl[w.ii]; };

	/**
	Increments w to the next Interval (increments ii).
	If ii is valid, sets the other indices to the start of the Interval and returns NEW_INTERVAL.
	Otherwise calls _next_line.
	*/
	const WkspaceStatus _next_interval(ROIworkspace& w) const;

	/**
	Sets w to the Interval that contains the x-coord (sets ii).
	If ii is valid, sets the other indices to the start of the Interval and returns NEW_INTERVAL. Otherwise return NOT_FOUND.
	Note that the Workspace is set to the start of the Interval, not to the point with coordinate x.
	*/
	const WkspaceStatus _set_interval(ROIworkspace& w, const int x) const;

	/**
	Increments ii to the Interval that overlaps the given Interval.
	The initial validity (_interval_status) of w is checked.
	If Interval is found, sets the other indices to the start of the Interval and returns NEW_INTERVAL.
	Otherwise, ii indicates where the Interval should be inserted, NOT_FOUND is returned.
	*/
	const WkspaceStatus _overlap_fwd_interval(ROIworkspace& w, const Interval ivl) const;

	/**
	Sets w to the start of the first Interval (in the current Line).
	Validity of ii checked by assert.
	*/
	void _first_interval(ROIworkspace& w) const { assert((w.li>0) && (_line_status(w)==ROI_STAT_OK)); w.ii=0; _first_x(w);};

	/**
	Advances w to the last point in the current Line.
	Initial validity of w is checked only by assert.
	*/
	void _end_of_line(ROIworkspace& w) const;

	//@}

	/** @name Intra-interval */
	//@{

	/**
	Checks the upper limit on w._xi and returns either OK or NEW_INTERVAL.
	The lower limit is only checked by assert.
	*/
	const WkspaceStatus _x_status(const ROIworkspace& w) const;

	/**
	Increments w to the next x-coordinate (increments xi).
	If w._xi is valid returns OK.
	Otherwise calls _next_interval.
	*/
	const WkspaceStatus _next_x(ROIworkspace& w) const;

	/**
	Sets w to the x-coordinate within the current Interval (sets xi).
	If xi is valid returns OK, otherwise NOT_FOUND.
	The initial validity of the other Workspace indicies is checked only by assert.
	*/
	const WkspaceStatus _set_x(ROIworkspace& w, const int x) const;

	/**
	Sets w to the first point of the current Interval.
	Validity of ii checked by assert.
	*/
	void _first_x(ROIworkspace& w) const { assert((w.ii>0) && (_interval_status(w)==ROI_STAT_OK)); w.xi=0; };

	/**
	Advances w to the last point in the current Interval.
	Initial validity of w is checked only by assert.
	*/
	void _end_of_interval(ROIworkspace& w) const;

	//@}

	//@}

private:

	/**
	Constructor.
	Accepts a 2-D continuous, closed contour (with direction such that inside the region is on the left of the boundary).
	If the contour (argument is not continuous it will be modified so that it is.
	Constructs a planar ROI.
	*/
	ROI(Darray<Point>&);

	/**
	A fast DANGEROUS method for appending intervals to an ROI in raster
	order.
	There is no checking of the validity of any of the arguments.
	w is assumed to be the location of the last interval in the ROI.
	If there are no elements in the ROI, w should have all of its indices
	set to -1.
	w is advanced to the location the the newly appended interval.
	@param	w	the location of the last interval in the ROI (the x-coordinate of the workspace is ignored)
	@param	x1	x-coordinate of first endpoint of the interval
	@param	x2	x-coordinate of second endpoint of the interval
	@param	y	y-coordinate of the interval
	@param	z	z-coordinate of the interval (default 0)
	*/
	void _append_interval(ROIworkspace& w, const int x1, const int x2, const int y, const int z=0);

	/**
	Algorithm based on Sonka's suggestion for morphological processing (erosion and dilation). See p. 429 of Image Processing, Analysis and Machine Vision.

	se: structuring element.
	trans_sign: the translation sign (-1 for erode or 1 for dilate).
	*/
	void _morph_shell(const ROI& se, int trans_sign);

	void _morph_shell_temp(const ROI& se, int trans_sign);

	/// Adds intervals from r that overlap the specified interval, and delete the intervals from r
	void _add_overlap_interval(const int x1, const int x2, const int y, const int z, ROI& r);

};

void ROI_unit_test();

#endif // !__ROI_h_

