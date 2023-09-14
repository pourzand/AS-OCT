

import os
import yaml, glob
import numpy as np
import pandas as pd
import skimage
import itertools
#import qia.common.img.image as qimage
from simplemind.apt_agents.optimizer.ga.src.core.evaluate import MIU_Result, Evaluator, ResultsCompiler

def apply_generic_CT_normalization(array, MINVAL, MAXVAL):
    """Apply generic CT normalization to a numpy array.
    Scales the array from -1000HU and 1000HU to 0 and 1 (values outside the limit are clipped).
    """
    RANGE = MAXVAL-MINVAL
     
    norm_arr = ((array-MINVAL)/RANGE).clip(0,1)
     
    return norm_arr
def view_img(qia_image,roi_arr, path, color):

    import matplotlib.pyplot as plt
    import numpy.ma as ma

    image_arr = np.array(qia_image.get_array())
    image_arr_norm = apply_generic_CT_normalization(image_arr, -20, 300)

    try:
        plt.figure(0, figsize=(5,2*len(image_arr)))

        for z in range(len(image_arr)):
            
            plt.subplot(len(image_arr_norm), 1 , z + 1)
            plt.imshow(image_arr_norm[z, :, :], cmap = 'gray')
            plt.imshow(ma.masked_where(roi_arr[z, :,:] == 0, roi_arr[z]), cmap = color, alpha=0.7)
            plt.axis('off')
            plt.title('Z= ' + str(z + 1) + '/' + str(len(image_arr_norm)))

        plt.savefig(path)
        plt.close()
        
    except:
        plt.close()
        plt.figure(0, figsize=(10,10))
        plt.imshow(np.zeros((20,20)), cmap = 'gray')
        plt.title('Image too large to show')
        plt.savefig(path)
        plt.close()
    return path



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
class MIU_Result_Renal(MIU_Result):
    def load_image(self, ):
    #    image = super().load_image()
    #    self.image = np.array(image.get_array())
    #    return self.image

        self.image = super().load_image()
        #image = np.array(self.image.get_array())

        #return [self.image, image]

        return self.image

    def load_rois(self, ):
        rois = super().load_rois()
        if rois.get('kidney_final_right') is not None and rois.get('kidney_final_left') is not None:

            rkidn = np.array(rois.get('kidney_final_right').get_array()) 
            lkidn = np.array(rois.get('kidney_final_left').get_array()) 

            seg = rkidn + lkidn

        elif rois.get('kidney_final_right') is not None and rois.get('kidney_final_left') is None:

            rkidn = np.array(rois.get('kidney_final_right').get_array()) 
            lkidn = np.zeros(rkidn.shape)    
            
            seg = rkidn + lkidn
        elif rois.get('kidney_final_right') is None and rois.get('kidney_final_left') is not None:

            lkidn = np.array(rois.get('kidney_final_left').get_array()) 
            rkidn = np.zeros(lkidn.shape) 
            seg = rkidn + lkidn

        elif rois.get('kidney_final_right') is None and rois.get('kidney_final_left') is None:
            image = np.array(self.image.get_array())
            seg = np.zeros(image.shape)
        
        rois['combined_kidneys'] = seg

        return rois


def read_miu_results(results_path, rois=None, visuals=None):
    print("in MIU results reader")
    print("ROIs to load:", rois)
    result = MIU_Result_Renal(results_path)
    image = result.load_image()
    result.set_rois_to_load(rois)                       # rois should be a list of "xxxxx" with the .roi
    loaded_rois = result.load_rois()
    vis_paths = result.load_miu_visualizations(visuals)
    extra = dict(visuals=vis_paths)
    print("ROIs:", loaded_rois)
    print("Extra:", extra)
    return image, loaded_rois, extra










##################################################


