#include "SchedulerKS.h"

float GroupFormerS(Blackboard& bb)
{
	float score=0.0;
	if ((bb.num_sol_elements()>0) && (bb.num_groups()==0))
		score = 0.8;
	return score;
}


void GroupFormerA(Blackboard& bb) {
	// Clear existing groups
	while(bb._group.N()>0)
		bb._group.delete_item(0);

	// Initially create one group for each solution element
	int i;
	for(i=0; i<bb._solel.N(); i++) {
		SEgroup seg;
		seg.add_sol_el(i);
		bb._group.push_last(seg);
	}

	int change=1, j, k, m, n;
	while(change) {
		change = 0;
		for(i=0; (i<bb._group.N()) && !change; i++) {
			for(j=0; (j<bb._group[i].num_sol_els()) && !change; j++) {
				SolElement& se = bb.sol_element(bb._group[i].sol_el_index(j));
				for(k=0; (k<se.num_attributes()) && !change; k++) {
				  const Attribute* att = se.attribute(k);
				  if (att->b_flag())
				    for(m=0; (m<att->num_rel_solels() && !change); m++) {
					int g_ind = bb.find_group(att->rel_solel_index(m));
					if (g_ind!=i) {
						change = 1;
						for(n=0; n<bb._group[g_ind].num_sol_els(); n++)
							bb._group(i).add_sol_el(bb._group[g_ind].sol_el_index(n));
						bb._group.delete_item(g_ind);
					}
				    }
				}
			}
		}
	}
}


float NextGroupS(Blackboard& bb)
{
	float score = 0.0;

	int i, cnt=0;
	for(i=bb.num_act_recs()-1; (i>=0) && (bb.act_rec(i).ks_name().compare("NextSolel")==0); i--)
		cnt++;

	if (bb.last_act_rec() && (bb.last_act_rec()->ks_name().compare("GroupFormer")==0))
		score = 0.2;
	else if ((bb.next_group()>-1) && (cnt>=bb.get_group(bb.next_group()).num_sol_els()))
		score = 0.2;

	return score;
}

void NextGroupA(Blackboard& bb)
{
	if (bb.next_group()>-1) {
		SEgroup& current_group = bb.group(bb.next_group());
		current_group.priority(-1.0);

		// If the stop-at solution element is in the group just processed then set all group priorities to -1.0
		int stop_at_solel_index = bb.sol_element_index(bb.stop_at_node());
		if ((stop_at_solel_index>-1) && current_group.in_group(stop_at_solel_index)) {
			int j;
			for(j=0; j<bb.num_groups(); j++) bb.group(j).priority(-1.0);
		}
	}

	int i, j, k, m, e_flag_fail=0;
	for(j=0; j<bb.num_groups(); j++) {
	  e_flag_fail = 0;
	  if (bb.group(j).priority()!=-1.0) {
		SEgroup& g = bb.group(j);
		int num_related_solels=0, num_processed_rel_solels=0;

		for(i=0; i<g.num_sol_els(); i++) {
			SolElement& se = bb.sol_element(g.sol_el_index(i));
			for(m=0; m<se.num_attributes(); m++) {
				const Attribute* const att = se.attribute(m);
				for(k=0; k<att->num_rel_solels(); k++) {
					int rel_solel_ind = att->rel_solel_index(k);
					if (!g.in_group(rel_solel_ind)) {
						int rel_grp_ind = bb.find_group(rel_solel_ind);
						if (rel_grp_ind!=-1) {
							num_related_solels++;
							if (bb.group(rel_grp_ind).priority()==-1.0)
								num_processed_rel_solels++;
							else if (att->e_flag())
								e_flag_fail = 1;
						}
					}
				}
			}
		}
		if (!num_related_solels)
			g.priority(1.0);
		else if (e_flag_fail)
			g.priority(0);
		else
			g.priority((float)num_processed_rel_solels/num_related_solels);
	  }
	}

	int best_i=-1;
	int best_priority=-1;
	for(i=0; i<bb.num_groups(); i++)
		if (bb.group(i).priority()>best_priority) {
			best_priority = (int)bb.group(i).priority();
			best_i = i;
		}

	bb.next_group(best_i);
	if (best_i>-1)
		bb.next_solel(bb.group(bb.next_group()).sol_el_index(0));
	else
		bb.next_solel(-1);
}


float NextSolelS(Blackboard& bb)
{
	if (bb.next_group()>-1)
		return 0.1;
	else
		return 0;
}


void NextSolelA(Blackboard& bb)
{
	int i;
	SEgroup& g = bb.group(bb.next_group());
	for(i=0; (i<g.num_sol_els()) && (g.sol_el_index(i)!=bb.next_solel()); i++);
	i++;
	if (i>=g.num_sol_els())
		i = 0;
	bb.next_solel(g.sol_el_index(i));
}

