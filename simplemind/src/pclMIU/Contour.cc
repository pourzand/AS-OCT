#include "Contour.h"

const int Contour::n() const
{
	return _pts.N();
}

const Point& Contour::operator[](long i) const
{
	if ((i<0) || (i>=_pts.N())) {
		cerr << "ERROR: Contour::operator[]\n";
		exit(1);
	}
	return _pts[i];
}


Contour::Contour()
	: _pts(100)
{
}


Contour::Contour(const Darray<Point>& dav)
	: _pts(dav)
{
}

	
Contour::Contour(const Contour& ba)
	: _pts(ba._pts)
{
}


Contour::~Contour()
{
}


void Contour::append(const Point& p)
{ 
	_pts.push_last(p);
}


void Contour::append(const Contour& p)
{
	int i;

	for(i=0; i<p._pts.N(); i++)
		append(p._pts[i]);
}


void Contour::prepend(const Point& p)
{ 
	_pts.push_first(p);
}


const Darray<Point>& Contour::points() const
{
	return _pts;
}


void Contour::clear()
{
	_pts.clear();
}


const int Contour::remove(const int i)
{
	int done=0;

	if ((i>=0)&&(i<_pts.N())) {
		_pts.delete_item(i);
		done=1;
	}
	return done;
}



const int Contour::replace(const int i, const Point p)
{
	int done=0;

	if ((i>=0)&&(i<_pts.N())) {
		_pts(i).x=p.x;
		_pts(i).y=p.y;
		done=1;
	}
	return done;
}


void Contour::reverse()
{
	Darray<Point> cpy(_pts);
	_pts.clear();
	for(int i=cpy.N()-1; i>=0; i--)
		_pts.push_last(cpy[i]);
}


void Contour::make_continuous()
{
	int i, flag=0;

	void insert_pts(Darray<Point>&, Point, Point, int&);

	flag = ((abs(_pts[0].x - _pts[_pts.N()-1].x)>1) || (abs(_pts[0].y - _pts[_pts.N()-1].y)>1));

	i=0;
	while(i<(_pts.N()-1)) {
		if ((abs(_pts[i].x - _pts[i+1].x)>1) || (abs(_pts[i].y - _pts[i+1].y)>1)) {
            i++;
			insert_pts(_pts, _pts[i], _pts[i+1], i);
		}
		else
			i++;
	}

	if (flag) {
        i++;
		insert_pts(_pts, _pts[_pts.N()-1], _pts[0], i);
    }
}


ostream& operator<<(ostream& s, const Contour& c)
{
	int i;
	
	for(i=0; i<c.n(); i++)
		s << c[i] << " ";

	return s;
}


