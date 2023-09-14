#ifndef __Plane_h_
#define __Plane_h_

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

using std::ostream;

#include "Darray.h"
#include "Line.h"

/** 
This structure is intended only for use internal to the ROI.
Stores set of Lines from a single Plane (z coordinate).
The Darray of Lines is ordered (in increasing y-coordinate).
*/
struct Plane {

	/// z-coordinate value.
	int z;

	/// Modulation factor for Interval Darray.
	long ivl_mod;						

	/// Darray of Lines (a Line only exists if it has at least 1 Interval).
	Darray<Line> ln;

	/**
	Constructor that creates empty Plane.
	First argument is z-coordinate. Other arguments are modulation factors for Line and Interval Darrays (they have defaults).
	*/
	Plane(int zv, long ln_mod_v=64, long ivl_mod_v=2) : z(zv), ivl_mod(ivl_mod_v), ln(ln_mod_v) {};

	/// Copy constructor.
	Plane(const Plane &p) : z(p.z), ivl_mod(p.ivl_mod), ln(p.ln) {};

	/// Destructor.
	~Plane() {};

	/**
	DANGEROUS Line dereferencing operator.
	Index into the array is supplied as an argument.
	There is no checking of the validity of the index (except by assert).
	Reference may be modified.
	*/
	Line& operator[] (const long n) { assert((n>=0)&&(n<ln.N())); return ln(n); };

};


// Prints the Plane and included Lines
ostream& operator<<(ostream& s, const Plane& p);

/*
Comparison function for use in ordering a Darray.
Ordering is based on z-coordinate only.
*/
inline int plane_comp(const Plane& p1, const Plane& p2) { return (p1.z-p2.z); };

#endif // !__Plane_h_

