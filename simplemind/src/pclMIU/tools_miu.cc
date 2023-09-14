#include "tools_miu.h"
#include "PercentileCalculator.h"
#include <math.h>

/**
Checks the points in the contour (c) with indices in the range [low_index, high_index). If the distance between the point in the contour and the given point p is > max_diameter_pix, then return the new value for max_diameter_pix and set mdist_pt1, and mdist_pt2.
Otherwise return return the existing value of max_diameter_pix and do not modify mdist_pt1, and mdist_pt2.
*/
float maxDist(Point p, Contour c, int low_index, int high_index, float mdist, Point& mdist_pt1, Point& mdist_pt2) {
	int i, dx, dy;
	float d;
	float newmdist = mdist;
	for (i = low_index; i < high_index; i++) {
		dx = (c[i].x-p.x);
		dy = (c[i].y-p.y);
		d = sqrtf((float)(dx*dx + dy*dy));

		if (d> newmdist) {
			newmdist = d;
			mdist_pt1 = p;
			mdist_pt2 = c[i];
		}
	}
	return newmdist;
}


    	/**
	Checks the points in the contour (c) with indices in the range [low_index, high_index).
	If the distance between the point in the contour and the given point p is > mdist, then mdist is changed and mdist_pt1, and mdist_pt2 are modified.
	mdist is assumed to be an array with one element.
	However, pairs of points are only considered if they are approximately perpendicular to line between ml_pt1 and ml_pt2
	Returns true if mdist was changed.
	Argument, perp_offset, indicates the offset from -1 that is allowable for gradients if lines are to be considered perpendicular.
	*/
bool maxDistConGrad(const Point& p, Contour c, int low_index, int high_index, const Point& ml_pt1, const Point& ml_pt2, float& mdist, Point& mdist_pt1, Point& mdist_pt2, float perp_offset) {
		int i, dx, dy;
		bool ml_vert=false, vert, found=false, ok;
		float d=0, ml_g=0, g=0;
		if (ml_pt1.x!=ml_pt2.x) {
			ml_g = (float)(ml_pt2.y-ml_pt1.y)/(ml_pt2.x-ml_pt1.x);
		}
		else {
		  ml_vert = true;
		}

		for(i=low_index; i<high_index; i++) {
			dx = (c[i].x-p.x);
			dy = (c[i].y-p.y);
			if (dx!=0) {
				vert=false;
				g = (float)dy/dx;
			}
			else {
				vert = true;
			}

			if (ml_vert) {
				ok = !vert && (g<=perp_offset) && (g>=(0-perp_offset));
			}
			else if (vert) {
				ok = (ml_g<=perp_offset) && (ml_g>=(0-perp_offset));
			}
			else {
				ok = (ml_g*g>=(-1-perp_offset)) && (ml_g*g<=(-1+perp_offset));
			}


			if (ok) {
				d = sqrtf((float)(dx*dx + dy*dy));
				if (d>mdist) {
					found = true;
					mdist = d;
					mdist_pt1 = p;
					mdist_pt2 = c[i];
				}
			}
		}	// end of statement for(i=low_index; i<high_index; i++)

		return found;
	}

void compute_diameters(const ROI& currentRoi, const float row_pixel_spacing, const float col_pixel_spacing, Point& mdist_pt1, Point& mdist_pt2, Point& mpdist_pt1, Point& mpdist_pt2, double& max_diameter, double& perp_diameter) {
		if (currentRoi.empty()) {
            max_diameter=0;
			perp_diameter=0;
            return;
        }

        int ci, pn, pi;
		float max_diameter_pix = 0, perp_diameter_pix = 0;
		Point fp, lp;
		currentRoi.first_point(fp);
		currentRoi.last_point(lp); 

		Contour* contours;
		int Ncontours;
		//find and compute the maximium diameter
		for(int z=fp.z; z<=lp.z; z++) {
			contours = currentRoi.boundaries(Ncontours, z);
			for(ci=0; ci<Ncontours; ci++) {
				Contour& cont = contours[ci];
				pn = cont.n();
				for(pi=0; pi<pn; pi++) {
					max_diameter_pix = maxDist(cont[pi], cont, 0, pn, max_diameter_pix, mdist_pt1, mdist_pt2);
				}
			}
		}

//cout << fp << lp << mdist_pt1 << mdist_pt2 << endl;

		//find and compute the perpendicular maximium diameter
		if (Ncontours>0) {
			contours = currentRoi.boundaries(Ncontours, mdist_pt1.z);
			bool found = false;
			float poffset = 0.25f;
			for (int i=1; i<9 && !found; i=2*i ) {
				for(ci=0; ci<Ncontours; ci++) {
					Contour& cont = contours[ci];
					pn = cont.n();
					for(pi=0; pi<pn ; pi++) {
						bool f = maxDistConGrad(cont[pi], cont, 0, pn, mdist_pt1, mdist_pt2, perp_diameter_pix, mpdist_pt1, mpdist_pt2, poffset*i);
						if ( f ) found = f;
					}
				}
			}
	  	}	// end of statement if (contours.size()>0)
		//double[] pixel_spacing = new double[2];

		max_diameter = sqrt(((mdist_pt1.x-mdist_pt2.x)*col_pixel_spacing)*((mdist_pt1.x-mdist_pt2.x)*col_pixel_spacing) + ((mdist_pt1.y-mdist_pt2.y)*row_pixel_spacing)*((mdist_pt1.y-mdist_pt2.y)*row_pixel_spacing));
		perp_diameter = sqrt(((mpdist_pt1.x-mpdist_pt2.x)*col_pixel_spacing)*((mpdist_pt1.x-mpdist_pt2.x)*col_pixel_spacing) + ((mpdist_pt1.y-mpdist_pt2.y)*row_pixel_spacing)*((mpdist_pt1.y-mpdist_pt2.y)*row_pixel_spacing));

        if ((row_pixel_spacing==0.0) && (col_pixel_spacing==0.0)) {
            max_diameter = -max_diameter;
            perp_diameter = -perp_diameter;
        }
		//cout << "compute_diameters: " <<  mdist_pt1 << mdist_pt2 << max_diameter << mpdist_pt1 << mpdist_pt2 << perp_diameter << endl;
}

