#include "KStools.h"

const int all_rel_solels_identified(Blackboard& bb, const Attribute* const a)
{
	int j, ok = 1;
	for(j=0; (j<a->num_rel_solels()) && ok; j++)
		ok = (bb.sol_element(a->rel_solel_index(j)).matched_prim()!=0);

	return ok;
}
