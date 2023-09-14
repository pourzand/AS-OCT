
import os
import glob

import yaml, pandas as pd, itertools, time
from simplemind.apt_agents.optimizer.ga.src.core.report import generate_report
import logging
DEFAULT_SEG_TEMPLATE = "path/to/somewhere"
DEFAULT_SEG_SUB_TEMPLATE = "path/to/somewhere"

"""
Instructions:

(1) [Optional] MIU Result reader function
(2) Casewise evaluation modification to fit your task
    (a) task-specific case-wise visualization 
        - original image
        - preprocessed image
        - annotations
        - output markings from algorithm
    (b) task-specific reference loading
    (c) task-specific computation of evaluation measures
        - if segmentation, then change DCE
(3) Overall results compilation to fit your task
    - set ref_keys
    - fitness function (outside function)
    - performance stratification (bad cases v. good cases) function
        - set thresholds
    - report HTMLs (overall and case-wise)

Configuration?
"""

############### Reading MIU Examples ####################

class MIU_Result():
    def __init__(self, results_path):
        self.log = logging.getLogger("opt.sm_reader")
        self.results_path = results_path
        self.point_roi_dict = {}
        self._load_funcs()
        return 
    def _load_funcs(self,):
        if True:
        # try:
            from simplemind import __qia__
            import qia.common.img.image as qimage
            # /simplemind/dependencies/bin/qia/common/img
            # import simplemind.dependencies.bin.qia.common.img.image as qimage
        # except:
        #     print("Failed to import qimage (evaluate.py)")
        self.read = qimage.read
        self.cast = qimage.cast
    def set_rois_to_load(self, roi_list):
        self.rois = roi_list
        return self.rois
    # def set_point_rois(self, point_roi_dict):
    #     self.point_roi_dict = point_roi_dict
    def load_image(self, ):
        image_path = os.path.join(self.results_path, "dicom.seri")
        if os.path.exists(image_path):
            self.image = self.read(image_path)
        else:
            image_path = os.path.join(self.results_path, "source_image.txt")
            if os.path.exists(image_path):
                with open(image_path, 'r') as f:
                    image_path = f.read()
                self.image = self.read(image_path)
            else:
                raise("Image path doesn't exist within MIU results.")
        return self.image
    def load_rois(self,):
        loaded_rois = {}
        for roi_id in self.rois:
            roi_path = os.path.join(self.results_path, "%s.roi"%roi_id)
            # print(roi_path, os.path.exists(roi_path))
            if os.path.exists(roi_path):
                roi = self.cast(self.image)
                roi.fill_with_roi(roi_path)
            else:
                roi = None

            loaded_rois[roi_id] = roi
            # if roi_id in self.point_roi_dict:
            #     mtype = self.point_roi_dict[roi_id]
            #     point_rois[mtype] = roi
        return loaded_rois

    ### This function is very likely to be overloaded to suit your custom needs ###
    """
    Examples:
        (1) file_list = ["trachea_cnn_*.png",]
        (2) file_list = ["preprocessed_image.png",]
        (3) file_list = ["preprocessed_image.png", "trachea_cnn_*.png"]
        (4) file_list = ["preprocessed_image.png", "trachea_cnn_*.png", "intermediate_output.jpg"]
    
    """
    def load_miu_visualizations(self, file_list=None):
        if file_list is None or not file_list:
            return glob.glob(os.path.join(self.results_path, "*.png"))
        filepaths = []
        for f in file_list:
            if "*" in f:
                self.log.debug("Searching... %s", os.path.join(self.results_path, f))
                try:
                    filepaths.append(glob.glob(os.path.join(self.results_path, f))[0])   # if there are multiple it will only get the first one, in this default version
                except:
                    self.log.warning("Warning: Couldn't find", os.path.join(self.results_path, f))
            else:
                filepaths.append(os.path.join(self.results_path, f))
        return filepaths


    # preprocessed_image_search = os.path.join(results_path, "trachea_cnn_*.png")
    # print(preprocessed_image_search)
    # if preprocessed_image_search:
    #     extra["preprocessed_image"] = glob.glob(preprocessed_image_search)[0]


### Default version ###
# User is expected to overwrite this function using MIU_Result 
# and potentially overload the class methods
"""
Provide: 
- results_path:: output MIU folder
- rois:: list of ROIs to load (as full ROIs) -- do not append ".roi" suffix
- visuals:: list of visualizations to read -- basename filepaths in MIU seg directory. 
            Can have wildcards to be used with glob. Include suffix (e.g. ".png")

Output:
image - QIA image object (just 1, not a list)
loaded_rois - dictionary { [roi_id]: [Loaded ROI object] }
extra - dictionary of anything extra
"""
def read_miu_results(results_path, rois=None, visuals=None):
    result = MIU_Result(results_path)
    image = result.load_image()
    result.set_rois_to_load(rois)                       # rois should be a list of "xxxxx" with the .roi
    loaded_rois = result.load_rois()
    vis_paths = result.load_miu_visualizations(visuals)
    extra = dict(visuals=vis_paths)
    return image, loaded_rois, extra



