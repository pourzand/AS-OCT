#ifndef __FPoint_h_
#define __FPoint_h_

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

#include <iostream>
#include <istream>
#include <math.h>
#include <ostream>
#include <string>

using std::istream;
using std::ostream;

/**
A 2-D or 3-D point with coordinates as floats.
The z-coordinate is initialized to 0 by default (so that it can handle 2-D case).
*/ 
struct FPoint {

	/// x-coordinate
	float x;

	/// y-coordinate
	float y;

	/// z-coordinate
	float z;

	/// Default constructor.
	FPoint() { z=0; };

	/// Constructor.
	FPoint(const float xv, const float yv, const float zv=0);

	/// Copy constructor.
	FPoint(const FPoint&);

	/// Destructor
	~FPoint();

	/// Assignment operator
	void operator=(const FPoint&);

	/// Returns the Euclidean distance between two points
	float distance(const FPoint&);

	/**
	Appends 2D point representation to a string (up to 5 decimal places for coord values).
	Format: (x1, y1)
	*/
	void append_to_string_2d(std::string&) const;

};


// Prints the coordinates of a FPoint
ostream& operator<<(ostream& s, const FPoint& p);

// Reads the coordinates of a FPoint of form (x, y, z)
istream& operator>>(istream& s, FPoint& p);

#endif // !__FPoint_h_

