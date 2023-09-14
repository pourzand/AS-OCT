#include "SegmentationKS.h"
#include <sys/stat.h>
#include <boost/filesystem.hpp>

#define MIN2(A, B) (((A) < (B)) ? (A) : (B))
#define MIN3(A, B, C) MIN2(MIN2(A, B), C)
#define MIN4(A, B, C, D) MIN2(MIN3(A, B, C), D)
#define MIN5(A, B, C, D, E) MIN3(MIN3(A, B, C), D, E)
#define MAX2(A, B) (((A) > (B)) ? (A) : (B))
#define MAX3(A, B, C) MAX2(MAX2(A, B), C)
#define MAX4(A, B, C, D) MAX2(MAX3(A, B, C), D)
#define MAX5(A, B, C, D, E) MAX3(MAX3(A, B, C), D, E)

using std::flush;

void SegmentationR(Blackboard& bb)
{
	char mess[100];
	sprintf(mess, "Solution element index: %d", bb.next_solel());
	bb.add_message_to_last_act_rec(mess);
}


/*
Computes search area for next solution element to be processed.
Optional argument: ph_area - computes preprocessing histogram ROI (combines IntensitySubregion matched primitives by OR).
The use_subsampled argument is set to 1 if an attribute with name UseSubSampledImage is found (set to 0 otherwise).
The ss_factor argument is the scale factors (steps) to be used for subsampling in the x, y and z- directions.
The stop argument is set to 1 if an attribute that is essential (_E) does not have a primitive matched (set to 0 otherwise).
The segment_2d argument is set to 1 if an attribute with name SegmentIn2D is found (set to 0 otherwise).
The min_num_vox argument is set if a DoNotFormCandsWith_NumVoxelsLessThan parameter is found (set to 0 otherwise); the returned value is scaled according to the subsampling factors.
The include_all_vox argument is set to 1 if an attribute with name IncludeAllVoxels is found (set to 0 otherwise).
*/
void compute_segmentation_parameters(ROI& search_area, int& use_subsampled, Point& ss_factor, int& stop, int& segment_2d, int& min_num_vox, int& include_all_vox, Blackboard& bb, ROI& ph_area)
{
//cout << "in compute_segmentation_parameters" << endl;
	SolElement& se = bb.sol_element(bb.next_solel());

	int i, j;

	use_subsampled = 0;
	ss_factor.x = ss_factor.y = ss_factor.z = 1;
	for(i=0; (i<se.num_attributes() && !use_subsampled); i++) {
		const Attribute* const a = se.attribute(i);
		if (!strcmp(a->name(), "UseSubSampledImage")) {
			use_subsampled = 1;
			ss_factor.x = ((UseSubSampledImage *)a)->x_step();
			ss_factor.y = ((UseSubSampledImage *)a)->y_step();
			ss_factor.z = ((UseSubSampledImage *)a)->z_step();
			cout << "Using subsampled image: " << ss_factor.x << " " << ss_factor.y << " " << ss_factor.z << endl;
		}
	}

	if (use_subsampled)
		subsample_roi(bb.overall_search_area(), search_area, ss_factor.x, ss_factor.y, ss_factor.z);
	else
		search_area.OR(bb.overall_search_area());

	stop=0;
	segment_2d=0;
	include_all_vox=0;
	min_num_vox = 0;
	for(i=0; (i<se.num_attributes()) && !stop; i++) {
		const Attribute* const a = se.attribute(i);
		if (!strcmp(a->type(), "SearchArea")) {
		    if (all_rel_solels_identified(bb, a)) {
			Darray<ImagePrimitive*> prim(1);

			cout << "Using " << a->name();
			if (((SearchArea*)a)->or_flag())
				cout << "_OR";
			for(j=0; (j<a->num_rel_solels()); j++) {
				SolElement& rel_se = bb.sol_element(a->rel_solel_index(j));
				prim.push_last((ImagePrimitive* const)rel_se.matched_prim());
				cout << " " << rel_se.name();
			}
			cout << ".... " << flush;

			((SearchArea*)a)->search_area(bb.med_im_seq(), ss_factor, prim, search_area);
			cout << "done" << endl;
		    }
		    else if (a->e_flag()) {
			cout << "Could not use " << a->name() << endl;
			if  (a->num_rel_solels()>1)
				cout << "One of ";
			for(j=0; (j<a->num_rel_solels()); j++)
				cout << bb.sol_element(a->rel_solel_index(j)).name() << " ";
			cout << "not found" << endl;
			cout << "Generate no candidates for " << se.name() << endl;

			i = se.num_attributes();
			stop=1;
		    }
		}
		if (!strcmp(a->type(), "SegParam") && !strcmp(a->name(), "IntensitySubregion")) {
		    if (all_rel_solels_identified(bb, a)) {
				Darray<ImagePrimitive*> prim(1);
				
				cout << "Using " << a->name();
				for(j=0; (j<a->num_rel_solels()); j++) {
					SolElement& rel_se = bb.sol_element(a->rel_solel_index(j));
					prim.push_last((ImagePrimitive* const)rel_se.matched_prim());
					cout << " " << rel_se.name();
				}
				cout << ".... " << flush;
				
				if ((prim.N() == 1) && !strcmp(prim[0]->type(), "ImageRegion")) {
					ImageRegion* r0 = (ImageRegion*)prim[0];
					ph_area.OR(r0->roi());
				}
				cout << "done" << endl;
		    }
		    else if (a->e_flag()) {
				cout << "Could not use " << a->name() << endl;
				if  (a->num_rel_solels()>1)
					cout << "One of ";
				for(j=0; (j<a->num_rel_solels()); j++)
					cout << bb.sol_element(a->rel_solel_index(j)).name() << " ";
				cout << "not found" << endl;
				cout << "Generate no candidates for " << se.name() << endl;

				i = se.num_attributes();
				stop=1;
		    }
		}
		else if (!strcmp(a->type(), "SegParam") && !strcmp(a->name(), "SegmentIn2D")) {
			segment_2d = 1;
		}
		else if (!strcmp(a->type(), "SegParam") && !strcmp(a->name(), "IncludeAllVoxels")) {
			include_all_vox = 1;
		}
		else if (!strcmp(a->type(), "SegParam") && !strcmp(a->name(), "MinNumVoxels"))
			min_num_vox = ((MinNumVoxels*)a)->minNumVoxels();

//cout << "num vox in SA = " << search_area.num_pix() << endl;
	}
//cout << "finished using attributes" << search_area.num_pix() << endl;

	// Do this crop to make sure that a dilation did not push the ROI outside the image bounds
	if (!stop) {
		Point crop_tl(0,0,0), crop_br(bb.med_im_seq().xdim()-1, bb.med_im_seq().ydim()-1, bb.med_im_seq().zdim()-1);
		search_area.crop(crop_tl, crop_br);
	}

	if (min_num_vox)
		cout << "Minimum number of voxels required to form a candidate is " << min_num_vox << endl;

	// Scale minimum number of voxels to account for subsampling
	min_num_vox = min_num_vox/(ss_factor.x*ss_factor.y*ss_factor.z);

	// save search area in here as a temporary solution.
	// consider to update the se instead and save it outside in the future
	std::string search_area_path;
	search_area_path = search_area_path + bb.temp_file_path() + "/search_area_" + se.name() + ".roi";
	cout << "Writing search area roi to " << search_area_path << " ..." << endl;
	ofstream searcharearoiFile(search_area_path);
	if (!searcharearoiFile) searcharearoiFile.open(search_area_path);
	searcharearoiFile<<search_area;
	searcharearoiFile.close();
	cout << "Done" << endl;

//cout << "out compute_segmentation_parameters" << endl;
}
void compute_segmentation_parameters(ROI& search_area, int& use_subsampled, Point& ss_factor, int& stop, int& segment_2d, int& min_num_vox, int& include_all_vox, Blackboard& bb)
{
	ROI ph_area;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb, ph_area);
}


