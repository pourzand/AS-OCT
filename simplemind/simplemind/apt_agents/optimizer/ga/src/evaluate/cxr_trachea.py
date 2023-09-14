
# coding: utf-8

# Preface:
#
# The primary difficulty here is that the initial implementation was done in
# a batch-mode approach (every step is done for each case before moving on to
# the next step), whereas the GA implementation is done in case-wise approach
# (cases are done independently, and results are pooled at the end).

"""
07182020

Revised fitness function
I would like to try a different fitness function = percentage of good NG tube cases + percentage of good carina cases

A good carina case = TN or TP with tip error <= 10 mm as computed for the html evaluation page

A good NG tube case = TN or TP with error <= 5 mm as computed for the html evaluation page
    (currently her threshold is 10 mm for a good case, but I would like to reduce it to 5mm).

fitness = n_good_ng/total_ng + n_good_carina/total_carina


"""

import os
import yaml
from skimage.measure import regionprops
from sklearn.metrics import confusion_matrix
from skimage.exposure import equalize_adapthist
from skimage import img_as_float64                              #

from .plotting_cxr import generate_screenshot, generate_display, \
                          generate_markings, gen_hist_plot, gen_scatter_plot



from simplemind.apt_agents.optimizer.ga.src.core.evaluate import Evaluator, ResultsCompiler


def casewise_visualization_cxr(outpath, image, case_dict, rois, extra=None):
    # from plotting import generate_markings, generate_screenshot     # you can move these import statements out of the function in your own file
    # from skimage import img_as_float64                              #
    image_dir = os.path.join(outpath, "image")
    os.makedirs(image_dir, exist_ok=True)
    image_arr = image.get_array()[0]
    image_arr = img_as_float64(image_arr)

    image_path = os.path.join(image_dir, "original.png")
    marking_path = os.path.join(image_dir, "markings.png")
    vis_dict = dict()
    vis_dict["marking_ss"] = generate_markings(image_arr, case_dict, rois, marking_path)
    vis_dict["original_ss"] = generate_screenshot(image_arr, case_dict, rois, image_path)
    preprocessed_ss = extra.get("visuals", ["path/to/fake/image",])
    vis_dict["preprocessed_ss"] = "path/to/fake/image" if not preprocessed_ss else preprocessed_ss[0]
    # if preprocessed_ss: 
    # vis_dict["preprocessed_ss"] = extra.get("visuals", ["path/to/fake/image",])[0]
    return vis_dict

