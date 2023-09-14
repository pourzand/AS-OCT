#include "Line.h"


void Line::add_ivl(const Interval &iv)
{
	Interval *ip;//, *ip2;
	unsigned int i;
	int touch=-1;

	for(i=0; (i<ivl.size()) && (touch<0); i++)
		touch = ivl_comp_touch(ivl[i], iv);
	
	if (touch==0) {
		i--;
		ip = &(ivl[i]);
		if (iv.x1<ip->x1) {
			ip->x1=iv.x1;
			while((i>0) && (ivl[i-1].x1>=ip->x1)) {
				ivl.erase(ivl.begin()+i-1);
				i--;
				ip = &(ivl[i]);
			}
			if ((i>0) && (ivl[i-1].x2>=(ip->x1-1))) {
				ip->x1=ivl[i-1].x1;
				ivl.erase(ivl.begin()+i-1);
				i--;
				ip = &(ivl[i]);
			}
		}
		if (iv.x2>ip->x2) {
			ip->x2=iv.x2;
			while(((i+1)<ivl.size()) && (ivl[i+1].x2<=ip->x2)) {
				ivl.erase(ivl.begin()+i+1);
				ip = &(ivl[i]);
			}
			if (((i+1)<ivl.size()) && (ivl[i+1].x1<=(ip->x2+1))) {
				ip->x2=ivl[i+1].x2;
				ivl.erase(ivl.begin()+i+1);
				ip = &(ivl[i]);
			}
		}
	}

	else if (touch>0) {
//cout << "here: " << i << endl;
//cout << ivl.size() << endl;
//cout << iv.x1 << endl;
		ivl.insert(ivl.begin()+i-1, iv);
	}

	else
		ivl.insert(ivl.begin()+i, iv);
}


void Line::subtract_ivl(const Interval &iv)
{
	long j, low, low_cov, low_int, up_cov, up_int, overlap=-1;
  unsigned long i, up;
	/// _cov => complete covers, _int => intersects
	
	for(i=0; (i<ivl.size()) && (overlap<0); i++)
		overlap = ivl_comp_overlap(ivl[i], iv);
//cout << "overlap: " << overlap << "  " << i << endl;	
	if (overlap==0) {
		i--;
		low = up = i;
		while((low>=0)&&(!ivl_comp_overlap(ivl[low], iv)))
			low--;
		low++;
		while((up<ivl.size())&&(ivl_comp_overlap(ivl[up], iv)==0))
			up++;
		up--;

		if (iv.x1<=ivl[low].x1) {
			low_cov = low;
			low_int = -1;
		}
		else {
			low_cov = low+1;
			low_int = low;
		}
		if (iv.x2>=ivl[up].x2) {
			up_cov = up;
			up_int = -1;
		}
		else {
			up_cov = up-1;
			up_int = up;
		}
//cout << low_int << "  " << low_cov << "  " << up_cov << "  " << up_int << endl << flush;

		for(j=0; j<(up_cov-low_cov+1); j++) {
			ivl.erase(ivl.begin()+low_cov);
			up_int--;
		}


		Interval li, ui;
		if (low_int>=0) {
			li.x1=ivl[low_int].x1;
			/// *** changed on 8-14-98 *** li.x2=iv.x1;
			li.x2=iv.x1-1;
		}
		if (up_int>=0) {
			/// *** changed on 8-14-98 *** ui.x1=iv.x2;
			ui.x1=iv.x2+1;
			ui.x2=ivl[up_int].x2;
		}

		if (low_int>=0) {
			ivl.erase(ivl.begin()+low_int);
			ivl.insert(ivl.begin()+low_int, li);
		}
		if (up_int>=0) {
			if (up_int!=low_int)
				ivl.erase(ivl.begin()+up_int);
			else
				up_int++;
			ivl.insert(ivl.begin()+up_int, ui);
		}
	}
}


ostream& operator<<(ostream& s, const Line& l)
{
	unsigned int i;
	
	s << "y=" << l.y << ":";
	for(i=0; i<l.ivl.size(); i++)
		s << "  " << l.ivl[i].x1 << "-" << l.ivl[i].x2;
	s << "\n";

	return s;
}

