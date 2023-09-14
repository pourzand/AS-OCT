
from simplemind.apt_agents.optimizer.ga.src.core.evaluate import Evaluator, MIU_Result, read_miu_results, get_xy, compute_point_error, point_detection, ResultsCompiler

import os
import yaml, glob
import numpy as np
import pandas as pd
import skimage
from skimage.measure import regionprops
from sklearn.metrics import confusion_matrix
from skimage.exposure import equalize_adapthist
from skimage import img_as_float64                              #

from .plotting_cxr import generate_screenshot, generate_display, \
                          generate_markings, gen_hist_plot, gen_scatter_plot
import yaml, pandas as pd, itertools, time



class MIU_Result_fixed(MIU_Result):
    def load_miu_visualizations(self, file_list=None):
        if file_list is None:
            return glob.glob(os.path.join(self.results_path, "*.png"))
        filepaths = []
        for f in file_list:
            if "*" in f:
                print("Searching...", os.path.join(self.results_path, f))
                try:
                    filepaths.extend(glob.glob(os.path.join(self.results_path, f)))   # if there are multiple it will only get the first one, in this default version
                    # consider sorting by name (?) or something else
                except:
                    print("Warning: Couldn't find", os.path.join(self.results_path, f))
            else:
                filepaths.append(os.path.join(self.results_path, f))
        return filepaths

def read_miu_results(results_path, rois=None, visuals=None):
    result = MIU_Result_fixed(results_path)
    image = result.load_image()
    result.set_rois_to_load(rois)                       # rois should be a list of "xxxxx" with the .roi
    loaded_rois = result.load_rois()
    vis_paths = result.load_miu_visualizations(visuals)
    extra = dict(visuals=vis_paths)
    return image, loaded_rois, extra



# ### miu_reader function
# """
# Example of overloading reader function to include point ROIs

# point_rois:: list of rois to be a point
# """
# def read_miu_results_point_markings(results_path, rois=None, visuals=None, point_rois=None):
#     image, loaded_rois, extra = read_miu_results(results_path, rois=rois, visuals=visuals)
#     loaded_point_rois = {}
#     if point_rois is not None:
#         for roi_id, roi in loaded_rois.items():
#             if roi_id in point_rois:
#                 mtype = point_rois[roi_id]
#                 loaded_point_rois[mtype] = get_xy(roi)  # extra["point_rois"]["Crina"] <--- still from MIU output, *not* from reference 
#     extra["point_rois"] = loaded_point_rois
#     return image, loaded_rois, extra

def casewise_visualization_cxr(outpath, image, case_dict, rois, extra=None):
    # from plotting import generate_markings, generate_screenshot     # you can move these import statements out of the function in your own file
    # from skimage import img_as_float64                              #
    image_dir = os.path.join(outpath, "image")
    os.makedirs(image_dir, exist_ok=True)
    image_arr = image.get_array()[0]
    image_arr = img_as_float64(image_arr) # TODO: consider downsizing

    image_path = os.path.join(image_dir, "original.png")
    marking_path = os.path.join(image_dir, "markings.png")
    vis_dict = dict()
    vis_dict["marking_ss"] = generate_markings(image_arr, case_dict, rois, marking_path, extra=extra)
    vis_dict["original_ss"] = generate_screenshot(image_arr, case_dict, rois, image_path)
    vis_dict["preprocessed_ss"] = []

    # TODO: explicitly delete image_arr
    # del image_arr

    if extra.get("visuals") is not None:
        for channel in extra.get("visuals"):
            vis_dict["preprocessed_ss"].append(channel)
    return vis_dict


