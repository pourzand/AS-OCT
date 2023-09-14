#include "SearchArea.h"

// These functions are defined in roitk
int conv_hull_direct(const Point& v1, const Point& v2, const Point& v3);
int conv_hull_input(Contour& c, Point& v);

SearchArea::SearchArea(const int e_flag)
	: Attribute(), _or_flag(0)
{
	_e_flag = e_flag;
}

SearchArea::SearchArea(const SearchArea& sa)
	: Attribute(sa), _or_flag(sa._or_flag)
{
}

SearchArea::~SearchArea()
{
}


void SearchArea::write(ostream& s) const
{
	_write_start_attribute(s);
	_write_end_attribute(s);
}


BetweenX::BetweenX(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BetweenX::~BetweenX()
{
}

const int BetweenX::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	ROItraverser rt;
	if ((ss_factor.x*ss_factor.y*ss_factor.z) != 1) {
		ROI ss_roi;
		subsample_roi(r0->roi(), ss_roi, ss_factor.x, ss_factor.y, ss_factor.z);
		rt.reinitialize(ss_roi);
	}
	else
		rt.reinitialize(r0->roi());

	ROI between;
	register TravStatus s=rt.valid();
	//register int first=1;
	register Point p1, p2, p2_prev(-1,-1,-1);
	if (s==ROI_STAT_OK)
		for(s=NEW_LINE; s<END_ROI; s=rt.next_interval()) {
			rt.current_interval(p1, p2);
			if ((p2.y==p2_prev.y) && (p2.z==p2_prev.z))
				between.append_interval(p2_prev.x+1, p1.x-1, p1.y, p1.z);
			p2_prev = p2;
		}

	roi.AND(between);
  }
  return done;
}


BoxRelCentroid::BoxRelCentroid(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset, const float z_offset,
const float x_length, const float y_length, const float z_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset), _z_offset(z_offset),
	_x_length(x_length), _y_length(y_length), _z_length(z_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelCentroid::~BoxRelCentroid()
{
}


const int BoxRelCentroid::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	Point cent(round_float(r0->centroid().x), round_float(r0->centroid().y), round_float(r0->centroid().z));
	Point box_cent = offset_mm(cent, _x_offset, _y_offset, _z_offset, mis);
	Point tl=box_cent, br=box_cent;
	
	if (_x_length>0) {
		tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 		br = offset_mm(br, _x_length/2, 0, 0, mis);
	}
	else if (_x_length<0) {
		tl.x = 0;
		br.x = mis.xdim()-1;
	}
	if (_y_length>0) {
		tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 		br = offset_mm(br, 0, _y_length/2, 0, mis);
	}
	else if (_y_length<0) {
		tl.y = 0;
		br.y = mis.ydim()-1;
	}
	if (_z_length>0) {
		tl = offset_mm(tl, 0, 0, -_z_length/2, mis);
 		br = offset_mm(br, 0, 0, _z_length/2, mis);
	}
	else if (_z_length<0) {
		tl.z = 0;
		br.z = mis.zdim()-1;
	}

	ROI box;
	if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
		if (tl.x<0) tl.x=0;
		if (tl.y<0) tl.y=0;
		if (tl.z<0) tl.z=0;
		if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
		if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
		if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

		tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
		br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

		box.add_box(tl, br);
	}
	if (or_flag())
		roi.OR(box);
	else {
		std::cout << "Here roi.AND(box)" << std::endl << std::flush;
		/****/
		roi.AND(box);
		/*****/
	}
 }
 return done;
}



BoxRelPlanarCentroid::BoxRelPlanarCentroid(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset,
const float x_length, const float y_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset),
	_x_length(x_length), _y_length(y_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelPlanarCentroid::~BoxRelPlanarCentroid()
{
}


const int BoxRelPlanarCentroid::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI box;

	Point fp, lp;
	r0->roi().first_point(fp);
	r0->roi().last_point(lp);

	for(int z=(fp.z/ss_factor.z)*ss_factor.z; z<=(lp.z/ss_factor.z)*ss_factor.z; z+=ss_factor.z) {
		const FPoint*  planar_cent = r0->centroid(z);
		if (planar_cent) {
			Point cent(round_float(planar_cent->x), round_float(planar_cent->y), z);
			Point box_cent = offset_mm(cent, _x_offset, _y_offset, 0, mis);
			Point tl=box_cent, br=box_cent;
			if (_x_length>0) {
				tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 				br = offset_mm(br, _x_length/2, 0, 0, mis);
			}
			else if (_x_length<0) {
				tl.x = 0;
				br.x = mis.xdim()-1;
			}
			if (_y_length>0) {
				tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 				br = offset_mm(br, 0, _y_length/2, 0, mis);
			}
			else if (_y_length<0) {
				tl.y = 0;
				br.y = mis.ydim()-1;
			}

			if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
				if (tl.x<0) tl.x=0;
				if (tl.y<0) tl.y=0;
				if (tl.z<0) tl.z=0;
				if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
				if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
				if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

				tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
				br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

				box.add_box(tl, br);
			}
		}
	}

	if (or_flag())
		roi.OR(box);
	else
		roi.AND(box);
 }
 return done;
}



