#include "Feature.h"
#include "tools_miu.h"

Feature::Feature(const Fuzzy& fuzzy, const int b_flag, const int e_flag)
	: _fuzzy(fuzzy)
{
 	_b_flag = b_flag;
	_e_flag = e_flag;
}

Feature::Feature(const Feature& f)
	: Attribute(f), _fuzzy(f._fuzzy)
{
	//_b_flag = f._b_flag;
}

Feature::~Feature()
{
}

const Fuzzy& Feature::fuzzy() const
{
	return _fuzzy;
}

void Feature::write(ostream& s) const
{
	_write_start_attribute(s);
	s << "Fuzzy: " << fuzzy() << endl;
	_write_end_attribute(s);
}


Area_maxXY::Area_maxXY(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}


Area_maxXY::Area_maxXY(const Area_maxXY& v)
	: Feature(v)
{
}


Area_maxXY::~Area_maxXY()
{
}


const int Area_maxXY::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	Point fp, lp;
	r0->roi().first_point(fp);
	r0->roi().last_point(lp);

	int z;
	float a;
	val=0;
	for(z=fp.z; z<=lp.z; z++) {
		a = r0->roi().num_pix(z)*mis.row_pixel_spacing(z)*mis.column_pixel_spacing(z);
		if (a>val) val=a;
	}
  }
  return done;
}


Area_perXY::Area_perXY(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}


Area_perXY::Area_perXY(const Area_perXY& v)
	: Feature(v)
{
}


Area_perXY::~Area_perXY()
{
}


const int Area_perXY::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	Point fp, lp;
	r0->roi().first_point(fp);
	r0->roi().last_point(lp);

	val = r0->area_xy()/(lp.z-fp.z+1);
  }
  return done;
}


Area_XY::Area_XY(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

Area_XY::Area_XY(const Area_XY& v)
	: Feature(v)
{
}

Area_XY::~Area_XY()
{
}

const int Area_XY::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	val = r0->area_xy();
  }
  return done;
}


AvgDiameter::AvgDiameter(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

AvgDiameter::AvgDiameter(const AvgDiameter& c)
	: Feature(c)
{
}

AvgDiameter::~AvgDiameter()
{
}

const int AvgDiameter::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
	
  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	
	Point fp;
	if (r1->roi().first_point(fp)) {
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
		mdist_pt1.x = -1;
		mpdist_pt1.x = -1;
  		double max_diameter, perp_diameter;
  
  		compute_diameters(r1->roi(), mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);

		if (mdist_pt1.x==-1) max_diameter=mis.row_pixel_spacing(fp.z);
		if (mpdist_pt1.x==-1) perp_diameter=mis.row_pixel_spacing(fp.z);
    
  		val = (max_diameter+perp_diameter)/2.0;
  	}
  }
  return done;  
}


AvgDiameterInSliceThicknesses::AvgDiameterInSliceThicknesses(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

AvgDiameterInSliceThicknesses::AvgDiameterInSliceThicknesses(const AvgDiameterInSliceThicknesses& c)
	: Feature(c)
{
}

AvgDiameterInSliceThicknesses::~AvgDiameterInSliceThicknesses()
{
}

const int AvgDiameterInSliceThicknesses::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
	
  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	
	Point fp;
	if (r1->roi().first_point(fp)) {
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
		mdist_pt1.x = -1;
		mpdist_pt1.x = -1;
  		double max_diameter, perp_diameter;
  
  		compute_diameters(r1->roi(), mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);

		if (mdist_pt1.x==-1) max_diameter=mis.row_pixel_spacing(fp.z);
		if (mpdist_pt1.x==-1) perp_diameter=mis.row_pixel_spacing(fp.z);

  		val = (max_diameter+perp_diameter)/(2.0*mis.slice_thickness(mdist_pt1.z));
  	}
  }
  return done;  
}