class Evaluator_CXR_ETtip(Evaluator):
    def __init__(self):
        visuals = ["ettip_cnn_*.png",]
        roi_dict = {"ettip_cnn": "EtTip"}  # roi_miu: roi_ref
        rois = list(roi_dict.keys())
        self.point_rois = ["ettip_cnn",]
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return

    def read_miu_results(self, result_path, rois=None, visuals=None):
        return read_miu_results(result_path, rois=rois, visuals=visuals)
        
    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single CXR.
        Assumes all references are held case-wise in "[reference_path]\[uid]\info.yaml"
        """
        with open(os.path.join(reference_path, 'info.yaml')) as f:
            reference_contents = yaml.load(f)

        return reference_contents

    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        return casewise_visualization_cxr(outpath, image, case_dict, rois, extra)

    ### Point localization eval ###
    # rois is a dictionary
    def _compute_evaluation_measures_point(self, image, rois, reference, outpath, skip_images=False, extra=None):
        """Computes all evaluation for a single case. Also generates screenshots.
        Returns a dictionary to be used for report generation.

        image: the QIA image
        points: a dictionary where keys are marking types and values are a point
                location
        reference: a dictionary from the reference yaml file
        outpath: folder where output for the individual case will be saved
        """
        print("Starting _compute_evaluation_measures for", outpath)

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

        # iterate through point rois
            # get_xy
            # compute eval metric (distance & detection)
            # store in case_dict
        for miu_key in self.point_rois:
            ref_key = self.roi_dict[miu_key]
            pred_xy = get_xy(rois[miu_key])
            ref_xy = None
            if reference[ref_key].get('x') is not None:
                ref_xy = [reference[ref_key]['x'], reference[ref_key]['y']]

            case_dict[ref_key] = dict(  
                                        marking=None,
                                        ref_xy=ref_xy,
                                        pred_xy=pred_xy,
                                        # class=detection_class,    #for safety, putting it in the next line
                                        )

            error_dict = compute_point_error(pred_xy, ref_xy, spacing)

            detection_dict = point_detection(pred_xy, ref_xy)
            case_dict[ref_key].update(error_dict)
            case_dict[ref_key].update(detection_dict)
    

        #### for later later later ###
        # iterate through non-point rois
            # compute eval metric (DCE)
            # store in case_dict



            if not skip_images:
                case_dict["visuals"] = self.casewise_visualization(outpath, image, case_dict, rois, extra)      
                if case_dict[ref_key].get("reference_roi") is not None:     ## deletes the QIA roi object before pickling to the evaluation file 
                    del case_dict[ref_key]["reference_roi"]
            else:
                case_dict["visuals"].update(dict(marking_ss="", original_ss="", preprocessed_ss=(), ))


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
        return self._compute_evaluation_measures_point(image, rois, reference, outpath, skip_images=skip_images, extra=extra)


## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    ### replace with your specific Evaluator class
    evaluator  = Evaluator_CXR_ETtip()    ## TODO: potentially generalizable if evaluator is passed through param
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
### TODO: Update this fitness function
def fitness_cxr(final_result):
    from math import isnan
    if isnan(final_result["EtTip"]["tot_err_raw"]):
        return -1000000
    elif final_result["EtTip"]["tot_err_raw"] is None:
        return -2000000
    else:
        return -1 * (final_result["EtTip"]["tot_err_raw"] + 10*(final_result["EtTip"]["fn"]))
    
from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "ettip_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "ettip_sub_template.tpl")

###################

class ResultsCompiler_CXR_ETtip(ResultsCompiler):
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

        self.ref_keys = ["EtTip",] # markings/outputs we're interested in
    
    ### Example of stratification for cxr trachea
    def stratify_performance(self, labelwise_result, ref_key, ):
        print(labelwise_result["tot_err"], self.thresh[ref_key])
        if labelwise_result["tot_err"] is None:
            return False
        if labelwise_result["tot_err"] > self.thresh[ref_key]:
            return False
        return True

    def stratified_performer_metrics(self, ref_key):
        strat_perf_dict = {
                            "num_bad_markings": len(self.bad_cases[ref_key]),
                            "num_good_markings": len(self.good_cases[ref_key]),
                            "threshold": self.thresh[ref_key],
        }
        return strat_perf_dict

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
                                        ['ref_detection', 'pred_detection', 'x_err', 'y_err', 'tot_err']],
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
                        df.loc[k, (mtype, 'ref_detection')] = labels.get('ref_detection')
                        df.loc[k, (mtype, 'pred_detection')] = labels.get('pred_detection')
                        df.loc[k, (mtype, 'x_err')] = labels.get('x_err')
                        df.loc[k, (mtype, 'y_err')] = labels.get('y_err')
                        df.loc[k, (mtype, 'tot_err')] = labels.get('tot_err')
                        # np.abs() calculates the absolute value element-wise
                        # df.loc[k, (mtype, 'abs_x_err')] = np.abs(labels.get('x_err'))
                        # df.loc[k, (mtype, 'abs_y_err')] = np.abs(labels.get('y_err'))
                        # df.loc[k, (mtype, 'abs_tot_err')] = np.abs(labels.get('tot_err'))
                if k not in list(itertools.chain(*self.bad_cases.values())):
                    remaining_cases.append(k)
                per_case_result[k] = case_dict
            df.to_csv(sp_final)

        except:
            print(k, case_dict)
            input("It failed. Hit enter to see results_dictionary.")
            print(result_dictionary)
            raise("Fail")
        
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
            print(mtype)
            marking_df = df[mtype]
            print(marking_df)
            final_result[mtype] = {"input_case_num": len(marking_df),
                                    "num_annotated": int(marking_df['ref_detection'].sum()),
            }
            final_result[mtype].update(self._add_performance_metrics(marking_df, "x_err"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "y_err"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "tot_err"))
            # final_result[mtype].update(self._add_performance_metrics(marking_df, "abs_x_err"))
            # final_result[mtype].update(self._add_performance_metrics(marking_df, "abs_y_err"))
            # final_result[mtype].update(self._add_performance_metrics(marking_df, "abs_tot_err"))
            ### TODO: Anything point specific here
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

## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()
    
    ### replace with your specific ResultsCompiler class
    results_compiler  = ResultsCompiler_CXR_ETtip(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))
    
    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)



