#ifndef __InferencingKS_h_
#define __InferencingKS_h_

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
#include "tools_miu.h"
#include "Feature.h"
#include "InfParam.h"

#include <stdio.h>


/**
Function for generating activation record for ImCandConf knowledge source.
Records the index of the solution element processed when the knowledge source was activated.
Adds a message of the form "Solution element index: ??".
*/
void ImCandConfR(Blackboard&);

/**
Activation score for the image candidate confidence score computer (4).
Returns 0.4 if the last activation record is of type SegmentationKS.
*/
float ImCandConfS(Blackboard&);

/**
Computes confidence scores for some features of image candidates of the next solution element.
Only does features for which all related solution elements have a best candidate selected, i.e. may not compute confidences if a related solution element is in the same group such features are handled by the GroupCandConf knowledge source.
*/
void ImCandConfA(Blackboard&);

/**
Function for generating activation record for RadLogicsCandConf knowledge source.
Records the index of the solution element processed when the knowledge source was activated.
Adds a message of the form "Solution element index: ??".
*/
//void RadLogicsCandConfR(Blackboard&);

/**
Activation score for the image candidate confidence score computer (4.5).
Returns 0.45 if the last activation record is of type SegmentationKS and the RadLogicsNoduleClassification feature exists.
*/
//float RadLogicsCandConfS(const Blackboard&);

/**
Computes confidence scores for some features of image candidates of the next solution element.
Only does features for which all related solution elements have a best candidate selected, i.e. may not compute confidences if a related solution element is in the same group such features are handled by the GroupCandConf knowledge source.
*/
//void RadLogicsCandConfA(Blackboard&);

/**
Activation score for the image candidate confidence score computer (4).
Score is 0.4 if: (1) the last N activation records were NextSolEl (where N=number of solution elements in the group); and (2) the activation record before that was not MatchCands or FreeCandidates.
*/
float FormGroupCandsS(Blackboard&);

/**
Forms group candidates and sets their overall confidence based on the partial confidences of the included image candidates (fuzzy logic).
Image candidates are only used if their partial confidence is > 0.
*/
void FormGroupCandsA(Blackboard&);

/**
Activation score for the group candidate confidence score computer (4).
Returns 0.4 if the last activation record is FormGroupCands.
*/
/*
Activation score for the group candidate confidence score computer (43).
Score is 0.43 if: (1) the next group index is > -1; (2) the set of activation records, S, contains at least as many NextSolel calls as there are solution elements in the group; and (3) the set of activation records, S, does not contain a record with name GroupCandConf and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float GroupCandConfS(Blackboard&);

/**
Computes overall confidence scores for group candidates of the next group.
Confidence scores for features of group candidates that were not handled by the ImCandConf knowledge source are computed.
Specifically, features that involve relationships between solution elements in the group.
*/
void GroupCandConfA(Blackboard&);


/**
Function for generating activation record for MatchCands knowledge source.
Records the index of the solution element processed when the knowledge source was activated.
Adds a message of the form "Solution element index: ??".
*/
void MatchCandsR(Blackboard&);

/**
Used to compute activation score for matching image candidates to next group (4).
Returns 0.4 if the last activation record is GroupCandConf.
*/
/*
Used to compute activation score for matching image candidates to next group (4).
Returns 0.4 if: (1) the next group index is > -1; (2) the set of activation records, S, contains at least as many NextSolel calls as there are solution elements in the group; and (3) the set of activation records, S, does not contain a record with name MatchCands and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float MatchCandsS(Blackboard&);

/**
Matches image candidates to the next group.
If a "MatchAboveConf" attribute is defined for a given solution element in the group, the appropriate image primitive is matched from each group candidate that has a non-zero confidence above a defined threshold.
If this attribute is not defined, then the group candidate with the highest confidence score (> 0) is selected.
If there are multiple such candidates it takes the first one in the array.
Sets the matched candidate indices and matched primitive in the solution elements of the group.
*/
void MatchCandsA(Blackboard&);


#endif // !__InferencingKS_h_
