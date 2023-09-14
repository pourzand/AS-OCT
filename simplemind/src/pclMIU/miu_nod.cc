#include <iostream>
#include <string>
#include <sstream>
#include <stdlib.h>

#include "DICOMsequence.h"
#include "PCLsequence.h"
#include "Model.h"
#include "Blackboard.h"
#include "KnowledgeSource.h"
#include "ModelKS.h"
#include "SegmentationKS.h"
#include "SchedulerKS.h"
#include "InferencingKS.h"
#include "MemManageKS.h"

#include "ImageRegion.h"
#include "ImageContour.h"
#include "SearchArea.h"

#include <pcl/misc/FileNameTokenizer.h>
#include <pcl/misc/FileHelper.h>

#include <pcl/misc/CommandLineParser.h>
#include <pcl/misc/FileHelper.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
//using namespace boost::filesystem;

// ***** MASK TEST ****
//#include "radlogics_segmentation.h"

//#include <boost/thread/thread.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>

int do_segmentation(const char *image_file, const char* model_file, const char *exec_directory, const char *output_directory, 
                    const char *roi_directory, const char *edm_directory, const char *chromosome, const char *stop_at_node, 
					const char *user_resource_directory, const char *condor_job_directory, 
					const bool skip_normalized_image_png, const bool skip_normalized_image_png_training, const bool skip_tensorboard_logging, 
					const bool predict_cpu_only) {
	if (roi_directory) cout << "ROI directory = " << roi_directory << endl;
	else cout << "ROI directory not specified" << endl;
	if (edm_directory) cout << "EDM directory = " << edm_directory << endl;
	else cout << "EDM directory not specified" << endl;
	if (chromosome) cout << "Chromosome = " << chromosome << endl;
	else cout << "Chromosome not specified" << endl;
	if (stop_at_node) cout << "Stop at Node = " << stop_at_node << endl;
	else cout << "Stop at Node not specified" << endl;
	if (user_resource_directory) cout << "User Resource Directory  = " << user_resource_directory << endl;
	else cout << "User Resource Directory not specified" << endl;
	if (condor_job_directory) cout << "Condor Job Directory  = " << condor_job_directory << endl;
	else cout << "MIU will be run by locally" << endl;

	std::string write_bb_fn = std::string(output_directory) + "/blackboard.out";
	//boost::filesystem::path write_bb_fn = std::string(output_directory) / std::string("blackboard.out");

	Point os_tl(-1,-1,-1), os_br(-1,-1,-1);
	int* im_inst_nums;
	int num_slices_z;

	int ok = 0;

	std::string extension = pcl::FileNameTokenizer(image_file).getExtensionWithoutDot();
	boost::shared_ptr<MedicalImageSequence> mis_ptr;
	if (extension.compare("txt")==0 || extension.compare("seri")==0 || extension.compare("ser")==0 || extension.compare("sers")==0) {
		cout << "Reading dicom image data..." << endl;
		std::vector<std::string> im_fname; 
		// std::ofstream os(std::string(output_directory) + "\\dicom.seri"); //MWW 03282020
		std::ofstream os(std::string(output_directory) + "/dicom.seri");
		std::ifstream fp(image_file);
		if (!fp) {
			cerr << "ERROR: unable to open input file: " << image_file << endl;
			exit(1);
		}
		char dummy[300];
		while(!fp.eof()) {
			fp.getline(dummy, 300);
			std::string file = std::string(dummy);
			boost::algorithm::trim(file);
			if (!file.empty()) {
				im_fname.push_back(std::string(dummy));
				os << std::string(dummy) << endl;
			}
		}
		fp.close();
		os.close();
		num_slices_z = im_fname.size();
		cout << "im_fname = " << im_fname[0] << endl;
		mis_ptr.reset(new DICOMsequence(im_fname));
	} else {
		mis_ptr.reset(new PCLsequence(image_file));
		// std::ofstream os(std::string(output_directory) + "\\source_image.txt"); //MWW 03282020
		std::ofstream os(std::string(output_directory) + "/source_image.txt");
		std::string abs_image = boost::filesystem::absolute(image_file).string(),
			// abs_out = boost::filesystem::absolute(output_directory).string()+"\\";
			abs_out = boost::filesystem::absolute(output_directory).string()+"/";
		if (boost::starts_with(abs_image, abs_out)) {
			os << abs_image.substr(abs_out.length());
		} else os << abs_image;
		os.close();
		num_slices_z = mis_ptr->num_images();
	}
	MedicalImageSequence &mis = *mis_ptr;

	cout << "Done reading " << num_slices_z << " slices" << endl;

	int i;
	im_inst_nums = new int [num_slices_z];
	for(i=0; i<num_slices_z; i++) {
		im_inst_nums[i] = mis.instance_number(i);
	}

	cout << "checking slice locations....." << endl;
	// Check that slice locations are valid and reorder if necessary
	int prevLocSet=0, prevDiffSet=0;
	float currDiff=0, prevLoc=0, prevDiff=0;
	int slice_loc_ok=1;
	float* sliceLocs = new float [mis.zdim()];
	for(i=0; i<mis.zdim() && slice_loc_ok; i++) {
		float ih_location = mis.slice_location(i);
		//cout << "i=" << i << endl;
		//cout << "inst num=" << mis.image(i).instance_number() << endl;
		//cout << "loc=" << ih_location << endl;

        if (prevLocSet) {
  			currDiff = ih_location - prevLoc;

  			if (prevDiffSet) slice_loc_ok = ((currDiff*prevDiff)>0);
  			prevDiff = currDiff;
  			prevDiffSet = 1;
  		}
  		//cout << "diff=" << currDiff << endl;
  		prevLoc = ih_location;
  		prevLocSet = 1;
  		if (currDiff<0) sliceLocs[i] = -ih_location;
        else sliceLocs[i] = ih_location;
        //cout << "ok=" << slice_loc_ok << endl;
      }
      if (!slice_loc_ok) {
	  	cerr << "ERROR: miu.cc: problem with slice locations: " << endl;
	  	cerr << "near image with instance number " << mis.image(i-1).instance_number() << endl;
	  	exit(1);
	  }
	  // **** MIU code assumes that slice location increases as z increases, so need this ****
      if ((currDiff<0)&&(mis.zdim()>0)) sliceLocs[0] = -1*sliceLocs[0];
      for(i=0; i<mis.zdim(); i++) {
		  mis.slice_location(i, sliceLocs[i]);
		//  cout << "slice location " << i << ": " << sliceLocs[i] << endl;
	  }
      delete [] sliceLocs;
	  cout << "Done checking slice locations." << endl;

	if (os_tl.x<0) os_tl.x = 0;
	if (os_tl.y<0) os_tl.y = 0;
	if (os_tl.z<0) os_tl.z = 0;
	if (os_br.x<0) os_br.x = mis.xdim()-1;
	if (os_br.y<0) os_br.y = mis.ydim()-1;
	if (os_br.z<0) os_br.z = mis.zdim()-1;

	// Unload (free) images that are outside bounding box given in input file
	for (int z=0; z<mis.zdim(); z++)
		if ((z<os_tl.z) || (z>os_br.z)) {
			delete [] mis.image(z).pixel_data();
		}

	std::cout << "Test: " << model_file << std::endl;
	int s_ind;
	for(s_ind=strlen(model_file)-1; (s_ind>0) && (model_file[s_ind]!='/' && model_file[s_ind]!='\\'); s_ind--);
	char path[300], model_name[300];
	if (s_ind>=0) {
		strncpy(path, model_file, s_ind);
		path[s_ind]='\0';
		strcpy(model_name, model_file+s_ind+1);
	}
	else {
		strcpy(path, "./");
		strcpy(model_name, model_file);
	}

	Model* m;
	cout << "Model name: " << model_name << endl;
	cout << "Model path: " << path << endl;

	//m = new Model (model_file, roi_directory, chromosome);
	m = new Model (model_file,chromosome);
	//cout << "Model roi_directory = " << m->roi_directory() << endl;
	cout << "Model chromosome = " << m->chromosome() << endl;

	//if (chromosome==0) {
	//	m = new Model (model_file);
	//}
	//else {
	//	m = new Model (model_file, chromosome);
	//}
	//m = new Model (model_name);
	//if (!strcmp(read_bb_fn, "#"))
	//	m = new Model (model_name);
	//else
	//	m = new PSmodel (model_name, mis.patient_id());

	//cout << "Model Version Number: " << m->set_latest_version(path) << endl;
	m->read();
	//cout << *m;

	ROI overall_sarea;
	overall_sarea.add_box(os_tl, os_br);
	Blackboard bb(mis, *m, overall_sarea, image_file, exec_directory, output_directory, roi_directory, edm_directory, 
	            stop_at_node, user_resource_directory, condor_job_directory,
				skip_normalized_image_png, skip_normalized_image_png_training,
				skip_tensorboard_logging, predict_cpu_only);
	overall_sarea.clear();

	//cout << "here: " << bb.exec_directory() << endl;

	Darray<KnowledgeSource> ks(5);
	KnowledgeSource ModelMapper("ModelMapper", "ModelKS", ModelMapperS, ModelMapperA);
	ks.push_last(ModelMapper);

	KnowledgeSource GroupFormer("GroupFormer", "SchedulerKS", GroupFormerS, GroupFormerA);
	ks.push_last(GroupFormer);

	KnowledgeSource NextGroup("NextGroup", "SchedulerKS", NextGroupS, NextGroupA);
	ks.push_last(NextGroup);

	KnowledgeSource NextSolel("NextSolel", "SchedulerKS", NextSolelS, NextSolelA);
	ks.push_last(NextSolel);

	KnowledgeSource AddMatchedCandidates("AddMatchedCandidates", "SegmentationKS", AddMatchedCandidatesS, AddMatchedCandidatesA, SegmentationR);
	ks.push_last(AddMatchedCandidates);

	KnowledgeSource DistanceMap2DPercLocal3DMax("DistanceMap2DPercLocal3DMax", "SegmentationKS", DistanceMap2DPercLocal3DMaxS, DistanceMap2DPercLocal3DMaxA, SegmentationR);
	ks.push_last(DistanceMap2DPercLocal3DMax);

	KnowledgeSource DistanceMapRegionGrowing("DistanceMapRegionGrowing", "SegmentationKS", DistanceMapRegionGrowingS, DistanceMapRegionGrowingA, SegmentationR);
	ks.push_last(DistanceMapRegionGrowing);
	
	KnowledgeSource DistanceMapWatershed("DistanceMapWatershed", "SegmentationKS", DistanceMapWatershedS, DistanceMapWatershedA, SegmentationR);
	ks.push_last(DistanceMapWatershed);

	KnowledgeSource FormCandsFromSearchArea("FormCandsFromSearchArea", "SegmentationKS", FormCandsFromSearchAreaS, FormCandsFromSearchAreaA, SegmentationR);
	ks.push_last(FormCandsFromSearchArea);

	KnowledgeSource GrowPartSolid("GrowPartSolid", "SegmentationKS", GrowPartSolidS, GrowPartSolidA, SegmentationR);
	ks.push_last(GrowPartSolid);
    
	KnowledgeSource LineToDots("LineToDots", "SegmentationKS", LineToDotsS, LineToDotsA, SegmentationR);
	ks.push_last(LineToDots);

	KnowledgeSource MaxCostPath("MaxCostPath", "SegmentationKS", MaxCostPathS, MaxCostPathA, SegmentationR);
	ks.push_last(MaxCostPath);

	KnowledgeSource NeuralNetKeras("NeuralNetKeras", "SegmentationKS", NeuralNetKerasS, NeuralNetKerasA, SegmentationR);
	ks.push_last(NeuralNetKeras);

	KnowledgeSource PlatenessThreshRegGrow("PlatenessThreshRegGrow", "SegmentationKS", PlatenessThreshRegGrowS, PlatenessThreshRegGrowA, SegmentationR);
	ks.push_last(PlatenessThreshRegGrow);

	// MWW 082920 - defined twice identically
	// KnowledgeSource MaxCostPath("MaxCostPath", "SegmentationKS", MaxCostPathS, MaxCostPathA, SegmentationR);
	// ks.push_last(MaxCostPath);

	// KnowledgeSource NeuralNetKeras("NeuralNetKeras", "SegmentationKS", NeuralNetKerasS, NeuralNetKerasA, SegmentationR);
	// ks.push_last(NeuralNetKeras);

	// KnowledgeSource PlatenessThreshRegGrow("PlatenessThreshRegGrow", "SegmentationKS", PlatenessThreshRegGrowS, PlatenessThreshRegGrowA, SegmentationR);
	// ks.push_last(PlatenessThreshRegGrow);

	KnowledgeSource ReadMatchedRoi("ReadMatchedRoi", "SegmentationKS", ReadMatchedRoiS, ReadMatchedRoiA, SegmentationR);
	ks.push_last(ReadMatchedRoi);

	KnowledgeSource SameCandidatesAs("SameCandidatesAs", "SegmentationKS", SameCandidatesAsS, SameCandidatesAsA, SegmentationR);
	ks.push_last(SameCandidatesAs);
    
	KnowledgeSource ThreshRegGrow("ThreshRegGrow", "SegmentationKS", ThreshRegGrowS, ThreshRegGrowA, SegmentationR);
	ks.push_last(ThreshRegGrow);

	KnowledgeSource ImCandConf("ImCandConf", "InferencingKS", ImCandConfS, ImCandConfA, ImCandConfR);
	ks.push_last(ImCandConf);

	KnowledgeSource FormGroupCands("FormGroupCands", "InferencingKS", FormGroupCandsS, FormGroupCandsA);
	ks.push_last(FormGroupCands);

	KnowledgeSource GroupCandConf("GroupCandConf", "InferencingKS", GroupCandConfS, GroupCandConfA);
	ks.push_last(GroupCandConf);

	KnowledgeSource MatchCands("MatchCands", "InferencingKS", MatchCandsS, MatchCandsA, MatchCandsR);
	ks.push_last(MatchCands);

	KnowledgeSource FreeCandidates("FreeCandidates", "MemManageKS", FreeCandidatesS, FreeCandidatesA, FreeCandidatesR);
	ks.push_last(FreeCandidates);

	
	int best_ind = 0;
	float act_score, best_score=1.0;
	while (best_score>0.0) {
		best_score = 0.0;
//cout << "computing activation scores" << endl;
		for(i=0; i<ks.N(); i++) {
			act_score = ks[i].activation_score(bb);
//cout << ks[i].name() << "   " << act_score << endl;
			if (act_score>best_score) {
				best_score = act_score;
				best_ind = i;
			}
		}

		if (best_score>0.0) {
			cout << "Activating " << ks[best_ind].name() << "...." << endl;
			ks[best_ind].activate(bb);
			cout << "Done" << endl;
			ks[best_ind].add_activation_rec(bb);
		}

		// *******
		// if a lung ROI was passed into do_segmentation and the BB contains a (lung_prone_model with an unmatched air_containing SE) OR a (lung_model with an unmatched lung SE)
		// then set the best candidate of the SE to be the ROI => set the matched candidate of the SE (see InferencingKS.MatchCandsA) find the group containing the SE and set its priority to -1.0 so that it won't be processed any further (see SchedulerKS.NextGroupA), also make sure it is the only SE in the group (otherwise throw an exception?)
		// set the lung ROI argument to null
		// *******
	}

	// Modify Rois to accommodate image instance numbers
	// check if image instance numbers are continuous to decide whether ROIs can be translated using their own method or whether the translation method defined in this class is required
	cout << "translating rois" << endl;
	int cts=1;
	for(int j=1; (j<num_slices_z) && cts; j++) {
		cts = (im_inst_nums[j]==(im_inst_nums[j-1]+1));
	}
	//cout << "cts=" << cts << endl;
	for(int j=0; j<bb.num_sol_elements(); j++) {
		SolElement& se = bb.sol_element(j);
		//cout << se.name() << endl;
		for(int k=0; k<se.num_candidates(); k++) {
			//cout << "candidate" <<endl;
			if (cts)
				se.candidate(k)->primitive()->translate_z(im_inst_nums[0]);
				//se.candidate(k)->primitive()->translate_to_inst_nums(im_inst_nums[0]);
			else
				se.candidate(k)->primitive()->map_to_inst_nums(im_inst_nums);
		}
		//cout << "se.matched_prim(): " << se.matched_prim() << endl;
		if (se.matched_prim()!=0) {
			//cout << "matched_prim" <<endl;
			if (cts) {
        // sumit - crashes on windows, with same inp, etc.
				//if (j==(bb.num_sol_elements()-1)) {
				//((ImageRegion*)se.matched_prim())->roi().print_all_points();
				//cout << "done print" << endl;
				//}
				se.matched_prim()->translate_z(im_inst_nums[0]);
				//se.matched_prim()->translate_to_inst_nums(im_inst_nums[0]);
				//cout << se.name() << " done" << endl;
			}
			else {
				//if (j==(bb.num_sol_elements()-1)) {
				//((ImageRegion*)se.matched_prim())->roi().print_all_points();
				//cout << "done print" << endl;
				//}
				//ImageRegion* ir = (ImageRegion*) se.matched_prim();
				//Point fp, lp;
				//ir->roi().first_point(fp);
				//ir->roi().last_point(lp);
				//cout << fp << lp << endl;

				se.matched_prim()->map_to_inst_nums(im_inst_nums);
			}
		}
	}
	cout << "done" << endl;

	//Actual saving of black board file
	cout << "Writing blackboard to " << write_bb_fn << " ..." << endl;
	//cout << "Writing blackboard to " << write_bb_fn.native() << " ..." << endl;
	{
		//ofstream outfile(write_bb_fn.native());
		ofstream outfile(write_bb_fn);
		if (!outfile) {
			write_bb_fn = std::string(output_directory) + "\\blackboard.out";
			cout << "Writing blackboard to " << write_bb_fn << " ..." << endl;
			outfile.open(write_bb_fn);
		}
		if (!outfile) {
			cerr << "ERROR: unable to open output file for writing: " << write_bb_fn << endl;
			exit(1);
		}
		bb.write_sol_elements(outfile);
		outfile.close();
	}
	cout << "done" << endl;

	//Writing of the black board in the new format
	cout << "Writing new format output to " << output_directory << " ..." << endl;
	{
		// std::string sol_element_file = std::string(output_directory)+"\\solution_info.txt"; //MWW 03282020
		std::string sol_element_file = std::string(output_directory)+"/solution_info.txt"; //MWW 03282020
		cout << "Writing solution info to " << sol_element_file << " ..." << endl;
		ofstream outfile(sol_element_file);
		// ofstream file_list_out(std::string(output_directory)+"\\file_list.txt"); //MWW 03282020
		ofstream file_list_out(std::string(output_directory)+"/file_list.txt"); //MWW 03282020
		if (!outfile) {
				sol_element_file = std::string(output_directory)+"\\solution_info.txt";
				cout << "Writing solution info to " << sol_element_file << " ..." << endl;
				outfile.open(sol_element_file);
				file_list_out.open(std::string(output_directory)+"\\file_list.txt");
		}
		if (!outfile) {
			cerr << "ERROR: unable to open output file for writing: " << sol_element_file << endl;
			exit(1);
		}
		for (int sol_index = 0; sol_index<bb.num_sol_elements(); ++sol_index) {
			auto &sol_elem = bb.sol_element(sol_index);
			outfile << "SolElement: " << sol_elem.name() << endl;
			for(int i=0; i<sol_elem.num_attributes(); i++)
				sol_elem.attribute(i)->write(outfile);
			outfile << "Num_Cands: " << sol_elem.num_candidates() << endl;
			int matched_cand_num, match_prim;
			match_prim = sol_elem.num_matched_cands(matched_cand_num);
			outfile << "Num_Matched_Cands: " << matched_cand_num << endl;

			// // possible option to save search area
			// std::cout << "#";  std::cout << std::flush;
			// std::string search_area_primitive_file = 'search_area_' + sol_elem.name()+sol_elem.matched_prim()->extension();
			// std::cout << "Saving: " << search_area_primitive_file << "...";  std::cout << std::flush;
			// ofstream os(std::string(output_directory)+"/"+search_area_primitive_file);
			// if (!os) os.open(std::string(output_directory)+"\\"+search_area_primitive_file);
			// // sol_elem.matched_prim()->writeEssentialOnly(os);
			// // sol_elem.search_area()->writeEssentialOnly(os);
			// os.close();
			// std::cout << "Done" << std::endl;  std::cout << std::flush;
			// file_list_out << search_area_primitive_file << std::endl;
			// outfile << "Search area file for PrimitiveRoi: " << search_area_primitive_file << std::endl;

			std::vector<bool> is_matched(sol_elem.num_candidates(), false);
			for (int i=0; i<matched_cand_num; ++i) is_matched[sol_elem.matched_cand_index(i)] = true;
			//Writing the ROI files
			int num_candidate_str_len = boost::lexical_cast<std::string>(sol_elem.num_candidates()).length();
			for (int i=0; i<sol_elem.num_candidates(); ++i) {
				std::cout << "@"; std::cout << std::flush;
				auto candidate = sol_elem.candidate(i);
				outfile << "Candidate_start" << endl;
				outfile << "Type: " << candidate->primitive()->type() << endl;
				outfile << "FeatureValue:";
				std::cout << "@"; std::cout << std::flush;
				for (int j=0; j<candidate->num_attributes(); ++j) {
					outfile << " ";
					if (candidate->conf_score(j)!=2.0) outfile << candidate->feature_value(j);
					else outfile << 0;
				}
				outfile << std::endl;
				outfile << "ConfidenceScore:";
				std::cout << "@"; std::cout << std::flush;
				for (int j=0; j<candidate->num_attributes(); ++j) outfile << " " << candidate->conf_score(j);
				outfile << std::endl;
				outfile << "PartialConfidence: " << candidate->partial_confidence() << std::endl;
				outfile << "Matched: ";
				if (is_matched[i]) outfile << "True";
				else outfile << "False";
				outfile << std::endl;
				std::cout << "@"; std::cout << std::flush;
				std::string index_str = boost::lexical_cast<std::string>(i);
				while (index_str.length()!=num_candidate_str_len) index_str = "0"+index_str;
				std::string primitive_file = sol_elem.name()+"-"+index_str;
				if (is_matched[i]) primitive_file += "_m";
				primitive_file += candidate->primitive()->extension();
				std::cout << "Saving: " << primitive_file << "...";  std::cout << std::flush;
				// ofstream os(std::string(output_directory)+"\\"+primitive_file); //MWW 03282020
				ofstream os(std::string(output_directory)+"/"+primitive_file); //MWW 03282020
				if (!os) os.open(std::string(output_directory)+"\\"+primitive_file); //MB 210121
				candidate->primitive()->writeEssentialOnly(os);
				os.close();
				std::cout << "Done" << std::endl;  std::cout << std::flush;
				outfile << "RoiFile: " << primitive_file << std::endl;
				file_list_out << primitive_file << std::endl;

				outfile << "Candidate_end" << endl;
			}

			if (match_prim) {
				std::cout << "#";  std::cout << std::flush;
				std::string primitive_file = sol_elem.name()+sol_elem.matched_prim()->extension();
				std::cout << "Saving: " << primitive_file << "...";  std::cout << std::flush;
				// ofstream os(std::string(output_directory)+"\\"+primitive_file); //MWW 03282020
				ofstream os(std::string(output_directory)+"/"+primitive_file); //MWW 03282020
				if (!os) os.open(std::string(output_directory)+"\\"+primitive_file); //MB 210121
				sol_elem.matched_prim()->writeEssentialOnly(os);
				os.close();
				std::cout << "Done" << std::endl;  std::cout << std::flush;
				file_list_out << primitive_file << std::endl;
				outfile << "MatchedPrimitiveRoiFile: " << primitive_file << std::endl;
			}

			outfile << endl;
		}
		outfile.close();
		file_list_out.close(); 
	}
	cout << "done" << endl;


	//UNCOMMENT NEXT LINE TO GENERATE OLD OUTPUT FILE FORMAT
	//write_bb_to_file(bb, "/data6/seg/test.out");

	//cout << endl;
	//bb.write_sol_elements(cout);
	//cout << endl;
	//bb.write_groups(cout);
	//cout << endl;
	//bb.write_act_recs(cout);

	cout << "Almost Done - do_segmentation" << endl;
	delete m;
	delete im_inst_nums;
	cout << "Done - do_segmentation" << endl;

	//const ActivationRecord* ar = bb.find_act_rec("DistanceMapWatershed", "SegmentationKS", "Writing EDM files");
	//const std::string s1 = ar->message(ar->find_message_starting_with("EDM Num Pix"));
	//const std::string s2 = ar->message(ar->find_message_starting_with("EDM Index"));
	//cout << s1 << endl;
	//cout << s2 << endl;

	return 0;
}