double meanHU(MedicalImageSequence& mis, ROItraverser& rt)
{ 
  double mean = 0.0;
  register int sum=0;
  register int n=0;
  register Point p1;
  TravStatus s = rt.reset();
  while(s<END_ROI) {
  	rt.current_point(p1);
  	sum += mis.fast_pix_val(p1.x, p1.y, p1.z);
  	n++;
  	s = rt.next_point();
  }  
  if (n>0) {
  	const Image& im = mis.image_const(p1.z);
 	sum = sum*im.rescale_slope()+im.rescale_intercept(); 
 	mean = (double)sum/(double)n;
 }

  return mean;
}

int medianHU(MedicalImageSequence& mis, ROItraverser& rt)
{ 
  PercentileCalculator<int> pc;
  register Point p1;
  TravStatus s = rt.reset();
  while(s<END_ROI) {
  	rt.current_point(p1);
  	pc.addValue(mis.fast_pix_val(p1.x, p1.y, p1.z));
  	s = rt.next_point();
  }  
  int val = pc.getMedian();

  const Image& im = mis.image_const(p1.z);
  val = val*im.rescale_slope()+im.rescale_intercept();  

  return val;
}

float sphericity(const ROI& r, const FPoint& cent, float volume, const MedicalImageSequence& mis)
{
	float val=-1.0;

	float max_r = 0;
	//ROItraverser rt(r0->roi());
	ROItraverser rt(r);
	TravStatus s = rt.valid();
	register Point p1, p2;
	register float d1, d2;
	//FPoint cent(r0->centroid());
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
		float vol_bnd_sphere = 4*3.14159*max_r*max_r*max_r/3;
		val = 100*volume/vol_bnd_sphere;
	}
  	return val;
}

float* distance_map_2d(const ROI& r, const int z, const MedicalImageSequence& mis) {
	// Euclidean Distance Map algorithm from textbook by Russ
	int xdim = mis.xdim();
	int ydim = mis.ydim();
	int np = xdim*ydim;
	float* dm = new float [np];

	// Assign background to 0 and Roi to large +ve number in the distance map image
	int i;
	for(i=0; i<np; i++) {
		dm[i] = 0;
	}

	if (!r.empty(z)) {
		int* inds = new int [np];
		int ind_cnt=0;

		ROItraverser rt(r, z);
		TravStatus s = rt.valid();
		Point p1, p2;
		int j;
		float x_spacing = mis.column_pixel_spacing(z);
		float y_spacing = mis.row_pixel_spacing(z);
		float diag_spacing = sqrt(x_spacing*x_spacing + y_spacing*y_spacing);
		float min;
//int jmark=-1;
		// Assign pixels in the Roi to large +ve number in the distance map image
		while(s<END_ROI) {
			rt.current_interval(p1, p2);
			// Since we are checking neighbors do not use pixels at the image border as central pixels
			if ((p1.y>0) && (p1.y<(ydim-1))) {
				if (p1.x<=0) p1.x=1;
				if (p2.x>=(xdim-1)) p1.x=xdim-2;
				if (p1.x<=p2.x) {
					j=p1.y*xdim+p1.x;
					for(; p1.x<=p2.x; ++p1.x) {
//if ((p1.z==98)&&(p1.x>140)&&(p1.x<150)&&(p1.y>253)&&(p1.y<260)) {
//cout << p1 << endl;
//jmark=j;
//}
						dm[j]=(float)np;
						inds[ind_cnt]=j;
						++ind_cnt;
						++j;
					}
				}
				s = rt.next_interval();
			}
			else {
				s = rt.next_line();
			}
		}
//if (jmark>0)
//cout << jmark << endl;

		// Proceeding left to right, top to bottom, assign each pixel within the ROI a brightness value one greater than the smallest value of any of its neighbors
		for(i=0; i<ind_cnt; i++) {
			j=inds[i];

			min = (dm[j-xdim-1]+diag_spacing);
			if ((dm[j-xdim]+y_spacing)<min) min=(dm[j-xdim]+y_spacing);
			if ((dm[j-xdim+1]+diag_spacing)<min) min=(dm[j-xdim+1]+diag_spacing);
			if ((dm[j-1]+x_spacing)<min) min=(dm[j-1]+x_spacing);
			if ((dm[j+1]+x_spacing)<min) min=(dm[j+1]+x_spacing);
			if ((dm[j+xdim-1]+diag_spacing)<min) min=(dm[j+xdim-1]+diag_spacing);
			if ((dm[j+xdim]+y_spacing)<min) min=(dm[j+xdim]+y_spacing);
			if ((dm[j+xdim+1]+diag_spacing)<min) min=(dm[j+xdim+1]+diag_spacing);
			dm[j]=min;
		}

		// Proceeding right to left, bottom to top, assign each pixel within the ROI a brightness value one greater than the smallest value of any of its neighbors
		for(i=ind_cnt-1; i>=0; i--) {
			j=inds[i];

			min = (dm[j-xdim-1]+diag_spacing);
			if ((dm[j-xdim]+y_spacing)<min) min=(dm[j-xdim]+y_spacing);
			if ((dm[j-xdim+1]+diag_spacing)<min) min=(dm[j-xdim+1]+diag_spacing);
			if ((dm[j-1]+x_spacing)<min) min=(dm[j-1]+x_spacing);
			if ((dm[j+1]+x_spacing)<min) min=(dm[j+1]+x_spacing);
			if ((dm[j+xdim-1]+diag_spacing)<min) min=(dm[j+xdim-1]+diag_spacing);
			if ((dm[j+xdim]+y_spacing)<min) min=(dm[j+xdim]+y_spacing);
			if ((dm[j+xdim+1]+diag_spacing)<min) min=(dm[j+xdim+1]+diag_spacing);
			dm[j]=min;
		}
//if (jmark>0)
//cout << dm[jmark] << endl;

		delete [] inds;
	}

/*
unsigned short* data = mis.pixel_data(z);
for(i=0; i<np; i++) {
dm[i] = dm[i]*((float)data[i]);
}
*/

	return dm;
}


