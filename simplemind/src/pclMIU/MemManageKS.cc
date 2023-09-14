#include "MemManageKS.h"

void FreeCandidatesR(Blackboard& bb)
{
	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
	bb.add_message_to_last_act_rec(mess);
}

float FreeCandidatesS(Blackboard& bb)
{
	float score=0.0;

	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
//if (bb.next_solel()==1)
//{
//cout << "FreeCandidatesS: message: " << mess << endl;
//cout << "find_act_rec=" << bb.find_act_rec("MatchCands", "InferencingKS", mess) << endl;
//}
	if ((bb.next_solel()>-1) && bb.find_act_rec("MatchCands", "InferencingKS", mess))
		score = 0.3;
//if (bb.next_solel()==1)
//{
//cout << "score = " << score << endl;
//}

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("FreeCandidates")==0) {
		int mess_ind = bb.act_rec(i).find_message_starting_with("Solution element index:");
		if (mess_ind>-1) {
			const std::string& s = bb.act_rec(i).message(mess_ind);
			int string_ind=0;
			if (advance_to(s, ':', string_ind)) {
				string_ind++;
				int se_ind;
				if (read_integer(s, se_ind, string_ind) && (se_ind==bb.next_solel()))
					score = 0;
			}
		}
	}

	return score;
}

/*
float FreeCandidatesS(const Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).matched_prim())
		score = 0.3;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("FreeCandidates")==0) {
		int mess_ind = bb.act_rec(i).message_starting_with("Solution element index:");
		if (mess_ind>-1) {
			const string& s = bb.act_rec(i).message(mess_ind);
			int string_ind=0;
			if (advance_to(s, ':', string_ind)) {
				string_ind++;
				int se_ind;
				if (read_integer(s, se_ind, string_ind) && (se_ind==bb.next_solel()))
					score = 0;
			}
		}
	}

	return score;
}
*/

void freeCands(SolElement& se, const int se_ind, Blackboard& bb)
{
	int ok = 1;
	ok = ok && (se.find_attribute("RetainCands")==0);

	// Make sure there is not an unmatched solution element that has a SameCandidatesAs feature that references this solution element.
	int i;
	for(i=0; ok && (i<bb.num_sol_elements()); i++) {
		SolElement& sei = bb.sol_element(i);
		const Attribute* a = sei.find_attribute("SameCandidatesAs");
		if (a) {
			for(int j=0; ok && (j<a->num_rel_solels()); j++) {
				int rse_ind = a->rel_solel_index(j);
				if (rse_ind == se_ind) {
					char mess[100];
					sprintf(mess, "Solution element index: %d", i);
					ok = (bb.find_act_rec("MatchCands", "InferencingKS", mess)!=0);
				}
			}
		}
	}

	// Make sure there is not an unmatched solution element that has a DistanceMapRegionGrowing feature that references this solution element.
	for(i=0; ok && (i<bb.num_sol_elements()); i++) {
		SolElement& sei = bb.sol_element(i);
		const Attribute* a = sei.find_attribute("DistanceMapRegionGrowing");
		if (a) {
			int rse_ind = a->rel_solel_index(0);
			if (rse_ind == se_ind) {
				char mess[100];
				sprintf(mess, "Solution element index: %d", i);
				ok = (bb.find_act_rec("MatchCands", "InferencingKS", mess)!=0);
			}
		}
	}

	// Make sure there is not an unmatched solution element that has a AddMatchedCandidates feature that references this solution element.
	for(i=0; ok && (i<bb.num_sol_elements()); i++) {
		SolElement& sei = bb.sol_element(i);
		const Attribute* a = sei.find_attribute("AddMatchedCandidates");
		if (a) {
			for(int j=0; ok && (j<a->num_rel_solels()); j++) {
				int rse_ind = a->rel_solel_index(j);
				if (rse_ind == se_ind) {
					char mess[100];
					sprintf(mess, "Solution element index: %d", i);
					ok = (bb.find_act_rec("MatchCands", "InferencingKS", mess)!=0);
				}
			}
		}
	}

	if (ok) {
		cout << "Freeing candidates of " << se.name() << endl;
		se.free_candidates();
		SEgroup& g = bb.group(bb.next_group());
		/****/
		g.free_candidates();
		/****/
	}
	else {
		cout << "Not freeing candidates of " << se.name() << endl;
	}

	// If the solution element itself has a SameCandidatesAs feature then this knowledge source should also be applied to the related solution element
	const Attribute* a = se.find_attribute("SameCandidatesAs");
	if (a) {
		for(int j=0; j<a->num_rel_solels(); j++) {
			int rse_ind = a->rel_solel_index(j);
			SolElement& se_rse_ind = bb.sol_element(rse_ind);
			freeCands(se_rse_ind, rse_ind, bb);
		}
	}

	// If the solution element itself has a DistanceMapRegionGrowing feature then this knowledge source should also be applied to the related solution element
	a = se.find_attribute("DistanceMapRegionGrowing");
	if (a) {
		int rse_ind = a->rel_solel_index(0);
		SolElement& se_rse_ind = bb.sol_element(rse_ind);
		freeCands(se_rse_ind, rse_ind, bb);
	}

	// If the solution element itself has a AddMatchedCandidates feature then this knowledge source should also be applied to the related solution element
	a = se.find_attribute("AddMatchedCandidates");
	if (a) {
		for(int j=0; j<a->num_rel_solels(); j++) {
			int rse_ind = a->rel_solel_index(j);
			SolElement& se_rse_ind = bb.sol_element(rse_ind);
			freeCands(se_rse_ind, rse_ind, bb);
		}
	}
}

/* OLD
Function for computing activation score for FreeCandidates (3).
Score is 0.3 if: (1) the next solution element has a matched primitive; and (2) the set of activation records, S, does not contain a record of type FreeCandidatesKS and solution element index (message) equal to the next solution element, where, S is formed by including the most recent activation records until NextGroup is found.
*/
/*
void freeCands(SolElement& se, const int se_ind, Blackboard& bb)
{
	int ok = 1;
	ok = ok && (se.find_attribute("RetainCands")==0);

	// Make sure there is not an unmatched solution element that has a SameCandidatesAs feature that references this solution element.
	int i;
	for(i=0; ok && (i<bb.num_sol_elements()); i++) {
		SolElement& sei = bb.sol_element(i);
		if (sei.matched_prim()==0) {
			Attribute* a = sei.find_attribute("SameCandidatesAs");
			if (a) {
				int rse_ind = a->rel_solel_index(0);
				ok = (rse_ind != se_ind);
			}
		}
	}

	if (ok) {
		cout << "Freeing candidates of " << se.name() << endl;
		se.free_candidates();
		SEgroup& g = bb.group(bb.next_group());
		g.free_candidates();
	}

	// If the solution element itself has a SameCandidatesAs feature then this knowledge source should also be applied to the related solution element
	Attribute* a = se.find_attribute("SameCandidatesAs");
	if (a) {
		int rse_ind = a->rel_solel_index(0);
		SolElement& se_rse_ind = bb.sol_element(rse_ind);
		freeCands(se_rse_ind, rse_ind, bb);
	}
}
*/
void FreeCandidatesA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	freeCands(se, bb.next_solel(), bb);
}