float AddMatchedCandidatesS(Blackboard& bb)
{
    float score=0.0;
    if (bb.next_solel()>-1) {
	const Attribute* sca=bb.sol_element(bb.next_solel()).find_attribute("AddMatchedCandidates");
	if (sca) {
		int sca_ind = sca->rel_solel_index(0);
		int sca_grp = bb.find_group(sca_ind);

		if (sca_grp==-1) {
			cerr << "ERROR: SegmentationKS: AddMatchedCandidates: could not find group of related solution element" << endl;
			exit(1);
		}

		if (sca_grp!=bb.next_group()) {
			if (bb.group_const(sca_grp).priority()==-1.0) {
				score = 0.65;
				for(int i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   	  			if ((bb.act_rec(i).ks_type().compare("SegmentationKS")==0) && (bb.act_rec(i).ks_name().compare("AddMatchedCandidates")==0)) {
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
			}
		}
		else {
			score = 0.65;
			int found=0;
			for(int i=bb.num_act_recs()-1; (score>0) && (!found) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   		  if (bb.act_rec(i).ks_type().compare("SegmentationKS")==0) {
				int mess_ind = bb.act_rec(i).find_message_starting_with("Solution element index:");
				if (mess_ind>-1) {
					const std::string& s = bb.act_rec(i).message(mess_ind);
					int string_ind=0;
					if (advance_to(s, ':', string_ind)) {
						string_ind++;
						int se_ind;
						if (read_integer(s, se_ind, string_ind)) {
							if ((bb.act_rec(i).ks_name().compare("AddMatchedCandidates")==0) && (se_ind==bb.next_solel()))
								score = 0;
							else if (se_ind==sca_ind)
								found = 1;
						}
					}
				}
			}
			if (!found)
				score = 0;
		}
	}
    }

    return score;
}

void AddMatchedCandidatesA(Blackboard& bb) {
	SolElement& s = bb.sol_element(bb.next_solel());
	const Attribute* sca=s.find_attribute("AddMatchedCandidates");

	if (sca) {
		cout << endl << endl << "Segmenting " << s.name() << " with AddMatchedCandidates ";

		for(int j=0; j<sca->num_rel_solels(); j++) {
			SolElement& rel_se = bb.sol_element(sca->rel_solel_index(j));
			cout << rel_se.name() << " ";

            int nmc;
            int matchFound = rel_se.num_matched_cands(nmc);
			for(int i=0; matchFound && (i<nmc); i++)
				s.add_candidate(rel_se.candidate(rel_se.matched_cand_index(i))->primitive()->create_copy());
		}

		cout << endl;
	}
	else {
		cerr << "ERROR: SegmentationKS: AddMatchedCandidates: unable to find AddMatchedCandidates attribute" << endl;
		exit(1);
	}
	//cout << "Number of candidates = " << s.num_candidates() << endl;
}



float GrowPartSolidS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("GrowPartSolid"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("GrowPartSolid")==0) {
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

float* readEDM(const int imSize, const char prefixChar1, const char prefixChar2, const Blackboard& bb) {
//float* readEDM(const char* const series_instance_uid, const int imSize, const char prefixChar1, const char prefixChar2, const Blackboard& bb) {
	char* edmOutputFileName = new char [500];
	if (bb.edm_directory().length()==0) {
		sprintf(edmOutputFileName, "%s/%c%c_distanceMapEDM.raw", bb.temp_file_path(), prefixChar1, prefixChar2);
		//sprintf(edmOutputFileName, "%s/%s/%c%c_distanceMapEDM.raw", bb.temp_file_path(), series_instance_uid, prefixChar1, prefixChar2);
	}
	else {
		const ActivationRecord* ar = bb.find_act_rec("DistanceMapWatershed", "SegmentationKS", "Writing EDM files");
		const std::string s1 = ar->message(ar->find_message_starting_with("EDM Num Pix"));
		const std::string s2 = ar->message(ar->find_message_starting_with("EDM Index"));
		int edm_num_pix, edm_index;
		sscanf (s1.c_str(), "%*s %*s %*s %d", &edm_num_pix);
		sscanf (s2.c_str(), "%*s %*s %d", &edm_index);		
		sprintf(edmOutputFileName, "%s/%c%c_distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), prefixChar1, prefixChar2, edm_num_pix, edm_index);
		//sprintf(edmOutputFileName, "%s/%s/%c%c_distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), series_instance_uid, prefixChar1, prefixChar2, edm_num_pix, edm_index);
	}

	float* edm = new float [imSize];
	ifstream edmResultFile (edmOutputFileName,ifstream::binary);
	edmResultFile.read ((char*)edm,imSize*sizeof(float));
	edmResultFile.close();
	
	delete [] edmOutputFileName;
	return edm;
}
		
float* readEDM_DMWS(int imSize, Blackboard& bb) {
//float* readEDM_DMWS(const char* const series_instance_uid, const int imSize, const Blackboard& bb) {
	char* edmOutputFileName = new char [500];

	const ActivationRecord* ar = bb.find_act_rec("DistanceMapWatershed", "SegmentationKS", "Writing EDM files");
	const std::string s1 = ar->message(ar->find_message_starting_with("EDM Num Pix"));
	const std::string s2 = ar->message(ar->find_message_starting_with("EDM Index"));
	int edm_num_pix, edm_index;
	sscanf (s1.c_str(), "%*s %*s %*s %d", &edm_num_pix);
	sscanf (s2.c_str(), "%*s %*s %d", &edm_index);		
	sprintf(edmOutputFileName, "%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), edm_num_pix, edm_index);
	//sprintf(edmOutputFileName, "%s/%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), series_instance_uid, edm_num_pix, edm_index);

	float* edm = new float [imSize];
	ifstream edmResultFile (edmOutputFileName,ifstream::binary);
	edmResultFile.read ((char*)edm,imSize*sizeof(float));
	edmResultFile.close();

	delete [] edmOutputFileName;
	return edm;
}

void getMaxDist(float* edm, ROItraverser& rt, float& max, Point& max_pt, const int xdim, const int ydim) {
	TravStatus s = rt.reset();
	Point p1, p2;
	max = 0;
	int j;
	while(s<END_ROI) {
		rt.current_interval(p1, p2);
		j=p1.z*xdim*ydim+p1.y*xdim+p1.x;
		for(int xi=p1.x; xi<=p2.x; xi++) {
			if (edm[j]>=max) {
				max=edm[j];
				max_pt = p1; max_pt.x = xi;
			}
			++j;
		}
		s = rt.next_interval();
	}
}
			

char* createBinaryMask(ROItraverser& sat, const int imSize, const int xdim, const int ydim) {
	char* binaryMaskSA = new char [imSize];
	TravStatus sas = sat.reset();
	Point sap1, sap2;
	while(sas<END_ROI) {
		sat.current_interval(sap1, sap2);
		int j = sap1.z*xdim*ydim + sap1.y*ydim + sap1.x;
		for(; sap1.x<=sap2.x; sap1.x++) {
			// Set voxel in char image
			binaryMaskSA[j] = (char) 1;
			j++;
		}
		sas = sat.next_interval();
	}
	return binaryMaskSA;
}


void dmRegGrow(float* edm, Point& max_pt, float max, float perc, int xdim, int ydim, int zdim, float xsize, float ysize, float zsize, ROI& search_area, ROI& lresult) {
	// Compute distance threshold
	float thresh_low;
	float thresh_high;
	thresh_low = max*(1.0-(perc/100.0));
	thresh_high = max*(1.0+(perc/100.0));
//int debug=((max_pt.z>=140) && (max_pt.z<=145) && (max_pt.x>=320) && (max_pt.x<=330) && (max_pt.y>=340) && (max_pt.y<=345));
//if  (debug) {
//cout << "Max_pt=" << max_pt  << "    max: " << max << "  thresh_low: " << thresh_low  << "   thresh_high: " << thresh_high << endl;
//}
			
	// Determine bounding box range as 30mm in each direction from seed point
	int x1 = max_pt.x - 30/xsize;
	if (x1<0) x1=0;
	int x2 = max_pt.x + 30/xsize;
	if (x2>=xdim) x2=xdim-1;
	int y1 = max_pt.y - 30/ysize;
	if (y1<0) y1=0;
	int y2 = max_pt.y + 30/ysize;
	if (y2>=ydim) y2=ydim-1;
	int z1 = max_pt.z - 30/zsize;
	if (z1<0) z1=0;
	int z2 = max_pt.z + 30/zsize;
	if (z2>=zdim) z2=zdim-1;				
				
	// Within bounding box scan EDM in raster order and append points that meet distance criteria to a meetsDistanceCriteria ROI
	ROI meetsDistanceCriteria;
	Point pedm;
	for (pedm.z=z1; pedm.z<=z2; pedm.z++) {
		for (pedm.y=y1; pedm.y<=y2; pedm.y++) {
			int jrow=pedm.z*xdim*ydim + pedm.y*xdim;
			for (pedm.x=x1; pedm.x<=x2; pedm.x++) {
				if ((edm[jrow+pedm.x]>=thresh_low) && (edm[jrow+pedm.x]<=thresh_high)) {
					meetsDistanceCriteria.append_point(pedm);
				}
			}
		}
	}

//if (debug) cout << "meetsDistanceCriteria num points = " << meetsDistanceCriteria.num_pix() << endl;
							
	// Form a result ROI by calling add_contig_3d method with meetsDistanceCriteria ROI and seed point as arguments			
	ROI dmRegion;
	dmRegion.add_contig_3d(meetsDistanceCriteria, max_pt);
//if (debug) cout << "dmRegion num points = " << dmRegion.num_pix() << endl;

Point bound1, bound2;
if (dmRegion.first_point(bound1) && dmRegion.last_point(bound2) && (bound1.z!=z1) && (bound1.z!=z2)) { 

	//lresult.OR(dmRegion);
	/*******/
	// Since the region growing stops when the distance map value goes below low_thresh the region will not extend to the boundary of the nodule. 
	// Therefore, dilation of the region is performed with a structuring element of half-width = low_thresh.
	// Then an AND operation is performed with the initial thresholded region.
	int seXD = thresh_low/xsize+1;
	int seYD = thresh_low/ysize+1;
	int seZD = thresh_low/zsize+1;
	Point dilSeTL(-seXD, -seYD, -seZD);
	Point dilSeBR(seXD, seYD, seZD);
	ROI dilSE;
	dilSE.add_box(dilSeTL, dilSeBR);
	dmRegion.dilate(dilSE);

	dmRegion.AND(search_area);
	lresult.add_contig_3d(dmRegion, max_pt);
	/*****/
	

//if (debug) cout << "lresult num points = " << lresult.num_pix() << endl;
//if (debug) lresult.print_all_points();
}
//else {
//	if (dmRegion.first_point(bound1)) cout << "dmRegGrow NO1" << "   " << thresh_low << "   " << thresh_high << "   " << dmRegion.num_pix() << endl;
//	else cout << "dmRegGrow NO2" << endl;
//}
}				
				
			
void GrowPartSolidA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		const GrowPartSolid* const dma = (GrowPartSolid*) se.find_attribute("GrowPartSolid");

		// Get matched region of related solution element
		SolElement& rs = bb.sol_element(dma->rel_solel_index(0));
		ImagePrimitive* rel_prim = rs.matched_prim();
		
		// Get percentage for thresholding
		// When performing region growing from the seed with distance
		float perc = dma->percentage();
		
		if (rel_prim && !strcmp(rel_prim->type(), "ImageRegion")) {
			cout << "Segmenting using GrowPartSolid....." << endl;
			
			MedicalImageSequence& medseq = bb.med_im_seq();
			int xdim = medseq.xdim();
			int ydim = medseq.ydim();
			int zdim = medseq.zdim();
			int imSize = xdim*ydim*zdim;
			float xsize = medseq.row_pixel_spacing(0);
			float ysize = medseq.column_pixel_spacing(0);
			float zsize = fabs(medseq.slice_location(1)-medseq.slice_location(0));
			
			// Get connected components from sd_nodule_init (related sol element).
			ROI rel_region(((ImageRegion*)rel_prim)->roi());
			rel_region.AND(search_area);
			Point rrPt;
			ROI partSolidROI;
			Darray<Point*> seedList(20);
			ROI sel;
			sel.add_circle(15/xsize, 0, 0);

			float* originalEDM;
			if (bb.edm_directory().length()==0) {
				originalEDM = readEDM(imSize, rs.name()[0], rs.name()[1], bb);
				//originalEDM = readEDM(medseq.series_instance_uid(), imSize, rs.name()[0], rs.name()[1], bb);
			}
			else {
				originalEDM = readEDM_DMWS(imSize, bb);
				//originalEDM = readEDM_DMWS(medseq.series_instance_uid(), imSize, bb);
			}

			// For each connected component of sd_nodule_init (related sol element):
			// Find the maximum EDM point and add it to seedList
			// Find the median HU (mhu)
			// Adaptive threshold (HU) = -1000 + (mhu - (-1000))/2
			// Perform adaptive thresholding and add the result to partSolidROI (using pixel OR)
			while (rel_region.first_point(rrPt)) {
				ROI comp;
				comp.add_contig_3d(rel_region, rrPt, 1);

				ROItraverser ccRT(comp);
				float max;
				Point* max_pt = new Point ();
				getMaxDist(originalEDM, ccRT, max, *max_pt, xdim, ydim);
				seedList.push_last(max_pt);
								
				int mhu = medianHU(medseq, ccRT);
					
				comp.dilate(sel);					
				comp.AND(search_area);
					
				int adaptiveThresholdGL;
				HU_to_GL(mhu/2 - 500, adaptiveThresholdGL, medseq);

				ccRT.reinitialize(comp);
				register Point p;
				register short gl;
				register TravStatus s = ccRT.reset();
				ROI thresh_res;
				register Point p1, p2;
				register int x1, x2;
				// Increment by interval rather than point because it is faster
				while(s<END_ROI) {
					ccRT.current_interval(p1, p2);
					x1 = p2.x+1;
					for(; p1.x<=p2.x; p1.x++) {
						gl = medseq.fast_pix_val(p1.x, p1.y, p1.z);
						if (gl>=adaptiveThresholdGL) {
							if (x1>p2.x) x1=p1.x;
						}
						else {
							x2 = p1.x-1;
							if (x2>=x1) thresh_res.append_interval(x1, x2, p1.y, p1.z);
							x1 = p2.x+1;
						}
					}
					if (x1<=p2.x)
						thresh_res.append_interval(x1, p1.x-1, p1.y, p1.z);
					s = ccRT.next_interval();
		    		}
		    			
		    		partSolidROI.OR(thresh_res);
			}
				
			ROI seSmall;
			seSmall.add_circle(1/xsize, 0, 0);
			partSolidROI.dilate(seSmall);
			partSolidROI.erode(seSmall);
			
			char* edmInputMaskFileName = new char [500];
			char* edmOutputFileName = new char [500];
			//char* tempFilePath = new char [500];
			//sprintf(tempFilePath, "%s/%s", bb.temp_file_path(), medseq.series_instance_uid());			

			// Create binary mask of the part solid ROI for input to the distance transform
			ROItraverser sat(partSolidROI);
			char* binaryMaskSA = createBinaryMask(sat, imSize, xdim, ydim);	

			bool exists = false;
			if (bb.edm_directory().length()==0) {
				sprintf(edmInputMaskFileName, "%s/ps_searchAreaEDM.raw", bb.temp_file_path());
				//sprintf(edmInputMaskFileName, "%s/ps_searchAreaEDM.raw", tempFilePath);
				sprintf(edmOutputFileName, "%s/ps_distanceMapEDM.raw", bb.temp_file_path());
				//sprintf(edmOutputFileName, "%s/ps_distanceMapEDM.raw", tempFilePath);
			}
			else {
				int num_pix = partSolidROI.num_pix();
				char* edmInputMaskFileNamePrefix = new char [500];
				sprintf(edmInputMaskFileNamePrefix, "%s/searchAreaEDM_%d_", bb.edm_directory().c_str(), num_pix);
				//sprintf(edmInputMaskFileNamePrefix, "%s/%s/searchAreaEDM_%d_", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix);

				int edm_index=-1;
				bool fileToCheck = false;
				do {
					edm_index++;
					sprintf(edmInputMaskFileName, "%s%d.raw", edmInputMaskFileNamePrefix, edm_index);
					ifstream existingMaskFile (edmInputMaskFileName,ifstream::binary);
					fileToCheck = !existingMaskFile.fail();
					if (fileToCheck) {
						//Read and compare the existing binary mask file with the current binary mask
						char* existingBinaryMask = new char [imSize];
						existingMaskFile.read ((char*)existingBinaryMask,imSize);
						existingMaskFile.close();
						exists = (memcmp (binaryMaskSA, existingBinaryMask, imSize)==0);
						delete [] existingBinaryMask;
					}
				} while (fileToCheck && !exists);

				sprintf(edmOutputFileName, "%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), num_pix, edm_index);
				//sprintf(edmOutputFileName, "%s/%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix, edm_index);

				cout << "edmInputMaskFileName = " << edmInputMaskFileName << endl;
			}

			if (!exists) { // Need to recompute EDM files	
	
				ofstream outfileBinaryMaskSA (edmInputMaskFileName,ofstream::binary);
				outfileBinaryMaskSA.write(binaryMaskSA,imSize);
				outfileBinaryMaskSA.close();

				// Compute Euclidean Distance Map
				char command[5000];

				boost::filesystem::path full_path(boost::filesystem::current_path());

				// ifstream distanceTransformExecutable ("./MyDistanceTransform.exe",ifstream::binary);
				ifstream distanceTransformExecutable (full_path.string()+"/MyDistanceTransform.exe",ifstream::binary);

				#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
				char wine[]= "";
				#else
				char wine[] ="wine";
				#endif

				bool dtExists = !distanceTransformExecutable.fail();
				if (dtExists) {
					sprintf (command, "%s %s/MyDistanceTransform.exe %s %s -s %d %d %d -v %f %f %f", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, xdim, ydim, zdim, xsize, ysize, zsize);
					// sprintf (command, "%s %s/MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s;", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
					// sprintf (command, "./MyDistanceTransform.exe %s %s -s %d %d %d -v %f %f %f", edmInputMaskFileName, edmOutputFileName, xdim, ydim, zdim, xsize, ysize, zsize);
				}
				else {
					sprintf (command, "Q:\\nodule_detection\\Executables\\MyDistanceTransform.exe %s %s -s %d %d %d -v %f %f %f", edmInputMaskFileName, edmOutputFileName, xdim, ydim, zdim, xsize, ysize, zsize);
				}

				int retVal = system(command);
				cout << command << endl << "MyDistanceTransform completed with return value = " << retVal << endl;
			}	

			float* edm2;
			if (bb.edm_directory().length()==0) {
				edm2 = readEDM(imSize, 'p', 's', bb);
				//edm2 = readEDM(medseq.series_instance_uid(), imSize, 'p', 's', bb);
			}
			else {
				edm2 = new float [imSize];
				ifstream edmResultFile (edmOutputFileName,ifstream::binary);
				edmResultFile.read ((char*)edm2,imSize*sizeof(float));
				edmResultFile.close();
			}

			for(int k=0; k<seedList.N(); k++) {
				ROI lresult;
				Point* mpt = seedList[k];
				dmRegGrow(edm2, *mpt, edm2[mpt->z*xdim*ydim+mpt->y*xdim+mpt->x], perc, xdim, ydim, zdim, xsize, ysize, zsize, partSolidROI, lresult);
				if (!lresult.empty()) {
					ImageRegion *ir = new ImageRegion (lresult, bb.med_im_seq());
					se.add_candidate(ir);
					if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
				}
				delete mpt;
			}
			
			delete [] binaryMaskSA;
			//delete [] tempFilePath;
			delete [] edmInputMaskFileName;
			delete [] edmOutputFileName;
			delete [] edm2;	
		}
	}
	cout << "done" << endl;
}



float PlatenessThreshRegGrowS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("PlatenessThreshRegGrow"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("PlatenessThreshRegGrow")==0) {
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


void PlatenessThreshRegGrowA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with PlatenessThreshRegGrow" << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		ROItraverser rt(search_area);
		register TravStatus s = rt.valid();

		const PlatenessThreshRegGrow* const pla = (PlatenessThreshRegGrow*) se.find_attribute("PlatenessThreshRegGrow");
		register float low = pla->low();
		register float high = pla->high();
			
		int xdim = medseq.xdim();
		int ydim = medseq.ydim();
		int zdim = medseq.zdim();
		int xySize = xdim*ydim;
		int imSize = xdim*ydim*zdim;
		
		float xsize = medseq.row_pixel_spacing(0);
		float ysize = medseq.column_pixel_spacing(0);
		float zsize = fabs(medseq.slice_location(1)-medseq.slice_location(0));
			
		// Read the axial plateness raw image file
		if (bb.roi_directory().length()==0) {
			cerr << "ERROR: PlatenessThreshRegGrowA: no ROI directory specified for the raw plateness image file" << endl;
			exit(1);
		}
		char* pltnsFileName = new char [500];
		sprintf(pltnsFileName, "%s/plateness_axial.raw", bb.roi_directory().c_str());
		//sprintf(pltnsFileName, "%s\\plateness_axial.raw", bb.roi_directory().c_str());
		float* pltns = new float [imSize];
		ifstream pltnsFile (pltnsFileName,ifstream::binary);
		pltnsFile.read ((char*)pltns,imSize*sizeof(float));
		pltnsFile.close();

		sprintf(pltnsFileName, "%s/plateness_coronal.raw", bb.roi_directory().c_str());	
		//sprintf(pltnsFileName, "%s\\plateness_coronal.raw", bb.roi_directory().c_str());
		float* pltns_add = new float [imSize];
		pltnsFile.open (pltnsFileName,ifstream::binary);
		pltnsFile.read ((char*)pltns_add,imSize*sizeof(float));
		pltnsFile.close();
		int i;
		for(i=0; i<imSize; i++) pltns[i] += pltns_add[i];

		sprintf(pltnsFileName, "%s/plateness_sagittal.raw", bb.roi_directory().c_str());
		//sprintf(pltnsFileName, "%s\\plateness_sagittal.raw", bb.roi_directory().c_str());	
		pltnsFile.open (pltnsFileName,ifstream::binary);
		pltnsFile.read ((char*)pltns_add,imSize*sizeof(float));
		pltnsFile.close();
		for(i=0; i<imSize; i++) pltns[i] += pltns_add[i];
		delete [] pltns_add;

		float pmax = 0;
		for(i=0; i<imSize; i++)
			if (pltns[i] > pmax) pmax = pltns[i];
		//cout << "pmax=" << pmax << endl;

		register Point p;
		register float v;
		ROI thresh_res;

		cout << "Thresholding....." << endl;
		register Point p1, p2;
		register int x1, x2;

		//Ignores use_subsampled
		/*
		int i=0;
		for (p.z=0; p.z<zdim; p.z++)
			for (p.y=0; p.y<ydim; p.y++)
				for (p.x=0; p.x<xdim; p.x++) {
					if ((pltns[i]>=m) && (pltns[i]<=b))
						thresh_res.append_point(p);
					i++;
				}
		*/
		while(s<END_ROI) {
			rt.current_interval(p1, p2);
			x1 = p2.x+1;
			int i = p1.z*xySize + p1.y*xdim;
			for(; p1.x<=p2.x; p1.x++) {
				if ((pltns[i]>=low) && (pltns[i]<=high)) {
					if (x1>p2.x)
						x1=p1.x;
				}
				else {
					x2 = p1.x-1;
					if (x2>=x1) thresh_res.append_interval(x1, x2, p1.y, p1.z);
					x1 = p2.x+1;
				}
				i++;
			}
			if (x1<=p2.x)
				thresh_res.append_interval(x1, p1.x-1, p1.y, p1.z);
			s = rt.next_interval();
		}
//cout << "thresh_res.num_pix() = " << thresh_res.num_pix() << endl;



		cout << "Forming candidates....." << endl;
		// Form candidates
		while (!thresh_res.empty()) {
			Point start;
			thresh_res.first_point(start);

			ROI blob;
			if (include_all_vox && segment_2d) {
				blob.copy(thresh_res, start.z);
				thresh_res.clear(start.z);
			}
			else if (include_all_vox) {
				blob.copy(thresh_res);
				thresh_res.clear();
			}
			else if (segment_2d) {
				blob.add_contig(thresh_res, start, 1);
				//thresh_res.subtract(blob);
			}
			else {
				blob.add_contig_3d(thresh_res, start, 1);
				//thresh_res.subtract(blob);
			}

			// min_num_vox is already scaled to account for subsampling
			if (!min_num_vox || blob.num_pix_grequal(min_num_vox)) {
				ImageRegion *ir = new ImageRegion (blob, bb.med_im_seq());
				se.add_candidate(ir);
				if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
			}
		}
		delete [] pltns;	
	}
	cout << "done" << endl;
}



float DistanceMapRegionGrowingS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("DistanceMapRegionGrowing"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("DistanceMapRegionGrowing")==0) {
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


void DistanceMapRegionGrowingA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		const DistanceMapRegionGrowing* const dma = (DistanceMapRegionGrowing*) se.find_attribute("DistanceMapRegionGrowing");

		// If the local maximum is below this threshold no candidate is formed (this stops candidates being formed that are only a couple of voxels thick at their widest from noise).
		register float dist_thresh = dma->dist_threshold();

		// Get percentage for thresholding
		// When performing region growing from the seed with distance
		float perc = dma->percentage();

		// Get matched region of related solution element
		SolElement& rs = bb.sol_element(dma->rel_solel_index(0));
		cout << "Segmenting using DistanceMapRegionGrowing....." << endl;
			
		int xdim = medseq.xdim();
		int ydim = medseq.ydim();
		int zdim = medseq.zdim();
		int imSize = xdim*ydim*zdim;
		
		float xsize = medseq.row_pixel_spacing(0);
		float ysize = medseq.column_pixel_spacing(0);
		float zsize = fabs(medseq.slice_location(1)-medseq.slice_location(0));
			
		// Read the distance transform map
		float* edm;
		if (bb.edm_directory().length()==0) {
			edm = readEDM(imSize, rs.name()[0], rs.name()[1], bb);
			//edm = readEDM(medseq.series_instance_uid(), imSize, rs.name()[0], rs.name()[1], bb);
		}
		else {
			edm = readEDM_DMWS(imSize, bb);
			//edm = readEDM_DMWS(medseq.series_instance_uid(), imSize, bb);
		}

		int nmc;
		rs.num_matched_cands(nmc);
		for(int i=0; i<nmc; i++) {
			ROItraverser rt(((ImageRegion*)rs.candidate(rs.matched_cand_index(i))->primitive())->roi());

			float max;
			Point max_pt;
			getMaxDist(edm, rt, max, max_pt, xdim, ydim);

			// Do the region growing from the seed point
			if ((max>dist_thresh) && (max>(0.5*medseq.slice_thickness(max_pt.z)))) {
				ROI lresult;
				dmRegGrow(edm, max_pt, max, perc, xdim, ydim, zdim, xsize, ysize, zsize, search_area, lresult);

				//cout << "YES " << dist_thresh << "   " << max <<  "   " << !lresult.empty() << endl;

				if (!lresult.empty() && (!min_num_vox || lresult.num_pix_grequal(min_num_vox))) {
					ImageRegion *ir = new ImageRegion (lresult, bb.med_im_seq());
					se.add_candidate(ir);
					if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
				}
			}
			//else {
			//	cout << "NO " << dist_thresh << "   " << max << endl;
			//}
		}
		delete [] edm;	
	}
	cout << "done" << endl;
}



float DistanceMapWatershedS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("DistanceMapWatershed"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("DistanceMapWatershed")==0) {
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


void DistanceMapWatershedA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		const DistanceMapWatershed* const dma = (DistanceMapWatershed*) se.find_attribute("DistanceMapWatershed");

		// If the local maximum is below this threshold no candidate is formed (this stops candidates being formed that are only a couple of voxels thick at their widest from noise).

		// Get matched region of related solution element
		SolElement& rs = bb.sol_element(dma->rel_solel_index(0));
		ImagePrimitive* rel_prim = rs.matched_prim();
		if (rel_prim && !strcmp(rel_prim->type(), "ImageRegion")) {
			cout << "Segmenting using DistanceMapWatershed....." << endl;
			ROI rel_region(((ImageRegion*)rel_prim)->roi());
			
			int xdim = medseq.xdim();
			int ydim = medseq.ydim();
			int zdim = medseq.zdim();

			// Write the search area ROI to a file of type char for input to the distance transform
			int imSize = xdim*ydim*zdim;
			float recon_interval = fabs(medseq.slice_location(1)-medseq.slice_location(0));
			ROItraverser sat(rel_region);
			char* binaryMaskSA = createBinaryMask(sat, imSize, xdim, ydim);

			char* edmInputMaskFileName = new char [500];
			char* edmOutputFileName = new char [500];
			char* edmSmoothedOutputFileName = new char [500];
			char* watershedOutputFileName = new char [500];

			bool exists = false;
			if (bb.edm_directory().length()==0) {
				//char* tempFilePath = new char [500];
				//sprintf(tempFilePath, "%s/%s", bb.temp_file_path(), medseq.series_instance_uid());
				boost::filesystem::create_directories(bb.temp_file_path());
				//boost::filesystem::create_directories(tempFilePath);
				sprintf(edmInputMaskFileName, "%s/%c%c_searchAreaEDM.raw", bb.temp_file_path(), se.name()[0], se.name()[1]);
				//sprintf(edmInputMaskFileName, "%s/%c%c_searchAreaEDM.raw", tempFilePath, se.name()[0], se.name()[1]);
				sprintf(edmOutputFileName, "%s/%c%c_distanceMapEDM.raw", bb.temp_file_path(), se.name()[0], se.name()[1]);
				//sprintf(edmOutputFileName, "%s/%c%c_distanceMapEDM.raw", tempFilePath, se.name()[0], se.name()[1]);
				sprintf(edmSmoothedOutputFileName, "%s/%c%c_distanceMapSmoothedEDM.raw", bb.temp_file_path(), se.name()[0], se.name()[1]);
				//sprintf(edmSmoothedOutputFileName, "%s/%c%c_distanceMapSmoothedEDM.raw", tempFilePath, se.name()[0], se.name()[1]);	
				sprintf(watershedOutputFileName, "%s/%c%c_watershedEDM.raw", bb.temp_file_path(), se.name()[0], se.name()[1]);	
				//sprintf(watershedOutputFileName, "%s/%c%c_watershedEDM.raw", tempFilePath, se.name()[0], se.name()[1]);
				//delete [] tempFilePath;
			
				// Attempt to read an existing input mask file
				ifstream existingMaskFile (edmInputMaskFileName,ifstream::binary);
				exists = !existingMaskFile.fail();
				if (exists) {
					//Read and compare the existing binary mask file with the current binary mask
					char* existingBinaryMask = new char [imSize];
					existingMaskFile.read ((char*)existingBinaryMask,imSize);
					existingMaskFile.close();
					exists = (memcmp (binaryMaskSA, existingBinaryMask, imSize)==0);
					delete [] existingBinaryMask;
				}				
			}
			else {
				//char* tempFilePath = new char [500];
				//sprintf(tempFilePath, "%s/%s", bb.edm_directory().c_str(), medseq.series_instance_uid());
				boost::filesystem::create_directories(bb.edm_directory().c_str());
				//boost::filesystem::create_directories(tempFilePath);
				//delete [] tempFilePath;

				int num_pix = rel_region.num_pix();
				char* edmInputMaskFileNamePrefix = new char [500];
				sprintf(edmInputMaskFileNamePrefix, "%s/searchAreaEDM_%d_", bb.edm_directory().c_str(), num_pix);
				//sprintf(edmInputMaskFileNamePrefix, "%s/%s/searchAreaEDM_%d_", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix);
				cout << "edmInputMaskFileNamePrefix = " << edmInputMaskFileNamePrefix << endl;

				int edm_index=-1;
				bool fileToCheck = false;
				do {
					edm_index++;
					sprintf(edmInputMaskFileName, "%s%d.raw", edmInputMaskFileNamePrefix, edm_index);
					ifstream existingMaskFile (edmInputMaskFileName,ifstream::binary);
					fileToCheck = !existingMaskFile.fail();
					if (fileToCheck) {
						//Read and compare the existing binary mask file with the current binary mask
						char* existingBinaryMask = new char [imSize];
						existingMaskFile.read ((char*)existingBinaryMask,imSize);
						existingMaskFile.close();
						exists = (memcmp (binaryMaskSA, existingBinaryMask, imSize)==0);
						delete [] existingBinaryMask;
					}
				} while (fileToCheck && !exists);

				sprintf(edmOutputFileName, "%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), num_pix, edm_index);
				//sprintf(edmOutputFileName, "%s/%s/distanceMapEDM_%d_%d.raw", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix, edm_index);
				sprintf(edmSmoothedOutputFileName, "%s/distanceMapSmoothedEDM_%d_%d.raw", bb.edm_directory().c_str(), num_pix, edm_index);
				//sprintf(edmSmoothedOutputFileName, "%s/%s/distanceMapSmoothedEDM_%d_%d.raw", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix, edm_index);
				sprintf(watershedOutputFileName, "%s/watershedEDM_%d_%d.raw", bb.edm_directory().c_str(), num_pix, edm_index);
				//sprintf(watershedOutputFileName, "%s/%s/watershedEDM_%d_%d.raw", bb.edm_directory().c_str(), medseq.series_instance_uid(), num_pix, edm_index);

				cout << "edmInputMaskFileName = " << edmInputMaskFileName << endl;
				
				bb.append_act_rec("DistanceMapWatershed", "SegmentationKS");
				bb.add_message_to_last_act_rec("Writing EDM files");

				std::string s1("EDM Num Pix: ");
				char s1buff[50];
				sprintf(s1buff,"%d",num_pix);
				s1.append(s1buff);
				bb.add_message_to_last_act_rec(s1);

				std::string s2("EDM Index: ");
				char s2buff[50];
				sprintf(s2buff,"%d",edm_index);
				s2.append(s2buff);								
				bb.add_message_to_last_act_rec(s2);
			}

			if (!exists) { // Need to recompute EDM files		
				ofstream outfileBinaryMaskSA (edmInputMaskFileName,ofstream::binary);
				outfileBinaryMaskSA.write(binaryMaskSA,imSize);
				outfileBinaryMaskSA.close();

				// Compute Euclidean Distance Map
				char command[5000];  
				// Cannot apply too much smoothing otherwise lesion on slice #56 of LIDC 0002 is merged in with the mediastinum
				float smoothing_parameter = recon_interval;

				// for(auto & p : boost::filesystem::directory_iterator( "./" )){ std::cout << p << std::endl; } //MWW debug
				boost::filesystem::path full_path(boost::filesystem::current_path());
				// std::cout << "Current path is : " << full_path << std::endl;
				// for(auto & p : boost::filesystem::directory_iterator( full_path )){ std::cout << p << std::endl; } //MWW debug
				// ifstream distanceTransformExecutable ("./MyDistanceTransform.exe",ifstream::binary);
				ifstream distanceTransformExecutable (full_path.string()+"/MyDistanceTransform.exe",ifstream::binary);

				#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
				char wine[]= "";
				#else
				char wine[] ="wine";
				#endif

				bool dtExists = !distanceTransformExecutable.fail();
				if (dtExists) {
					sprintf (command, "%s %s/MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s;", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
					// sprintf (command, "./MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s", edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
					// sprintf (command, "#!/bin/bash; echo 'hello'; %s %s/MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s;", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
					// retVal = system(command);
					// cout << command << endl << "MyDistanceTransform completed with return value = " << retVal << endl;
					// retVal = system(command);
					// cout << command << endl << "MyDistanceTransform completed with return value = " << retVal << endl;
					// sprintf (command, "/usr/bin/%s %s/MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s;", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
					// retVal = system(command);
					// cout << command << endl << "MyDistanceTransform completed with return value = " << retVal << endl;
				}
				else {
					sprintf (command, "Q:\\nodule_detection\\Executables\\MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s", edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);   
				}

				// sprintf (command, "/bin/%s %s/MyDistanceTransform.exe %s %s -w %s -s %d %d %d -v %f %f %f -smooth %f -save_smooth %s;", wine, full_path.c_str(), edmInputMaskFileName, edmOutputFileName, watershedOutputFileName, xdim, ydim, zdim, medseq.row_pixel_spacing(0), medseq.column_pixel_spacing(0), recon_interval, smoothing_parameter, edmSmoothedOutputFileName);
				int retVal = system(command);
				cout << command << endl << "MyDistanceTransform completed with return value = " << retVal << endl;
				// retVal = system(NULL);
				// cout << command << endl << "NULL completed with return value = " << retVal << endl;
			}
			
			// Read the watershed segmentation result
			int* wsSeg = new int [imSize];
			ifstream wsResultFile (watershedOutputFileName,ifstream::binary);
			wsResultFile.read ((char*)wsSeg,imSize*sizeof(int));
			wsResultFile.close();			

			// Within the search area form candidates from each of the watershed ROIs
			Point fp, lp, lpl;
			ROI saCopy(rel_region);
			saCopy.AND(search_area);
			ROItraverser sact(saCopy);
			TravStatus sacs = sact.valid();
			int wsMaskVal = 1;

			while (wsMaskVal) {
				wsMaskVal = 0;
				Point nzmp1, nzmp2;
				while((sacs<END_ROI) && !wsMaskVal) {
					sact.current_interval(nzmp1, nzmp2);
					int j = nzmp1.z*xdim*ydim + nzmp1.y*ydim + nzmp1.x;
					int inReg = 0;
					int x1=0, x2=0;
					for(; (nzmp1.x<=nzmp2.x) && !wsMaskVal; nzmp1.x++) {
						wsMaskVal = wsSeg[j];
						j++;
					}
					if (!wsMaskVal) sacs = sact.next_interval();
				}

				ROI lr; // watershed region

				Point sacp1, sacp2;
				int lastZ = nzmp1.z;
				while(sacs<END_ROI) {
					sact.current_interval(sacp1, sacp2);
					int j = sacp1.z*xdim*ydim + sacp1.y*ydim + sacp1.x;
					int inReg = 0;
					int x1=0, x2=0;
					for(; sacp1.x<=sacp2.x; sacp1.x++) {
						if (wsSeg[j]==wsMaskVal) {
							lastZ = sacp1.z;
							wsSeg[j] = 0;
							if (!inReg) {
								x1 = sacp1.x;
								inReg = 1;
							}
							x2 = sacp1.x;
						}
						else if (inReg) {
							lr.append_interval(x1, x2, sacp1.y, sacp1.z);
							inReg = 0;
						}
						j++;
					}
					if (inReg) {
						lr.append_interval(x1, x2, sacp1.y, sacp1.z);
						inReg = 0;
					}
					if (sacp1.z>(lastZ+1)) sacs=END_ROI; 
					else sacs = sact.next_interval();
				}		
				
				int lrnp = lr.num_pix();

				int max_num_vox = 30000/(medseq.row_pixel_spacing(0)*medseq.column_pixel_spacing(0)*recon_interval);				
				if ((!min_num_vox || (lrnp>=min_num_vox)) && (lrnp<=max_num_vox)) {				
					ImageRegion *ir = new ImageRegion (lr, bb.med_im_seq());
					se.add_candidate(ir);
					if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
				}

				sacs = sact.set_plane(nzmp1.z);
			}
			delete [] binaryMaskSA;
			delete [] wsSeg;
			delete [] edmInputMaskFileName;
			delete [] edmOutputFileName;
			delete [] edmSmoothedOutputFileName;
			delete [] watershedOutputFileName;
		}
		cout << "done" << endl;
	}
}


float DistanceMap2DPercLocal3DMaxS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("DistanceMap25DPercMax"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("DistanceMap2DPercLocal3DMax")==0) {
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

void DistanceMap2DPercLocal3DMaxA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << endl;

	//int i, j;
	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		const DistanceMap25DPercMax* const dma = (DistanceMap25DPercMax*) se.find_attribute("DistanceMap25DPercMax");

		// If the local maximum is below this threshold no candidate is formed (this stops candidates being formed that are only a couple of voxels thick at their widest from noise).
		register float dist_thresh = dma->dist_threshold();

		// Get percentage for thresholding
		register float perc = dma->percentage();

		/// Get allowable threshold on absolute difference in HU between current voxel and HU-value at local (distance) maximum
		register int hu_diff_threshold = dma->hu_diff_threshold();

		// Need to limit the maximum z-distance for growing so that we don't have to compute the distance map for all slices all the time, which would be memory intensive
		float max_z_dist = dma->max_z_distance();

		// Get matched region of related solution element
		SolElement& rs = bb.sol_element(dma->rel_solel_index(0));
		ImagePrimitive* rel_prim = rs.matched_prim();
		if (rel_prim && !strcmp(rel_prim->type(), "ImageRegion")) {
			cout << "Segmenting using distance map....." << endl;

			float** edms=0;
			Point fp, lp, lpl;
			int i;
			int prevfpz=0;
			if (search_area.last_point(lp)) {
				edms = new float* [lp.z+1];
				for(i=0; i<=lp.z; i++) {
					edms[i]=0;
				}
			}
			int xdim = medseq.xdim();
			int ydim = medseq.ydim();
			int j;
			float max=0, thresh;
			float min_slice_loc_grow;
			float max_slice_loc_grow;

			int xdyd = xdim*ydim;
//cout << "MB: image dims: " << xdim << ", " << ydim << ", " <<medseq.zdim() << endl;

			ROI rel_region(((ImageRegion*)rel_prim)->roi());
			while (rel_region.first_point(fp)) {
				//cout << "first point: " << fp << endl;
				ROI lr; // local region - connected component of the related image primitive
				lr.add_contig_3d(rel_region, fp, 1);
//Point p(116,66,3);
//int printDebug = lr.in_roi(p);
//if (printDebug) cout << "MB: " << fp << endl;


				if (fp.z>=0) {
					// Change fp.z to be the minimum possible z-coordinate of the growing
					// IN THIS VERSION THE MINIMUM LOCATION SEEMS TO BE THE Z-COORD OF THE FIRST POINT IN THE CONNECTED COMPONENT OF THE RELATED PRIMITIVE MINUS THE MAXIMUM-Z-DISTANCE
					min_slice_loc_grow = slice_loc(fp.z, medseq) - max_z_dist;
					while ((slice_loc(fp.z, medseq)>min_slice_loc_grow) && (fp.z>0)) fp.z--;

					// Since fp.z is the minimum possible coordinate for growing within the distance map then can delete distance map slices with lower coord value
					for (i=prevfpz; i<fp.z; i++) {
						if (edms[i]!=0) {
							delete [] edms[i];
							edms[i]=0;
						}
					}
					prevfpz=fp.z;

					lr.last_point(lpl);
					// Change lpl.z to be the maximum possible z-coordinate of the growing
					// IN THIS VERSION THE MAXIMUM LOCATION SEEMS TO BE THE Z-COORD OF THE LAST POINT IN THE CONNECTED COMPONENT OF THE RELATED PRIMITIVE PLUS THE MAXIMUM-Z-DISTANCE
					max_slice_loc_grow = slice_loc(lpl.z, medseq) + max_z_dist;
					while ((slice_loc(lpl.z, medseq)<max_slice_loc_grow) && (lpl.z<lp.z)) lpl.z++;

					for(i=fp.z; i<=lpl.z; i++) {
						if (!edms[i]) {
							//cout << "edm for z=" << i << endl;
							edms[i] = distance_map_2d(search_area, i, medseq);
						}
					}

					// Find maximum distance map value within the local matched region of the related solution element
					// Also compute starting point for growing, max_pt, and threshold
					ROItraverser rt(lr);
					TravStatus s = rt.valid();
					Point p1, p2, max_pt;
					max = 0;
					int gl_at_max = 0;
					while(s<END_ROI) {
						rt.current_interval(p1, p2);
						j=p1.y*xdim+p1.x;
						for(; p1.x<=p2.x; ++p1.x) {
							if (edms[p1.z][j]>=max) {
								max=edms[p1.z][j];
								max_pt = p1;
								GL_to_HU(medseq.fast_pix_val(max_pt.x, max_pt.y, max_pt.z), gl_at_max, medseq);
							}
							++j;
						}
						s = rt.next_interval();

					}
//if (printDebug) cout << "MB: max_pt = " << max_pt << endl;
//if (printDebug) cout << "MB: max = " << max << endl;

					// Compute distance threshold
					thresh = perc*max/100;
//if (printDebug) cout << "MB: thresh = " << thresh << endl;
//if (printDebug) cout << "MB: gl_at_max = " << gl_at_max << endl;

					// Do the region growing from the seed point
					if ((max>dist_thresh) && (thresh>0)) {
//if (printDebug) cout << "MB: here1" << endl;

						ROI lresult;

						// from start voxel get start interval
						Interval start_ivl;
						Point p;
						p = max_pt;
						p.x--;
						while (search_area.in_roi(p)) p.x--;
						int x1 = p.x+1;
						p = max_pt;
						p.x++;
						while (search_area.in_roi(p)) p.x++;
						int x2 = p.x-1;

						ROI to_check;
						to_check.add_interval(x1, x2, max_pt.y, max_pt.z);

						float** edms_copy=new float* [lpl.z+1];
						for(i=0; i<=lpl.z; i++) {
							edms_copy[i]=0;
						}
						for (i=fp.z; i<=lpl.z; i++) {
							edms_copy[i] = new float [xdyd];
							for(int k=0; k<xdyd; k++)
								edms_copy[i][k] = edms[i][k];
						}
//if (printDebug) cout << "MB: zbounds: " << fp.z << ", " << lpl.z<< endl;


						// while to_check is not empty, pop the first interval
						int xi1, xi2, yi, zi, xi12, xa1, xa2;
						int too_big=0;
						while(to_check.pop_first_interval(xi1, xi2, yi, zi) && !too_big) {
							j=yi*xdim+xi1;
							const short *pix_data = medseq.pixel_data(zi);

							while (xi1<=xi2) {
								while (((edms_copy[zi][j]<thresh)) && (xi1<=xi2)) {
									xi1++;
									j++;
								}
								if (xi1<=xi2) {
									edms_copy[zi][j]=0;

									xi12 = xi1+1;
									j++;

									int hu_value;
									GL_to_HU(pix_data[j], hu_value, medseq);
									while ((edms_copy[zi][j]>=thresh) && (abs(hu_value-gl_at_max)<=hu_diff_threshold) && (xi12<=xi2)) {
										edms_copy[zi][j]=0;

										xi12++;
										j++;
										GL_to_HU(pix_data[j], hu_value, medseq);
									}

									lresult.add_interval(xi1, xi12-1, yi, zi);
									too_big = ((zi==fp.z)||(zi==lpl.z));

									// add adjacent intervals to to_check
									if (xi1>0)
										xa1 = xi1-1;
									else
										xa1 = 0;
									if (xi12<xdim)
										xa2 = xi12;
									else
										xa2 = xdim-1;

									if (yi>0)
										to_check.add_interval(xa1, xa2, yi-1, zi);
									if (yi<(ydim-1)) {
										to_check.add_interval(xa1, xa2, yi+1, zi);
									}

									if (zi>fp.z) {
										//if (yi>0)
										//	to_check.add_interval(xa1, xa2, yi-1, zi-1);
										to_check.add_interval(xa1, xa2, yi, zi-1);
										//if (yi<(ydim-1))
										//	to_check.add_interval(xa1, xa2, yi+1, zi-1);
									}

									if (zi<lpl.z) {
										if (yi>0)
											to_check.add_interval(xa1, xa2, yi-1, zi+1);
										to_check.add_interval(xa1, xa2, yi, zi+1);
										if (yi<(ydim-1))
											to_check.add_interval(xa1, xa2, yi+1, zi+1);
									}
									xi1 = xi12+1;
									j++;
								}
							}
						}
						//cout << "done growing" << endl;

						if (!too_big) {
							// Remove first and last slice of result if necessary
							Point lrf, lrl;
							lresult.first_point(lrf);
							lresult.last_point(lrl);
							if ((lrf.z<lrl.z)&&(abs((int)(slice_loc(lrf.z, medseq)-slice_loc(lrf.z-1, medseq)))/3<thresh) && (lrf.z!=max_pt.z)) {
								lresult.clear(lrf.z);
							}
							lresult.first_point(lrf);
							lresult.last_point(lrl);
							if ((lrf.z<lrl.z)&&(abs((int)(slice_loc(lrl.z+1, medseq)-slice_loc(lrl.z, medseq)))/3<thresh) && (lrl.z!=max_pt.z)) {
								lresult.clear(lrl.z);
							}

							if (!min_num_vox || lresult.num_pix_grequal(min_num_vox)) {
								ImageRegion *ir = new ImageRegion (lresult, bb.med_im_seq());
								se.add_candidate(ir);
								if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
							}
						}

						for (i=fp.z; i<=lpl.z; i++) {
							delete [] edms_copy[i];
						}
						delete [] edms_copy;
					}
				}
			}

			if (edms) {
				for (i=0; i<=lp.z; i++) {
					if (edms[i]!=0) {
						delete [] edms[i];
					}
				}
				delete [] edms;
			}

			cout << "done" << endl;
		}
	}
}



float FormCandsFromSearchAreaS(Blackboard& bb)
{
	float score=0.0;

	if (bb.next_solel()>-1)
		score = 0.6;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_type().compare("SegmentationKS")==0) {
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


void FormCandsFromSearchAreaA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with FormCandsFromSearchArea" << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
  //std::cout << std::endl << std::endl << "SEARCH AREA" << std::endl;
  //search_area.print_all_points();

		// Form candidates
		while (!search_area.empty()) {
			Point start;
			search_area.first_point(start);

			ROI blob;
			if (include_all_vox && segment_2d) {
				blob.copy(search_area, start.z);
				search_area.clear(start.z);
			}
			else if (include_all_vox) {
				blob.copy(search_area);
				search_area.clear();
			}
			else if (segment_2d) {
				blob.add_contig(search_area, start, 1);
			}
			else {
				blob.add_contig_3d(search_area, start, 1);
//std::cout << std::endl << std::endl << "BLOB" << std::endl;
//blob.print_all_points();
//std::cout << std::endl << std::endl << "SEARCH AREA" << std::endl;
//search_area.print_all_points();
			}

			// min_num_vox is already scaled to account for subsampling
			if (!min_num_vox || blob.num_pix_grequal(min_num_vox)) {
				ImageRegion *ir;
				if (use_subsampled) {
					ROI expanded_blob;
					expand_roi(blob, expanded_blob, ss_factor.x, ss_factor.y, ss_factor.z, bb.overall_search_area());
		 			ir = new ImageRegion (expanded_blob, bb.med_im_seq());
				}
				else {
		 			ir = new ImageRegion (blob, medseq);
		 		}
				se.add_candidate(ir);

				if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
			}
		}
	}
}



float LineToDotsS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("LineToDots"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("LineToDots")==0) {
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


void LineToDotsA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with LineToDots" << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		const LineToDots* const trgl = (LineToDots*) se.find_attribute("LineToDots");
		float spacing = (float)trgl->spacing(); //  spacing between dots in mm
		float check_distance = (float)trgl->check_distance(); // maximum distance checked in case of curve discontinuities in mm

		ROI blob;
		register Point p1, p2;
		register int x, y;
		Point tl, br;
		int z;
		if (search_area.bounding_cube(tl, br)) // Search area provides the input curve
		  for (z=tl.z; z<=br.z; z++) {
			int x_inc = 0;
			int y_inc = 0;
			if (search_area.bounding_box(tl, br, z)) {
				// "x" or "y" is the initial direction of travel (this will also determine whether topmost or leftmost point is used as the start)
				if (!trgl->direction().compare("x")) { 
					x_inc = spacing/medseq.column_pixel_spacing(tl.z);
					p1 = tl;
					while(!search_area.in_roi(p1)) p1.y++;
					p2 = p1;
					while ((p2.y<(medseq.ydim()-1)) && search_area.in_roi(p2)) p2.y++;
					p1.y = (p1.y + p2.y)/2;
				}
				else {
					y_inc = spacing/medseq.row_pixel_spacing(p1.z);
					p1 = tl;
					while(!search_area.in_roi(p1)) p1.x++;
					p2 = p1;
					while ((p2.x<(medseq.xdim()-1)) && search_area.in_roi(p2)) p2.x++;
					p1.x = (p1.x + p2.x)/2;
				}
			}
			if ((x_inc!=0) || (y_inc!=0)) {
				int x_inc_base = x_inc;
				int y_inc_base = y_inc;
				int x_inc_max = check_distance/medseq.column_pixel_spacing(p1.z);
				int y_inc_max = check_distance/medseq.row_pixel_spacing(p1.z);

				blob.add_point(p1);

				int x_sum = 0;
				int y_sum = 0;
				int cnt = 1;
				while ((abs(x_inc)<x_inc_max) && (abs(y_inc)<y_inc_max)) {
					ROI sub;
					sub.add_circle(abs(x_inc)+abs(y_inc), p1.x, p1.y, p1.z);
					x_sum = 0;
					y_sum = 0;
					cnt = 0;
					int x_sign = 1;
					if (x_inc<0) x_sign=-1;
					int y_sign = 1;
					if (y_inc<0) y_sign=-1;
					// From current point scan out in a semicircle with radius of the spacing to the next point
					// For each point along the profile perpendicular to the direction of travel, compute the other coordinate which gives the necessary distance
					// If there is more than one intersecting point take the average of the perpendicular coordinates and use the necessary coordinate in the travel direction to maintain spacing
					for(x=-abs(y_inc); x<abs(y_inc); x++) {
						y = y_sign*(int)sqrt((float)y_inc*y_inc - x*x);
						p2.x = p1.x+x;
						if ((p2.x>=0) && (p2.x<medseq.xdim())) {
							p2.y = p1.y+y;
							if ((p2.y>=0) && (p2.y<medseq.ydim()) && search_area.in_roi(p2)) {
								x_sum += p2.x;
								cnt++;
							}
						}
					}
					for(y=-abs(x_inc); y<abs(x_inc); y++) {
						x = x_sign*(int)sqrt((float)x_inc*x_inc - y*y);
						p2.y = p1.y+y;
						if ((p2.y>=0) && (p2.y<medseq.ydim())) {
							p2.x = p1.x+x;
							if ((p2.x>=0) && (p2.x<medseq.xdim()) && search_area.in_roi(p2)) {
								y_sum += p2.y;
								cnt++;
							}
						}
					}
					if (x_sum>0) {
						p2 = p1;
						int new_x = (int)x_sum/cnt;
						if (y_inc!=y_inc_base) new_x = p1.x + (new_x-p1.x)*y_inc_base/y_inc;
						p1.y += y_sign*(int)sqrt((float)y_inc_base*y_inc_base - (p1.x-new_x)*(p1.x-new_x));
						p1.x = new_x;
						if (p1.y >= medseq.ydim())
							cnt = 0;
						else {
							blob.add_point(p1);
							//cout << p1 << endl;
						}
					}
					else if (y_sum>0) {
						p2 = p1;
						int new_y = (int)y_sum/cnt;
						if (x_inc!=x_inc_base) new_y = p1.y + (new_y-p1.y)*x_inc_base/x_inc;
						p1.x += x_sign*(int)sqrt((float)x_inc_base*x_inc_base - (p1.y-new_y)*(p1.y-new_y));
						p1.y = new_y;
						if (p1.x >= medseq.xdim())
							cnt = 0;
						else {
							blob.add_point(p1);
							//cout << p1 << endl;
						}
					}
					// If no intersection found then increase circle until a point is hit or the max distance is reached and no next point found
					// Then project back to the desired radius as the next point
					if (cnt==0) {
						if (y_inc != 0) y_inc += y_sign;
						else if (x_inc != 0) x_inc += x_sign;
					}
					else {
						if (abs(p2.y-p1.y)>abs(p2.x-p1.x)) {
							x_inc = 0;
							if (p1.y>p2.y)
								y_inc = spacing/medseq.row_pixel_spacing(p1.z);
							else
								y_inc = -spacing/medseq.row_pixel_spacing(p1.z);
							y_inc_base = y_inc;
						}
						else {
							y_inc = 0;
							if (p1.x>p2.x)
								x_inc = spacing/medseq.column_pixel_spacing(p1.z);
							else 
								x_inc = -spacing/medseq.column_pixel_spacing(p1.z);
							x_inc_base = x_inc;
						}
						// Eliminate all points in the current circle to make sure the curve tracking doesn't double back on itself
						search_area.subtract(sub);
					}
				}
			}

			if (!min_num_vox || blob.num_pix_grequal(min_num_vox)) {
				ImageRegion *ir;
				ir = new ImageRegion (blob, bb.med_im_seq());
				se.add_candidate(ir);
			}
		}
		cout << "done" << endl;
	}
}




float MaxCostPathS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("MaxCostPath"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("MaxCostPath")==0) {
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
Computes minimum cost path from top to bottom (row 0->N) of image X.
xd and yd are the dimensions of X.
The contour is cleared before points are added to it.
Points in the contour are in increasing y-order.
*/
void minpath(short **X, int xd, int yd, Contour& path)
{
	int	i, j, m, n;
	int     i1, m1, strength, minj;
	Point ip;
	// Use int inc ase sum exceeds short
	int **Y;

	path.clear();

	n = xd;
	m = yd;
	Y = (int **) calloc(yd, sizeof(int *));
	for(j=0; j<yd; j++)
		Y[j] = (int *) calloc(xd, sizeof(int));

	i = 0;
	for(j=0; j<n; j++)
	    Y[i][j] = X[i][j];
	for(i=1; i<m; i++){
	    i1 = i-1;
	    Y[i][0] = X[i][0] + MIN3(Y[i1][0], Y[i1][1], Y[i1][2]);
	    Y[i][1] = X[i][1] + MIN4(Y[i1][0], Y[i1][1], Y[i1][2],
					 Y[i1][3]);
	    for(j=2; j<n-2; j++)
		Y[i][j] = X[i][j] + MIN5(Y[i1][j-2], Y[i1][j-1], Y[i1][j],
					     Y[i1][j+1], Y[i1][j+2]);
	    Y[i][n-2] = X[i][n-2] + MIN4(Y[i1][n-4], Y[i1][n-3],
					     Y[i1][n-2], Y[i1][n-1]);
	    Y[i][n-1] = X[i][n-1] + MIN3(Y[i1][n-3], Y[i1][n-2],
					     Y[i1][n-1]);

	}
	m1 = m - 1;
	minj = 0;
	strength = Y[m1][0];
	for(j=1; j<n; j++){
	    if(Y[m1][j] < strength){
		minj = j;
		strength = Y[m1][j];
	    }
	}

	for(i=m-2; i>=0; i--){
	    int jj,j1,j2,k1;

	    k1 = 2;
	    j1 = MAX2((minj - k1),0);
	    j2 = MIN2((minj + k1), (n-1));
	    for (jj = j1; jj <= j2; jj++)
		if (Y[i][jj] < Y[i][minj])
		    minj = jj;
		ip.x = minj;
		ip.y = i;
		path.prepend(ip);
	}

	for(j=0; j<yd; j++)
		free(Y[j]);
	free(Y);
}

void MaxCostPathA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with MaxCostPath" << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (use_subsampled) {
		cout << "MaxCostPathA: Need to implement for subsampled" << endl;
		exit(1);
	}

	if (!stop) {
		MedicalImageSequence& medseq = bb.med_im_seq();
		// If segmentation is to be performed on subsampled data then computed search area will already be subsampled
		ROItraverser rt(search_area);

		register Point p;
		register TravStatus s = rt.valid();

		cout << "MaxCostPathA....." << endl;
		register Point p1, p2;
		Point pmin, pmax;
		Point cp;
		int xd, yd, x, y, xc, yc, i;
		short **sim;
		short imin, imax, ival;
		Contour mpath;
		int gl_cutoff;

		//int ct = !strcmp(medseq.modality(), "CT");
		int ct = 0;
		HU_to_GL(-100, gl_cutoff, medseq);

		// Increment by interval rather than point because it is faster
		while(s<END_ROI) {
		    rt.current_point(cp);
		    rt.bounding_box(pmin, pmax, cp.z);

			xd = pmax.x-pmin.x+1;
			yd = pmax.y-pmin.y+1;
			sim = new short* [yd];
			for(y=0; y<yd; y++) {
				sim[y] = new short [xd];
				for(x=0; x<xd; x++) {
					sim[y][x] = -1;
				}
			}

			if (ct) {
				imin = 4095;
				s = ROI_STAT_OK;
		    	while (s<NEW_PLANE) {
					rt.current_interval(p1, p2);
					yc = p1.y-pmin.y;

					for(; p1.x<=p2.x; p1.x++) {
						xc = p1.x-pmin.x;
						// Voxels inside the search area retain their but are capped at -100HU (gl_cutoff) to avoid too much attraction to sternum.
						sim[yc][xc] = medseq.fast_pix_val(p1.x, p1.y, p1.z);
						if (sim[yc][xc]>gl_cutoff) sim[yc][xc]=gl_cutoff;
						if (sim[yc][xc]<imin) imin = sim[yc][xc];
					}
					s = rt.next_interval();
				}

				// Voxels outside take the minimum value of any voxels inside.
				// All voxels are inverted by subtracting their value from 4095.
				ival = 4095 - imin;
				for(y=0; y<yd; y++) {
		 			for(x=0; x<xd; x++) {
						if (sim[y][x]==-1)
							sim[y][x] = ival;
						else
							sim[y][x] = 4095 - sim[y][x];
		  			}
				}
			}
			else {
				imin = 30000;
				imax = -30000;
				s = ROI_STAT_OK;
		    	while (s<NEW_PLANE) {
					rt.current_interval(p1, p2);
					yc = p1.y-pmin.y;

					for(; p1.x<=p2.x; p1.x++) {
						xc = p1.x-pmin.x;
						// Voxels inside the search area retain their intensity.
						sim[yc][xc] = medseq.fast_pix_val(p1.x, p1.y, p1.z);
						if (sim[yc][xc]<imin) imin = sim[yc][xc];
						if (sim[yc][xc]>imax) imax = sim[yc][xc];
					}
					s = rt.next_interval();
				}

				// Voxels outside take the minimum value of any voxels inside.
				// All voxels are inverted by subtracting their value from imax.
				ival = imax - imin;
				for(y=0; y<yd; y++) {
		 			for(x=0; x<xd; x++) {
						if (sim[y][x]==-1)
							sim[y][x] = ival;
						else
							sim[y][x] = imax - sim[y][x];
		  			}
				}
			}

			minpath(sim, xd, yd, mpath);

			//cout << "Forming candidates....." << endl;
			if ((mpath.n()>0) && (mpath.n()>=min_num_vox)) {
				ROI blob;
				for(i=0; i<mpath.n(); i++) {
					Point p(mpath[i]);
					p.x += pmin.x;
					p.y += pmin.y;
					p.z = cp.z;
					blob.append_point(p);
				}

				ImageRegion *ir;
				ir = new ImageRegion (blob, bb.med_im_seq());
				se.add_candidate(ir);
			}

			for(y=0; y<yd; y++) {
				delete [] sim[y];
			}
			delete [] sim;
		}
		cout << "done" << endl;
	}
}



