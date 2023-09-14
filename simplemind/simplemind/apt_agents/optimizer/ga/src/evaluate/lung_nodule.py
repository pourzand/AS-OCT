### This is broken in APT/GA 3.0 ###

"""
Making your own evaluate document:

Need:

evaluate_task(result_path, reference_path, outpath, dataset="lidc", id=None, skip_images=False)
- evaluate task
    - compute case-wise evaluation measures
    - generate screenshots per case


compile function
    - [optional] generate overall evaluation/performance diagram
    - compile overall evaluation measures
    - generate report



"""


import os
import yaml

import numpy as np
try:
    from simplemind import __qia__
    import qia.common.img.image as qimage
    import qia.common.img.overlay as qovr
    from qia.common.img.utils import get_casted_roi
    from qia.common.img.region import Region
except:
    print("Can't import QIA img modules.")
# from qia.segmentation.ct_nodule import get_template_from_outpath, iter_outpath_nodule2

# from qia.segmentation.ct_nodule.conf import DEFAULT_TEMPLATE, DEFAULT_SUB_TEMPLATE
# from qia.segmentation.ct_nodule.reference import load as load_reference
# from qia.segmentation.ct_nodule.report import generate_report

from ga.core.ga_reader import BaseResultsLoader


SCREENSHOT_SIZE_MM = 50
LEVEL = -550
WINDOW = 1600

def get_default_orientation():
    ORIENTATION = {
        "c": dict(crsstype=qovr.CrossSection.custom, xaxis=(1,0,0), yaxis=(0,0,-1)),
        "s": dict(crsstype=qovr.CrossSection.custom, xaxis=(0,1,0), yaxis=(0,0,-1)),
        "a": dict(crsstype=qovr.CrossSection.axial),
    }
    return ORIENTATION


"""
Generating a screenshot per nodule
    :param pos: center position of nodule (image coordinates)
    :param image: loaded QIA image obj 
    :param mask: loaded detected nodule ROI
    :param prefix: path/naming prefix  
    :param truth_mask: [optional] loaded matching truth ROI
    :param paths_only: True -> Not making the images, just returning the paths 

- Will save to "[prefix]_[k]_org.png" where [prefix] is the prefix path (eg "/path/to/something/with_this_prefix")
    and [k] is the orientation -- in this case, "c", "s", or "a" (coronal, sagittal, axial)
- Will make an original "org" version without overlay, and overlay version (without "org")
"""
def generate_screenshot(pos, image, mask, prefix, truth_mask=None, paths_only=False):
    ret = {}

    if not paths_only:
        minp = image.to_image_coordinates([i-SCREENSHOT_SIZE_MM/2 for i in pos])
        maxp = image.to_image_coordinates([i+SCREENSHOT_SIZE_MM/2 for i in pos])
        region = [[round(min(i,j)) for i,j in zip(minp, maxp)], [round(max(i,j)) for i,j in zip(minp, maxp)]]
    ret = {}
    ORIENTATION = get_default_orientation()
    for k,o in ORIENTATION.items():
        org_file = "%s_%s_org.png" % (prefix, k)
        ovr_file = "%s_%s.png" % (prefix, k)
        if not paths_only:  # arranges and writes screenshots
            ### This is just a fancy way to handle all the screenshot orientations in one object
            gen = qovr.auto(pos, image=image, region=region, **o)   
            gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)
            gen.write(org_file)
            # gen.add(mask, (255,0,0), 1, boundval=0)
            gen.add(mask, (255,0,0), .5, boundval=0)
            if truth_mask is not None:
                gen.add(truth_mask, (0,255,0), 0.2, boundval=0)
            gen.write(ovr_file)
        ### filling dict with file paths
        ret["%s_org" % k] = org_file
        ret["%s_ovr" % k] = ovr_file
    return ret



from ga.evaluate import Evaluator, ResultsCompiler

############# EXAMPLE EVALUATOR FOR NODULE CG DETECTION #####################
### TODO: bring this in from private repo and save in ga directory
from legacy.qia.segmentation.ct_nodule.reference import load as load_reference_lidc
from legacy.qia.segmentation.ct_nodule import get_template_from_outpath, iter_outpath_nodule2