CentroidDistance::CentroidDistance(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidDistance::CentroidDistance(const CentroidDistance& c)
	: Feature(c)
{
}

CentroidDistance::~CentroidDistance()
{
}

const int CentroidDistance::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	Point p1, p2;
	fpoint_to_point(r1->centroid(), p1);
	fpoint_to_point(r2->centroid(), p2);
	val = distance_mm(p1, p2, mis);
  }
  return done;
}



CentroidInside::CentroidInside(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidInside::CentroidInside(const CentroidInside& c)
	: Feature(c)
{
}

CentroidInside::~CentroidInside()
{
}

const int CentroidInside::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	Point cent;
	fpoint_to_point(r1->centroid(), cent);
	val = r2->roi().in_roi(cent);
  }
  return done;
}



CentroidOffsetMaxZ::CentroidOffsetMaxZ(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidOffsetMaxZ::CentroidOffsetMaxZ(const CentroidOffsetMaxZ& c)
	: Feature(c)
{
}

CentroidOffsetMaxZ::~CentroidOffsetMaxZ()
{
}

const int CentroidOffsetMaxZ::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	register FPoint r1c(r1->centroid());
	Point p1(round_float(r1c.x), round_float(r1c.y), round_float(r1c.z));
	Point p2;
	r2->roi().last_point(p2);
	p2.x = p1.x;
	p2.y = p1.y;
	val = distance_mm(p1, p2, mis);
	if (p1.z>p2.z)
		val = -val;
	//cout << "z1 z2 val = " << p1.z << " " << p2.z << " " << val << endl;
  }
  return done;
}


CentroidOffsetX::CentroidOffsetX(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidOffsetX::CentroidOffsetX(const CentroidOffsetX& c)
	: Feature(c)
{
}

CentroidOffsetX::~CentroidOffsetX()
{
}

const int CentroidOffsetX::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	FPoint p1(r1->centroid()), p2(r2->centroid());
	val = (p1.x - p2.x)*mis.column_pixel_spacing((int)p1.z);
  }
  return done;
}



CentroidOffsetY::CentroidOffsetY(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidOffsetY::CentroidOffsetY(const CentroidOffsetY& c)
	: Feature(c)
{
}

CentroidOffsetY::~CentroidOffsetY()
{
}

const int CentroidOffsetY::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	FPoint p1(r1->centroid()), p2(r2->centroid());
	val = (p1.y - p2.y)*mis.column_pixel_spacing((int)p1.z);
  }
  return done;
}




CentroidOffsetZ::CentroidOffsetZ(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

CentroidOffsetZ::CentroidOffsetZ(const CentroidOffsetZ& c)
	: Feature(c)
{
}

CentroidOffsetZ::~CentroidOffsetZ()
{
}

const int CentroidOffsetZ::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	register FPoint r1c(r1->centroid());
	Point p1(round_float(r1c.x), round_float(r1c.y), round_float(r1c.z));
	Point p2(p1);
	p2.z = round_float(r2->centroid().z);
	val = distance_mm(p1, p2, mis);
	if (p2.z>p1.z)
		val = -val;
	//cout << "z1 z2 val = " << p1.z << " " << p2.z << " " << val << endl;
  }
  return done;
}



CircularityAtMaxDia::CircularityAtMaxDia(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

CircularityAtMaxDia::CircularityAtMaxDia(const CircularityAtMaxDia& s)
	: Feature(s)
{
}

CircularityAtMaxDia::~CircularityAtMaxDia()
{
}

const int CircularityAtMaxDia::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	Point fp, lp;
	r0->roi().first_point(fp);
	r0->roi().last_point(lp);

	if (r0->roi().first_point(fp)) {
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
		mdist_pt1.x = -1;
  		double max_diameter, perp_diameter;
  
  		compute_diameters(r0->roi(), mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);
 
		//if (mdist_pt1.x==-1) max_diameter=mis.row_pixel_spacing(fp.z);
  		//val = max_diameter/mis.slice_thickness(mdist_pt1.z);
		
		float max_r;
		ROItraverser rt(r0->roi());
		Point cent;
		register Point p1, p2;
		register float d1, d2;
		TravStatus s;
		float area;
		val = 100;

		int z=mdist_pt1.z;
		const FPoint* fcent= r0->centroid(z);
		if (fcent) {
			fpoint_to_point(*fcent, cent);
			max_r = 0;
			rt.set_plane(z);
			s = ROI_STAT_OK;
			area = 0;

			while(s<NEW_PLANE) {
				rt.current_interval(p1, p2);
				area += (p2.x-p1.x+1)*mis.row_pixel_spacing(z)*mis.column_pixel_spacing(z);
				d1 = distance_mm(p1, cent, mis);
				if (d1>max_r)
					max_r = d1;
				d2 = distance_mm(p2, cent, mis);
				if (d2>max_r)
					max_r = d2;
				s = rt.next_interval();
			}

			if (max_r>0) {
				done = 1;
				float area_bnd_circle = 3.14159*max_r*max_r;
				val = 100*area/area_bnd_circle;
			}
		}

  	}
  }
  return done;
}