float NeuralNetKerasS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("NeuralNetKeras"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("NeuralNetKeras")==0) {
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


std::string bit_tag(std::string chromosome, std::vector<bool> bits_used) {
	std::string bit_tag_bin;
	std::string bit_tag;
	int val = 0;
	int hex_cnt=0;
	int index;
	int bit_cnt=0;
	
	for (index=0; index<bits_used.size(); index++) {
		if (bits_used[index]) {
			bit_tag_bin.append(chromosome.substr(index,1));
			//cout << bit_tag_bin[bit_tag_bin.size()-1];
			if (bit_tag_bin[bit_tag_bin.size()-1]=='1') {
				val += (int)pow(2.0, hex_cnt);
			}
			hex_cnt++;
			bit_cnt++;
		}
		if ((hex_cnt==4) || ((index == (bits_used.size()-1)) && (hex_cnt>0))) {
			char ts[10];
			sprintf(ts, "%X", val);
			//cout << "  " << ts << endl;
			bit_tag.append(ts);
			val = 0;
			hex_cnt = 0;
		}
	}
	if (bit_cnt>0) {
		char ts[100];
		sprintf(ts, "%i_", bit_cnt);
		bit_tag.insert(0, ts);
	}
//cout << "bit_tag_bin: " << bit_tag_bin << endl;
//cout << "bit_tag: " << bit_tag << endl;

	return bit_tag;
}

// youngwon edited
void write_ini(std::string refconfig_path, std::string config_path, std::string se_name,
               std::string image_path, std::string model_path, std::string output_path, std::string working_path, std::string weights_path,
			   std::string bounding_box, std::string chromosome, std::string hd5_bit_tag, std::string hdf5_bit_tag,
               const NeuralNetKeras* trgl, std::string user_resource_directory, std::string condor_job_directory,
			   const bool skip_normalized_image_png, const bool skip_normalized_image_png_training, const bool skip_tensorboard_logging,
			   const bool predict_cpu_only, const bool ph_area_exists) {
	if (predict_cpu_only){
		cout << "Prediction using only the default resource (a single-core CPU)" << endl;
	}
	if (skip_normalized_image_png) {
		cout << "Skip saving png file for normalized image input" << endl;
	}
	if (skip_normalized_image_png_training) {
		cout << "Skip saving png file for normalized image input for training phase" << endl;
	}
	if (skip_tensorboard_logging) {
		cout << "Skip saving tensorboard log file" << endl;
	}
	cout << "Writing model configuration ini file to " << config_path << " ..." << endl;

	ofstream configFileWriter(config_path.c_str(),ofstream::binary);
	configFileWriter.close();

	mINI::INIFile def_file(refconfig_path);
	mINI::INIStructure ini;
	def_file.read(ini);
	
	ini["path_info"]["image_file"] = image_path;
	ini["path_info"]["model_file"] = model_path;
	ini["path_info"]["working_directory"] = working_path;
	ini["path_info"]["output_directory"] = output_path;
	ini["path_info"]["weights_file"] = weights_path;
	ini["path_info"]["user_resource_directory"] = user_resource_directory;
	ini["path_info"]["condor_job_directory"] = condor_job_directory;
	ini["model_info"]["node_name"] = se_name;
	ini["model_info"]["bounding_box"] = bounding_box;
	
	if (predict_cpu_only){
		ini["switch_from_miu_info"]["predict_cpu_only"] = "true";
	}
	if (skip_normalized_image_png){
		ini["switch_from_miu_info"]["skip_png"] = "true";
	}
	if (skip_normalized_image_png_training){
		ini["switch_from_miu_info"]["skip_png_training"] = "true";
	}
	if (skip_tensorboard_logging){
		ini["switch_from_miu_info"]["skip_tb"] = "true";
	}
	if (ph_area_exists){
		ini["switch_from_miu_info"]["use_ph_area"] = "true";
	}

	// youngwon edited
	if (chromosome.length()>0) {
	ini["chromosome_info"]["chromosome"] = chromosome;
	}
	if (hd5_bit_tag.length()>0) {
		ini["chromosome_info"]["weights_tag"] = hd5_bit_tag;
	}if (hdf5_bit_tag.length()>0) {
		ini["chromosome_info"]["input_tag"] = hdf5_bit_tag;
	}

	trgl->set_ini(ini);

	mINI::INIFile file(config_path);
	file.generate(ini);
	cout << "Done" << endl;
}

void NeuralNetKerasA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with NeuralNetKeras" << endl;
	// cout << "[SegmentationKS.cc|2091|DEBUG] Stop at node = " << bb.stop_at_node() << ", node name = " << se.name()<<  endl;
	// cout << "[SegmentationKS.cc|2092|DEBUG] (Stop at node == node name)? " << ((bb.stop_at_node() == se.name())? "exactly same":"not exactly same") <<  endl;
	// cout << "[SegmentationKS.cc|2093|DEBUG] (Stop at node != node name) = " << (bb.stop_at_node() != se.name()) <<  endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	ROI ph_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb, ph_area);

	if (!stop) {
		char roi_path[5000];
		sprintf (roi_path, "%s%s%s%s", bb.roi_directory().c_str(), "/pred_", se.name().c_str(), ".roi");
		cout << "Attempting roi from " << roi_path << endl;
		ifstream roiFile(roi_path);
		if (!roiFile) {
			sprintf (roi_path, "%s%s%s%s", bb.roi_directory().c_str(), "\\pred_", se.name().c_str(), ".roi");
			cout << "Attempting roi from " << roi_path << endl;
			roiFile.open(roi_path);
		}
		if (roiFile) {
			cout << "Reading roi from " << roi_path << endl;
			MedicalImageSequence& medseq = bb.med_im_seq();
			ROI r;
			roiFile >> r;
			
			if (!r.empty()) {
				ImageRegion* ir = new ImageRegion (r, medseq);
				se.add_candidate(ir);
			}
			stop = 1;
			cout << "done" << endl;
		}
	}

	if (!stop) {
		const NeuralNetKeras* const trgl = (NeuralNetKeras*) se.find_attribute("NeuralNetKeras");
		int input_image_rows = (int)trgl->image_rows();
		int input_image_cols = (int)trgl->image_cols();
		// youngwon edited
		// int input_image_slices = (int)trgl->image_slices();

		std::string hd5_bit_tag;
		std::string hdf5_bit_tag;

		if (bb.model().chromosome().length()>0) {
			// The hd5 weight file chromosome bit tag will include any bits involved in the NeuralNetKeras attribute
			std::vector<bool> hd5_bits_used;
			int index;
			for (index = 0; index < bb.model().chromosome().length(); index++)
				hd5_bits_used.push_back(trgl->chromosome_bit_used(index));

			std::vector<bool> hdf5_bits_used;
			for (index=0; index<bb.model().chromosome().length(); index++)
				hdf5_bits_used.push_back(trgl->normalization_bit_used(index));

			// The hd5 weight file chromosome bit tag will include any bits from search area attributes and any bits from ancestor nodes
			std::vector<int> se_anc;
			int am, ak;
			for(am=0; am<se.num_attributes(); am++) {
				if (!strcmp(se.attribute(am)->type(), "SearchArea")) {
					// Adding bits from search area attribute
					if (!se.attribute(am)->no_chromosome_bits_used()) {
						for (index=0; index<hd5_bits_used.size(); index++)
							hd5_bits_used[index] = hd5_bits_used[index] || se.attribute(am)->chromosome_bit_used(index);
						for (index=0; index<hdf5_bits_used.size(); index++)
							hdf5_bits_used[index] = hdf5_bits_used[index] || se.attribute(am)->chromosome_bit_used(index);
					}
					// Identifying ancestor nodes
					for(ak=0; ak<se.attribute(am)->num_rel_solels(); ak++) {
						int rsi = se.attribute(am)->rel_solel_index(ak);
						if (std::find(se_anc.begin(), se_anc.end(), rsi) == se_anc.end()) {
							se_anc.push_back(rsi);
							bb.add_ancestors(bb.sol_element(rsi),se_anc);
							//cout << bb.sol_element(rsi).name() << endl;
						}
					}
				}
			}
//for (index=0; index<hd5_bits_used.size(); index++)
//	cout << hd5_bits_used[index];
//cout << endl;
			// Adding bits from ancestor nodes
			std::sort (se_anc.begin(), se_anc.end());
			int an;
			for(an=0; an<se_anc.size(); an++) {
				SolElement& ase = bb.sol_element(se_anc[an]);
//cout << se_anc[an] << "   " << ase.name() << "    ";
				for(am=0; am<ase.num_attributes(); am++) {
					if (!ase.attribute(am)->no_chromosome_bits_used()) {
//cout << ase.attribute(am)->name() << endl;
						for (index=0; index<hd5_bits_used.size(); index++)
							hd5_bits_used[index] = hd5_bits_used[index] || ase.attribute(am)->chromosome_bit_used(index);
						for (index=0; index<hdf5_bits_used.size(); index++)
							hdf5_bits_used[index] = hdf5_bits_used[index] || ase.attribute(am)->chromosome_bit_used(index);
						//for (index=0; index<hd5_bits_used.size(); index++)
//	cout << hd5_bits_used[index];
//cout << endl;
					}
				}
			}
			
			hd5_bit_tag.append(bit_tag(bb.model().chromosome(), hd5_bits_used));
			cout << "hd5 bit_tag: " << hd5_bit_tag << endl;

			hdf5_bit_tag.append(bit_tag(bb.model().chromosome(), hdf5_bits_used));
			cout << "hdf5 bit_tag: " << hdf5_bit_tag << endl;

			//std::string weights_path = trgl->weights_path().substr(0, trgl->weights_path().length()-4);
			//weights_path = weights_path + "_" + hd5_bit_tag + ".hd5";
			//cout << "Model weights_path: " << weights_path << endl;
		}

		std::string refconfig_path = bb.exec_directory() + "/../../../../../src/qia/common/miu/script/ref_config.ini";
		ifstream refconfig_file (refconfig_path.c_str(),ifstream::binary);
		if (refconfig_file.fail()) {
			// youngwon edited TODO: the place of ref_config.ini
			refconfig_path = bb.exec_directory() + "/script/ref_config.ini";
			// refconfig_path = bb.exec_directory() + "/../../../../../bin/miu/script/ref_config.ini";
		}

		std::string weights_path = trgl->weights_path().substr(0, trgl->weights_path().length()-4);
		if (hd5_bit_tag.length()>0)
			weights_path = weights_path + "_" + hd5_bit_tag + ".hd5";
		else	
			weights_path = weights_path + "_NA.hd5";
		cout << "Model weights_path: " << weights_path << endl;

		ifstream weightsFile(weights_path.c_str(),ifstream::binary);
		if (weightsFile.fail()) {
			//std::string working_weights_path(bb.edm_directory());
			weights_path = bb.edm_directory() + "/" + se.name() + "_KerasModel/weights/" + se.name() + "_weights";
			if (hd5_bit_tag.length()>0)
				weights_path = weights_path + "_" + hd5_bit_tag + ".hd5";
			else
				weights_path = weights_path + "_NA.hd5";
			cout << "Working weights_path: " << weights_path << endl;
			weightsFile.open(weights_path.c_str(),ifstream::binary);
		}

		// youngwon edited
		if (weightsFile.fail() && !bb.edm_directory().empty() && (bb.stop_at_node() != se.name())) {
		// if (weightsFile.fail() && !bb.edm_directory().empty()) {
			char pypath[5000];
			sprintf (pypath, "%s/../../../../../src/qia/common/miu/script/cnn_train_wrapper.py", bb.exec_directory().c_str());
			ifstream cnnInpImageExecutable (pypath,ifstream::binary);
			bool ciiExists = !cnnInpImageExecutable.fail();
			if (!ciiExists) {
				sprintf (pypath, "%s/script/cnn_train_wrapper.py", bb.exec_directory().c_str());
				ifstream cnnExecutable2 (pypath,ifstream::binary);
				ciiExists = !cnnExecutable2.fail();
			}
			if (ciiExists) {	
				// youngwon edited /////////////////////////////////////////
				std::string config_path(bb.edm_directory());
				config_path = config_path + "/" + se.name() + "_KerasModel/config";
				boost::filesystem::create_directories(config_path);
				config_path = config_path + "/" + se.name();
				if (hd5_bit_tag.length()>0) {
					config_path = config_path + "_" + hd5_bit_tag;
				} else {
					config_path = config_path + "_NA";
				}
				if (hdf5_bit_tag.length()>0) {
					config_path = config_path + "_" + hdf5_bit_tag;
				} else {
					config_path = config_path + "_NA";
				}
				config_path = config_path + "_config.ini";
				Point tl, br;
				std::ostringstream bbs;
				if (search_area.bounding_cube(tl, br)) {
					bbs << tl.x << ", " << tl.y << ", " << tl.z << ", " << br.x << ", " << br.y << ", " << br.z;
				} else {
					bbs << "";
				}

				bool ph_area_exists = !ph_area.empty();
				if (ph_area_exists) {
					std::string ph_area_path;
					ph_area_path = ph_area_path + bb.temp_file_path() + "/ph_area_" + se.name() + ".roi";
					cout << "Writing ph area roi to " << ph_area_path << " ..." << endl;
					ofstream pharearoiFile(ph_area_path);
					if (!pharearoiFile) pharearoiFile.open(ph_area_path);
					pharearoiFile<<ph_area;
					pharearoiFile.close();
					cout << "Done" << endl;
				}

				std::string source_config_dir;
				if (!bb.user_resource_directory().empty()) {
					source_config_dir = source_config_dir + bb.user_resource_directory();
				// } else {
				// 	source_config_dir = source_config_dir + bb.edm_directory();
				}
				// std::string source_config_path(bb.edm_directory());
				std::string source_config_path(source_config_dir);
				source_config_path = source_config_path + "/" + se.name() + "_resource.ini";
				write_ini(refconfig_path, config_path, se.name(),
				          "", bb.model().file().c_str(), bb.temp_file_path(), bb.edm_directory(), 
						  "",  bbs.str(), bb.model().chromosome(), hd5_bit_tag, hdf5_bit_tag, trgl, source_config_dir, bb.condor_job_directory(),
				          bb.skip_normalized_image_png(), bb.skip_normalized_image_png_training(), bb.skip_tensorboard_logging(), 
						  bb.predict_cpu_only(), ph_area_exists);
				////////////////////////////////////////////////////////////////////////////
				char commstr[5000];
				char lrstr[200];
				//sprintf (commstr, "python %s/../../../../../src/qia/common/miu/script/cnn_train.py %s %s %s/%s_KerasModel %s %s %d %d", bb.exec_directory().c_str(), se.name(), bb.model().file().c_str(), bb.edm_directory().c_str(), se.name(), trgl->cnn_arch().c_str(), trgl->weights_path().c_str(), input_image_rows, input_image_cols);
				//sprintf (commstr, "python %s %s %s %s/%s_KerasModel %s %s %d %d", pypath, se.name(), bb.model().file().c_str(), bb.edm_directory().c_str(), se.name(), trgl->cnn_arch().c_str(), trgl->weights_path().c_str(), input_image_rows, input_image_cols);
				
				//youngwon edited
				// python node_name model_path training_path cnn_arch rows columns
				// sprintf (commstr, "python %s %s %s %s/%s_KerasModel %s %d %d", pypath, se.name().c_str(), bb.model().file().c_str(), bb.edm_directory().c_str(), se.name().c_str(), trgl->cnn_arch().c_str(), input_image_rows, input_image_cols);
				sprintf (commstr, "python %s", pypath);
				std::string command(commstr);
				// for (int k=0; k<trgl->num_channels(); k++) {
				// 		command.append(" -i ");
				// 		command.append(trgl->intensity_normalization(k));
				// }
				// sprintf(lrstr, " --learning_rate %f", trgl->learning_rate());
				// command.append(lrstr);
				// if (hd5_bit_tag.length()>0)
				// 	command = command + " --weights_tag " + hd5_bit_tag;
				// if (hdf5_bit_tag.length()>0)
				// 	command = command + " --input_tag " + hdf5_bit_tag;
				command = command + " --model_config " + config_path;
				command = command + " --resource_config " + source_config_path;
				char logging_level[200];
				//youngwon TODO discuss: for debelopver level 2 and user level 1
				sprintf(logging_level, " --verbose %d", 2);
				command.append(logging_level);

				cout << command << endl << flush;
				
				int retVal = system(command.c_str());
				cout << command << endl << "cnn_train.py completed with return value = " << retVal << endl;				
			}
		}
				
		Point tl, br;
		if (search_area.bounding_cube(tl, br)) {
			std::ostringstream bbs;
			bbs << tl.x << ", " << tl.y << ", " << tl.z << ", " << br.x << ", " << br.y << ", " << br.z;
			std::string config_path(bb.temp_file_path());
			config_path = config_path + "/" + se.name() + "_config.ini";

			bool ph_area_exists = !ph_area.empty();
			if (ph_area_exists) {
				std::string ph_area_path;
				ph_area_path = ph_area_path + bb.temp_file_path() + "/ph_area_" + se.name() + ".roi";
				cout << "Writing ph area roi to " << ph_area_path << " ..." << endl;
				ofstream pharearoiFile(ph_area_path);
				if (!pharearoiFile) pharearoiFile.open(ph_area_path);
				pharearoiFile<<ph_area;
				pharearoiFile.close();
				cout << "Done" << endl;
			}

			char pypath[5000];
			sprintf (pypath, "%s/../../../../../src/qia/common/miu/script/cnn_predict.py", bb.exec_directory().c_str());
			ifstream cnnInpImageExecutable (pypath,ifstream::binary);
			bool ciiExists = !cnnInpImageExecutable.fail();
			if (!ciiExists) {
				sprintf (pypath, "%s/script/cnn_predict.py", bb.exec_directory().c_str());
				//cout << endl << endl << endl << pypath << endl << endl << endl;
				ifstream cnnExecutable2 (pypath,ifstream::binary);
				ciiExists = !cnnExecutable2.fail();
			}
			// youndwon edited ////////////////////////////////////////
			std::string weights_path = trgl->weights_path().substr(0, trgl->weights_path().length()-4);
			if (hd5_bit_tag.length()>0)
				weights_path = weights_path + "_" + hd5_bit_tag + ".hd5";
			else	
				weights_path = weights_path + "_NA.hd5";
			cout << "Model weights_path: " << weights_path << endl;

			ifstream weightsFile(weights_path.c_str(),ifstream::binary);
			if (weightsFile.fail()) {
				//std::string working_weights_path(bb.edm_directory());
				weights_path = bb.edm_directory() + "/" + se.name() + "_KerasModel/weights/" + se.name() + "_weights";
				if (hd5_bit_tag.length()>0)
					weights_path = weights_path + "_" + hd5_bit_tag + ".hd5";
				else	
					weights_path = weights_path + "_NA.hd5";
				cout << "Working weights_path: " << weights_path << endl;
				weightsFile.open(weights_path.c_str(),ifstream::binary);
			}
			// When a user_resource_directory is not specified,
			// miu will use the working directory instead
			std::string source_config_dir;
			if (!bb.user_resource_directory().empty()) {
				source_config_dir = source_config_dir + bb.user_resource_directory();
			// } else {
			// 	source_config_dir = source_config_dir + bb.edm_directory();
			}
			// std::string source_config_path(bb.edm_directory());
			std::string source_config_path(source_config_dir);
			source_config_path = source_config_path + "/" + se.name() + "_resource.ini";
			write_ini(refconfig_path, config_path, se.name(), 
			          bb.image_path(), bb.model().file().c_str(), bb.temp_file_path(), bb.edm_directory(), weights_path, 
			          bbs.str(), bb.model().chromosome(), hd5_bit_tag, hdf5_bit_tag, trgl, source_config_dir, bb.condor_job_directory(),
					  bb.skip_normalized_image_png(), bb.skip_normalized_image_png_training(), bb.skip_tensorboard_logging(), 
					  bb.predict_cpu_only(), ph_area_exists);
			////////////////////////////////////////

			if (ciiExists && !weightsFile.fail()) {
				char commstr[5000];
				//sprintf (commstr, "python %s \"%s\" %s %s %s %d %d %d %d %d %d %d %d", pypath, bb.image_path(), trgl->cnn_arch().c_str(), trgl->weights_path().c_str(), bb.temp_file_path(), input_image_rows, input_image_cols, tl.x, tl.y, tl.z, br.x, br.y, br.z);
				
				// sprintf (commstr, "python %s \"%s\" %s %s %s %d %d %d %d %d %d %d %d", pypath, bb.image_path(), trgl->cnn_arch().c_str(), weights_path.c_str(), bb.temp_file_path(), input_image_rows, input_image_cols, tl.x, tl.y, tl.z, br.x, br.y, br.z);
				
				// youngwon edited
				sprintf (commstr, "python %s", pypath);

				// youngwon edited
				std::string command(commstr);
				// for (int k=0; k<trgl->num_channels(); k++) {
				// 		command.append(" -i ");
				// 		command.append(trgl->intensity_normalization(k));
				// }
				//if (hd5_bit_tag.length()>0) {
				//	command = command + " --training_path " + bb.edm_directory();
				//	command = command + " --weights_tag " + hd5_bit_tag;
				//}
				// std::string model_config_path(bb.temp_file_path());
				// model_config_path = model_config_path + "/" + se.name() + "_config.ini";
				command = command + " --model_config " + config_path;
				command = command + " --resource_config " + source_config_path;
				char logging_level[200];
				//youngwon TODO discuss: for debelopver level 2 and user level 1
				sprintf(logging_level, " --verbose %d", 2);
				command.append(logging_level);

				cout << command << endl;
				
				int retVal = system(command.c_str());
				cout << command << endl << "cnn_predict.py completed with return value = " << retVal << endl;				
				
				char roi_path[5000];
				sprintf (roi_path, "%s%s", bb.temp_file_path(), "/pred.roi");
				//sprintf (roi_path, "%s%s", bb.temp_file_path(), "\\pred.roi");
				cout << "pred path: " << roi_path << endl;

				ifstream roiFile(roi_path);
				if (!roiFile) {
					sprintf (roi_path, "%s%s", bb.temp_file_path(), "\\pred.roi");
					cout << "pred path: " << roi_path << endl;
					roiFile.open(roi_path);
				}
				if ((retVal == 0) && roiFile) {
					cout << "pred found" << endl;
					MedicalImageSequence& medseq = bb.med_im_seq();
					ROI r;
					roiFile >> r;
					
					if (!r.empty()) {
						ImageRegion* ir = new ImageRegion (r, medseq);
						se.add_candidate(ir);
					}
				}
			}
			else {
				cout << "cnn_predict.py script and/or weights file not found, so adding search area as a candidate" << endl;	
				MedicalImageSequence& medseq = bb.med_im_seq();
				ImageRegion* ir = new ImageRegion (search_area, medseq);
				se.add_candidate(ir);
			}
		}
		cout << "done" << endl;
	}
}



