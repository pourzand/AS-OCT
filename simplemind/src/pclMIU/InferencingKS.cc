#include "InferencingKS.h"


void ImCandConfR(Blackboard& bb)
{
	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
	bb.add_message_to_last_act_rec(mess);
}


float ImCandConfS(Blackboard& bb)
{
	float score;

	if ((bb.num_act_recs()>1) && (bb.act_rec(bb.num_act_recs()-1).ks_type().compare("SegmentationKS")==0))
		score = 0.4;
	else
		score = 0.0;

	return score;
}


void ImCandConfA(Blackboard& bb)
{
	SolElement& s = bb.sol_element(bb.next_solel());
	register int i, j, k, ok;
	register float val;//, fuzzy_mem;

	for(i=0; i<s.num_attributes(); i++) {
		const Attribute* a = s.attribute(i);
		if (!strcmp(a->type(), "Feature")) {
			Darray<ImagePrimitive*> prim(2);
			ok = 1;
			for(k=0; (k<a->num_rel_solels()) && ok; k++) {
				SolElement& rs = bb.sol_element(a->rel_solel_index(k));
				if (rs.matched_prim()) {
					prim.push_last((ImagePrimitive* const)rs.matched_prim());
				}
				else
					ok = 0;
			}

			if (ok)
			  for(j=0; j<s.num_candidates(); j++) {
				ImagePrimitive* const s_prim = s.candidate(j)->primitive();
				prim.push_first(s_prim);
				if (((Feature*)a)->value(bb.med_im_seq(), prim, val)) {
					s.candidate(j)->feature_value(i, val);
					s.candidate(j)->conf_score(i, ((Feature*)a)->fuzzy().val(val));
				}
				prim.delete_item(0);
			  }
			else if (a->e_flag())
			  for(j=0; j<s.num_candidates(); j++) {
				s.candidate(j)->feature_value(i, 0);
				s.candidate(j)->conf_score(i, 0);
			  }
		}
	}
}

/*
void RadLogicsCandConfR(Blackboard& bb)
{
	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
	bb.add_message_to_last_act_rec(mess);
}


float RadLogicsCandConfS(const Blackboard& bb)
{
	float score;

	if ((bb.num_act_recs()>1) && (bb.act_rec(bb.num_act_recs()-1).ks_type().compare("SegmentationKS")==0) && (bb.next_solel().find_attribute("RadLogicsNoduleClassification")!=0))
		score = 0.45;
	else
		score = 0.0;

	return score;
}

// Dummy function. Will be provided by RadLogics
//void RunNoduleClassification(double& classification_score, int& classification_result, const short* const image_volume, ROI& roi, double meanHU, double max_diameter, double perp_diameter, double sphericity, const std::string& const node_name) {
//}

void RadLogicsCandConfA(Blackboard& bb)
{
	SolElement& s = bb.sol_element(bb.next_solel());
	register int i, j, k, ok;
	register float val;//, fuzzy_mem;

	for(i=0; i<s.num_attributes(); i++) {
		const Attribute* a = s.attribute(i);
		if (!strcmp(a->type(), "Feature")) {
			Darray<ImagePrimitive*> prim(2);
			ok = 1;
			for(k=0; (k<a->num_rel_solels()) && ok; k++) {
				SolElement& rs = bb.sol_element(a->rel_solel_index(k));
				if (rs.matched_prim()) {
					prim.push_last((ImagePrimitive* const)rs.matched_prim());
				}
				else
					ok = 0;
			}

			if (ok) {
				if (!strcmp(((Feature*)a)->name(), "RadLogicsNoduleClassification")) {
					MedicalImageSequence& mis = bb.med_im_seq();
					for(j=0; j<s.num_candidates(); j++) {
						ImageRegion* const ir = (ImageRegion*) s.candidate(j)->primitive();
						const ROI& roi = ir->roi();
						RoiTraverser rt(roi);
						
						Point mdist_pt1, mdist_pt2, mpdist_pt1, mpdist_pt2, fp;
						double max_diameter, perp_diameter;
						if (roi.first_point(&fp)) 
							compute_diameters(&roi, mis.row_pixel_spacing(fp.z), mis.col_pixel_spacing(fp.z), &mdist_pt1, &mdist_pt2, &mpdist_pt1, &mpdist_pt2, &max_diameter, &perp_diameter);

						int classification_result; // 0,1 values indicating whether nodule or not
						double classification_score;
						RunNoduleClassification(&classification_score, &classification_result, bb.hu_values_column_major(), roi, meanHU(bb.med_im_seq(), rt), max_diameter, perp_diameter, sphericity(ir), s.name());
						
						s.candidate(j)->feature_value(i, classificationResult);
						s.candidate(j)->conf_score(i, ((Feature*)a)->fuzzy().val(classificationResult));
					}
				}
				else {
				  for(j=0; j<s.num_candidates(); j++) {
					ImagePrimitive* const s_prim = s.candidate(j)->primitive();
					prim.push_first(s_prim);
					if (((Feature*)a)->value(bb.med_im_seq(), prim, val)) {
						s.candidate(j)->feature_value(i, val);
						s.candidate(j)->conf_score(i, ((Feature*)a)->fuzzy().val(val));
					}
					prim.delete_item(0);
				  }
				}
			}
			else if (a->e_flag())
			  for(j=0; j<s.num_candidates(); j++) {
				s.candidate(j)->feature_value(i, 0);
				s.candidate(j)->conf_score(i, 0);
			  }
		}
	}
}
*/

