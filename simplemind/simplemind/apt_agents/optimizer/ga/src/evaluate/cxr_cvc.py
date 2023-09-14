"""
Created Nov. 12 2021

Modified Nov. 01 2022 to migrate to the pulic repo.
"""

from simplemind.apt_agents.optimizer.ga.src.core.evaluate import Evaluator, ResultsCompiler, MIU_Result
import os
import yaml, glob
import pandas as pd
import skimage
import itertools
from skimage import img_as_float64                              
from .plotting_cxr import generate_screenshot, generate_markings

def casewise_visualization_cxr(outpath, image, case_dict, rois, extra=None):
    image_dir = os.path.join(outpath, "image")
    os.makedirs(image_dir, exist_ok=True)
    image_arr = image.get_array()[0]
    image_arr = img_as_float64(image_arr)

    image_path = os.path.join(image_dir, "original.png")
    marking_path = os.path.join(image_dir, "markings.png")
    vis_dict = dict()
    ### TODO: If error is in making screenshots, look here.
    vis_dict["marking_ss"] = generate_markings(image_arr, case_dict, rois, marking_path)
    vis_dict["original_ss"] = generate_screenshot(image_arr, case_dict, rois, image_path)
    ### TODO: tell wasil to document that this goes into the report
    # vis_dict["preprocessed_ss_ch1"] = extra.get("visuals", ["path/to/fake/image", "path/to/fake/image", "path/to/fake/image",])[0]
    # vis_dict["preprocessed_ss_ch2"] = extra.get("visuals", ["path/to/fake/image", "path/to/fake/image", "path/to/fake/image",])[1]
    # vis_dict["preprocessed_ss_ch3"] = extra.get("visuals", ["path/to/fake/image", "path/to/fake/image", "path/to/fake/image",])[2]
    vis_dict["preprocessed_ss"] = []
    if extra.get("visuals") is not None:
        for channel in extra.get("visuals"):
            vis_dict["preprocessed_ss"].append(channel)
    return vis_dict

class MIU_Result_CVC(MIU_Result):
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
    result = MIU_Result_CVC(results_path)
    image = result.load_image()
    result.set_rois_to_load(rois)                       # rois should be a list of "xxxxx" with the .roi
    loaded_rois = result.load_rois()
    vis_paths = result.load_miu_visualizations(visuals)
    extra = dict(visuals=vis_paths)
    return image, loaded_rois, extra

class Evaluator_CXR_CVC(Evaluator):
    def __init__(self):
        visuals = ["cvc_cnn_*.png",]
        roi_dict = {"cvc_final": "CVC"}
        rois = list(roi_dict.keys())
        super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return
    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        """Loads the reference data for a single CXR.
        Assumes all references are held case-wise in "[reference_path]\[uid]\info.yaml"
        """

        ### TODO: message wasil about including this in documentation -- can just be the ROI path directly
        # with open(os.path.join(reference_path, id, 'info.yaml')) as f: # Not sure if this is right for cvc 
        #     reference_contents = yaml.load(f)

        # reference_contents["CVC"]["roi"] = None
        # if reference_contents["CVC"].get("roi_path") is not None and image is not None:
        reference_contents = {"CVC":{}}
        if os.path.exists(reference_path):
            try:
                import qia.common.img.image as qimage
            except:
                print("Failed to import qimage (evaluate_cxr_cvc.py)")
            roi = qimage.cast(image)
            roi.fill_with_roi(reference_path)

            """
            ### 08052021 By Matt's direction, Reference ROI will be truncated at the carina, to match Liza's evaluation process 
            for pt in roi.find(1,None):
                # print(pt)
                x,y,z,val = pt
                if y > reference_contents["Crina"]["y"]:    #anything in ROI with y position lower than carina will be automatically set as 0 (background)
                    roi.set_value(pt, 0)
            """

            reference_contents["CVC"]["roi"] = roi

        #### reference_contents["CVC"]["roi"] = #the loaded ref roi -- as a QIA roi object

        return reference_contents

    def read_miu_results(self, result_path, rois=None, visuals=None):
        return read_miu_results(result_path, rois=rois, visuals=visuals)

    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        return casewise_visualization_cxr(outpath, image, case_dict, rois, extra)

    def _compute_evaluation_measures(self, image, rois, reference, outpath, skip_images=False, extra=None):
        return self._compute_evaluation_measures_segmentation(image, rois, reference, outpath, skip_images=skip_images, extra=extra)

    # def _custom_dce_score(self, reference, rois, ref_key, miu_key):
        # this method is embeded in _compute_evaluation_measures_segmentation()
        # overload this if you want to make own custom DCE score
    def _hausdorff_distance(self, image, rois, reference):
        h_dist = None
        return h_dist
    