class Evaluator_Nodule_CG(Evaluator):
    def __init__(self):
        ### TODO: Make sure this works for nodule detection
        # visuals = ["trachea_cnn_*.png",]
        # roi_dict = {"trachea": "TraCh"}
        # rois = list(roi_dict.values())
        # super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        super().__init__()
        return
    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        if dataset=="lidc":
            truth_union = None
            truth_info = {}
            reference = load_reference_lidc(reference_path)
            for i,info,m in reference.iter_roi():
                truth_info[i] = {
                    "diameter": info["diameter"],
                    "perp_diameter": info["perpendicular_diameter"],
                    "z": info.get("z", "NA"),
                    "mask": m,
                    "region": info["region"]
                }
                if truth_union is None:
                    truth_union = m
                else:
                    truth_union = truth_union + m
            reference.truth_union = truth_union
            reference.truth_info = truth_info
            return reference
        
    def roi_wise_visualization(self, res, template, outpath, roi_out, skip_images=False, extra=None):
        if extra is None: extra = dict()
        try:
            from simplemind import __qia__
            import qia.common.img.overlay as qovr
        except:
            print("Failed to import qimage (evaluate.py)")
        pos = list(template.to_image_coordinates(qovr.get_centroid(roi_out)))
        pos[2] = res["z"]
        pos = list(template.to_physical_coordinates(pos))
        res_ss = dict()
        if res.get("type")=="TP":
            res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_TP" % res["id"]), truth_mask=res["truth_mask"]))
        elif res.get("type")=="pTP_overlap":
            if res.get("pTP_reason", "overlap")=="fp_ref":
                    # specific for nodule detection
                res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_pTP" % res["id"]), truth_mask=extra.get("fp_ref")))
            else:   # default pTP screenshot
                res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_pTP" % res["id"]), truth_mask=res["truth_mask"]))
        elif res.get("type")=="FP":
            res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_FP" % res["id"]), paths_only=skip_images))
        else:
            print("No classification given for this ROI.")
        return res_ss

    def _process_match(self, res, roi_out, extra=None):
        if extra is None: extra=dict()
        # res = {}
        if res.get("truth_mask") is not None:  # if there is a match with "hard" truth                  
            if res["dice_coefficient"] > .25:    #041217 added Dice Coefficient requirement for true positive
                res["type"] = "TP"
                # detected_truths.add(matched_truth_info["index"])
            else:
                res["type"] = "pTP"
                res["pTP_reason"] = "overlap"
        elif extra.get("fp_ref") is not None:
            # compute intersection between soft positives truth ROI and the nodule 
            joint_hist = roi_out.get_joint_histogram(extra["fp_ref"])
            if joint_hist.get((1,1), 0)>0:  # if there is a match, then pTP
                res["type"] = "pTP"
                res["pTP_reason"] = "fp_ref"
            else:                           # elsewise, FP
                res["type"] = "FP"
        else:
            res["type"] = "FP"
        return res

    def evaluate_task(self, result_path, reference_path, outpath, dataset="lidc", id=None, skip_images=False):
        ### (1)
        ### Casted template from result_path seri file
        template = get_template_from_outpath(result_path, dummy=False)
        if template is None:
            print("Warning: Results missing for", result_path)
            return {}
        ### (2)
        ### Loading reference
        reference = self.load_reference(reference_path, dataset=dataset)

        ### (3)
        ### Extra processing
        ### In this case, getting the references for the 'other' markings
        fp_ref = reference.get_compiled_markings(load_roi=True) # loading LIDC soft-reference -- because some nodules might not be false positives

        truth = dict(truth_info=reference.truth_info, truth_union=reference.truth_union)
        extra = dict(fp_ref=fp_ref)
        return self._compute_evaluation_measures_detection(template, truth, result_path, outpath, extra=extra, skip_images=skip_images)

    def _iter_detections(self, result_path, template):
        for index, nodule, result_union in iter_outpath_nodule2(result_path, template):
            yield index, nodule, result_union
        
    def casewise_visualization(self, *args, **kargs): #dummy stand-in and don't want to use default
        return dict()

## TODO: Include in every evaluation script
def evaluate_task(result_path, reference_path, outpath, dataset="lidc", id=None, skip_images=False):
    evaluator  = Evaluator_Nodule_CG()
    return evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=id, skip_images=skip_images)
##### Results Compiler Example for CXR ######
"""
For each task, results compiler needs:
    Fitness function

    Which marking you're comparing your MIU output to (e.g. "TraCh")

    [Optional] Overall population & results visualization
    
    [Optional] Custom performance metric (e.g. "dce_score")

    [Optional] Definition of performance stratification

"""
# final_result["fitness"] = (3*max(0, 2*(final_result["sensitivity"]-0.5))) + final_result["normalized_fp"] + final_result["overlap_ratio"]
# final_result["formula"] = "(3*max(0, 2*(%(sensitivity)s-0.5))) + %(normalized_fp)s + %(overlap_ratio)s" % final_result
def fitness_nodule_cg(final_result):
    fitness = (3*max(0, 5*(final_result["sensitivity"]-0.8))) + final_result["normalized_fp"] + final_result["overlap_ratio"]
    fitness_str = "(3*max(0, 5*(%(sensitivity)s-0.8))) + %(normalized_fp)s + %(overlap_ratio)s" % final_result

    return fitness, fitness_str
    
from conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "ct_nodule", "default_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "ct_nodule", "default_sub_template.tpl")

###################

class ResultsCompiler_Nodule_CG(ResultsCompiler):
    def __init__(self, fitness_func=fitness_nodule_cg, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)
    
    # basically, input results dictionary, and the outpath (directory), and report generator for your task
    # output fitness 
    # def compile_function(result_dictionary, outpath, report_generator=None, rerun=False, canary=False):
    def compile_function(self, result_dictionary, outpath,
                        rerun=False,
                        canary=False):
        print("Compiling results...")
        """An example compile function based on some older codes.
        :param result_dictionary: A dictionary containing the evaluation results.
        :param outpath: String containing the path to the folder where compiled results will be stored.
        :param report_generator: A function for generating reports, accepts result_dictionary, final result from the compile function and outpath as input. No report will be generated by default.
        TODO: RERUN & CANARY DESCRIPTORS
        """

        outfile = os.path.join(outpath, "final.yml")
        if canary:
            outfile = os.path.join(outpath, "canary.yml")

        ### If already finished (ie aggregated & report done), just return the fitness value ### 
        if os.path.exists(outfile) and not rerun:
            print("Compile Function::: Loading", outfile)
            with open(outfile) as f:
                res = yaml.load(f)
            return (res["fitness"],)

        ### Computing per-case results ###
        per_case_result = {}
        try:
            for k,res_dict in result_dictionary.items():
                ### Migrated from _compute_evaluation
                if res_dict.get("nodule") is None:
                    res_dict["nodule"] = []
                    res_dict["finished"] = False

                if res_dict.get("overall") is None:
                    res_dict["overall"] = {"spacing":(0,0,0),}
                    res_dict["finished"] = False

                tp = fn = fp = p_tp = None
                matched_gt_nodules = []

                ### nodule-wise metrics
                for res in res_dict.get("nodule", []):
                    if tp is None:
                        tp = fn = fp = p_tp = 0
                    if res["type"]=="FN":
                        fn += 1
                    elif res["type"]=="TP":                    
                        if res["truth_id"] in matched_gt_nodules: # don't double count nodules that matched a nodule
                            continue
                        tp += 1
                        matched_gt_nodules.append(res["truth_id"])
                    elif res["type"]=="FP":
                        fp += 1
                    elif res["type"]=="pTP":
                        p_tp += 1
                    else:
                        raise ValueError('Invalid type "%s" encountered!' % res["type"])


                ### overall case metrics ###
                ovlr = None
                sensitivity = None
                normalized_fp = None
                if tp is not None and fn is not None:
                    if tp+fn>0:
                        overall = res_dict.get("overall")
                        if overall is not None:
                            ovlr = overall["tp"]/(overall["tp"]+overall["fp"]+overall["fn"])
                            sensitivity = tp/(tp+fn)
                if fp is not None:
                    normalized_fp = 1 - min(1, fp/1000)
                
                ### Saving ... ###
                per_case_result[k] = {
                    "overlap_ratio": ovlr,
                    "sensitivity": sensitivity,
                    "normalized_fp": normalized_fp,
                    "tp": tp,
                    "fn": fn,
                    "fp": fp,
                    "p_tp": p_tp,
                }
        except:
            print(result_dictionary)
            print(result_dictionary.keys())
            print(k, res_dict)
            # input("It failed...ready for results_dictionary..?")
            # print(result_dictionary)
            raise("Fail")

        ### Computing overall results
        total_tp = sum([i["tp"] for i in per_case_result.values() if i["tp"] is not None] )
        total_fn = sum([i["fn"] for i in per_case_result.values() if i["fn"] is not None] )
        total_p_tp = sum([i["p_tp"] for i in per_case_result.values() if i["p_tp"] is not None] )
        fp_list = [i["fp"] for i in per_case_result.values() if i["p_tp"] is not None]
            
        ### overall case-wise sensitivity, normalized false positive values, and overlap ratio
        s_list = [i["sensitivity"] for i in per_case_result.values() if i["sensitivity"] is not None]
        n_list = [i["normalized_fp"] for i in per_case_result.values() if i["normalized_fp"] is not None]
        o_list = [i["overlap_ratio"] for i in per_case_result.values() if i["overlap_ratio"] is not None]

        
        final_result = {
            "nodule_num": total_tp+total_fn,
            "total_tp": total_tp,
            "total_p_tp": total_p_tp,
            "nodule_sensitivity": total_tp/(total_tp+total_fn),
            
            "sensitivity_n": len(s_list),
            "sensitivity_mean": float(np.mean(s_list)) if s_list else "NA",
            "sensitivity_std": float(np.std(s_list)) if s_list else "NA",
            "sensitivity_median": float(np.median(s_list)) if s_list else "NA",
            "sensitivity_q1": float(np.percentile(s_list, 25)) if s_list else "NA",
            "sensitivity_q3": float(np.percentile(s_list, 75)) if s_list else "NA",
            
            "fp_n": len(fp_list),
            "fp_mean": float(np.mean(fp_list)),
            "fp_std": float(np.std(fp_list)),
            "fp_median": float(np.median(fp_list)),
            "fp_q1": float(np.percentile(fp_list, 25)),
            "fp_q3": float(np.percentile(fp_list, 75)),
            
            "sensitivity": float(np.mean(s_list) if s_list else 0),
            "normalized_fp": float(np.mean(n_list) if n_list else 0),
            "overlap_ratio": float(np.mean(o_list) if o_list else 0),
            "per_case_result": per_case_result,
        }
        final_result["fitness"], final_result["formula"] = self.fitness_func(final_result)
        
        ### Make the report ###
        case_wise_contents = {}
        for k in result_dictionary.keys():
            case_wise_contents[k] = {   "key": k,
                                        "per_case_result": final_result["per_case_result"][k],
                                        "tp": sorted([n for n in result_dictionary[k].get("nodule", ()) if n["type"]=="TP"], key=lambda x: x["truth_id"]),
                                        "fp": sorted([n for n in result_dictionary[k].get("nodule", ()) if n["type"]=="FP"], key=lambda x: x["id"]),
                                        "fn": sorted([n for n in result_dictionary[k].get("nodule", ()) if n["type"]=="FN"], key=lambda x: x["truth_id"]),
                                        "overall": result_dictionary[k].get("overall", {"spacing":(0,0,0),})    # to provide spacing
                                        }
        self.report_generator(result_dictionary, final_result, outpath, template=self.overall_report_template, case_wise_contents=case_wise_contents, sub_template=self.casewise_report_template, canary=canary)
        
        ### Save results to yaml file ###
        with open(outfile, "w") as f:
            f.write(yaml.dump(final_result))
            
        ### Return just the fitness value ### 
        return (final_result["fitness"],)


