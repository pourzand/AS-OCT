"""
Created Nov. 18 2021

"""
import os
import yaml, glob
import pandas as pd
from skimage.measure import regionprops
from sklearn.metrics import confusion_matrix
from skimage.exposure import equalize_adapthist
import itertools
from skimage import img_as_float64                              
from .plotting_cxr import generate_screenshot, generate_markings
from simplemind.apt_agents.optimizer.ga.src.core.evaluate import Evaluator, ResultsCompiler, MIU_Result

SCREENSHOT_SIZE_MM = 50
LEVEL = 128
WINDOW = 256
try:
    from simplemind import __qia__
    import qia.common.img.image as qimage
    import qia.common.img.overlay as qovr
except:
    pass
def get_default_orientation():
    ORIENTATION = {
        # "c": dict(crsstype=qovr.CrossSection.custom, xaxis=(1,0,0), yaxis=(0,0,-1)),
        # "s": dict(crsstype=qovr.CrossSection.custom, xaxis=(0,1,0), yaxis=(0,0,-1)),
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
def generate_screenshot(pos, image, mask, prefix, truth_mask=None, paths_only=False, region=None):
    ret = {}

    if not paths_only and region is None:
        minp = image.to_image_coordinates([i-SCREENSHOT_SIZE_MM/2 for i in pos])
        maxp = image.to_image_coordinates([i+SCREENSHOT_SIZE_MM/2 for i in pos])
        region = [[round(min(i,j)) for i,j in zip(minp, maxp)], [round(max(i,j)) for i,j in zip(minp, maxp)]]
    ret = {}
    ORIENTATION = get_default_orientation()
    for k,o in ORIENTATION.items():
        # print(k,o)
        org_file = "%s_%s_org.png" % (prefix, k)
        ovr_file = "%s_%s.png" % (prefix, k)
        if not paths_only:  # arranges and writes screenshots
            ### This is just a fancy way to handle all the screenshot orientations in one object
            gen = qovr.auto(pos, image=image, region=region, **o)   
            gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)
            gen.write(org_file)
            # gen.add(mask, (255,0,0), 1, boundval=0)
            if mask is not None:
                gen.add(mask, (255,0,0), .5, boundval=0)
            if truth_mask is not None:
                gen.add(truth_mask, (0,255,0), 0.2, boundval=0)
            gen.write(ovr_file)
        ### filling dict with file paths
        ret["%s_org" % k] = org_file
        ret["%s_ovr" % k] = ovr_file
    return ret