"""
Example of overloading reader function to include point ROIs

point_rois:: dictionary of rois to be a point
"""
def read_miu_results_point_markings(results_path, rois=None, visuals=None, point_rois=None):
    image, loaded_rois, extra = read_miu_results(results_path, rois=rois, visuals=visuals)
    loaded_point_rois = {}
    if point_rois is not None:
        for roi_id, roi in loaded_rois.items():
            if roi_id in point_rois:
                mtype = point_rois[roi_id]
                loaded_point_rois[mtype] = get_xy(roi)
    extra["point_rois"] = loaded_point_rois
    return image, loaded_rois, extra



"""
CXR Trachea Example:
    - Reads specific png ("trachea_cnn_*.png" output that is the preprocessed image)
    - Maps loaded rois into point_rois dictionary 
"""
def read_miu_results_cxr_trachea(results_path, rois=None, visuals=None, point_rois=None):
    if visuals is None:
        visuals = ["trachea_cnn_*.png",]
    image, loaded_rois, extra = read_miu_results(results_path, rois=rois, visuals=visuals)
    extra["preprocessed_image"] = extra["visuals"][0]
    loaded_point_rois = {}
    if point_rois is not None:
        for roi_id, roi in loaded_rois.items():
            if roi_id in point_rois:
                mtype = point_rois[roi_id]
                loaded_point_rois[mtype] = roi
    extra["point_rois"] = loaded_point_rois
    ### for general CXR specifically ###
        #### Can re-enable later ###
        # get the predicted point locations from the MIU output
        # points = {}
        ### Retrieving centroid
        # for mtype in POINT_MARKINGS:
        #     points[mtype] = get_xy(point_rois[mtype])   

        # points['NgTub'] = point_rois['NgTub']
        ###############################
    return image, loaded_rois, extra

"""
CXR Trachea Example but with MIU Reader overloading:
    - Defines 
    - Reads specific png ("trachea_cnn_*.png" output that is the preprocessed image)
    - Maps loaded rois into point_rois dictionary 

(This example tries to replicate what was done previously for CXR)

"""

### Example Overloaded Class ###
"""
Changes: 
    - Make "load_miu_visualizations" method have less inputs, and just statically load trachea_cnn png
    - Make static point roi dictionary
    - Make static list
"""
CXR_ROIS = ['carina',
    'gejct',
    'et_tip',
    'ng_tube',
    'et_tube',
    'et_tip_correct',
    'ng_tube_correct',
    'et_zone',
    'ng_zone',
    'trachea',
    ]
CXR_DICT = {'carina': 'Crina',
            'gejct': 'GEjct',
            'et_tip': 'EtTip',
            'trachea': 'TraCh',
            'ng_tube': 'NgTub',
            'et_tube': 'EtTub'
        }

class MIU_Result_Trachea(MIU_Result):
    ### Overload __init__ method
    def __init__(self, results_path):
        super().__init__(results_path)
        self.point_rois = CXR_DICT
        return
    ### Overload visualization loading method
    def load_miu_visualizations(self):
        return glob.glob(os.path.join(self.results_path, "trachea_cnn_*.png"))[0]

    ### Make new functions for loading point ROIs
    def load_point_rois(self, loaded_rois):
        loaded_point_rois = {}
        for roi_id, roi in loaded_rois.items():
            if roi_id in self.point_rois:
                mtype = self.point_rois[roi_id]
                loaded_point_rois[mtype] = roi
        return loaded_point_rois

def read_miu_results_cxr_trachea_overload(results_path, rois=CXR_ROIS, visuals=None):
    ## Just completely ignore visuals param (intentionally)
    result = MIU_Result_Trachea(results_path)
    image = result.load_image()
    loaded_rois = result.set_rois_to_load(rois)
    extra = dict()
    extra["preprocessed_image"] = result.load_miu_visualizations()
    extra["point_rois"] = result.load_point_rois(loaded_rois)
    return image, loaded_rois, extra



###########################################################################
###### Evaluation Code Block ######

### Visualization ###
"""
By default:
    - screenshot that shows original image
    - screenshot that shows preprocessed image (probably linked in "extra" rather than made by visualization code)
    - screenshot that shows annotations
    - screenshot that shows algorithm output (sometimes combined with annotations image)


"""



def gen_ss(image, case_dict, rois, original_image_path):
    print("Warning: Not implemented.")
    return original_image_path

def gen_annotation_ss(image, case_dict, rois, annotation_path):
    print("Warning: Not implemented.")
    return annotation_path

def gen_output_ss(image, case_dict, rois, output_path):
    print("Warning: Not implemented.")
    return output_path

def casewise_visualization(outpath, image, case_dict, rois, extra=None, ref_key=None, miu_key=None):
    if extra is None: extra = dict()
    image_dir = os.path.join(outpath, "image")
    os.makedirs(image_dir, exist_ok=True)

    original_image_path = os.path.join(image_dir, "original.png")
    # preprocessed_image_path = os.path.join(image_dir, "preprocessed.png")
    annotation_path = os.path.join(image_dir, "annotation.png")
    output_path = os.path.join(image_dir, "output.png")

    vis_dict = dict()
    vis_dict["original_ss"] = gen_ss(image, case_dict, rois, original_image_path)
    vis_dict["preprocessed_ss"] = extra.get("preprocessed_image", "path/to/fake/image")     # most likely going to be an output from MIU
    vis_dict["annotation_ss"] = gen_annotation_ss(image, case_dict, rois, annotation_path)  # will require reading the reference
    vis_dict["output_ss"] = gen_output_ss(image, case_dict, rois, output_path)
    return vis_dict


