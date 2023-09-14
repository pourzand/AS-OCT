#include "ImageRegion.h"

float z_spacing_for_vol(const MedicalImageSequence& mis, const int z)
{
	float spacing;
	if (mis.zdim()<=1)
		spacing = 0.0;
	else if (z==0)
		spacing = mis.slice_location(z+1) - mis.slice_location(z);
	else if (z==(mis.zdim()-1))
		spacing = mis.slice_location(z) - mis.slice_location(z-1);
	else
		spacing = (mis.slice_location(z+1) - mis.slice_location(z-1))/2;

	return fabs(spacing);
}

ImageRegion::ImageRegion(const ROI& roi, const MedicalImageSequence& mis)
	: ImagePrimitive(), _roi(roi)
{
	ROItraverser rt(_roi);
	register Point rtp1, rtp2;
	register int n=0, ni, planar_n;

	if (!roi.last_point(rtp1)) {
		cerr << "ERROR: ImageRegion: Attempt to create a region containing no points" << endl;
		exit(1);
	}

	_max_z = rtp1.z;
	_planar_centroids = new FPoint* [_max_z+1];
	for(int i=0; i<=_max_z; i++)
		_planar_centroids[i] = 0;
	
	_centroid.x = _centroid.y = _centroid.z = 0;
	_volume = 0;
	rt.current_point(rtp1);
	register float pix_area = mis.row_pixel_spacing(rtp1.z)*mis.column_pixel_spacing(rtp1.z);
	register float vox_vol = mis.row_pixel_spacing(rtp1.z)*mis.column_pixel_spacing(rtp1.z)*z_spacing_for_vol(mis, rtp1.z);
	_planar_centroids[rtp1.z] = new FPoint (0, 0, rtp1.z);
	planar_n = 0;
	TravStatus s;
	_area_xy = 0;
	while(rt.current_interval(rtp1, rtp2)==ROI_STAT_OK) {
		ni = rtp2.x-rtp1.x+1;
		_planar_centroids[rtp1.z]->x += (ni*(rtp1.x+rtp2.x)/2);
		_planar_centroids[rtp1.z]->y += (ni*rtp1.y);
		planar_n += ni;

		s = rt.next_interval();
		if (s >= NEW_PLANE) {
			_centroid.x += _planar_centroids[rtp1.z]->x;
			_centroid.y += _planar_centroids[rtp1.z]->y;
			_centroid.z += (planar_n*_planar_centroids[rtp1.z]->z);
			_area_xy += (planar_n*pix_area);
			_volume += (planar_n*vox_vol);
			n += planar_n;

			_planar_centroids[rtp1.z]->x = _planar_centroids[rtp1.z]->x/(float)planar_n;
			_planar_centroids[rtp1.z]->y = _planar_centroids[rtp1.z]->y/(float)planar_n;

			planar_n = 0;
		}

		if (s == NEW_PLANE) {
			rt.current_point(rtp1);
			_planar_centroids[rtp1.z] = new FPoint (0, 0, rtp1.z);
			pix_area = mis.row_pixel_spacing(rtp1.z)*mis.column_pixel_spacing(rtp1.z);
//cout << "pix_area=" << pix_area << endl;
			vox_vol = mis.row_pixel_spacing(rtp1.z)*mis.column_pixel_spacing(rtp1.z)*z_spacing_for_vol(mis, rtp1.z);
//cout << "vox_vol=" << vox_vol << endl;
		}
	}

	_centroid.x = (float)_centroid.x/(float)n;
	_centroid.y = (float)_centroid.y/(float)n;
	_centroid.z = (float)_centroid.z/(float)n;
}


ImageRegion::ImageRegion(const ImageRegion& i)
	: ImagePrimitive(),
	_roi(i._roi),
	_centroid(i._centroid),
	_max_z(i._max_z),
	_area_xy(i._area_xy),
	_volume(i._volume)
{
	_planar_centroids = new FPoint* [_max_z+1];
	for(int j=0; j<=_max_z; j++) {
		if (i._planar_centroids[j])
			_planar_centroids[j] = new FPoint (*(i._planar_centroids[j]));
		else
			_planar_centroids[j] = 0;
	}
}

ImageRegion::~ImageRegion()
{
	for(int j=0; j<=_max_z; j++)
		if (_planar_centroids[j])
			delete _planar_centroids[j];
	delete [] _planar_centroids;
}


ImagePrimitive* ImageRegion::create_copy() const
{
	return new ImageRegion (*this);
}	


ImagePrimitive* ImageRegion::create_subsampled_prim(const MedicalImageSequence& orig_image, const MedicalImageSequence& subsampled_image) const
{
	ROI subsamp_roi;

	subsample_roi(_roi, subsamp_roi, orig_image.xdim()/subsampled_image.xdim(), orig_image.ydim()/subsampled_image.ydim(), orig_image.zdim()/subsampled_image.zdim());

	ImageRegion* ir = new ImageRegion (subsamp_roi, subsampled_image);
	return ir;
}	


const FPoint* ImageRegion::centroid(const int z) const
{
	if ((z<0) || (z>_max_z))
		return 0;
	else
		return _planar_centroids[z];
}


//void ImageRegion::translate_to_inst_nums(const int offset) {
void ImageRegion::translate_z(const int offset) {
	_roi.translate(0,0,offset);
	_centroid.z += offset;

	int nmz = _max_z + offset;
	FPoint** npc = new FPoint* [nmz+1];
	for(int i=0; i<=nmz; i++)
		npc[i] = 0;

	for(int i=0; i<=_max_z; i++) {
	  if (_planar_centroids[i]!=0) {
		_planar_centroids[i]->z += offset;
		npc[i+offset] = _planar_centroids[i];
	   }
	}
	_max_z += offset;
	delete [] _planar_centroids;
	_planar_centroids = npc;
}


