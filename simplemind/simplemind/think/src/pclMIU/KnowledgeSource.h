#ifndef __KnowledgeSource_h_
#define __KnowledgeSource_h_

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

#include "Blackboard.h"

/// A class for knowledge sources that can operate on the blackboard
class KnowledgeSource {
public:
	/// Constructor
	KnowledgeSource(const std::string name, const std::string type, float (*activation_score) (Blackboard&), void (*activate) (Blackboard&), void (*activation_record) (Blackboard&)=0);

	/// Copy constructor
	KnowledgeSource(const KnowledgeSource&);

	/// Destructor
	~KnowledgeSource();

	/// Name of the knowledge source
	const std::string& name() const;

	/// Type of the knowledge source
	const std::string& type() const;

	/**
	Add activation record.
	A record is added containing the knowledge source name and type.
	If an activation record function was supplied in the constructor it is called, this was intended to allow the addition of messages to the activation record (using blackboard method add_message_to_last_act_rec). 
	*/
	void add_activation_rec(Blackboard&) const;

	/// Compute an activation score to determine which knowledge source will be activated next
	const float activation_score(Blackboard&) const;

	/// Operate on the blackboard, i.e. contribute to the solution
	void activate(Blackboard&) const;

private:
	/// Name of the knowledge source
	std::string _name;

	/// Type of the knowledge source
	std::string _type;

	/// Pointer to function to compute an activation score to determine which knowledge source will be activated next
	float (*_activation_score) (Blackboard&);

	/// Pointer to function to operate on the blackboard, i.e. contribute to the solution
	void (*_activate) (Blackboard&);

	/**
	Pointer to function to be called after an activation record is added to the blackboard.
	The intention is that the function should be used to add messages to an activation record when appropriate (using blackboard method add_message_to_last_act_rec).
	It is zero by default in which case nothing is called.
	*/
	void (*_activation_record) (Blackboard&);
};

#endif // !__KnowledgeSource_h_

