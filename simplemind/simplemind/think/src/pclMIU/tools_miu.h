#ifndef __tools_miu_h_
#define __tools_miu_h_

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
#include <stdlib.h>

#include "MedicalImageSequence.h"
#include "FPoint.h"
#include "Fuzzy.h"
#include "ROItraverser.h"

/**
Returns mean of the ROI given by rt.
Assumes image is a CT.
Returns 0 if ROI is empty.
*/
double meanHU(MedicalImageSequence& mis, ROItraverser& rt);

/**
Returns mean of the ROI given by rt.
Assumes image is a CT.
*/
int medianHU(MedicalImageSequence& mis, ROItraverser& rt);

/**
Returns sphericity of the ROI given by rt.
Returns -1.0 if radius of ROI is zero.
*/
float sphericity(const ROI& r, const FPoint& cent, float volume, const MedicalImageSequence& mis);
//float sphericity(const ImageRegion* r0);

/**
Returns a 2D Euclidean distance map for the given slice of the ROI.
The distances in the image are in mm and therefore the MedicalImageSequence is required to provide the spacings.
The dimensions of the resulting image are the same as the MedicalImageSequence.
However, since the algorithm checks pixels in a neighborhood when computing the map, the outermost pixels at the edge of the image will always have value 0 in the resulting image.
*/
float* distance_map_2d(const ROI& r, const int z, const MedicalImageSequence& mis);

/// Rounds a float to the nearest integer
int round_float(float v);

/// Converts an FPoint to a Point by rounding the coordinates to the nearest integer
void fpoint_to_point(const FPoint&, Point&);

/**
Returns the (theoretical) slice location corresponding to a z-coordinate, where the z-coordinate may lie outside the image dimensions. If the coordinate is outside the dimensions the spacing between the most first (or last) pair of slices is used as appropriate.
If the image is 2D, the function always returns 0.
*/
float slice_loc(const int z, const MedicalImageSequence& mis);

/**
Converts a gray level (GL) to HU.
Checks whether the modality of the image sequence is CT, returns 0 if not.
Uses the rescale slope and intercept from the first image in the series, i.e., assumes they are constant throughout the series.
Program exits with an error message if the slope and intercept cannot be determined (if there are no images in the series).
*/
const int GL_to_HU(const int gl, int& hu, const MedicalImageSequence& mis);

/**
Converts a HU to gray level (GL).
Checks whether the modality of the image sequence is CT, returns 0 if not.
Uses the rescale slope and intercept from the first image in the series, i.e., assumes they are constant throughout the series.
Program exits with an error message if the slope and intercept cannot be determined (if there are no images in the series).
*/
const int HU_to_GL(const int hu, int& gl, const MedicalImageSequence& mis);

/**
Advances string index, i, over any blanks or until end of string is reached.
Returns 0 if end of string reached, 1 otherwise.
*/
const int skip_blanks(const std::string& s, int& i);

/**
Advances string index, i, until it reaches the designated character or until end of string is reached.
If character is found 1 is returned, otherwise 0.
*/
int advance_to(const std::string& s, const char c, int& i);

/**
Compute the maximium diameter and its perpendicular diameter of a given ROI.
The corresponding Points will also be set.
*/
void compute_diameters(const ROI& currentRoi, const float row_pixel_spacing, const float col_pixel_spacing, Point& mdist_pt1, Point& mdist_pt2, Point& mpdist_pt1, Point& mpdist_pt2, double& max_diameter, double& perp_diameter);

/**
Starting from the index position, i, in the string, s, reads an ROIdescription string from s and appends it to roi_descr. It skips over characters up to '[', going to the end of the string if necessary and then reads the descriptor, leaving the index at the character past ']' or at the end of the string.
If the string has parameters as genes and a chromosome is provided then they are reflected in the generated ROIdescription. If no chromosome is provided the default value is used.
1 is returned if a valid ROIdescription is read, 0 otherwise.
*/
int read_roi_descr_genetic(const std::string& s, std::string& roi_descr, int& i, const std::string& chromosome, std::vector<bool>& bits_used);

/**
Appends characters into word up to blanks which it skips over, leaving the index at the start of the next word or at the end of the string.
If the index starts at a blank then nothing is read into word.
1 is returned if data read into word, 0 otherwise.
*/
int read_word(const std::string& s, std::string& word, int& i);

/**
Reads an integer into v, skipping whitespaces - DOES NOT ADVANCE THE STRING INDEX.
Returns 0 on failure.
*/
int read_integer(const std::string& s, int& v, int& i);

/**
Reads a float, skipping whitespaces - DOES NOT ADVANCE THE STRING INDEX.
Returns 0 on failure.
*/
int read_float(const std::string& s, float& f, int& i);

/**
Reads a FPoint (2D or 3D), and advances the string index accordingly.
Returns 1 if format was correct, e.g. (1,1).
Returns 0 otherwise.
*/
int read_fpoint(const std::string& s, FPoint& p, int& i);

