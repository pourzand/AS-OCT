#ifndef __Fuzzy_h_
#define __Fuzzy_h_


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

#include <ios>
#include <iostream>
#include <vector>
#include <algorithm>

#include "FPoint.h"

/**
Trapezoidal fuzzy membership function.
x-coordinates of membership function must be increasing and y-coordinates must be between 0 and 1.
*/
class Fuzzy {
public:

	/// Default constructor
	Fuzzy();

	/** Constructor.
	The Points are the vertices of the trapezoidal membership function.
	Validity of membership function is checked, program exits on error.
	*/
	Fuzzy(const std::vector<FPoint>&);

	/** Constructor for a trapezoidal membership function with 4 vertices.
	The arguments are the x-coordinates of the vertices, the y-components
	are either 0 or 1 (as indicated in the argument name).
	Validity of membership function is checked, program exits on error.
	*/
	Fuzzy(const float x0a, const float x1a, const float x1b, const float x0b);

	/** Constructor for a trapezoidal membership function with 3 vertices.
	The arguments are the x-coordinates of the vertices, the y-components
	are either 0 or 1 (as indicated in the argument name).
	Validity of membership function is checked, program exits on error.
	*/
	Fuzzy(const float x0a, const float x1b, const float x0c);

	/// Copy constructor
	Fuzzy(const Fuzzy&);

	/// Destructor
	~Fuzzy();

	/** Checks validity of membership function.
	Checks that x-coordinates of membership function are in ascending order and that y values are between 0 and 1.
	Membership function must contain at least 1 point.
	Returns 1 if OK, 0 otherwise.
	*/
	const int valid() const;

	/**
	Returns the fuzzy membership score (y-value) for a given x-value.
	If two vertices have the same y-value then the higher value is returned.
	If the membership function has no vertices then 0 is returned.
	*/
	const float val(const float x) const;

	/**
	Appends a point to the membership function.
	There is no checking as to the validity of the point.
	*/
	void append(const FPoint p);

	/**
	Multiplies the x-coordinates of the vertices of the membership function by a scaling factor.
	If the scaling factor is negative then the order of the points is reversed, so they should remain in ascending order.
	The initial validity of the membership function is assumed and not checked.
	*/
	void scale_x(const float scaling_factor);

	/**
	Adds the offset to the x-coordinates of the vertices of the membership function.
	The initial validity of the membership function is assumed and not checked.
	*/
	void offset_x(const float offset);

	/**
	Appends fuzzy set representation to a string.
	Format: [(x1, y1)(x2, y2)(x3, y3)]
	*/
	void append_to_string(std::string&) const;

	/**
	Output stream operator.
	Format: [(x1, y1) (x2, y2) (x3, y3)]
	*/
	friend ostream& operator<<(ostream& s, const Fuzzy& f);

	/**
	**** THIS METHOD HAS NOT BEEN TESTED ****
	Input stream operator.
	Expects data in a format similar to the following: [(0,0) (1,1) (2,1) (3,0)].
	If the format of the stream data is not correct the bad bit is set.
	*/
	friend istream& operator>>(istream& s, Fuzzy& f);

private:

	/// Vector of vertices
  std::vector<FPoint> _vert;
};

ostream& operator<<(ostream& s, const Fuzzy& f);
istream& operator>>(istream& s, Fuzzy& f);


/***************************/
/****	mid_Fuzzy	****/
/***************************
int * mid_Fuzzy(f)
	Fuzzy_t f;

	Returns average of x-coordinates of first point with first non-zero
		y-value and first subsequent point with y-value equal to zero
	If the second point can't be found just return x-coordinate of first
		point
	If the first point can't be found then return NULL
****************************/
//extern int * mid_Fuzzy(Fuzzy_t f);



/***************************/
/****	first_non_zero	****/
/***************************
int first_non_zero(f, x)
	Fuzzy_t f;
	int *x;

	Tries to set x parameter to x-coordinate of first Fuzzy point with
		non-zero y-value, excluding the very first point
	Returns 1 if successful
	Returns 0 if could not find such a point
	Returns -1 if first point has non-zero y-value
****************************/
//extern int first_non_zero(Fuzzy_t f, int *x);

/***************************/
/****	last_non_zero	****/
/***************************
int last_non_zero(f, x)
	Fuzzy_t f;
	int *x;

	Similar to first_non_zero except looks for last point with non-zero
		y-value, excluding the very last point
	Returns 1 if successful
	Returns 0 if could not find such a point
	Returns -1 if last point has non-zero y-value
****************************/
//extern int last_non_zero(Fuzzy_t f, int *x);

#endif // !__Fuzzy_h_
