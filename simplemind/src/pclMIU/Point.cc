#include "Point.h"


Point::Point(const int xv, const int yv, const int zv)
	: x(xv), y(yv), z(zv)
{
}


Point::Point(const Point& p)
	: x(p.x), y(p.y), z(p.z)
{
}

Point::~Point()
{
}


void Point::operator=(const Point& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
}

void Point::set(const int xv, const int yv, const int zv)
{
	x = xv;
	y = yv;
	z = zv;
}

ostream& operator<<(ostream& s, const Point& p)
{
	return s << "(" << p.x << ", " << p.y << ", " << p.z << ")";
}

istream& operator>>(istream& s, Point& p)
{
	char c;
	int ok=0;

	s >> c;
	if (c=='(') {
		s >> p.x;
		s >> c;
		if (c == ',') {
			s >> p.y;
			s >> c;
			if (c == ',') {
				s >> p.z;
				s >> c;
				ok = (c == ')');
			}
		}
	}

	if (!ok) s.clear(ios::badbit);

	return s;
}


int point_comp_xy(const Point &p1, const Point &p2)
{
	int d=p1.y-p2.y;

	if (d==0)
		d=p1.x-p2.x;

	return d;
}