class Evaluator_OCT(Evaluator):
    def __init__(self):
        visuals = ["iris_cnn_*.png", "cornea_cnn_*.png",]
        roi_dict = {"iris_cnn": "IRIS", "cornea_cnn": "CORNEA"}
        rois = list(roi_dict.keys())
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return

    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single OCT slice.
        
        Example: /radraid/apps/personal/wasil/USC/oct/segmentation/SSA_003_OD_Scan3_1.pngblue.pngout_0000.nii.gz
        
        Assumes all references are held case-wise in "[reference_path]\[id].png[color].pngout_0000.nii.gz"
        
        """
        self.log.debug("Loading reference...")
        reference_contents = {"IRIS":{}, "CORNEA":{}}
        try:
            from simplemind import __qia__
            import qia.common.img.image as qimage
        except:
            print("Failed to import qimage (evaluate_oct.py)")
        self.log.debug("IRIS")

        ### Load iris (red)
        roi_reference_path = os.path.join(reference_path, "{}.pngred.pngout_0000.nii.gz".format(id))
        self.log.debug(roi_reference_path)
        if os.path.exists(roi_reference_path):

            # roi = qimage.cast(image)
            # roi.fill_with_roi(reference_path)
            roi = qimage.read(roi_reference_path)
            reference_contents["IRIS"]["roi"] = roi.get_ge(1)

        ### Load cornea (blue)
        roi_reference_path = os.path.join(reference_path, "{}.pngblue.pngout_0000.nii.gz".format(id))
        if os.path.exists(roi_reference_path):
            roi = qimage.read(roi_reference_path)

            reference_contents["CORNEA"]["roi"] = roi.get_ge(1)
        self.log.debug("CORNEA")

        return reference_contents

    def _compute_evaluation_measures(self, image, rois, reference, outpath, skip_images=False, extra=None):
        return self._compute_evaluation_measures_segmentation(image, rois, reference, outpath, skip_images=skip_images, extra=extra)

    # def _custom_dce_score(self, reference, rois, ref_key, miu_key):
        # this method is embeded in _compute_evaluation_measures_segmentation()
        # overload this if you want to make own custom DCE score
    def casewise_visualization(self, outpath, image, case_dict, rois, extra=None, miu_key=None, ref_key=None):
        if extra is None: extra = dict()
        image_dir = os.path.join(outpath, "image")
        os.makedirs(image_dir, exist_ok=True)

        region_min, region_max = image.get_region()
        midpoint = (region_max[0] - region_min[0],  region_max[1] - region_min[1], 1)

        original_image_path = os.path.join(image_dir, "original.png")
        # preprocessed_image_path = os.path.join(image_dir, "preprocessed.png")
        annotation_path = os.path.join(image_dir, "annotation.png")
        output_path = os.path.join(image_dir, "output.png")

        ### Prep 
        vis_dict = dict()
        vis_dict["preprocessed_ss"] = extra.get("preprocessed_image", "path/to/fake/image")     # most likely going to be an output from MIU

        ss_dict = generate_screenshot(midpoint, image, rois[miu_key], os.path.join(image_dir, miu_key), truth_mask=None, paths_only=False, region=image.get_region())
        vis_dict["original_ss"] = str(ss_dict["a_org"])
        vis_dict["output_ss"] = str(ss_dict["a_ovr"])
        ### ref_roi
        ss_dict = generate_screenshot(midpoint, image, case_dict[ref_key]["reference_roi"], os.path.join(image_dir, ref_key), truth_mask=None, paths_only=False, region=image.get_region())
        vis_dict["annotation_ss"] = str(ss_dict["a_ovr"])
        # vis_dict["original_ss"] = gen_ss(image, case_dict, rois, original_image_path)
        # vis_dict["annotation_ss"] = gen_annotation_ss(image, case_dict, rois, annotation_path)  # will require reading the reference
        # vis_dict["output_ss"] = gen_output_ss(image, case_dict, rois, output_path)
        return vis_dict
## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    ### replace with your specific Evaluator class
    evaluator  = Evaluator_OCT()    ## TODO: potentially generalizable if evaluator is passed through param
    case_dict = evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=case_id, skip_images=skip_images)
    case_dict["finished"] = True

    return case_dict



### Include in every evaluation task
from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "oct", "default_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "oct", "default_sub_template.tpl")
###################

def fitness_cxr(final_result):
    return (final_result["IRIS"]["dce_score_raw"]+final_result["CORNEA"]["dce_score_raw"])/2

class ResultsCompiler_OCT(ResultsCompiler):
    def __init__(self, fitness_func=fitness_cxr, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

        ### This section for performance stratification, which is optional ###
        self.thresh = {'IRIS': .80,
                'CORNEA': .80,
                }

        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []
        #######################################################################

        self.ref_keys = ["IRIS","CORNEA"] # markings/outputs we're interested in

    ### Example of stratification for cxr trachea
    # any cases with DCE score less than the corresponding threshold for that type of marking (e.g. trachea) is defined as a poor case
    #   and therefore returns as False
    def stratify_performance(self, labelwise_result, ref_key, ):
        if labelwise_result["dce_score"] is not None:
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

    

## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()
    
    ### replace with your specific ResultsCompiler class
    results_compiler  = ResultsCompiler_OCT(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))
    
    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)