# can change the ratio over time

class Canary():
    # canary_data, ratio=.8, ratio_increment=0, original_ratio=None
    def __init__(self, *args, **kargs):
        
        self.canary_data = kargs.get("canary_data", None)
        self.ratio = kargs.get("ratio", .8)
        self.original_ratio = kargs.get("original_ratio", self.ratio)
        self.ratio_increment = kargs.get("ratio_increment", 0)
        self.best_fitness = kargs.get("best_fitness", 0)
        return
    def assess_perf(self, fitness):
        # it passes if the fitness is better than a ratio of the best available fitness
        print(fitness, "compared to", self.ratio*self.best_fitness)
        if fitness > self.ratio*self.best_fitness:
            return True
        return False

    def update_best_fitness(self, results):
        # parse through results to get best fitness
        # update self.best_fitness
        #   task specific
        halloffame = results
        print("Current Hall of Fame:")
        for (chromosome, score) in zip(halloffame.items, halloffame.keys):
            print(score)
            print("".join([str(x) for x in chromosome]))
        score_improved = False
        for chrom in halloffame.items:
            score = float(chrom.fitness.values[0])
            # print(score)
            if score > self.best_fitness:
                self.best_fitness = float(score)
                score_improved = True

        if not score_improved:
            # if improvement wasn't seen in this batch, heighten the requirements of peforming well on the canary cases
            if self.ratio < 1:
                self.ratio += self.ratio_increment
            # set ceiling to be 1
            if self.ratio > 1:
                self.ratio = 1

        return

    def update_params(self, *args,**kargs):
        self.canary_data = kargs.get("canary_data", self.canary_data)
        self.ratio = kargs.get("ratio", self.ratio)
        self.original_ratio = kargs.get("original_ratio", self.original_ratio)
        self.ratio_increment = kargs.get("ratio_increment", self.ratio_increment)
        self.best_fitness = kargs.get("best_fitness", self.best_fitness)
        return True

    def export_params(self):    #all params in a dictinoary for export and import when another canary obj is made
        param_dict = dict(canary_data=self.canary_data, original_ratio=self.original_ratio, 
                            ratio=self.ratio, ratio_increment=self.ratio_increment, best_fitness=self.best_fitness)
        return param_dict


