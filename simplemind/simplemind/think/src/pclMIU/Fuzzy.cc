#include "Fuzzy.h"

using std::cerr;
using std::endl;
using std::ios;

Fuzzy::Fuzzy()
{
}

Fuzzy::Fuzzy(const std::vector<FPoint>& v)
{
  std::vector<FPoint>::const_iterator p;
	for(p=v.begin(); p!=v.end(); ++p)
		_vert.push_back(*p);

	if (!this->valid()) {
		cerr << "ERROR: Fuzzy.cc: Fuzzy membership function invalid" << endl;
		cerr << *this << endl;
		exit(1);
	}
}

Fuzzy::Fuzzy(const float x0a, const float x1a, const float x1b, const float x0b)
{
	FPoint p(x0a, 0);
	_vert.push_back(p);
	p.x = x1a; p.y = 1.0;
	_vert.push_back(p);
	p.x = x1b;
	_vert.push_back(p);
	p.x = x0b; p.y = 0;
	_vert.push_back(p);

	if (!this->valid()) {
		cerr << "ERROR: Fuzzy.cc: Fuzzy membership function invalid" << endl;
		cerr << *this << endl;
		exit(1);
	}
}

Fuzzy::Fuzzy(const float x0a, const float x1b, const float x0c)
{
	FPoint p(x0a, 0);
	_vert.push_back(p);
	p.x = x1b; p.y = 1.0;
	_vert.push_back(p);
	p.x = x0c; p.y = 0;
	_vert.push_back(p);

	if (!this->valid()) {
		cerr << "ERROR: Fuzzy.cc: Fuzzy membership function invalid" << endl;
		cerr << *this << endl;
		exit(1);
	}
}

Fuzzy::Fuzzy(const Fuzzy& f)
{
  std::vector<FPoint>::const_iterator p;
	for(p=f._vert.begin(); p!=f._vert.end(); ++p)
		_vert.push_back(*p);
}

Fuzzy::~Fuzzy()
{
}

const int Fuzzy::valid() const
{
	int valid=0;
  std::vector<FPoint>::const_iterator p=_vert.begin();
	if (p!=_vert.end()) {
		p++;
		valid=1;
	}
	while ((p!=_vert.end())&&valid) {
		valid = (p->x>=(p-1)->x) && (p->y>=0) && (p->y<=1);
		p++;
	}
	return valid;
}

const float Fuzzy::val(const float x) const
{
  std::vector<FPoint>::const_iterator ptr=_vert.begin();
	float fv=0.0;

  //sumit - had to comment out not sure how to get a bool value
	if (!_vert.empty()) {
		while((ptr!=_vert.end())&&(ptr->x<x))
			ptr++;

		if (ptr==_vert.end())
			fv = (ptr-1)->y;

		else if (ptr->x==x) {
			float max=ptr->y;
			ptr++;
			while((ptr!=_vert.end())&&(ptr->x==x)) {
				if (ptr->y>max) max=ptr->y;
				ptr++;
			}
			fv = max;
		}
		
		else if (ptr==_vert.begin())
			fv = ptr->y;

		else
			fv = (ptr-1)->y + (ptr->y-(ptr-1)->y) *
				(x-(ptr-1)->x) / (ptr->x-(ptr-1)->x);

	}
	return fv;
}

void Fuzzy::append(const FPoint p)
{
	_vert.push_back(p);
}

void Fuzzy::scale_x(const float scaling_factor)
{
  std::vector<FPoint>::iterator ptr=_vert.begin();

	while(ptr!=_vert.end()) {
		ptr->x = ptr->x * scaling_factor;
		ptr++;
	}

	if (scaling_factor<0)
		std::reverse(_vert.begin(), _vert.end());
}

void Fuzzy::offset_x(const float offset)
{
  std::vector<FPoint>::iterator ptr=_vert.begin();

	while(ptr!=_vert.end()) {
		ptr->x = ptr->x + offset;
		ptr++;
	}
}

void Fuzzy::append_to_string(std::string& s) const
{
  std::vector<FPoint>::const_iterator p;
	s += "[";
	for(p=_vert.begin(); p!=_vert.end(); ++p)
		p->append_to_string_2d(s);
	s += "]";
}

ostream& operator<<(ostream& s, const Fuzzy& v)
{
  std::vector<FPoint>::const_iterator p;
	s << "[";
	for(p=v._vert.begin(); p!=v._vert.end(); ++p)
		s << *p;
	s << "]";
	return s;
}

istream& operator>>(istream& s, Fuzzy& f)
{
	char c;
	int ok=0;
	FPoint p;

	s >> c;
	if (c=='[') {
		ok = 1;
		c = s.peek();
		while (s.good() && (c!=']')) {
			s >> p;
			f.append(p);
		}
	}
	ok = ok && (c==']');

	if (!ok) s.clear(ios::badbit);
	return s;
}


/***************************/
/****	mid_fuzzy	****/
/***************************
	Returns average of x-coordinates of first point with first non-zero
		y-value and first subsequent point with y-value equal to zero
	If the second point can't be found just return x-coordinate of first
		point
	If the first point can't be found then return NULL
****************************/
/*
int * mid_fuzzy(fuzzy_t f)
{
	int i, lo, hi, done=0, stop=0;
	ipoint_t *ptr;
	int *mp=NULL;

	ptr = (ipoint_t *) getfirst_address(f);
	i = 0;
	while((i<f->N)&&(!done)) {
		if (ptr[i].y>0) {
			lo = hi = ptr[i].x;
			done = 1;
		}
		i++;
	}
	while((i<f->N)&&(!stop)) {
		if (ptr[i].y==0) {
			hi = ptr[i-1].x;
			stop = 1;
		}
		i++;
	}
	if (done) {
		mp = (int *) calloc(1, sizeof(int));
		*mp = (lo+hi)/2;
	}
	return mp;
}
*/

/***************************/
/****	first_non_zero	****/
/***************************
	Tries to set x parameter to x-coordinate of first fuzzy point with 
		non-zero y-value, excluding the very first point
	Returns 1 if successful
	Returns 0 if could not find such a point
	Returns -1 if first point has non-zero y-value
****************************/
/*
int first_non_zero(fuzzy_t f, int *x)
{
	int i, done=0;
	ipoint_t *ptr;

	if (f->N>0) {
		ptr = (ipoint_t *) getfirst_address(f);
		if (ptr->y!=0)
			done = -1;
		i = 1;
		while((i<f->N)&&(done==0)) {
			if (ptr[i].y>0) {
				*x = ptr[i].x;
				done = 1;
			}
			i++;
		}
	}
	return done;
}
*/

/***************************/
/****	last_non_zero	****/
/***************************
	Similar to first_non_zero except looks for last point with non-zero 
		y-value, excluding the very last point
	Returns 1 if successful
	Returns 0 if could not find such a point
	Returns -1 if last point has non-zero y-value
****************************/
/*
int last_non_zero(fuzzy_t f, int *x)
{
	int i, done=0;
	ipoint_t *ptr;

	if (f->N>0) {
		ptr = (ipoint_t *) getfirst_address(f);
		i = f->N-1;
		if (ptr[i].y!=0)
			done = -1;
		i--;
		while((i>=0)&&(done==0)) {
			if (ptr[i].y>0) {
				*x = ptr[i].x;
				done = 1;
			}
			i--;
		}
	}
	return done;
}
*/