int round_float(float v)
{
	if (v<0) v-=0.5;
	else if (v>0) v+=0.5;
	return (int)v;
}


void fpoint_to_point(const FPoint& fp , Point& p)
{
	if (fp.x<0) p.x = (int)(fp.x-0.5);
	else p.x = (int)(fp.x+0.5);

	if (fp.y<0) p.y = (int)(fp.y-0.5);
	else p.y = (int)(fp.y+0.5);

	if (fp.z<0) p.x = (int)(fp.z-0.5);
	else p.z = (int)(fp.z+0.5);
}


float slice_loc(const int z, const MedicalImageSequence& mis)
{
	float sl=0;
	if (mis.zdim()>1) {
		if (z<0) {
			float zdiff = slice_loc(1, mis)-slice_loc(0, mis);
			if (zdiff<=0) {
				cerr << "ERROR: tools_miu: slice_loc: invalid slice locations for first two slices" << endl;
				exit(1);
			}
			sl = slice_loc(0, mis) + (z*zdiff);
		}
		else if (z>=mis.zdim()) {
			float zdiff = slice_loc(mis.zdim()-1, mis)-slice_loc(mis.zdim()-2, mis);
			if (zdiff<=0) {
				cerr << "ERROR: tools_miu: slice_loc: invalid slice locations for last two slices" << endl;
				exit(1);
			}
			sl = mis.slice_location(mis.zdim()-1) + (z-mis.zdim()+1)*(mis.slice_location(mis.zdim()-1)-mis.slice_location(mis.zdim()-2));
		}
		else
			sl = mis.slice_location(z);
	}
	return sl;
}

/*
float slice_loc(const int z, const MedicalImageSequence& mis)
{
	float sl=0;
	if (mis.zdim()>1) {
		if (z<0)
			sl = z*(mis.slice_location(1)-mis.slice_location(0));
		else if (z>=mis.zdim())
			sl = mis.slice_location(mis.zdim()-1) + (z-mis.zdim()+1)*(mis.slice_location(mis.zdim()-1)-mis.slice_location(mis.zdim()-2));
		else
			sl = mis.slice_location(z);
	}
	return sl;
}
*/


const int GL_to_HU(const int gl, int& hu, const MedicalImageSequence& mis)
{
	int done = 0;
	if (!strcmp(mis.modality(),"CT")) {
		if (mis.num_images()>0) {
			const Image& im = mis.image_const(0);
			done = 1;
			hu = (int)(((float)gl)*im.rescale_slope() + im.rescale_intercept());
		}
   		else {
			cerr << "ERROR: tools_miu: HU_to_GL: there are no images in the series" << endl;
			exit(1);
		}
    	}
	return done; 
}

const int HU_to_GL(const int hu, int& gl, const MedicalImageSequence& mis)
{
	int done = 0;
	if (!strcmp(mis.modality(),"CT")) {
		if (mis.num_images()>0) {
			const Image& im = mis.image_const(0);
			done = 1;
			gl = (int)(((float)hu - im.rescale_intercept())/im.rescale_slope());
		}
   		else {
			cerr << "ERROR: tools_miu: HU_to_GL: there are no images in the series" << endl;
			exit(1);
		}
    	}
	return done; 
}


const int skip_blanks(const std::string& s, int& i)
{
	while ((i<(int)s.length()) && (s[i]==' ')) i++;
	return (i<(int)s.length());
}


int advance_to(const std::string& s, const char c, int& i)
{
	while ((i<(int)s.length()) && (s[i]!=c)) i++;
	return (i<(int)s.length());
}


