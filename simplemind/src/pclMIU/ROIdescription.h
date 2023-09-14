#ifndef __ROIdescription_h_
#define __ROIdescription_h_

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

#include <ostream>
#include <string>

#include "tools_miu.h"
#include "ROI.h"
#include "MedicalImageSequence.h"

using std::ostream;

/**
A textual description of an ROI.
Currently supports descriptions of the following forms: circle radius_mm.
*/ 
class ROIdescription {
public:
	/// Constructor (does not check validity of description)
	ROIdescription(const std::string& descr);

	/// Copy constructor.
	ROIdescription(const ROIdescription&);

	/// Destructor
	~ROIdescription();

	/// Returns description
	const std::string& description() const;

	/**
	Generates the ROI and stores it in the ROI argument (assumed to be empty).
	Returns 1 if successful (description valid), 0 otherwise.
	*/
	const int roi(ROI&, const MedicalImageSequence&) const;

	/// Prints the ROI description
	friend ostream& operator<<(ostream& s, const ROIdescription& r);

private:
	/// Description
  std::string _descr;
};


// Prints the ROI description
ostream& operator<<(ostream& s, const ROIdescription& r);

// Reads the coordinates of a ROIdescription of form (x, y, z)
//istream& operator>>(istream& s, ROIdescription& p);

#endif // !__ROIdescription_h_

