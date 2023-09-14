#include "Plane.h"

ostream& operator<<(ostream& s, const Plane& p)
{
	int i;
	
	s << "z=" << p.z << "\n";
	for(i=0; i<p.ln.N(); i++) {
		s << "\t" << p.ln[i];
	}
	s << "\n";

	return s;
}
