#include "ROI.h"
#include "tools_miu.h"


//ofstream& operator<<(ofstream& file, const ROI& r) {
ostream& operator<<(ostream& file, const ROI& r) {
	assert(file != 0);

	file << r._pl.N() << " "; 
	int i, j;
  unsigned int k;
	for(i=0; i<r._pl.N(); i++) {
		file << r._pl[i].z << " ";
		const Darray<Line>& lp = r._pl[i].ln;
		file << lp.N() << " ";
		for(j=0; j<lp.N(); j++) {
			file << lp[j].y << " ";
			const std::vector<Interval> ip = lp[j].ivl;
			file << ip.size() << " ";
			for(k=0; k<ip.size(); k++)
				file << ip[k].x1 << " " << ip[k].x2 << " ";
		}
	}

	file << endl;

  	return file;
}


istream& operator>>(istream& file, ROI& r) {
    assert(file != 0);

  	//Get rid of old ROI
  	r.clear();

	int i, j, k;
	int np, nl, ni;
	int x1, x2, y, z;  
  	file >> np;
	for(i=0; i<np; i++) {
		file >> z;
		Plane pt(z, r._ln_mod, r._ivl_mod);

		file >> nl;
		for(j=0; j<nl; j++) {
			file >> y;
			Line lt(y);

			file >> ni;
			for(k=0; k<ni; k++) {
				file >> x1 >> x2;
				Interval it(x1, x2);
				lt.ivl.push_back(it);
			}

			pt.ln.push_last(lt);
		}

		r._pl.push_last(pt);
	}

  	return file;
}


ROI::ROI()
	: _pl(10), _ln_mod(64), _ivl_mod(2)
{
}

ROI::ROI(const long plane_mod_fact, const long line_mod_fact, const long interval_mod_fact)
	: _pl(plane_mod_fact), _ln_mod(line_mod_fact), _ivl_mod(interval_mod_fact)
{
}

/*
ROI::ROI(const ROI &r)
	: _ln_mod(r._ln_mod), _ivl_mod(r._ivl_mod), _pl(r._pl)
{	
}
*/

ROI::ROI(const ROI &b)
	: _pl((b._pl.N()>0)?b._pl.N():1), 
	_ln_mod((b._pl.N()>0)?b._pl[b._pl.N()/2].ln.N()/2+1:b._ln_mod), 
	_ivl_mod(b._ivl_mod)
{
	ROIworkspace wb;
	WkspaceStatus s;
	Plane *pp = NULL;
	Line *lp = NULL;

	s = b._plane_status(wb);
	if (s==ROI_STAT_OK)
		s = NEW_PLANE;

	while(s<END_ROI) {
		if (s>=NEW_PLANE) {
			Plane p(b._plane_const(wb).z, _ln_mod, _ivl_mod);
			_pl.push_last(p);
			pp = &_pl(_pl.N()-1);
		}
		if (s>=NEW_LINE) {
			Line l(b._line_const(wb).y);
			pp->ln.push_last(l);
			lp = &(pp->ln(pp->ln.N()-1));
		}
		Interval i(b._interval_const(wb).x1, b._interval_const(wb).x2);
		lp->ivl.push_back(i);

		s=b._next_interval(wb);		
	}
}


ROI::ROI(const ROI& r, const int z)
	: _pl(1), _ln_mod(r._ln_mod), _ivl_mod(r._ivl_mod)
{
	ROIworkspace w;
	WkspaceStatus s = r._set_plane(w, z);
	if (s<NOT_FOUND) {
		Plane newp(r._plane_const(w));
		_pl.push_last(newp);
	}
}

/*
ROI::ROI(const ROI& r, const int z, const int dummy)
	: _ln_mod(r._ln_mod), _ivl_mod(r._ivl_mod), _pl(1)
{
	ROIworkspace w;
	WkspaceStatus s = r._set_plane(w, z);
	if (s<NOT_FOUND) {
//cout << r._plane(w) << endl;
		Plane newp(r._plane(w), 2);
//cout << newp << endl << endl << endl;
//		_pl.push_last(newp);
	}
}
*/

void ROI::add_point(const Point& p)
{
	Plane pla(p.z, _ln_mod, _ivl_mod);
	Line lne(p.y);

	Plane& pp = _pl(_pl.find_or_add(pla, plane_comp));
	Line& lp = pp.ln(pp.ln.find_or_add(lne, line_comp));

	lp.add_ivl(p.x, p.x);
}


void ROI::add_box(const Point& tl, const Point& br)
{
	ROI box;
	int y, z;
	Interval it(tl.x, br.x);
	Line lt(0);
	Plane pt(0, _ln_mod, _ivl_mod);
	
	if ((tl.x>br.x) || (tl.y>br.y) || (tl.z>br.z)) {
		cerr << "Error: ROI::add_box\n";
		exit(1);
	}
	
	lt.ivl.push_back(it);

	for (y=tl.y; y<=br.y; y++) {
		lt.y = y;
		pt.ln.push_last(lt);
	}
	
	for(z=tl.z; z<=br.z; z++) {
		pt.z = z;
		box._pl.push_last(pt);
	}
		
	OR(box);
}


void ROI::add_planar_rect(const Point &center, const int x_length, const int y_length)
{
	if ((x_length<=0)||(y_length<=0)) {
		cerr << "Error: ROI::add_planar_rect\n";
		exit(1);
	}

	Point tl(center.x-x_length/2, center.y-y_length/2, center.z);
	Point br(tl.x+x_length-1, tl.y+y_length-1, tl.z);
	
	add_box(tl, br);
}

void ROI::add_contig(ROI& r, const Point &start, const int sub)
{
	// from start voxel get start interval
	ROIworkspace wr;
	WkspaceStatus sr;
	sr = r._set_plane(wr, start.z);
	if (sr<NOT_FOUND)
		sr = r._set_line(wr, start.y);
	if (sr<NOT_FOUND) {
		sr = r._set_interval(wr, start.x);
	}
	
	if (sr<NOT_FOUND) {
		ROI* rc;
		if (sub) {
			rc = &r;
		}
		else {
			rc = new ROI (r, start.z);
			wr.pi=0;
		}

		ROI to_check;
		to_check.add_interval(rc->_interval(wr).x1, rc->_interval(wr).x2, rc->_line(wr).y, rc->_plane(wr).z);
		rc->subtract(to_check);

		// while to_check is not empty, pop the first interval
		int xi1, xi2, yi, zi;

		while(to_check.pop_first_interval(xi1, xi2, yi, zi)) {
			add_interval(xi1, xi2, yi, zi);
			//cout << xi1 << "  " << xi2 << "  " << yi << "  " << zi << endl << flush;

			// add adjacent intervals to to_check
			to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi, *rc);
		}
		if (!sub) {
			delete rc;
		}
	}
}

/*
void ROI::add_interval_temp(const int x1, const int x2, const int y, const int z)
{
	if (x1<=x2) {
		ROIworkspace w;
		WkspaceStatus s;
		
		s = _set_plane(w, z);
		if (s>=NOT_FOUND) {
			Plane pt(z, _ln_mod, _ivl_mod);
			Line lt(y);
			Interval it(x1, x2);
			lt.ivl.push_back(it);
			pt.ln.push_last(lt);
			_pl.push_inorder_temp(pt, plane_comp);
		}
		else {			
			s = _set_line(w, y);
			if (s>=NOT_FOUND) {
				Line lt(y);
				Interval it(x1, x2);
				lt.ivl.push_back(it);
				_pl(w.pi).ln.push_inorder_temp(lt, line_comp);
			}
			else
				_line(w).add_ivl(x1, x2);
		}
	}
}
*/

void ROI::add_contig_3d(ROI& r, const Point &start, const int sub)
{	
//Point p;
//r.first_point(p);	
//int db = (p.x==168)&&(p.y==147)&&(p.z==1);

	// from start voxel get start interval
	ROIworkspace wr;
	WkspaceStatus sr;
	sr = r._set_plane(wr, start.z);
	if (sr<NOT_FOUND)
		sr = r._set_line(wr, start.y);
	if (sr<NOT_FOUND)
		sr = r._set_interval(wr, start.x);
	
	if (sr<NOT_FOUND) {
		ROI* rc;
		if (sub) {
			rc = &r;
		}
		else {
			rc = new ROI (r);
		}

		ROI to_check;
		to_check.add_interval(rc->_interval(wr).x1, rc->_interval(wr).x2, rc->_line(wr).y, rc->_plane(wr).z);
		rc->subtract(to_check);

		// while to_check is not empty, pop the first interval
		int xi1, xi2, yi, zi;
		while(to_check.pop_first_interval(xi1, xi2, yi, zi)) {
			/*
			if (((xi1==170)&&(xi2==171)&&(yi==145)&&(zi==2)) || ((xi1==163)&&(xi2==171)&&(yi==146)&&(zi==2))) {
				std::cout << std::endl << std::endl << std::endl;
				print_all_points();
				std::cout << "*************" << xi1 << ", " << xi2 << ", " << yi << ", " << zi << "*************" << std::endl;
			
				add_interval_temp(xi1, xi2, yi, zi);

				std::cout << std::endl;
				print_all_points();
			}
			else
				add_interval(xi1, xi2, yi, zi);
			*/
			
			add_interval(xi1, xi2, yi, zi);

			// add adjacent intervals to to_check
			to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi-1, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi, zi-1, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi-1, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi+1, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi, zi+1, *rc);
			to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi+1, *rc);
		}

		if (!sub) {
			delete rc;
		}
	}
}

/*
void ROI::add_contig_3d(const ROI& r, const Point &start)
{	
	// *** This line added by Matt on 7/26/02 ***
	ROI rc(r);

	// from start voxel get start interval
	ROIworkspace wr;
	WkspaceStatus sr;
	sr = rc._set_plane(wr, start.z);
	if (sr<NOT_FOUND)
		sr = rc._set_line(wr, start.y);
	if (sr<NOT_FOUND)
		sr = rc._set_interval(wr, start.x);
	

	ROI to_check;
	if (sr<NOT_FOUND) {
		to_check.add_interval(rc._interval(wr).x1, rc._interval(wr).x2, rc._line(wr).y, rc._plane(wr).z);
	}

	// while to_check is not empty, pop the first interval
	int xi1, xi2, yi, zi;
	while(to_check.pop_first_interval(xi1, xi2, yi, zi)) {
		add_interval(xi1, xi2, yi, zi);
		//cout << xi1 << "  " << xi2 << "  " << yi << "  " << zi << endl << flush;

		// add adjacent intervals to to_check
		to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi-1, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi, zi-1, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi-1, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi-1, zi+1, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi, zi+1, rc);
		to_check._add_overlap_interval(xi1-1, xi2+1, yi+1, zi+1, rc);
	}
}
*/

