

import os, sys
from re import L, S
import yaml, glob
import numpy as np
import pandas as pd
import yaml, pandas as pd, itertools, time

from medpy import metric as medmetric

from simplemind.apt_agents.optimizer.ga.src.core.evaluate import MIU_Result, Evaluator, ResultsCompiler

class MIUProsatateResult(MIU_Result):
    def __init__(self, results_path):
        super(MIUProsatateResult,self).__init__(results_path)
        self.time_to_wait_roi = 20
        self.time_counter = 0
    
    def load_miu_visualizations(self, file_list=None):
        if file_list is None:
            return glob.glob(os.path.join(self.results_path, "*.png"))
        filepaths = []
        for f in file_list:
            if "*" in f:
                print("Searching...", os.path.join(self.results_path, f))
                try:
                    filepaths.extend(glob.glob(os.path.join(self.results_path, f)))
                except:
                    print("Warning: Couldn't find", os.path.join(self.results_path, f))
            else:
                filepaths.append(os.path.join(self.results_path, f))
        return filepaths
    
    def load_rois(self,):
        loaded_rois = {}
        for roi_id in self.rois:
            roi_path = os.path.join(self.results_path, "%s.roi"%roi_id)
            time_counter = 0
            print(f'    Load ROI path: {roi_path}')
            while not os.path.exists(roi_path):
                time.sleep(1)
                time_counter += 1
                if time_counter > self.time_to_wait_roi:
                    break
                print(f"        Sleep on {roi_path} for {time_counter} sec, exist={os.path.exists(roi_path)}")      

            print(f'         ROI path: {roi_path}, exist={os.path.exists(roi_path)}')
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

def read_miu_results(results_path, rois=None, visuals=None):
    print("-----------------------------------------------------------------------------")
    print(f'Load MIU results: {results_path}')
    print("-----------------------------------------------------------------------------")
    result = MIUProsatateResult(results_path)
    image = result.load_image()
    print(f'    ROIs to load: {rois}')
    result.set_rois_to_load(rois)                       # rois should be a list of "xxxxx" with the .roi
    loaded_rois = result.load_rois()
    print(f'    Extra visualizations to load: {visuals}')
    vis_paths = result.load_miu_visualizations(visuals)
    vis_paths.sort()
    extra = dict(visuals=vis_paths)
    print(f'    Path of extra vis: ')
    print('        ', '\n        '.join(vis_paths))
    print("-----------------------------------------------------------------------------")
    sys.stdout.flush()
    return image, loaded_rois, extra

def _gen_output(case_dict):
    chrom_str = "DEFAULT"
    if case_dict.get("gene", ""):
        chrom_str = case_dict.get("gene", "")
    gene_results_dir = os.path.join(case_dict["results_dir"], chrom_str)
    seg_file = os.path.join(gene_results_dir, case_dict["id"], "seg_res.yml")
    log_file = os.path.join(gene_results_dir, case_dict["id"], "log", "seg_log.txt")
    seg_dir = os.path.join(gene_results_dir, case_dict["id"], "seg")
    seg_img = os.path.join(seg_dir, "source_image.txt")
    seg_done_file = os.path.join(seg_dir, "file_list.txt")
    seg_error_logs = [os.path.join(seg_dir, "error_log_*.log"), ]     # MWW 09202021
    eval_dir = os.path.join(gene_results_dir, case_dict["id"], "eval")
    eval_file = os.path.join(gene_results_dir, case_dict["id"], "eval_res.yml")
    return {    "id": case_dict["id"],
                "log_file": log_file,
                "seg": seg_dir,
                "seg_done": seg_done_file,
                "seg_error_logs": seg_error_logs,
                "seg_img": seg_img,
                "seg_res": seg_file,
                "eval": eval_dir,
                "eval_file": eval_file,
                "gene_results_dir": gene_results_dir,
            }