BoxRelPlanarYmin::BoxRelPlanarYmin(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset,
const float x_length, const float y_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset),
	_x_length(x_length), _y_length(y_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelPlanarYmin::~BoxRelPlanarYmin()
{
}


const int BoxRelPlanarYmin::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI box;

	ROItraverser rt(r0->roi());
	Point p1, p2;
	TravStatus s = rt.current_interval(p1, p2);

	while (s<NOT_FOUND) {
	    if ((p1.z%ss_factor.z)==0) {
		Point pm((p1.x+p2.x)/2, p1.y, p1.z);
		Point box_cent = offset_mm(pm, _x_offset, _y_offset, 0, mis);
		Point tl=box_cent, br=box_cent;
		if (_x_length>0) {
			tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 			br = offset_mm(br, _x_length/2, 0, 0, mis);
		}
		else if (_x_length<0) {
			tl.x = 0;
			br.x = mis.xdim()-1;
		}
		if (_y_length>0) {
			tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 			br = offset_mm(br, 0, _y_length/2, 0, mis);
		}
		else if (_y_length<0) {
			tl.y = 0;
			br.y = mis.ydim()-1;
		}

		if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
			if (tl.x<0) tl.x=0;
			if (tl.y<0) tl.y=0;
			if (tl.z<0) tl.z=0;
			if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
			if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
			if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

			tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
			br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

			box.add_box(tl, br);
		}
	    }
	    s = rt.next_plane();
	    if (s<NOT_FOUND)
		rt.current_interval(p1, p2);
	}
	if (or_flag())
		roi.OR(box);
	else
		roi.AND(box);
 }
 return done;
}



BoxRelPlanarYmax::BoxRelPlanarYmax(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset,
const float x_length, const float y_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset),
	_x_length(x_length), _y_length(y_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelPlanarYmax::~BoxRelPlanarYmax()
{
}


const int BoxRelPlanarYmax::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	
	ROI box;

	//ROItraverser rt(roi);
	ROItraverser rt(r0->roi());
	Point p1, p2;
	TravStatus s = rt.last_point_in_plane();
	rt.current_interval(p1, p2);

	while (s<NOT_FOUND) {
	    if ((p1.z%ss_factor.z)==0) {
		//rt.current_interval(p1, p2);
		Point pm((p1.x+p2.x)/2, p1.y, p1.z);
		Point box_cent = offset_mm(pm, _x_offset, _y_offset, 0, mis);
		Point tl=box_cent, br=box_cent;
		if (_x_length>0) {
			tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 			br = offset_mm(br, _x_length/2, 0, 0, mis);
		}
		else if (_x_length<0) {
			tl.x = 0;
			br.x = mis.xdim()-1;
		}
		if (_y_length>0) {
			tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 			br = offset_mm(br, 0, _y_length/2, 0, mis);
		}
		else if (_y_length<0) {
			tl.y = 0;
			br.y = mis.ydim()-1;
		}

		if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
			if (tl.x<0) tl.x=0;
			if (tl.y<0) tl.y=0;
			if (tl.z<0) tl.z=0;
			if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
			if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
			if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

			tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
			br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

			box.add_box(tl, br);

			//cout << "BoxRelPlanarYmax: " << pm << tl << br << endl;
		}
	    }
	    s = rt.next_plane();
	    if (s<NOT_FOUND) {
		rt.last_point_in_plane();
		rt.current_interval(p1, p2);
	    }
	}
	roi.AND(box);
 }
 return done;
}