bool is_directory_used(const std::string& path)
{
	if(!boost::filesystem::is_directory(path)) return false;
	boost::filesystem::directory_iterator end_it;
	boost::filesystem::directory_iterator it(path);
	if(it == end_it)
		return false;
	else
		return true;
}

char* hexToBinary (const char* hexChromosome) {
	int p[] = {1, 2, 4, 8};
	char* chromosome = new char [4*strlen(hexChromosome)+1];
	for(int i=0; i<strlen(hexChromosome); i++) {
		char hexchar = hexChromosome[i];
		int intval = (hexchar >= 'A') ? (hexchar - 'A' + 10) : (hexchar - '0');
		for(int j=3; j>=0; j--) {
			if (intval>=p[j]) {
				chromosome[i*4+j] = '1';
				intval -= p[j];
			}
			else chromosome[i*4+j] = '0';
		}		
	}
	chromosome[4*strlen(hexChromosome)] = '\0';
	return chromosome;
}

bool isBinary (const char* s) {
	bool isBin = true;
	for(int i=0; (i<strlen(s)) && isBin; i++) {
		isBin = ((s[i]=='0') || (s[i]=='1'));
	}
	return isBin;
}


int main(int argc, char *argv[]) pcl_MainStart {
					//std::cout << "Number of arguments " << argc << std::endl;
					//for (int m=0; m<argc; m++) std::cout << "Argument " << m << ": " << argv[m] << std::endl;
	
	/*
	Point tl(5,5,5), br(10,10,10);
	ROI r;
	r.add_box(tl, br);

	r.print_all_points();

	ROI se;
	Point tlse(-2,-2, -2), brse(2,2,2);
	se.add_box(tlse, brse);
	r.dilate(se);	

	r.erode(se);

	r.print_all_points();
	*/

	/*
	Darray<int> d;
	d.push_last(7);
	std::cout << d.N() << std::endl;
	std::cout << d[0] << std::endl;
	
	Point tl(5,5,5), br(10,10,10);
	ROI r;
	r.add_box(tl, br);

	Point tl2(3,3,3), br2(7,7,7);
	ROI r2;
	r2.add_box(tl2, br2);

	r.subtract(r2);

	r.print_all_points();

	ROI se;
	se.add_circle(3, 0, 0);
	r.dilate(se);	

	r.print_all_points();
	*/

	/*****/
	std::cout << "miu_nod main" << std::endl;

	pcl::CommandLineParser parser(argc, argv);
	parser.setOverallDescription("Performs model-based image segmentation. Example usage: Q:\\nodule_detection\\Executables\\miu_nod.exe M:\\apps\\personal\\mbrown\\image.seri X:\\seg_model\\lung_prone_model\\lung_prone_model.1\\lung_prone_model M:\\apps\\personal\\mbrown\\output -r M:\\apps\\personal\\mbrown\\roi");
	parser.addArgument<std::string>("image_file", "IMAGE_FILE", "A text file containing a list of DICOM files comprising a series. The full path is listed, one file per line, e.g., \\\\research\\qiws15\\LIDC-05\\2000-01-01\\0131.dcm");
	parser.addArgument<std::string>("model_file", "MODEL_FILE", "A text file listing the nodes of the segmentation model, e.g., Q:\\\\seg_model\\lung_nod_model\\lung_nod_model.11\\lung_nod_model");
	parser.addArgument<std::string>("output_directory", "OUTPUT_DIRECTORY", "Directory where outputs (blackboard, ROIs, etc) will be stored, e.g., \\\\apps\\mbrown\\working\\1.3.6.1.4.1.14519.5.2.1.6279.606225");
	parser.addOption<std::string>(1, "-c", "-c CHROMOSOME", "Input chromosome (optional), e.g., 010101010100011100101010101001010101010");
	parser.addOption<std::string>(1, "-r", "-r ROI_DIRECTORY", "Directory where pre-computed (input) ROIs are stored. ROIs should use DICOM instance numbers as their Z-coordinate. The ROI filenames are of the form ModelNodeName.roi");
	parser.addOption<std::string>(1, "-d", "-d WORKING_DIRECTORY", "Directory where working files (e.g., EDM, CNN) are read from if they exist (were computed previously) or are written otherwise.");
	parser.addOption<std::string>(1, "-s", "-s STOP_AT", "Name of model node at which to stop processing.");
	parser.addOption("-f", "Force usage of OUTPUT_DIRECTORY (WARNING: will delete whatever files or directories are currently in OUTPUT_DIRECTORY!)");
	parser.addOption<std::string>(1, "-u", "-u USER_RESOURCE_DIRECTORY", "Directory where the resource configuration files for CNN nodes are stored. If this is not specified, default resource (a single-core CPU) will be used instead.");
	parser.addOption<std::string>(1, "-w", "-w CONDOR_JOB_DIRECTORY", "Directory where the condor job and resource configuration files for CNN nodes are stored. If this is not specified, it will be run by locally instead of using Condor.");
	parser.addOption("-p", "Prediction only with a single-core CPU");
	parser.addOption("-i", "Skip generating png image to review the normalized input");
	parser.addOption("-it", "Skip generating png image to review the normalized input for training phase");
	parser.addOption("-t", "Skip generating tensorboard logging");
	parser.update();

	//cout << argv[0] << endl;
	boost::filesystem::path full_path( boost::filesystem::initial_path<boost::filesystem::path>() );
    full_path = boost::filesystem::system_complete( boost::filesystem::path( argv[0] ) );
    //std::cout << full_path << std::endl;
    //Without file name
    //std::cout << full_path.stem() << std::endl;
    //std::cout << boost::filesystem::basename(full_path) << std::endl;
	//std::cout << full_path.parent_path() << std::endl;

	std::string image_file = parser.get("image_file")->getElementDatum(),
		model_file = parser.get("model_file")->getElementDatum(),
		output_directory = parser.get("output_directory")->getElementDatum(),
		exec_directory = (full_path.parent_path()).string();

	//std::cout << exec_directory << std::endl;
	//std::cout << output_directory << std::endl;

	if (!boost::filesystem::exists(output_directory)) boost::filesystem::create_directories(output_directory);
	else {
		if (is_directory_used(output_directory)) {
			if (parser.get("-f")->declared()) {
				std::cout << "Deleting " << output_directory << std::endl;
				boost::filesystem::remove_all(output_directory);
				std::cout << "Recreating " << output_directory << std::endl;
				bool error = true;
				while (error) {
					error = false;
					try {
						boost::filesystem::create_directories(output_directory);
					} catch (...) {
						error = true;
						//boost::this_thread::sleep(boost::posix_time::millisec(2000));
					}
				}
			} else {
				std::cout << "Skipping as output directory " << output_directory << " is not empty!" << std::endl;
				std::cout << "Please delete the contents in " << output_directory << " if you wish or use -f option (WARNING: will delete whatever files or directories in OUTPUT_DIRECTORY!) to reuse it." << std::endl;
				return 0;
			}
		}
	}

	//if (parser.get("-c")->declared()) {
	//	return do_segmentation(image_file.c_str(), model_file.c_str(), output_directory.c_str(), parser.get("-c")->getElementDatum().c_str());
	//} else {
	//	return do_segmentation(image_file.c_str(), model_file.c_str(), output_directory.c_str(), 0);
	//}

	char* roi_directory = 0;
	char* chromosome = 0;
	char* working_directory = 0;
	char* stop_at_node = 0;
	char* user_resource_directory = 0;
	char* condor_job_directory = 0;
	bool skip_normalized_image_png = false;
	bool skip_normalized_image_png_training = false;
	bool skip_tensorboard_logging = false;
	bool predict_cpu_only = false;
	if (parser.get("-r")->declared()) {
		//std::cout <<  parser.get("-r")->getElementDatum().c_str() << std::endl;
		roi_directory = new char [strlen(parser.get("-r")->getElementDatum().c_str())+1];
		strcpy(roi_directory, parser.get("-r")->getElementDatum().c_str());
	}
	if (parser.get("-d")->declared()) {
		working_directory = new char [strlen(parser.get("-d")->getElementDatum().c_str())+1];
		strcpy(working_directory, parser.get("-d")->getElementDatum().c_str());
	}
	if (parser.get("-c")->declared()) {
		std::cout <<  parser.get("-c")->getElementDatum().c_str() << std::endl;
		if (isBinary(parser.get("-c")->getElementDatum().c_str())) {
			chromosome = new char [strlen(parser.get("-c")->getElementDatum().c_str())+1];
			strcpy(chromosome, parser.get("-c")->getElementDatum().c_str());
		}
		else {
			chromosome = hexToBinary(parser.get("-c")->getElementDatum().c_str());
		}
	}
	if (parser.get("-s")->declared()) {
		stop_at_node = new char [strlen(parser.get("-s")->getElementDatum().c_str())+1];
		strcpy(stop_at_node, parser.get("-s")->getElementDatum().c_str());
	}
	if (parser.get("-u")->declared()) {
		user_resource_directory = new char [strlen(parser.get("-u")->getElementDatum().c_str())+1];
		strcpy(user_resource_directory, parser.get("-u")->getElementDatum().c_str());
	}
	if (parser.get("-w")->declared()) {
		condor_job_directory = new char [strlen(parser.get("-w")->getElementDatum().c_str())+1];
		strcpy(condor_job_directory, parser.get("-w")->getElementDatum().c_str());
	}
	if (parser.get("-p")->declared()) {
		std::cout << "Prediction only with a single-core CPU" << std::endl;
		predict_cpu_only = true;
	}
	if (parser.get("-i")->declared()) {
		std::cout << "Skipping generate png normalized input " << std::endl;
		skip_normalized_image_png = true;
	}
	if (parser.get("-it")->declared()) {
		std::cout << "Skipping generate png normalized input for training" << std::endl;
		skip_normalized_image_png_training = true;
	}
	if (parser.get("-t")->declared()) {
		std::cout << "Skipping generate tensorboard logging " << std::endl;
		skip_tensorboard_logging = true;
	}

	int stat = do_segmentation(image_file.c_str(), model_file.c_str(), exec_directory.c_str(), output_directory.c_str(), 
	                        roi_directory, working_directory, chromosome, stop_at_node, 
							user_resource_directory, condor_job_directory,
							skip_normalized_image_png, skip_normalized_image_png_training, skip_tensorboard_logging, predict_cpu_only);

	// ***** MASK TEST ****
	//int stat = 1; 
	//int xdim, ydim, zdim;
	//int vox_cnt = 0;
	//unsigned char* lung_mask = radlogics_segmentation(image_file.c_str(), "lung_basic_model\\lung_basic_model.1\\lung_basic_model", output_directory.c_str(), "lung_init", xdim, ydim, zdim);
	//tl.x=xdim; tl.y=ydim; tl.z=zdim;
	//br.x=0; br.y=0; br.z=0;
	//for (int k=0; k<zdim; k++) {
	//	for (int j=0; j<ydim; j++) {
	//		for (int i=0; i<xdim; i++) {
	//			if (lung_mask[k*xdim*ydim + j*xdim + i] == '1') {
	//				vox_cnt++;
	//				if (i<tl.x) tl.x=i;
	//				if (j<tl.y) tl.y=j;
	//				if (k<tl.z) tl.z=k;
	//				if (i>br.x) br.x=i;
	//				if (j>br.y) br.y=j;
	//				if (k>br.z) br.z=k;
	//			}
	//		}
	//	}
	//}
	//std::cout <<  "LUNG MASK C++" << std::endl;
	//std::cout << "Num voxels = " << vox_cnt << std::endl;
	//std::cout << "TL = " << tl.x << ", " << tl.y << ", " << tl.z << std::endl;
	//std::cout << "BR = " << br.x << ", " << br.y << ", " << br.z << std::endl;


	return stat;
	/****/
} pcl_MainEnd(std::cout);

