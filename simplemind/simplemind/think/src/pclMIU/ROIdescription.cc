#include "ROIdescription.h"


ROIdescription::ROIdescription(const std::string& descr)
	: _descr(descr)
{
}


ROIdescription::ROIdescription(const ROIdescription& r)
	: _descr(r._descr)
{
}


ROIdescription::~ROIdescription()
{
}


const std::string& ROIdescription::description() const
{
	return _descr;
}


const int ROIdescription::roi(ROI& r, const MedicalImageSequence& mis) const
{
    int i=0, done=0;
    std::string se_type;

    if (read_word(_descr, se_type, i)) {
	if (se_type.compare("Circle")==0) {
		float rad_mm;
		if (read_float(_descr, rad_mm, i) && (rad_mm>0)) {
			float rad_pix = rad_mm/mis.row_pixel_spacing(0);
			r.add_circle((int)rad_pix, 0, 0);
			done = 1;
		}
	}
	else if (se_type.compare("Line_X")==0) {
		float length_mm;
		if (read_float(_descr, length_mm, i) && (length_mm>0)) {
			float length_pix = length_mm/mis.column_pixel_spacing(0);
			Point cent(0, 0);
			r.add_planar_rect(cent, (int)length_pix, 1);
			done = 1;
		}
	}
	else if (se_type.compare("Line_Y")==0) {
		float length_mm;
		if (read_float(_descr, length_mm, i) && (length_mm>0)) {
			float length_pix = length_mm/mis.row_pixel_spacing(0);
			Point cent(0, 0);
			r.add_planar_rect(cent, 1, (int)length_pix);
			done = 1;
		}
	}
	else if (se_type.compare("Line_Z")==0) {
		float length_mm;
		if (read_float(_descr, length_mm, i) && (length_mm>0)) {
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
		if (read_fpoint(_descr, tlf, i) && read_fpoint(_descr, brf, i)) {
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

    return done;
}


ostream& operator<<(ostream& s, const ROIdescription& r)
{
	s << r._descr;
	return s;
}
