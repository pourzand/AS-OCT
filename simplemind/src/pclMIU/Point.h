#ifndef __Point_h_
#define __Point_h_

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

#include <iostream>
#include <ios>
#include <istream>
#include <ostream>

using std::ostream;
using std::istream;
using std::ios;

/**
A 2-D or 3-D point.
The z-coordinate is initialized to 0 by default (so that it can handle 2-D case).
*/
struct Point {

	/// x-coordinate
	int x;

	/// y-coordinate
	int y;

	/// z-coordinate
	int z;

	/// Default constructor.
	Point() { z=0; };

	/// Constructor.
	Point(const int xv, const int yv, const int zv=0);

	/// Copy constructor.
	Point(const Point&);

	/// Destructor
	~Point();

	/// Assignment operator
	void operator=(const Point&);

	void set(const int xv, const int yv, const int zv=0);
};


// Prints the coordinates of a Point
ostream& operator<<(ostream& s, const Point& p);

// Reads the coordinates of a Point of form (x, y, z)
istream& operator>>(istream& s, Point& p);

/*
Comparison function for use in ordering a Darray.
Ordering is raster based on x,y coords only (z not used).
*/
int point_comp_xy(const Point& p1, const Point& p2);

#endif // !__Point_h_