BoxRelZmin::BoxRelZmin(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset, const float z_offset,
const float x_length, const float y_length, const float z_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset), _z_offset(z_offset),
	_x_length(x_length), _y_length(y_length), _z_length(z_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelZmin::~BoxRelZmin()
{
}


const int BoxRelZmin::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI box;

	Point fp;
	if (!r0->roi().first_point(fp)) {
		cerr << "ERROR: SearchArea: BoxRelZmin: can't find first point of related region" << endl;
		exit(1);

	}
	const FPoint* zmin = r0->centroid(fp.z);
	if (!zmin) {
		cerr << "ERROR: SearchArea: BoxRelZmin: can't find z-minimum-point" << endl;
		exit(1);

	}

	Point pm;
	fpoint_to_point(*zmin, pm);
	Point box_cent = offset_mm(pm, _x_offset, _y_offset, _z_offset, mis);
	Point tl=box_cent, br=box_cent;
	if (_x_length>0) {
		tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 		br = offset_mm(br, _x_length/2, 0, 0, mis);
	}
	else if (_x_length<0) {
		tl.x = 0;
		br.x = mis.xdim()-1;
	}
	if (_y_length>0) {
		tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 		br = offset_mm(br, 0, _y_length/2, 0, mis);
	}
	else if (_y_length<0) {
		tl.y = 0;
		br.y = mis.ydim()-1;
	}
	if (_z_length>0) {
		tl = offset_mm(tl, 0, 0, -_z_length/2, mis);
 		br = offset_mm(br, 0, 0, _z_length/2, mis);
	}
	else if (_z_length<0) {
		tl.z = 0;
		br.z = mis.zdim()-1;
	}

	if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
		if (tl.x<0) tl.x=0;
		if (tl.y<0) tl.y=0;
		if (tl.z<0) tl.z=0;
		if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
		if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
		if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

		tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
		br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

		box.add_box(tl, br);
	}

	if (or_flag())
		roi.OR(box);
	else
		roi.AND(box);
 }
 return done;
}




BoxRelZmax::BoxRelZmax(const std::string& rel_solel_name, const int rel_solel_ind,
const float x_offset, const float y_offset, const float z_offset,
const float x_length, const float y_length, const float z_length,
const int e_flag)
	:SearchArea(e_flag),
	_x_offset(x_offset), _y_offset(y_offset), _z_offset(z_offset),
	_x_length(x_length), _y_length(y_length), _z_length(z_length)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


BoxRelZmax::~BoxRelZmax()
{
}


const int BoxRelZmax::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI box;

	Point lp;
	if (!r0->roi().last_point(lp)) {
		cerr << "ERROR: SearchArea: BoxRelZmax: can't find last point of related region" << endl;
		exit(1);

	}
	const FPoint* zmax = r0->centroid(lp.z);
	if (!zmax) {
		cerr << "ERROR: SearchArea: BoxRelZmax: can't find z-maximum-point" << endl;
		exit(1);

	}

	Point pm;
	fpoint_to_point(*zmax, pm);
	Point box_cent = offset_mm(pm, _x_offset, _y_offset, _z_offset, mis);
	Point tl=box_cent, br=box_cent;
	if (_x_length>0) {
		tl = offset_mm(tl, -_x_length/2, 0, 0, mis);
 		br = offset_mm(br, _x_length/2, 0, 0, mis);
	}
	else if (_x_length<0) {
		tl.x = 0;
		br.x = mis.xdim()-1;
	}
	if (_y_length>0) {
		tl = offset_mm(tl, 0, -_y_length/2, 0, mis);
 		br = offset_mm(br, 0, _y_length/2, 0, mis);
	}
	else if (_y_length<0) {
		tl.y = 0;
		br.y = mis.ydim()-1;
	}
	if (_z_length>0) {
		tl = offset_mm(tl, 0, 0, -_z_length/2, mis);
 		br = offset_mm(br, 0, 0, _z_length/2, mis);
	}
	else if (_z_length<0) {
		tl.z = 0;
		br.z = mis.zdim()-1;
	}

	if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
		if (tl.x<0) tl.x=0;
		if (tl.y<0) tl.y=0;
		if (tl.z<0) tl.z=0;
		if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
		if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
		if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

		tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
		br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

		box.add_box(tl, br);
	}

	if (or_flag())
		roi.OR(box);
	else
		roi.AND(box);
 }
 return done;
}



BoxSizeTlProp::BoxSizeTlProp(const float x_length, const float y_length, const float z_length,
const float tlx_prop, const float tly_prop, const float tlz_prop)
	:SearchArea(0),
	_x_length(x_length), _y_length(y_length), _z_length(z_length),
	_tlx_prop(tlx_prop), _tly_prop(tly_prop), _tlz_prop(tlz_prop)
{
}


BoxSizeTlProp::~BoxSizeTlProp()
{
}