def casewise_visualization_cxr(outpath, image, case_dict, rois, extra=None):
    from plotting import generate_markings, generate_screenshot     # you can move these import statements out of the function in your own file
    from skimage import img_as_float64                              #
    image_dir = os.path.join(outpath, "image")
    os.makedirs(image_dir, exist_ok=True)
    image_arr = image.get_array()[0]
    image_arr = img_as_float64(image_arr)

    image_path = os.path.join(image_dir, "original.png")
    marking_path = os.path.join(image_dir, "markings.png")
    vis_dict = dict()
    vis_dict["marking_ss"] = generate_markings(image_arr, case_dict, rois, marking_path)
    vis_dict["original_ss"] = generate_screenshot(image_arr, case_dict, rois, image_path)
    vis_dict["preprocessed_ss"] = ""
    return vis_dict

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
SCREENSHOT_SIZE_MM = 50
LEVEL = -550
WINDOW = 1600

def get_default_orientation():
    import qia.common.img.overlay as qovr
    ORIENTATION = {
        "c": dict(crsstype=qovr.CrossSection.custom, xaxis=(1,0,0), yaxis=(0,0,-1)),
        "s": dict(crsstype=qovr.CrossSection.custom, xaxis=(0,1,0), yaxis=(0,0,-1)),
        "a": dict(crsstype=qovr.CrossSection.axial),
    }
    return ORIENTATION

def generate_screenshot(pos, image, mask, prefix, truth_mask=None, paths_only=False, ss_params=None):
    import qia.common.img.overlay as qovr
    if ss_params is None:
        ss_params = dict()
    SCREENSHOT_SIZE_MM = ss_params.get("SCREENSHOT_SIZE_MM", 50)
    LEVEL = ss_params.get("LEVEL", -550) 
    WINDOW = ss_params.get("WINDOW", 1600) 
        

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