float FormGroupCandsS(Blackboard& bb)
{
	float score=0.0;

	if (bb.next_group()>-1) {
		register int i;
		for(i=bb.num_act_recs()-1; (i>=0) && (bb.act_rec(i).ks_name().compare("NextSolel")==0); i--);

		if (((bb.num_act_recs()-1-i)>=bb.group_const(bb.next_group()).num_sol_els()) && (i>=0) && (bb.act_rec(i).ks_name().compare("MatchCands")!=0) && (bb.act_rec(i).ks_name().compare("FreeCandidates")!=0))
			score = 0.4;
	}

	return score;
}


void add_im_cands_to_group_cand(Blackboard& bb, SEgroup& g, GroupCandidate gc, const int se_ind_ind)
{
	if (se_ind_ind>=g.num_sol_els())
		g.add_group_cand(gc);
	else {
		int sol_el_ind = g.sol_el_index(se_ind_ind);
		SolElement& s = bb.sol_element(sol_el_ind);
		register int k;
		register float partial_conf;
		register float orig_conf = gc.conf_score();
		for(k=0; k<s.num_candidates(); k++) {
			partial_conf = s.candidate(k)->partial_confidence();
			if (partial_conf>0) {
				gc.im_cand_index(se_ind_ind, k);
				if (partial_conf<orig_conf)
					gc.conf_score(partial_conf);
				else
					gc.conf_score(orig_conf);

				add_im_cands_to_group_cand(bb, g, gc, se_ind_ind+1);
			}
		}
	}
}


void FormGroupCandsA(Blackboard& bb)
{
	SEgroup& g = bb.group(bb.next_group());

	GroupCandidate gc(g.num_sol_els());
	add_im_cands_to_group_cand(bb, g, gc, 0);
}


float GroupCandConfS(Blackboard& bb)
{
	float score;

	if ((bb.num_act_recs()>1) && (bb.act_rec(bb.num_act_recs()-1).ks_name().compare("FormGroupCands")==0))
		score = 0.4;
	else
		score = 0.0;

	return score;
}