const int BoxSizeTlProp::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
	//cout << "BoxSizeTlProp " << _x_length << " " << _y_length << " " << _z_length << " " << _tlx_prop << " " << _tly_prop << " " << _tlz_prop << endl;

	float x_imsize = mis.xdim()*mis.column_pixel_spacing(0);
	float y_imsize = mis.ydim()*mis.row_pixel_spacing(0);
	float z_imsize = std::abs(mis.slice_location(0)-mis.slice_location(mis.zdim()-1));

	Point tl(0,0,0), br(mis.xdim()-1, mis.ydim()-1, mis.zdim()-1);
	if ((_x_length>0) && (_x_length<x_imsize) && (_tlx_prop>=0) && (_tlx_prop<=1)) {
		float x_offset_mm = (x_imsize - _x_length)*_tlx_prop;
		tl = offset_mm(tl, x_offset_mm, 0, 0, mis);
		br.x = 0;
		br = offset_mm(br, x_offset_mm+_x_length, 0, 0, mis);
	}
	if ((_y_length>0) && (_y_length<y_imsize) && (_tly_prop>=0) && (_tly_prop<=1)) {
		float y_offset_mm = (y_imsize - _y_length)*_tly_prop;
		tl = offset_mm(tl, 0, y_offset_mm, 0, mis);
		br.y = 0;
		br = offset_mm(br, 0, y_offset_mm+_y_length, 0, mis);
	}
	if ((_z_length>0) && (_z_length<z_imsize) && (_tlz_prop>=0) && (_tlz_prop<=1)) {
		float z_offset_mm = (z_imsize - _z_length)*_tlz_prop;
		tl = offset_mm(tl, 0, 0, z_offset_mm, mis);
		br.z = 0;
		br = offset_mm(br, 0, 0, z_offset_mm+_z_length, mis);
	}

	ROI box;
	if ((tl.x<mis.xdim()) && (tl.y<mis.ydim()) && (tl.z<mis.zdim()) && (br.x>=0) && (br.y>=0) && (br.z>=0)) {
		if (tl.x<0) tl.x=0;
		if (tl.y<0) tl.y=0;
		if (tl.z<0) tl.z=0;
		if (br.x>(mis.xdim()-1)) br.x=mis.xdim()-1;
		if (br.y>(mis.ydim()-1)) br.y=mis.ydim()-1;
		if (br.z>(mis.zdim()-1)) br.z=mis.zdim()-1;

		tl.x = tl.x/ss_factor.x; tl.y = tl.y/ss_factor.y; tl.z = tl.z/ss_factor.z;
		br.x = br.x/ss_factor.x; br.y = br.y/ss_factor.y; br.z = br.z/ss_factor.z;

		box.add_box(tl, br);
	}
	//cout << tl << br << endl;

	if (or_flag())
		roi.OR(box);
	else
		roi.AND(box);
 return 1;
}


Clear::Clear()
	: SearchArea(0)
{
}

Clear::~Clear()
{
}


const int Clear::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 1;
  roi.clear();

  return done;
}


ConvexHull::ConvexHull(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

ConvexHull::~ConvexHull()
{
}

/*
int conv_hull_direct(const Point& v1, const Point& v2, const Point& v3)
{
	register double a1 = atan2(v2.y-v1.y, v2.x-v1.x); 
	register double a2 = atan2(v3.y-v1.y, v3.x-v1.x); 
	register double ad = a2-a1;

	if (ad<0)
		ad = ad + (2*PI_DBL);

	int d=1;
	if (ad==0)
		d = 0;
	else if (ad>PI_DBL)
		d = -1;
	return d;
}



int conv_hull_input(Contour& c, Point& v)
{
	int done = 0;
	if (c.n()>0) {
		v = c[c.n()-1];
		c.remove(c.n()-1);
		done = 1;
	}
	return done;
}
*/

const int ConvexHull::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
    //cout << "chull here1" << endl;
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	//const ROI& r = r0->roi();
	ROI r;
	subsample_roi(r0->roi(), r, ss_factor.x, ss_factor.y, ss_factor.z);

	ROI chull;
	Point p1, p2;
	register int ok, triangle;

	ROI se;
	Point tlse(0, 0), brse(1, 1);
	se.add_box(tlse,brse);
	r.dilate(se);
	//cout << "chull here2" << endl;

	if (r.first_point(p1) && r.last_point(p2))
	    for(int z=p1.z; z<=p2.z; z++) {
	    //cout << "chull here3 " << z << endl;
		int n;
		Contour* bndy = r.boundaries(n, z);
		//cout << "chull here3a " << n << endl;
		for(int i=0; i<n; i++) {
		//cout << "chull here3b " << i << endl;
			int mpi=0;
			int j;
			for(j=0; j<n; j++) {
				if (bndy[j].n()>bndy[mpi].n())
					mpi = j;
			}
			//cout << "chull here3c " << n << endl;

			if (!chull.in_roi(bndy[mpi][0])) {
			    Contour h(bndy[mpi]);

			    if (bndy[mpi].n()>=3) {
				Point v1, v2, v3, v;
				conv_hull_input(bndy[mpi], v1);
				conv_hull_input(bndy[mpi], v2);
				conv_hull_input(bndy[mpi], v3);
				triangle = 1;
				//cout << "chull here3d " << z << endl;
				while(triangle && conv_hull_direct(v1, v2, v3)==0) {
					v2 = v3;
					triangle = conv_hull_input(bndy[mpi], v3);
				}
				//cout << "chull here3e " << z << endl;
				if (triangle) {
					h.clear();
					if (conv_hull_direct(v1, v2, v3)>0) {
						h.append(v1);
						h.append(v2);
					}
					else {
						h.append(v2);
						h.append(v1);
					}
					h.append(v3);
					h.prepend(v3);
					//cout << "chull here3f " << z << endl;
					while(bndy[mpi].n()>0) {
						ok = 0;
						while(!ok && conv_hull_input(bndy[mpi], v)) {
							ok = (conv_hull_direct(v, h[0], h[1])<0) || (conv_hull_direct(h[h.n()-2], h[h.n()-1], v)<0);
						}				
						if (ok) {
							while(conv_hull_direct(h[h.n()-2], h[h.n()-1], v)<=0) {
								h.remove(h.n()-1);
							}
							h.append(v);
					
							while(conv_hull_direct(v, h[0], h[1])<=0)
								h.remove(0);
							h.prepend(v);	
						}	
					}
					//cout << "chull here3g " << z << endl;
					h.reverse();
					//cout << "chull here3h " << z << endl;
				}
			    }
			    //cout << "chull here3i " << z << endl;
			    chull.add_planar_polygon(h);
			    //cout << "chull here3j " << z << endl;
			}
			else
				bndy[mpi].clear();
		//cout << "chull here3k " << z << endl;
		}
		if (n>0)
			delete [] bndy;
	    //cout << "chull here3 " << z << endl;
	    }
	//cout << "chull here4" << endl;
	chull.erode(se);
	roi.AND(chull);
  }
  return done;
}