struct image_t {
	int xdim, ydim;
	int *data;
};


/***************************/
/****	create_im	****/
/***************************/

image_t create_im(int xd, int yd)
{
	image_t im;

	im.xdim=xd; im.ydim=yd;
	im.data = (int *) calloc(im.xdim*im.ydim, sizeof(int));

	return im;
}

/**************************/
/****	pts_to_bim	***/
/**************************
	Sets the pixel values of bim to 1 at the coordinates given by the Points
***************************/

void pts_to_bim(Darray<Point> &pts, image_t bim)
{
	int i;

	for(i=0; i<pts.N(); i++)
		bim.data[pts[i].y*bim.xdim+pts[i].x] = 1;
}

/**********************************/
/****	get_connected		***/
/**********************************
	Uses 4-connection
***********************************/
void get_connected(Darray<Point> &conn, image_t &im, const Point &start)
{
	Darray<Point> to_check(100); 
	Point cp, np;
	const Point* pp;
	int i, j;//, k;
	int *val;

	to_check.push_last(start);
	im.data[start.y*im.xdim+start.x] = 1;

	while((to_check.N()>0)) {
		pp = &to_check[to_check.N()-1];
		cp.x = pp->x; cp.y = pp->y;
		conn.push_last(*pp);
		to_check.delete_item(to_check.N()-1);

		np.y = cp.y;
		for(i=(cp.x-1); i<=(cp.x+1); i+=2)
			if ((i>=0)&&(i<im.xdim)) {
				np.x = i;
				val = &im.data[np.y*im.xdim+np.x];
				if (!(*val)) {
					*val = 1;
					to_check.push_last(np);
				}
			}

		np.x = cp.x;		 
		for(j=(cp.y-1); j<=(cp.y+1); j+=2)
			if ((j>=0)&&(j<im.ydim)) {
				np.y = j;
				val = &im.data[np.y*im.xdim+np.x];
				if (!(*val)) {
					*val = 1;
					to_check.push_last(np);
				}
			}
	}
}


void insert_pts(Darray<Point>& c, Point p1, Point p2, int &j)
{
//cout << "insert " << p1 << p2 << endl; cout.flush();
	int inc, x, y;
  //int x1, x2, y1, y2;
	float grad;
	Point tp;

	//LINE BELOW ADDED ON 110717
	j++;
	tp.z = p1.z;

	if (abs(p1.x-p2.x)>abs(p1.y-p2.y)) {
		if (p1.x<p2.x)
			inc = 1;
		else
			inc = -1;
		grad = (float)(p2.y-p1.y)/(float)(p2.x-p1.x);
		for(x=p1.x+inc; x!=p2.x; x+=inc) {
			tp.x = x;
			tp.y = (int)(p1.y + grad*(x-p1.x));
			c.push_here(tp, j);
			j++;
		}
	}
	else {
		if (p1.y<p2.y)
			inc = 1;
		else
			inc = -1;
		grad = (float)(p2.x-p1.x)/(float)(p2.y-p1.y);
		for(y=p1.y+inc; y!=p2.y; y+=inc) {
			tp.y = y;
			tp.x = (int)(p1.x + grad*(y-p1.y));
			c.push_here(tp, j);
			j++;
		}
	}
//cout << "done insert" << endl; cout.flush();
}

void make_continuous_2D(Darray<Point>& oc)
{
	int i, flag=0;

	void insert_pts(Darray<Point>&, Point, Point, int&);

	flag = ((abs(oc[0].x - oc[oc.N()-1].x)>1) || (abs(oc[0].y - oc[oc.N()-1].y)>1));
//cout << "flag=" << flag << endl; cout.flush();
	i=0;
	while(i<(oc.N()-1)) {
		if ((abs(oc[i].x - oc[i+1].x)>1) || (abs(oc[i].y - oc[i+1].y)>1)) {
            //LINE BELOW COMMENTED OUT ON 110717
	//i++;
//cout << "i=" << i << ", oc.N=" << oc.N() << endl; cout.flush();
			insert_pts(oc, oc[i], oc[i+1], i);
		}
		else
			i++;
	}

	if (flag) {
//cout << "entering flag" << endl; cout.flush();
            i++;
		insert_pts(oc, oc[oc.N()-1], oc[0], i);
    }
}


ROI::ROI(Darray<Point>& c)
	: _pl(1)
{
//cout << "Roi a" << endl; cout.flush();
	if (c.N() == 0 ) {
		return;
	}
	
	int n;
	image_t im;
	int xmin=10000, xmax=-10000, ymin=10000, ymax=-10000;
	int i, j, k, dbc;
	Point start;
	Darray<Point> reg(100);
	Interval ivl;

	image_t create_im(int, int);
	void pts_to_bim(Darray<Point>&, image_t);
	void get_connected(Darray<Point>&, image_t&, const Point&);
//cout << "Roi b" << endl; cout.flush();

	make_continuous_2D(c);
//cout << "Roi c" << endl; cout.flush();
	n = c.N();

	for(i=0; i<n; i++) {
		if (c[i].x<xmin) xmin=c[i].x;
		if (c[i].x>xmax) xmax=c[i].x;
		if (c[i].y<ymin) ymin=c[i].y;
		if (c[i].y>ymax) ymax=c[i].y;
	}

	int x_offset=0, y_offset=0;
	if (xmin<1)
		x_offset = 1 - xmin;
	if (ymin<1)
		y_offset = 1 - ymin;
	if ((x_offset>0) || (y_offset>0)) {
		for(i=0; i<n; i++) {
			c(i).x += x_offset;
			c(i).y += y_offset;
		}
		xmin += x_offset;
		xmax += x_offset;
		ymin += y_offset;
		ymax += y_offset;
	}
//cout << "Roi c" << endl << cout.flush();

	im = create_im(xmax+1, ymax+1);
	pts_to_bim(c, im);

	if (n>2)
	for(i=0; i<n; i++) {
		int start_found=0;
		if (i==0) {
			start_found = ((c[1].y<c[0].y) && (c[n-1].y>=c[0].y));
			// LINE BELOW ADDED ON 4/30/03
			start_found = start_found || ((c[1].y<=c[0].y) && (c[n-1].y>c[0].y));
		} else if (i==(n-1)) {
			start_found = ((c[0].y<c[n-1].y) && (c[n-2].y>=c[n-1].y));
			// LINE BELOW ADDED ON 4/30/03
			start_found = start_found || ((c[0].y<=c[n-1].y) && (c[n-2].y>c[n-1].y));
		}
		else {
			start_found = ((c[i+1].y<c[i].y) && (c[i-1].y>=c[i].y));
			// LINE BELOW ADDED ON 4/30/03
			start_found = start_found || ((c[i+1].y<=c[i].y) && (c[i-1].y>c[i].y));
		}

		if (start_found) {
			start.x = c[i].x-1;
			start.y = c[i].y;

			if(!im.data[start.y*im.xdim + start.x]) {

				// make sure it's not a Point where Contour
				//   doubles back on itself
				dbc = 0;
				for(k=0; k<n; k++)
					if ((c[k].x==c[i].x) && (c[k].y==c[i].y)) dbc++;

				if (dbc<2) {
					get_connected(reg, im, start);
				}
			}
		}
	}
//cout << "Roi d" << endl; cout.flush();

	free(im.data);
//cout << "Roi e" << endl; cout.flush();

	im = create_im(xmax+2, ymax+2);
	pts_to_bim(c, im);

	for(k=0; k<reg.N(); k++)
		im.data[reg[k].y*im.xdim + reg[k].x] = 1;
//cout << "Roi f" << endl; cout.flush();

	/* if overflow occured do not add any Points to region */
	if (!im.data[0]) {
		Plane dpl(c[0].z, 64, 2);

		for(j=ymin; j<=ymax; j++) {
			Line ln(j);
			for(i=xmin; i<=xmax; i++) {
				if (im.data[j*im.xdim+i]&&(!im.data[j*im.xdim+i-1]))
					ivl.x1 = i;
				if (im.data[j*im.xdim+i]&&(!im.data[j*im.xdim+i+1])) {
					ivl.x2 = i;
					ln.ivl.push_back(ivl);
				}
			}
			if (ln.ivl.size()>0)
				dpl.ln.push_last(ln);
		}
		_pl.push_last(dpl);
	}
//cout << "Roi g" << endl; cout.flush();

	free(im.data);

	if ((x_offset>0) || (y_offset>0))
		translate(-x_offset, -y_offset);
//cout << "Roi h" << endl; cout.flush();
}


