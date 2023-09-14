#ifndef __Contour_h_
#define __Contour_h_

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

#include <ostream>
#include <iostream>

using std::cerr;
using std::ostream;

#include "Darray.h"
#include "Point.h"

/// Stores a set of points (defining a contour)
class Contour {
public:
	/// Returns number of points in the contour
	const int n() const;

	/**
	Returns the i'th Point in the contour.
	If i is out of range program exits with an error message.
	*/
	const Point& operator[](long i) const;

	/// Default constructor
	Contour();

	/// Constructor.
	Contour(const Darray<Point>&);

	/// Copy constructor
	Contour(const Contour&);

	/// Destructor
	~Contour();

	/// Appends a Point to the end of the Contour
	void append(const Point&);

	/// Appends another Contour to the end of the Contour
	void append(const Contour&);

	/// Inserts a Point at the start of the Contour
	void prepend(const Point&);

	/// Returns Darray of Points in the Contour
	const Darray<Point>& points() const;

	/// Clears all points from the contour
	void clear();

	/**
	Removes i'th point from the contour.
	Returns 1 on success, 0 on failure (if index out of range).
	*/
	const int remove(const int i);

	/**
	Replaces i'th point in the contour with a new point.
	Returns 1 on success, 0 on failure (if index out of range).
	*/
	const int replace(const int i, const Point p);

	/// Reverses the points in the contour
	void reverse();

	/// Makes contour continuous set of points in 2D
	void make_continuous();

private:

	/// Darray of points
	Darray<Point> _pts;
};

ostream& operator<<(ostream& s, const Contour& c);

#endif // !__Contour_h_