DistanceMap2D::DistanceMap2D(const std::string& rel_solel_name, const int rel_solel_ind, const float dist_thresh, const int e_flag)
	: SearchArea(e_flag), _dist_thresh(dist_thresh)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

DistanceMap2D::~DistanceMap2D()
{
}


const int DistanceMap2D::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI result;

	Point fp;
	if (!r0->roi().first_point(fp)) {
		cerr << "ERROR: SearchArea: DistanceMap2D: can't find first point of related region" << endl;
		exit(1);

	}
	Point lp;
	r0->roi().last_point(lp);

	// Euclidean Distance Map algorithm from textbook by Russ

	int xdim = mis.xdim();
	float* dm;
	int z, j;

	for (z=fp.z; z<=lp.z; z++)
	  if (!r0->roi().empty(z)) {
		dm = distance_map_2d(r0->roi(), z, mis);

		ROItraverser rt(r0->roi(), z);
		TravStatus s = rt.valid();
		Point p1, p2;
		while(s<END_ROI) {
			rt.current_interval(p1, p2);
			j=p1.y*xdim+p1.x;
			for(; p1.x<=p2.x; ++p1.x) {
				if (dm[j]>=_dist_thresh) {
					result.append_point(p1);
				}
				++j;
			}
			s = rt.next_interval();
		}

		delete [] dm;
	}

	if (or_flag())
		roi.OR(result);
	else
		roi.AND(result);
 }
 return done;
}


/*
DistanceMap25DPercMax::DistanceMap25DPercMax(const std::string& rel_solel_name, const int rel_solel_ind, const float percentage, const int e_flag)
	: SearchArea(e_flag), _perc(percentage)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

DistanceMap25DPercMax::~DistanceMap25DPercMax()
{
}


const int DistanceMap25DPercMax::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI result;
	float** edms=0;

	ROI r(r0->roi());
	Point fp, lp, lpl;
	int i;
//	int prevfpz = 0;
	if (r.last_point(lp)) {
		edms = new float* [lp.z+1];
		for(i=0; i<=lp.z; i++) {
			edms[i]=0;
		}
	}
	int xdim = mis.xdim();
	int j;
	float max=0, thresh;

	while (r.first_point(fp)) {
		ROI lr;
		lr.add_contig_3d(r, fp, 1);

		if (fp.z>=0) {
			//for (i=prevfpz; i<fp.z; i++) {
			//	if (edms[i]!=0) {
			//		delete [] edms[i];
			//		edms[i]=0;
			//	}
			//}
			//prevfpz=fp.z;

			lr.last_point(lpl);
			for(i=fp.z; i<=lpl.z; i++) {
				if (!lr.empty(i)) {
//cout << "allocate: " << i << endl;
					edms[i] = distance_map_2d(lr, i, mis);
				}
			}

			ROItraverser rt(lr);
			TravStatus s = rt.valid();
			Point p1, p2;
			max = 0;
			while(s<END_ROI) {
				rt.current_interval(p1, p2);
				j=p1.y*xdim+p1.x;
				for(; p1.x<=p2.x; ++p1.x) {
					if (edms[p1.z][j]>=max) {
						max=edms[p1.z][j];
					}
					++j;
				}
				s = rt.next_interval();
			}
	
			thresh = _perc*max/100;

			ROI lresult;
			rt.reset();
			s = rt.valid();
			while(s<END_ROI) {
				rt.current_interval(p1, p2);
				j=p1.y*xdim+p1.x;
				for(; p1.x<=p2.x; ++p1.x) {
					if (edms[p1.z][j]>=thresh) {
						lresult.append_point(p1);
					}
					++j;
				}
				s = rt.next_interval();
			}

			for(i=fp.z; i<=lpl.z; i++) {
				if (edms[i]!=0) {
//cout << "free: " << i << endl;
					delete [] edms[i];
					edms[i] = 0;
				}
			}


			result.OR(lresult);
		}
	}

	if (edms) {
		//for (i=0; i<=lp.z; i++) {
		//	if (edms[i]!=0) {
		//		delete [] edms[i];
		//	}
		//}
		delete [] edms;
	}

	if (or_flag())
		roi.OR(result);
	else
		roi.AND(result);
 }
 return done;
}
*/



