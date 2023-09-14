#ifndef __SegmentationKS_h_
#define __SegmentationKS_h_

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

#include "tools_miu.h"
#include "SearchArea.h"
#include "SegParam.h"
#include "Blackboard.h"
#include "KStools.h"

#include <stdio.h>
#include <pcl/os.h>


/**
Function for generating activation record for segmentation knowledge sources
Records the index of the solution element processed when the knowledge source was activated.
Adds a message of the form "Solution element index: ??".
*/
void SegmentationR(Blackboard&);


/**
Function for computing activation score for AddMatchedCandidates (65).
Score is 0.65 if: (A) the next solution element to be processed has a AddMatchedCandidates attribute; and either (B) [(1) the group containing the solution element specified by the AddMatchedCandidates attribute has been processed (i.e. has priority of -1); and (2) the set of activation records, S, does not contain a record with name AddMatchedCandidates and solution element index (message) equal to the next solution element (where S is formed by including the most recent activation records until NextGroup is found)]; OR (C) [(1) the solution element specified by the AddMatchedCandidates attribute is in the current group being processed; and (2) the set of activation records, S1, contains a record (R) with type SegmentationKS and solution element index (message) equal to the solution element specified by the AddMatchedCandidates attribute (where S1 is formed by including the most recent activation records until NextGroup is found); and (3) the set of activation records, S2, does not contain a record with name AddMatchedCandidates and solution element index (message) equal to the next solution element (where S2 is formed by including the most recent activation records until R is found).]
*/
float AddMatchedCandidatesS(Blackboard&);

/**
Make a copy of the matched image candidates (primitives only) of the solution element(s) specified in the AddMatchedCandidates relationship.
Feature values and confidence scores are initialized according to the current solution element being processed, i.e. only the image primitive is actually copied.
Does not check minimum number of voxels requirement (even if MinNumVoxels SegParam is specified).
*/
void AddMatchedCandidatesA(Blackboard&);

/**
Function for computing activation score for the DistanceMap2DPercLocal3DMax knowledge source (65).
Score is 0.65 if: (1) the next solution element to be processed has a DistanceMap25DPercMax attribute; and (2) the set of activation records, S, does not contain a record with name DistanceMap2DPercLocal3DMax and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float DistanceMap2DPercLocal3DMaxS(Blackboard&);

/**
Generate image candidates by thresholding a 2D Euclidean distance map (EDM) of the search area.
The EDM is computed from the search area. The seed point for region growing in the EDM is the a local EDM maximum. This local EDM maximum is determined by searching within a 3-D connected component of the matched primitive of the related solution element. For a candidate to be formed the local maximum must exceed a threshold specified as a parameter of the DistanceMap25DPercMax attribute.
The distance map is computed in mm and the threshold is given as a percentage of the local maximum. The percentage is a number in the range [0.0, 100.0].
For a given voxel to be included in the candidate its value must be greater than or equal to this threshold. A single candidate is formed from a single connected component.
While doing a region growing on the EDM from the point of local maximum there is also gray level thresholding that constrains the growing s.t. voxels are only added if the HU value is within a specified range of the HU value at the EDM local maximum (given as a parameter).
Incorporating the attenuation thresholding stops bright nodules bleeding into faint vessels and faint nodules bleeding into bright vessels.
There is a limit on the maximum z-distance for growing so that we don't have to compute the distance map for all slices all the time, which would be memory intensive. Input parameters to this method give a maximum z-distance measured from the first and last points of the related connected component.
If the region growing reaches the maximum z-distance the object is considered too big to be a nodule and no candidate is generated.
Since we computing only a 2D distance map a check is done to determine whether the minimum distance values for the first and last slices might actually be in the z-dimension, and thus whether the first and last slices should be removed from the candidate. If there are more than two slices in a candidate ROI and the slice_spacing/3 at the first slice is less than the threshold then all voxels from the first slice of the ROI are removed. A similar check is then performed for the last slice of the ROI.
Before creating a candidate the minimum number of voxels requirement is checked if specified in MinNumVoxels SegParam.
*/
void DistanceMap2DPercLocal3DMaxA(Blackboard&);

float DistanceMapRegionGrowingS(Blackboard&);
void DistanceMapRegionGrowingA(Blackboard&);

float DistanceMapWatershedS(Blackboard&);
void DistanceMapWatershedA(Blackboard&);


/**
Function for computing activation score for extracting contiguous regions from the search area as candidates (6).
Score is 0.6 if the set of activation records, S, does not contain a record of type SegmentationKS and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float FormCandsFromSearchAreaS(Blackboard&);

/**
Generates contiguous image candidates from the search area provided the solution element has at least one attribute of type SearchArea (and the related primitive has been matched).
Checks minimum number of voxels requirement if specified in MinNumVoxels SegParam.
*/
void FormCandsFromSearchAreaA(Blackboard&);

float GrowPartSolidS(Blackboard&);
void GrowPartSolidA(Blackboard&);