class Evaluator_CXR_Trachea(Evaluator):
    def __init__(self):
        visuals = ["trachea_cnn_*.png",]
        roi_dict = {"trachea": "TraCh"}
        rois = list(roi_dict.keys())
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return

    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single CXR.
        Assumes all references are held case-wise in "[reference_path]\[uid]\info.yaml"
        """
        print(reference_path)
        with open(os.path.join(reference_path, 'info.yaml')) as f:
            reference_contents = yaml.load(f)

        reference_contents["TraCh"]["roi"] = None
        if reference_contents["TraCh"].get("roi_path") is not None and image is not None:
            try:
                import qia.common.img.image as qimage
            except:
                print("Failed to import qimage (evaluate_cxr.py)")
            roi = qimage.cast(image)
            roi.fill_with_roi(reference_contents["TraCh"]["roi_path"])

            ### 08052021 By Matt's direction, Reference ROI will be truncated at the carina, to match Liza's evaluation process 
            for pt in roi.find(1,None):
                # print(pt)
                x,y,z,val = pt
                if y > reference_contents["Crina"]["y"]:    #anything in ROI with y position lower than carina will be automatically set as 0 (background)
                    roi.set_value(pt, 0)
                
            reference_contents["TraCh"]["roi"] = roi
        return reference_contents


    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        return casewise_visualization_cxr(outpath, image, case_dict, rois, extra)
    """
        ref_key e.g. "TraCh" (marking abbrev from QIWS database usually)
        miu_key e.g. "trachea" (from MIU output)
    """
    def _custom_dce_score(self, reference, rois, ref_key, miu_key):
        # reference[ref_key]["roi"] is from reference
        # rois[miu_key] is from MIU
        print("ROIs",rois)
        dice_coefficient = None
        dce_score = None
        if reference[ref_key].get("roi") is not None:   # if ref exists
            ref_detection = 1
            if rois.get(miu_key) is None:
                dce_score = 0.75
                pred_detection = 0
                detection_class = "FN"
            else:
                joint_hist = rois[miu_key].get_joint_histogram(reference[ref_key]["roi"])
                dice_coefficient = 0
                if joint_hist.get((1,1), 0)>0:
                    aNb = joint_hist[(1,1)]
                    aUb = 0
                    for k,v in joint_hist.items():
                        if k!=(0,0):
                            aUb += v
                    #dice coefficient defined by 2*(XnY)/(X + Y) --> 2*(XnY)/(XnY+XuY)
                    dice_coefficient = 2*aNb/(aNb+aUb)
                dce_score = min(dice_coefficient, 0.95)
                pred_detection = 1
                detection_class = "TP"
        else:   # ref is absent
            ref_detection = 0
            if rois.get(miu_key) is None:
                dce_score = 1
                pred_detection = 0
                detection_class = "TN"
            else:
                dce_score = 0 
                pred_detection = 1
                detection_class = "FP"
        return dict(dce_score=dce_score, dice_coefficient=dice_coefficient, pred_detection=pred_detection, 
                        detection_class=detection_class, ref_detection=ref_detection, )
    def _compute_evaluation_measures(self, image, rois, reference, outpath, skip_images=False, extra=None):
        return self._compute_evaluation_measures_segmentation(image, rois, reference, outpath, skip_images=skip_images, extra=extra)

## Include in every evaluation script
# def evaluate_task(result_path, reference_path, outpath, dataset=None, id=None, skip_images=False):
#     evaluator  = Evaluator_CXR_Trachea()
#     return evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=id, skip_images=skip_images)

## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    evaluator  = Evaluator_CXR_Trachea()    ## TODO: potentially generalizable if evaluator is passed through param
    case_dict = evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=case_id, skip_images=skip_images)
    case_dict["finished"] = True

    return case_dict



##### Results Compiler Example for CXR ######
"""
For each task, results compiler needs:
    Fitness function

    Which marking you're comparing your MIU output to (e.g. "TraCh")

    [Optional] Overall population & results visualization
    
    [Optional] Custom performance metric (e.g. "dce_score")

    [Optional] Definition of performance stratification

"""
def fitness_cxr(final_result):
    return final_result["TraCh"]["dce_score_raw"]

from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "trachea_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "trachea_sub_template.tpl")

###################

class ResultsCompiler_CXR_Trachea(ResultsCompiler):
    def __init__(self, fitness_func=fitness_cxr, overall_report_template=None, casewise_report_template=None ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)
        
        ### Change 1
        if overall_report_template is None:
            overall_report_template = DEFAULT_TEMPLATE
        if casewise_report_template is None:
            casewise_report_template = DEFAULT_SUB_TEMPLATE 

        self.thresh = {'Crina': 10,
                'GEjct': 20,
                'EtTip': 10,
                'NgTub': 5, #changed from 10mm MWW 07182020
                'TraCh': 0.8,
                }

        self.bad_cases = dict()
        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []

        self.ref_keys = ["TraCh",] # markings/outputs we're interested in
    
    ### Example of stratification for cxr trachea
    def stratify_performance(self, labelwise_result, ref_key, ):
        print(labelwise_result["dce_score"], self.thresh[ref_key])
        if labelwise_result["dce_score"] < self.thresh[ref_key]:
            return False
        return True
    def stratified_performer_metrics(self, ref_key):
        strat_perf_dict = {
                            "num_bad_markings": len(self.bad_cases[ref_key]),
                            "num_good_markings": len(self.good_cases[ref_key]),
                            "threshold": self.thresh[ref_key],
        }
        return strat_perf_dict

# ## Include in every evaluation script
# def compile_function(result_dictionary, outpath, rerun=False, canary=False):
#     results_compiler  = ResultsCompiler_CXR_Trachea()
#     return results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()

    results_compiler  = ResultsCompiler_CXR_Trachea(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))

    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)
