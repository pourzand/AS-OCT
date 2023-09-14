#ifndef __ROItraverser_h_
#define __ROItraverser_h_

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

#include "ROI.h"
#include "TravStatus.h"

/**
Allows traversal of an ROI.
The ROI to be travsered is a fixed copy of the ROI passed to the constructor.
*/
class ROItraverser : private ROI {
public:
	/// Default constructor
	ROItraverser() {};

	/// Constructor
	ROItraverser(const ROI&);

	/// Constructor (traverser considers only a single plane)
	ROItraverser(const ROI&, const int z);
	
	/// Reinitializes the traverser using a new ROI
	void reinitialize(const ROI&);

	/// Returns OK if current location in the ROI is valid, otherwise returns NOT_FOUND
	const TravStatus valid() const;

	/**
	Sets the coordinates of the Point (p) according to the current location within the ROI.
	The validity of the current location is checked. If valid, OK is returned, otherwise NOT_FOUND is returned and p is not set.
	*/
	const TravStatus current_point(Point& p) const;

	/**
	Sets the coordinates of the Points (p1, p2) to be the endpoints of the interval in which the traverser is currently located.
	The validity of the current location is checked. If valid, OK is returned, otherwise NOT_FOUND is returned and the points are not set.
	*/
	const TravStatus current_interval(Point& p1, Point& p2) const;

	/**
	Advances to the next point in the ROI.
	The initial validity of the current location is checked. If not valid, NOT_FOUND is returned.
	Otherwise, the status of the traverser is returned after advancement.
	*/
	const TravStatus next_point();

	/**
	Advances to the last interval in the current plane.
	The initial validity of the current location is checked. If not valid, NOT_FOUND is returned.
	Otherwise, NEW_INTERVAL is returned.
	*/
	const TravStatus last_point_in_plane();

	/**
	Advances to the next interval in the ROI.
	The initial validity of the current location is checked. If not valid, NOT_FOUND is returned.
	Otherwise, the status of the traverser is returned after advancement.
	*/
	const TravStatus next_interval();

	/**
	Advances to the next line in the ROI.
	The initial validity of the current location is checked. If not valid, NOT_FOUND is returned.
	Otherwise, the status of the traverser is returned after advancement.
	*/
	const TravStatus next_line();

	/**
	Advances to the next plane in the ROI.
	The initial validity of the current location is checked. If not valid, NOT_FOUND is returned.
	Otherwise, the status of the traverser is returned after advancement.
	*/
	const TravStatus next_plane();

	/**
	Sets the current location of the traverser to the first point in the plane with specified z-coordinate.
	If there are no points in the specified plane, NOT_FOUND is returned.
	Otherwise NEW_PLANE is returned.
	*/
	const TravStatus set_plane(const int z);

	/**
	Returns the current status of the traverser.
	If the current location is invalid or has moved beyond the end of the ROI, NOT_FOUND is returned.
	If the traverser is at the start of a plane NEW_PLANE is returned.
	Otherwise, check for NEW_LINE or NEW_INTERVAL.
	*/
	const TravStatus status() const;
	
	/// Resets the traverser returns its validity (i.e. make sure there is a first point in the ROI)
	const TravStatus reset();


	
	/** @name ROI Queries */
	//@{

	/// Returns 1 if the Point is in the ROI, 0 otherwise
	const int in_roi(const Point&) const;

	/// Returns the number of pixels in the ROI
	const int num_pix() const;

	/// Returns the number of pixels in the image slice with the specified z-coordinate.
	const int num_pix(const int) const;

	/**
	Returns the (closed) boundaries of the ROI at a given slice.
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

	//@}
	

private:
	/// Workspace
	ROIworkspace _wkspace;
};

#endif // !__ROItraverser_h_