CircularityMin::CircularityMin(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

CircularityMin::CircularityMin(const CircularityMin& s)
	: Feature(s)
{
}

CircularityMin::~CircularityMin()
{
}

const int CircularityMin::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	Point fp, lp;
	r0->roi().first_point(fp);
	r0->roi().last_point(lp);

	float max_r;
	ROItraverser rt(r0->roi());
	Point cent;
	register Point p1, p2;
	register float d1, d2;
	TravStatus s;
	float area;
	float val_plane;
	val = 100;

	for(int z=fp.z; z<=lp.z; z++) {
	  const FPoint* fcent= r0->centroid(z);
		if (fcent) {
			fpoint_to_point(*fcent, cent);
			max_r = 0;
			rt.set_plane(z);
			s = ROI_STAT_OK;
			area = 0;

			while(s<NEW_PLANE) {
				rt.current_interval(p1, p2);
				area += (p2.x-p1.x+1)*mis.row_pixel_spacing(z)*mis.column_pixel_spacing(z);
				d1 = distance_mm(p1, cent, mis);
				if (d1>max_r)
					max_r = d1;
				d2 = distance_mm(p2, cent, mis);
				if (d2>max_r)
					max_r = d2;
				s = rt.next_interval();
			}

			if (max_r>0) {
				done = 1;
				float area_bnd_circle = 3.14159*max_r*max_r;
				val_plane = 100*area/area_bnd_circle;
				if (val_plane<val)
					val= val_plane;
			}
		}
				
	}
  }
  return done;
}


PlanarCentroidOffsetX::PlanarCentroidOffsetX(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

PlanarCentroidOffsetX::PlanarCentroidOffsetX(const PlanarCentroidOffsetX& c)
	: Feature(c)
{
}

PlanarCentroidOffsetX::~PlanarCentroidOffsetX()
{
}




Contacts::Contacts(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag, const int threed_flag)
	: Feature(f, b_flag, e_flag), _threed(threed_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

Contacts::Contacts(const Contacts& c)
	: Feature(c)
{
}

Contacts::~Contacts()
{
}

const int Contacts::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];
	
	ROI r1d(r1->roi());
	ROI se;
	Point tl(-1,-1,0);
	Point br(1,1,0);
	if (_threed==1) {
		tl.z = -1;
		br.z = 1;
	}
	se.add_box(tl, br);
	r1d.dilate(se);

	val = (float)r1d.overlaps(r2->roi());
  }
  return done;
}


LengthInSliceThicknesses::LengthInSliceThicknesses(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

LengthInSliceThicknesses::LengthInSliceThicknesses(const LengthInSliceThicknesses& c)
	: Feature(c)
{
}

LengthInSliceThicknesses::~LengthInSliceThicknesses()
{
}

const int LengthInSliceThicknesses::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];

	Point fp, lp;
	if (r1->roi().first_point(fp) && r1->roi().last_point(lp))
		val = fabs(mis.slice_location(lp.z)-mis.slice_location(fp.z))/mis.slice_thickness(fp.z);
  }
  return done;
}