/**
Function for computing activation score for the NeuralNetKeras KS (65).
Score is 0.65 if: (1) the next solution element to be processed has a LineToDots attribute; and (2) the set of activation records, S, does not contain a record with name LineToDots and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float LineToDotsS(Blackboard&);

/**
Tracks a smooth curve (provided by the search area) at intervals specified by a spacing (and searching ahead up to check_distance if there are discontinuities), starting in the axis direction specified. Axis is "x" or "y". Spacing and check_distance are in mm. If the direction is x, the leftmost point is used as the start, and if the direction is y, the top point is used. The algorithm operates in 2D and processes each slice independently, finding one curve per slice.
*/
void LineToDotsA(Blackboard&);


/**
Function for computing activation score for the NeuralNetKeras KS (65).
Score is 0.65 if: (1) the next solution element to be processed has a NeuralNetKeras attribute; and (2) the set of activation records, S, does not contain a record with name NeuralNetKeras and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float NeuralNetKerasS(Blackboard&);

/**
Generate an image candidate by calling a Keras CNN model.
Will ignore MinNumVoxels and UseSubsampled.
*/
void NeuralNetKerasA(Blackboard&);


/**
Function for computing activation score for the PlatenessThreshRegGrow (65).
Score is 0.65 if: (1) the next solution element to be processed has a PlatenessThreshRegGrow attribute; and (2) the set of activation records, S, does not contain a record with name PlatenessThreshRegGrow and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float PlatenessThreshRegGrowS(Blackboard&);

/**
Generate image candidates by thresholding voxels in the search area based on their plateness values.
XXXX.
Before creating a candidate the minimum number of voxels requirement is checked if specified in MinNumVoxels SegParam.
*/
void PlatenessThreshRegGrowA(Blackboard&);


/**
Function for computing activation score for reading an roi from an external file as the matched primitive (67).
Score is 0.67 if: (1) the designated roi_directory has a file called solution_element_name.roi; and (2) the set of activation records, S, does not contain a record of type SegmentationKS and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float ReadMatchedRoiS(Blackboard&);

/**
Generates contiguous image candidates from the search area provided the solution element has at least one attribute of type SearchArea (and the related primitive has been matched).
Checks minimum number of voxels requirement if specified in MinNumVoxels SegParam.
*/
void ReadMatchedRoiA(Blackboard&);

/**
Function for computing activation score for SameCandidatesAs (65).
Score is 0.65 if: (A) the next solution element to be processed has a SameCandidatesAs attribute; and either (B) [(1) the group containing the solution element specified by the SameCandidatesAs attribute has been processed (i.e. has priority of -1); and (2) the set of activation records, S, does not contain a record with name SameCandidatesAs and solution element index (message) equal to the next solution element (where S is formed by including the most recent activation records until NextGroup is found)]; OR (C) [(1) the solution element specified by the SameCandidatesAs attribute is in the current group being processed; and (2) the set of activation records, S1, contains a record (R) with type SegmentationKS and solution element index (message) equal to the solution element specified by the SameCandidatesAs attribute (where S1 is formed by including the most recent activation records until NextGroup is found); and (3) the set of activation records, S2, does not contain a record with name SameCandidatesAs and solution element index (message) equal to the next solution element (where S2 is formed by including the most recent activation records until R is found).]
*/
float SameCandidatesAsS(Blackboard&);

/**
Make a copy of the image candidates (primitives only) of the solution element(s) specified in the SameCandidatesAs relationship.
Feature values and confidence scores are initialized according to the current solution element being processed, i.e. only the image primitive is actually copied.
Does not check minimum number of voxels requirement (even if MinNumVoxels SegParam is specified).
*/
void SameCandidatesAsA(Blackboard&);

/**
Function for computing activation score for the ThreshRegGrow (65).
Score is 0.65 if: (1) the next solution element to be processed has a ThreshRangeGL attribute; and (2) the set of activation records, S, does not contain a record with name ThreshRegGrow and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float ThreshRegGrowS(Blackboard&);

/**
Generate image candidates by 3D threshold-based region growing.
Checks minimum number of voxels requirement if specified in MinNumVoxels SegParam.
*/
void ThreshRegGrowA(Blackboard&);

/**
Function for computing activation score for MaxCostPath (65).
Score is 0.65 if: (1) the next solution element to be processed has a MaxCostPath attribute; and (2) the set of activation records, S, does not contain a record with name MaxCostPath and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
float MaxCostPathS(Blackboard&);

/**
Generate image candidates by generating 2D maximum cost paths.
One path for each slice in which the search area exists.
The image from which the path is computed is formed as follows: voxels inside the search area retain their but are capped at -100HU (to avoid too much attraction to sternum); voxels outside take the minimum value of any voxels inside; all voxels are inverted by subtracting their value from 4095.
The path is in the y-direction.
The computation of the path respects the UseSampled attribute.
Checks minimum number of voxels requirement if specified in MinNumVoxels SegParam.
*/
void MaxCostPathA(Blackboard&);

#endif // !__SegmentationKS_h_