/*
ROI::ROI(Darray<Point>& c)
	: _pl(1)
{
	int n;
	image_t im;
	int xmin=10000, xmax=-10000, ymin=10000, ymax=-10000;
	int i, j, k, dbc;
	Point start;
	Darray<Point> reg(100);
	Interval ivl;

	image_t create_im(int, int);
	void pts_to_bim(Darray<Point>&, image_t);
	void get_connected(Darray<Point>&, image_t&, const Point&);

	make_continuous_2D(c);
	n = c.N();

	for(i=0; i<n; i++) {
		if (c[i].x<xmin) xmin=c[i].x;
		if (c[i].x>xmax) xmax=c[i].x;
		if (c[i].y<ymin) ymin=c[i].y;
		if (c[i].y>ymax) ymax=c[i].y;
	}

	int x_offset=0, y_offset=0;
	if (xmin<1)
		x_offset = 1 - xmin;
	if (ymin<1)
		y_offset = 1 - ymin;
	if ((x_offset>0) || (y_offset>0)) {
		for(i=0; i<n; i++) {
			c(i).x += x_offset;
			c(i).y += y_offset;
		}
		xmin += x_offset;
		xmax += x_offset;
		ymin += y_offset;
		ymax += y_offset;
	}

	im = create_im(xmax+1, ymax+1);
	pts_to_bim(c, im);

	for(i=1; i<(n-1); i++)
		if ((c[i+1].y<c[i].y) && (c[i-1].y>=c[i].y)){
			start.x = c[i].x-1;
			start.y = c[i].y;
			if(!im.data[start.y*im.xdim + start.x]) {

				// make sure it's not a Point where Contour
				//   doubles back on itself
				dbc = 0;
				for(k=0; k<n; k++)
					if ((c[k].x==c[i].x) && (c[k].y==c[i].y)) dbc++;

				if (dbc<2) {
					get_connected(reg, im, start);
					//j++;
				}
			}
		}

	free(im.data);

	im = create_im(xmax+2, ymax+2);
	pts_to_bim(c, im);

	for(k=0; k<reg.N(); k++)
		im.data[reg[k].y*im.xdim + reg[k].x] = 1;
	if (!im.data[0]) {
		Plane dpl(c[0].z, 64, 2);

		for(j=ymin; j<=ymax; j++) {
			Line ln(j);
			for(i=xmin; i<=xmax; i++) {
				if (im.data[j*im.xdim+i]&&(!im.data[j*im.xdim+i-1]))
					ivl.x1 = i;
				if (im.data[j*im.xdim+i]&&(!im.data[j*im.xdim+i+1])) {
					ivl.x2 = i;
					ln.ivl.push_back(ivl);
				}
			}
			if (ln.ivl.size()>0)
				dpl.ln.push_last(ln);
		}
		_pl.push_last(dpl);
	}

	free(im.data);

	if ((x_offset>0) || (y_offset>0))
		translate(-x_offset, -y_offset);
}
*/


void ROI::add_planar_polygon(const Darray<Point>& poly)
{
	Darray<Point> tp(poly);
	ROI r(tp);

	OR(r);
}


void ROI::add_planar_polygon(const Point* const poly, const int n)
{
	int i;

	Darray<Point> tp(100);
	for(i=0; i<n; i++)
		tp.push_last(poly[i]);

	ROI r(tp);

	OR(r);
}



void ROI::add_planar_polygon(const Contour& cont)
{
	add_planar_polygon(cont.points());
}



void ROI::add_interval(const int x1, const int x2, const int y, const int z)
{
	if (x1<=x2) {
		ROIworkspace w;
		WkspaceStatus s;
		
		s = _set_plane(w, z);
		if (s>=NOT_FOUND) {
			Plane pt(z, _ln_mod, _ivl_mod);
			Line lt(y);
			Interval it(x1, x2);
			lt.ivl.push_back(it);
			pt.ln.push_last(lt);
			_pl.push_inorder(pt, plane_comp);
		}
		else {			
			s = _set_line(w, y);
			if (s>=NOT_FOUND) {
				Line lt(y);
				Interval it(x1, x2);
				lt.ivl.push_back(it);
				_pl(w.pi).ln.push_inorder(lt, line_comp);
			}
			else
				_line(w).add_ivl(x1, x2);
		}
	}
}


void ROI::add_circle(const int r, const int xc, const int yc, const int zc)
{
    	if (r<0) {
		cout << "ERROR: ROI: add_circle: radius must be greater than zero" << endl;
		exit(1);
    	}
 
 	int xd;
	for(int y=yc-r; y<=yc+r; y++) {
		xd =  abs((int)sqrt((float)(r*r - (y-yc)*(y-yc))));
		add_interval(xc-xd, xc+xd, y, zc);
	}
}


void ROI::append_point(const Point& p)
{
    ROIworkspace w;
    register Point ep;
    if (_end_of_roi(w)==ROI_STAT_OK) {
	_current_point(w, ep);

	if (p.z==ep.z) {
		if (p.y==ep.y) {
			if (p.x==(ep.x+1))
				_interval(w).x2=p.x;
			else if (p.x>ep.x) {
				Interval it(p.x, p.x);
				_line(w).ivl.push_back(it);
			}
			else if (p.x<ep.x)
				_line(w).add_ivl(p.x, p.x);
		}
		else if (p.y>ep.y) {	
			Line lt(p.y);
			Interval it(p.x, p.x);
			lt.ivl.push_back(it);
			_pl(w.pi).ln.push_last(lt);
		}
		else if (p.y<ep.y)
			add_point(p);
	}	
	else if (p.z>ep.z) {
		Plane pt(p.z, _ln_mod, _ivl_mod);
		Line lt(p.y);
		Interval it(p.x, p.x);
		lt.ivl.push_back(it);
		pt.ln.push_last(lt);
		_pl.push_last(pt);
	}
	else if (p.z<ep.z)
		add_point(p);
    }
    else
	add_point(p);		
}


void ROI::append_interval(const int x1, const int x2, const int y, const int z)
{
  if (x1<=x2) {
    ROIworkspace w;
    register Point ep1, ep2;
    if (_end_of_roi(w)==ROI_STAT_OK) {
	_current_interval(w, ep1, ep2);

	if (z==ep1.z) {
		if (y==ep1.y) {
			if (x1>(ep2.x+1)) {
				Interval it(x1, x2);
				_line(w).ivl.push_back(it);
			}
			else
				_line(w).add_ivl(x1, x2);
		}
		else if (y>ep1.y) {	
			Line lt(y);
			Interval it(x1, x2);
			lt.ivl.push_back(it);
			_pl(w.pi).ln.push_last(lt);
		}
		else 
			add_interval(x1, x2, y, z);
	}	
	else if (z>ep1.z) {
		Plane pt(z, _ln_mod, _ivl_mod);
		Line lt(y);
		Interval it(x1, x2);
		lt.ivl.push_back(it);
		pt.ln.push_last(lt);
		_pl.push_last(pt);
	}
	else
		add_interval(x1, x2, y, z);
    }
    else
	add_interval(x1, x2, y, z);
  }	
}


void ROI::clear()
{
	Plane* pl;
	Line *ln;
	//Interval *ivl;

	while(_pl.N()>0) {
		pl = &_pl(_pl.N()-1);
		while(pl->ln.N()>0) {
			ln = &pl->ln(pl->ln.N()-1);
			while(ln->ivl.size()>0)
				ln->ivl.erase(ln->ivl.begin()+ln->ivl.size()-1);
			pl->ln.delete_item(pl->ln.N()-1);
		}
		_pl.delete_item(_pl.N()-1);
	}
}



const int ROI::clear(const int z)
{
	ROIworkspace w;
	int done=0;

	if (_set_plane(w, z)==NEW_PLANE) {
		_pl.delete_item(w.pi);
		done = 1;
	}

	return done;
}


int conv_hull_direct(const Point& v1, const Point& v2, const Point& v3)
{
	register double a1 = atan2(static_cast<double>(v2.y-v1.y), v2.x-v1.x); 
	register double a2 = atan2(static_cast<double>(v3.y-v1.y), v3.x-v1.x); 
	register double ad = a2-a1;

	if (ad<0)
		ad = ad + (2*PI_DBL);

	int d=1;
	if (ad==0)
		d = 0;
	else if (ad>PI_DBL)
		d = -1;
	return d;
}



int conv_hull_input(Contour& c, Point& v)
{
	int done = 0;
	if (c.n()>0) {
		v = c[c.n()-1];
		c.remove(c.n()-1);
		done = 1;
	}
	return done;
}


void ROI::convex_hull()
{
	ROI chull;
	Point p1, p2;
	register int ok, triangle;

	ROI se;
	Point tlse(0, 0), brse(1, 1);
	se.add_box(tlse,brse);
	dilate(se);

	if (first_point(p1) && last_point(p2))
	    for(int z=p1.z; z<=p2.z; z++) {
		int n;
		Contour* bndy = boundaries(n, z);
		clear(z);
		for(int i=0; i<n; i++) {
			int mpi=0;
			int j;
			for(j=0; j<n; j++) {
				if (bndy[j].n()>bndy[mpi].n())
					mpi = j;
			}

			if (!chull.in_roi(bndy[mpi][0])) {
			    Contour h(bndy[mpi]);

			    if (bndy[mpi].n()>=3) {
				Point v1, v2, v3, v;
				conv_hull_input(bndy[mpi], v1);
				conv_hull_input(bndy[mpi], v2);
				conv_hull_input(bndy[mpi], v3);
				triangle = 1;
				while(triangle && conv_hull_direct(v1, v2, v3)==0) {
					v2 = v3;
					triangle = conv_hull_input(bndy[mpi], v3);
				}
				if (triangle) {
					h.clear();
					if (conv_hull_direct(v1, v2, v3)>0) {
						h.append(v1);
						h.append(v2);
					}
					else {
						h.append(v2);
						h.append(v1);
					}
					h.append(v3);
					h.prepend(v3);
					while(bndy[mpi].n()>0) {
						ok = 0;
						while(!ok && conv_hull_input(bndy[mpi], v)) {
							ok = (conv_hull_direct(v, h[0], h[1])<0) || (conv_hull_direct(h[h.n()-2], h[h.n()-1], v)<0);
						}				
						if (ok) {
							while(conv_hull_direct(h[h.n()-2], h[h.n()-1], v)<=0) {
								h.remove(h.n()-1);
							}
							h.append(v);
					
							while(conv_hull_direct(v, h[0], h[1])<=0)
								h.remove(0);
							h.prepend(v);	
						}	
					}
					h.reverse();
				}
			    }
			    add_planar_polygon(h);
			}
			else
				bndy[mpi].clear();
		}
		if (n>0)
			delete [] bndy;
	    }
	erode(se);
}

