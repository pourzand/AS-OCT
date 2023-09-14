#include "ROItraverser.h"


ROItraverser::ROItraverser(const ROI& r)
	: ROI(r), _wkspace()
{
}


ROItraverser::ROItraverser(const ROI& r, const int z)
	: ROI(r, z), _wkspace()
{
}


void ROItraverser::reinitialize(const ROI& r)
{
	this->clear();
	this->OR(r);
	_wkspace.zero();
}


const TravStatus ROItraverser::valid() const
{
	return _valid_wkspace(_wkspace);
}


const TravStatus ROItraverser::current_point(Point& p) const
{
	TravStatus s = valid();

	if (s<NOT_FOUND) {
		p.z = _plane_const(_wkspace).z;
		p.y =_line_const(_wkspace).y;
		p.x = _interval_const(_wkspace).x1+_wkspace.xi;
	}

	return s;
}


const TravStatus ROItraverser::current_interval(Point& p1, Point& p2) const
{
	TravStatus s = valid();

	if (s<NOT_FOUND) {
		p1.z = p2.z = _plane_const(_wkspace).z;
		p1.y = p2.y = _line_const(_wkspace).y;
		p1.x = _interval_const(_wkspace).x1;
		p2.x = _interval_const(_wkspace).x2;
	}

	return s;
}


const TravStatus ROItraverser::next_point()
{
	TravStatus s = valid();

	if (s<NOT_FOUND)
		s = _next_x(_wkspace);

	return s;
}


const TravStatus ROItraverser::last_point_in_plane()
{
	TravStatus s = valid();

	if (s<NOT_FOUND) {
		s = NEW_INTERVAL;
		_end_of_plane(_wkspace);
	}

	return s;
}


const TravStatus ROItraverser::next_interval()
{
	TravStatus s = valid();

	if (s<NOT_FOUND)
		s = _next_interval(_wkspace);

	return s;
}

const TravStatus ROItraverser::next_line()
{
	TravStatus s = valid();

	if (s<NOT_FOUND)
		s = _next_line(_wkspace);

	return s;
}


const TravStatus ROItraverser::next_plane()
{
	TravStatus s = valid();

	if (s<NOT_FOUND)
		s = _next_plane(_wkspace);

	return s;
}


const TravStatus ROItraverser::set_plane(const int z)
{
	TravStatus s = _set_plane(_wkspace, z);
	if (s==NEW_PLANE) {
		_first_line(_wkspace);
		_first_interval(_wkspace);
		_first_x(_wkspace);
	}

	return s;
}



const TravStatus ROItraverser::status() const
{
	if (valid()>=END_ROI)
		return NOT_FOUND;
	else {
		TravStatus ts = ROI_STAT_OK;
		if (_wkspace.xi==0) {
			ts = NEW_INTERVAL;
			if (_wkspace.ii==0) {
				ts = NEW_LINE;
				if (_wkspace.li==0)
					ts = NEW_PLANE;
			}
		}
		return ts;
	}
}


const TravStatus ROItraverser::reset()
{
	_wkspace.zero();

	return valid();
}


const int ROItraverser::in_roi(const Point& p) const
{
	return ROI::in_roi(p);
}


const int ROItraverser::num_pix() const
{
	return ROI::num_pix();
}


const int ROItraverser::num_pix(const int z) const
{
	return ROI::num_pix(z);
}


Contour* ROItraverser::boundaries(int& n, const int zv) const
{
	return ROI::boundaries(n, zv);
}


const int ROItraverser::bounding_box(Point& ul, Point& br, const int z) const
{
	return ROI::bounding_box(ul, br, z);
}


const int ROItraverser::bounding_cube(Point& tl, Point& br) const
{
	return ROI::bounding_cube(tl, br);
}
