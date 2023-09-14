#include "ImagePrimitive.h"

using std::cerr;
using std::endl;

ImagePrimitive::ImagePrimitive()
//	:  _formed_from(5)
{
}


ImagePrimitive::ImagePrimitive(const ImagePrimitive& i)
//	: _formed_from(i._formed_from)
{
}


ImagePrimitive::~ImagePrimitive()
{
}


ImagePrimitive* ImagePrimitive::create_matched_prim(Darray<ImagePrimitive*>& prims, const MedicalImageSequence& mis) const
{
	ImagePrimitive* p = _combine(prims, mis);
	//ImagePrimitive* ptr_this = this;
	//p->_formed_from.push_last(ptr_this);
	int i;
	for(i=0; i<prims.N(); i++) {
		if (strcmp(prims[i]->type(), type())!=0) {
			cerr << "ERROR: ImagePrimitive: create_matched_prim: all supplied primitives must be of the same type" << endl;
			exit(1);
		}
		//p->_formed_from.push_last(prims[i]);
	}
	/*
	ImagePrimitive* p_lmc = p->_low_memory_copy();
	delete p;

	return p_lmc;
	*/

	return p;
}