void ROI::crop(const Point &tl, const Point &br)
{
    Point p;
    while (first_point(p) && (p.z<tl.z))
    {
        _pl.delete_item(0);
    }
    while (last_point(p) && (p.z>br.z))
    {
        _pl.delete_item(_pl.N()-1);
    }

    int i;
    int z;
    Point tlr;
    Point brr;
    Point tlt;
    Point brt;
    for (i=0; i<_pl.N(); i++)
    {
        z = _pl[i].z;
        bounding_box(tlr, brr, z);
        ROI sub;

        tlt = tlr;
        brt = brr;
        brt.x = tl.x-1;
        if ((tlt.x<=brt.x) && (tlt.y<=brt.y) && (tlt.z<=brt.z)) sub.add_box(tlt, brt);

        tlt = tlr;
        brt = brr;
        tlt.x = br.x+1;
        if ((tlt.x<=brt.x) && (tlt.y<=brt.y) && (tlt.z<=brt.z)) sub.add_box(tlt, brt);      

        tlt = tlr;
        brt = brr;
        brt.y = tl.y-1;
        if ((tlt.x<=brt.x) && (tlt.y<=brt.y) && (tlt.z<=brt.z)) sub.add_box(tlt, brt);

        tlt = tlr;
        brt = brr;
        tlt.y = br.y+1;
        if ((tlt.x<=brt.x) && (tlt.y<=brt.y) && (tlt.z<=brt.z)) sub.add_box(tlt, brt);

        subtract(sub);
    }
}


void ROI::translate(	const int x_shift,
			const int y_shift,
			const int z_shift)
{
    ROIworkspace w;
    
    if (_valid_wkspace(w) == ROI_STAT_OK) {
      if ((x_shift==0) && (y_shift==0)) {
    	for(WkspaceStatus s = NEW_PLANE; s<END_ROI;  s=_next_plane(w)) {
		_plane(w).z += z_shift;
    	}
      }
      else {
    	for(WkspaceStatus s = NEW_PLANE; s<END_ROI;  s=_next_interval(w)) {
		_interval(w).x1 += x_shift;
		_interval(w).x2 += x_shift;
		
		if (s>=NEW_LINE)
			_line(w).y += y_shift;
			
		if (s>=NEW_PLANE) {
			_plane(w).z += z_shift;
		}
    	}
      }
    }
}

int get_image_index(int z, const int* map, const int map_n) {
	int index = -1;
	for (int i=0; (i<map_n) && (index==-1); i++) 
		if (map[i]==z) index=i;
	
	return index;
}

bool ROI::inverse_map_z(const int* map, const int map_n)
{
	bool ok = 1;
	ROI rim;

	Point fp;
	if (first_point(fp)) {
		Point lp;
		last_point(lp);

		int index = 0;
		for(int z=fp.z; (z<=lp.z) && (index>=0); z++) {
			ROI rz(*this, z);
			if (!rz.empty()) {
				index = get_image_index(z, map, map_n);
				rz.translate(0,0,index-z);
			}
			rim.OR(rz);
		}
		if (index<0) ok = 0;
		else {
			clear();
			OR(rim);
		}
	}
	return ok;
}

void ROI::fill_holes_2D(const int z)
{
	Point tl, br, start;
	if (this->bounding_box(tl, br, z)) {
		ROI holes;
		holes.add_box(tl, br);
		holes.subtract(*this);
		
		start = tl;
		for(;start.x<=br.x; start.x++) {
			ROI outside;
			outside.add_contig(holes, start);
			holes.subtract(outside);
		}

		start = tl; start.x = br.x;
		for(;start.y<=br.y; start.y++) {
			ROI outside;
			outside.add_contig(holes, start);
			holes.subtract(outside);
		}
			
		start = br;
		for(;start.x>=tl.x; start.x--) {
			ROI outside;
			outside.add_contig(holes, start);
			holes.subtract(outside);
		}
			
						
		start = tl; start.y = br.y;
		for(;start.y>=tl.y; start.y--) {
			ROI outside;
			outside.add_contig(holes, start);
			holes.subtract(outside);
		}

		this->OR(holes);
	}
}


/*
ROI* ROI::low_memory_copy(const ROI& b) const
{
	ROI* c = new ROI (b._pl.N(), 1, 1);

	ROIworkspace wb;
	WkspaceStatus s;
	Plane *pp;
	Line *lp;

	s = b._plane_status(wb);
	if (s==ROI_STAT_OK)
		s = NEW_PLANE;

	while(s<END_ROI) {
		if (s>=NEW_PLANE) {
			Plane p(b._plane(wb).z, b._plane(wb).ln.N(), _ivl_mod);
			_pl.push_last(p);
			pp = &_pl(_pl.N()-1);
		}
		if (s>=NEW_LINE) {
			Line l(b._plane(wb).y, _ivl_mod);
			pp->ln.push_last(l);
			lp = &(pp->ln(pp->ln.N()-1));
		}
		Interval i(b._interval(wb).x1, b._interval(wb).x2);
		lp->ivl.push_last(i);

		s=b._next_interval(wb);		
	}
	return c;
}
*/


const int ROI::pop_first_interval(int& x1, int& x2, int& y, int& z)
{
	ROIworkspace w;
	int ok=0;

	if (_valid_wkspace(w)==ROI_STAT_OK) {
		ok = 1;
		Interval& ivl = _interval(w);
		Line& ln = _line(w);
		Plane& pl = _plane(w);
		x1 = ivl.x1;
		x2 = ivl.x2;
		y = ln.y;
		z = pl.z;

		ln.ivl.erase(ln.ivl.begin());
		if (ln.ivl.size()==0) {
			pl.ln.delete_item(0);
			if (pl.ln.N()==0)
				_pl.delete_item(0);
		}
	}
	return ok;
}


void ROI::AND(const ROI& b)
{
	ROIworkspace w, wb;
	WkspaceStatus s;
	int w_advanced;

	s = _plane_status(w);
	if (s==ROI_STAT_OK)
		s = NEW_PLANE;

	while(s<END_ROI) {
	    w_advanced = 0;
	    		
	    if (s>=NEW_PLANE) {
		while ((_plane_status(w)==ROI_STAT_OK) && (b._find_fwd_plane(wb, _plane(w).z)==NOT_FOUND))
		    _pl.delete_item(w.pi);
	    }
	    // Either matching planes have been found or the end of the ROI has been reached

		if (_plane_status(w)!=ROI_STAT_OK)  // end of ROI has been reached
	        s = END_ROI;
	    else { // matching plane has been found
	    	if (s>=NEW_LINE) {
				while ((_line_status(w)==ROI_STAT_OK) && (b._find_fwd_line(wb, _line(w).y)==NOT_FOUND)) {
					_plane(w).ln.delete_item(w.li);
				}
			}
			// Either matching lines have been found or the end of the plane has been reached

			if (_line_status(w)!=ROI_STAT_OK) { // end of plane has been reached
				// Advance ROI to next plane (deleting current plane if it has no lines left)
				if (_plane(w).ln.N()==0) {
					_pl.delete_item(w.pi);
					_first_line(w);
					s = NEW_PLANE;
				}
				else
					s = _next_plane(w);
			}
			else { // matching line has been found
				while ((_interval_status(w)==ROI_STAT_OK) && (b._overlap_fwd_interval(wb, _interval(w))==NOT_FOUND))
					_line(w).ivl.erase(_line(w).ivl.begin()+w.ii);
				// Either matching intervals have been found or the end of the line has been reached

				if (_interval_status(w)!=ROI_STAT_OK) { // end of line has been reached
					// Advance ROI to next line
					// Delete current line if it has no intervals left (delete plane if it then has no lines left)
					if (_line(w).ivl.size()==0) {
						_plane(w).ln.delete_item(w.li);
					
						if (_plane(w).ln.N()==0) {
							_pl.delete_item(w.pi);
							_first_line(w);
							s = NEW_PLANE;
						}
						else
								_first_interval(w);
					}
					else
						s = _next_line(w);
				}
				else { // matching interval has been found
					if (b._interval_const(wb).x1>_interval(w).x1)
						_interval(w).x1 = b._interval_const(wb).x1;
						
					if (b._interval_const(wb).x2<_interval(w).x2) {
						Interval iv(b._interval_const(wb).x2+1, _interval(w).x2);
						_interval(w).x2 = b._interval_const(wb).x2;
						_line(w).ivl.insert(_line(w).ivl.begin()+w.ii+1, iv);
					}					
					s=_next_interval(w);
				}
			}
	    }	
	}
}


void ROI::subtract(const ROI& b)
{
//cout << "start subtract" << endl;
	ROIworkspace w, wb;
	WkspaceStatus s;

	s = b._plane_status(wb);
	if (s==ROI_STAT_OK)
		s = NEW_PLANE;

	while(s<END_ROI) {
		if (s>=NEW_PLANE) {
			if (_find_fwd_plane(w, b._plane_const(wb).z)==NOT_FOUND) {
				b._end_of_plane(wb);
				s = ROI_STAT_OK;
			}
		}
		if (s>=NEW_LINE) {
			if (_find_fwd_line(w, b._line_const(wb).y)==NOT_FOUND) {
				b._end_of_line(wb);
				s = ROI_STAT_OK;
			}
		}
//cout << b._interval(wb).x1 << "  " << b._interval(wb).x2 << endl;
		if (s>=NEW_INTERVAL) {
//cout << _line(w) << endl;
			_line(w).subtract_ivl(b._interval_const(wb));
//cout << _line(w) << endl;

			if (_line(w).ivl.size()==0) {
				_plane(w).ln.delete_item(w.li);
				w.ii = 0;
				b._end_of_line(wb);

				if (_plane(w).ln.N()==0) {
					_pl.delete_item(w.pi);
					w.li = 0;
					b._end_of_plane(wb);
				}
			}
		}

		s=b._next_interval(wb);		
	}
//cout << "done subtract" << endl;
}


void ROI::erode(const ROI& struct_element)
{
	//std::cout << "Here A" << std::endl << std::flush;
	_morph_shell(struct_element, -1);
	//std::cout << "Here B" << std::endl << std::flush;
}


void ROI::dilate(const ROI& struct_element)
{
	_morph_shell(struct_element, 1);
}


void ROI::copy(const ROI& b)
{
	clear();
	_ln_mod = b._ln_mod;
	_ivl_mod = b._ivl_mod;

	for(int i=0; i<b._pl.N(); i++)
		_pl.push_last(b._pl[i]);
}


void ROI::copy(const ROI& b, const int z)
{
	clear(z);
	_ln_mod = b._ln_mod;
	_ivl_mod = b._ivl_mod;

	ROIworkspace w, wb;
	if (b._set_plane(wb, z)==NEW_PLANE) {
		_find_fwd_plane(w, b._plane_const(wb).z);
		_pl.push_here(b._plane_const(wb), w.pi);
	}
}