class Evaluator_Renal(Evaluator):
    def __init__(self):
        # visuals =  ['kidney_cnn_*.png', 'kidney_right_cnn_*.png', 'kidney_left_cnn_*.png' ]
        # roi_dict = {"trachea": "TraCh"}
        visuals = None
        roi_dict = {'combined_kidneys': 'combined_kidneys'}
        rois = ['kidney_final_right', 'kidney_final_left']
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return
    
    def read_miu_results(self, result_path, rois=None, visuals=None):
        return read_miu_results(result_path, rois=rois, visuals=visuals)

    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        import qia.common.img.image as qimage

        roi_path = reference_path

        if os.path.splitext(roi_path)[1] == '.roi':
            ref_roi = qimage.cast(image)
            ref_roi.fill_with_roi(roi_path) # This one will load the combined kidney ROI, which is why I don't have "fill_with_roi" twice

        else:
            ref_roi = qimage.read(roi_path)

        ref_roi_np = np.array(ref_roi.get_array())


        '''TODO if I want to load just one of the ROIs, I can convert half of ref_roi_np into 0 depending on the kidney. Example:     RKid_arr = np.zeros((num_slice, img_height, img_width))
         LKid_arr = np.zeros((num_slice, img_height, img_width))

        for k in range(num_slice):
        RKid_arr[k,:,0:256] = pred_scan[k,:,0:256]
        LKid_arr[k,:,256:512] = pred_scan[k,:,256:512]

        '''
        reference_contents = {}
        reference_contents['combined_kidneys'] = {'roi_path':roi_path, 'roi':ref_roi_np}
        
        return reference_contents

    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        
        vis_dict =  {}
        image_dir = os.path.join(outpath, "image")
        os.makedirs(image_dir, exist_ok=True)
        # image_arr = image.get_array()[0]
        # image_arr = skimage.img_as_float64(image_arr)
        
        image_path = os.path.join(image_dir, "original.png")
        marking_path = os.path.join(image_dir, "markings.png")
        vis_dict["marking_ss"] = view_img(image, rois['combined_kidneys'], marking_path, 'autumn')    #generate_markings(image_arr, vis_dict, rois, marking_path)
        vis_dict["original_ss"] = view_img(image, case_dict['combined_kidneys']['reference_roi'], image_path, 'ocean')#generate_screenshot(image_arr, vis_dict, rois, image_path)
        vis_dict["preprocessed_ss"] = extra.get("preprocessed_image", "path/to/fake/image")
        return vis_dict

    def _custom_dce_score(self, reference, rois, ref_key, miu_key):

        gt = reference['combined_kidneys']['roi']
        seg = rois['combined_kidneys']
        
        dice = np.sum(seg[gt==1])*2.0 / (np.sum(seg) + np.sum(gt))

        return dict(dce_score=dice, dice_coefficient=dice, )

    def _compute_evaluation_measures_segmentation(self, image, rois, reference, outpath, skip_images=False, extra=None):
        """Computes all evaluation for a single case. Also generates screenshots.
        Returns a dictionary to be used for report generation.

        image: the QIA image
        points: a dictionary where keys are marking types and values are a point
                location
        reference: a dictionary from the reference yaml file
        outpath: folder where output for the individual case will be saved
        """
        print("Starting _compute_evaluation_measures for", outpath)

        if not reference:
            raise("Reference is not defined. Referenceless execution not yet implemented.")
        if not os.path.exists(outpath):
            os.makedirs(outpath)

        spacing = image.get_spacing()

        case_dict = {}
        print("ROI Dict:")
        print(self.roi_dict)
        for miu_key, ref_key in self.roi_dict.items():
            #### Computing Dice Coefficient ####
            dce_dict = self._custom_dce_score(reference, rois, ref_key, miu_key)

            case_dict[ref_key] = dict(  dce_score=float(dce_dict["dce_score"]), 
                                        dice_coefficient=float(dce_dict["dice_coefficient"]),
                                        marking=None,
                                        roi=reference[ref_key].get("roi_file"),
                                        reference_roi=reference[ref_key].get("roi"),
                                        )


            """
                ::: case_dict has the case-wise results that will be passed on to compile function
                ::: rois has the ROIs output from MIU (loaded)
            """
            case_dict["visuals"] = dict()
            if not skip_images:
                case_dict["visuals"] = self.casewise_visualization(outpath, image, case_dict, rois, extra)      
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

        return case_dict

    def _compute_evaluation_measures(self, image, rois, reference, outpath, skip_images=False, extra=None):
        return self._compute_evaluation_measures_segmentation(image, rois, reference, outpath, skip_images=skip_images, extra=extra)


## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    ### replace with your specific Evaluator class
    evaluator  = Evaluator_Renal()    ## TODO: potentially generalizable if evaluator is passed through param
    case_dict = evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=case_id, skip_images=skip_images)
    case_dict["finished"] = True

    return case_dict