MaxBBoxLength::MaxBBoxLength(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

MaxBBoxLength::MaxBBoxLength(const MaxBBoxLength& c)
	: Feature(c)
{
}

MaxBBoxLength::~MaxBBoxLength()
{
}

const int MaxBBoxLength::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
	
  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	
	Point tl, br;
	if (r1->roi().bounding_cube(tl, br)) {
        Point px(br.x,tl.y,tl.z);
		float dx = distance_mm(tl, px, mis);
        Point py(tl.x,br.y,tl.z);
		float dy = distance_mm(tl, py, mis);
        Point pz(tl.x,tl.y,br.z);
		float dz = distance_mm(tl, pz, mis);
 
//cout << "mis.row_pixel_spacing(py.z) = " << mis.row_pixel_spacing(py.z) <<endl;
//cout << "mis.column_pixel_spacing(px.z) = " << mis.column_pixel_spacing(px.z) <<endl;

  		val = std::max(dx,std::max(dy,dz));
  	}
  }
  return done;  
}




MaxDiameterInSliceThicknesses::MaxDiameterInSliceThicknesses(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

MaxDiameterInSliceThicknesses::MaxDiameterInSliceThicknesses(const MaxDiameterInSliceThicknesses& c)
	: Feature(c)
{
}

MaxDiameterInSliceThicknesses::~MaxDiameterInSliceThicknesses()
{
}

const int MaxDiameterInSliceThicknesses::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
	
  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	
	Point fp;
	if (r1->roi().first_point(fp)) {
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
		mdist_pt1.x = -1;
  		double max_diameter, perp_diameter;
  
  		compute_diameters(r1->roi(), mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);
 
		if (mdist_pt1.x==-1) max_diameter=mis.row_pixel_spacing(fp.z);

  		val = max_diameter/mis.slice_thickness(mdist_pt1.z);
  	}
  }
  return done;  
}


MedianHU::MedianHU(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

MedianHU::MedianHU(const MedianHU& s)
	: Feature(s)
{
}

MedianHU::~MedianHU()
{
}

const int MedianHU::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
  
  if ((prim.N()==1) && (!strcmp(prim[0]->type(), "ImageRegion"))) {
  	done = 1;
  	const ImageRegion* r0 = (ImageRegion*)prim[0];
  	ROItraverser rt(r0->roi());
  	val = (float) medianHU(mis, rt);
  }
  else if (prim.N()>1) {
  	ROI r;
  	for(int i=0; i<prim.N(); i++) {  
	  if (!strcmp(prim[i]->type(), "ImageRegion")) {
		done = 1;
		const ImageRegion* r0 = (ImageRegion*)prim[i];
		r.OR(r0->roi()); 
	  }
	}
	if (done) {
		ROItraverser rt(r);
  		val = (float) medianHU(mis, rt);
  	}
  }
  return done;
}


Overlaps::Overlaps(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

Overlaps::Overlaps(const Overlaps& c)
	: Feature(c)
{
}

Overlaps::~Overlaps()
{
}

const int Overlaps::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	val = (float)r1->roi().overlaps(r2->roi());
  }
  return done;
}



PerpDiameterInSliceThicknesses::PerpDiameterInSliceThicknesses(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

PerpDiameterInSliceThicknesses::PerpDiameterInSliceThicknesses(const PerpDiameterInSliceThicknesses& c)
	: Feature(c)
{
}

PerpDiameterInSliceThicknesses::~PerpDiameterInSliceThicknesses()
{
}

const int PerpDiameterInSliceThicknesses::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;
	
  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	
	Point fp;
	if (r1->roi().first_point(fp)) {
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
  		double max_diameter, perp_diameter;
  
  		compute_diameters(r1->roi(), mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);
    
  		val = perp_diameter/mis.slice_thickness(mdist_pt1.z);
  	}
  }
  return done;  
}