void ROI::OR(const ROI& b)
{
	ROIworkspace w, wb;
	WkspaceStatus s;
	//Plane *pp;
	//Line *lp;
	//Interval *ip;

	s = b._plane_status(wb);
	if (s==ROI_STAT_OK)
		s = NEW_PLANE;

	while(s<END_ROI) {
		if (s>=NEW_PLANE) {
			//pp = &b._plane(wb);
			//if (_find_fwd_plane(w, pp->z)==NOT_FOUND) {
			if (_find_fwd_plane(w, b._plane_const(wb).z)==NOT_FOUND) {
				/*
				Plane tp(*pp);
				_pl.push_here(tp, w.pi);
				*/
//cout << _pl.N() << ", " << pp->z << ", " << w.pi << endl;
				//_pl.push_here(*pp, w.pi);
				_pl.push_here(b._plane_const(wb), w.pi);
				_end_of_plane(w);
				b._end_of_plane(wb);
				s = ROI_STAT_OK;
			}
		}
		if (s>=NEW_LINE) {
			//lp = &b._line(wb);
			//if (_find_fwd_line(w, lp->y)==NOT_FOUND) {
			if (_find_fwd_line(w, b._line_const(wb).y)==NOT_FOUND) {
				//_plane(w).ln.push_here(*lp, w.li);
				_plane(w).ln.push_here(b._line_const(wb), w.li);
				_end_of_line(w);
				b._end_of_line(wb);
				s = ROI_STAT_OK;
			}
		}
		if (s>=NEW_INTERVAL) {
			_line(w).add_ivl(b._interval_const(wb));
		}

		s=b._next_interval(wb);		
	}
}


const int ROI::in_roi(const Point &p) const
{
	ROIworkspace w;
	//int i;

	return ((_set_plane(w, p.z)<NOT_FOUND) && (_set_line(w, p.y)<NOT_FOUND) && (_set_interval(w, p.x)<NOT_FOUND));
}


const int ROI::num_pix() const
{
	ROIworkspace w;
	int status, n=0;
	
	for(status=_plane_status(w); (status<END_ROI); status=_next_interval(w))
		n += _interval_const(w).num_pts();
	return n;
}


const int ROI::num_pix(const int zv) const
{
	ROIworkspace w;
	int n=0;
	
	if (_set_plane(w, zv)<NOT_FOUND)
	  do {
		n += _interval_const(w).num_pts();
	  } while(_next_interval(w)<NEW_PLANE);
	return n;
}


const int ROI::num_pix_grequal(const int v) const
{
	ROIworkspace w;
	int status, n=0;
	
	for(status=_plane_status(w); (status<END_ROI) && (n<v); status=_next_interval(w))
		n += _interval_const(w).num_pts();
	return (n>=v);
}


const int ROI::overlaps(const ROI& r) const {
	ROIworkspace w, wr;
	int s=_plane_status(w), sr=r._valid_wkspace(wr), s_result=NOT_FOUND;

	while ((s<END_ROI) && (sr<END_ROI) && (s_result==NOT_FOUND)) {
		if (_plane_const(w).z!=r._plane_const(wr).z) {
			_find_fwd_plane(w, r._plane_const(wr).z);
			s = _plane_status(w);
		}
		if ((s<END_ROI) && (_plane_const(w).z==r._plane_const(wr).z)) {
			if (_line_const(w).y!=r._line_const(wr).y) {
				_find_fwd_line(w, r._line_const(wr).y);
			}
			if (_line_status(w)!=ROI_STAT_OK) {
				s = _next_plane(w);
				sr = r._next_plane(wr);
			}
			else if (_line_const(w).y==r._line_const(wr).y) {
				s_result = _overlap_fwd_interval(w, r._interval_const(wr));
				sr = r._next_interval(wr);
			}
			else {
				sr = r._next_line(wr);
			}
		}
		else {
			sr = r._next_plane(wr);
		}
	}
	return (s_result!=NOT_FOUND);
}


void add_point_to_bndy(Point &cpp, Darray<Point> &pts, Darray<Point> &sp)
{
	pts.push_last(cpp);

	//if (sp.N()>0) {
		int i = sp.find_item(cpp, point_comp_xy);
		if (i>-1) {
			sp.delete_item(i);
		}
	//}
}

Contour* ROI::boundaries(int& n, const int zv) const
{
	int j, k, l, m;
  //int i;
	Darray<Contour> ba(3);
	Contour *b;
	int search_dir, nfc;
	Point cp[3];
	ROIworkspace w;

	
	n = 0;
	for(m=0; m<3; m++)
		cp[m].z = zv;
	
	if (_set_plane(w, zv)<NOT_FOUND) {
		//Darray<Contour> *ca = new Darray<Contour> (3);
		Darray<Point> sp(100);

		do {
		  	Point p2(_interval_const(w).x2, _line_const(w).y, zv);
			sp.push_last(p2);
		} while(_next_interval(w)<NEW_PLANE);

		_set_plane(w, zv);
		ROIworkspace w0(w), w1(w), w2(w);

		while(sp.N()) {
			Darray<Point> pts(50);

			_set_line(w, sp[sp.N()-1].y);
			pts.push_last(sp[sp.N()-1]);
			sp.delete_item(sp.N()-1);

      			search_dir=0;
			nfc=0;

      			while((nfc<5) && ((pts.N()==1) || (pts[0].x!=pts[pts.N()-1].x) || (pts[0].y!=pts[pts.N()-1].y))) {  /* do until first = last */
	  	  
	  			k = pts[pts.N()-1].x;
	  			l = pts[pts.N()-1].y;
	  	  	  
	  			switch(search_dir) {
	    				case 0:	cp[0].x=k+1; cp[0].y=l-1;
						cp[1].x=k; cp[1].y=l-1;
						cp[2].x=k-1; cp[2].y=l-1;
						break;
	    				case 1:	cp[0].x=k+1; cp[0].y=l+1;
						cp[1].x=k+1; cp[1].y=l;
						cp[2].x=k+1; cp[2].y=l-1;
						break;
					case 2:	cp[0].x=k-1; cp[0].y=l+1;
						cp[1].x=k; cp[1].y=l+1;
						cp[2].x=k+1; cp[2].y=l+1;
						break;
	    				case 3:	cp[0].x=k-1; cp[0].y=l-1;
						cp[1].x=k-1; cp[1].y=l;
						cp[2].x=k-1; cp[2].y=l+1;
						break;
				}

				w0.li = w.li+cp[0].y-l;
				w1.li = w.li+cp[1].y-l;
				w2.li = w.li+cp[2].y-l;

				if ((w0.li>=0) && (w0.li<_plane_const(w0).ln.N()) && (_line_const(w0).y==cp[0].y) && (_set_interval(w0, cp[0].x)<NOT_FOUND)) {
		    			add_point_to_bndy(cp[0], pts, sp);
				    	search_dir = (search_dir+1)%4;
					nfc=0;
					w.li = w0.li;
		  		}
				else if ((w1.li>=0) && (w1.li<_plane_const(w1).ln.N()) && (_line_const(w1).y==cp[1].y) && (_set_interval(w1, cp[1].x)<NOT_FOUND)) {
		    			add_point_to_bndy(cp[1], pts, sp);
					nfc=0;
					w.li = w1.li;
				}
				else if ((w2.li>=0) && (w2.li<_plane_const(w2).ln.N()) && (_line_const(w2).y==cp[2].y) && (_set_interval(w2, cp[2].x)<NOT_FOUND)) {
 		    			add_point_to_bndy(cp[2], pts, sp);
					nfc=0;
					w.li = w2.li;
				}
				else {
				    	search_dir--;
					if (search_dir<0) search_dir=3;
					nfc++;
				}
	      		}		
			if (pts.N()>1)
				pts.delete_item(pts.N()-1);

			Contour c(pts);
			ba.push_last(c);
		}


		n = ba.N();
		b = new Contour[n];
		for(j=0; j<n; j++) {
			b[j].append(ba[j]);
//ba[j].print();
		}
	}
	else
		b = NULL;
	
	return b;
}



/*
Contour* ROI::boundaries(int& n, const int zv) const
{
	int i, j, k, l, m;
	Darray<Contour> ba(3);
	Contour *b;
	int search_dir, nfc;
	Point cp[3];
	ROIworkspace w;

	
	n = 0;
	for(m=0; m<3; m++)
		cp[m].z = zv;
	
	if (_set_plane(w, zv)<NOT_FOUND) {
		Darray<Contour> *ca = new Darray<Contour> (3);
		Darray<Point> sp(100);

		do {
		  	Point p2(_interval(w).x2, _line(w).y, zv);
			sp.push_last(p2);
		} while(_next_interval(w)<NEW_PLANE);

		while(sp.N()) {
			Darray<Point> pts(50);

			pts.push_last(sp[sp.N()-1]);
			sp.delete_item(sp.N()-1);

      			search_dir=0;
			nfc=0;

      			while((nfc<5) && ((pts.N()==1) || (pts[0].x!=pts[pts.N()-1].x) || (pts[0].y!=pts[pts.N()-1].y))) {  	  	  
	  			k = pts[pts.N()-1].x;
	  			l = pts[pts.N()-1].y;
	  	  	  
	  			switch(search_dir) {
	    				case 0:	cp[0].x=k+1; cp[0].y=l-1;
						cp[1].x=k; cp[1].y=l-1;
						cp[2].x=k-1; cp[2].y=l-1;
						break;
	    				case 1:	cp[0].x=k+1; cp[0].y=l+1;
						cp[1].x=k+1; cp[1].y=l;
						cp[2].x=k+1; cp[2].y=l-1;
						break;
					case 2:	cp[0].x=k-1; cp[0].y=l+1;
						cp[1].x=k; cp[1].y=l+1;
						cp[2].x=k+1; cp[2].y=l+1;
						break;
	    				case 3:	cp[0].x=k-1; cp[0].y=l-1;
						cp[1].x=k-1; cp[1].y=l;
						cp[2].x=k-1; cp[2].y=l+1;
						break;
				}

				if (in_roi(cp[0])) {
		    			add_point_to_bndy(cp[0], pts, sp);
				    	search_dir = (search_dir+1)%4;
					nfc=0;
		  		}
				else if (in_roi(cp[1])) {
		    			add_point_to_bndy(cp[1], pts, sp);
					nfc=0;
				}
				else if (in_roi(cp[2])) {
 		    			add_point_to_bndy(cp[2], pts, sp);
					nfc=0;
				}
				else {
				    	search_dir--;
					if (search_dir<0) search_dir=3;
					nfc++;
				}
	      		}		
			if (pts.N()>1)
				pts.delete_item(pts.N()-1);

			Contour c(pts);
			ba.push_last(c);
		}


		n = ba.N();
		b = new Contour[n];
		for(j=0; j<n; j++) {
			b[j].append(ba[j]);
		}
	}
	else
		b = NULL;
	
	return b;
}
*/

