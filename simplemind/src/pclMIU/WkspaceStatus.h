#ifndef __WkspaceStatus_h_
#define __WkspaceStatus_h_

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     UCLA Department of Radiological Sciences (c) 1997
** ======================================================================
** This software comprises unpublished confidential information of the
** University of California and may not be used, copied or made
** available to anyone, except with written permission of the
** Department of Radiological Sciences and Regents of the University
** of California.  All rights reserved.
**  
** This software program and documentation are copyrighted by The
** Regents of the University of California. The software program and
** documentation are supplied "as is", without any accompanying
** services from The Regents. The Regents does not warrant that the
** operation of the program will be uninterrupted or error-free. The
** end-user understands that the program was developed for research
** purposes and is advised not to rely exclusively on the program for
** any reason.
**  
** IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY
** PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
** DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
** SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
** CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE
** UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
** PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
** CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
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


/**
This structure is intended only for use internal to the ROI.
Enumerated type for indicating the status of a Workspace following a move or find operation.
Traveral of an ROI is based on a raster ordering. The ROI is treated as a set of planes, which consist of lines, which consist of intervals, which consist of continuous points.
The WkspaceStatus indicates whether the last move was between one of these units.
*/

enum WkspaceStatus
{
	/// The Workspace moved within an Interval
	ROI_STAT_OK,

	/// The Workspace moved to a new Interval
	NEW_INTERVAL,

	/// The Workspace moved to a new Line
	NEW_LINE,

	/// The Workspace moved to a new Plane
	NEW_PLANE,

	/// The Workspace has reached the end of the ROI
	END_ROI,

	/// The requested location in the ROI could not be found, or current Workspace (pre-move) is invalid.
	NOT_FOUND
};

#endif // !__WkspaceStatus_h_