ExpandContractPlanar::ExpandContractPlanar(const std::string& rel_solel_name, const int rel_solel_ind,
const float distance, /*const float max_angle_diff,*/
const int e_flag)
	:SearchArea(e_flag),
	_distance(distance) /*_max_angle_diff(max_angle_diff),*/
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


ExpandContractPlanar::~ExpandContractPlanar()
{
}

int bounding_area(Contour& c)
{
	int i;
	int minx, miny, maxx, maxy;

	minx=maxx=c[0].x; miny=maxy=c[0].y;
	for(i=1; i<c.n(); i++) {
		if (c[i].x<minx) minx=c[i].x;
		if (c[i].y<miny) miny=c[i].y;
		if (c[i].x>maxx) maxx=c[i].x;
		if (c[i].y>maxy) maxy=c[i].y;
	}
	return ((maxx-minx)*(maxy-miny));
}

const int ExpandContractPlanar::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];
	ROI result;

	Point p1, p2;
	r0->roi().first_point(p1);
	r0->roi().last_point(p2);

	for(int z=(p1.z/ss_factor.z)*ss_factor.z; z<=(p2.z/ss_factor.z)*ss_factor.z; z+=ss_factor.z) {
		int n;
		Contour* c = r0->roi().boundaries(n, z);
		int ok=1;

		if (n>0) {
			int cont_i=0;
			int cont_ba=bounding_area(c[0]);
			for(int i=1; i<n; i++) {
				int ba = bounding_area(c[i]);
				//if (c[i].n()>c[cont_i].n())
				if (ba>cont_ba) {
					cont_i = i;
					cont_ba = ba;
				}
			}
			Contour& cont = c[cont_i];

			Point cent(0, 0, z);
			for(int j=0; j<cont.n(); j++) {
				cent.x += cont[j].x;
				cent.y += cont[j].y;
			}
			cent.x = cent.x/cont.n();
			cent.y = cent.y/cont.n();

			Contour new_cont;
			Point o1(cont[0]);
			float d_cent = distance_mm(cent, o1, mis);
			if ((_distance*-1)>d_cent)
				ok = 0;
			float xd = (float)(o1.x-cent.x)*(_distance+d_cent)/d_cent;
			float yd = (float)(o1.y-cent.y)*(_distance+d_cent)/d_cent;
			Point n1((int)(cent.x+xd), (int)(cent.y+yd), cent.z);

			int pixel_gap = (int)(_distance/mis.row_pixel_spacing(cent.z));

			new_cont.append(n1);
			for(int j=1; j<cont.n(); j++)
			  if ((j%pixel_gap)==0) {
				const Point& o2 = cont[j];
				d_cent = distance_mm(cent, o2, mis);
				if ((_distance*-1)>d_cent)
					ok = 0;
				xd = (float)(o2.x-cent.x)*(_distance+d_cent)/d_cent;
				yd = (float)(o2.y-cent.y)*(_distance+d_cent)/d_cent;
				Point n2((int)(cent.x+xd), (int)(cent.y+yd), cent.z);
				if ((n2.x!=n1.x) || (n2.y!=n1.y)) {
					new_cont.append(n2);
					n1.x=n2.x; n1.y=n2.y;
				}
			}
			if (ok) {
				result.add_planar_polygon(new_cont);
			}
		}

		delete [] c;
	}

	if ((ss_factor.x*ss_factor.y*ss_factor.z)>1) {
		ROI ss_result;
		subsample_roi(result, ss_result, ss_factor.x, ss_factor.y, ss_factor.z);
		if (or_flag())
			roi.OR(ss_result);
		else
			roi.AND(ss_result);
	}
	else {
		if (or_flag())
			roi.OR(result);
		else
			roi.AND(result);
	}
 }
 return done;
}


