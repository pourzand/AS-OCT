#include "ImageContour.h"

using std::cerr;
using std::endl;

ImageContour::ImageContour(const Contour& cont)
	: ImagePrimitive(), _cont(cont)
{
}

ImageContour::ImageContour(const ImageContour& i)
	: _cont(i._cont)
{
}

ImageContour::~ImageContour()
{
}


ImagePrimitive* ImageContour::create_copy() const
{
	return new ImageContour (*this);
}		


const Contour& ImageContour::contour() const
{
	return _cont;
}

ImagePrimitive* ImageContour::create_subsampled_prim(const MedicalImageSequence& orig_image, const MedicalImageSequence& subsampled_image) const
{
	cerr << "ERROR: ImageContour: create_subsampled_prim: method not implemented" << endl;
	exit(1);
	return 0;
}

void ImageContour::translate_z(const int offset) {
}

void ImageContour::map_to_inst_nums(const int* im_inst_nums) {
}

void ImageContour::write(ostream& s) const
{
}



ImagePrimitive* ImageContour::_combine(Darray<ImagePrimitive*>&, const MedicalImageSequence&) const
{
	cerr << "ERROR: ImageContour: _combine: method not implemented" << endl;
	exit(1);
	return 0;
}


/*
ImagePrimitive* ImageContour::_low_memory_copy() const
{
	cerr << "ERROR: ImageContour: _low_memory_copy: method not implemented" << endl;
	exit(1);
	return 0;
}
*/