void ROI::print_all_points() const
{
	int i, j;
  unsigned int k;

	for(i=0; i<_pl.N(); i++) {
		cout << "z=" << _pl[i].z << "\n";
		const Darray<Line>& lp = _pl[i].ln;
		for(j=0; j<lp.N(); j++) {
			cout << "\ty=" << lp[j].y << ":";
			const std::vector<Interval> ip = lp[j].ivl;
			for(k=0; k<ip.size(); k++)
				cout << "  " << ip[k].x1 << "-" << ip[k].x2;
			cout << "\n";
		}
	}
}



void ROI::subsample(ROI& new_roi, const int x_step, const int y_step, const int z_step, const bool conservative) const
{
	if ((x_step<1) || (y_step<1) || (z_step<1)) {
		cout << "ERROR: Roi: subsample: step parameters invalid" << endl;
		exit(1);
	}

	new_roi.clear();

	Point se_tl(-((int)(x_step/2)), -((int)(y_step/2)), -((int)(z_step/2)));
	Point se_br(x_step/2, y_step/2, z_step/2);
	ROI se;
	se.add_box(se_tl, se_br);
	ROI rt(*this);
	if (conservative) {
		rt.erode(se);
	}
	else {
		rt.dilate(se);
	}
	
	ROIworkspace w;
	WkspaceStatus s = rt._valid_wkspace(w);
	register Point p1;
	register Point p2;
	register int new_y, new_z;
	while(s<END_ROI) {
		rt._current_point(w, p1);
		s = ROI_STAT_OK; 
		if ((p1.z%z_step)==0) {
			new_z = p1.z/z_step;
			while(s<NEW_PLANE) {
				rt._current_point(w, p1);
				s = ROI_STAT_OK;
				if ((p1.y%y_step)==0) {
					new_y = p1.y/y_step;
					while(s<NEW_LINE) {
						rt._current_interval(w, p1, p2);
						new_roi.append_interval(rnd_up_step(p1.x, x_step)/x_step, rnd_step(p2.x, x_step)/x_step, new_y, new_z);
						s = rt._next_interval(w);
					}
				}
				else {
					s = rt._next_line(w);
				}
			}
		}
		else {
			s = rt._next_plane(w);
		}
	}
}



void ROI::upsample(ROI& new_roi, const int x_step, const int y_step, const int z_step, const bool conservative) const
{
	if ((x_step<1) || (y_step<1) || (z_step<1)) {
		cout << "ERROR: Roi: upsample: step parameters invalid" << endl;
		exit(1);
	}

	new_roi.copy(*this);

	ROIworkspace w;
	WkspaceStatus s = new_roi._valid_wkspace(w);
	if ((x_step==1) && (y_step==1)) {
		while(s<END_ROI) {
			Plane& p = new_roi._plane(w);
			p.z = p.z*z_step;
			s = new_roi._next_plane(w);
		}
	}
	else {
		while(s<END_ROI) {
			Plane& p = new_roi._plane(w);
			p.z = p.z*z_step;
			s = ROI_STAT_OK;
			while(s<NEW_PLANE) {
				Line& l = p.ln(w.li);
				l.y = l.y*y_step;
				s = ROI_STAT_OK;
				while(s<NEW_LINE) {
					Interval& i = l.ivl[w.ii];
					i.x1 = i.x1*x_step;
					i.x2 = i.x2*x_step;
					s = new_roi._next_interval(w);
				}
			}
		}
	}

	Point se_tl;
	Point se_br;
	if (conservative) {
		se_tl.set(-((int)(x_step/2)), -((int)(y_step/2)), -((int)(z_step/2)));
		se_br.set(x_step/2, y_step/2, z_step/2);
	}
	else {
		se_tl.set(-x_step+1, -y_step+1, -z_step+1);
		se_br.set(x_step-1, y_step-1, z_step-1);
	}
	ROI se;
	se.add_box(se_tl, se_br);
	new_roi.dilate(se);
}


const WkspaceStatus ROI::_next_point(ROIworkspace& w) const
{
	assert(_valid_wkspace(w)<NOT_FOUND);

	return _next_x(w);
}


void ROI::_current_point(ROIworkspace& w, Point& p) const
{
	assert(_valid_wkspace(w)<NOT_FOUND);

	p.z = _plane_const(w).z;
	p.y =_line_const(w).y;
	p.x = _interval_const(w).x1+w.xi;
}


void ROI::_current_interval(ROIworkspace& w, Point& p1, Point& p2) const
{
	assert(_valid_wkspace(w)<NOT_FOUND);

	p1.z = p2.z = _plane_const(w).z;
	p1.y = p2.y = _line_const(w).y;
	p1.x = _interval_const(w).x1;
	p2.x = _interval_const(w).x2;
}

const WkspaceStatus ROI::_set_workspace(ROIworkspace& w, const int z, const int y, const int x) const
{
	WkspaceStatus ws;

	ws = _set_plane(w, z);
	if ((ws<NOT_FOUND) && (y>=0)) {
		ws = _set_line(w, y);
		if ((ws<NOT_FOUND) && (x>=0)) {
			ws = _set_interval(w, x);
			if (ws<NOT_FOUND)
				ws = _set_x(w, x);
		}
	}
	if (ws<NOT_FOUND) ws=ROI_STAT_OK;
	return ws;
}


const int ROI::bounding_box(Point& ul, Point& br, const int z) const
{
	ROIworkspace w;
	Point p;

	WkspaceStatus ws = _set_workspace(w, z);

	if (ws==ROI_STAT_OK) {
		_current_point(w, p);
		ul = p;
		br = p;
	}
	while (ws<NEW_PLANE) {
		_current_point(w, p);
		if (p.x<ul.x) ul.x=p.x;

		_end_of_line(w);
		_current_point(w, p);
		if (p.x>br.x) br.x=p.x;

		br.y = p.y;

		ws = _next_interval(w);
	}

	return (ws<NOT_FOUND);
}


const int ROI::bounding_cube(Point& tl, Point& br) const
{
	ROIworkspace w;
	Point p;
	Point v1, v2;

	WkspaceStatus ws = _valid_wkspace(w);

	if(ws==ROI_STAT_OK) {
		_current_point(w, p);
		tl = p;
		br = p;
	}

	while(ws<END_ROI) {
		_current_point(w, p);
		bounding_box(v1, v2, p.z);
		if (v1.x<tl.x) tl.x=v1.x;
		if (v1.y<tl.y) tl.y=v1.y;
		if (v2.x>br.x) br.x=v2.x;
		if (v2.y>br.y) br.y=v2.y;
		br.z = v2.z;

		ws = _next_plane(w);
	}

	return (ws<NOT_FOUND);
}

const int ROI::centroid(Point& c, const int z) const
{
	ROIworkspace w;
	Point p1, p2, p(0,0,z);
	int cnt=0, n;

	WkspaceStatus ws = _set_workspace(w, z);
	/*
	if (ws==ROI_STAT_OK) {
		_current_point(w, p);
		ul = p;
		br = p;
	}
	*/
	while (ws<NEW_PLANE) {
		_current_interval(w, p1, p2);
		n = p2.x-p1.x+1;
		p.x += (n*(p1.x+p2.x)/2);
		p.y += (n*p1.y);
		cnt += n;
		ws = _next_interval(w);
	}

	if (ws<NOT_FOUND) {
		c.x=(p.x/cnt); c.y=(p.y/cnt);  c.z=p.z;
	}

	return (ws<NOT_FOUND);
}


const int ROI::empty() const
{
	ROIworkspace w;

	if (_valid_wkspace(w)==ROI_STAT_OK)
		return 0;
	else
		return 1;
}

const int ROI::empty(const int z) const
{
	ROIworkspace w;

	if (_set_plane(w, z)==NOT_FOUND)
		return 1;
	else
		return 0;
}

const int ROI::first_point(Point& p) const
{
	ROIworkspace w;
	if (_valid_wkspace(w)==ROI_STAT_OK) {
		_current_point(w, p);
		return 1;
	}
	else {
		return 0;
	}
}

const int ROI::last_point(Point& p) const
{
	ROIworkspace w;
	if (_end_of_roi(w)==ROI_STAT_OK) {
		_current_point(w, p);
		return 1;
	}
	else {
		return 0;
	}
}

const WkspaceStatus ROI::_plane_status(const ROIworkspace& w) const
{
	assert(w.pi>=0);

	if (w.pi<_pl.N())
		return ROI_STAT_OK;
	else
		return END_ROI;
}



const WkspaceStatus ROI::_next_plane(ROIworkspace& w) const
{
	assert(w.pi>=0);

	w.pi++;
	if (_plane_status(w)==ROI_STAT_OK) {
		w.li = w.ii = w.xi = 0;
		return NEW_PLANE;
	}
	else
		return END_ROI;
}


const WkspaceStatus ROI::_set_plane(ROIworkspace& w, const int z) const
{
	Plane dp(z);
	int i=_pl.find_item(dp, plane_comp);

	if (i>=0) {
		w.pi = i;
		w.li = w.ii = w.xi = 0;
		return NEW_PLANE;
	}
	else
		return NOT_FOUND;
}


const WkspaceStatus ROI::_find_fwd_plane(ROIworkspace& w, const int z) const
{
	while((_plane_status(w)==ROI_STAT_OK) && (_plane_const(w).z<z)) w.pi++;

	if ((_plane_status(w)==ROI_STAT_OK) && (_plane_const(w).z==z)) {
		w.li = w.ii = w.xi = 0;
		return NEW_PLANE;
	}
	else
		return NOT_FOUND;
}