const int PlanarCentroidOffsetX::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	FPoint p1(r1->centroid());
	const FPoint* p2 = r2->centroid((int)p1.z);
	if (!p2) {
		Point p;
		r2->roi().first_point(p);
		if (p1.z<p.z) {
			p2 = r2->centroid(p.z);
		}
		else {
			r2->roi().last_point(p);
			if (p1.z>p.z) {
				p2 = r2->centroid((int)p.z);
			}
			else {
				for(int i=1; !p2; i++) {
					p2 = r2->centroid((int)(p1.z+i));
					if (!p2) p2 = r2->centroid((int)(p1.z-i));
				}
			}
		}
	}
	val = (p1.x - p2->x)*mis.column_pixel_spacing((int)p1.z);
  }
  return done;
}
/*
const int PlanarCentroidOffsetX::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	FPoint p1(r1->centroid());
	FPoint* p2 = r2->centroid(p1.z);
	if (p2) {
		val = (p1.x - p2->x)*mis.column_pixel_spacing(p1.z);
		done = 1;
	}
  }
  return done;
}
*/



PlanarCentroidOffsetY::PlanarCentroidOffsetY(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

PlanarCentroidOffsetY::PlanarCentroidOffsetY(const PlanarCentroidOffsetY& c)
	: Feature(c)
{
}

PlanarCentroidOffsetY::~PlanarCentroidOffsetY()
{
}

const int PlanarCentroidOffsetY::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];

	FPoint p1(r1->centroid());
	const FPoint* p2 = r2->centroid((int)p1.z);
	if (!p2) {
		Point p;
		r2->roi().first_point(p);
		if (p1.z<p.z) {
			p2 = r2->centroid(p.z);
		}
		else {
			r2->roi().last_point(p);
			if (p1.z>p.z) {
				p2 = r2->centroid(p.z);
			}
			else {
				for(int i=1; !p2; i++) {
					p2 = r2->centroid((int)(p1.z+i));
					if (!p2) p2 = r2->centroid((int)(p1.z-i));
				}
			}
		}
	}
	val = (p1.y - p2->y)*mis.column_pixel_spacing((int)p1.z);
  }
  return done;
}

/*
RadLogicsNoduleClassification::RadLogicsNoduleClassification(const Fuzzy& f, const int node_to_be_classified)
	: Feature(f, 0, 0), _node_to_be_classified(node_to_be_classified)
{
}

RadLogicsNoduleClassification::RadLogicsNoduleClassification(const RadLogicsNoduleClassification& s)
	: Feature(s), _node_to_be_classified(s._node_to_be_classified)
{
}

RadLogicsNoduleClassification::~RadLogicsNoduleClassification()
{
}

// Dummy function. Will be provided by RadLogics
void RunNoduleClassification(double& classification_score, int& classification_result, const short* const image_volume, const ROI& roi, double meanHU, double max_diameter, double perp_diameter, double sphericity, int node_to_be_classified) {
}

const int RadLogicsNoduleClassification::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	const ImageRegion* r0 = (ImageRegion*)prim[0];
	
	const ROI& roi = r0->roi();
	ROItraverser rt(roi);
	
	Point fp;
	if (roi.first_point(fp)) {
		done = 1;
		
		Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
		double max_diameter, perp_diameter;		
		compute_diameters(roi, mis.row_pixel_spacing(fp.z), mis.column_pixel_spacing(fp.z), mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);

		int classification_result; // 0,1 values indicating whether nodule or not
		double classification_score;
		RunNoduleClassification(classification_score, classification_result, mis.hu_values_column_major(), roi, meanHU(mis, rt), max_diameter, perp_diameter, sphericity(roi, r0->centroid(), r0->volume(), mis), _node_to_be_classified);
			
		val = (float) classification_result;
	}
  }
  return done;			
}
*/


ShortAxisInPixels::ShortAxisInPixels(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

ShortAxisInPixels::ShortAxisInPixels(const ShortAxisInPixels& c)
	: Feature(c)
{
}

ShortAxisInPixels::~ShortAxisInPixels()
{
}

const int ShortAxisInPixels::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
	cout << "Entering ShortAxisInPixels" << endl;
  int done = 0;

  if ((prim.N()==1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];

	Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2;
	mpdist_pt1.x = -1;
	double max_diameter, perp_diameter;

	// Pixel spacing arguments set to 1.0 so that distances are in units of pixels
	compute_diameters(r1->roi(), 1.0, 1.0, mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, max_diameter, perp_diameter);

	if (mpdist_pt1.x==-1) perp_diameter=1.0;

	//cout << "short axis (pixels): " << mdist_pt1 <<  mdist_pt2 <<  max_diameter <<  mpdist_pt1 << mpdist_pt2 << perp_diameter << endl;

	val = perp_diameter;
  }
  return done;
}


