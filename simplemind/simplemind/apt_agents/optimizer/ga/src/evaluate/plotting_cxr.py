# -*- coding: utf-8 -*-
"""
Created on Wed Apr 29 13:24:50 2020

@author: mdaly
"""
import os
# os.environ['HOME'] = r"M:\apps\personal\mdaly\conda"    # MWW 07272020, to get matplotlib to work

from copy import copy
import matplotlib
if os.environ.get('DISPLAY','') == '':
    # print('No display found. Using non-interactive Agg backend.')
    matplotlib.use('Agg')
# matplotlib.use('Agg')
# matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.colors as colors

from numpy import float64 as npfloat64
# Because of this error: https://stackoverflow.com/questions/37604289/tkinter-tclerror-no-display-name-and-no-display-environment-variable



############### Plots for individual case sub-report #########################
#
#          ----------------     ----------------
#          |       1      |     |      2       |
#          |              |     |   CLAHE w/   |
#          |    CLAHE     |     |   system     |
#          |              |     |   overlay    |
#          |              |     |              |
#          ----------------     ----------------
#          |      3       |     |      4       |
#          |              |     | Origianl w/  |
#          |   Original   |     | reference &  |
#          |    Image     |     | CAD overlay  |
#          |              |     |              |
#          ----------------     ----------------
###############################################################################



### Doesn't use rois parameter, just saves png image of img_arr
def generate_screenshot(img_arr, case_dict, rois, save_path):
    """ Use this for creating plots 1 and 3. """

    # print("gen_ss: plt.figure >>> ", save_path)
    fig = plt.figure(figsize=(10, 10), dpi=128)
    ax = fig.add_subplot(1, 1, 1)
    ax.imshow(img_arr, cmap='Greys')   
    
    _gen_disclaimer(img_arr)
    
    # print("gen_ss: plt.axis off >>> ", save_path)
    plt.axis('off')
    fig.savefig(save_path, 
                bbox_inches='tight', 
                pad_inches=0)
    # print("gen_ss: plt.close >>> ", save_path)
    # plt.close(fig)
    print("gen_ss: done >>> ", save_path)
    return save_path



def generate_display(img_arr, case_dict, rois, save_path):
    " Use this for creating plot 2. """
    
    et_txt, et_color, ng_txt, ng_color = _gen_display_text(rois)
    
    # print("gen_disp: plt.figure >>> ", save_path)
    fig = plt.figure(figsize=(10, 10), dpi=128)
    ax = fig.add_subplot(1, 1, 1)
    ax.imshow(img_arr, cmap='Greys')
    
    h, w = img_arr.shape
    

    
    
    cmap_dict = {'tab:green': matplotlib.cm.winter,
                 'tab:orange': matplotlib.cm.Wistia,
                 'tab:red': matplotlib.cm.bwr}
    
    if rois['et_tube'] is not None:
        palette = copy(cmap_dict[et_color])
        palette.set_under(alpha = 0.0)
        ax.imshow(rois['et_tube'].get_array()[0], cmap=palette, 
                  norm=colors.Normalize(vmin=0.5,vmax=1.0),
                  alpha = 0.5)
        
    # print("gen_disp: plt.text >>> ", save_path)
    plt.text(50, h/20, et_txt, {'color': et_color, 'fontsize': 12},
             bbox=dict(facecolor='white', ec='none', alpha=0.7))
        
    if rois['NgTub'] is not None:
        palette = copy(cmap_dict[ng_color])
        palette.set_under(alpha = 0.0)
        ax.imshow(rois['NgTub'].get_array()[0], cmap=palette, 
                  norm=colors.Normalize(vmin=0.5,vmax=1.0),
                  alpha = 0.5)
    
    # print("gen_disp: plt.text 2 >>> ", save_path)
    plt.text(50, h/10, ng_txt, {'color': ng_color, 'fontsize': 12},
             bbox=dict(facecolor='white', ec='none', alpha=0.7))
    
    _gen_disclaimer(img_arr)
    
    # print("gen_disp: plt.axis off >>> ", save_path)
    plt.axis('off')
    fig.savefig(save_path, 
                bbox_inches='tight', 
                pad_inches=0)
    # print("gen_disp: plt.close >>> ", save_path)
    # plt.close(fig)
    
    return save_path




