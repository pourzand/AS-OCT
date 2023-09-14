#ifndef __Line_h_
#define __Line_h_

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
#include <vector>

#include "Darray.h"
#include "Interval.h"

using std::ostream;


/**
This structure is intended only for use internal to the ROI.
Stores set of intervals from a single line (same y and z coordinates).
The Darray of intervals is ordered (in increasing x-coordinate).
*/
struct Line {

	/// y-coordinate value.
	int y;

	/// Darray of Intervals.
  std::vector<Interval> ivl;

	/**
	Constructor that creates empty Line.
	First argument is y-coordinate.
	*/
	Line(int yv) : y(yv) {};
					
	/// Copy constructor.
	Line(const Line &l) : y(l.y), ivl(l.ivl) {};

	/// Destructor.
	~Line() {};

	/// Adds an Interval.	
	void add_ivl(const Interval &);

	/// Adds an Interval.	
	void add_ivl(const int x1, const int x2) { Interval iv(x1, x2); add_ivl(iv); };

	/// Removes points from the Line which are common with the supplied Interval.	
	void subtract_ivl(const Interval&);

	/// Removes points from the Line which are within the x-coordinate range specified.	
	void subtract_ivl(const int x1, const int x2) { Interval iv(x1, x2); subtract_ivl(iv); };

	//const Interval& const ivl_const (const long n) { assert((n>=0)&&(n<ivl.size())); return ivl[n]; };

	/**
	DANGEROUS dereferencing operator.
	Index into the array is supplied as an argument.
	There is no checking of the validity of the index (except by assert).
	Reference may be modified.
	*/
	Interval& operator[] (const long n) { assert((n>=0)&&(n<ivl.size())); return ivl[n]; };

};


// Prints the Line and included Intervals
ostream& operator<<(ostream& s, const Line& l);

/*
Comparison function for use in ordering a Darray.
Ordering is based on y-coordinate only (z not used).
*/
inline int line_comp(const Line& l1, const Line& l2) { return (l1.y - l2.y); };

#endif // !__Line_h_