/**
Reads a FPoint (2D or 3D), and advances the string index accordingly. Any number can be a genetic value. It also works if the parameter does not include genetic values.
Returns 1 if format was correct, e.g. (1 {0, 3, 1, 10},1).
Returns 0 otherwise.
*/
int read_fpoint_genetic(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, FPoint& p, int& i);

/**
Reads a fuzzy membership function, and advances the string index accordingly.
Leading blanks are skipped if necessary.
Returns 1 if format was correct, e.g. [(0,0) (1,1) (2,1) (3,0)].
Returns 0 otherwise.
*/
int read_fuzzy(const std::string& s, Fuzzy& f, int& i);

/**
Reads a fuzzy membership function, and advances the string index accordingly. Any number in the membership function can be a genetic value. It also works if the parameter does not include genetic values.
Leading blanks are skipped if necessary.
Returns 1 if format was correct, e.g. [(0,0) (1,1) (2,1) (3,0)] or [(0,0) (1,1) (2,1) (3 {8, 10, 3, 30},0)].
Returns 0 otherwise.
*/
int read_fuzzy_genetic(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, Fuzzy& f, int& i);

/**
Reads a portion of a chromosome (gene), and sets the argument value to the binary value of the gene,
where start_bit is the lowest power of 2 and stop_bit is the highest.
Returns 1 if 0 <= start_bit <= stop_bit < length of chromosome.
Returns 0 otherwise and value is not set.
*/
int gene_value(const std::string& chromosome, int start_bit, int stop_bit, int& value);

/**
Reads a genetic model value and uses to read the specified portion of a chromosome (gene) and then compute an integer value (considering the low and high values provided). It advances the string index accordingly.
Leading blanks are skipped.
Returns 1 if next non-blank character is not '{' or format was correct, e.g. {0, 7, -800, -100}
Returns 0 otherwise and v is not set.
*/
int read_gene_integer(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, int& v, int& i);

/**
Reads a genetic model value and uses to read the specified portion of a chromosome (gene) and then compute a float value (considering the low and high values provided). It advances the string index accordingly.
Leading blanks are skipped.
Returns 1 if next non-blank character is not '{' or format was correct, e.g. {0, 7, -8.1, -1.2}
Returns 0 otherwise and v is not set.
*/
int read_gene_float(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, float& v, int& i);

/// Rounds a value down to the nearest multiple of step
int rnd_step(const int v, const int step);

/// Rounds a value up to the nearest multiple of step
int rnd_up_step(const int v, const int step);

/**
Subsamples an image sequence by selecting every i'th point in the x, y and z directions, where i is given by the respective step parameters.
If parameters are invalid then 0 is returned.
*/
MedicalImageSequence* subsample(MedicalImageSequence& mis, const int x_step, const int y_step, const int z_step);

/**
Scales an ROI consistent with the image subsampling.
Rounding of rescaled raster endpoints is done conservatively.
Parameters must be >=1, if they are invalid then program exits with error message.
*/
void subsample_roi(const ROI& orig_roi, ROI& new_roi, const int x_step, const int y_step, const int z_step);

/**
Expands an ROI consistent with the image subsampling.
Rounding of rescaled raster endpoints is done generously, as is filling of missing lines and planes.
Expanded ROI will not go outside the given bounding region.
Parameters must be >=1, if they are invalid then program exits with error message.
*/
void expand_roi(const ROI& orig_roi, ROI& new_roi, const int x_step, const int y_step, const int z_step, const ROI& bounding_region);

/**
Computes the distance in mm between two points.
Function is dangerous in that it does not check whether points lie within dimensions of the image sequence - although it is assumed.
Assuming equal row and column pixel spacings on all images in the sequence.
Does not assume equal spacings between images in the z-direction.
*/
float distance_mm(const Point& p1, const Point& p2, const MedicalImageSequence& mis);

/**
Computes the distance in mm between two points with coordinates as floats.
Function is dangerous in that it does not check whether points lie within dimensions of the image sequence - although it is assumed.
Assuming equal row and column pixel spacings on all images in the sequence.
Does not assume equal spacings between images in the z-direction.
*/
float distance_mm(const FPoint& p1, const FPoint& p2, const MedicalImageSequence& mis);

/**
Computes a slice location for a non-integer z-coordinate.
The function is dangerous in that it assumes 0<=z<mis.zdim(), but does not check the condition.
*/
float fractional_slice_loc(const float z, const MedicalImageSequence& mis);

/**
Computes the point with x,y and z offsets (in mm) from a given point.
Assumes equal row and column pixel spacings on all images in the sequence.
Method assumes that first slice is at top of chest and increasing z corresponds to increasing slice location.
Does not assume equal spacings between images in the z-direction, but it is assumed that image locations are sequential.
The points may lie outside the image dimensions. If so the spacing between the most first (or last) pair of slices is used if needed.
*/
Point offset_mm(const Point& p, const float x_offset, const float y_offset, const float z_offset, const MedicalImageSequence& mis);

#endif // !__tools_miu_h_