/*
void write_bb_to_file(Blackboard& bb, const char* const filename)
{
	ofstream s(filename);
	if (!s) {
		cerr << "ERROR: write_bb_to_file: unable to open " << filename << " for writing" << endl;
		exit(1);
	}

	s << bb.num_sol_elements() << endl;
	int i, j, k;
	for(i=0; i<bb.num_sol_elements(); i++) {
		SolElement& se = bb.sol_element(i);
		s << se.name() << endl;

		s << "1 1 1 1 1 1 1" << endl;
		//fprintf(fp, "%f %i %i %i %i %f %i\n", bbf.priority, bbf.low_H, bbf.high_H, bbf.low_grey, bbf.high_grey, bbf.non_cont_conf_frac, bbf.xy_step);

		s << "0" << endl;
		//if (bbf.struct_el) {
			//fprintf(fp, "%i\n", 1);
			//write_repr(fp, bbf.struct_el);
		//}
		//else
			//fprintf(fp, "%i\n", 0);


		s << "0" << endl;
		//fprintf(fp, "%i\n", bbf.seg_consts->N);

		s << se.num_attributes() << endl;
		//fprintf(fp, "%i\n", bbf.feats->N);


		for(j=0; j<se.num_attributes(); j++) {
			const Attribute* const a = se.attribute(j);
			s << a->name() << endl;
			if (a->num_rel_solels()>0)
				s << 1 << endl << bb.sol_element(a->rel_solel_index(0)).name() << endl;
			else
				s << 0 << endl;

			s << 0 << endl;
			// no parameter points
		}


		// Compute all feature values and confidences for current solution element
		bb.next_solel(i);
		ImCandConfA(bb);


		int num_matched_cands;
		se.num_matched_cands(num_matched_cands);
		if ((num_matched_cands!=1) && se.matched_prim()) {
			if (strcmp(se.matched_prim()->type(), "ImageRegion")!=0) {
				cerr << "ERROR: Blackboard: write_bb_to_file: Matched primitive must be of type ImageRegion" << endl;
				exit(1);

			}

			ImageRegion* ir = new ImageRegion(((ImageRegion*)se.matched_prim())->roi(), bb.med_im_seq());
			se.add_candidate(ir);
		}


		s << se.num_candidates() << endl;
		for(j=0; j<se.num_candidates(); j++) {

			if (strcmp(se.candidate(j)->primitive()->type(), "ImageRegion")!=0) {
				cerr << "ERROR: Blackboard: write_bb_to_file: Image primitive must be of type ImageRegion" << endl;
				exit(1);

			}
			ImageRegion *ir = (ImageRegion *) se.candidate(j)->primitive();
			const ROI& r = ir->roi();

			Point fp, lp;
			int plane_cnt = 0, z;
			r.first_point(fp);
			r.last_point(lp);
			for(z=fp.z; z<=lp.z; z++)
				if (!r.empty(z))
					plane_cnt++;
			s << plane_cnt << endl;

			Point p, p1, p2;
			ROItraverser rt(r);
			TravStatus st = rt.valid();
			while(st<END_ROI) {
				rt.current_point(p);

				int ivl_cnt=0;
				st = ROI_STAT_OK;
				while(st<NEW_PLANE) {
					ivl_cnt++;
					st = rt.next_interval();
				}
				s << p.z << " " << 1 << " " << ivl_cnt << endl;
				//fprintf(fp, "%i %i %i\n", rp[i].z, rp[i].xy_step, rp[i].eps->N);

				rt.set_plane(p.z);
				st = ROI_STAT_OK;
				while(st<NEW_PLANE) {
					rt.current_interval(p1, p2);
					s << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << endl;
					st = rt.next_interval();
				}
			}

			int feat_count=0;
			for(k=0; k<se.num_attributes(); k++) {
				//const Attribute* const a = se.attribute(k);
				se.attribute(k);
				if (se.candidate(j)->conf_score(k)<2)
					feat_count++;
			}
			s << feat_count << endl;

			for(k=0; k<se.num_attributes(); k++) {
				//const Attribute* const a = se.attribute(k);
				se.attribute(k);
				if (se.candidate(j)->conf_score(k)<2) {
					s << k << " " << (int)(se.candidate(j)->feature_value(k)) << " " << se.candidate(j)->conf_score(k) << endl;
					// fprintf(fp, "%i %i %f\n", get_feat_ind(bbf, vp[j].feat), vp[j].val, vp[j].conf);
}
			}


			s << 1  << " " << fp.z << " " << lp.z << endl;
			//fprintf(fp, "%i %i %i\n", cp[i].xy_step, cp[i].low_z, cp[i].high_z);

			s << 0 << endl;
			//if (cp[i].cent) {
				//fprintf(fp, "%i\n", 1);
				//fprintf(fp, "%i %i %i\n", cp[i].cent->x, cp[i].cent->y, cp[i].cent->z);
			//}
			//else
				//fprintf(fp, "%i\n", 0);

			s << 1 << endl;
			//fprintf(fp, "%f\n", cp[i].conf);
		}


		if ((num_matched_cands!=1) && se.matched_prim()) {
			s << 1 << endl;
			s << se.num_candidates()-1 << endl;
		}
		else if (num_matched_cands==1) {
			s << 1 << endl;
			s << se.matched_cand_index(0) << endl;
		}
		else {
			s << 0 << endl;
		}
	}
}
*/