Inside2D::Inside2D(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

Inside2D::~Inside2D()
{
}

const int Inside2D::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	ROI rcpy;
	if ((ss_factor.x*ss_factor.y*ss_factor.z)>1)
		subsample_roi(r0->roi(), rcpy, ss_factor.x, ss_factor.y, ss_factor.z);
	else
		rcpy.copy(r0->roi());

	Point p1, p2;
	if (rcpy.first_point(p1) && rcpy.last_point(p2)) {
		int z;
		for(z=p1.z; z<=p2.z; z++)
			rcpy.fill_holes_2D(z);
	}
	if (or_flag())
		roi.OR(rcpy);
	else
		roi.AND(rcpy);
  }
  return done;
}


Morph::Morph(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag)
	: SearchArea(e_flag), _struct_el(struct_el_descr)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}


Morph::Morph(const std::string& struct_el_descr, const int e_flag)
	: SearchArea(e_flag), _struct_el(struct_el_descr)
{
}


Morph::~Morph()
{
}


const int Morph::struct_el(ROI& r, const MedicalImageSequence& mis) const
{
	return _struct_el.roi(r, mis);
}


void Morph::write(ostream& s) const
{
	_write_start_attribute(s);

	s << "Structuring Element: " << _struct_el.description() << endl;

	_write_end_attribute(s);
}


MorphErode::MorphErode(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag)
	: Morph(rel_solel_name, rel_solel_ind, struct_el_descr, e_flag)
{
}


MorphErode::MorphErode(const std::string& struct_el_descr, const int e_flag)
	: Morph(struct_el_descr, e_flag)
{
}


MorphErode::~MorphErode()
{
}


const int MorphErode::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;
  ROI se;
  int ok, use_subsampled = ((ss_factor.x*ss_factor.y*ss_factor.z)>1);
  if (!use_subsampled)
	ok = struct_el(se, mis);
  else {
	ROI se_temp;
	ok = struct_el(se_temp, mis);
	if (ok) {
		subsample_roi(se_temp, se, ss_factor.x, ss_factor.y, ss_factor.z);
		ok = !se.empty();
	}
  }

  if (prim.N() == 0) {
	done = 1;
	if (ok)
		roi.erode(se);
  }
  else if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	if (ok) {
		ROI rcpy;
		if (use_subsampled)
			subsample_roi(r0->roi(), rcpy, ss_factor.x, ss_factor.y, ss_factor.z);
		else
			rcpy.copy(r0->roi());

		rcpy.erode(se);
		if (or_flag())
			roi.OR(rcpy);
		else
			roi.AND(rcpy);
	}
  }
  return done;
}


MorphOpen::MorphOpen(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag)
	: Morph(rel_solel_name, rel_solel_ind, struct_el_descr, e_flag)
{
}


MorphOpen::~MorphOpen()
{
}


const int MorphOpen::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;
  ROI se;
  int ok, use_subsampled = ((ss_factor.x*ss_factor.y*ss_factor.z)>1);
  if (!use_subsampled)
	ok = struct_el(se, mis);
  else {
	ROI se_temp;
	ok = struct_el(se_temp, mis);
	if (ok) {
		subsample_roi(se_temp, se, ss_factor.x, ss_factor.y, ss_factor.z);
		ok = !se.empty();
	}
  }

  if (prim.N() == 0) {
	done = 1;
	if (ok) {
		roi.erode(se);
		roi.dilate(se);
	}
  }
  else if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	if (ok) {
		ROI rcpy;
		if (use_subsampled)
			subsample_roi(r0->roi(), rcpy, ss_factor.x, ss_factor.y, ss_factor.z);
		else
			rcpy.copy(r0->roi());

		rcpy.erode(se);
		rcpy.dilate(se);

		if (or_flag())
			roi.OR(rcpy);
		else
			roi.AND(rcpy);
	}
  }
  return done;
}


MorphClose::MorphClose(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag)
	: Morph(rel_solel_name, rel_solel_ind, struct_el_descr, e_flag)
{
}

//MorphClose::MorphClose(const std::string& rel_solel_name, const int rel_solel_ind, const ROI& roi, const int e_flag)
//	: Morph(rel_solel_name, rel_solel_ind, roi, e_flag)
//{
//}

MorphClose::~MorphClose()
{
}