"""Evaluator for prostate segmentation
"""
class EvaluatorProstate(Evaluator):
    def __init__(self):
        # roi_dict = {"trachea": "TraCh"}
        visuals = ['prostate_*.png',]
        roi_dict = {'prostate_final': 'Prost', 
                    'prostate_apex_attention':'Prost_apex', 
                    'prostate_base_attention':'Prost_base', 
                    'prostate_center':'Prost_center',
                    # 'prostate_base':'Prost_base_area',
                    # 'prostate_apex':'Prost_apex_area'
                    }
        # rois = list(roi_dict.keys())
        rois = ['prostate_final','prostate_apex_attention','prostate_base_attention', 'prostate_center',
                'prostate_center_box', 'prostate_base_box','prostate_apex_box']
        
        # self.rois = [f'{x}.roi' for x in rois]
        self.rois = rois
        self.visuals = []
        # print(roi_dict)
        self.roi_dict = dict()
        if visuals is not None: self.visuals = visuals
        if roi_dict is not None: self.roi_dict = roi_dict
        print("-----------------------------------------------------------------------------")
        print('Start Evaluator...')
        print("-----------------------------------------------------------------------------")
        print(f"ROI dict: {self.roi_dict}")
        print(f"visuals: {self.visuals}")
        print("-----------------------------------------------------------------------------")
        # super().__init__(rois=rois, visuals=visuals, roi_dict=roi_dict)
        return
    
    def read_miu_results(self, result_path, rois=None, visuals=None):
        return read_miu_results(result_path, rois=rois, visuals=visuals)

    def load_reference(self, reference_path, image=None, dataset=None, id=None):
        print("-----------------------------------------------------------------------------")
        print(f'Load reference: {reference_path}')
        print(f'    dataset: {dataset}')
        print(f'    id: {id}')
        print("-----------------------------------------------------------------------------")
        ref_roi_path = reference_path
        try:
            from simplemind import __qia__
            import qia.common.img.image as qimage
        except:
            print("Failed to import qimage (evaluate_prostate.py)")
        try:
            if 'roi' in ref_roi_path.split('.')[-1]:
                ref_roi = qimage.cast(image)
                ref_roi.fill_with_roi(ref_roi_path)
            else:
                ref_roi = qimage.read(ref_roi_path)
            # ref_roi_np = ref_roi.get_array()
        except:
            ref_roi = None
            # ref_roi_np = None
        reference_contents = {'dataset':dataset, 'patient_id':id, 'initials':reference_path.split('/')[-2]}
        reference_contents['Prost'] = {'roi_path':ref_roi_path, 'roi':ref_roi}
        reference_contents['Prost_apex'] = {'roi_path':ref_roi_path, 'roi':ref_roi}
        reference_contents['Prost_base'] = {'roi_path':ref_roi_path, 'roi':ref_roi}
        reference_contents['Prost_center'] = {'roi_path':ref_roi_path, 'roi':ref_roi}
        
        return reference_contents

    def casewise_visualization(self, outpath, image, case_dict, rois, extra):
        print("-----------------------------------------------------------------------------")
        print(f'Visualizations...')
        print("-----------------------------------------------------------------------------")
        image_dir = os.path.join(outpath, "image")
        os.makedirs(image_dir, exist_ok=True)
        image_path = os.path.join(image_dir, "original.png")
        marking_path = os.path.join(image_dir, "markings.png")
        preprocessed_path = os.path.join(image_dir, "preprocessed.png")
        
        vis_dict = dict()
        vis_dict["marking_ss"], figsize_marking = self._generate_images(image, rois['prostate_final'], marking_path, cmap='autumn')
        vis_dict["original_ss"], figsize_original = self._generate_images(image, case_dict['Prost']['reference_roi'], image_path, cmap='summer')
        vis_dict["preprocessed_ss"] = self._generate_combined_pngs(extra['visuals'], preprocessed_path, figsize_original)
        # vis_dict["preprocessed_ss"] = extra.get("preprocessed_image", "path/to/fake/image")
        # vis_dict.update(extra)
        return vis_dict
        
    def _generate_combined_pngs(self, images_list, path, figsize):
        import matplotlib.pyplot as plt
        import imageio
        # titles = [x.split('/')[-1] for x in images_list]
        # images = [imageio.imread(im_path) for im_path in images_list]
        # n_images = len(images)
        # fig = plt.figure()
        # for n, (image, title) in enumerate(zip(images, titles)):
        #     a = fig.add_subplot(np.ceil(n_images/float(cols)), cols, n + 1)
        #     if image.ndim == 2:
        #         plt.gray()
        #     plt.imshow(image)
        #     a.set_title(title)
        #     a.set_axis_off()
        # size_x, size_y = fig.get_size_inches()
        # fig.set_size_inches(size_x, size_y)
        images_list.sort()
        titles = [x.split('/')[-1] for x in images_list]
        images = [imageio.imread(im_path) for im_path in images_list]
        n_images = len(images)
        cols = n_images
        fig, axes = plt.subplots(nrows=1, ncols=cols, figsize=(figsize[0]*cols, figsize[1]))
        for n, (image, title) in enumerate(zip(images, titles)):
            if image.ndim == 2:
                axes[n].gray()
            axes[n].imshow(image)
            title='_cnn\n'.join(title.split('_cnn')).strip('_input_image.png')
            axes[n].set_title(title, fontsize=8)
            axes[n].set_axis_off()
        plt.tight_layout()
        plt.savefig(path)
        plt.close()
        return path

    def _generate_images(self, image, roi, path, cmap='autumn', c=8):
        import matplotlib.pyplot as plt
        import numpy.ma as ma
        import logging
        logging.getLogger("matplotlib.pyplot").setLevel(logging.ERROR) # matplotlib logs too much.
        logging.getLogger("numpy.ma").setLevel(logging.ERROR) # matplotlib logs too much.

        image_arr = image.get_array()
        roi_arr = roi.get_array()
        figsize = (c,c*image_arr.shape[0])
        for z in range(len(image_arr)):
            plt.figure(0, figsize=figsize)
            plt.subplot(len(image_arr), 1 , z + 1)
            plt.imshow(image_arr[z, :, :], cmap = 'gray')
            plt.imshow(ma.masked_where(roi_arr[z, :,:] == 0, roi_arr[z]), cmap = cmap, alpha=0.7)
            plt.axis('off')
            plt.title('Slice ' + str(z + 1) + '/' + str(len(image_arr)))
        plt.savefig(path)
        plt.close()
        return path, figsize


    def _custom_score(self, reference, rois, ref_key, miu_key, spacing=None):
        """
        for miu_key, ref_key in self.roi_dict:
            self._custon_score(...)

        self.roi_dict = {miu_key:ref_key}
            ref_key e.g. "Prost" (marking abbrev from QIWS database usually)
            miu_key e.g. "prostate_final" (from MIU output)
        """
        print("-----------------------------------------------------------------------------")
        print(f'Compute scores...')
        print("-----------------------------------------------------------------------------")
        # reference[ref_key]["roi"] is from reference
        # rois[miu_key] is from MIU
        print("ROIs", rois)
        print("reference", reference)
        print("ref_key", ref_key)
        print("miu_key", miu_key)
        print("-----------------------------------------------------------------------------")
        # roi_dict = {'prostate_final': 'Prost', 
        #             'prostate_apex_attention':'Prost_apex', 
        #             'prostate_base_attention':'Prost_base', 
        #             'prostate_center':'Prost_center',
        #             }

        dice_coefficient = None
        dce_score = None
        assd = None
        hd = None
        hd95 = None
        precision = None
        recall = None
        sensitivity = None
        specificity = None

        if reference[ref_key].get("roi") is not None:   # if ref exists
            ref_detection = 1
            if 'center' in ref_key:
                # TODO: when rois['prostate_center_box'] not exist
                if rois['prostate_center_box'] is not None:
                    gt = reference[ref_key]['roi'] * rois['prostate_center_box']
                else: 
                    gt = None
                    print(f'No prostate_center_box roi')
            elif 'base' in ref_key:
                if rois['prostate_base_box'] is not None:
                    gt = reference[ref_key]['roi'] * rois['prostate_base_box']
                else: 
                    gt = None
                    print(f'No prostate_base_box roi')
            elif 'apex' in ref_key:
                if rois['prostate_apex_box'] is not None:
                    gt = reference[ref_key]['roi'] * rois['prostate_apex_box']
                else: 
                    gt = None
                    print(f'No prostate_apex_box roi')
            else:
                gt = reference[ref_key]['roi']
        else: gt = None

        if gt is not None:
            if rois.get(miu_key) is None:
                detection_class = "FN"
                pred_detection = 0
                dice_coefficient = 0
            else:
                detection_class = "TP"
                prediction = rois[miu_key]
                pred_detection = 1
                prediction = rois[miu_key]
                gt_arr = gt.get_array()
                prediction_arr = prediction.get_array()
                dice_coefficient = medmetric.dc(prediction_arr, gt_arr)
                if spacing is None:
                    spacing = list(gt.get_spacing())[::-1]
                else:
                    spacing = list(spacing)[::-1]
                print(f'    shape: {prediction_arr.shape}, spacing: {spacing}')
                if (np.sum(gt_arr) > 0) and (np.sum(prediction_arr) > 0):
                    assd = float(medmetric.assd(prediction_arr, gt_arr, voxelspacing=spacing, connectivity=1))
                    hd = float(medmetric.hd(prediction_arr, gt_arr, voxelspacing=spacing, connectivity=1))
                    hd95 = float(medmetric.hd95(prediction_arr, gt_arr, voxelspacing=spacing, connectivity=1))
                precision = medmetric.precision(prediction_arr, gt_arr)
                recall = medmetric.recall(prediction_arr, gt_arr)
                sensitivity = medmetric.sensitivity(prediction_arr, gt_arr)
                specificity = medmetric.specificity(prediction_arr, gt_arr)
        else:   # ref is absent
            ref_detection = 0
            if rois.get(miu_key) is None:
                detection_class = "TN"
                dice_coefficient = 1
                pred_detection = 0
            else:
                detection_class = "FP"
                dice_coefficient = 0 
                pred_detection = 1
        dce_score = dice_coefficient
        print("-----------------------------------------------------------------------------")
        sys.stdout.flush()

        return dict(dce_score=dce_score, dice_coefficient=dice_coefficient, 
                    assd = assd, hd = hd, hd95 = hd95, 
                    precision = precision, recall = recall, 
                    sensitivity = sensitivity, specificity = specificity, 
                    pred_detection=pred_detection, ref_detection=ref_detection, 
                    detection_class=detection_class)

    def _compute_evaluation_measures_segmentation(self, image, rois, reference, outpath, skip_images=False, extra=None):
        """Computes all evaluation for a single case. Also generates screenshots.
        Returns a dictionary to be used for report generation.

        image: the QIA image
        points: a dictionary where keys are marking types and values are a point
                location
        reference: a dictionary from the reference yaml file
        outpath: folder where output for the individual case will be saved
        """
        print("-----------------------------------------------------------------------------")
        print(f"Starting compute_evaluation_measures: see {outpath}")
        print("-----------------------------------------------------------------------------")
        sys.stdout.flush()

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
        print(f"Output path: {outpath}")
        print(f"ROI Dict: {self.roi_dict}")
        sys.stdout.flush()
        for miu_key, ref_key in self.roi_dict.items():
            #### Computing Dice Coefficient ####
            dce_dict = self._custom_score(reference, rois, ref_key, miu_key, spacing)
            print(f'    {miu_key}:{ref_key}')
            print(f'    dce_dict: {dce_dict}')
            case_dict[ref_key] = dict(  dce_score=float(dce_dict["dce_score"]), 
                                        dice_coefficient=float(dce_dict["dice_coefficient"]),
                                        marking=None, #rois
                                        roi=reference[ref_key].get("roi_file"), # reference roi path
                                        ref_detection=dce_dict.get("ref_detection"),
                                        pred_detection=dce_dict.get("pred_detection"),
                                        reference_roi=reference[ref_key].get("roi"), # reference roi
                                        )
            case_dict[ref_key]["class"] = dce_dict["detection_class"]
            case_dict[ref_key]["assd"] = dce_dict["assd"]
            case_dict[ref_key]["hd"] = dce_dict["hd"]
            case_dict[ref_key]["hd95"] = dce_dict["hd95"]
            case_dict[ref_key]["precision"] = dce_dict["precision"]
            case_dict[ref_key]["recall"] = dce_dict["recall"]
            case_dict[ref_key]["sensitivity"] = dce_dict["sensitivity"]
            case_dict[ref_key]["specificity"] = dce_dict["specificity"]

            """
                ::: case_dict has the case-wise results that will be passed on to compile function
                ::: rois has the ROIs output from MIU (loaded)
            """
            if not skip_images:
                case_dict["visuals"] = self.casewise_visualization(outpath, image, case_dict, rois, extra)      
                # if case_dict[ref_key].get("reference_roi") is not None:     ## deletes the QIA roi object before pickling to the evaluation file 
                    # print(f'del reference_roi from {ref_key}')
                    # del case_dict[ref_key]["reference_roi"]
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

        if not skip_images:
            for miu_key, ref_key in self.roi_dict.items():
                if case_dict[ref_key].get("reference_roi") is not None:     ## deletes the QIA roi object before pickling to the evaluation file 
                    print(f'Try to del reference_roi from {ref_key}')
                    sys.stdout.flush()
                    del case_dict[ref_key]["reference_roi"]

        print("Casedict", case_dict)
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
    evaluator  = EvaluatorProstate()    ## TODO: potentially generalizable if evaluator is passed through param
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
def fitness_prost(final_result):
    fitness = final_result["Prost"]["dce_score_raw"]
    total_weight = 1.
    if final_result["Prost_center"]["dce_score_raw"]:
        weight = 1.
        fitness += weight * final_result["Prost_center"]["dce_score_raw"]
        total_weight += weight
    if final_result["Prost_apex"]["dce_score_raw"]:
        weight = 2.
        fitness += weight * final_result["Prost_apex"]["dce_score_raw"]
        total_weight += weight
    if final_result["Prost_base"]["dce_score_raw"]:
        weight = 2.
        fitness += weight * final_result["Prost_base"]["dce_score_raw"]
        total_weight += weight
    fitness /= total_weight
    return fitness

