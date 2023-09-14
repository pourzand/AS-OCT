from simplemind.apt_agents.optimizer.ga.src.core.evaluate import Evaluator, ResultsCompiler
import os, yaml

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



SCREENSHOT_SIZE_MM = 50
LEVEL = -550
WINDOW = 1600

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
        print(k,o)
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




class Evaluator_CXR_Nodule_CG_2D(Evaluator):
    def __init__(self):
        visuals = ["nodule_cnn_*.png",]
        roi_dict = {"nodule_cnn": "slice_nodule_ref"}
        rois = list(roi_dict.keys())
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return

    """ TODO:
        "[reference_path]\[uid]\info.yaml":
            slice_nodule_ref:
                roi_path: path/to/slice hr2


    """
    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single CT slice.
        Assumes all references are held case-wise in "[reference_path]\[uid]\info.yaml"
        """

        with open(os.path.join(reference_path, 'info.yaml')) as f:
            reference_contents = yaml.load(f)

        reference_contents["slice_nodule_ref"]["roi"] = None
        print("slice_nodule_ref", reference_contents["slice_nodule_ref"])
        if reference_contents["slice_nodule_ref"].get("roi_path") is not None and image is not None:
            try:
                import qia.common.img.image as qimage
            except:
                print("Failed to import qimage (evaluate_nodule_cg.py)")

            ### because it's .hr2, don't need to do this
            # roi = qimage.cast(image)
            # roi.fill_with_roi(reference_contents["slice_nodule_ref"]["roi_path"])
            print(">>>>>>>>>>>>>>>>>>>>>>>>>>> GOOOD >>>>>>")
            roi = qimage.read(reference_contents["slice_nodule_ref"]["roi_path"])
            print(">>>>>>>>>>>>>>>>>>>>>>>>>>> GOOOD 2 >>>>>>")

            reference_contents["slice_nodule_ref"]["roi"] = roi
        return reference_contents
        
    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        image_dir = os.path.join(outpath, "image")
        os.makedirs(image_dir, exist_ok=True)
        # image_arr = image.get_array()[0]
        # image_arr = img_as_float64(image_arr)

        image_path = os.path.join(image_dir, "original.png")
        marking_path = os.path.join(image_dir, "markings.png")
        vis_dict = dict()

        ### estimate central position to be:
        phys_region = image.get_physical_region() # ( (x0,y0,z0), (x1, y1, z1) ) 
        pos = ( (phys_region[0][0]+phys_region[1][0])/2, (phys_region[0][1]+phys_region[1][1])/2, (phys_region[0][2]+phys_region[1][2])/2)
        image_region = image.get_region()
        # if rois["nodule_cnn"] is not None:
        #     print(rois["nodule_cnn"])
        #     output_roi = qimage.cast(image)
        #     output_roi.fill_with_roi(rois["nodule_cnn"])
        # else:
        #     output_roi = None
        # print(case_dict["slice_nodule_ref"])
        # print(case_dict["slice_nodule_ref"]["roi"].find(1, None))
        ss_dict = generate_screenshot(pos, image, rois["nodule_cnn"], os.path.join(outpath, "nodule_slice" ), truth_mask=case_dict["slice_nodule_ref"]["reference_roi"], region=image_region)
        ### we want "a_org" "a_ovr"
        vis_dict["marking_ss"] = ss_dict["a_ovr"]
        vis_dict["original_ss"] = ss_dict["a_org"]
        
        ### You can also pull anything that is preprocessed in MIU and read in through loading ``visuals`` in __init__
        ### They will be stored in ``extra``
        vis_dict["preprocessed_ss"] = extra.get("visuals", ["path/to/fake/image",])
        if vis_dict["preprocessed_ss"]:
            vis_dict["preprocessed_ss"] = vis_dict["preprocessed_ss"][0]
        else:
            vis_dict["preprocessed_ss"]  = "path/to/fake/image"
        return vis_dict

    def _compute_evaluation_measures(self, image, rois, reference, outpath, skip_images=False, extra=None):
        return self._compute_evaluation_measures_segmentation(image, rois, reference, outpath, skip_images=skip_images, extra=extra)

    def _custom_dce_score(self, reference, rois, ref_key, miu_key):
        # reference[ref_key]["roi"] is from reference
        # rois[miu_key] is from MIU
        print("ROIs",rois)
        dice_coefficient = None
        dce_score = None
        if reference[ref_key].get("roi") is not None:   # if ref exists
            ref_detection = 1
            if rois.get(miu_key) is None:
                dce_score = 0
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
                # dce_score = min(dice_coefficient, 0.95)
                dce_score = dice_coefficient
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

## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    evaluator  = Evaluator_CXR_Nodule_CG_2D()    ## TODO: potentially generalizable if evaluator is passed through param
    case_dict = evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=case_id, skip_images=skip_images)
    case_dict["finished"] = True

    return case_dict

def fitness_nodule_cg_2D(final_result):
    return final_result["slice_nodule_ref"]["dce_score_raw"]

### Include in every evaluation task
from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH

### TODO: Create template file for visiualization/report
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "ct_nodule", "cg_2D_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "ct_nodule", "cg_2D_sub_template.tpl")
###################
class ResultsCompiler_Nodule_CG_2D(ResultsCompiler):
    def __init__(self, fitness_func=fitness_nodule_cg_2D, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

        ### This section for performance stratification, which is optional ###
        self.thresh = {'slice_nodule_ref': 0.80,
                }

        self.bad_cases = dict()
        self.good_cases = dict()
        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []

        self.ref_keys = ["slice_nodule_ref",] # markings/outputs we're interested in
    
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



## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()

    results_compiler  = ResultsCompiler_Nodule_CG_2D(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))

    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)
