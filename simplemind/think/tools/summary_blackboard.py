"""Blackboard summary tools

This script contains tools for summarizing a blackboard (BB) 
in a SimpleMind application. It will generate a visualization 
summary report (HTML) of the BBs from multiple cases.
This BB summary will visualize each solution element in BB.
"""

from argparse import ArgumentParser
import logging
import os, sys, re
import copy, yaml, inspect
import pandas as pd
import numpy as np
from medpy import metric as medmetric
from functools import partial
import matplotlib.pyplot as plt
import matplotlib.colors as pcolors
import numpy.ma as ma
from skimage.segmentation import mark_boundaries

from simplemind import __qia__
import qia.common.img.image as qimage
import qia.common.img.overlay as qovr


class BlackBoardSummarizer:
    def __init__(self, result_list, node_list, 
                reference_keys=None, cal_nodes=[], 
                metric_list = ['DC','Precision', 'Recall']):

        self.result_list = result_list
        self.node_list = node_list
        if not reference_keys: 
            self.reference_file_name_dict = dict(zip(node_list, [f'output' for node in node_list]))
        else: 
            self.reference_file_name_dict = dict(zip(node_list, reference_keys))
        self.cal_nodes = cal_nodes
        self.ORIENTATION = {
            "c": dict(crsstype=qovr.CrossSection.custom, xaxis=(1,0,0), yaxis=(0,0,-1)), #Coronal
            "s": dict(crsstype=qovr.CrossSection.custom, xaxis=(0,1,0), yaxis=(0,0,-1)), #sagittal
            "a": dict(crsstype=qovr.CrossSection.axial), #axial
        }
        cmap_red = copy.copy(plt.cm.Reds)
        mask_cmap_pred = cmap_red(np.arange(cmap_red.N)) # Get the colormap colors
        mask_cmap_pred[:,-1] = np.linspace(0.0,1.,cmap_red.N) # Set alpha
        self.mask_cmap_pred = pcolors.ListedColormap(mask_cmap_pred) # Create new colormap

        cmap_green = copy.copy(plt.cm.Greens)
        mask_cmap_gt = cmap_green(np.arange(cmap_green.N)) # Get the colormap colors
        mask_cmap_gt[:,-1] = np.linspace(0.0,1.,cmap_green.N) # Set alpha
        self.mask_cmap_gt = pcolors.ListedColormap(mask_cmap_gt) # Create new colormap

        cmap_yellow = copy.copy(plt.cm.autumn)
        mask_cmap_se = cmap_yellow(np.arange(cmap_yellow.N)) # Get the colormap colors
        mask_cmap_se[:,-1] = np.linspace(0.0,1.,cmap_yellow.N) # Set alpha
        self.mask_cmap_se = pcolors.ListedColormap(mask_cmap_se) # Create new colormap

        def catch_nan(ftn, y_pred, y_true, spacing=None):
            try:
                if 'voxelspacing' in inspect.getfullargspec(ftn).args: return ftn(y_pred, y_true, voxelspacing=spacing)
                else: return ftn(y_pred, y_true)
            except: return np.nan

        # TODO: use metric_list
        self.metric_ftns = {'DC': partial(catch_nan, medmetric.dc),
                            'Precision': partial(catch_nan, medmetric.precision),
                            'Recall': partial(catch_nan, medmetric.recall),
                            # 'ASSD': partial(catch_nan, medmetric.assd),
                            # 'HD': partial(catch_nan, medmetric.hd),
                            # 'HD95': partial(catch_nan, medmetric.hd95),
                            }

    def _get_metric(self, image_path, mask_path_list, truth_path_list=None, cal_nodes=[], reference_file_name_dict={}):
        image = qimage.read(image_path)
        spacing = np.array(image.get_spacing())[::-1]
        performance_dict_list = []
        for mask_path in mask_path_list:
            if os.path.isfile(mask_path):
                node = os.path.basename(mask_path).split(".")[0]
                performance_dict = {}
                performance_dict['node'] = node
                if node in cal_nodes:
                    print(f"    calculate {node}")
                    ref_key = reference_file_name_dict[node]
                    truth_path = truth_path_list[ref_key]
                    if os.path.isfile(truth_path):
                        if '.roi' in truth_path:
                            truth_mask = qimage.cast(image)
                            truth_mask.fill_with_roi(truth_path)
                        else:
                            truth_mask = qimage.read(truth_path)
                        truth_mask.get_alias(min_point=(0, 0, 0))
                        y_true = truth_mask.get_array()
                    else: y_true = None

                    mask = qimage.cast(image)
                    mask.fill_with_roi(mask_path)
                    mask.get_alias(min_point=(0, 0, 0))
                    y_pred = mask.get_array()

                    y_pred_cal = copy.copy(y_pred)
                    y_true_cal = copy.copy(y_true)

                    # TODO: remove this exception and replace with performance_within_search_area
                    if node == 'prostate_center':
                        center_box = qimage.cast(image)
                        center_box.fill_with_roi([x for x in mask_path_list if 'prostate_center_box.roi' in x][0])
                        center_box = center_box.get_alias(min_point=(0, 0, 0))
                        y_true_cal = y_true_cal * center_box.get_array()
                        y_pred_cal = y_pred_cal * center_box.get_array()
                        print(f'\t{node} performace calculated within the center box.')
                    if node in ['prostate_apex_attention']: #, 'prostate_apex_focused', 'prostate_apex_focused_cnn']:
                        apex_box = qimage.cast(image)
                        apex_box.fill_with_roi([x for x in mask_path_list if 'prostate_apex_box.roi' in x][0])
                        apex_box = apex_box.get_alias(min_point=(0, 0, 0))
                        y_true_cal = y_true_cal * apex_box.get_array()
                        y_pred_cal = y_pred_cal * apex_box.get_array()
                        print(f'\t{node} performace calculated within the apex box.')
                    if node in ['prostate_base_attention']: #, 'prostate_base_focused', 'prostate_base_focused_cnn']:
                        base_box = qimage.cast(image)
                        base_box.fill_with_roi([x for x in mask_path_list if 'prostate_base_box.roi' in x][0])
                        base_box = base_box.get_alias(min_point=(0, 0, 0))
                        y_true_cal = y_true_cal * base_box.get_array()
                        y_pred_cal = y_pred_cal * base_box.get_array()
                        print(f'\t{node} performace calculated within the base box.')

                        
                    for k, v in self.metric_ftns.items():
                        performance_dict[k] = v(y_pred_cal, y_true_cal, spacing=spacing)
                        print(f"\t[{node}]\t {k}:\t{performance_dict[k]}")
                    print(f'\tPerformance of {node} prediction calculated by comparing:\n\tprediction: {mask_path}\n\treference: {truth_path}')
                else:
                    for k, v in self.metric_ftns.items():
                        performance_dict[k] = None
                performance_dict_list.append(performance_dict)
        return pd.DataFrame(performance_dict_list)
    #%%

    def _generate_image_per_slices(self, image_path, mask_path_list, prefix, truth_path_list, reference_file_name_dict={}, c = 2, ncol = 4, crop_upper=0, crop_lower=0):
        org_file = f"{prefix}_t_org.png"
        image = qimage.read(image_path)
        image = image.get_alias(min_point=(0, 0, 0))
        img = image.get_array()

        nrow_before_crop = np.ceil(img.shape[0]/ncol).astype(int)
        i_min = int(nrow_before_crop * crop_upper)
        i_max = int(nrow_before_crop * (1.-crop_lower))
        nrow= i_max - i_min
        fig, axes = plt.subplots(nrow, ncol, figsize=(c*ncol,c*nrow))
        if ncol == 1: axes = [[ax] for ax in axes]
        for i in range(i_min, i_max):
            for j in range(ncol):
                ns = i*ncol + j 
                try:
                    scaled_slice = copy.copy(img[ns]).astype(np.float)
                    scaled_slice = (scaled_slice - np.min(scaled_slice))/(np.max(scaled_slice) - np.min(scaled_slice))
                    axes[i-i_min][j].imshow(scaled_slice, cmap='gray')
                except: pass
                axes[i-i_min][j].set_axis_off()
        plt.tight_layout()
        plt.savefig(org_file)
        plt.close()

        for mask_path in mask_path_list:
            if os.path.isfile(mask_path):
                node = os.path.basename(mask_path).split(".")[0]
                
                seg_file = f"{prefix}_t_{node}_seg.png"
                mask = qimage.cast(image)
                mask.fill_with_roi(mask_path)
                mask = mask.get_alias(min_point=(0, 0, 0))
                pred_roi = mask.get_array()

                fig, axes = plt.subplots(nrow, ncol, figsize=(c*ncol,c*nrow))
                if ncol == 1: axes = [[ax] for ax in axes]
                for i in range(i_min, i_max):
                    for j in range(ncol):
                        ns = i*ncol + j 
                        try:
                            scaled_slice = copy.copy(img[ns]).astype(np.float)
                            scaled_slice = (scaled_slice - np.min(scaled_slice))/(np.max(scaled_slice) - np.min(scaled_slice))
                            axes[i-i_min][j].imshow(scaled_slice, cmap='gray')
                            
                            mask_pred = pred_roi[ns] #.astype(np.int)
                            axes[i-i_min][j].imshow(ma.masked_where(mask_pred == 0, mask_pred), cmap = 'autumn', alpha=0.4)
            
                            # edges_pred = mark_boundaries(np.zeros_like(mask_pred).astype(np.float), mask_pred, color=(1,0,0), mode='thick')
                            # axes[i][j].imshow(edges_pred[:,:,0], cmap=self.mask_cmap_pred)
                            
                            # axes[i][j].text(0, 0, f'={ns}, red=pred, gred=gt', color='black', bbox=dict(facecolor='white', alpha=1))
                        except Exception as e:
                            print(f'pass drawoing {i},{j} with error {e}') 
                            pass
                        axes[i-i_min][j].set_axis_off()
                plt.tight_layout()
                plt.savefig(seg_file)
                plt.close()

                se_file = f"{prefix}_t_{node}_se.png"
                search_area_path = os.path.join(os.path.dirname(mask_path), f'search_area_{os.path.basename(mask_path)}')
                if os.path.isfile(search_area_path):
                    search_area = qimage.cast(image)
                    search_area.fill_with_roi(search_area_path)
                    search_area = search_area.get_alias(min_point=(0, 0, 0))
                    se_roi = search_area.get_array()

                    fig, axes = plt.subplots(nrow, ncol, figsize=(c*ncol,c*nrow))
                    if ncol == 1: axes = [[ax] for ax in axes]
                    for i in range(i_min, i_max):
                        for j in range(ncol):
                            ns = i*ncol + j 
                            try:
                                scaled_slice = copy.copy(img[ns]).astype(np.float)
                                scaled_slice = (scaled_slice - np.min(scaled_slice))/(np.max(scaled_slice) - np.min(scaled_slice))
                                axes[i-i_min][j].imshow(scaled_slice, cmap='gray')
                                
                                mask_se_roi = se_roi[ns] #.astype(np.int)
                                axes[i-i_min][j].imshow(ma.masked_where(mask_se_roi == 0, mask_se_roi), cmap = 'Wistia', alpha=0.4)
                                # edges_se = mark_boundaries(np.zeros_like(mask_se_roi).astype(np.float), mask_se_roi, color=(1,1,0), mode='thick')
                                # axes[i][j].imshow(edges_se[:,:,0], cmap=self.mask_cmap_se)
                                # axes[i][j].text(0, 0, f'={ns}, red=pred, gred=gt', color='black', bbox=dict(facecolor='white', alpha=1))
                            except Exception as e:
                                print(f'pass drawoing {i},{j} with error {e}') 
                                pass
                            axes[i-i_min][j].set_axis_off()
                    plt.tight_layout()
                    plt.savefig(se_file)
                    plt.close()

                gt_file = f"{prefix}_t_{node}_gt.png"
                ref_key = reference_file_name_dict[node]
                truth_path = truth_path_list[ref_key]
                if os.path.isfile(truth_path):
                    if '.roi' in truth_path:
                        truth_mask = qimage.cast(image)
                        truth_mask.fill_with_roi(truth_path)
                    else:
                        truth_mask = qimage.read(truth_path)

                    truth_mask = truth_mask.get_alias(min_point=(0, 0, 0))
                    ref_roi = truth_mask.get_array()

                    fig, axes = plt.subplots(nrow, ncol, figsize=(c*ncol,c*nrow))
                    if ncol == 1: axes = [[ax] for ax in axes]
                    for i in range(i_min, i_max):
                        for j in range(ncol):
                            ns = i*ncol + j 
                            try:
                                scaled_slice = copy.copy(img[ns]).astype(np.float)
                                scaled_slice = (scaled_slice - np.min(scaled_slice))/(np.max(scaled_slice) - np.min(scaled_slice))
                                axes[i-i_min][j].imshow(scaled_slice, cmap='gray')

                                mask_gt = ref_roi[ns] #.astype(np.int)
                                axes[i-i_min][j].imshow(ma.masked_where(mask_gt == 0, mask_gt), cmap = 'summer', alpha=0.4)
                                # edges_gt = mark_boundaries(np.zeros_like(mask_gt).astype(np.float), mask_gt, color=(1,0,0), mode='thick')
                                # axes[i][j].imshow(edges_gt[:,:,0], cmap=self.mask_cmap_gt)
                                # axes[i][j].text(0, 0, f'z={z}, red=pred, gred=gt', color='black', bbox=dict(facecolor='white', alpha=1))
                            except Exception as e:
                                print(f'pass drawoing {i},{j} with error {e}') 
                                pass
                            axes[i-i_min][j].set_axis_off()
                    plt.tight_layout()
                    plt.savefig(gt_file)
                    plt.close()

    def _get_default_orientation(self, mode='c'):
        return {mode: self.ORIENTATION[mode]}
    #%%

    def _generate_screenshot(self, image_path, mask_path_list, prefix, truth_path_list, reference_file_name_dict=None, mode='c'):
        ret = {}
        SCREENSHOT_SIZE_MM = 500
        LEVEL = 50
        WINDOW = 400
        ALPHA = 0.5
        image = qimage.read(image_path)
        image = image.get_alias(min_point=(0, 0, 0))
        mask = qimage.cast(image)
        mask.fill_with_roi(mask_path_list[0])
        mask = mask.get_alias(min_point=(0, 0, 0))

        pos = list(image.to_image_coordinates(qovr.get_centroid(mask)))
        pos = list(image.to_physical_coordinates(pos))

        minp = image.to_image_coordinates([i-SCREENSHOT_SIZE_MM/2 for i in pos])
        maxp = image.to_image_coordinates([i+SCREENSHOT_SIZE_MM/2 for i in pos])
        region = [[round(min(i,j)) for i,j in zip(minp, maxp)], [round(max(i,j)) for i,j in zip(minp, maxp)]]
        ret = {}
        ORIENTATION = self._get_default_orientation(mode)

        for k,o in ORIENTATION.items():
            org_file = f"{prefix}_{k}_org.png"
            ### This is just a fancy way to handle all the screenshot orientations in one object
            gen = qovr.auto(pos, image=image, region=region, **o)   
            gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)
            gen.write(org_file)

            for mask_path in mask_path_list:
                if os.path.isfile(mask_path):
                    node = os.path.basename(mask_path).split('.')[0]
                    seg_file = f"{prefix}_{k}_{node}_seg.png"

                    mask = qimage.cast(image)
                    mask.fill_with_roi(mask_path)
                    mask = mask.get_alias(min_point=(0, 0, 0))
                        
                    gen = qovr.auto(pos, image=image, region=region, **o)   
                    gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)
                    gen.add(mask, (255,0,0), ALPHA, boundval=0) #Seg
                    gen.write(seg_file)

                    search_area_path = os.path.join(os.path.dirname(mask_path), f'search_area_{os.path.basename(mask_path)}')
                    if os.path.isfile(search_area_path):
                        se_file = f"{prefix}_{k}_{node}_se.png"
                        gen = qovr.auto(pos, image=image, region=region, **o)   
                        gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)

                        search_area = qimage.cast(image)
                        search_area.fill_with_roi(search_area_path)
                        search_area = search_area.get_alias(min_point=(0, 0, 0))
                        gen.add(search_area, (255,255,0), ALPHA, boundval=0) #Seg
                    
                        gen.write(se_file)

                    gt_file = f"{prefix}_{k}_{node}_gt.png"
                    ref_key = reference_file_name_dict[node]
                    truth_path = truth_path_list[ref_key]
                    # print(truth_path_list)
                    # print(truth_path)
                    if os.path.isfile(truth_path):
                        if '.roi' in truth_path:
                            truth_mask = qimage.cast(image)
                            truth_mask.fill_with_roi(truth_path)
                        else:
                            truth_mask = qimage.read(truth_path)
                        truth_mask = truth_mask.get_alias(min_point=(0, 0, 0))

                        gen = qovr.auto(pos, image=image, region=region, **o)   
                        gen.set(image, LEVEL-WINDOW/2, LEVEL+WINDOW/2, boundval=-1000)
                        gen.add(truth_mask, (0,255,0), ALPHA, boundval=0) #GT
                        gen.write(gt_file)

    #%%
    def _html_path(self, outfile, linux_path):
        basepath = os.path.dirname(os.path.abspath(outfile))
        func = lambda x: os.path.relpath(x, basepath).replace("\\", "/")
        return func(linux_path)

    #%%
    def generate_visuals(self, png_dir, html_file, fig_mode='c', eval_mode='w', performance_dict_df = None):
        css="\n".join([
            ".wrapper1 {width:95%; height: 350px; overflow:hidden;}",
            # "div {text-align: center; overflow-x:auto; overflow-y:auto; display:flex;}",
            ".div1 {height: 350px; text-align: center; overflow:auto; display:flex;}",
            "figure {display: inline-block; text-align: center; width: 300px; margin:2px;}",
            "img {display: inline-block; width: 300px; margin:0px;}",
            "figcaption {text-align:center; height: auto; word-wrap: break-word; width:200px;}"
        ])
        # js="\n".join([
        #     "$(function( \){",
        #     "$('.wrapper1').scroll(function(){",
        #     "    $('.wrapper2').scrollLeft($('.wrapper1').scrollLeft());",
        #     "});",
        #     "$('.wrapper2').scroll(function(){",
        #     "    $('.wrapper1').scrollLeft($('.wrapper2').scrollLeft());",
        #     "});",
        #     "});"
        # ])
        html_contents = f"<!DOCTYPE html> \n <html> \n <head> \n"
        html_contents += f"<style> \n {css} \n </style> \n"
        # html_contents += f"<script type='text/javascript'> \n {js} \n </script> \n"
        html_contents += f"</head> \n <body> \n"
        html_contents+=f"Reference ROI is overlayed with green.\n"
        html_contents+=f"Predicted ROI is overlayed with red.\n"

        if not eval_mode.strip() == 'r': performance_df_list = []
        for elem in self.result_list.to_dict('records'):
            case_id = elem['id']
            print(f'Process {case_id}:')
            output_path = os.path.join(elem['output'], f'{self.node_list[0]}.roi')
            # print(f"  {output_path}"")
            if os.path.isfile(elem['image_file']) and os.path.isfile(output_path):
                id_fig_path = os.path.join(png_dir, case_id, case_id)
                os.makedirs(os.path.dirname(id_fig_path), exist_ok = True)
                # print(f"    {id_fig_path}")
                image_path = elem['image_file']
                mask_path_list = [os.path.join(elem['output'], f'{node}.roi') for node in self.node_list]
                truth_path_list = {}
                for k, v in self.reference_file_name_dict.items():
                    if v: truth_path_list[v] = elem[v]
                    else: truth_path_list[v] = ''
                print(f"    {image_path}")
                print(f"    {mask_path_list}")
                print(f"    {truth_path_list}")
                print("-----------------------------------------------------------")
                sys.stdout.flush()

                if not eval_mode.strip() == 'r':
                    print('Calculate performance')
                    performance_df = self._get_metric(image_path, mask_path_list, truth_path_list=truth_path_list, 
                                            cal_nodes = self.cal_nodes, reference_file_name_dict=self.reference_file_name_dict)
                    performance_df['case_id'] = case_id
                    performance_df_list.append(performance_df)
                    print("-----------------------------------------------------------")
                    sys.stdout.flush()
                else:
                    performance_df = performance_dict_df.loc[performance_dict_df.case_id == case_id]
                
                print('Visualize')
                if fig_mode == 't': 
                    self._generate_image_per_slices(image_path, mask_path_list, id_fig_path, 
                                    truth_path_list = truth_path_list, reference_file_name_dict=self.reference_file_name_dict,
                                    c = 2, ncol = 1, crop_upper=0.1, crop_lower=0.4)
                else: self._generate_screenshot(image_path, mask_path_list, id_fig_path,
                                truth_path_list = truth_path_list, reference_file_name_dict=self.reference_file_name_dict,
                                mode=fig_mode)

                html_contents+=f"<h2> {case_id} </h2> \n <div class='wrapper1'> \n <div class='div1'> \n "

                org_file = f"{id_fig_path}_{fig_mode}_org.png"
                html_contents+=f"<figure>\n<figurecaption>Original</figurecaption>\n<img src='{self._html_path(html_file, org_file)}' />\n</figure> \n"
                
                for node in self.node_list:
                    gt_file = f"{id_fig_path}_{fig_mode}_{node}_gt.png"
                    if not os.path.isfile(gt_file):
                        pass
                    else:
                        html_contents+=f"<figure>\n<figurecaption>{node} Reference</figurecaption>\n<img src='{self._html_path(html_file, gt_file)}' />\n</figure> \n"

                    seg_file = f"{id_fig_path}_{fig_mode}_{node}_seg.png" 
                    if not os.path.isfile(seg_file):
                        pass
                    else:
                        dc = np.round(np.squeeze(performance_df.loc[performance_df.node == node, 'DC'].values), 3)
                        print(f"\t[{node}\t] DC={dc}")
                        html_contents+=f"<figure>\n<figurecaption>{node} Prediction (DC={dc})</figurecaption>\n<img src='{self._html_path(html_file, seg_file)}' />\n</figure> \n"
                
                    se_file = f"{id_fig_path}_{fig_mode}_{node}_se.png" 
                    if not os.path.isfile(se_file):
                        pass
                    else:
                        dc = np.round(np.squeeze(performance_df.loc[performance_df.node == node, 'DC'].values), 3)
                        html_contents+=f"<figure>\n<figurecaption>{node} Search Area</figurecaption>\n<img src='{self._html_path(html_file, se_file)}' />\n</figure> \n"
                html_contents+="</div> \n </div> \n"
            else:
                print([elem['image_file'], os.path.isfile(elem['image_file']), 
                        os.path.join(elem['output'], f'{self.node_list[0]}.roi'), os.path.isfile(os.path.join(elem['output'], f'{self.node_list[0]}.roi') )])
                continue
            print("-----------------------------------------------------------")
            sys.stdout.flush()
        
        html_contents += "</body></html>"
        print(html_file)
        with open(html_file, 'w') as f:
            f.write(html_contents)
        if not eval_mode.strip() == 'r': 
            performance_dict_df = pd.concat(performance_df_list, ignore_index=True)
            # print(performance_dict_df)
        return performance_dict_df