from simplemind.apt_agents.optimizer.ga.src.conf import CONFIG_PATH
DEFAULT_TEMPLATE = os.path.join(CONFIG_PATH, "prost", "prost_template.tpl")
DEFAULT_SUB_TEMPLATE = os.path.join(CONFIG_PATH, "prost", "prost_sub_template.tpl")

###################

class ResultsCompilerProst(ResultsCompiler):
    def __init__(self, fitness_func=fitness_prost, overall_report_template=DEFAULT_TEMPLATE, casewise_report_template=DEFAULT_SUB_TEMPLATE ):
        super().__init__(fitness_func=fitness_func, overall_report_template=overall_report_template, casewise_report_template=casewise_report_template)

        self.thresh = {
                'Prost': 0.8,
                'Prost_apex': 0.8,
                'Prost_base': 0.8,
                'Prost_center': 0.8,
                }

        self.bad_cases = dict()
        for k in self.thresh.keys():
            self.bad_cases[k] = []
            self.good_cases[k] = []

        self.ref_keys = ["Prost",
        "Prost_apex","Prost_base","Prost_center"
        ] # markings/outputs we're interested in
    
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
                                        ['ref_detection', 'pred_detection',
                                         'dce_score', 'dice_coefficient',
                                         'assd', 'hd', 'hd95',
                                         'precision', 'recall',
                                         'voxelwise_sensitivity', 'voxelwise_specificity'
                                         ]],
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
                        df.loc[k, (mtype, 'assd')] = labels.get('assd')
                        df.loc[k, (mtype, 'hd')] = labels.get('hd')
                        df.loc[k, (mtype, 'hd95')] = labels.get('hd95')
                        df.loc[k, (mtype, 'precision')] = labels.get('precision')
                        df.loc[k, (mtype, 'recall')] = labels.get('recall')
                        df.loc[k, (mtype, 'voxelwise_sensitivity')] = labels.get('sensitivity')
                        df.loc[k, (mtype, 'voxelwise_specificity')] = labels.get('specificity')
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
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dce_score"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "dice_coefficient"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "assd"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "hd"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "hd95"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "precision"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "recall"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "voxelwise_sensitivity"))
            final_result[mtype].update(self._add_performance_metrics(marking_df, "voxelwise_specificity"))
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
    results_compiler  = ResultsCompilerProst(overall_report_template=report_templates.get("report"), casewise_report_template=report_templates.get("subreport"))
    
    if previous_output is not None and previous_output.get("result_dictionary") is not None:
        result_dictionary = previous_output["result_dictionary"]
    else:
        result_dictionary = results_compiler.load(cases, results_dir=outpath)

    fitness_tuple = results_compiler.compile_function(result_dictionary, outpath, rerun=rerun, canary=canary)

    return dict(fitness=fitness_tuple, finished=True)

