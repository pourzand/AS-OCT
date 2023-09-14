#include "FPoint.h"
#include <stdio.h>

using std::ios;


FPoint::FPoint(const float xv, const float yv, const float zv)
	: x(xv), y(yv), z(zv)
{
}


FPoint::FPoint(const FPoint& p)
	: x(p.x), y(p.y), z(p.z)
{
}

FPoint::~FPoint()
{
}


void FPoint::operator=(const FPoint& p)
{
	x = p.x;
	y = p.y;
	z = p.z;
}

float FPoint::distance(const FPoint& p)
{
	return sqrt((x-p.x)*(x-p.x) + (y-p.y)*(y-p.y) + (z-p.z)*(z-p.z));
}


void FPoint::append_to_string_2d(std::string& s) const
{
	char v[10];
	s += "(";
	sprintf(v, "%.5f", x);
	s += v;
	s += ", ";
	sprintf(v, "%.5f", y);
	s += v;
	s += ")";
}


ostream& operator<<(ostream& s, const FPoint& p)
{
	return s << "(" << p.x << ", " << p.y << ", " << p.z << ")";
}

istream& operator>>(istream& s, FPoint& p)
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

