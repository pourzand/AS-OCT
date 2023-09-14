#ifndef __MemParam_h_
#define __MemParam_h_

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

#include <string>

#include "Attribute.h"

/// An abstract base class for storing parameters relating to memory management on the blackboard
class MemParam : public Attribute {
public:

	/// Constructor
	MemParam();

	/// Copy constructor
	MemParam(const MemParam&);

	/// Destructor
	virtual ~MemParam();

	/// Type of attribute
	const char* const type() const { return "MemParam"; };	
};


/// Specifies that candidates should not be freed after a match is made to the solution element
class RetainCands : public MemParam {
public:
	/// Constructor
	RetainCands();

	/// Copy constructor
	RetainCands(const RetainCands&);

	/// Destructor
	~RetainCands();

	/// Name of the attribute
	const char* const name() const { return "RetainCands"; };

	/**
	Write parameter to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Attribute: End <newline>
	*/
	void write(ostream& s) const;
};

#endif // !__MemParam_h_