"""

 - visuals ::
 - roi_dict ::

"""
### Case-wise Evaluation
class Evaluator():
    def __init__(self, rois=None, visuals=None, roi_dict=None, ):
        self.log = logging.getLogger("opt.evaluator")
        self.rois = rois
        self.visuals = []
        self.roi_dict = dict()
        if visuals is not None: self.visuals = visuals
        if roi_dict is not None: self.roi_dict = roi_dict
    def load_reference(self, reference_path, image=None, dataset=None, id=None):

        return dict()
        # case-wise reference output

    def read_miu_results(self, result_path, rois=None, visuals=None):
        return read_miu_results(result_path, rois=rois, visuals=visuals)
    #     return image, rois, extra
    # ignore dataset param for now
    ### MAIN FUNCTION ###
    def evaluate_task(self, result_path, reference_path, outpath, id=None, dataset=None, skip_images=False):
        """Evaluates performance for a single case.

        Loads predictions from saved .rois and loads relevant reference. These are
        passed onto an evaluation computation function. Essentially a wrapper to
        load things differently if there is a different dataset/ output/ reference
        format.

        Process:
        (1) Load MIU output
        (2) Load CXR Reference
        (3) Compute evaluation metric

        :param result_path: A directory containing the segmentation results.
        :param reference_path: A directory containing the reference, expects a
                            file named "info.yaml" in it.
        :param outpath: A directory containing the evaluation results.
        """
        # read in relevant ROIs
        image, rois, extra = self.read_miu_results(result_path, rois=self.rois, visuals=self.visuals ) 
        
        # load reference from the case's directory
        reference = self.load_reference(reference_path, image=image, dataset=dataset, id=id)

        return self._compute_evaluation_measures(image, rois, reference, outpath, skip_images=skip_images, extra=extra)
    


    """
        By default dce_score is the same as DCE
        ref_key e.g. "TraCh" (marking abbrev from QIWS database usually)
        miu_key e.g. "trachea" (from MIU output)
    """
    def _custom_dce_score(self, reference, rois, ref_key, miu_key):
        # reference[ref_key]["roi"] is from reference
        # rois[miu_key] is from MIU
        dice_coefficient = None
        dce_score = None
        if reference[ref_key].get("roi") is not None:   # if ref exists
            ref_detection = 1
            if rois.get(miu_key) is None:
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
                pred_detection = 1
                detection_class = "TP"
        else:   # ref is absent
            ref_detection = 0
            if rois.get(miu_key) is None:
                pred_detection = 0
                detection_class = "TN"
            else:
                pred_detection = 1
                detection_class = "FP"
        dce_score = dice_coefficient    
        return dict(dce_score=dce_score, dice_coefficient=dice_coefficient, pred_detection=pred_detection, 
                        detection_class=detection_class, ref_detection=ref_detection, )

    ### overwrite this with your own visualization ###
    def casewise_visualization(self, outpath, image, case_dict, rois, extra, miu_key=None, ref_key=None):
        return casewise_visualization(outpath, image, case_dict, rois, extra, miu_key=miu_key, ref_key=ref_key)   # replace with your own outside function

    ### Segmentation eval with an added detect/no detect metric ###
    def _compute_evaluation_measures_segmentation(self, image, rois, reference, outpath, skip_images=False, extra=None):
        """Computes all evaluation for a single case. Also generates screenshots.
        Returns a dictionary to be used for report generation.

        image: the QIA image
        points: a dictionary where keys are marking types and values are a point
                location
        reference: a dictionary from the reference yaml file
        outpath: folder where output for the individual case will be saved
        """
        self.log.debug("Starting to compute segmentation evaluation measures for %s", outpath)

        """
        In the Case Results table can we add two columns: ET Correct, NG Correct.
        They will both have Y/N/NA outputs. The will be
        - yes if the et_tip_correct/ng_tube_correct is non empty,
        - NA if et_tube/ng_tube is empty,
        - No otherwise

        """
        if not reference:
            raise("Reference is not defined. Referenceless execution not yet implemented.")
        if not os.path.exists(outpath):
            os.makedirs(outpath)

        spacing = image.get_spacing()

        case_dict = {}
        for miu_key, ref_key in self.roi_dict.items():
            #### Computing Dice Coefficient ####
            dce_dict = self._custom_dce_score(reference, rois, ref_key, miu_key)

            case_dict[ref_key] = dict(  dce_score=dce_dict["dce_score"], 
                                        dice_coefficient=dce_dict["dice_coefficient"],
                                        marking=None,
                                        roi=reference[ref_key].get("roi_file"),
                                        ref_detection=dce_dict.get("ref_detection"),
                                        pred_detection=dce_dict.get("pred_detection"),
                                        reference_roi=reference[ref_key].get("roi"),
                                        # class=detection_class,    #for safety, putting it in the next line
                                        )
            case_dict[ref_key]["class"] = dce_dict["detection_class"] 
            ### Screenshot ###
            # For trachea detection+segmentation:
            #   - original image
            #   - outline/shaded trachea & model output over original
            #   - point to the preprocessed jpg in MIU output -- TODO: location of segmentation jpg


            """
                TODO: THIS NEEDS TO BE MOVED OUTSIDE OF THE ROI DICTIONARY
                ::: case_dict has the case-wise results that will be passed on to compile function
                ::: rois has the ROIs output from MIU (loaded)
            """
            if not skip_images:
                case_dict["visuals"] = self.casewise_visualization(outpath, image, case_dict, rois, extra, miu_key=miu_key, ref_key=ref_key)      
                if case_dict[ref_key].get("reference_roi") is not None:     ## deletes the QIA roi object before pickling to the evaluation file 
                    del case_dict[ref_key]["reference_roi"]
            else:
                case_dict["visuals"].update(dict(marking_ss="", original_ss="", preprocessed_ss="", ))


            ### TODO: Fix this ###
            # Either -- one image loaded as "preprocessed_image"
            # or     -- multiple images from self.visuals list
            # -> Assume that if there are specific images custom-loaded through the read_miu_results, then self.visuals
            #       is -not- used.
            for i, vis in enumerate(self.visuals):
                if "*" in vis: continue
                if extra.get("preprocessed_image") is not None and extra.get("preprocessed_image"):
                    case_dict["visuals"][vis] = extra["preprocessed_image"][i]
                else:
                    case_dict["visuals"][vis] = "path/to/fake/image"

        case_dict['spacing'] = spacing
        case_dict['patient_id'] = reference.get('patient_id', "N/A")
        case_dict['initials'] = reference.get('initials', "N/A")

        self.log.debug("Finished %s", outpath)

        return case_dict

    def _compute_roi_measures(self, nodule):
        from numpy import sqrt
        lad_info = nodule.compute_longest_axial_diameter()
        
        ### Computing the perpendicular diameter
        if lad_info.get("perpendicular_diameter") is None:               #041217, Wasil Wahi
            slice_spacing = nodule.get_spacing()
            point_a = lad_info["perpendicular_start"]
            point_b = lad_info["perpendicular_end"]
            lad_info["perpendicular_diameter"] = sqrt(sum([((point_a[i]-point_b[i])*slice_spacing[i])**2 for i, _ in enumerate(point_a)]))
        res = {
            "diameter": lad_info["diameter"],
            "perp_diameter": lad_info["perpendicular_diameter"],
            "z": lad_info["z"],
        }
        return res
    ### Overload this function with your own function or one of the template functions
    def detection_match(self, roi_out, truth_info):
        return self.detection_match_overlap(roi_out, truth_info)

    ### Warning: Untested ###
    def detection_match_point(self, roi_out, truth_info):
        ### Allows focusing on just the nodule region for computational efficiency ###
        res = dict()
        for truth_index, t_info in truth_info.items():
            if roi_out.get_value(t_info["point"]) > 0:
                res["truth_point"] = t_info["point"]
                res["truth_id"] = truth_index
                break
        return res

    def detection_match_overlap(self, roi_out, truth_info):
        from qia.common.img.region import Region
        ### Allows focusing on just the nodule region for computational efficiency ###
        region = Region(roi_out.find_region(1,None))
        res = dict()
        for truth_index, t_info in truth_info.items():
            if region.intersect(t_info["region"]):
                joint_hist = roi_out.get_joint_histogram(t_info["mask"], region=region)
                if joint_hist.get((1,1), 0)>0:
                    aNb = joint_hist[(1,1)]
                    aUb = 0
                    for k,v in joint_hist.items():
                        if k!=(0,0):
                            aUb += v
                    #dice coefficient defined by 2*(XnY)/(X + Y) --> 2*(XnY)/(XnY+XuY)
                    res["truth_mask"] = t_info["mask"]
                    res["dice_coefficient"] = 2*aNb/(aNb+aUb)
                    res["overlap_ratio"] = aNb/aUb
                    res["truth_id"] = truth_index
                    res["truth_diameter"] = t_info["diameter"]
                    res["truth_perp_diameter"] = t_info["perp_diameter"],
                    break
        return res

    ### Example for ROI
    def _process_match(self, res, roi_out, extra=None):
        if extra is None: extra=dict()
        # res = {}
        if res.get("truth_mask") is not None:  # if there is a match with "hard" truth                  
            res["type"] = "TP"
        else:
            res["type"] = "FP"
        return res

    def _unmatched_truth(self, truth_index, template, truth_info, outpath):
        import qia.common.img.overlay as qovr
        t_info = truth_info[truth_index]
        res = {
            "truth_id": truth_index,
            "truth_diameter": t_info["diameter"],
            "truth_perp_diameter": t_info["perp_diameter"],
            "type": "FN",
        }       
        pos = list(template.to_image_coordinates(qovr.get_centroid(t_info["mask"])))
        pos[2] = t_info["z"]
        pos = template.to_physical_coordinates(pos)
        res.update(generate_screenshot(pos, template, t_info["mask"], os.path.join(outpath, "%s_FN" % truth_index)))
        return res
        
    def roi_wise_visualization(self, res, template, outpath, roi_out, skip_images=False, extra=None):
        if extra is None: extra = dict()
        import qia.common.img.overlay as qovr
        pos = list(template.to_image_coordinates(qovr.get_centroid(roi_out)))
        pos[2] = res["z"]
        pos = list(template.to_physical_coordinates(pos))
        res_ss = dict()
        if res.get("type")=="TP":
            res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_TP" % res["id"]), truth_mask=res["truth_mask"]))
        elif res.get("type")=="FP":
            res_ss.update(generate_screenshot(pos, template, roi_out, os.path.join(outpath, "%s_FP" % res["id"]), paths_only=skip_images))
        else:
            self.log.warning("No classification given for this ROI.")
        return res_ss

    ### Default gets overlap of overall detections v. overall truth
    def overall_metrics(self, result_union, truth):
        truth_union = truth["truth_union"]
        if result_union is None and truth_union is None:
            overall_res = {
                "tp": 0,
                "fp": 0,
                "fn": 0,
            }
        elif result_union is None:
            overall_res = {
                "tp": 0,
                "fp": 0,
                "fn": truth_union.get_histogram().get(1, 0),
            }
        elif truth_union is None:
            overall_res = {
                "tp": 0,
                "fp": result_union.get_histogram().get(1, 0),
                "fn": 0,
            }
        else:
            temp = result_union.get_joint_histogram(truth_union)
            overall_res = {
                "tp": temp.get((1,1), 0),
                "fp": temp.get((1,0), 0),
                "fn": temp.get((0,1), 0)
            }

        return overall_res 
    def _iter_detections(self, result_path, template):
        return NotImplementedError
    """

        :param fp_ref: Soft positive reference from LIDC.


        More info: 
        - pTP is a pseudo True Positive, which doesn't improve the Sensitivity metric but doesn't hurt the Specificity metric


        return {"nodule":result, "overall":overall_res}

        Process:
        - Iterate through each nodule to see if match exists (match requires minimum overlap ratio of 0.25)
        - Measure each detected nodule, as well as each reference nodule
        - Compute overall overlap metrics

        Output: 
        - results dictionary, with two elements -- nodule-wise results in list via "nodule", and overall results via "overall"
        

        TODO: 
        - Improvement to say the reason for false positive (if default or if because of not enough overlap) or pseudo TP -- keep in nodule list dict
    """

    # def _compute_evaluation_measures_detection(self, template, truth_info, truth_union, result_path, outpath, fp_ref=None, skip_images=False):
    def _compute_evaluation_measures_detection(self, template, truth, result_path, outpath, skip_images=False, extra=None):

        self.log.debug("Starting computing detection evaluation metrics for %s", outpath)
        
        if not os.path.exists(outpath):
            os.makedirs(outpath)
            
        detected_truths = set()
        result = []
        result_union = None
        
        ### algorithm performance metric variables ###
        overall_truth_time = 0
        overall_ss_time = 0
        overall_union_time = 0
        time_n = 0
        starttime_2 = time.time()
        starttime = time.time()

        ### [SECTION]
        ### For each detected roi
        #   (1) compute roi metrics (diameter, volume, etc.)
        #   (2) find truth match if exists
        #   (3) finalize detection classification 
        #   (4) make screenshots for report
        #   (result_union continuously updates to add the latest roi)
        for index, roi_out, result_union in self._iter_detections(result_path, template):
            # print("*",end='')
            # print("Loading nodule:",time.time()-starttime)
            starttime = time.time()
            res = {"id": index,}

            ### (1)
            res.update(self._compute_roi_measures(roi_out))

            ### (2)
            ### Examining overlap with nodule truth (if any exist)
            ### Should return the dice coefficient, overlap ratio, and information about the truth nodule (id, diameter, perp diameter)
            ### :truth_mask: will not be None if there is a match
            res.update(self.detection_match(roi_out, truth["truth_info"]))


            time_elapsed = time.time() - starttime
            overall_truth_time+=time_elapsed
            time_n+=1
            
            ### (3)
            ### Fine-tuning match and processing screenshots ###        
            ### generates the type of match ("TP", "FP", or any special categorization (like "pTP" in LIDC) )
            starttime = time.time()
            res.update(self._process_match(res, roi_out, extra=extra))
            
            ### (4)
            ### Case-wise visualization -- make sure you overload with your task-specific visualization
            res.update(self.roi_wise_visualization(res, template, outpath, roi_out, skip_images=skip_images, extra=extra))
            if res["type"]=="TP":
                detected_truths.add(res["truth_id"])
            if res.get("mask") is not None: del res["mask"] # for computing purposes, deletes QIA mask
            if res.get("truth_mask") is not None: del res["truth_mask"] # for computing purposes, deletes QIA mask
            
            result.append(res)
            
            time_elapsed = time.time() - starttime
            # print("SS nodule:",time_elapsed)
            overall_ss_time+=time_elapsed
            starttime = time.time()


        self.log.debug("")
        self.log.debug("Overall Truthing Time: %s",str(overall_truth_time))
        if time_n > 0:
            self.log.debug("Average Truthing Time per Nodule: %s",str(overall_truth_time/time_n))
        self.log.debug("Overall Truthing Screenshot Time: %s",str(overall_ss_time))
        self.log.debug("Overall Detection Iteration: %s",str(time.time() - starttime_2))

        ### Tracking remaining FNs (aka positives not detected) and calculating metrics ###
        self.log.debug("Going through remaining FN nodules") 
        overall_fp_time = 0
        time_n = 0
        starttime = time.time()
        for truth_index in set(truth["truth_info"].keys())-detected_truths:
            res = self._unmatched_truth(truth_index, template, truth["truth_info"], outpath)
            result.append(res)

            time_elapsed = time.time() - starttime
            overall_fp_time+=time_elapsed
            time_n+=1
        self.log.debug("\nOverall FN Time: %s",str(overall_fp_time))
        if time_n > 0:
            self.log.debug("Average FN Time per Nodule: %s",str(overall_fp_time/time_n))

        overall_res = dict()
        overall_res["visualization"] = self.casewise_visualization(result_union, truth)
        starttime = time.time()
        if result_union is not None:
            result_union.ge(1)
        overall_union_time = time.time() - starttime
        self.log.debug("Overall Union Time: %s", str(overall_union_time))

        self.log.debug("Overall eval metrics")
        starttime = time.time()
        ### Computing overall overlap metric ###
        overall_res.update(self.overall_metrics(result_union, truth))
        time_elapsed = time.time() - starttime
        self.log.debug("")
        self.log.debug("Overall Eval Metrics Time: %s",str(time_elapsed))
        

        overall_res["spacing"] = list(template.get_spacing())   # recording slice spacing while we can
        self.log.debug("Finished _compute_evaluation_measures for %s", outpath)
        return {"nodule":result, "overall":overall_res}