"""
For each task, results compiler needs:
    Fitness function

    Which marking you're comparing your MIU output to (e.g. "TraCh")

    [Optional] Overall population & results visualization
    
    [Optional] Custom performance metric (e.g. "dce_score")

    [Optional] Definition of performance stratification

"""
def fitness(final_result):
    return final_result["combined_kidneys"]["dce_score_raw"]
    
from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "qia", "segmentation", "renal", "renal_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "qia", "segmentation", "renal", "renal_sub_template.tpl")

###################

class ResultsCompiler_Renal(ResultsCompiler):
    def __init__(self, fitness_func=fitness, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

        self.thresh = {
                'combined_kidneys': 0.85,
                }

        self.bad_cases = dict()
        self.good_cases = dict()
        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []

        self.ref_keys = ["combined_kidneys",] # markings/outputs we're interested in
    
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
    
    def _compute_confusion_matrix(self, marking_df):

        print('Not being used')
        return dict()
        
    def compile_function_renal(self, result_dictionary, outpath,
                        rerun=False,
                        canary=False):

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
                                        ['dce_score', 'dice_coefficient']],
                                        names=['mtype', 'value'])
        df = pd.DataFrame(columns=cols)
        

        try:  # <--- purely for debugging
            per_case_result = {}
            remaining_cases = []
            print(result_dictionary)
            print("How many in dictionary", len(list(result_dictionary.keys())))
            for k, case_dict in result_dictionary.items():
                for mtype, labels in case_dict.items():
                    ## For now just including whole ROIs, later on can include POINT_MARKINGS
                    print(">>>>", mtype, self.ref_keys)
                    if mtype in self.ref_keys:
                        print(":::", labels)
                        if self.stratify_performance(labels, mtype):
                            self.good_cases[mtype].append(k)
                        else:
                            self.bad_cases[mtype].append(k)
                        df.loc[k, (mtype, 'dce_score')] = labels.get('dce_score')
                        df.loc[k, (mtype, 'dice_coefficient')] = labels.get('dice_coefficient')
                if k not in list(itertools.chain(*self.bad_cases.values())):
                    remaining_cases.append(k)
                per_case_result[k] = case_dict
            df.to_csv(sp_final)

        except:
            print(k, case_dict)
            input("It failed. Hit enter to see results_dictionary.")
            print(result_dictionary)
            raise("Fail")

        remaining_cases = list(dict.fromkeys(remaining_cases))

        ### final_result goes into the overall report
        final_result = dict()
        for mtype in self.ref_keys:
            print(mtype)
            marking_df = df[mtype]
            print(marking_df)
            final_result[mtype] = {"input_case_num": len(marking_df),
                                    # "num_annotated": int(marking_df['ref_detection'].sum()),
            }
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dce_score"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dice_coefficient"))
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

## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()
    
    ### replace with your specific ResultsCompiler class
    results_compiler  = ResultsCompiler_Renal(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))
    
    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)






## Defines output parameters
# General version, that can be overriden 
# TODO: Questions
# - is seg_img necessary for GA -- only used in _compute_evaluation in ga_2_0.py
# def _gen_output_renal(case_dict):
#     chrom_str = "DEFAULT"
#     if case_dict.get("gene", ""):
#         chrom_str = case_dict.get("gene", "")
#     gene_results_dir = os.path.join(case_dict["results_dir"], chrom_str)
#     seg_file = os.path.join(gene_results_dir, case_dict["id"], "seg_res.yml")
#     log_file = os.path.join(gene_results_dir, case_dict["id"], "log", "seg_log.txt")
#     seg_dir = os.path.join(gene_results_dir, case_dict["id"], "seg")
#     seg_img = os.path.join(seg_dir, "dicom.seri")
#     seg_done_file = os.path.join(seg_dir, "file_list.txt")
#     seg_error_logs = [os.path.join(seg_dir, "error_log_*.log"), ]     # MWW 09202021
#     eval_dir = os.path.join(gene_results_dir, case_dict["id"], "eval")
#     eval_file = os.path.join(gene_results_dir, case_dict["id"], "eval_res.yml")
#     return {    "id": case_dict["id"],
#                 "log_file": log_file,
#                 "seg": seg_dir,
#                 "seg_done": seg_done_file,
#                 "seg_error_logs": seg_error_logs,
#                 "seg_img": seg_img,
#                 "seg_res": seg_file,
#                 "eval": eval_dir,
#                 "eval_file": eval_file,
#                 "gene_results_dir": gene_results_dir,
#             }
