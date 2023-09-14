#include "ROIworkspace.h"


ROIworkspace::ROIworkspace() : pi(0), li(0), ii(0), xi(0) {};

ROIworkspace::ROIworkspace(	const long pi_init,
				const long li_init,
				const long ii_init,
				const long xi_init)
		: pi(pi_init), li(li_init), ii(ii_init), xi(xi_init) {};


void ROIworkspace::zero(){
	pi = li = ii = xi = 0;
}
