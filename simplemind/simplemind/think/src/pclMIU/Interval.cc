#include "Interval.h"

Interval::Interval(const Interval& i)
	: x1(i.x1), x2(i.x2)
{
}


int ivl_comp_touch(const Interval& i1, const Interval& i2)
{
	int d;

	d = i1.x2-i2.x1;
	if (d>=-1) {
		d = i1.x1-i2.x2;
		if (d<=1) d = 0;
	}
	return d;
}


int ivl_comp_overlap(const Interval& i1, const Interval& i2)
{
	int d;

	d = i1.x2-i2.x1;
	if (d>0) {
		d = i1.x1-i2.x2;
		if (d<0) d=0;
	}
	return d;
}
