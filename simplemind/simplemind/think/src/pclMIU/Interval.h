#ifndef __Interval_h_
#define __Interval_h_

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


/**
This structure is intended only for use internal to the ROI.
Represents continuous set of points on the same line (y and z coordinates fixed).
It is assumed that x1<=x2. The user must ensure this, the system does not do any checking to allow greater speed (except using assert).
*/
struct Interval {

	/**
	Lower x-coordinate (assumed to be <= x2).
	This must be an integer so that it can be negative for structuring elements etc.
	*/
	signed short x1;

	/**
	Upper x-coordinate (assumed to be >= x1).
	This must be an integer so that it can be negative for structuring elements etc.
	*/
	signed short x2;

	/// Constructor
	Interval(int x1v, int x2v) { assert(x1v<=x2v); x1=x1v; x2=x2v; };

	/// Constructor	
	Interval(int xv=0) { x1=x2=xv; };

	/// Copy constructor
	Interval(const Interval&);

	/// Destructor
	~Interval() { /*cout << "ivl dest" << endl;*/};

	/// Returns the number of points in the interval
	const int num_pts() const { return (x2-x1+1); };
};

/*
Comparison function for use in ordering a Darray.
Returns 0 if the intervals overlap or touch; otherwise returns <0 if i1<i2, or >0 if i1>i2.
*/
int ivl_comp_touch(const Interval& i1, const Interval& i2);

/*
Comparison function for use in ordering a Darray.
Returns 0 if the intervals overlap (does not include touching); otherwise returns <0 if i1<i2, or >0 if i1>i2.
*/
int ivl_comp_overlap(const Interval& i1, const Interval& i2);

#endif // !__Interval_h_