## Include in every evaluation script
## returns case dictionary of results
def evaluate_task(param, previous_output=None):
    result_path, reference_path, outpath, dataset, case_id, skip_images = param

    ## in case this was done sequentially with sm.runner
    if previous_output is not None and previous_output.get("result_path"):
         result_path = previous_output.get("result_path")
    if skip_images is None: skip_images=False

    ### replace with your specific Evaluator class
    evaluator  = Evaluator_CXR_CVC()    ## TODO: potentially generalizable if evaluator is passed through param
    case_dict = evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=case_id, skip_images=skip_images)
    case_dict["finished"] = True

    return case_dict

#Old version
# def evaluate_task(result_path, reference_path, outpath, dataset=None, id=None, skip_images=False):
#     evaluator  = Evaluator_CXR_CVC()  # this class name will be whatever is specific to yours
#     return evaluator.evaluate_task(result_path, reference_path, outpath, dataset=dataset, id=id, skip_images=skip_images)

### Include in every evaluation task
from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "cvc_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "cxr", "cvc_sub_template.tpl")
###################

def fitness_cxr(final_result):
    return final_result["CVC"]["dce_score_raw"]

class ResultsCompiler_CXR_CVC(ResultsCompiler):
    def __init__(self, fitness_func=fitness_cxr, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

        ### This section for performance stratification, which is optional ###
        self.thresh = {'Crina': 10,
                'GEjct': 20,
                'EtTip': 10,
                'NgTub': 5, #changed from 10mm MWW 07182020
                'TraCh': 0.8,
                'CVC': 0.2,
                }

        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []
        #######################################################################

        self.ref_keys = ["CVC",] # markings/outputs we're interested in

    ### Example of stratification for cxr trachea
    # any cases with DCE score less than the corresponding threshold for that type of marking (e.g. trachea) is defined as a poor case
    #   and therefore returns as False
    def stratify_performance(self, labelwise_result, ref_key, ):
        print(labelwise_result["dce_score"], self.thresh[ref_key])
        if labelwise_result["dce_score"] is not None:
            if labelwise_result["dce_score"] < self.thresh[ref_key]:
                return False
            return True
        return False
    def stratified_performer_metrics(self, ref_key):
        strat_perf_dict = {
                            "num_bad_markings": len(self.bad_cases[ref_key]),
                            "num_good_markings": len(self.good_cases[ref_key]),
                            "threshold": self.thresh[ref_key],
        }
        return strat_perf_dict

    def compile_function_cvc(self, result_dictionary, outpath,
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
                                        ['ref_detection', 'pred_detection', 'dce_score', 'dice_coefficient']],
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

#Old version
# def compile_function(result_dictionary, outpath, rerun=False, canary=False):
#     results_compiler  = ResultsCompiler_CXR_CVC()
#     return results_compiler.compile_function_cvc(result_dictionary, outpath, rerun=rerun, canary=canary)

## Include in every evaluation script
def compile_function(param, previous_output=None):
    cases, outpath, rerun, report_templates, canary = param
    if rerun is None: rerun=False
    if canary is None: canary=False
    if report_templates is None: report_templates = dict()
    
    ### replace with your specific ResultsCompiler class
    results_compiler  = ResultsCompiler_CXR_CVC(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))
    
    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function_cvc(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)