void ImageRegion::map_to_inst_nums(const int* im_inst_nums) {
	ROI rt;

	Point fp;
	if (_roi.first_point(fp)) {
		Point lp;
		_roi.last_point(lp);

		//cout << "map_to_inst_nums0" << endl;
		//cout << "fp.z="+fp.z << endl;
		//cout << "im_inst_nums[z]="+im_inst_nums[fp.z] << endl;

		int nmz=-1;
		for(int z=fp.z; z<=lp.z; z++) {
			ROI rz(_roi, z);
			if (!rz.empty()) {
				rz.translate(0,0,(im_inst_nums[z]-z));
				if (im_inst_nums[z]>nmz) nmz=im_inst_nums[z];
			}
			rt.OR(rz);
		}
	_roi.clear();
	_roi.OR(rt);

	_centroid.z = im_inst_nums[(int)(_centroid.z+0.5)];

	// ************
	//int nmz = im_inst_nums[(int)(_planar_centroids[_max_z]->z+0.5)];
	FPoint** npc = new FPoint* [nmz+1];
	for(int i=0; i<=nmz; i++)
		npc[i] = 0;
	//cout << "nmz=" << nmz << endl;
	
	/************/
	//cout << "_max_z=" << _max_z << endl;
	for(int i=0; i<=_max_z; i++)
	    if (_planar_centroids[i]) {
		//cout << i << ": " << _planar_centroids[i]->z << endl;
		_planar_centroids[i]->z = im_inst_nums[(int)(_planar_centroids[i]->z)];
		npc[(int)(_planar_centroids[i]->z)] = _planar_centroids[i];
		//cout << i << ": " << _planar_centroids[i]->z << endl;
	    }
		//Point lp;

	_roi.last_point(lp);
	_max_z = lp.z;
	delete [] _planar_centroids;
	_planar_centroids = npc;
	}
}


/*
void ImageRegion::map_to_inst_nums(const int* im_inst_nums) {
	ROI rt;

	Point fp;
	if (_roi.first_point(fp)) {
		Point lp;
		_roi.last_point(lp);

		cout << "fp.z="+fp.z << endl;
		cout << "im_inst_nums[z]="+im_inst_nums[fp.z] << endl;

		for(int z=fp.z; z<=lp.z; z++) {
			ROI rz(_roi, z);
			if (!rz.empty()) {
				rz.translate(0,0,(im_inst_nums[z]-z));
			}
			rt.OR(rz);
		}
	}
	_roi.clear();
	_roi.OR(rt);

	cout << "map_to_inst_nums1" << endl;

	_centroid.z = im_inst_nums[(int)(_centroid.z+0.5)];

	int nmz = im_inst_nums[(int)(_planar_centroids[_max_z]->z+0.5)];
	FPoint** npc = new FPoint* [nmz+1];
	for(int i=0; i<=nmz; i++)
		npc[i] = 0;
	cout << "map_to_inst_nums2" << endl;
	
	for(int i=0; i<=_max_z; i++)
	    if (_planar_centroids[i]) {
		_planar_centroids[i]->z = im_inst_nums[(int)(_planar_centroids[i]->z)];
		npc[(int)(_planar_centroids[i]->z)] = _planar_centroids[i];
	    }
	cout << "map_to_inst_nums3" << endl;
		Point lp;
	_roi.last_point(lp);
	_max_z = lp.z;
	delete [] _planar_centroids;
	_planar_centroids = npc;
	cout << "map_to_inst_nums4" << endl;
}
*/

void ImageRegion::write(ostream& s) const 
{
	s << type() << endl;

	s << _roi;

	s << _centroid /*<< " " << _max_z*/ << endl;
	Point fp;
	_roi.first_point(fp);
	for(int z=fp.z; z<=_max_z; z++) {
		if (_planar_centroids[z])
			s << *(_planar_centroids[z]) << " ";
		//else
		//	s << 0 << " ";
	}
	s << endl;
	s << _area_xy << " " << _volume << endl;
}

void ImageRegion::writeEssentialOnly(ostream& s) const
{
	s << _roi;
}


ImagePrimitive* ImageRegion::_combine(Darray<ImagePrimitive*>& prims, const MedicalImageSequence& mis) const
{
	ImageRegion* new_p;

	if (prims.N()==0)
		new_p = new ImageRegion (*this);
	else {
		ROI r(_roi);
		int i;
		for(i=0; i<prims.N(); i++) {
			if (strcmp(prims[i]->type(), "ImageRegion") != 0) {
				cerr << "ERROR: ImageRegion: combine: argument is not an ImageRegion" << endl;
				exit(1);
			}
			r.OR(((ImageRegion*)prims[i])->roi());
		}
		new_p = new ImageRegion (r, mis);
	}
	return new_p;
}

/*
ImagePrimitive* ImageRegion::_low_memory_copy() const
{
	ImageRegion* lmir = new ImageRegion;

	ROI* lmr = low_memory_copy(_roi);
	lmir->_roi.copy(*lmr);
	delete lmr;
	
	lmir->_centroid = _centroid;
	lmir->_max_z = _max_z;

	lmir->_planar_centroids = new FPoint* [lmir._max_z];
	int i;
	for(i=0; i<lmir._max_z; i++)
		if (!_planar_centroids[i])
			lmir->_planar_centroids[i] = 0;
		else {
			lmir->_planar_centroids[i] = new FPoint (*(_planar_centroids[i]));
		}

	lmir->_area_xy = _area_xy;
	lmir->_volume = _volume;

	return lmir;
}
*/
