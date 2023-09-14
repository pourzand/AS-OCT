#ifndef __Attribute_h_
#define __Attribute_h_

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

#include "Darray.h"

using std::ostream;

/// An abstract base class for storing attributes on the blackboard
class Attribute {
public:

	/// Constructor
	Attribute();

	/// Copy constructor
	Attribute(const Attribute&);

	/// Destructor
	virtual ~Attribute();

	/// Name of the attribute
	virtual const char* const name() const = 0;

	/// Type of the attribute (Feature, SearchArea, etc)
	virtual const char* const type() const = 0;

	/**
	Add the name and index (blackboard) of a related solution element.
	Does not do any checking as to whether name or index exists.
	*/
	void add_rel_solel(const std::string& rel_name, const int rel_index);

	/// Number of related solution elements
	const int num_rel_solels() const;

	/**
	Name of i'th related solution element
	Program exits if i is invalid.
	*/
	const std::string& rel_solel_name(const int i) const;

	/**
	Index (blackboard) of i'th related solution element
	Program exits if i is invalid.
	*/
	const int rel_solel_index(const int i) const;

	/// Set to 1 the flag that indicates whether a _B (bi-directional relationship) was included in the model descriptor
	void set_b_flag();

	/// Return value of flag that indicates whether a _B (bi-directional relationship) was included in the model descriptor
	const int b_flag() const;

	/**
	Set to 1 the flag that indicates whether an _E (essential relationship) was included in the model descriptor.
	Used to indicated whether a SearchArea attribute is essential if candidates are to be computed.
	For an essential relationship, the search area should be cleared (i.e. no candidates will be extracted) if the related solution element has not been matched.
	*/
	inline void set_e_flag() { _e_flag = 1; };

	/**
	Return value of flag that indicates whether an _E (essential relationship) was included in the model descriptor.
	Used to indicated whether a SearchArea attribute is essential if candidates are to be computed.
	For an essential relationship, the search area should be cleared (i.e. no candidates will be extracted) if the related solution element has not been matched.
	Initialized to zero.
	*/
	inline const int e_flag() const { return _e_flag; };

	/// For each provided bit that is True, set it to True in the member (otherwise do nothing to the member).
	void set_chromosome_bits_used(const std::vector<bool> bits);

	/// Returns true if none have been set as used
	const bool no_chromosome_bits_used() const;

	/**
	Returns whether chromosome index was used (if outside range then returns false).
	*/
	inline const bool chromosome_bit_used(const int index) const { return (index>=0)&&(index<_chromosome_bits_used.size())&&_chromosome_bits_used[index]; };

	/**
	Write attribute to an output stream operator.
	This member function is typically overloaded in derived classes.
	Format of output is as follows.
	Attribute: Type: Name <newline> Related SolElement: name <newline> Related SolElement: name <newline> Attribute: End <newline>
	*/
	virtual void write(ostream& s) const;

protected:
	/**
	Write the start of an attrbute to an output stream operator.
	Format of output is as follows.
	Attribute: Type: Name <newline> Related SolElement: name <newline> Related SolElement: name <newline>
	*/
	void _write_start_attribute(ostream& s) const;

	/**
	Write the end of an attrbute to an output stream operator.
	Format of output is as follows.
	Attribute: End <newline>
	*/
	void _write_end_attribute(ostream& s) const;

protected:
	/**
	Flag that indicates whether a _B (bi-directional relationship) was included in the model descriptor.
	Initialized to zero.
	*/
	int _b_flag;

	/**
	Flag that indicates whether an _E (essential relationship) was included in the model descriptor.
	Used to indicated whether a SearchArea attribute is essential if candidates are to be computed.
	For an essential relationship, the search area should be cleared (i.e. no candidates will be extracted) if the related solution element has not been matched.
	Initialized to zero.
	*/
	int _e_flag;

private:
	/// Vector of solution element indices (blackboard)
	Darray<int> _index_rel_solel;

	/// Vector of solution element names
	Darray<std::string> _name_rel_solel;
	
	/** 
	Indicates which chromosome bit numbers were used in setting this attributes values.
	The vector is NOT guaranteed to be the length of the chromosome, any bits not included in the vector are assumed to be False, 
	i.e., if the vector is empty we assume no chromosome bits were used.
	*/
	std::vector<bool> _chromosome_bits_used;
};


#endif // !__Attribute_h_

