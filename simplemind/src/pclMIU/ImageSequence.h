#ifndef __ImageSequence_h_
#define __ImageSequence_h_

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

#include "Image.h"

//extern "C" {
//#include "assert.h"
//}

/// A sequential set of Images with the same geometry and bit depth.
class ImageSequence
{
  /**
   *  Overloaded stream insertion operator to write contents to a file.
   *  Must ensure that file is opened first.
   *  Usage: file << anImageSequence.
   */
  friend ofstream& operator<<(ofstream& file, const ImageSequence& is);
  /**
   *  Overloaded stream extraction operator to get data from file.
   *  Must ensure that file is opened, and in proper format.
   *  Usage: file >> anImageSequence.
   */
  friend ifstream& operator>>(ifstream& file, ImageSequence& is);
public:
  /// Default constructor.
  ImageSequence();

  /// Copy constructor.
  ImageSequence(const ImageSequence& is);

  /// Constructor for preallocated images.
  ImageSequence(const int num_images, Image** images);

  /// Overloaded assignment operator.
  const ImageSequence& operator=(const ImageSequence& Rhs);

  /**
   *  @param num_images Number of images.
   *  @param width Width of images in pixels.
   *  @param height Height of images in pixels.
   *  @param bits_per_pixel Gray scale bit depth of images (out of 16).
   *  @param file_names Array of char array corresponding to one file
   *  name per image.  Zero corresponds to sequence of black images.
   *  @memo Constructor.
   */
  ImageSequence(const int num_images, const int width, const int height,
		const int bits_per_pixel,
		const char** const file_names = 0);

  /// Destructor.
  virtual ~ImageSequence();

  /// Load all images in sequence from disk to memory.
  virtual void force_load();

  /// Purge all image pixel data in sequence from memory.
  virtual void force_unload();

  /// Load images around #i# from disk to memory.
  virtual void precache_around_image(const int i);

  /// Returns number of images in sequence.
  const int num_images() const;

  /// Returns reference to #i#th image.
  Image& image(const int i);

  /// Returns constant reference to #i#th image.
  const Image& image_const(const int i) const;

  ///Returns the instance number of the i'th image.
  const int instance_number(const int i) const;

  /// Returns image width.
  const int width() const;

  /// Returns image height.
  const int height() const;

  /// Returns image bits stored per pixel.
  const int bits_per_pixel() const;

  /// Returns short description of sequence (probably override in derived).
  virtual const char* const short_desc() const;

  /// Returns long description of sequence (probably override in derived).
  virtual const char* const long_desc() const;

protected:
  /// Number of images in sequence.
  int _num_images;

  /// Array of Image pointers to the images.
  Image** _images;
};

inline
const int ImageSequence::width() const
{
  return _images[0]->width();
};

inline
const int ImageSequence::height() const
{
  return _images[0]->height();
};

inline
const int ImageSequence::bits_per_pixel() const
{
  return _images[0]->bits_per_pixel();
};


#endif /* !__ImageSequence_h_ */