const WkspaceStatus ROI::_end_of_roi(ROIworkspace& w) const
{
	WkspaceStatus s=ROI_STAT_OK;

	w.pi=_pl.N()-1;
	if (_valid_wkspace(w)==ROI_STAT_OK) {
		_end_of_plane(w);
	}
	else {
		s = NOT_FOUND;
	}

	return s;
}



void ROI::_end_of_plane(ROIworkspace& w) const
{
	assert(_plane_status(w)==ROI_STAT_OK);

	w.li=_plane_const(w).ln.N()-1;
	_end_of_line(w);
}


const WkspaceStatus ROI::_line_status(const ROIworkspace& w) const
{
	assert((w.li>=0)&&_plane_status(w)==ROI_STAT_OK);

	if (w.li<_pl[w.pi].ln.N())
		return ROI_STAT_OK;
	else
		return NEW_PLANE;
}


const WkspaceStatus ROI::_next_line(ROIworkspace& w) const
{
	assert(w.li>=0);

	w.li++;
	if (_line_status(w)==ROI_STAT_OK) {
		w.ii = w.xi = 0;
		return NEW_LINE;
	}
	else
		return _next_plane(w);
}


const WkspaceStatus ROI::_set_line(ROIworkspace& w, const int y) const
{
	assert((_plane_status(w)==ROI_STAT_OK) && (w.pi>=0));

	const Plane& pl = _plane_const(w);
	Line dp(y);
	int i = pl.ln.find_item(dp, line_comp);

	if (i>=0) {
		w.li = i;
		w.ii = w.xi = 0;
		return NEW_LINE;
	}
	else
		return NOT_FOUND;
}


const WkspaceStatus ROI::_find_fwd_line(ROIworkspace& w, const int y) const
{
	while((_line_status(w)==ROI_STAT_OK) && (_line_const(w).y<y)) w.li++;

	if ((_line_status(w)==ROI_STAT_OK) && (_line_const(w).y==y)) {
		w.ii = w.xi = 0;
		return NEW_LINE;
	}
	else
		return NOT_FOUND;
}


const WkspaceStatus ROI::_interval_status(const ROIworkspace& w) const
{
	assert((w.ii>=0)&&_line_status(w)==ROI_STAT_OK);

	if (w.ii<(signed int)_pl[w.pi].ln[w.li].ivl.size())
		return ROI_STAT_OK;
	else
		return NEW_LINE;
}


const WkspaceStatus ROI::_next_interval(ROIworkspace& w) const
{
	assert(w.ii>=0);

	w.ii++;

	if (_interval_status(w)==ROI_STAT_OK) {
		w.xi = 0;
		return NEW_INTERVAL;
	}
	else
		return _next_line(w);
}


const WkspaceStatus ROI::_set_interval(ROIworkspace& w, const int x) const
{
	assert((_line_status(w)==ROI_STAT_OK) && (w.li>=0));

	const Line& ln = _line_const(w);

  	const std::vector<Interval>& ivl=ln.ivl;
	unsigned int i;
	for(i=0; (i<ivl.size())&&(x>ivl[i].x2); i++);	

	if ((i<ivl.size()) && (ivl[i].x1<=x)) {
		w.ii = i;
		w.xi = 0;
		return NEW_INTERVAL;
	}
	else
		return NOT_FOUND;
}

const WkspaceStatus ROI::_overlap_fwd_interval(ROIworkspace& w, const Interval ivl) const
{
	while((_interval_status(w)==ROI_STAT_OK) && (ivl_comp_overlap(_interval_const(w), ivl)<0)) w.ii++;

	if ((_interval_status(w)==ROI_STAT_OK) && (ivl_comp_overlap(_interval_const(w), ivl)==0)) {
		w.xi = 0;
		return NEW_INTERVAL;
	}
	else
		return NOT_FOUND;
}


void ROI::_end_of_line(ROIworkspace& w) const
{
	assert(_line_status(w)==ROI_STAT_OK);

	w.ii=_line_const(w).ivl.size()-1;
	_end_of_interval(w);
}


const WkspaceStatus ROI::_x_status(const ROIworkspace& w) const
{
	assert(_interval_status(w)==ROI_STAT_OK&&(w.xi>=0));

	if (w.xi<_interval_const(w).num_pts())
		return ROI_STAT_OK;
	else
		return NEW_INTERVAL;
}


const WkspaceStatus ROI::_next_x(ROIworkspace& w) const
{
	assert(_x_status(w)==ROI_STAT_OK);

	w.xi++;
	if (_x_status(w)==ROI_STAT_OK)
		return ROI_STAT_OK;
	else
		return _next_interval(w);
}


const WkspaceStatus ROI::_set_x(ROIworkspace& w, const int x) const
{
	assert((_interval_status(w)==ROI_STAT_OK) && (w.ii>=0));

	const Interval& iv = _interval_const(w);
	int nxi = x-iv.x1;

	if ((nxi>=0) && (x<=iv.x2)) {
		w.xi = nxi;
		return ROI_STAT_OK;
	}
	else
		return NOT_FOUND;
}


void ROI::_end_of_interval(ROIworkspace& w) const
{
	assert(_interval_status(w)==ROI_STAT_OK);

	w.xi=_interval_const(w).num_pts()-1;
}


const WkspaceStatus ROI::_valid_wkspace(const ROIworkspace& w) const
{
	int ok;

	ok = (w.pi>=0) && (w.pi<_pl.N());
	if (ok) ok = (w.li>=0) && (w.li<_plane_const(w).ln.N());
	if (ok) ok = (w.ii>=0) && (w.ii<(signed int)_line_const(w).ivl.size());
	if (ok) ok = (w.xi>=0) && (w.xi<_interval_const(w).num_pts());

	if (ok)
		return ROI_STAT_OK;
	else
		return NOT_FOUND;
}


/*
void ROI::_append_interval(ROIworkspace& w, const int x1, const int x2, const int y, const int
z)
{
	if ((w.pi==-1) || (_plane(w).z!=z)) {
		Plane pt(z, _ln_mod, _ivl_mod);
		_pl.push_last(pt);
		_next_plane(w);
	}

	if ((w.li==-1) || (_line(w).y!=y)) {
		Line lt(y);
		_plane(w).ln.push_last(lt);
		_next_line(w);
	}

	Interval it(x1, x2);
	_line(w).ivl.push_back(it);
	_next_interval(w);
}
*/

void ROI::_append_interval(ROIworkspace& w, const int x1, const int x2, const int y, const int
z)
{
	if ((w.pi==-1) || (_plane(w).z!=z)) {
		Plane pt(z, _ln_mod, _ivl_mod);
		_pl.push_last(pt);
		w.pi = w.pi+1;
		w.li = -1;
	}

	if ((w.li==-1) || (_line(w).y!=y)) {
		Line lt(y);
		_plane(w).ln.push_last(lt);
		w.li++;
	}

	Interval it(x1, x2);
	_line(w).ivl.push_back(it);
	w.ii++;
}

void ROI::_morph_shell(const ROI& se, const int trans_sign)
{
    	if ((trans_sign!=-1) && (trans_sign!=1)) {
		cout << endl << endl << "ERROR: ROI: morph_shell: trans_sign must be -1 or 1" << endl;
		exit(1);
    	}

	ROI result;
	int first_time=1;
	int x1, x2, y, z;
	int x_prev, y_prev, z_prev;

	ROIworkspace wse;
	for(WkspaceStatus stat_se=se._valid_wkspace(wse); stat_se<END_ROI; stat_se=se._next_interval(wse)) {
		ROI partial_result;
		ROIworkspace w, wpr(-1, -1, -1, -1);
		register int z_offset = trans_sign*se._plane_const(wse).z;
		register int y_offset = trans_sign*se._line_const(wse).y;
		register int x1_offset = trans_sign*se._interval_const(wse).x1;
		register int x2_offset = trans_sign*se._interval_const(wse).x2;

		WkspaceStatus stat=_valid_wkspace(w);
		if (stat==ROI_STAT_OK) {
			z_prev = _plane(w).z + z_offset;
			y_prev = _line(w).y + y_offset;
			x_prev = _interval(w).x2 + x2_offset;
			
			for(; stat<END_ROI; stat=_next_interval(w)) {
			//cout << "here1" << endl;
				z = _plane(w).z + z_offset;

				y = _line(w).y + y_offset;

				x1 = _interval(w).x1 + x1_offset;
				x2 = _interval(w).x2 + x2_offset;
				//cout << "here2" << endl;
			
				if (x1<=x2) {
					if ((y==y_prev) && (x1<=(x_prev+1)) && (z==z_prev)) {
					//cout << "here4" << endl;
						partial_result.add_interval(x1, x2, y, z);
						//cout << "here5" << endl;
						if (wpr.pi==-1) {
							wpr.zero();
						}
					}
					else {
					//cout << "here6" << endl;
						partial_result._append_interval(wpr, x1, x2, y, z);
						//cout << "here7" << endl;
					}
				}
				//cout << "here3" << endl;
				z_prev = z;
				y_prev = y;
				x_prev = x2;
			}
		}

		if ((trans_sign==1) || first_time) {
			first_time=0;
			result.OR(partial_result);
		}
		else {
			result.AND(partial_result);
		}
	}


	clear();
	OR(result);
}

void ROI::_add_overlap_interval(const int x1, const int x2, const int y, const int z, ROI& r)
{
	ROIworkspace w;
	if ((r._set_plane(w, z)<NOT_FOUND) && (r._set_line(w, y)<NOT_FOUND)) {
		Interval iv(x1, x2);
		WkspaceStatus s = r._overlap_fwd_interval(w, iv);	
		while (s<NOT_FOUND) {
			add_interval(r._interval(w).x1, r._interval(w).x2, r._line(w).y, r._plane(w).z);
			r._line(w).subtract_ivl(r._interval(w).x1, r._interval(w).x2);
			if (r._line(w).ivl.empty()) {
				r._plane(w).ln.delete_item(w.li);
				if (r._plane(w).ln.N()==0) {
					r._pl.delete_item(w.pi);
				}
				s = NOT_FOUND;
			}
			else {
				s = r._overlap_fwd_interval(w, iv);
			}
		}
	}
}