int read_roi_descr_genetic(const std::string& s, std::string& roi_descr, int& i, const std::string& chromosome, std::vector<bool>& bits_used)
{
    int done=0;
    std::string se_type;

	//cout << s << endl;
	//cout << s[i] << endl;
	
	done = advance_to(s, '[', i) &&	skip_blanks(s, ++i) && read_word(s, se_type, i);
	//cout << se_type << endl;
	//cout << s[i] << endl;

    if (done) {
		roi_descr.append(se_type);
		done = 0;

		if ((se_type.compare("Circle")==0) || (se_type.compare("Line_X")==0) || (se_type.compare("Line_Y")==0) || (se_type.compare("Line_Z")==0)) {
			float rad_mm;
			if (read_float(s, rad_mm, i)) {
				done = 1;
				int j = i;
				if (advance_to(s, ' ', j)) {
					i = j;
					done = read_gene_float(s, chromosome, bits_used, rad_mm, i);
				}
				std::ostringstream ss;
				ss << rad_mm;
				roi_descr.append(" ");
				roi_descr.append(ss.str());			
			}
		}
		else if (se_type.compare("Box")==0) {
			FPoint tlf, brf;
			//if (read_fpoint(_descr, tlf, i) && read_fpoint(_descr, brf, i)) {
			if (read_fpoint_genetic(s, chromosome, bits_used, tlf, i) && read_fpoint_genetic(s, chromosome, bits_used, brf, i)) {
				done = 1;
				std::ostringstream ss;
				ss << " " << tlf << " " << brf;
				roi_descr.append(ss.str());
			}
		}
		else
			cerr << "WARNING: ROIdescription: roi: don't know what a " << se_type << " is" << endl;
		
		done = done && advance_to(s, ']', i);
		i++;
	}
    return done;
}
/*

int read_roi_descr_genetic(const std::string& s, ROI& r, int& i, const MedicalImageSequence& mis, const std::string& chromosome, std::vector<bool> bits_used)
{
    int i=0, done=0;
    std::string se_type;
	
	done = advance_to(s, '[', i) &&
			skip_blanks(s, ++i) &&
			read_word(s, se_type, i);

    if (done) {
		done = 0;

		if (se_type.compare("Circle")==0) {
			float rad_mm;
			if (read_float(s, rad_mm, i) && (rad_mm>0)) {
				if (advance_to(s, ' ', i)) 
					read_gene_float(s, chromosome, bits_used, rad_mm, i);
				float rad_pix = rad_mm/mis.row_pixel_spacing(0);
				r.add_circle((int)rad_pix, 0, 0);
				done = 1;
			}
		}

		else if (se_type.compare("Line_X")==0) {
			float length_mm;
			if (read_float(s, length_mm, i) && (length_mm>0)) {
				if (advance_to(s, ' ', i)) 
					read_gene_float(s, chromosome, bits_used, length_mm, i);
				float length_pix = length_mm/mis.column_pixel_spacing(0);
				Point cent(0, 0);
				r.add_planar_rect(cent, (int)length_pix, 1);
				done = 1;
			}
		}

		else if (se_type.compare("Line_Y")==0) {
			float length_mm;
			if (read_float(s, length_mm, i) && (length_mm>0)) {
				if (advance_to(s, ' ', i)) 
					read_gene_float(s, chromosome, bits_used, length_mm, i);
				float length_pix = length_mm/mis.row_pixel_spacing(0);
				Point cent(0, 0);
				r.add_planar_rect(cent, 1, (int)length_pix);
				done = 1;
			}
		}

		else if (se_type.compare("Line_Z")==0) {
			float length_mm;
			if (read_float(s, length_mm, i) && (length_mm>0)) {
				if (advance_to(s, ' ', i)) 
					read_gene_float(s, chromosome, bits_used, length_mm, i);
				int half_length_pix=0;
				if (mis.zdim()>=2) {
					float spacing = fabs(mis.slice_location(0) - mis.slice_location(1));
					if (spacing>0)
						half_length_pix = (int)(length_mm/(2*spacing) + 0.5);
				}
				Point tl(0, 0, -half_length_pix), br(0, 0, half_length_pix);
				r.add_box(tl, br);
				done = 1;
			}
		}

		else if (se_type.compare("Box")==0) {
			FPoint tlf, brf;
			//if (read_fpoint(_descr, tlf, i) && read_fpoint(_descr, brf, i)) {
			if (read_fpoint_genetic(s, tlf, i) && read_fpoint_genetic(s, brf, i)) {
				tlf.x = tlf.x/mis.column_pixel_spacing(0);
				tlf.y = tlf.y/mis.row_pixel_spacing(0);
				brf.x = brf.x/mis.column_pixel_spacing(0);
				brf.y = brf.y/mis.row_pixel_spacing(0);

				//int half_length_pix=0;
				if (mis.zdim()>=2) {
					float spacing = fabs(mis.slice_location(0) - mis.slice_location(1));
					if (spacing>0) {
						tlf.z = tlf.z/spacing;
						brf.z = brf.z/spacing;
					}
					else {
						tlf.z = 0;
						brf.z = 0;
					}
				}
				else {
					tlf.z = 0;
					brf.z = 0;
				}

				Point tl((int)tlf.x, (int)tlf.y, (int)tlf.z), br((int)brf.x, (int)brf.y, (int)brf.z);
	//cout << "ROIdescription: " << _descr << endl;
	//cout << tl << br << endl;
				r.add_box(tl, br);
				done = 1;
			}
		}
		else
			cerr << "WARNING: ROIdescription: roi: don't know what a " << se_type << " is" << endl;
		}
		
		done = done && advance_to(s, ']', i);
		i++;
	}
    return done;
}
*/

