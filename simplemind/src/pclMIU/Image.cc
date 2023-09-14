extern "C" 
{
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
//#include <sys/mman.h>
#include <sys/stat.h>
//#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
}
#include <iostream>
#include <ostream>

#include "Image.h"
//#include "../config.h"

using std::endl;

Image::Image(const int instance_number, const int width, const int height, 
	     const int bits_per_pixel, 
	     const char* const file_name,
	     const int file_offset,
	     const float rescale_slope, 
	     const float rescale_intercept)
{ 
  assert(width >= 0);
  assert(height >= 0);
  assert(bits_per_pixel >= 0 && bits_per_pixel <= 16);

  _instance_number = instance_number;
  _width = width; 
  _height = height; 
  _rs = rescale_slope;
  _ri = rescale_intercept;
  _bits_per_pixel = bits_per_pixel; 
  _pixel_data = 0;
  if (file_name) {
    _pixel_filename = new char[strlen(file_name) + 1];
    assert(file_name != 0);
    strcpy(_pixel_filename, file_name);
    //    _pixel_filename = strdup(file_name);
  }
  else {
    _pixel_filename = 0;
  };
  _file_offset = file_offset;
};

Image::Image(const int instance_number, const int width, const int height, 
	     const int bits_per_pixel, 
	     short* pixels,
	     const float rescale_slope, 
	     const float rescale_intercept)
{
  assert(width >= 0);
  assert(height >= 0);
  assert(bits_per_pixel >= 0 && bits_per_pixel <= 16);

  _instance_number = instance_number;
  _width = width; 
  _height = height; 
  _bits_per_pixel = bits_per_pixel; 
  _rs = rescale_slope;
  _ri = rescale_intercept;
  const int num_pixels = _width*_height;
  _pixel_data = new short[num_pixels];
  int i;
  for (i = 0; i < num_pixels; i++) {
    _pixel_data[i] = pixels[i];
  };
  _file_offset = 0;
  _pixel_filename = 0;
};

Image::~Image() 
{
  delete [] _pixel_data;
  delete [] _pixel_filename;
};

void Image::force_load()
{
  pixel_data();
};

void Image::unload()
{
  /*
  if (_pixel_filename) {
    delete [] _pixel_data;
    _pixel_data = 0;
  };
  */
};

const int Image::bits_per_pixel() const 
{
  return _bits_per_pixel;
};

const short Image::pixel(const int x, const int y)
{
  assert(x >= 0 && x < _width);
  assert(y >= 0 && x < _height);
  return pixel_data()[x + y*_width];
};

short*& Image::pixel_data() 
{
  if (!_pixel_data) {
    _load_image_from_file();
    //    _map_image_from_file();
  }
  return _pixel_data;
};

const bool Image::pixels_in_memory() const 
{ 
  if (_pixel_data) {
    return true;
  }
  else {
    return false;
  };
};
  

Image::Image() 
{ 
  assert(0);
};


//Changed by BW to allow copy constructor use
Image::Image(const Image& im)
  : _pixel_filename(NULL), _pixel_data(NULL)
{ 
  *this = im;
}
  
//Changed by BW to allow usage
const Image& Image::operator=(const Image& Rhs) 
{
  if(&Rhs != this) {  //check self-assignment
    _width = Rhs._width;
    _height = Rhs._height;
    _bits_per_pixel = Rhs._bits_per_pixel;
    _file_offset = Rhs._file_offset;

    if (Rhs._pixel_filename) {
      delete [] _pixel_filename;
      _pixel_filename = new char[strlen(Rhs._pixel_filename) + 1];
      assert(_pixel_filename != 0);
      strcpy(_pixel_filename, Rhs._pixel_filename);
    }   
    else {
      _pixel_filename = 0;
      const int num_pixels = _width*_height;
      _pixel_data = new short[num_pixels];
      int i;
      for (i = 0; i < num_pixels; i++) {
	_pixel_data[i] = Rhs._pixel_data[i];
      };
      _file_offset = 0;      
    };
  };
  return *this;
};

void Image::_load_image_from_file()
{
  if (!_pixel_data) { // Only do work on first call.
    // Allocate memory.
    int image_size = _width*_height;
    _pixel_data = new short[image_size];

    int bytes_per_pixel = 2;
    if (_bits_per_pixel <= 8) {
      bytes_per_pixel = 1;
    };
    memset(_pixel_data, 0, image_size*bytes_per_pixel);

    // Fill memory.
    if (_pixel_filename) {  // Load image if we were passed a name.
      // buffered i/o
      FILE* f = fopen(_pixel_filename, "rb");
      fseek(f, _file_offset, SEEK_SET);
      int ret = fread((void*)_pixel_data, image_size*bytes_per_pixel, 1, f);
      ret++;
#ifdef WORDS_BIGENDIAN
      int i;
      for(i = 0; i < image_size; i++) {
        short tmp = _pixel_data[i]&0x00FF;
        tmp = ((_pixel_data[i]&0xFF00) >> 8) | (tmp << 8);
        _pixel_data[i] = tmp;
      }
#endif 

      // hack hack hack to pad eight bit data out to 16!!!!
      if (_bits_per_pixel <= 8) {
	int i;
	for (i = image_size-1; i >= 0; i--) {
	  ((unsigned char*)_pixel_data)[i*2+1] 
	    = ((unsigned char*)_pixel_data)[i];
	  ((unsigned char*)_pixel_data)[i*2] = (unsigned char)0;
	};
      };
      fclose(f);

    };
  };
};
/*
void Image::_map_image_from_file()
{
  if (!_pixel_data) { // Only do work on first call.
    // Allocate memory.
    int image_size = _width*_height;
    _pixel_data = new short[image_size];
    // Fill memory.
    if (_pixel_filename) {  // Load image if we were passed a name.
      int f = open(_pixel_filename, O_RDONLY);
      //char* xxx = mmap((caddr_t)0, image_size*2, 
      void* xxx = mmap((caddr_t)0, image_size*2, 
			 PROT_READ, MAP_SHARED, f, 
			 _file_offset);
      // hack hack hack should munmap this in destructor.
      close(f);

    }
    else { // If no file name, make an image of zeroes
      for (int i = 0; i < image_size; i++) {
	_pixel_data[i] = 0;
      };
    };
  };
};
*/

//Added by BW for dmtk checkpoints
ofstream& operator<<(ofstream& file, const Image& image) {
  assert(file != 0);
  file << image._width << endl;
  file << image._height << endl;
  file << image._bits_per_pixel << endl;
  file << strlen(image._pixel_filename) << endl 
       << image._pixel_filename << endl;
  file << image._file_offset << endl;
  return file;
}

//Added by BW for dmtk checkpoints
ifstream& operator>>(ifstream& file, Image& image) {
  assert(file != 0);
  int length;
  
  file >> image._width;
  file >> image._height;
  file >> image._bits_per_pixel;
  
  //Read in pixel filename
  file >> length; 
  file.get();  // Get rid of newline
  delete [] image._pixel_filename;
  image._pixel_filename = new char[length + 1];
  assert(image._pixel_filename != 0);
  file.getline(image._pixel_filename, length + 1);
  
  file >> image._file_offset;
  return file;
}

