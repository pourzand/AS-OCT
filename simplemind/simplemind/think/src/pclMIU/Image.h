#ifndef __Image_h_
#define __Image_h_

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     UCLA Department of Radiological Sciences
**     Imaging and Information Systems (c) 1997
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
** The UCLA Radiology Information and Picture Archiving and
** Communications System (RIS/PACS) is a division of the Department of
** Radiological Sciences, University of California at Los Angeles.
**
** for more information, or for permission to use this software for
** commercial or non-commercial purposes, please contact:
**
** Daniel J. Valentino, Ph.D.
** Assistant Professor and Faculty Director of Imaging and Information Systems
** Department of Radiological Sciences
** Mail Stop172115
** UCLA Medical Center
** Los Angeles  CA 90024-1721
** 310-794-7131
** http://www.radsci.ucla.edu/
** ======================================================================
-*/

#include<fstream>
#include "misc.h"

using std::ifstream;
using std::ofstream;

/// Simple 16 bit gray scale image slice with on demand data loading.
class Image {
  /**
   *  Overloaded stream insertion operator to write contents to a file.
   *  Must ensure that file is opened first.
   *  Usage: file << anImage.
   */
  friend ofstream& operator<<(ofstream& file, const Image& image);
  /**
   *  Overloaded stream extraction operator to get data from file.
   *  Must ensure that file is opened, and in proper format.
   *  Usage: file >> anImage.
   */
  friend ifstream& operator>>(ifstream& file, Image& image);

public:
  /**
   *  Constructor.
   *  @param width Width of image in pixels.
   *  @param height Height of image in pixels.
   *  @param bits_per_pixel Bits used out of 16.
   *  @param file_name File where 16 stored bit per pixels reside.  If \
   *         file name is omitted, a black image will be generated.
   *  @param file_offset The byte number (0 based) at which the 16 bit \
   *         data starts.  Omitted parameter denotes zero file offset.
   */
  Image(const int instance_number, const int width, const int height,
	const int bits_per_pixel,
	const char* const file_name = 0,
	const int file_offset = 0,
	const float rescale_slope = 1,
	const float rescale_intercept = 0);


  /**
   *  Constructor.
   *  @param width Width of image in pixels.
   *  @param height Height of image in pixels.
   *  @param bits_per_pixel Bits used out of 16.
   *  @param pixels Array of image data to build image from.
   */
  Image(const int instance_number, const int width, const int height,
	const int bits_per_pixel,
	short* pixels,
	const float rescale_slope = 1,
	const float rescale_intercept = 0);

  /// Copy constructor (made public by BW).
  Image(const Image& im);

  /// Overloaded assignment operator (made public by BW).
  const Image& operator=(const Image& Rhs);

  /// Destructor.
  ~Image();

  /// Causes image data to be read from disk into memory.
  void force_load();

  /// Causes image data to be discarded from memory.
  void unload();

  /// Returns image instance number.
  const int instance_number() const { return _instance_number; };

  /// Returns height of image in pixels.
  const int height() const;

  /// Returns width of image in pixels.
  const int width() const;

  /// Returns number of bits used (out of 16) for pixel value.
  const int bits_per_pixel() const;

  /// Returns the rescale slope
  const float rescale_slope() const;

  /// Returns the rescale intercept
  const float rescale_intercept() const;

  /// Returns value of pixel at coordinates x, y.
  const short pixel(const int x, const int y);

  /**
   *  Calling this member insures that image data is
   *  loaded from file if necessary.  After calling, the image data
   *  will remain in memory.
   *  @memo Returns pointer to two bytes per pixel image data in row
   *  major order.
   */
  short*& pixel_data();

  /**
   *  Calling this member DOES NOT insure that image data is
   *  loaded from file.  It will return a zero if it has not been.
   *  @memo Returns pointer to two bytes per pixel image data in row
   *  major order if data has been loaded.
   */
  short*& unsafe_pixel_data();

  /// Returns whether the image data has been loaded from disk to memory.
  const bool pixels_in_memory() const;

private:
  /// Not to be used.
  Image();

  /// Performs the real work of reading image data into memory.
  void _load_image_from_file();

  /// Maps image data memory to disk file.
  //void _map_image_from_file();

  /// Image instance number
  int _instance_number;

  /// Image width in pixels.
  int _width;

  /// Image height in pixels.
  int _height;

  /// Number of bits (out of 16) used per pixel.
  int _bits_per_pixel;

  /// rescale slope
  float _rs;

  /// rescale intercept
  float _ri;

  /// Name of image file, 0 means black image.
  char* _pixel_filename;

  /// Pointer to array of pixels, 0 if unallocated.
  short* _pixel_data;

  /// Offset (zero based) of first pixel in file.
  int _file_offset;
};

inline
const int Image::height() const
{
  return _height;
};

inline
const int Image::width() const
{
  return _width;
};

inline
const float Image::rescale_slope() const
{
  return _rs;
};

inline
const float Image::rescale_intercept() const
{
  return _ri;
};

inline
short*& Image::unsafe_pixel_data()
{
  return _pixel_data;
};


#endif /* !__Image_h_ */