void GroupCandConfA(Blackboard& bb)
{
	SEgroup& g = bb.group(bb.next_group());
	register int i, j, k, m, ok, fail;
	float val, conf;

	for(m=0; m<g.num_sol_els(); m++) {
		SolElement& s = bb.sol_element(g.sol_el_index(m));

		for(i=0; i<s.num_attributes(); i++) {
			const Attribute* a = s.attribute(i);
			if (!strcmp(a->type(), "Feature")) {
				ok = 0, fail = 0;
				for(k=0; (k<a->num_rel_solels()) && !fail; k++) {
					SolElement& rs = bb.sol_element(a->rel_solel_index(k));
					if (g.in_group(a->rel_solel_index(k)))
						ok = 1;
					else if (!rs.matched_prim())
						fail = 1;
				}

				if (ok) {
			  		for(j=0; j<g.num_group_cands(); j++) {
						GroupCandidate& gc = g.group_cand(j);
						Darray<ImagePrimitive*> prim(2);
						ImagePrimitive* const s_prim = s.candidate(gc.im_cand_index(m))->primitive();
						prim.push_last(s_prim);

						for(k=0; k<a->num_rel_solels(); k++) {
							SolElement& rs = bb.sol_element(a->rel_solel_index(k));
							if (rs.matched_prim()) {
								prim.push_last((ImagePrimitive* const)rs.matched_prim());
							}
							else {
								int rs_grp_ind = g.find_solel_grp_ind(a->rel_solel_index(k));
								if (rs_grp_ind>-1) {
								   if (gc.im_cand_index(rs_grp_ind)>-1) {
									ImagePrimitive* const related_prim = bb.sol_element(a->rel_solel_index(k)).candidate(gc.im_cand_index(rs_grp_ind))->primitive();
									prim.push_last(related_prim);
								   }
								}
								else {
									cerr << "ERROR: BestGroupCandA: Cannot figure out what to do with solution element" << endl;
									exit(1);
								}
							}

							if (((Feature*)a)->value(bb.med_im_seq(), prim, val)) {
								conf = ((Feature*)a)->fuzzy().val(val);
								if (conf<gc.conf_score())
									gc.conf_score(conf);
							}
						}
					}
				}
			}
		}
	}
}


void MatchCandsR(Blackboard& bb)
{
	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
	bb.add_message_to_last_act_rec(mess);
	
	/*
	if (bb.next_solel()==bb.sol_element_index("right_main_bronchus")) {
		cout << endl << "MatchCandsR for right_main_bronchus **************************************" << endl;
		exit(1);
	}
	*/
}


float MatchCandsS(Blackboard& bb)
{
	float score;

	if ((bb.num_act_recs()>1) && (bb.act_rec(bb.num_act_recs()-1).ks_name().compare("GroupCandConf")==0))
		score = 0.4;
	else
		score = 0.0;

	return score;
}


void MatchCandsA(Blackboard& bb)
{
	SEgroup& g = bb.group(bb.next_group());
	if (g.num_group_cands()>0) {
		register int i, j, best_group_cand=-1;
		register float best_group_conf=0;

		// Determine best group candidate (the one with the highest confidence)
		for(j=0; j<g.num_group_cands(); j++) {
			if (g.group_cand(j).conf_score()>best_group_conf) {
				best_group_cand = j;
				best_group_conf = g.group_cand(j).conf_score();
			}
		}

		if (best_group_cand>-1) {
			GroupCandidate& bgc = g.group_cand(best_group_cand);
			for(i=0; i<g.num_sol_els(); i++) {
				SolElement& s = bb.sol_element(g.sol_el_index(i));
				const Attribute* const a = s.find_attribute("MatchAboveConf");

				if (a) {
					register float conf_thresh = ((MatchAboveConf*)a)->conf_thresh();
					for(j=0; j<g.num_group_cands(); j++)
						if (g.group_cand(j).conf_score()>conf_thresh) {
						s.append_matched_cand_index(g.group_cand(j).im_cand_index(i));
						}
					int num_cands;
					s.num_matched_cands(num_cands);
					//cout << "MatchCandsA" << endl;
					//cout << "num_group_cands = " << g.num_group_cands() << endl;
					//cout << "num_matched_cands = " << num_cands << endl;
					if (num_cands) {
						s.create_matched_prim(bb.med_im_seq());		
						//cout << "num voxels = " << ((ImageRegion*)s.matched_prim())->roi().num_pix() << endl;
					}
				}

				else if (bgc.im_cand_index(i)>-1) {
					s.append_matched_cand_index(bgc.im_cand_index(i));
					s.create_matched_prim(bb.med_im_seq());
				}
			}
		}
	}
}