def summary_blackboards(summary_dir, case_list_path, result_dir_list_path,
                    summary_html_name = 'summary.html', summary_performance_name = 'performance.csv', 
                    node_list = '', reference_keys = '', cal_nodes = '', 
                    fig_mode = 'c',  eval_mode = 'w', metric_list=['DC','Precision','Recall']
                    ):
    """generating the visualization summary report (HTML) of the BB

    Parameters
    ----------
    summary_dir : str
        path to the directory to save summary figures and report files.
    case_list_path : str
        path to the case list csv
    result_dir_list_path : str
        list of the result directory paths
    summary_html_name : str
        summary report file name (Defalut = summary.html)
    summary_performance_name : str
        summary performance csv file name (Defalut = performance.csv)
    node_list : str
        list of node name to summarize separated by "," (Default = '')
        If '', it will summarize all nodes
    reference_keys : str
        list of node name to summarize separated by "," (Default = '')
    cal_nodes : str
        list of node name to evaluate performance separated by "," (Default = '')
    fig_mode: str
        choose figure mode from c (coronal), s (sagittal), a (axial), or t (2D_slices) (Default='c')
    eval_mode : str
        choose evaluation mode from r (read) or w (write or overwrite) (Defalut = 'w')
    Returns
    -------

    """
    try:
        case_list = pd.read_csv(case_list_path, index_col=0, header=0)
        input_list = case_list[['id', 'dataset', 'image_file','reference']]
    except: 
        case_list = pd.read_csv(case_list_path, header=0)
        input_list = case_list[['id', 'dataset', 'image_file','reference']]
    
    if os.path.isfile(result_dir_list_path):
        try:
            case_list = pd.read_csv(result_dir_list_path, index_col=0, header=0)
            result_dir_list = case_list[['id', 'dataset', 'output']]
        except: 
            case_list = pd.read_csv(result_dir_list_path, header=0)
            result_dir_list = case_list[['id', 'dataset', 'output']]
    else:
        if os.path.isdir(result_dir_list_path):
            result_dir_list = []
            for elem in input_list.to_dict('records'):
                elem_id = elem['id']
                elem_dict = {'id':elem_id, 'dataset':elem['dataset'],
                            'output': os.path.join(result_dir_list_path, f'{elem_id}/sm_runner')}
                result_dir_list.append(elem_dict)
            result_dir_list = pd.DataFrame(result_dir_list)
    print(result_dir_list)
    result_list = input_list.merge(result_dir_list, how='inner', on=['id', 'dataset'])

    if node_list: node_list = [x.strip() for x in node_list.split(',')]
    else: node_list = []
    if reference_keys: reference_keys = [x.strip() for x in reference_keys.split(',')]
    else: reference_keys = []
    if cal_nodes: cal_nodes = [x.strip() for x in cal_nodes.split(',')]
    else: cal_nodes = []
    # print(node_list)
    # print(reference_keys)
    # print(cal_nodes)
    scans = BlackBoardSummarizer(result_list, node_list, reference_keys, cal_nodes, metric_list=metric_list)

    perf_path = os.path.join(summary_dir, summary_performance_name)
    summary_html_path = os.path.join(summary_dir, summary_html_name)
    if not eval_mode.strip() == 'r':
        performance_dict_df = scans.generate_visuals(summary_dir, summary_html_path, fig_mode=fig_mode, eval_mode=eval_mode)
        performance_dict_df.to_csv(perf_path, index=False)
    else:
        performance_dict_df = pd.read_csv(perf_path)
        _ = scans.generate_visuals(summary_dir, summary_html_path, fig_mode=fig_mode, eval_mode=eval_mode,
                                    performance_dict_df=performance_dict_df)