const int MorphClose::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
	//std::cout << std::endl << "Entering MorphClose::search_area" << std::endl << std::flush;
  int done = 0;
  ROI se;
  int ok, use_subsampled = ((ss_factor.x*ss_factor.y*ss_factor.z)>1);
  if (!use_subsampled)
	ok = struct_el(se, mis);
  else {
	ROI se_temp;
	ok = struct_el(se_temp, mis);
	if (ok) {
		subsample_roi(se_temp, se, ss_factor.x, ss_factor.y, ss_factor.z);
		ok = !se.empty();
	}
  }

  if (prim.N() == 0) {
	done = 1;
	if (ok) {
		roi.dilate(se);
		roi.erode(se);
	}
  }
  else if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	if (ok) {
		ROI rcpy;
		if (use_subsampled)
			subsample_roi(r0->roi(), rcpy, ss_factor.x, ss_factor.y, ss_factor.z);
		else
			rcpy.copy(r0->roi());

  //std::cout << std::endl << std::endl << "ORIGINAL" << std::endl;
  //rcpy.print_all_points();
		rcpy.dilate(se);
  //std::cout << std::endl << std::endl << "DILATE" << std::endl;
  //rcpy.print_all_points();
		rcpy.erode(se);
  //std::cout << std::endl << std::endl << "ERODE" << std::endl;
  //rcpy.print_all_points();
		if (or_flag())
			roi.OR(rcpy);
		else
			roi.AND(rcpy);
  //std::cout << std::endl << std::endl << "SEARCH AREA" << std::endl;
  //roi.print_all_points();
	}
  }
  return done;
}


MorphDilate::MorphDilate(const std::string& rel_solel_name, const int rel_solel_ind, const std::string& struct_el_descr, const int e_flag)
	: Morph(rel_solel_name, rel_solel_ind, struct_el_descr, e_flag)
{
}

MorphDilate::MorphDilate(const std::string& struct_el_descr, const int e_flag)
	: Morph(struct_el_descr, e_flag)
{
}

MorphDilate::~MorphDilate()
{
}


const int MorphDilate::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;
  ROI se;
  int ok, use_subsampled = ((ss_factor.x*ss_factor.y*ss_factor.z)>1);
  if (!use_subsampled)
	ok = struct_el(se, mis);
  else {
	ROI se_temp;
	ok = struct_el(se_temp, mis);
	if (ok) {
		subsample_roi(se_temp, se, ss_factor.x, ss_factor.y, ss_factor.z);
		ok = !se.empty();
	}
  }

  if (prim.N() == 0) {
	done = 1;
	if (ok)
		roi.dilate(se);
  }
  else if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	if (ok) {
		ROI rcpy;
		if (use_subsampled)
			subsample_roi(r0->roi(), rcpy, ss_factor.x, ss_factor.y, ss_factor.z);
		else
			rcpy.copy(r0->roi());

		rcpy.dilate(se);
		if (or_flag())
			roi.OR(rcpy);
		else
			roi.AND(rcpy);
	}
  }
  //roi.print_all_points();
  return done;
}


Found::Found(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag, const int yes_flag)
	: SearchArea(e_flag), _yes_flag(yes_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

Found::~Found()
{
}

const int Found::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if (((_yes_flag==0)&&(prim.N()==1)) || ((_yes_flag==1)&&(prim.N()==0))) {
	done = 1;

	roi.clear();
  }
  return done;
}



NotPartOf::NotPartOf(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

NotPartOf::~NotPartOf()
{
}

const int NotPartOf::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

 	 if ((ss_factor.x*ss_factor.y*ss_factor.z)>1) {
		ROI ss_r0;
		subsample_roi(r0->roi(), ss_r0, ss_factor.x, ss_factor.y, ss_factor.z);
		roi.subtract(ss_r0);
  	}
  	else
		roi.subtract(r0->roi());
  }
  return done;
}


PartOf::PartOf(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

PartOf::~PartOf()
{
}

const int PartOf::search_area(MedicalImageSequence& mis, const Point& ss_factor, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

 	if ((ss_factor.x*ss_factor.y*ss_factor.z)>1) {
		ROI ss_r0;
		subsample_roi(r0->roi(), ss_r0, ss_factor.x, ss_factor.y, ss_factor.z);
		if (or_flag())
			roi.OR(ss_r0);
		else
			roi.AND(ss_r0);
	}
	else {
		if (or_flag()) {
			roi.OR(r0->roi());
		}
		else
			roi.AND(r0->roi());
	}
  } 
  return done;
}


/*
PartOfOR::PartOfOR(const std::string& rel_solel_name, const int rel_solel_ind, const int e_flag)
	: SearchArea(e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

PartOfOR::~PartOfOR()
{
}

const int PartOfOR::search_area(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, ROI& roi)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	ImageRegion* r0 = (ImageRegion*)prim[0];

	roi.or(r0->roi());
  }
  return done;
}
*/
