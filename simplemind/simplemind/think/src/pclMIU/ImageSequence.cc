#include "ImageSequence.h"
#include <iostream>

extern "C" {
#include "assert.h"  
}

using std::endl;
using std::ifstream;
using std::ofstream;

ImageSequence::ImageSequence() 
  : _num_images(0), _images(0)
{};

ImageSequence::ImageSequence(const int num_images, 
			     const int width, const int height,
			     const int bits_per_pixel, 
			     const char** const file_names)
{
  _num_images = num_images;
  _images = new Image*[num_images];
  int i;
  for (i = 0; i < _num_images; i++) {
    if (file_names) {
      _images[i] = new Image(i, width, height, bits_per_pixel, 
			     file_names[i]); 
    }
    else {
      _images[i] = new Image(i, width, height, bits_per_pixel); 
    };
  };
};

ImageSequence::ImageSequence(const int num_images,
			     Image** images)
{
  _num_images = num_images;
  _images = images;
};

ImageSequence::ImageSequence(const ImageSequence& is) 
  : _num_images(0), _images(0)
{
  *this = is;
}

ImageSequence::~ImageSequence()
{
  int i;
  for (i = 0; i < _num_images; i++) {
    delete _images[i];
  };
  delete [] _images;
};

void ImageSequence::force_load()
{
  int i;
  for (i = 0; i < _num_images; i++) {
    _images[i]->force_load();
  };
};

void ImageSequence::force_unload()
{
  /*
  int i;
  for (i = 0; i < _num_images; i++) {
    _images[i]->unload();
  };
  */
};

void ImageSequence::precache_around_image(const int i)
{
  const int border = 2;
  if (_images[i]->pixels_in_memory()) {
    int j; 
    for (j = -border; j <= border; j++) {
      int candidate = i+j;
      if (candidate > 0 && candidate < _num_images) {
	_images[candidate]->force_load();
      }; 
    };
  };
};

const int ImageSequence::num_images() const
{
  return _num_images;
};

Image& ImageSequence::image(const int i)
{
  return *_images[i];
};

const Image& ImageSequence::image_const(const int i) const
{
  return *_images[i];
};

const int ImageSequence::instance_number(const int i) const
{
  return _images[i]->instance_number();
};

const char* const ImageSequence::short_desc() const
{
  return 0;
};

const char* const ImageSequence::long_desc() const
{
  return 0;
};

ofstream& operator<<(ofstream& file, const ImageSequence& is) {
  assert(file != 0);
  file << is._num_images << endl;
  for(int i = 0; i < is._num_images; i++) {
    file << *(is._images[i]);
  }
  return file;
}

ifstream& operator>>(ifstream& file, ImageSequence& is) {
  assert(file != 0);
   
  //Get rid of old image array
  for(int i = 0; i < is._num_images; i++) {
    delete is._images[i];
  }
  delete [] is._images;
  
  //Read in new number of images, and allocate new image array
  file >> is._num_images;
  is._images = new Image* [is._num_images];
  assert(is._images != 0);
 
  //Allocate and initialize new images
  for(int i = 0; i < is._num_images; i++) {
    //Fill new Image w/dummy values, then read them in from file
    is._images[i] = new Image(i, 0, 0, 0);
    assert(is._images[i] != 0);
    file >> *(is._images[i]);
  }
  return file;
}

const ImageSequence& ImageSequence::operator=(const ImageSequence& Rhs) {
  if(&Rhs != this) {
    //Get rid of old image array
    int i;
    for(i = 0; i < _num_images; i++) {
      delete _images[i];
    }
    delete [] _images;

    //Allocate new image array
    _num_images = Rhs._num_images;
    _images = new Image*[_num_images];
    assert(_images != 0);
    for(i = 0; i < _num_images; i++) {
      _images[i] = new Image(*(Rhs._images[i]));
      assert(_images[i] != 0);
    }
  }
  return *this;
}
