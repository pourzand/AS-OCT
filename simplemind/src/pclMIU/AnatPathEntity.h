#ifndef __AnatPathEntity_h_
#define __AnatPathEntity_h_

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
#include <string>
#include <vector>

//#include "Darray.h"

using std::istream;
using std::ostream;

/// A description of an anatomically or pathologically-based entity in the model
class AnatPathEntity {
public:
	/// Default constructor
	AnatPathEntity();

	/// Constructor
	AnatPathEntity(const std::string& name);

	/// Copy constructor
	AnatPathEntity(const AnatPathEntity&);

	/// Destructor
	~AnatPathEntity();

	/**
	Add a descriptor.
	The format of the descriptor is checked (no semicolon at the end of the line), if the format is recognized 1 is returned, 0 otherwise.
	If the format is not recognized the descriptor is not added.
	*/
	const int add_descriptor(const std::string& descr);

	/// Name of the entity
	const std::string& name() const;

	/// Number of descriptors
	const int n() const;

	/**
	Returns the i'th descriptor.
	If i is invalid program exits with error message to cerr.
	*/
	const std::string descriptor(const unsigned int i) const;

	/**
	Output stream operator.
	Format of output is as for input stream operator.
	*/
	friend ostream& operator<<(ostream& s, const AnatPathEntity& a);

	/**
	Input stream operator.
	Exits with an error message to cerr if format is not correct.
	The format of a AnatPathEntity object is as follows:
	AnatPathEntity: entity_name;
	descriptor;
	descriptor;
	descriptor;
	End: entity_name;
	*/
	friend istream& operator>>(istream& s, AnatPathEntity& a);

private:
	/// Name of the anatomically or pathologically-based entity
  std::string _name;

	/// Vector of descriptors
  std::vector<std::string> _descr;
  //Darray<std::string> _descr;
};

ostream& operator<<(ostream& s, const AnatPathEntity& a);
istream& operator>>(istream& s, AnatPathEntity& m);

#endif // !__AnatPathEntity_h_

