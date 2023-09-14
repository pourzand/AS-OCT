#ifndef __SchedulerKS_h_
#define __SchedulerKS_h_

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

/**
Function for computing activation score for the GroupFormer (8).
Returns 0.8 if there are solution elements on the blackboard but no groups.
*/
float GroupFormerS(Blackboard&);

/**
Form groups of solution elements on the blackboard.
Erases any previously formed groups.
*/
void GroupFormerA(Blackboard&);


/** Activation score for NextGroup (2).
Returns 0.2 if the last N activation records are all NextSolel, i.e. all elements of the group have been iterated through and no other knowledge sources activated.
Also returns 0.2 if the last knowledge source activated was the GroupFormer.
*/
float NextGroupS(Blackboard&);

/**
Sets index of next group to be processed.
The priority of the current group is set to -1.0.
If the stop-at solution element is part of the current group (just processed) then set all group priorities to -1.0.
Otherwise computes a priority score for each group.
Selects the group with the highest priority and sets the index accordingly.
The next solution element index is set to the first solution element in the group.
If there are two or more such groups, it selects the first one it encounters. If all groups have priority -1.0, then the next group index is set to -1.
Priority score for a given group is proportion of related groups which have been processed.
If the related solution element in an essential (_E) relationship has not processed then the priority score of the dependent group is set to 0.
*/
void NextGroupA(Blackboard&);


/**
Activation score for NextSolel (1).
Returns 0.1 provided next group index is not equal to -1.
*/
float NextSolelS(Blackboard&);

/**
Sets index of next solution element to be processed.
Iterates through the solution elements in the current group, in the order in which they are stored in the group.
Keeps looping repeatedly through elements.
*/
void NextSolelA(Blackboard&);


#endif // !__SchedulerKS_h_