float ReadMatchedRoiS(Blackboard& bb)
{
	float score=0.0;

	if (bb.next_solel()>-1) {
		SolElement& se = bb.sol_element(bb.next_solel());
		ifstream roiFile(bb.roi_directory()+"/"+se.name()+".roi");
		if (roiFile) score = 0.67;
		roiFile.close();
	}

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_type().compare("SegmentationKS")==0) {
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


void ReadMatchedRoiA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Reading ROI from external file for " << se.name() << " using ReadMatchedRoi" << endl;

	ifstream roiFile(bb.roi_directory()+"/"+se.name()+".roi");
	if (!roiFile) cout << "ERROR: could not open file "+bb.roi_directory()+"/"+se.name()+".roi" << endl;
	else {
		MedicalImageSequence& medseq = bb.med_im_seq();

		ROI r;
		roiFile >> r;

		Point tl, br;
		r.bounding_cube(tl, br);
		cout << "tl=" << tl << "   br=" << br << endl;

		int num_slices_z = medseq.num_images();
		int* im_inst_nums = new int [num_slices_z];
		for(int i=0; i<num_slices_z; i++) {
			im_inst_nums[i] = medseq.instance_number(i);
			cout << "instance num = " << im_inst_nums[i] << endl;
		}
		
		// Modify Rois to convert them from image instance numbers to image index
		// check if image instance numbers are continuous to decide whether ROIs can be translated using their own method or whether the translation method defined in this class is required
		int cts=1;
		for(int j=1; (j<num_slices_z) && cts; j++) {
			cts = (im_inst_nums[j]==(im_inst_nums[j-1]+1));
		}
		
		if (cts)
			r.translate(0,0,-im_inst_nums[0]);
		else {
			bool ok = r.inverse_map_z(im_inst_nums, num_slices_z);
			if (!ok) {
				cerr << "ERROR: ReadMatchedRoiA: no instance number in the image matches that from the ROI" << endl;
				exit(1);
			}
		}

		ImageRegion* ir = new ImageRegion (r, medseq);		
		ir->roi().bounding_cube(tl, br);
		cout << "tl=" << tl << "   br=" << br << endl;
		se.add_candidate(ir);
	}
	roiFile.close();
}