int read_word(const std::string& s, std::string& word, int& i)
{
	int j=i, done=0;
	while ((j<(int)s.length()) && (s[j]!=' ')) j++;

	if (j>i) {
		done = 1;
		word.append(s, i, j-i);
		i = j;
		skip_blanks(s, i);
	}
	return done;
}


int read_integer(const std::string& s, int& v, int& i)
{
	int ok = ((unsigned int)i<s.length());
	if (ok) {
		char* cstr = new char [s.length() + 1];
		s.copy(cstr, s.length(), 0);
		cstr[s.length()] = 0;
		char* cptr = cstr;
		cptr += i;

		ok = sscanf(cptr, " %d", &v);
		delete [] cstr;
	}
	return ok;
}


int read_float(const std::string& s, float& f, int& i)
{
	int ok = (i<(signed int)s.length());
	if (ok) {
		char* cstr = new char [s.length() + 1];
		s.copy(cstr, s.length(), 0);
		cstr[s.length()] = 0;
		char* cptr = cstr;
		cptr += i;

		ok = sscanf(cptr, " %f", &f);
		delete [] cstr;
	}
	return ok;
}

int read_fpoint(const std::string& s, FPoint& p, int& i)
{
	p.z=0;

	int ok =skip_blanks(s, i) &&
		(s[i]=='(') &&
		(++i) &&
		read_float(s, p.x, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_float(s, p.y, i);
	if (ok) {
		int j1=i, j2=i;
		if (advance_to(s, ',', j1) && advance_to(s, ')', j2) && (j1<j2)) {
			i = j1+1;
			ok = read_float(s, p.z, i);
		}
	}
	
	return ok && advance_to(s, ')', i) && (++i); 
}

int read_fpoint_genetic(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, FPoint& p, int& i)
{
	//cout << "Entering read_fpoint_genetic" << endl;

	p.z=0;
	int j1, j2, j3;

	int ok = skip_blanks(s, i) &&
		(s[i]=='(') &&
		(++i) &&
		read_float(s, p.x, i);

	//cout << "p.x=" << p.x << endl;
	
	j1=i; j2=i;
	if (ok && advance_to(s, '{', j1) && advance_to(s, ',', j2) && (j1<j2)) {
		ok = advance_to(s, ' ', i) && read_gene_float(s, chromosome, bits_used, p.x, i);
		//cout << "p.x=" << p.x << endl;
	}

	ok = ok &&
		advance_to(s, ',', i) &&
		(++i) && skip_blanks(s, i) &&
		read_float(s, p.y, i);

	j1=i; j2=i; j3=i;
	if (ok && advance_to(s, '{', j1) && advance_to(s, ',', j2) && advance_to(s, ')', j3) && (j1<j2) && (j1<j3)) {
		ok = advance_to(s, ' ', i);
		ok = ok && read_gene_float(s, chromosome, bits_used, p.y, i);
		//cout << "p.y=" << p.y << endl;
	}

	if (ok) {
		j1=i; j2=i;
		if (advance_to(s, ',', j1) && advance_to(s, ')', j2) && (j1<j2)) {
			i = j1+1;
			ok = skip_blanks(s, i) && read_float(s, p.z, i);
			
			j1=i; j2=i;
			if (ok && advance_to(s, '{', j1) && advance_to(s, ')', j2) && (j1<j2)) {
				ok = advance_to(s, ' ', i) && read_gene_float(s, chromosome, bits_used, p.z, i);
			}
		}
	}
	//cout << p << i << endl;
	return ok && advance_to(s, ')', i) && (++i); 
}

int read_fuzzy(const std::string& s, Fuzzy& f, int& i)
{
	int ok = skip_blanks(s, i) && (s[i]=='[') && (++i);

	while (ok && s[i]!=']') {
		FPoint p;
		ok = read_fpoint(s, p, i);
		if (ok) {
			f.append(p);
			ok = skip_blanks(s, i);
		}
	}
	return ok && (s[i]==']') && (++i);
}

int read_fuzzy_genetic(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, Fuzzy& f, int& i)
{
	int ok = skip_blanks(s, i) && (s[i]=='[') && (++i);

	while (ok && s[i]!=']') {
		FPoint p;
		ok = read_fpoint_genetic(s, chromosome, bits_used, p, i);
		if (ok) {
			f.append(p);
			ok = skip_blanks(s, i);
		}
	}
	return ok && (s[i]==']') && (++i);
}

int gene_value(const std::string& chromosome, int start_bit, int stop_bit, int& value) {
//if (start_bit==17) cout << "gene_value" << endl;
//if (start_bit==17) cout << "value=" << value << endl;
	int ok = (start_bit>=0) && (start_bit<=stop_bit) && (stop_bit<chromosome.length());
	if (ok) {
		value = 0;
		for (int i=start_bit; i<=stop_bit; i++) {
			char bit_char = chromosome[i];
//if (start_bit==17) cout << "bit_char=" << bit_char << endl;
//if (start_bit==17) cout << "atoi(&bit_char)=" << atoi(&bit_char) << endl;
//if (start_bit==17) cout << "i-start_bit=" << i-start_bit << endl;
//if (start_bit==17) cout << "pow(2.0, (double)(i-start_bit))=" << pow(2.0, (double)(i-start_bit)) << endl;
			value = value + pow(2.0, (double)(i-start_bit))*((int)(bit_char-'0'));
			//value = value + pow(2.0, (double)(i-start_bit))*atoi(&bit_char);
//if (start_bit==17) cout << "value=" << value << endl;
		}
	}
	return ok;
}

int read_gene_integer(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, int& v, int& i)
{
	//cout << "Entering read_gene_integer" << endl;

	int ok = skip_blanks(s, i);
	int ok1 = 1;

	int start_bit, stop_bit, low_value, high_value, gene_val=0;
	
	if (s[i]=='{') {		
		++i;
		ok1 = read_integer(s, start_bit, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_integer(s, stop_bit, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_integer(s, low_value, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_integer(s, high_value, i) &&
		advance_to(s, '}', i);
		//skip_blanks(s, i) &&
		//s[i]=='}' &&
		ok = ok1 &&
		(++i) &&
		(low_value<=high_value) && 
		(chromosome.length() >= stop_bit) &&
		gene_value(chromosome, start_bit, stop_bit, gene_val);		

		if (ok) {
			int gene_max_value = pow(2.0, stop_bit-start_bit+1) - 1;
			//int gene_max_value = 2^(stop_bit-start_bit);
		//cout << "chromosome=" << chromosome << endl;
		//cout << "start_bit=" << start_bit << endl;
		//cout << "stop_bit=" << stop_bit << endl;
		//cout << "gene_val=" << gene_val << endl;
		//cout << "gene_max_value=" << gene_max_value << endl;
		//cout << "low_value=" << low_value << endl;
		//cout << "high_value=" << high_value << endl;

			v = round_float((float)low_value + (high_value-low_value)*(float)gene_val/gene_max_value);
		
			for (int cbi=start_bit; cbi<=stop_bit; cbi++) {
				if (bits_used[cbi]) cerr << "WARNING: Chromosome bit " << cbi << " is already used." << endl;
				bits_used[cbi] = true;
			}
		}
		else if (ok1 && (chromosome.length()==0)) { // If no chromosome is provided don�t fail and ignore the constraint, just use the default value
			ok = 1;
		}
		else {
			cerr << "WARNING: Chromosome model format incorrect or input chromosome not long enough" << endl;
		}
	
//cout << "******v=" << v << endl;
//cout << "ok=" << ok << endl;
	}
	//cout << "Exiting read_gene_integer" << endl;
	return ok;
}

int read_gene_float(const std::string& s, const std::string& chromosome, std::vector<bool>& bits_used, float& v, int& i)
{
	//cout << "Entering read_gene_float" << endl;

	int ok = skip_blanks(s, i);
	int ok1 = 1;

	int start_bit, stop_bit, gene_val=0;
	float low_value, high_value;
	
	if (s[i]=='{') {		
		++i;
		ok1 = read_integer(s, start_bit, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_integer(s, stop_bit, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_float(s, low_value, i) &&
		advance_to(s, ',', i) &&
		(++i) &&
		read_float(s, high_value, i) &&
		advance_to(s, '}', i);
		ok = ok1 &&
		(++i) &&
		(low_value<=high_value) && 
		(chromosome.length() >= stop_bit) &&
		gene_value(chromosome, start_bit, stop_bit, gene_val);		

		if (ok) {
			int gene_max_value = pow(2.0, stop_bit-start_bit+1) - 1;
		//cout << "chromosome=" << chromosome << endl;
		//cout << "start_bit=" << start_bit << endl;
		//cout << "stop_bit=" << stop_bit << endl;
		//cout << "gene_val=" << gene_val << endl;
		//cout << "gene_max_value=" << gene_max_value << endl;
		//cout << "low_value=" << low_value << endl;
		//cout << "high_value=" << high_value << endl;

			v = low_value + ((high_value-low_value)*(float)gene_val/gene_max_value);
		
			for (int cbi=start_bit; cbi<=stop_bit; cbi++) {
				if (bits_used[cbi]) cerr << "WARNING: Chromosome bit " << cbi << " is already used." << endl;
				bits_used[cbi] = true;
				//cout << "read_gene_float: Setting bit: " << cbi << endl;
			}
		}
		else if (ok1 && (chromosome.length()==0)) { // If no chromosome is provided don�t fail and ignore the constraint, just use the default value
			ok = 1;
		}
		else {
			cerr << "WARNING: Chromosome model format incorrect or input chromosome not long enough" << endl;
			cerr << "ok1: " << ok1 << " start_bit " << start_bit << " high_value " << high_value << " low_value " << low_value << endl;
			cerr << "chromosome.length(): " << chromosome.length() << endl;
		}
//cout << "******v=" << v << endl;
//cout << "ok=" << ok << endl;
	}
//	cout << "Exiting read_gene_float" << endl;
	return ok;
}

int rnd_step(const int v, const int step)
{
	int d;

	if (v>=0)
		d = ((int)(v/step))*step;
	else
		d = rnd_up_step(-1*v, step)*-1;
	return d;
}

int rnd_up_step(const int v, const int step)
{
	int d;

	if (v>=0)
		d = ((int)((v+step-1)/step))*step;
	else 
		d = rnd_step(-1*v, step)*-1;
	return d;
}


MedicalImageSequence* subsample(MedicalImageSequence& mis, const int x_step, const int y_step, const int z_step)
{
	MedicalImageSequence* ss_mis=0;

	if ((x_step>0) && (y_step>0) && (z_step>0)) {
		register int x, y, z, i, j;
		register int new_xdim=(rnd_up_step(mis.xdim(), x_step)/x_step);
		register int new_ydim=(rnd_up_step(mis.ydim(), y_step)/y_step);
		register int new_zdim=(rnd_up_step(mis.zdim(), z_step)/z_step);
		Image** ims = new Image* [new_zdim];
		j = 0;
		for(z=0; z<mis.zdim(); z+=z_step) {
			const short* const orig_pix = mis.pixel_data(z);
			short*  pix = new short [new_xdim*new_ydim];
			i = 0;
			for(y=0; y<mis.ydim(); y+=y_step)
			  for(x=0; x<mis.xdim(); x+=x_step) {
				pix[i] = orig_pix[y*mis.xdim()+x];
				i++;
			}

			ims[j] = new Image (z, new_xdim, new_ydim, mis.bits_per_pixel(), pix);
			j++;

			delete [] pix;
		}

		ss_mis = new MedicalImageSequence (new_zdim, ims);
		for(j=0; j<ss_mis->zdim(); j++) {
			ss_mis->row_pixel_spacing(j, mis.row_pixel_spacing(j*z_step)*x_step);
			ss_mis->column_pixel_spacing(j, mis.column_pixel_spacing(j*z_step)*y_step);
			ss_mis->slice_location(j, mis.slice_location(j*z_step));
		}
	}

	return ss_mis;
}

void subsample_roi(const ROI& orig_roi, ROI& new_roi, const int x_step, const int y_step, const int z_step)
{
	if ((x_step<1) || (y_step<1) || (z_step<1)) {
		cerr << "ERROR: tools_miu: subsample_roi: step parameters invalid" << endl;
		exit(1);
	}

	ROItraverser rt(orig_roi);
	TravStatus s = rt.valid();
	register Point p1, p2;
	register int new_y, new_z;
	while(s<END_ROI) {
		s = rt.current_point(p1);
		if ((p1.z%z_step)==0) {
			new_z = p1.z/z_step;
			while(s<NEW_PLANE) {
				s = rt.current_point(p1);
				if ((p1.y%y_step)==0) {
					new_y = p1.y/y_step;
					while(s<NEW_LINE) {
						rt.current_interval(p1, p2);
						new_roi.append_interval(rnd_up_step(p1.x, x_step)/x_step, rnd_step(p2.x, x_step)/x_step, new_y, new_z);
						s = rt.next_interval();
					}
				}
				else
					s = rt.next_line();
			}
		}
		else
			s = rt.next_plane();
	}
}


void expand_roi(const ROI& orig_roi, ROI& new_roi, const int x_step, const int y_step, const int z_step, const ROI& bounding_region)
{
	if ((x_step<1) || (y_step<1) || (z_step<1)) {
		cerr << "ERROR: tools_miu: expand_roi: step parameters invalid" << endl;
		exit(1);
	}

	ROItraverser rt(orig_roi);
	TravStatus s = rt.valid();
	register Point p1, p2;
	register int new_y = 0, new_z = 0;
	if (s==ROI_STAT_OK) s=NEW_PLANE;
	while(s<END_ROI) {
		rt.current_interval(p1, p2);
		if (s>=NEW_LINE)
			new_y = p1.y*y_step;
		if (s>=NEW_PLANE)
			new_z = p1.z*z_step;

		p1.x = p1.x*x_step - x_step + 1;
		p2.x = p2.x*x_step + x_step - 1;
		new_roi.append_interval(p1.x, p2.x, new_y, new_z);

		s = rt.next_interval();
	}

	int d;
	ROI new_roi_cpy(new_roi);
	for(d=(-y_step+1); d<y_step; d++)
	  if (d!=0) {
		ROI shifted_roi(new_roi_cpy);
		shifted_roi.translate(0, d, 0);
		new_roi.OR(shifted_roi);
	  }

	new_roi_cpy.clear();
	new_roi_cpy.OR(new_roi);
	for(d=(-z_step+1); d<z_step; d++)
	  if (d!=0) {
		ROI shifted_roi(new_roi_cpy);
		shifted_roi.translate(0, 0, d);
		new_roi.OR(shifted_roi);
	  }

	new_roi.AND(bounding_region);
}


float distance_mm(const Point& p1, const Point& p2, const MedicalImageSequence& mis)
{
	register float xd = (p1.x-p2.x)*mis.column_pixel_spacing(p1.z);
	register float yd = (p1.y-p2.y)*mis.row_pixel_spacing(p1.z);
	register float zd = mis.slice_location(p1.z)-mis.slice_location(p2.z);

	return sqrt(xd*xd + yd*yd + zd*zd);
}


float distance_mm(const FPoint& p1, const FPoint& p2, const MedicalImageSequence& mis)
{
	register float xd = (p1.x-p2.x)*mis.column_pixel_spacing((int)p1.z);
	register float yd = (p1.y-p2.y)*mis.row_pixel_spacing((int)p1.z);
	register float zd = fractional_slice_loc(p1.z, mis)-fractional_slice_loc(p2.z, mis);

	return sqrt(xd*xd + yd*yd + zd*zd);
}


float fractional_slice_loc(const float z, const MedicalImageSequence& mis) {
	if (z==((float)mis.zdim()-1.0)) {
		return mis.slice_location((int)z);
	}
	else {
		int iz = (int)z;
		return (mis.slice_location(iz)+ (mis.slice_location(iz+1)-mis.slice_location(iz))*(z-(float)iz));
	}
}


Point offset_mm(const Point& p, const float x_offset, const float y_offset, const float z_offset, const MedicalImageSequence& mis)
{
	Point n;
	n.x = p.x + round_float(x_offset/mis.column_pixel_spacing(0));
	n.y = p.y + round_float(y_offset/mis.row_pixel_spacing(0));
	n.z=p.z;

	if (z_offset!=0) {
		if (mis.zdim()==1) {
			cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset for a 2D image" << endl;
			exit(1);
		}

		// Determine original slice location
		float orig_slice_loc = slice_loc(p.z, mis);

		float new_z_loc = orig_slice_loc + z_offset;
		float n_loc = orig_slice_loc;
		if (new_z_loc<slice_loc(0, mis)) {
			if ((mis.zdim()<2) || (slice_loc(0, mis)==slice_loc(1, mis))) {
				cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset because first two image slices are at the same location" << endl;
				exit(1);
			}
			n.z = (int)((new_z_loc - slice_loc(0, mis))/(slice_loc(1, mis)-slice_loc(0, mis))-0.5);
		}
		else if (new_z_loc>slice_loc(mis.zdim()-1, mis)) {
			if ((mis.zdim()<2) || (slice_loc(mis.zdim()-1, mis)==slice_loc(mis.zdim()-2, mis))) {
				cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset because last two image slices are at the same location" << endl;
				exit(1);
			}
			n.z = (int)((new_z_loc - slice_loc(mis.zdim()-1, mis))/(slice_loc(mis.zdim()-1, mis)-slice_loc(mis.zdim()-2, mis))+0.5+mis.zdim()-1);
		}
		else if (n_loc>new_z_loc) {
			while (n_loc>new_z_loc) {
				n.z--;
				n_loc = slice_loc(n.z, mis);
			}
			if ((n_loc<new_z_loc) && (((new_z_loc-n_loc)/(slice_loc(n.z+1, mis)-n_loc))>0.5))
				n.z++;
		}
		else if (n_loc<new_z_loc) {
			while (n_loc<new_z_loc) {
				n.z++;
				n_loc = slice_loc(n.z, mis);
			}
			if ((n_loc>new_z_loc) && (((n_loc-new_z_loc)/(n_loc-slice_loc(n.z-1, mis)))>0.5))
				n.z--;
		}
	}
	return n;
}

/*

Point offset_mm(const Point& p, const float x_offset, const float y_offset, const float z_offset, const MedicalImageSequence& mis)
{
	Point n;
	n.x = p.x + round_float(x_offset/mis.column_pixel_spacing(0));
	n.y = p.y + round_float(y_offset/mis.row_pixel_spacing(0));
	n.z=p.z;

	if (z_offset!=0) {
		if (mis.zdim()==1) {
			cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset for a 2D image" << endl;
			exit(1);
		}

		// Determine original slice location
		float orig_slice_loc;
		if (p.z<0) {
			float zdiff = slice_loc(1, mis)-slice_loc(0, mis);
			if (zdiff<=0) {
				cerr << "ERROR: tools_miu: offset_mm: invalid slice locations for first two slices" << endl;
				exit(1);
			}
			orig_slice_loc = slice_loc(0, mis) + (p.z*zdiff);
		}
		else if (p.z>=mis.zdim()) {
			float zdiff = slice_loc(mis.zdim()-1, mis)-slice_loc(mis.zdim()-2, mis);
			if (zdiff<=0) {
				cerr << "ERROR: tools_miu: offset_mm: invalid slice locations for last two slices" << endl;
				exit(1);
			}
			orig_slice_loc = slice_loc(mis.zdim()-1, mis) + ((p.z-(mis.zdim()-1))*zdiff);
		}
		else
			orig_slice_loc = slice_loc(p.z, mis);

		float new_z_loc = orig_slice_loc + z_offset;
		float n_loc = orig_slice_loc;
		if (new_z_loc<slice_loc(0, mis)) {
			if ((mis.zdim()<2) || (slice_loc(0, mis)==slice_loc(1, mis))) {
				cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset because first two image slices are at the same location" << endl;
				exit(1);
			}
			n.z = (int)((new_z_loc - slice_loc(0, mis))/(slice_loc(1, mis)-slice_loc(0, mis))-0.5);
		}
		else if (new_z_loc>slice_loc(mis.zdim()-1, mis)) {
			if ((mis.zdim()<2) || (slice_loc(mis.zdim()-1, mis)==slice_loc(mis.zdim()-2, mis))) {
				cerr << "ERROR: tools_miu: offset_mm: cannot compute z-offset because last two image slices are at the same location" << endl;
				exit(1);
			}
			n.z = (int)((new_z_loc - slice_loc(mis.zdim()-1, mis))/(slice_loc(mis.zdim()-1, mis)-slice_loc(mis.zdim()-2, mis))+0.5+mis.zdim()-1);
		}
		else if (n_loc>new_z_loc) {
			while (n_loc>new_z_loc) {
				n.z--;
				n_loc = slice_loc(n.z, mis);
			}
			if ((n_loc<new_z_loc) && (((new_z_loc-n_loc)/(slice_loc(n.z+1, mis)-n_loc))>0.5))
				n.z++;
		}
		else if (n_loc<new_z_loc) {
			while (n_loc<new_z_loc) {
				n.z++;
				n_loc = slice_loc(n.z, mis);
			}
			if ((n_loc>new_z_loc) && (((n_loc-new_z_loc)/(n_loc-slice_loc(n.z-1, mis)))>0.5))
				n.z--;
		}
	}
	return n;
}
*/