if __name__=='__main__':
    parser = ArgumentParser(description='SM GA result figure evaluation')
    parser.add_argument('--summary_dir', type=str, dest='summary_dir', 
                        help="path to save summary figures and report file.")
    parser.add_argument('--case_list_path', type=str, dest='case_list_path', 
                        help="path to the case list csv")
    parser.add_argument('--summary_html_name', type=str, dest='summary_html_name', default='summary.html', 
                        help="summary report file name. Defalut = summary.html")
    parser.add_argument('--summary_performance_name', type=str, dest='summary_performance_name', default='performance.csv', 
                        help="summary performance csv file name. Defalut = performance.csv")
    parser.add_argument('--node_list', type=str, dest='node_list', default='',
                        help="list of node name to summarize")
    parser.add_argument('--reference_keys', type=str, dest='reference_keys', default='',
                        help="list of reference_file name for each node")
    parser.add_argument('--cal_nodes', type=str, dest='cal_nodes', default='',
                        help="list of node name to evaluate performance")
    parser.add_argument('--fig_mode', type=str, dest='fig_mode', default='c', 
                        help="figure mode (c:coronal, s:sagittal, a:axial, t:2D_slices)")
    parser.add_argument('--eval_mode', type=str, dest='eval_mode', default='w', 
                        help="evaluation mode (r: read, w: write or overwrite)")
    args = parser.parse_args()

    print("--------------------------------------------------------------------------------")
    print(f'Summary BBs with {args.case_list_path}.')
    print(f'    Evaluate nodes : {args.node_list}')
    print(f'    Reference keys: {args.reference_keys}')
    print(f'    Performance evaluate nodes : {args.cal_nodes}')
    print("--------------------------------------------------------------------------------")

    os.makedirs(args.summary_dir, exist_ok = True)
    summary_blackboards(args.summary_dir, args.case_list_path,
                    summary_html_name = args.summary_html_name, summary_performance_name = args.summary_performance_name, 
                    node_list = args.node_list, reference_keys = args.reference_keys, cal_nodes = args.cal_nodes, 
                    fig_mode = args.fig_mode,  eval_mode = args.eval_mode)
    print("--------------------------------------------------------------------------------")
    print(f'Done: {os.path.join(args.summary_dir, args.summary_html_name)}')
    print("--------------------------------------------------------------------------------")