def generate_markings(img_arr, case_dict, rois, save_path, extra=None):
    """ Use this for creating plot 4. """

    # Create figure and plot input image
    print("gen_marks: plt.figure >>> ", save_path)
    fig = plt.figure(figsize=(10, 10), dpi=128)
    # print("a", save_path)
    ax = fig.add_subplot(1, 1, 1)
    # print("b", save_path)
    ax.imshow(img_arr, cmap='Greys')
    # print("c", save_path)

    # Choose palette so that overlay is red and everything else is transparaent
    # print("d", save_path)
    palette = copy(matplotlib.cm.bwr)
    # print("e", save_path)
    palette.set_under(alpha = 0.0)
    
    # Color list forces each marking type to be the same color
    color_list = ['tab:blue','tab:orange','tab:green','tab:purple','tab:red',
                  'tab:brown','tab:pink','tab:gray','tab:olive','tab:cyan']                   
    i = 0
    print(case_dict.keys())
    for mtype, vals in case_dict.items():
        c = color_list[i]
        i += 1
        if mtype == 'NgTub':
            if rois['ng_cnn'] is not None:
                ax.imshow(rois['ng_cnn'].get_array()[0], cmap=palette, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
            if vals.get('reference_roi') is not None:

                palette_ref = copy(matplotlib.cm.PRGn)
                ax.imshow(vals.get('reference_roi').get_array()[0], cmap=palette_ref, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
                # add_legend = True
                # for point in vals['marking']:
                #     if add_legend:
                #         ax.plot(point['x'], point['y'], marker='.', color=c,
                #                 linestyle = 'None', label=f'{mtype} reference')
                #         add_legend = False
                #     else:
                #         ax.plot(point['x'], point['y'], marker='.', color=c,
                #                 linestyle = 'None')    
        elif mtype == 'TraCh':
            #### TODO: Run this code separately ####
            if rois['trachea'] is not None: #if there is trachea.roi in the MIU results, it will plot it
                ax.imshow(rois['trachea'].get_array()[0], cmap=palette, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
            if vals.get('reference_roi') is not None: # if the reference ROI exists, then overlay it
                # add_legend = True
                # for point in vals['marking']:
                    # if add_legend:
                palette_ref = copy(matplotlib.cm.PRGn)
                ax.imshow(vals.get('reference_roi').get_array()[0], cmap=palette_ref, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
                        # ax.plot(point['x'], point['y'], marker='.', color=c,
                        #         linestyle = 'None', label=f'{mtype} reference')
        elif mtype == 'CVC':
            #### TODO: Run this code separately ####
            if rois['cvc_final'] is not None: #if there is cvc_final.roi in the MIU results, it will plot it
                ax.imshow(rois['cvc_final'].get_array()[0], cmap=palette, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
            if vals.get('reference_roi') is not None: # if the reference ROI exists, then overlay it
                # add_legend = True
                # for point in vals['marking']:
                    # if add_legend:
                palette_ref = copy(matplotlib.cm.PRGn)
                ax.imshow(vals.get('reference_roi').get_array()[0], cmap=palette_ref, 
                          norm=colors.Normalize(vmin=0.5,vmax=1.0),
                          alpha = 0.6)
                        # ax.plot(point['x'], point['y'], marker='.', color=c,
                        #         linestyle = 'None', label=f'{mtype} reference')

        else:
            if vals.get('ref_xy'): 
                ax.plot(vals['ref_xy'][0], vals['ref_xy'][1], marker='o', color=c, 
                        linestyle = 'None', label=f'{mtype} reference')
            if vals.get('pred_xy'): 
                ax.plot(vals['pred_xy'][0], vals['pred_xy'][1], marker='x', color=c, 
                        linestyle = 'None', label=f'{mtype} prediction')
                
                
    # choosing a color palette so that overlays will be blue
    palette = copy(matplotlib.cm.cool_r)
    palette.set_under(alpha = 0.0)
    
    if rois.get('et_zone') is not None:
        ax.imshow(rois['et_zone'].get_array()[0], cmap=palette, 
                  norm=colors.Normalize(vmin=0.5,vmax=1.0),
                  alpha = 0.7)       
    if rois.get('ng_zone') is not None:
        ax.imshow(rois['ng_zone'].get_array()[0], cmap=palette, 
                  norm=colors.Normalize(vmin=0.5,vmax=1.0),
                  alpha = 0.7)
                
    ax.legend(loc='lower left')
    _gen_disclaimer(img_arr)

    # print("gen_marks: plt.axis off >>> ", save_path)
    plt.axis('off')
    fig.savefig(save_path, 
                bbox_inches='tight', 
                pad_inches=0)
    # print("gen_marks: plt.close >>> ", save_path)
    # plt.close(fig)
    return save_path



def _gen_display_text(rois):
        
    if rois['et_tube']:
        if rois['et_tip_correct']:
            # If et_tube not empty and et_tip_correct not empty: "ET Tube Locate output shown" 
            # (green font to indicate Safe)
            et_txt = "ET Tube Locate: output shown"
            et_color = 'tab:green'
        else:
            # If et_tube not empty and et_tip_correct is empty: "ET Tube Locate alert shown" 
            # (red font to indicate Check Required)
            et_txt = "ET Tube Locate: alert shown"
            et_color = 'tab:red'
        if rois['et_zone'] is None:
            # If et_tube not empty and et_zone is empty: "ET Tube Locate output shown" 
            # (orange font to indicate Check Required)
            et_txt = "ET Tube Locate: output shown"
            et_color = 'tab:orange'
    else:
        # If et_tube is empty: “No ET Tube Locate output" 
        # (orange font to indicate Check Required)
        et_txt = "ET Tube Locate: no output" 
        et_color = 'tab:orange'

    if rois['ng_cnn']:
        if rois['NgTub_correct']:
            # If NgTub not empty and NgTub_correct not empty: "NG Tube Locate output shown" 
            # (green font to indicate safe)
            ng_txt = "NG Tube Locate: output shown"
            ng_color = 'tab:green'
        else:
            # If NgTub not empty and NgTub_correct is empty: "NG Tube Locate alert shown" 
            # (red font to indicate Check Required)
            ng_txt = "NG Tube Locate: alert shown"
            ng_color = 'tab:red'
        if rois['ng_zone'] is None:
            # If NgTub not empty and ng_zone is empty: "NG Tube Locate output shown" 
            # (orange font to indicate Check Required)
            ng_txt = "NG Tube Locate: output shown"
            ng_color = 'tab:orange'
    else:
        # If NgTub is empty: “No NG Tube Locate output" 
        # (orange font to indicate Check Required)
        ng_txt = "NG Tube Locate: no output" 
        ng_color = 'tab:orange'
        
        
    
    return et_txt, et_color, ng_txt, ng_color



def _gen_disclaimer(img_arr):
    h, w = img_arr.shape
    disclaimer = 'UCLA CVIB. For Investigational Use Only. \nThe performance characteristics of this research system have not been established.'
    plt.text(w/2.6, h-20, disclaimer, {'color': 'black', 'fontsize': 8},
             bbox=dict(facecolor='white', ec='none', alpha=0.7))
    return








############# Plots for overall compiled results report #######################

# MWW 08272020 updated to fix bug that didn't automatically rescale all the plots
def gen_hist_plot(df, save_path, plot_dict):
    fig, ax = plt.subplots(nrows=1, ncols=len(list(plot_dict.keys())), sharey=False, figsize=(40, 10), dpi=128)

    i = 0
    for mtype,v in plot_dict.items():
        data = df[mtype]
        vals = data[v["metric"]][~data[v["metric"]].isnull()].values.astype(npfloat64)  # MWW 08272020 must be put as np float type to autobin properly
        if len(vals) > 0:
            ax[i].hist(vals, range=(0, vals.max()), bins='auto')
            ax[i].set_title(v["title"], fontsize=20)
            ax[i].set_xlabel(v["x_label"], fontsize=16)
        i+=1

    fig.savefig(save_path,
                bbox_inches='tight',
                pad_inches=0)
    plt.close(fig)
    return save_path


def gen_scatter_plot(df, save_path, plot_dict):
    
    fig, ax = plt.subplots(nrows=1, ncols=len(list(plot_dict.keys())), figsize=(40, 10), dpi=128)
    
    i=0
    for mtype,v in plot_dict.items():
        data = df[mtype]   
        if len(data.shape) > 0:
            print(data.shape)
            ax[i].scatter(v["x_metric"], v["y_metric"])
            ax[i].set_title(v["title"], fontsize=20)
            ax[i].set_xlabel(v["x_label"], fontsize=16)
            ax[i].set_ylabel(v["y_label"], fontsize=16)
        i+=1
    # plt.subplots_adjust(wspace=0, hspace=0)
    fig.savefig(save_path, 
                bbox_inches='tight', 
                pad_inches=0)
    plt.close(fig)
    return save_path



#### non-generalized versions ####
# MWW 08272020 updated to fix bug that didn't automatically rescale all the plots
def gen_hist_plot_cxr(df, save_path):
    fig, ax = plt.subplots(nrows=1, ncols=4, sharey=False, figsize=(40, 10), dpi=128)

    titles = ['Carina', 'GE Junction', 'ETT Tip', 'NG Tube Overall']
    for i, mtype in enumerate(['Crina','GEjct','EtTip','NgTub']):        
        data = df[mtype]
        vals = data['tot_err'][~data['tot_err'].isnull()].values.astype(npfloat64)  # MWW 08272020 must be put as np float type to autobin properly
        if len(vals) > 0:
            ax[i].hist(vals, range=(0, vals.max()), bins='auto')
            ax[i].set_title(titles[i], fontsize=20)
            ax[i].set_xlabel('error [mm]', fontsize=16)
        # fig.show()

    #plt.subplots_adjust(wspace=0, hspace=0)
    fig.savefig(save_path,
                bbox_inches='tight',
                pad_inches=0)
    plt.close(fig)
    return save_path




def gen_scatter_plot_cxr(df, save_path):
    
    fig, ax = plt.subplots(nrows=1, ncols=4, figsize=(40, 10), dpi=128)
    
    titles = ['Carina', 'GE Region', 'ETT Tip', 'NG Tube Overall']
    for i, mtype in enumerate(['Crina','GEjct','EtTip','NgTub']):
        data = df[mtype]   
        if len(data.shape) > 0:
            print(data.shape)
            ax[i].scatter(data['x_err'], data['y_err'])
            ax[i].set_title(titles[i], fontsize=20)
            ax[i].set_xlabel('error in x [mm]', fontsize=16)
            ax[i].set_ylabel('error in y [mm]', fontsize=16)
    # plt.subplots_adjust(wspace=0, hspace=0)
    fig.savefig(save_path, 
                bbox_inches='tight', 
                pad_inches=0)
    plt.close(fig)
    return save_path
    
    
    
    
    
# MWW 08272020
# def gen_hist_plot_archive(df, save_path):    
    # fig, ax = plt.subplots(nrows=1, ncols=4, sharey=False, figsize=(40, 10), dpi=128)
    
    # titles = ['Carina', 'GE Junction', 'ETT Tip', 'NG Tube Overall']
    # for i, mtype in enumerate(['Crina','GEjct','EtTip','NgTub']):
        # data = df[mtype]
        # vals = data['tot_err'][~data['tot_err'].isnull()].values
        
        # if len(vals) > 0:
            # print(vals)
            # print(vals[:].shape)
            # print(i)
            # ax[i].hist(vals, range=(0, vals.max()), bins='auto')
            # ax[i].set_title(titles[i], fontsize=20)
            # ax[i].set_xlabel('error [mm]', fontsize=16)
    # #plt.subplots_adjust(wspace=0, hspace=0)
    # fig.savefig(save_path, 
                # bbox_inches='tight', 
                # pad_inches=0)
    # plt.close(fig)
    # return save_path

  
    