Sphericity::Sphericity(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

Sphericity::Sphericity(const Sphericity& s)
	: Feature(s)
{
}

Sphericity::~Sphericity()
{
}

const int Sphericity::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	float s = sphericity(r0->roi(), r0->centroid(), r0->volume(), mis);
	if (s != -1.0) {
		done = 1;
		val = s;
	}
  }
  return done;
}

/*
const int Sphericity::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	float max_r = 0;
	ROItraverser rt(r0->roi());
	TravStatus s = rt.valid();
	register Point p1, p2;
	register float d1, d2;
	FPoint cent(r0->centroid());
	FPoint p1f, p2f;
	while(s<END_ROI) {
		rt.current_interval(p1, p2);
		p1f.x=p1.x; p1f.y=p1.y; p1f.z=p1.z;
		p2f.x=p2.x; p2f.y=p2.y; p2f.z=p2.z;
		d1 = distance_mm(p1f, cent, mis);
		if (d1>max_r) {
			max_r = d1;
		}
		d2 = distance_mm(p2f, cent, mis);
		if (d2>max_r) {
			max_r = d2;
		}
		s = rt.next_interval();
	}

	if (max_r>0) {
		done = 1;
		float vol_bnd_sphere = 4*3.14159*max_r*max_r*max_r/3;
		val = 100*r0->volume()/vol_bnd_sphere;
	}
  }
  return done;
}
*/

SurfaceContactPercentage::SurfaceContactPercentage(const std::string& rel_solel_name, const int rel_solel_ind, const Fuzzy& f, const int b_flag, const int e_flag)
	: Feature(f, b_flag, e_flag)
{
	add_rel_solel(rel_solel_name, rel_solel_ind);
}

SurfaceContactPercentage::SurfaceContactPercentage(const SurfaceContactPercentage& c)
	: Feature(c)
{
}

SurfaceContactPercentage::~SurfaceContactPercentage()
{
}

const int SurfaceContactPercentage::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N()==2)
    && !strcmp(prim[0]->type(), "ImageRegion")
    && !strcmp(prim[1]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r1 = (ImageRegion*)prim[0];
	const ImageRegion* r2 = (ImageRegion*)prim[1];
	
	ROI r1d(r1->roi());
	ROI se;
	Point tl(-1,-1,-1);
	Point br(1,1,1);
	se.add_box(tl, br);
	r1d.dilate(se);

	r1d.subtract(r1->roi());
	int denominator = r1d.num_pix();
	r1d.AND(r2->roi());
	int numerator = r1d.num_pix();

	val = (float)100.0*numerator/denominator;
//Point t, b;
//r1->roi().bounding_cube(t,b);
//Point m((t.x+b.x)/2,(t.y+b.y)/2,(t.z+b.z)/2);
//cout << m << endl;
//cout << "SurfaceContactPercentage value = " << val << endl;
//cout << "Number of voxels = " << r1->roi().num_pix() << endl;
  }
  return done;
}




Volume::Volume(const Fuzzy& f)
	: Feature(f, 0, 0)
{
}

Volume::Volume(const Volume& v)
	: Feature(v)
{
}

Volume::~Volume()
{
}

const int Volume::value(MedicalImageSequence& mis, const Darray<ImagePrimitive*>& prim, float& val)
{
  int done = 0;

  if ((prim.N() == 1)
    && !strcmp(prim[0]->type(), "ImageRegion")) {
	done = 1;
	const ImageRegion* r0 = (ImageRegion*)prim[0];

	val = r0->volume();
  }
  return done;
}