"""
Specialized general functions

"""
def _gen_output_nodule(case_dict):
    if case_dict["dataset"] in ("lidc", "cxr"):
        gene_str = "DEFAULT"
        if case_dict.get("gene", ""):
            gene_str = case_dict.get("gene", "")
        gene_results_dir = os.path.join(case_dict["results_dir"], gene_str)
        seg_file = os.path.join(gene_results_dir, case_dict["id"], "seg_res.yml")
        log_file = os.path.join(gene_results_dir, case_dict["id"], "log", "seg_log.txt")
        seg_dir = os.path.join(gene_results_dir, case_dict["id"], "seg")
        seg_img = os.path.join(seg_dir, "dicom.seri")
        seg_done_file = os.path.join(seg_dir, "file_list.txt")
        eval_dir = os.path.join(gene_results_dir, case_dict["id"], "eval")
        eval_file = os.path.join(gene_results_dir, case_dict["id"], "eval_res.yml")
    elif case_dict["dataset"] == "lcs_pipeline":
        log_dir = os.path.join(case_dict["results_dir"], "log")
        gene_str = ""
        if case_dict.get("gene", ""):
            gene_str = "_"+case_dict.get("gene", "")
        seg_file = os.path.join(log_dir, "seg_res%s.yml"%gene_str)
        seg_dir = os.path.join(case_dict["results_dir"], "qi_dir", "cad%s"%gene_str)
        seg_done_file = os.path.join(seg_dir, "file_list.txt")
        seg_img = os.path.join(seg_dir, "source_image.txt")
        eval_dir = os.path.join(case_dict["results_dir"], "eval", "cad%s"(gene_str) )
        eval_file = os.path.join(log_dir, "eval_res%s.yml"%gene_str)
    else:
        raise("Dataset not accommodated.")
    return {
        "id": case_dict["id"],
        "log_file": log_file,
        "seg": seg_dir,
        "seg_done": seg_done_file,
        "seg_img": seg_img,
        "seg_res": seg_file,
        "eval": eval_dir,
        "eval_file": eval_file,
        "gene_results_dir": gene_results_dir,
    }

class NoduleResultsLoader(BaseResultsLoader):
    def seg_out(self, load_results=True):
        seg_out = None
        if not os.path.exists(self.output.get("seg_res", "")):
            return seg_out
        else:
            if load_results:
                print("Seg already finished...loading output")
                with open(self.output["seg_res"]) as f:
                    ret = yaml.load(f)
                if ret:
                    seg_out = ret
                    if os.path.exists(self.output.get("seg_done", "")):
                        with open(self.output["seg_done"]) as f:
                           contents = f.read()
                        all_files_exist = True
                        for line in contents.split("\n"):
                            if not line.startswith("nodule-"):
                                continue
                            print(line)
                            if not os.path.exists(os.path.join(seg_out, line)) or not os.stat(os.path.join(seg_out, line)).st_size>0:
                                print(os.path.join(seg_out, line), "missing.")
                                all_files_exist = False
                        if not all_files_exist:
                            return None
                # have it make sure that all of the files in the solution file
            else:
                seg_out = True
        return seg_out
    def eval_out(self, load_results=True):
        eval_out = None
        if os.path.exists(self.output.get("eval_file", "")):
            print(self.output["eval_file"])
            if load_results:
                print("Loading eval...", self.output["eval_file"])
                with open(self.output["eval_file"]) as f:
                    ret = yaml.load(f)
                if ret is not None and ret.get("overall") is not None and ret.get("nodule") is not None: 
                    eval_out = ret
            else:
                eval_out = True
        return eval_out


##### Probably Unused #####
"""
Returns whether test_region is encapsulated within image_region

"""
def in_region(test_region, image_region):
    for region in test_region:
        for i,v in enumerate(region):
            if v < image_region[0][i] or v > image_region[1][i]:
                return False
    return True

"""
Not sure if used...
"""
def iter_reference_nodule(path, template=None, file_only=False):
    with open(os.path.join(path, "list.txt")) as f:
        for l in f:
            l = l.strip()
            if l:
                l = os.path.join(path, l)
                id = int(os.path.basename(l).replace(".roi", ""))
                if file_only:
                    yield id, l
                else:
                    if template is not None:
                        yield id, get_casted_roi(l, template=template)
                    else:
                        yield id, qimage.read(l)