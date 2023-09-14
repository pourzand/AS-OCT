#ifndef __MemManageKS_h_
#define __MemManageKS_h_

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
#include <stdio.h>

/**
Function for generating activation record for FreeCandidates knowledge source.
Records the index of the solution element processed when the knowledge source was activated.
Adds a message of the form "Solution element index: ??".
*/
void FreeCandidatesR(Blackboard&);

/**
Function for computing activation score for FreeCandidates (3).
Score is 0.3 if: (1) MatchCands has been called for the next solution element; and (2) the set of activation records, S, does not contain a record of type FreeCandidatesKS and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
We could check whether the solution element has a matched primitive (rather than using MatchCands), but for some objects we don't always expect a match and we don't want to save these candidates unnecessarily.
*/
float FreeCandidatesS(Blackboard&);

/**
For the next solution element remove the unmatched candidates.
Unless: (1) the solution element has a RetainCandidates attribute;or (2) there is a solution element, for which MatchCands has not been called, that has a SameCandidatesAs attribute that references this solution element.
If the solution element itself has a SameCandidatesAs feature then this knowledge source should also be applied to the related solution element.
In addition to SameCandidatesAs the same logic above applies for DistanceMapRegionGrowing and AddMatchedCandidates attributes.
We could check whether the solution element has a matched primitive (rather than using MatchCands), but for some objects we don't always expect a match and we don't want to save these candidates unnecessarily.
*/
void FreeCandidatesA(Blackboard&);


#endif // !__MemManageKS_h_