############# EXAMPLE EVALUATOR FOR CXR #####################

class Evaluator_CXR_Trachea(Evaluator):
    def __init__(self):
        visuals = ["trachea_cnn_*.png",]
        roi_dict = {"trachea": "TraCh"}
        rois = list(roi_dict.values())
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return
    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single CXR.
        Assumes all references are held case-wise in "[reference_path]\[uid]\info.yaml"
        """
        with open(os.path.join(reference_path, 'info.yaml')) as f:
            reference_contents = yaml.load(f)

        reference_contents["TraCh"]["roi"] = None
        if reference_contents["TraCh"].get("roi_path") is not None and image is not None:
            try:
                from simplemind import __qia__
                import qia.common.img.image as qimage
                # import simplemind.dependencies.bin.qia.common.img.image as qimage
            except:
                self.log.error("Failed to import qimage")
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
def evaluate_task(result_path, reference_path, outpath, dataset=None, id=None, skip_images=False):
    evaluator  = Evaluator_CXR_Trachea()
    return evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=id, skip_images=skip_images)

############# EXAMPLE EVALUATOR FOR NODULE CG DETECTION #####################
### TODO: Import the following
# from ga.ct_nodule.reference import load as load_reference_lidc
# from ga.ct_nodule import get_template_from_outpath, iter_outpath_nodule2

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
        import qia.common.img.overlay as qovr
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
            self.log.warning("No classification given for this ROI.")
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
            self.log.error("Warning: Results missing for %s", result_path)
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

## Include in every evaluation script
def evaluate_task(result_path, reference_path, outpath, dataset="lidc", id=None, skip_images=False):
    evaluator  = Evaluator_Nodule_CG()
    return evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=id, skip_images=skip_images)

#################### COMPILING ALL RESULTS ACROSS POPULATION ###############################

class ResultsCompiler():
    def __init__(self, report_generator=generate_report, fitness_func=None, overall_report_template=None, casewise_report_template=None ):        
        
        self.report_generator = report_generator
        self.overall_report_template=overall_report_template
        self.casewise_report_template=casewise_report_template
        self.fitness_func = lambda t: 1.0
        if fitness_func is not None: self.fitness_func = fitness_func
        self.bad_cases = dict()
        self.good_cases = dict()
        self.ref_keys = []  # e.g. self.ref_keys = ["TraCh",]

    def load(self, cases, results_dir=None):
        case_results_dir = results_dir
        result_dictionary = {}
        for case in cases:
            if results_dir is None:
                case_results_dir = case.get("results_dir")
            eval_results = os.path.join(case_results_dir, case["id"], "eval_res.yml")
            with open(eval_results, 'r') as f:
                contents = yaml.load(f, Loader=yaml.FullLoader)
            result_dictionary[case["id"]]= contents
        return result_dictionary
        
    ### For stratifying good performers and bad performers  ###
    # By default, will do nothing
    def stratify_performance(self,*args, **kargs):
        return True
    def stratified_performer_metrics(self, *args, **kargs):
        return dict()

    def _add_performance_metrics(self, marking_df, metric):
        perf_dict = {
                    "%s_raw"%metric: float(marking_df[metric].mean()),   #no rounding
                    "%s_mean"%metric: round(float(marking_df[metric].mean()), 3),
                    "%s_std"%metric: round(float(marking_df[metric].std()), 3),
                    "%s_median"%metric: round(float(marking_df[metric].median()), 3),
                    "%s_q1"%metric: round(float(marking_df[metric].quantile(0.25)), 3),
                    "%s_q3"%metric: round(float(marking_df[metric].quantile(0.75)), 3),
        }
        return perf_dict
    def _compute_confusion_matrix(self, marking_df):
        from sklearn.metrics import confusion_matrix
        # print("ref")
        # print(marking_df['ref_detection'].astype(int))
        # print("pred")
        # print(marking_df['pred_detection'].astype(int))
        tn, fp, fn, tp = confusion_matrix(marking_df['ref_detection'].astype(int),
                                        marking_df['pred_detection'].astype(int),
                                        labels=[0, 1]).ravel()
        confusion_dict = {
                    "tn": int(tn),
                    "fp": int(fp),
                    "fn": int(fn),
                    "tp": int(tp),
                    "sensitivity": float(tp / (tp + fn)),
                    "specificity": float(tn / (tn + fp)),
        }
        return confusion_dict

    def overall_visualization(self, df):
        overall_vis = dict()
        # sp = os.path.join(outpath, 'error_hist.png')
        # final_result['error_hist_plot'] = gen_hist_plot(df, sp)

        # sp = os.path.join(outpath, 'error_scatter.png')
        # final_result['error_scatter_plot'] = gen_scatter_plot(df, sp)
        return overall_vis

    ### MAIN FUNCTION ###
    def compile_function(self, result_dictionary, outpath,
                        rerun=False,
                        canary=False):
        """Compiles all case-wise results to compute overall metrics and generates
        HTML reports.

        Iterates through cases in result_dictionary where keys are uids. Computes
        any overall metrics. Overall metrics are stored into  the dictionary
        “final_result”. Generates reports using the results_dictionary and
        final_results.
        Returns fitness for GA evolution purposes.

        For CXR specifically:
            Looks to see if Carina, GE Junction, ET Tip, and NG Tube were aligned within thresholds, to establish TP/FP/TN/FN.

            Currently:
            thresh = {'Crina': 10,
                    'GEjct': 20,
                    'EtTip': 10,
                    'NgTub': 5, #changed from 10mm MWW 07182020
                    }
        """
        # this part is included in every compile function
        outfile = os.path.join(outpath, "final.yml")
        if canary:
            outfile = os.path.join(outpath, "canary.yml")
        if os.path.exists(outfile) and not rerun:
            with open(outfile) as f:
                res = yaml.load(f)
            return (res["fitness"],)


        ### [STEP] Preprocessing, plus saving overall results to .csv for readability/review ###
        
        sp_final = os.path.join(outpath, 'final.csv')
        # detection_sp = os.path.join(outpath, 'detection.csv')
        cols = pd.MultiIndex.from_product([self.ref_keys,
                                        ['ref_detection', 'pred_detection', 'dce_score', 'dice_coefficient']],
                                        names=['mtype', 'value'])
        df = pd.DataFrame(columns=cols)
        

        try:  # <--- purely for debugging
            per_case_result = {}
            remaining_cases = []
            # print(result_dictionary)
            # print("How many in dictionary", len(list(result_dictionary.keys())))
            for k, case_dict in result_dictionary.items():
                for mtype, labels in case_dict.items():
                    ## For now just including whole ROIs, later on can include POINT_MARKINGS
                    # print(">>>>", mtype, self.ref_keys)
                    if mtype in self.ref_keys:
                        # print(":::", labels)
                        if self.stratify_performance(labels, mtype):
                            self.good_cases[mtype].append(k)
                        else:
                            self.bad_cases[mtype].append(k)
                        df.loc[k, (mtype, 'ref_detection')] = labels.get('ref_detection')
                        df.loc[k, (mtype, 'pred_detection')] = labels.get('pred_detection')
                        df.loc[k, (mtype, 'dce_score')] = labels.get('dce_score')
                        df.loc[k, (mtype, 'dice_coefficient')] = labels.get('dice_coefficient')
                if k not in list(itertools.chain(*self.bad_cases.values())):
                    remaining_cases.append(k)
                per_case_result[k] = case_dict
            df.to_csv(sp_final)

        except:
            # print(k, case_dict)
            # input("It failed. Hit enter to see results_dictionary.")
            # print(result_dictionary)
            raise("Failed to load.")
        
        #################################################################################
        ### [STEP] Overall Report ###

        ### Overall Report ###
        # Metrics to include:
        # OVERALL:
        # - sensitivity
        # - specificity
        # - average, mean, stdev, 1st/3rd quartiles dice-coefficient
        # - average, mean, stdev, 1st/3rd quartiles DCE Score
        # - N good cases
        # - N "bad" cases
        # CASE-WISE:
        # - ref present/absent
        # - MIU output present/absent
        # - TP/FP/FN/TN
        # - DCE Score
        # - DCE

        # In report, partition by "bad" cases on top
        # Good cases: DCE Score >= 0.80

        remaining_cases = list(dict.fromkeys(remaining_cases))

        ### final_result goes into the overall report
        final_result = dict()
        for mtype in self.ref_keys:
            # print(mtype)
            marking_df = df[mtype]
            # print(marking_df)
            final_result[mtype] = {"input_case_num": len(marking_df),
                                    "num_annotated": int(marking_df['ref_detection'].sum()),
            }
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dce_score"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dice_coefficient"))
            final_result[mtype].update(self._compute_confusion_matrix(marking_df))
            final_result[mtype].update(self.stratified_performer_metrics(mtype))

        final_result.update(self.overall_visualization(df))

        final_result['per_case_result'] = per_case_result
        final_result['bad_cases'] = self.bad_cases
        final_result['good_cases'] = self.good_cases
        final_result['remaining_cases'] = remaining_cases

        final_result['fitness'] = self.fitness_func(final_result)

        with open(outfile, "w") as f:
            f.write(yaml.dump(final_result))

        if self.report_generator is not None:
            case_wise_contents = {}
            for k in per_case_result.keys():
                case_wise_contents[k] = {"key": k,
                                        "per_case_result": final_result["per_case_result"][k]
                                        }
            self.report_generator(result_dictionary, final_result, outpath, template=self.overall_report_template, case_wise_contents=case_wise_contents, sub_template=self.casewise_report_template)
        return (final_result.get("fitness"),)




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
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "default_trachea_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "default_trachea_sub_template.tpl")
###################
class ResultsCompiler_CXR_Trachea(ResultsCompiler):
    def __init__(self, fitness_func=fitness_cxr, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

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
    def stratify_performance_cxr_trachea(self, labelwise_result, ref_key, ):
        if labelwise_result["dce_score"] < self.thresh[ref_key]:
            return False
        return True
    def stratified_performer_metrics_cxr(self, ref_key):
        strat_perf_dict = {
                            "num_bad_markings": len(self.bad_cases[ref_key]),
                            "num_good_markings": len(self.good_cases[ref_key]),
                            "threshold": self.thresh[ref_key],
        }
        return strat_perf_dict



"""
Useful for potential applications??
"""

#### FOR "POINT ROIs"
def get_xy(roi):
    from skimage.measure import regionprops
    if roi is not None:
        roi_arr = roi.get_array()[0]
        # props = skimage.measure.regionprops(roi_arr)
        props = regionprops(roi_arr)
        y, x = props[0]['Centroid']
        return [float(x), float(y)]
    else:
        return None

def compute_point_error(pred_xy, ref_xy, spacing):
    from numpy.linalg import norm
    if pred_xy is None or ref_xy is None:
        return dict(x_err=None, y_err=None, tot_err=None)
    eval_dict = {'x_err': (ref_xy[0] - pred_xy[0]) * spacing[0],
                 'y_err': (ref_xy[1] - pred_xy[1]) * spacing[1],
                 }

    eval_dict['tot_err'] = norm([eval_dict['x_err'],
                                           eval_dict['y_err']])

    return {k: round(float(v), 3) for k, v in eval_dict.items()}



def point_detection(pred_xy, ref_xy):
    if ref_xy is not None:
        ref_exists = 1
    else:
        ref_exists = 0
    if pred_xy is not None:
        pred_exists = 1
    else:
        pred_exists = 0

    if ref_exists: 
        if pred_exists:
            detection_class = 'tp'
        else:
            detection_class = 'fn'
    else:
        if pred_exists:
            detection_class = 'fp'
        else:
            detection_class = 'tn'

    return {'ref_detection': ref_exists,
            'pred_detection': pred_exists,
            'class': detection_class,
            }