/*
void ROI::_add_overlap_interval(const int x1, const int x2, const int y, const int z, const ROI& r)
{
	ROIworkspace w;
	if ((r._set_plane(w, z)<NOT_FOUND) && (r._set_line(w, y)<NOT_FOUND)) {
		Interval iv(x1, x2);
		WkspaceStatus s = r._overlap_fwd_interval(w, iv);	
		while (s<NOT_FOUND) {
			add_interval(r._interval(w).x1, r._interval(w).x2, r._line(w).y, r._plane(w).z);
			r._line(w).subtract_ivl(r._interval(w).x1, r._interval(w).x2);
			s = r._overlap_fwd_interval(w, iv);
		}
	}
}
*/

// fill "buffer" with x,y,z coordinates of roi pixels
// "buffer" should be pre-allocated to 3xNcoords(and freed) by calling function
// Ncoords is number of pixels/coordinates
void ROI::getPixelCoordColumnMajor(int buffer[], const int Ncoords) const
{

	int i = 0;


	ROItraverser rt(*this);
	register Point p1;
	TravStatus s = rt.reset();
	while (s<END_ROI) {
		rt.current_point(p1);
		// p1 has the coordinates of the ROI point
		buffer[i] = p1.x;
		buffer[i + Ncoords] = p1.y;
		buffer[i + 2*Ncoords] = p1.z;

		s = rt.next_point();
		i++;
	}

}

void ROI_unit_test() {
	cout << "ROI_unit_test.... " << endl;

	/*
	VFC::Dims se_tl(-1,-1,0);
	VFC::Dims se_br(1,1,0);
	Roi se;
	se.add_box(se_tl, se_br);
	se.print_all_points();
	*/

	Point r_tl;
	Point r_br;
	ROI r;
	Contour c;
	/*
	r_tl.set(-3,-3,0);
	r_br.set(3,3,0);
	r.add_box(r_tl, r_br);
	r.print_all_points();
	r.erode(se);
	r.print_all_points();
	*/

		
	/*
	cout << "Testing dilate...." << endl;
	Roi se;
	VFC::Dims tlse(0, 0), brse(1, 1);
	se.add_box(tlse, brse);
	r_tl.set(2,2,1);
	r_br.set(6,2,1);
	r.clear();
	r.add_box(r_tl, r_br);
	r_tl.set(6,2,1);
	r_br.set(6,6,1);
	r.add_box(r_tl, r_br);
	r_tl.set(8,3,1);
	r_br.set(10,5,1);
	r.add_box(r_tl, r_br);
	r.print_all_points();
	r.dilate(se);
	r.print_all_points();
	*/

	/*
	cout << "Testing transpose of XY to YZ.... " << endl;
	cout << "See p.17 of VTAL notebook for expected output" << endl;
	r.clear();
	r_tl.set(0,0,0);
	r_br.set(2,2,0);
	r.add_box(r_tl, r_br);
	r_tl.set(1,1,1);
	r_br.set(3,3,1);
	r.add_box(r_tl, r_br);
	Roi r_trans;
	r_trans.transpose(r, 0);
	r_trans.print_all_points();
	cout << "Testing transpose of YZ to XY.... " << endl;
	cout << "See p.17 of VTAL notebook for expected output" << endl;
	r.clear();
	r.transpose(r_trans, 0);
	r.print_all_points();
	*/

	/*
	cout << "Testing transpose of XY to XZ.... " << endl;
	cout << "See p.17 of VTAL notebook for expected output" << endl;
	r.clear();
	r_tl.set(0,0,0);
	r_br.set(2,2,0);
	r.add_box(r_tl, r_br);
	r_tl.set(1,1,1);
	r_br.set(3,3,1);
	r.add_box(r_tl, r_br);
	r_trans.clear();
	r_trans.transpose(r, 1);
	r_trans.print_all_points();
	cout << "Testing transpose of XZ to XY.... " << endl;
	cout << "See p.17 of VTAL notebook for expected output" << endl;
	r.clear();
	r.transpose(r_trans, 1);
	r.print_all_points();
	cout << endl;
	*/

	/*
	cout << "Testing convex_hull.... " << endl;
	cout << "Expecting box with top left=(0,0,0), bottom right=(5,5,0)" << endl;
	c.clear();
	c.append(VFC::Dims(0, 0, 0));
	c.append(VFC::Dims(2, 2, 0));
	c.append(VFC::Dims(5, 0, 0));
	c.append(VFC::Dims(5, 5, 0));
	c.append(VFC::Dims(4, 4, 0));
	c.append(VFC::Dims(0, 5, 0));
	c.append(VFC::Dims(1, 1, 0));
	c.reverse();
	r.clear();
	r.add_planar_polygon(c);
	r.convex_hull();
	r.print_all_points();
	cout << "Expecting triangle with vertices: (2,2,1)(6,2,1)(6,6,1) and box with top left=(10,3,1) and bottom right (13,5,1)" << endl;
	r_tl.set(2,2,1);
	r_br.set(6,2,1);
	r.clear();
	r.add_box(r_tl, r_br);
	r_tl.set(6,2,1);
	r_br.set(6,6,1);
	r.add_box(r_tl, r_br);
	r_tl.set(10,3,1);
	r_br.set(13,5,1);
	r.add_box(r_tl, r_br);
	r.convex_hull();
	r.print_all_points();
	*/

	/*
	cout << "Testing add_planar_polygon.... " << endl;
	cout << "Expecting single point (5,5,0)" << endl;
	c.clear();
	c.append(VFC::Dims(5, 5, 0));
	r.clear();
	r.add_planar_polygon(c);
	r.print_all_points();
	r.clear();
	cout << "Expecting three points (6,5,0)(7,5,0)(8,5,0)" << endl;
	//cout << "Expecting two points (5,5,0)(6,5,0)(7,5,0)(8,5,0)" << endl;
	c.clear();
	c.append(VFC::Dims(6, 5, 0));
	c.append(VFC::Dims(7, 5, 0));
	c.append(VFC::Dims(8, 5, 0));
	r.add_planar_polygon(c);
	r.print_all_points();
	r.clear();
	cout << "Expecting ROI with intervals (1,1,0)-(5,1,0)(1,2,0)-(4,2,0)(1,3,0)-(3,3,0)" << endl;
	c.clear();
	c.append(VFC::Dims(3, 3, 0));
	c.append(VFC::Dims(3, 2, 0));
	c.append(VFC::Dims(4, 2, 0));
	c.append(VFC::Dims(5, 1, 0));
	c.append(VFC::Dims(1, 1, 0));
	c.append(VFC::Dims(1, 2, 0));
	c.append(VFC::Dims(1, 3, 0));
	r.add_planar_polygon(c);
	r.print_all_points();
	r.clear();
	*/

	/*
	cout << "Testing subsampled.... " << endl;
	cout << "Expecting empty ROI" << endl;
	ROI rs;
	r.clear();
	r.add_interval(5,5,5,5);
	r.subsample(rs, 4, 4, 4, true);
	rs.print_all_points();
	rs.clear();
	cout << "Expecting ROI: (1,1,1)" << endl;
	r.subsample(rs, 4, 4, 4, false);
	rs.print_all_points();
	rs.clear();
	r.clear();
	cout << "Expecting empty ROI" << endl;
	r.add_interval(3,9,4,4);
	r.subsample(rs, 4, 4, 4, true);
	rs.print_all_points();
	rs.clear();
	cout << "Expecting ROI: (1,1,1)-(2,1,1)" << endl;
	r.subsample(rs, 4, 4, 4, false);
	rs.print_all_points();
	r.clear();
	rs.clear();
	cout << "Expecting ROI with tl=(2,4,2), br=(3,4,3)" << endl;
	r_tl.set(5,11,5);
	r_br.set(14,19,14);
	r.add_box(r_tl, r_br);
	r.subsample(rs, 4, 4, 4, true);
	rs.print_all_points();
	r.clear();
	rs.clear();
	cout << "Expecting ROI with tl=(1,3,1), br=(2,4,2)" << endl;
	r_tl.set(5,11,5);
	r_br.set(7,15,7);
	r.add_box(r_tl, r_br);
	r.subsample(rs, 4, 4, 4, false);
	rs.print_all_points();
	r.clear();
	rs.clear();
	cout << "Expecting empty ROI" << endl;
	r_tl.set(4,4,4);
	r_br.set(6,6,6);
	r.add_box(r_tl, r_br);
	r.subsample(rs, 3, 1, 1, true);
	rs.print_all_points();
	rs.clear();
	cout << "Expecting ROI: (1,4,4)-(2,6,6)" << endl;
	r.subsample(rs, 3, 1, 1, false);
	rs.print_all_points();
	rs.clear();
	r.clear();
	cout << "Expecting points: (4,5,1)(7,24,24)" << endl;
	r_tl.set(10,5,1);
	r_br.set(24,24,24);
	r.add_box(r_tl, r_br);
	r.subsample(rs, 3, 1, 1, true);
	rs.bounding_cube(r_tl, r_br);
	cout << r_tl << r_br << endl;
	rs.clear();
	cout << "Expecting points: (3,5,1)(8,24,24)" << endl;
	r.subsample(rs, 3, 1, 1, false);
	rs.bounding_cube(r_tl, r_br);
	cout << r_tl << r_br << endl;
	rs.clear();
	r.clear();
	*/

	cout << "Testing upsampled.... " << endl;
	ROI rs;
	cout << "Expecting empty ROI" << endl;
	r.clear();
	r.upsample(rs, 4, 4, 4, true);
	rs.print_all_points();
	rs.clear();
	cout << "Expecting points: (1,1,1)(7,7,7)" << endl;
	r.add_interval(1,1,1,1);
	r.upsample(rs, 4, 4, 4, false);
	rs.bounding_cube(r_tl, r_br);
	cout << r_tl << r_br << endl;
	rs.clear();
	cout << "Expecting points: (2,2,2)(6,6,6)" << endl;
	r.upsample(rs, 4, 4, 4, true);
	rs.bounding_cube(r_tl, r_br);
	cout << r_tl << r_br << endl;
	rs.clear();
	r.clear();
	cout << "Expecting ROI: (3,18,4)-(9,22,4)" << endl;
	r.add_interval(3,9,4,4);
	r.upsample(rs, 1, 5, 1, true);
	rs.print_all_points();
	rs.clear();
	r.clear();

	cout << "Done: ROI_unit_test" << endl;
}