float ThreshRegGrowS(Blackboard& bb)
{
	float score=0.0;

	if ((bb.next_solel()>-1) && bb.sol_element(bb.next_solel()).find_attribute("ThreshRangeGL"))
		score = 0.65;

	int i;
	for(i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   if (bb.act_rec(i).ks_name().compare("ThreshRegGrow")==0) {
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


void ThreshRegGrowA(Blackboard& bb)
{
	SolElement& se = bb.sol_element(bb.next_solel());
	cout << endl << endl << "Segmenting " << se.name() << " with ThreshRegGrow" << endl;

	int use_subsampled, stop, segment_2d, include_all_vox, min_num_vox;
	ROI search_area;
	Point ss_factor;
	compute_segmentation_parameters(search_area, use_subsampled, ss_factor, stop, segment_2d, min_num_vox, include_all_vox, bb);

	if (!stop) {
		const ThreshRangeGL* const trgl = (ThreshRangeGL*) se.find_attribute("ThreshRangeGL");
		register int low_gray = (int)trgl->lowGL();
		register int high_gray = (int)trgl->highGL();

		MedicalImageSequence& medseq = bb.med_im_seq();
		// If segmentation is to be performed on subsampled data then computed search area will already be subsampled
		ROItraverser rt(search_area);

		register Point p;
		register short gl;
		register TravStatus s = rt.valid();
		ROI thresh_res;

		cout << "Thresholding....." << endl;
		register Point p1, p2;
		register int x1, x2;

		// Increment by interval rather than point because it is faster
		if (!use_subsampled) {
		    while(s<END_ROI) {
			rt.current_interval(p1, p2);
			x1 = p2.x+1;
			//x2 = p1.x;
			for(; p1.x<=p2.x; p1.x++) {
				gl = medseq.fast_pix_val(p1.x, p1.y, p1.z);
				if ((gl>=low_gray) && (gl<=high_gray)) {
					if (x1>p2.x)
						x1=p1.x;
				}
				else {
					x2 = p1.x-1;
					if (x2>=x1) thresh_res.append_interval(x1, x2, p1.y, p1.z);
					x1 = p2.x+1;
				}
			}
			if (x1<=p2.x)
				thresh_res.append_interval(x1, p1.x-1, p1.y, p1.z);
			s = rt.next_interval();
		    }
		}
		else {
while(s<END_ROI) {
			rt.current_interval(p1, p2);
			x1 = p2.x+1;
			//x2 = p1.x;
			for(; p1.x<=p2.x; p1.x++) {
				gl = medseq.fast_pix_val(p1.x*ss_factor.x, p1.y*ss_factor.y, p1.z*ss_factor.z);
				if ((gl>=low_gray) && (gl<=high_gray)) {
					if (x1>p2.x)
						x1=p1.x;
				}
				else {
					x2 = p1.x-1;
					if (x2>=x1) thresh_res.append_interval(x1, x2, p1.y, p1.z);
					x1 = p2.x+1;
				}
			}
			if (x1<=p2.x)
				thresh_res.append_interval(x1, p1.x-1, p1.y, p1.z);
			s = rt.next_interval();
		    }
		}
		cout << "done" << endl;

		cout << "Forming candidates....." << endl;
		// Form candidates
		while (!thresh_res.empty()) {
			Point start;
			thresh_res.first_point(start);

			ROI blob;
			if (include_all_vox && segment_2d) {
				blob.copy(thresh_res, start.z);
				thresh_res.clear(start.z);
			}
			else if (include_all_vox) {
				blob.copy(thresh_res);
				thresh_res.clear();
			}
			else if (segment_2d) {
				blob.add_contig(thresh_res, start, 1);
				//thresh_res.subtract(blob);
			}
			else {
				blob.add_contig_3d(thresh_res, start, 1);
				//thresh_res.subtract(blob);
			}

			// min_num_vox is already scaled to account for subsampling
			if (!min_num_vox || blob.num_pix_grequal(min_num_vox)) {
				ImageRegion *ir;
				if (use_subsampled) {

					ROI expanded_blob;
					expand_roi(blob, expanded_blob, ss_factor.x, ss_factor.y, ss_factor.z, bb.overall_search_area());
		 			ir = new ImageRegion (expanded_blob, bb.med_im_seq());
				}
				else {
		 			ir = new ImageRegion (blob, bb.med_im_seq());
				}
				se.add_candidate(ir);
				if (se.num_candidates()%100000==0) cout << "Number of candidates generated = " << se.num_candidates() << endl;
			}
		}
		cout << "done" << endl;
	}
}


float SameCandidatesAsS(Blackboard& bb)
{
    float score=0.0;
    if (bb.next_solel()>-1) {
	const Attribute* sca=bb.sol_element(bb.next_solel()).find_attribute("SameCandidatesAs");
	if (sca) {
		int sca_ind = sca->rel_solel_index(0);
		int sca_grp = bb.find_group(sca_ind);

		if (sca_grp==-1) {
			cerr << "ERROR: SegmentationKS: SameCandidatesAsS: could not find group of related solution element" << endl;
			exit(1);
		}

		if (sca_grp!=bb.next_group()) {
			if (bb.group_const(sca_grp).priority()==-1.0) {
				score = 0.65;
				for(int i=bb.num_act_recs()-1; (score>0) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   	  			if ((bb.act_rec(i).ks_type().compare("SegmentationKS")==0) && (bb.act_rec(i).ks_name().compare("SameCandidatesAs")==0)) {
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
			}
		}
		else {
			score = 0.65;
			int found=0;
			for(int i=bb.num_act_recs()-1; (score>0) && (!found) && (i>=0) && (bb.act_rec(i).ks_name().compare("NextGroup")!=0); i--)
	   		  if (bb.act_rec(i).ks_type().compare("SegmentationKS")==0) {
				int mess_ind = bb.act_rec(i).find_message_starting_with("Solution element index:");
				if (mess_ind>-1) {
					const std::string& s = bb.act_rec(i).message(mess_ind);
					int string_ind=0;
					if (advance_to(s, ':', string_ind)) {
						string_ind++;
						int se_ind;
						if (read_integer(s, se_ind, string_ind)) {
							if ((bb.act_rec(i).ks_name().compare("SameCandidatesAs")==0) && (se_ind==bb.next_solel()))
								score = 0;
							else if (se_ind==sca_ind)
								found = 1;
						}
					}
				}
			}
			if (!found)
				score = 0;
		}
	}
    }

    return score;
}

void SameCandidatesAsA(Blackboard& bb) {
	SolElement& s = bb.sol_element(bb.next_solel());
	const Attribute* sca=s.find_attribute("SameCandidatesAs");

	if (sca) {
		cout << endl << endl << "Segmenting " << s.name() << " with SameCandidatesAs ";

		for(int j=0; j<sca->num_rel_solels(); j++) {
			SolElement& rel_se = bb.sol_element(sca->rel_solel_index(j));
			cout << rel_se.name() << " ";

			for(int i=0; i<rel_se.num_candidates(); i++)
				s.add_candidate(rel_se.candidate(i)->primitive()->create_copy());
		}

		cout << endl;
	}
	else {
		cerr << "ERROR: SegmentationKS: SameCandidatesAsA: unable to find SameCandidatesAs attribute" << endl;
		exit(1);
	}
	//cout << "Number of candidates = " << s.num_candidates() << endl;
}

