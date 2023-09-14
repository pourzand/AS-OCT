"""Reader classes family for CNN

Spec
----
ABC base_reader
simple_image_reader

TODO 
----
@youngwonchoi
    1. simple_image_reader
        - augmentation
        - Multi-channel input loading
        - Multi-channel input resizing
    1. cxr_reader
        - code review and test
    1. image_reader
        - Building without load whole images
        - Multi-processing ability (if needed)
    1. simple_patch_reader
        - Subsampling patches
"""
import os, sys
import logging
import numpy as np
from numpy.lib.twodim_base import _trilu_dispatcher
import pandas as pd
import time, timeit
from datetime import datetime
# import psutil, pwd, getpass
import traceback
import csv
import copy
import shutil
import h5py
import subprocess
import imageio
logging.getLogger('matplotlib').setLevel(logging.ERROR)
import matplotlib.pyplot as plt
import matplotlib.colors as pcolors
logging.getLogger("PIL.PngImagePlugin").setLevel(logging.ERROR) # PIL logs too much.
logging.getLogger("matplotlib.font_manager").setLevel(logging.ERROR) # matplotlib logs too much.

from skimage.transform import resize as skresize
from sklearn.model_selection import train_test_split
# from sklearn.model_selection import KFold
# # from sklearn.preprocessing import scale, minmax_scale, robust_scale

# import pydicom

import multiprocessing as mp
from multiprocessing import Pool, Manager, Process
from functools import partial

import __qia__
import qia.common.img.image as qimage
import qia.common.dicom.obj as qdicom
from io_test import Writer
import cnntools as cnntls
from cnn_configurator import Configurator
from cnntools import _get_owner, _get_original_cmd

from skimage.segmentation import mark_boundaries

"""Note
ref: https://stackoverflow.com/questions/489861/locking-a-file-in-python

TODO: might require atomic open!!!

`fcntl` for Posix based file locking (Linux, Ubuntu, MacOS, etc.)
Only allows locking on writable files, might cause
strange results for reading.

you create a lock record on the file at filesystem level including process id.
If the process dies or closes any filedescriptor to this file, the lock record gets removed by the system.
simply: fnctl locks work as a Process <--> File relationship, ignoring filedescriptors
(See https://stackoverflow.com/questions/29611352/what-is-the-difference-between-locking-with-fcntl-and-flock)

`msvcrt` for Windows file locking
"""
# try: 
#     import psutil
# except:
#     import subprocess
#     subprocess.check_call([sys.executable, "-m", "pip", "install", "psutil", "--user"])
# try: 
#     import fcntl
# except:
#     try: # Posix based 
#         subprocess.check_call([sys.executable, "-m", "pip", "install", "fcntl", "--user"])
#     except: # Windows
#         subprocess.check_call([sys.executable, "-m", "pip", "install", "msvcrt", "--user"])
try:
    import fcntl
    def lock_file(f):
        if f.writable(): fcntl.lockf(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
    def unlock_file(f):
        if f.writable(): fcntl.lockf(f, fcntl.LOCK_UN)
except ModuleNotFoundError:
    raise ValueError('Not yet implemented for Windows file locking')
    # import msvcrt
    # def file_size(f):
    #     return os.path.getsize( os.path.realpath(f.name) )
    # def lock_file(f):
    #     msvcrt.locking(f.fileno(), msvcrt.LK_RLCK, file_size(f))
    # def unlock_file(f):
    #     msvcrt.locking(f.fileno(), msvcrt.LK_UNLCK, file_size(f))


def _save_previous_node_roi(idx, case_dict_list, previous_node_rois, node_name, _miu_previous_node_seg):
    """Compute processed inputs from case {idx}
    Parameters
    ----------
    idx : int
        index of case from training_list
    """
    process_id =  os.getpid()
    image_path = case_dict_list[idx]['image'].strip()
    try: input_roi_folder = case_dict_list[idx]['inp_roi'].strip()
    except: input_roi_folder = None
    previous_node_roi_folder = os.path.join(previous_node_rois, str(idx))
    previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'{node_name}.roi')
    """temporary avoid re-calculation"""
    if not os.path.exists(previous_seg_roi_file):
        print(f'[{process_id}|{datetime.now()}] Sub-miu call:{previous_node_roi_folder}')
        _miu_previous_node_seg(image_path, previous_node_roi_folder, input_roi_folder)
        return 1
    else:
        # print(f'[{process_id}|{datetime.now()}] Pass segmenting prvious node. Previous segmentation exists at : {previous_seg_roi_file}')
        # print(f'[{process_id}|{datetime.now()}] Pass segmenting previous nodes.')
        return 0

class base_reader(object):
    """Base Reader Class
    Abstract base class of a class family for reading images.
    Primal example of the child calss is image_reader class. 
    This subclass supports most of the image formats 
    (2D/3D; single/multi-channel) in CVIB.

    Parameters
    ----------
    config_model : dict
        dictionary of model configuration information
    log : object
        logging class instance
    mode : str
        'train' or 'prediction'

    Attributes
    ----------
    self.config_model
        dictionary of model configuration information
    self.log : object
        logging class instance
    self.mode : str
        'train' or 'prediction'
    self.model_path : str
        path of miu model
    self.node_name : str
        miu node name for CNN
    self.miu_path : str
        path of miu executable
        TODO: set as global?
    self.target_shape : list of integers
        target image shape for analysing input image
        For 3D images, target size of z, x, y dimension.
        For 2D images, target size of x, y dimension.
    self.img_channels: int
        number of the channels for analysing input image
    self.intensity_norm: list of str
        intensity_normalization method(s)
    self.bb : list of int
        bounding box coordindates for cropping CNN input image
        self.bb = [top_left_x, top_left_y, top_left_z,
                    bottom_right_x, bottom_right_y, bottom_right_z]
    self.interporlation_order: int
        order of interpolation for resizing images (default = 1)
        order parameter from resizing functions of scikit-image. 
        (skimage.transform.resize)
    self.case_dict_list: ndarray of collections.OrderedDict
        ndarray of collections.OrderedDict instances containing 
        each case's information
        (e.g., image path, reference roi path, input roi path)
    self.case_index_list: ndarray
        int index mapping case
    self.train_batch_size : int
        size of mini-batch for training
    self.validation_batch_size : int
        size of mini-batch for validation
    self.augment : bool
        whether to do augmentation on mini-batch for traiing
    self.previous_node_rois : str
        base directory to save rois from previous miu nodes
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False)
        Build a training set. If you need to specify a function for 
        specific tasks or formats, you should overwright this function 
        to build your new reader class.
    get_mini_batch(self, idxs)
        Return a mini-batch [x,y] containing given {idxs} cases for training.
        If you need to specify a function for specific tasks or formats, 
        you should overwrite this function to build your new reader class.
    get_predict_mini_batch(self, idx)
        Return a mini-batch [x] containing given {idxs} cases for testing.
        If you need to specify a function for specific tasks or formats, 
        you should overwrite this function to build your new reader class.
    resize(self, input_arr, target_shape)
        Resize 2D or 3D single-channel (currently) input images
    normalization(input_arr, intensity_norm)
        Normalize 2D or 3D single-channel (currently) input images
    write_roi(self, pred, base_img, threshold=0.5)
        write roi instance from qia.common.image based on prediction probability
    on_training_start(self)
        callback function for training start
    on_epoch_end(self)
        callback function for epoch end
    set_case_dict_list(self, image_path_list, roi_path_list=None)
        setter for self.case_dict_list
    get_case_info(self, idx)
        Return collections.OrderedDict instances containing {idx} case information
    get_image_size(self, image)
        Return image size from qia.common.img.image instance
    get_image_shape(self, image_array)
        Return image shape from numpy array of image
    _execute(self, cmd)
        helper function for execution
    _miu_previous_node_seg(self, image_path, segmentation_output_folder, input_roi_folder=None):
        miu execution for segmentation of previous nodes
    _save_previous_node_roi(self, image_path, input_roi_folder, previous_node_roi_folder)
        Compute processed inputs from case {idx}

    get_target_shape(self)
    get_intensity_norm(self)
    get_image_channels(self)
    get_mini_batch_size(self)
    get_validation_batch_size(self)
    
    Usage
    -----
    You can inherit this class to implement a new reader class 
    with a own load function, if you need to specify a
    function for specific tasks or formats.

    Examples
    --------

    """
    def __init__(self, config_model, config_resource, log, mode):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        mode : str
            'train' or 'prediction'
        """
        self.config_model = config_model
        self.config_resource = config_resource
        self.log = log
        self.mode = mode

        """miu model, node and executable information"""
        self.model_path = self.config_model['path_info']['model_file'].strip()
        self.node_name = self.config_model['model_info']['node_name'].strip()
        # with open(self.model_path, 'r') as f:
        #     node_list = f.readlines()
        # start_num = [i for i in range(len(node_list)) if 'Model:' in node_list[i]][0]
        # end_num = [i for i in range(len(node_list)) if 'End:' in node_list[i]][0]
        # node_list = node_list[start_num+2:end_num]
        # self.node_list = [node.strip() for node in node_list]
        # # self.node_list = self.node_list[::-1]
        # self.log.debug(f'Model node list: {self.node_list}')
        # current_node_num = [i for i in range(len(self.node_list)) if self.node_name == self.node_list[i]][0]
        # # self.log.debug(f'current_node_num: {current_node_num}')
        # self.log.debug(f'current_node_name: {self.node_name}')
        # try: self.previous_node_name = self.node_list[current_node_num+1]
        # except:
        #     """for the first node""" 
        #     self.previous_node_name = self.node_name
        # self.log.debug(f'previous_node_name: {self.previous_node_name}')
        current_path = os.path.realpath(__file__)
        self.miu_path = os.path.join(os.path.dirname(os.path.dirname(current_path)), 'sm')
        self.log.debug(f'SM think module executable path:  {self.miu_path}')
        
        """target image shape"""
        self.target_shape = []
        target_shape = self.config_model['model_info']['img_shape'].split(',')
        target_shape = target_shape[:-1] # exclude channel
        # self.img_channels = target_shape[:-1]
        for ishape in target_shape:
            try:
                self.target_shape.append(int(ishape.strip()))
            except:
                raise ValueError(f'{target_shape} is not an acceptable target shape.')
        if len(self.target_shape) > 2:
            self.target_shape = [self.target_shape[2], self.target_shape[0], self.target_shape[1]]
        self.log.debug(f'Configuration target image shape: {self.target_shape}')
        
        """bounding box coordinate
        TODO: change self.bb to become propertiess
        """
        bb = self.config_model['model_info']['bounding_box'].split(',')
        self.log.debug(f'Configuration bounding box coordindate: {bb}')
        bb_clean = []
        for bc in bb:
            try: bb_clean.append(int(bc.strip()))
            except: raise ValueError(f'{bb} is not an acceptable bounding box coordinate.')
        self.bb = bb_clean
        
        """Intensity normalization methods"""
        intensity_norm = self.config_model['model_info']['intensity_norm'].split(',')
        self.intensity_norm = [x.strip() for x in intensity_norm]
        self.log.debug(f'Configuration intensity_norm: {self.intensity_norm}')
        self.img_channels = 0
        for norm_descr in self.intensity_norm:
            if ("nochannel" in norm_descr) or ("empty" in norm_descr):
                continue
            else: self.img_channels += 1
        # self.img_channels = len(self.intensity_norm)
        self.log.debug(f'Configuration Number of channels: {self.img_channels}')
        
        """chromosome"""
        try:
            chromosome = self.config_model['chromosome_info']['chromosome'].strip()
            if len(chromosome) > 0:
                self.chromosome = str(self.config_model['chromosome_info']['chromosome'].strip())
            else:
                self.chromosome = None
        except: self.chromosome = None

        """default parameter for image preprocessing
        TODO: add as a NeuralNetwork node ?
        """
        self.interpolation_order = None

        """Attributes for constructing training set"""
        self.case_dict_list = None
        self.case_index_list = None

        """Configuration for mini-batch"""
        try: self.train_batch_size = int(self.config_model['training_info']['batch_size'].strip())
        except Exception as e:
            raise ValueError(f'Batch size for training is not readable: {e}')
        try: self.validation_batch_size = int(self.config_model['validation_info']['batch_size'].strip())
        except Exception as e:
            self.validation_batch_size  = None
            self.log.critical(f'Batch size for validation is not readable: {e}')
            self.log.critical(f'Computation will be done without validation.')
        try: 
            if self.config_model['training_info']['augment'].strip().lower() == 'true': self.augment = True
            else: self.augment = False
        except: self.augment = False

        """Configuration for search area attention"""
        self.search_area_attention_type = None
        search_area_attention_type=self.config_model['search_area_attention_info']['attention_type'].strip().lower()
        if search_area_attention_type !="none": self.search_area_attention_type = search_area_attention_type
        search_area_bounding_box=self.config_model['search_area_attention_info']['bounding_box'].strip().lower()
        if search_area_bounding_box == "true": self.search_area_bounding_box = True
        else: self.search_area_bounding_box = False
        self.log.debug(f'Search area attention type: {self.search_area_attention_type}')
        self.log.debug(f'Use search area bounding box: {self.search_area_bounding_box}')

        """Configuration for ph area attention"""
        use_ph_area = self.config_model['switch_from_miu_info']['use_ph_area'].strip().lower()
        if use_ph_area == "true": self.use_ph_area = True
        else: self.use_ph_area = False
        
        """Configuration for saving PNG"""
        png_skip = self.config_model['png_info']['skip_png'].strip().lower()
        png_skip_from_miu = self.config_model['switch_from_miu_info']['skip_png'].strip().lower()
        png_skip_training_from_miu = self.config_model['switch_from_miu_info']['skip_png_training'].strip().lower()
        self.log.debug(f'Skip png from miu argument: {png_skip_from_miu}')
        self.log.debug(f'Skip png for this model: {png_skip}')
        if (png_skip == 'true') or (png_skip_from_miu == 'true'): self.png_skip = True
        else: self.png_skip = False
        self.log.debug(f'Skip png: {self.png_skip}')
        if (png_skip == 'true') or (png_skip_training_from_miu == 'true'): self.png_skip_training = True
        else: self.png_skip_training = False
        self.log.debug(f'Skip png at training phase: {self.png_skip_training}')
        self.png_overwrite = True
        output_dir = self.config_model['path_info']['output_directory'].strip()
        self.training_child = os.path.join('input','previous_node_rois') in output_dir
        self.log.debug(f'The current MIU node has a child in the training phase. (self.training_child={self.training_child})')
    
        """Configuration for file locking system"""
        self.check_frequency_s=5
        
        """Configuration for resource"""
        self.log.info('---------------------------------------------------------------')
        self.log.info('CPU resources for input creation')
        self.log.info('---------------------------------------------------------------')
        self.workers=int(config_resource['CPU']['num_cpu_core'])
        self.use_multiprocessing=config_resource['CPU']['use_multiprocessing'].strip().lower()=='true'
        self.log.info(f'use_multiprocessing: {self.use_multiprocessing}')
        self.log.info(f'num_cpu_core: {self.workers}')

    def build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False):
        """Build a training set

        Parameters
        ----------
        idx : int
            index of mini-batch to construct
        workers : int
            number of cpu workers
        use_multiprocessing : bool
            whether to use multiprocessing
        
        Returns
        -------
        train_idx : ndarray
        validation_idx : ndarray

        Raises
        ------
        NotImplementedError
            You should have to implement this function for generating 
            a reader subclass
        """
        raise NotImplementedError()

    def get_mini_batch(self, idxs):
        """Return a mini-batch [x,y] containing given {idxs} cases for training
        [x]: preprocessed image array containing given {idxs} cases
        [y]: preprocessed reference array containing given {idxs} cases

        Parameters
        ----------
        idxs: ndarray
            ndarray of index of the cases for construting mini-batch
        
        Returns
        -------
        tuple of ndarray
            mini-batch [x,y] for training
        
        Raises
        ------
        NotImplementedError
            You should have to implement this function for generating 
            a reader subclass
        """
        raise NotImplementedError()
    
    def get_predict_mini_batch(self, idxs):
        """Return a mini-batch [x] containing given {idxs} cases for testing
        [x]: preprocessed image array containing given {idxs} cases

        Parameters
        ----------
        idx: int
            index of the mini-batch
        
        Returns
        -------
        ndarray
            mini-batch [x] for training
        
        Raises
        ------
        NotImplementedError
            You should have to implement this function for generating 
            a reader subclass
        """
        raise NotImplementedError()

    
    def _execute(self, cmd):
        """helper function for execution
        Parameters
        ----------
        cmd : str
            command line to execute

        Returns
        -------
        """
        proc = subprocess.Popen(cmd, shell=False)
        return proc.wait()

    def _miu_previous_node_seg(self, image_path, segmentation_output_folder, input_roi_folder=None):
        """miu execution for segmentation of previous nodes
        Parameters
        ----------
        image_path : str
            path of image to segment
        segmentation_output_folder : str
            path of output directory the segmentation from previous nodes will be helded
        input_roi_folder : str
            path of input roi directory.

        Returns
        -------

        """
        cmd = [self.miu_path, image_path, self.model_path, segmentation_output_folder, '-s', self.node_name]
        if input_roi_folder != None and len(input_roi_folder) > 0:
            cmd.extend(['-r', input_roi_folder])
        if self.chromosome != None and len(self.chromosome) > 0:
            cmd.extend(['-c', self.chromosome])
        user_resource_directory = self.config_model['path_info']['user_resource_directory'].strip()
        if len(user_resource_directory) > 0:
            cmd.extend(['-u', user_resource_directory])
        working_directory = self.config_model['path_info']['working_directory'].strip()
        cmd.extend(['-d', working_directory])
        cmd.extend(['-f'])
        png_skip_from_miu = self.config_model['switch_from_miu_info']['skip_png'].strip().lower()
        if (png_skip_from_miu == 'true'):
            cmd.extend(['-i'])
        png_skip_from_miu_training = self.config_model['switch_from_miu_info']['skip_png_training'].strip().lower()
        if (png_skip_from_miu_training == 'true'):
            cmd.extend(['-it'])
        skip_tb = self.config_model['switch_from_miu_info']['skip_tb'].strip().lower()
        if (skip_tb == 'true'):
            cmd.extend(['-t'])
        is_predict_with_cpu = self.config_model['switch_from_miu_info']['predict_cpu_only'].strip().lower()
        #if (is_predict_with_cpu == 'true'): 
        cmd.extend(['-p'])
        self._execute(cmd)

    def _generating_previous_node_rois(self, case_index_list):
        """Calculating previoud node segmentations
        
        image_path : str
            image path for calculating previous node roi
        input_roi_folder : str
            directory for input roi
        previous_node_roi_folder : str
            directory to save the roi output of the previous miu node
            currently using case_idx (training_idx or validation_idx)
        """
        # """check previous node's failure"""
        # previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag)
        # if os.path.exists(previous_node_rois) and len(os.listdir(previous_node_rois)) > 0:
        #     any_previous_input = os.listdir(previous_node_rois)[0]
        #     check_previous_roi = os.path.join(previous_node_rois, any_previous_input)
        #     if os.path.exists(check_previous_roi):
        #         self.log.debug(os.listdir(check_previous_roi))
        #         ini_from_any_previous_input = [x for x in os.listdir(check_previous_roi) if '.ini' in x]
        #         for ini_name in ini_from_any_previous_input:
        #             self.log.debug(f"From {ini_name}")
        #             ini_path = os.path.join(check_previous_roi, ini_name)
        #             tmp_config_model_class = Configurator(ini_path, self.log)
        #             tmp_config_model_class.set_config_map(tmp_config_model_class.get_section_map())
        #             tmp_config_model = tmp_config_model_class.get_config_map()
        #             tmp_node_name = tmp_config_model['model_info']['node_name']
        #             tmp_input_tag = tmp_config_model['chromosome_info']['input_tag']
        #             tmp_weight_tag = tmp_config_model['chromosome_info']['weights_tag']
        #             tmp_working_dir = os.path.join(tmp_config_model['path_info']['working_directory'], f'{tmp_node_name}_KerasModel')
        #             error_working_path = os.path.join(tmp_working_dir, f'error_log', f'error_{tmp_node_name}_{tmp_input_tag}_{tmp_weight_tag}.log')
        #             self.log.debug(f"Checking {error_working_path}")
        #             if (os.path.exists(error_working_path)):
        #                 with open(error_working_path) as f:
        #                     error_string = f.read()
        #                 check_error_string = f'Error Log for {tmp_node_name} from cnn_train.py'
        #                 if error_string.count(check_error_string) >= 3:
        #                     self.log.info('---------------------------------------------------------------')
        #                     self.log.info(f'Error is recursively repeated. Stop computation for {self.node}')
        #                     self.log.info('---------------------------------------------------------------')
        #                     sys.stdout.flush()
        #                     raise ValueError(f'Error is recursively repeated. Stop computation for {self.node}')
        # raise ValueError("debug")
        self.log.debug(f'Start computing previous node rois. Rois will be saved at {self.previous_node_rois}')
        os.makedirs(self.previous_node_rois, exist_ok=True)
        
        if self.use_multiprocessing:
            self.log.debug(f'Start computing previous node rois with {self.workers} number of CPU cores.')
            result=[]
            for idx in case_index_list[:3]:
                result.append(_save_previous_node_roi(idx, self.case_dict_list, self.previous_node_rois, self.node_name, self._miu_previous_node_seg))
            pool = Pool(processes=self.workers)
            result_multi = pool.map_async(partial(_save_previous_node_roi,
                                            case_dict_list = self.case_dict_list, 
                                            previous_node_rois = self.previous_node_rois, 
                                            node_name = self.node_name, 
                                            _miu_previous_node_seg = self._miu_previous_node_seg), 
                                    case_index_list).get()
            pool.close()
            pool.join()
            self.log.debug(f'Finished multi-process computing {len(result_multi)} number of previous node rois using {self.workers} number of CPU cores.')
            result.extend(result_multi)
            self.log.debug('--------------------------------------------------------------------')
            self.log.debug(f'Finished computing {len(result)} number of previous node rois using {self.workers} number of CPU cores.')
            self.log.debug('--------------------------------------------------------------------')
        else:
            self.log.debug(f'Start computing previous node rois with a single CPU core.')
            result=[]
            for idx in case_index_list:
                result.append(_save_previous_node_roi(idx, self.case_dict_list, self.previous_node_rois, self.node_name, self._miu_previous_node_seg))
            self.log.debug('--------------------------------------------------------------------')
            self.log.debug(f'Finished computing {len(result)} number of previous node rois using a single CPU core.')
            self.log.debug('--------------------------------------------------------------------')
        self.log.debug(f'Newly calculated cases: {np.sum(result)}.')
        self.log.debug(f'Rois were saved at {self.previous_node_rois}')
        self.log.debug('--------------------------------------------------------------------')

    def resize(self, input_arr, target_shape):
        """Resize 2D or 3D single-channel (currently) input images

        TODO
        ----
        1. update for no resizing
        1. update for list of images
        1. update for multi-channel image input
        1. update for 4D input

        Parameters
        ----------
        input_arr : ndarray
            numpy array of (currently) single-channel input images. 
            Shapes can be 2D (dx, dy) or 3D (dz, dx, dy)
        target_shape : list of int
            For 3D images, target size of z, x, y dimension.
            For 2D images, target size of x, y dimension.
        Returns
        -------
        ndarray
            resized images array
        """
        # self.log.debug(f'Original image shape: {input_arr.shape}')
        # self.log.debug(f'Target image shape: {target_shape}')
        input_arr = input_arr.squeeze() # for single 2d image, reader returns (z,x,y), thus z needs to be removed, prior resize.
        input_arr = skresize(input_arr, target_shape) #, 
                            # order=self.interpolation_order, 
                            # anti_aliasing=None)
        # self.log.debug(f'Resized image shape: {input_arr.shape}')
        return input_arr

    def resize_roi(self, input_arr, target_shape):
        """Resize 2D or 3D single-channel (currently) input images

        TODO
        ----
        1. update for no resizing
        1. update for list of images
        1. update for multi-channel image input
        1. update for 4D input

        Parameters
        ----------
        input_arr : ndarray
            numpy array of (currently) single-channel input images. 
            Shapes can be 2D (dx, dy) or 3D (dz, dx, dy)
        target_shape : list of int
            For 3D images, target size of z, x, y dimension.
            For 2D images, target size of x, y dimension.
        Returns
        -------
        ndarray
            resized images array
        """
        # self.log.debug(f'Original image shape: {input_arr.shape}')
        # self.log.debug(f'Target image shape: {target_shape}')
        # sys.stdout.flush()
        input_arr = input_arr.squeeze() # for single 2d image, reader returns (z,x,y), thus z needs to be removed, prior resize.
        # self.log.debug(f'Squeezed Original image shape: {input_arr.shape}')
        # sys.stdout.flush()
        input_arr = skresize(input_arr, target_shape, preserve_range=True, anti_aliasing=False)
        # self.log.debug(f'Resized image shape: {input_arr.shape}')
        # sys.stdout.flush()
        return input_arr

    def normalization(self, img_arr_input, intensity_norm, ph_area_image_array=None):
        """Normalize (currently) single-channel input images

        TODO
        ---- 
        1. check for no normalization case option input
        1. update for multi-channel input

        Parameters
        ----------
        img_arr_input: ndarray
            numpy array of (currently) single-channel input images.
        intensity_norm : str
            descriptor(s) that defines what intensity normalization(s) to use
        
        Returns
        -------
        ndarray
            noramlized images array
        """
        # self.log.debug(f'img_arr_input: {img_arr_input.shape}')
        if self.search_area_attention_type is not None:
            img_arr = img_arr_input[...,0]
            search_area_arr = img_arr_input[...,-1]
            # self.log.debug(f'search_area_arr: {search_area_arr.shape}')
        else: img_arr = img_arr_input

        # self.log.debug(f'img_arr: {img_arr.shape}')
        img_arr_list = []
        # self.log.debug(f'intensity_norm: {intensity_norm}')
        for norm_descr in intensity_norm:
            if ("nochannel" in norm_descr) or ("empty" in norm_descr):
                continue
            img_norm = cnntls.intensity_normalization(img_arr, norm_descr, ph_area_image_array)
            if img_norm.size==0:
                raise Exception('Intensity normalization not found: ', norm_descr)

            """(x,y) -> (x,y,1) to make it 3D/channel, but with a single channel"""
            img_norm = np.expand_dims(img_norm, axis=-1)
            img_arr_list.append(img_norm)
        
        if self.search_area_attention_type is not None:
            search_area_arr_norm = np.expand_dims(search_area_arr, axis=-1)
            search_area_arr_norm[search_area_arr_norm > 0.] = 1.
            img_arr_list.append(search_area_arr_norm)
        multichinput = np.concatenate(img_arr_list, axis=-1)
        # self.log.debug(f'multichinput: {multichinput.shape}')
        return multichinput

    def augmentation(self, img_arr):
        """on-the-fly augmentation

        TODO
        ----
        using augmentation class

        Parameters
        ----------
        img_arr : ndarray
            input array to augment

        Returns
        -------
        ndarray
        """
        return img_arr
    
    def write_roi(self, pred, base_img, threshold=0.5, norm_img_arr = None):
        """write roi based on prediction probability

        Parameters
        ----------
        pred : ndarray
            numpy array of the set of prediction probabilities
        base_img : qia.common.img.image instance
            base image for reference. Used for writing 
            roi instance from qia.common.img.image
        threshold : float
            threshold for roi rescale (default = 0.5)
        """

        """TODO: [YC] I think this squeezing way can cause bugs in non-binary cases. Take a look later
        Then change it to self.resize.."""
        pred = np.squeeze(pred)
        self.log.debug(f'Prediction output shape {pred.shape}')
        # original_img_shape = base_img.get_size()
        original_img_shape = self.get_image_shape(base_img.get_array())
        self.log.debug(f'Target output shape {original_img_shape}')
        
        if self.previous_node_rois is not None:
            previous_seg_roi_file = os.path.join(self.previous_node_rois, f'search_area_{self.node_name}.roi')
        else:
            previous_seg_roi_file = None
        if previous_seg_roi_file is not None:
            try:
                assert (os.path.exists(previous_seg_roi_file))
            except:
                """TODO:temp way for prediction. need for revising"""
                previous_seg_roi_file_dir = os.path.dirname(os.path.dirname(previous_seg_roi_file))
                previous_seg_roi_file_name = os.path.basename(previous_seg_roi_file)
                previous_seg_roi_file = os.path.join(previous_seg_roi_file_dir, previous_seg_roi_file_name)
                self.log.debug(f'Search area for prediction: {previous_seg_roi_file}')
                assert (os.path.exists(previous_seg_roi_file))
                
            previous_seg_roi = qimage.read(previous_seg_roi_file)
            # previous_seg_roi = previous_seg_roi.get_alias(min_point=(0, 0, 0))
            minpoint, maxpoint = previous_seg_roi.find_region(1,1)
            tlx, tly, tlz = minpoint[0], minpoint[1], minpoint[2]
            brx, bry, brz =  maxpoint[0], maxpoint[1], maxpoint[2]
            bb = [tlx, tly, tlz, brx, bry, brz]
        else:
            bb = copy.copy(self.bb)
            
        if self.search_area_attention_type is not None and not self.search_area_bounding_box:
            tlx, tly, tlz, brx, bry, brz = 0, 0, 0, original_img_shape[2]-1, original_img_shape[1]-1, original_img_shape[0]-1
        else:
            tlx, tly, tlz, brx, bry, brz = bb
            
        if len(pred.shape) == 2: # if 2d mask encountered, add z dimension to front.
            pred = np.expand_dims(pred,axis=0)
        if len(pred.shape) > 3:
            raise ValueError("qia/miu/roi does not support 4d images currently")
        pred = skresize(pred, (brz-tlz+1, bry-tly+1, brx-tlx+1)) #, self.interpolation_order)
        # TODO: pending code review. order threshold then resize, or the other way around.
        pred = (pred >= threshold).astype(np.float) # resize require float

        # if base_img.get_min_point() == (0,0,0):
        #     # inhouse roi format, min-point starts with (0,0,1) # DIFFERENT from nifti and the rest of standard formats!
        #     # the inhouse rule was made since roi format assume your typical first image slice z of 0 starts with instance number of 1.
        #     self.log.warning('(0,0,0) detected from base_img, using (0,0,1) as min_point prior storing roi file.')
        #     base_img = base_img.get_alias(min_point=(0,0,1))

        # """check image orientation for 3D image input"""
        # if len(pred.shape) > 2:
        #     """Note: for 3D images open with .seri file
        #     @Youngwon
        #     flip on z-axis bases on
        #     https://gitlab.cvib.ucla.edu/qia/pcl/-/blob/master/include/pcl/image_io/DicomImageSeriesReader.h#L91
        #     """
        #     image_orientation = base_img.get_orientation()
        #     image_orientation_rounded = np.round(image_orientation, 1)
        #     if image_orientation_rounded[-1] < 0:
        #         self.log.debug(f'orientation: {image_orientation}')
        #         self.log.debug(f'orientation (rounded): {image_orientation_rounded}')
        #         pred = pred[::-1]
        #         self.log.debug(f'Images and rois are flipped on z-axis.')
        pred_cast = pred.astype(np.uint8)
        pred_rescale = np.zeros((original_img_shape[0], original_img_shape[1], original_img_shape[2]))
        pred_rescale[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)] = pred_cast       
        
        pred_image = qimage.from_array(pred_rescale, template=base_img)

        self.log.debug(f'Written roi size: {pred_image.get_size()}')
        # return pred_image
        return [pred_image], ['pred'], None, None

    def draw_fig(self, review_fig_dir, img_norm_list, intensity_norm, roi_arr=None, ph_area_mask_array=None, original_img=None, c=4, alhpa=0.7, png_overwrite=False):
        """save the figure of preprocessed input and roi under
        review_fig_dir for the review purpose
        For training, our convention is to save it under
        {working_dir}/input/previous_node_roi_folder/{input_tag}/{case_name}
        (and figure file name: 
        {case_name}_{node_name}_{norm_descr}_inp_img.png)
        For prediction, our convention is to save it under
        {output_dir}/
        (and figure file name: {node_name}_{norm_descr}_inp_img.png)
        
        Parameters
        ----------
        review_fig_dir : str
            directory to save png
        img_norm_list : ndarray
            preprocessed input
        roi_arr : ndarray
            preprocessed roi
        intensity_norm : list of str
            list of descriptor(s) that defines what intensity normalization(s) to use
        c : int
            figure image resolution
        alpha: float within range [0,,1.]
            The transparency ratio for ROI overlay
        png_overwrite: bool
            whether to overwrite png or pass
        """
        # try:
        if review_fig_dir is not None:
            # try:
            #     self.log.debug(f"ph_area_mask_array: {ph_area_mask_array.shape}")
            #     self.log.debug(f"original_img: {original_img.shape}")
            # except: pass
            intensity_norm_draw = copy.copy(intensity_norm)
            os.makedirs(review_fig_dir, exist_ok=True)
            if self.search_area_attention_type is not None:
                intensity_norm_draw.append(f'attention{self.search_area_attention_type}')
            n_norm = 0
            for norm_descr in intensity_norm_draw:
                png_output_path = os.path.join(review_fig_dir, f'{self.node_name}_{norm_descr}_input_image.png')
                if ("nochannel" in norm_descr) or ("empty" in norm_descr):
                    continue
                img_norm = img_norm_list[...,n_norm]
                # self.log.debug(f"img_norm: {img_norm.shape}")
                # self.log.debug(f'png {png_output_path}: {n_norm} {img_norm.shape}')
                n_norm += 1
                draw_img_norm = ((not os.path.exists(png_output_path))| png_overwrite)
                if draw_img_norm:
                    if len(img_norm.shape) == 2:
                        """Assume 2D"""
                        nrow = 1
                        ncol = 2  #len(intensity_norm_draw)
                        if roi_arr is not None: ncol += 1
                        if ph_area_mask_array is not None: ncol+=1 
                        if original_img is not None: ncol+=2 #+1 for original image, +1 for orignal image histogram
                        i = 0
                        fig, axes = plt.subplots(nrow, ncol, figsize=(ncol*c, nrow*c))
                        # fig.set_size_inches(ncol*c, nrow*c)
                        # fig.set_dpi(10*c)
                        """original image"""
                        if original_img is not None:
                            axes[i].imshow(original_img, cmap=plt.cm.gray)
                            axes[i].set_title('2D slice\n[original image]', y=-0.3, ha="center")
                            axes[i].axis('off')
                            i+=1
                        """display images"""
                        # img_display = (np.clip(img_norm, -4, 4) + 4)*1024/8 # in previous cnn_predict
                        img_display = img_norm.copy() # in previous cnn_training
                        axes[i].imshow(img_display, cmap=plt.cm.gray)
                        axes[i].set_title('2D slice\n[preprocessed image]',y=-0.3, ha="center") 
                        axes[i].axis('off')
                        i += 1
                        """display rois"""
                        if roi_arr is not None:
                            """
                            TODO
                            ----
                            update for not binary roi
                            """
                            if len(roi_arr.shape) == 1:
                                """point roi (roi_arr) = (y, x)"""
                                """TODO
                                check img_display.shape ordering is (y,x) or (x,y)
                                """
                                rows, cols = img_display.shape
                                prf, pcf = roi_arr
                                # if (0<=scaled_prf<=1) and (0<=scaled_pcf<=1):
                                #     tlx, tly, _, brx, bry, _ = self.bb
                                #     prf = (int)(scaled_prf*(bry-tly)) + tly
                                #     pcf = (int)(scaled_pcf*(brx-tlx)) + tlx
                                #     self.log.debug(f'(prs, pcs)={(prf, pcf)}, bb = {self.bb}')

                                axes[i].imshow(img_display, cmap=plt.cm.gray)
                                if (prf>=0) and (pcf>=0):
                                    prs = round(rows*prf).astype(int)
                                    pcs = round(cols*pcf).astype(int)

                                    # for m in range(max(prs-7, 0), min(prs+8, cols)):
                                    #     for n in range(max(pcs-7, 0), min(pcs+8, rows)):
                                    #         img_display[m][n] = 1
                                    # for m in range(max(prs-5, 0), min(prs+6, cols)):
                                    #     for n in range(max(pcs-5, 0), min(pcs+6, rows)):
                                    #         img_display[m][n] = 0
                                    axes[i].scatter([pcs], [prs], c='r')
                                    axes[i].set_title(f'2D slice with ROI\n(pcs, prs)=({prs},{pcs}), (pcf, prf)=({pcf},{prf})\n[on preprocessed image]',y=-0.3, ha="center") 
                                else:
                                    axes[i].set_title('2D slice with missing ROI\n[on preprocessed image]',y=-0.3, ha="center") 
                                axes[i].axis('off')

                                """
                                TODO
                                -----
                                Adding text label to print warning if point marking is placed outside the bounding box
                                """
                            elif len(roi_arr.shape) == 3:
                                """2D ROI"""
                                axes[i].imshow(roi_arr[...,0], cmap=plt.cm.gray)
                                axes[i].set_title('2D slice\nwith ROI',y=-0.3, ha="center") 
                                axes[i].axis('off')
                            else:
                                raise ValueError(f'ROI shape is not matched with image input shape = {img_norm.shape}. roi shape = {roi_arr.shape}')
                            i += 1
                        """display ph_area"""
                        if ph_area_mask_array is not None:
                            cmap_red = copy.copy(plt.cm.Reds)
                            mask_cmap_pred = cmap_red(np.arange(cmap_red.N)) # Get the colormap colors
                            mask_cmap_pred[:,-1] = np.linspace(0.0,1.,cmap_red.N) # Set alpha
                            mask_cmap_pred = pcolors.ListedColormap(mask_cmap_pred) # Create new colormap
                            mask_pred = ph_area_mask_array.astype(np.int)
                            if (np.count_nonzero(mask_pred) != 0):
                                axes[i].imshow(img_display, cmap='gray')
                                edges_pred = mark_boundaries(np.zeros_like(mask_pred).astype(np.float), mask_pred, color=(1,0,0), mode='thick')
                                axes[i].imshow(edges_pred[:,:,0], cmap=mask_cmap_pred)
                            axes[i].set_title('2D slice\nph area',y=-0.3, ha="center")
                            axes[i].axis('off')
                            i+=1
                        if original_img is not None:
                            """display original image histogram"""
                            axes[i].hist(original_img.ravel(), bins=256, histtype='step', color='black')
                            axes[i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                            axes[i].set_xlabel('Pixel intensity\n[original image]')
                            axes[i].set_ylabel('Number of pixels')
                            axes[i].set_yticks([])
                            i+=1
                        """display preprocessed histogram"""
                        axes[i].hist(img_norm.ravel(), bins=256, histtype='step', color='black')
                        axes[i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                        axes[i].set_xlabel('Pixel intensity\n[preprocessed image]')
                        axes[i].set_ylabel('Number of pixels')
                        axes[i].set_yticks([])
                        """save"""
                        fig.tight_layout()
                        plt.savefig(png_output_path) #bbox_inches='tight', pad_inches=0)
                        plt.close()
                        self.log.debug(f'PNG saved at {png_output_path}')
                        sys.stdout.flush()
                    elif len(img_norm.shape) == 3:
                        """Assume 3D"""
                        nrow = img_norm.shape[0] + 1
                        ncol = 2  #len(intensity_norm_draw)
                        if roi_arr is not None: ncol += 1
                        if ph_area_mask_array is not None: ncol+=1
                        if original_img is not None: ncol+=2 #+1 for original image, +1 for orignal image histogram 
                        
                        i = 0
                        fig, axes = plt.subplots(nrow, ncol, figsize=(ncol*c, nrow*c))
                        # fig.set_size_inches(ncol*c, nrow*c)
                        # fig.set_dpi(10*c)
                        """cmap for ROI overay"""                    
                        cmap_inferno = copy.copy(plt.cm.inferno)
                        cmap_overay = cmap_inferno([0, cmap_inferno.N]) # Get the colormap colors
                        cmap_overay[:,-1] = [0.,1.] # Set alpha
                        cmap_overay = pcolors.ListedColormap(cmap_overay) # Create new colormap
                        
                        """Original Image Display"""
                        if original_img is not None:
                            for j in range(1,nrow):
                                axes[j][i].imshow(original_img[j-1], cmap=plt.cm.gray)
                                axes[j][i].set_title(f'{j}th 2D slice\n[original image]', ha="center", y=-0.3)
                                axes[j][i].axis('off')
                            i+=1

                        """display images"""
                        # img_display = (np.clip(img_norm, -4, 4) + 4)*1024/8 # in previous cnn_predict
                        img_display = img_norm.copy() # in previous cnn_training
                        for j in range(1,nrow):
                            axes[j][i].imshow(img_display[j-1], cmap=plt.cm.gray)
                            axes[j][i].set_title(f'{j}th 2D slice\n[preprocessed image]', ha="center", y=-0.3)
                            axes[j][i].axis('off')
                        i += 1
                        """display rois"""
                        if roi_arr is not None:
                            """
                            TODO
                            ----
                            update for not binary roi
                            """
                            if len(roi_arr.shape) == 4:
                                """3D ROI"""
                                """Overay ROIs"""
                                for j in range(1,nrow):
                                    axes[j][i].imshow(img_display[j-1], cmap=plt.cm.gray)
                                    axes[j][i].imshow(roi_arr[j-1,...,0], cmap=cmap_overay, alpha=alhpa)
                                    axes[j][i].set_title(f'{j}th 2D slice\nwith ROI\n[on preprocessed image]', ha="center", y=-0.3)
                                    axes[j][i].axis('off')
                            else:
                                raise ValueError(f'ROI shape is not matched with image input = {img_norm.shape}. roi shape = {roi_arr.shape}')
                            i += 1
                        #ph_area_mask_array shape is 4D or 3D in main code?
                        if ph_area_mask_array is not None:
                            cmap_red = copy.copy(plt.cm.Reds)
                            mask_cmap_pred = cmap_red(np.arange(cmap_red.N)) # Get the colormap colors
                            mask_cmap_pred[:,-1] = np.linspace(0.0,1.,cmap_red.N) # Set alpha
                            mask_cmap_pred = pcolors.ListedColormap(mask_cmap_pred) # Create new colormap
                            
                            for j in range(1,nrow):
                                mask_pred = ph_area_mask_array[j-1].astype(np.int)
                                if (np.count_nonzero(mask_pred) != 0):
                                    scaled_slice = copy.copy(img_display[j-1]).astype(np.float)
                                    axes[j][i].imshow(scaled_slice, cmap='gray')

                                    mask_pred = ph_area_mask_array[j-1].astype(np.int)
                                    edges_pred = mark_boundaries(np.zeros_like(mask_pred).astype(np.float), mask_pred, color=(1,0,0), mode='thick')
                                    axes[j][i].imshow(edges_pred[:,:,0], cmap=mask_cmap_pred)

                                axes[j][i].set_title(f'{j}th 2D slice\nph area', ha="center", y=-0.3)
                                axes[j][i].axis('off')
                            i+=1

                        if original_img is not None:
                            """display histogram for whole 3D original image"""
                            axes[0][i].hist(original_img.ravel(), bins=256, histtype='step', color='black')
                            axes[0][i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                            axes[0][i].set_xlabel('Pixel intensity\n(for whole 3D input)\n[original image]')
                            axes[0][i].set_ylabel('Number of pixels')
                            axes[0][i].set_yticks([])
                            """display histogram for each 2D slice"""
                            for j in range(1,nrow):
                                axes[j][i].hist(original_img[j-1].ravel(), bins=256, histtype='step', color='black')
                                axes[j][i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                                axes[j][i].set_xlabel(f'Pixel intensity\n(for {j}th 2D slice)\n[original image]')
                                axes[j][i].set_ylabel('Number of pixels')
                                axes[j][i].set_yticks([])

                            i+=1
                        """display histogram for whole 3D input"""
                        axes[0][i].hist(img_display.ravel(), bins=256, histtype='step', color='black')
                        axes[0][i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                        axes[0][i].set_xlabel('Pixel intensity\n(for whole 3D input)\n[preprocessed image]')
                        axes[0][i].set_ylabel('Number of pixels')
                        axes[0][i].set_yticks([])
                        """display histogram for each 2D slice"""
                        for j in range(1,nrow):
                            axes[j][i].hist(img_display[j-1].ravel(), bins=256, histtype='step', color='black')
                            axes[j][i].ticklabel_format(axis='y', style='scientific', scilimits=(0, 0))
                            axes[j][i].set_xlabel(f'Pixel intensity\n(for {j}th 2D slice)\n[preprocessed image]')
                            axes[j][i].set_ylabel('Number of pixels')
                            axes[j][i].set_yticks([])
                        """save"""
                        fig.tight_layout()
                        plt.savefig(png_output_path) #bbox_inches='tight', pad_inches=0)
                        plt.close()
                        self.log.debug(f'PNG saved at {png_output_path}')
                        sys.stdout.flush()
                    else:
                        raise ValueError(f'Image with shape {img_norm.shape} is not supported for png review.')
                else:
                    # self.log.debug(f'Skip PNG save {png_output_path}')
                    pass

    def on_training_start(self):
        """callback function for training start
        """
        pass
    
    def on_epoch_end(self):
        """callback function for epoch end
        """
        pass

    def set_case_dict_list(self, image_path_list, roi_path_list=None):
        """setter for self.case_dict_list

        Parameters
        ----------
        image_path_list : list of str
        roi_path_list : list of str (default=None)
            If None, it will be ignored

        TODO
        ----
        make case_dict_list to be a collections.OrderedDict instance
        add inp_path_list parameters
        """
        self.case_dict_list = []
        if roi_path_list is not None:
            assert len(image_path_list) == len(roi_path_list), f'len(image_path_list)={len(image_path_list)} is not \
                same as len(roi_path_list)={len(roi_path_list)}'
            for i, (img_path, roi_path) in enumerate(zip(image_path_list, roi_path_list)):
                self.case_dict_list.append({'image':img_path, 'ref_roi': roi_path})
        else:
            for i, img_path in enumerate(image_path_list):
                self.case_dict_list.append({'image':img_path})
        
    
    def get_case_info(self, idx):
        """
        Parameters
        ----------
        int
            case index to get the case information

        Returns
        -------
        collections.OrderedDict
            collections.OrderedDict instances containing {idx} case information
        """
        return self.case_dict_list[idx]
    
    def get_image_size(self, image):
        """Return image size from qia.common.img.image object
        Parameters
        ----------
        image : qia.common.img.image object
            qia.common.img.image object

        Returns
        -------
        tuple of integers
            original image size (x,y,z)
        """
        return image.get_size()
    
    def get_image_shape(self, image_array):
        """Return image shape from numpy array of image
        Parameters
        ----------
        image_array : ndarray
            numpy array of input image

        Returns
        -------
        tuple of integers
            original image dimensions (z,y,x)
        """
        return image_array.shape
    
    # getters
    def get_target_shape(self):
        """
        Returns
        -------
        list of integers
            target image dimensions
        """
        return self.target_shape
    
    def get_intensity_norm(self):
        """
        Returns
        -------
        list of str
            intensity_normalization methods
        """
        return self.intensity_norm

    def get_image_channels(self):
        """
        Returns
        -------
        int
            target image channels
        """
        return self.img_channels
    
    def get_mini_batch_size(self):
        """
        Returns
        -------
        int
            size of mini-batch for training
        """
        return self.train_batch_size
    
    def get_validation_batch_size(self):
        """
        Returns
        -------
        int
            size of mini-batch for validation
        """
        return self.validation_batch_size

class image_reader(base_reader):
    """Image reader class for loading image formats in CVIB.
    This class supports most of the image formats (2D/3D; 
    single/multi-channel) in CVIB.
    This image reader load and process images in online-manner, without
    saving processed whole dataset as hdf5. This reader is appropriate 
    for large dataset. If your dataset is small, or want to process whole
    dataset once before training, you can use simple_image_reader instead.
    It assume roi_reference is given. For dealing with x,y coordinates,
    use cxr_image_reader.

    Attributes
    ----------
    self.config_model
        dictionary of model configuration information
    self.log : object
        logging class instance
    self.mode : str
        'train' or 'prediction'
    self.target_shape : list of integers
        target image shape for analysing input image
        For 3D images, target size of z, x, y dimension.
        For 2D images, target size of x, y dimension.
    self.img_channels: int
        number of the channels for analysing input image
    self.intensity_norm: list of str
        intensity_normalization method(s)
    self.bb : list of int
        bounding box coordindates for cropping CNN input image
        self.bb = [top_left_x, top_left_y, top_left_z,
                    bottom_right_x, bottom_right_y, bottom_right_z]
    self.interporlation_order: int
        order of interpolation for resizing images (default = 1)
        order parameter from resizing functions of scikit-image. (skimage.transform.resize)
    self.case_dict_list: ndarray of collections.OrderedDict
        ndarray of collections.OrderedDict instances containing each case's information
        (e.g., mapped_idx, image path, reference roi path, input roi path)
    self.train_batch_size : int
        size of mini-batch for training
    self.validation_batch_size : int
        size of mini-batch for validation
    self.augmentation : bool
        whether to do augmentation on mini-batch for training
    self.previous_node_rois : str
        base directory to save rois from previous miu nodes
    self.search_area_attention_type : str
        type of search area attention
    
    Methods
    -------
    build_training_set(self, overwrite_input=False, workers, use_multiprocessing)
        Build a training set, save it as hdf5, and load whole set.
        Assume roi_reference is given.
    get_mini_batch(self, idxs)
        Return a mini-batch [x,y] containing given {idxs} cases for training
    get_predict_mini_batch(self, idx)
        Return a mini-batch [x] containing given {idxs} cases for testing
    resize(self, input_arr, target_shape)
        Resize 2D or 3D single-channel (currently) input images
    normalization(input_arr, intensity_norm)
        Normalize 2D or 3D single-channel (currently) input images
    write_roi(self, pred, base_img, threshold=0.5)
        write roi object from qia.common.image based on prediction probability
    on_training_start(self)
        callback function for training start
    on_epoch_end(self)
        callback function for epoch end
    get_case_info(self, idx)
        Return collections.OrderedDict instances containing {idx} case information
    get_image_size(self, image)
        Return image size from qia.common.img.image object
    get_image_shape(self, image_array)
        Return image shape from numpy array of image
    _execute(self, cmd)
        helper function for execution
    _miu_previous_node_seg(self, image_path, segmentation_output_folder, input_roi_folder=None):
        miu execution for segmentation of previous nodes
    _save_previous_node_roi(self, image_path, input_roi_folder, previous_node_roi_folder)
        Compute processed inputs from case {idx}
    _load(self, image_path, roi_path=None, input_roi_folder=None, tmp_roi_name=None)
        Private function return qia.common.img.image and numpy array of 
        loaded image (and roi) from image_path (and roi_path)
    _get_processed_inputs(self, idx)
        Private function return processed inputs from case {idx}
    
    get_target_shape(self)
    get_intensity_norm(self)
    get_image_channels(self)
    get_mini_batch_size(self)
    get_validation_batch_size(self)

    Spec
    ----
    Supporting image formats:
        image formats supported by qia.common.img.image.read
            - .dcm
            - .nii
            - .mhd
            - .seri: in-house seri file listing DICOM image paths 
    
    Supporting image shapes: 2D / 3D

    Supporting image channels: single-channel (TODO: multi-channel)
    
    Usage
    -----

    Examples
    --------

    """
    # __doc__ = base_reader.__doc__ + __doc__ # TODO: docstring inheritation
    def __init__(self, config_model, config_resource, log, mode):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        mode : str
            'train' or 'prediction'
        """
        super(image_reader,self).__init__(config_model, config_resource, log, mode)
        self.png_overwrite = False

    def _clean_input(self):
        process_id =  os.getpid()
        process_user = _get_owner(process_id)
        process_cmd = ' '.join(_get_original_cmd(process_id))
        # status_update_input = False

        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        input_dir = os.path.join(working_dir, 'input')
        # input_avaialble_path = os.path.join(input_dir, f'available_input_{input_tag}')
        input_hist_path = os.path.join(input_dir, f'history_input_{input_tag}')
        os.makedirs(input_dir, exist_ok=True)
        
        # with open(input_avaialble_path, 'a') as available_input_f:
        #     lock_file(available_input_f)
        self.log.info('---------------------------------------------------------------')
        self.log.info(f'Something wrong with the previous input with tag {input_tag}. Remove the regacy input...')
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        summary_process_removing = f'[{process_user}|{process_id}|{datetime.now()}] Start removing any input-related files with tag {input_tag}\n'
        # available_input_f.write(summary_process_removing)
        # available_input_f.flush()
        self.log.info(summary_process_removing)
        sys.stdout.flush()

        train_idx_path = os.path.join(input_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(input_dir, f'valid_{input_tag}.csv')
        previous_node_rois = os.path.join(input_dir, 'previous_node_rois', input_tag)
        hdf5_train_path = os.path.join(input_dir, f'train_{input_tag}.hdf5')
        hdf5_validation_path = os.path.join(input_dir, f'valid_{input_tag}.hdf5')
        npy_train_dir_path = os.path.join(input_dir, f'train_{input_tag}_npy')
        npy_validation_dir_path = os.path.join(input_dir, f'valid_{input_tag}_npy')
        if os.path.exists(previous_node_rois): shutil.rmtree(previous_node_rois)
        if os.path.exists(hdf5_train_path): os.remove(hdf5_train_path)
        if os.path.exists(hdf5_validation_path): os.remove(hdf5_validation_path)
        if os.path.exists(npy_train_dir_path): shutil.rmtree(npy_train_dir_path)
        if os.path.exists(npy_validation_dir_path): shutil.rmtree(npy_validation_dir_path)
        if os.path.exists(train_idx_path): os.remove(train_idx_path)
        if os.path.exists(validation_idx_path): os.remove(validation_idx_path)
        if os.path.exists(input_hist_path): os.remove(input_hist_path)
    
        summary_process_removing = f'[{process_user}|{process_id}|{datetime.now()}] Done. Any input related file under input tag: {input_tag} is cleared.\n'
        # available_input_f.write(summary_process_removing)
        # available_input_f.flush()
        # unlock_file(available_input_f)
        self.log.info(summary_process_removing)
        sys.stdout.flush()

    def _build_training_set(self, train_idx, validation_idx):
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        hdf5_dir = os.path.join(working_dir, 'input')
        train_idx_path = os.path.join(hdf5_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(hdf5_dir, f'valid_{input_tag}.csv')

        """Save train and validation idx"""
        np.savetxt(train_idx_path, train_idx, delimiter=",")
        self.log.info(f'Writing train index finished: {train_idx_path}')
        
        np.savetxt(validation_idx_path, validation_idx, delimiter=",")
        self.log.info(f'Writing validation index finished: {validation_idx_path}')

    def _load_training_set(self):
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        hdf5_dir = os.path.join(working_dir, 'input')
        train_idx_path = os.path.join(hdf5_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(hdf5_dir, f'valid_{input_tag}.csv')

        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        """Load training and validation list"""
        train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Train index: {train_idx}')
        validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Validation index: {validation_idx}')
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        return train_idx, validation_idx
    
    # def build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False):
    #     """Build a training set for online-loading mini-batches.
    #     Assume roi_reference is given.
        
    #     Parameters
    #     ----------
    #     idx : int
    #         index of mini-batch to construct

    #     Returns
    #     -------
    #     train_idx : ndarray
    #     validation_idx : ndarray
    #     """
    #     tic = timeit.default_timer()

    #     self.log.info('---------------------------------------------------------------')
    #     self.log.info('Buidling training set')
    #     self.log.info('---------------------------------------------------------------')
    #     working_dir = self.config_model['path_info']['working_directory']
    #     working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
    #     input_tag = self.config_model['chromosome_info']['input_tag']
    #     hdf5_dir = os.path.join(working_dir, 'input')
    #     os.makedirs(hdf5_dir, exist_ok=True)
    #     train_idx_path = os.path.join(hdf5_dir, f'train_{input_tag}.csv')
    #     validation_idx_path = os.path.join(hdf5_dir, f'valid_{input_tag}.csv')

    #     """If overwrite input: remove all the previous input under input_tag"""
    #     if overwrite_input: self._clean_input()
        
    #     """1. Construct training and validation list"""
    #     """1.1 Load train_list.csv"""
    #     train_list_path = os.path.join(working_dir, "train_list.csv")
    #     self.case_dict_list = np.array(list(csv.DictReader(open(train_list_path,'r'))))
    #     case_index_list = np.arange(self.case_dict_list.shape[0])
    #     self.log.info('---------------------------------------------------------------')
    #     self.log.info('Check the available input')
    #     self.log.info('---------------------------------------------------------------')
    #     """Check the train idx and validatoin idx."""
    #     if (not os.path.exists(train_idx_path)) and (not os.path.exists(validation_idx_path)):
    #         self.log.debug('No input founded. Generate new input...')
    #         self.log.info('---------------------------------------------------------------')

    #         """1.2 Split given cases into training and validation set"""
    #         validation_size = self.config_model['training_info']['validation_size']
    #         try: 
    #             validation_ratio = float(validation_size.strip())
    #             train_idx, validation_idx = train_test_split(case_index_list, test_size = validation_ratio)
    #             n_train = len(train_idx)
    #             n_validation = len(validation_idx)
    #         except: 
    #             validation_ratio = 0
    #             self.log.critical(f'Can not distinguish the validation ratio from {validation_size}.')
    #             self.log.critical(f'Will processed without validation set.')
    #             train_idx = copy.copy(case_index_list)
    #             validation_idx = None
    #             n_train = len(train_idx)
    #             n_validation = 0
    #         self.log.info(f'Number of cases for training: {n_train}')
    #         self.log.info(f'Number of cases for validation: {n_validation}')
    #         self.log.info(f'Train index: {train_idx}')
    #         self.log.info(f'Validation index: {validation_idx}')

    #         """1.3. Calculating previoud node segmentations
    #         TODO: parallization
    #         """
    #         previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag)
    #         # previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag, self.node_name)
    #         self.previous_node_rois = previous_node_rois
    #         self._generating_previous_node_rois(case_index_list)
    #     else:
    #         self.log.debug(f'Available input founded from {hdf5_dir}. Load the input...')
    #         self.log.info('---------------------------------------------------------------')
    #         """1.1 Load training and validation list"""
    #         train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
    #         self.log.info(f'Train index: {train_idx}')
    #         validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
    #         self.log.info(f'Validation index: {validation_idx}')
    #         previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag)
    #         # previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag, self.node_name)
    #         self.previous_node_rois = previous_node_rois
        
    #     """2. Finish building training set"""
    #     self.log.info('---------------------------------------------------------------')
    #     self._build_training_set(train_idx, validation_idx)
        
    #     self.log.info('---------------------------------------------------------------')
    #     self.log.info(f'Success to build a training set with time {timeit.default_timer() - tic} s')
    #     self.log.info('---------------------------------------------------------------')
    #     return train_idx, validation_idx

    def build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False):
        """Build a training set
        TODO: multiprocessing
        """

        tic = timeit.default_timer()

        self.log.info('---------------------------------------------------------------')
        self.log.info('Buidling training set')
        self.log.info('---------------------------------------------------------------')
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        hdf5_dir = os.path.join(working_dir, 'input')
        os.makedirs(hdf5_dir, exist_ok=True)
        # train_idx_path = os.path.join(hdf5_dir, f'train_{input_tag}.csv')
        # validation_idx_path = os.path.join(hdf5_dir, f'valid_{input_tag}.csv')
        previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag)
        self.previous_node_rois = previous_node_rois

        """avoid input racing conditions"""
        input_hist_path = os.path.join(hdf5_dir, f'history_input_{input_tag}')
        
        """Construct training and validation list"""
        """Load train_list.csv"""
        train_list_path = os.path.join(working_dir, "train_list.csv")
        self.case_dict_list = np.array(list(csv.DictReader(open(train_list_path,'r'))))
        case_index_list = np.arange(self.case_dict_list.shape[0])

        process_id =  os.getpid()
        process_user = _get_owner(process_id)
        process_cmd = ' '.join(_get_original_cmd(process_id))

        """If overwrite input: remove all the previous input under input_tag"""
        if overwrite_input: self._clean_input()

        if os.path.exists(input_hist_path):
            pass
        else:
            summary_process_creation = f'[{process_user}|{process_id}|{datetime.now()}] Start creating input with tag {input_tag} with process_cmd:\n' + \
                    f'\t{process_cmd}\n'
            # hist_input_f.write(summary_process_creation)
            # hist_input_f.flush()
            print(summary_process_creation)
            
            """Split given cases into training and validation set"""
            validation_size = self.config_model['training_info']['validation_size']
            try: 
                validation_ratio = float(validation_size.strip())
                train_idx, validation_idx = train_test_split(case_index_list, test_size = validation_ratio)
                n_train = len(train_idx)
                n_validation = len(validation_idx)
            except: 
                validation_ratio = 0
                self.log.critical(f'Can not distinguish the validation ratio from {validation_size}.')
                self.log.critical(f'Will processed without validation set.')
                train_idx = copy.copy(case_index_list)
                validation_idx = None
                n_train = len(train_idx)
                n_validation = 0
            self.log.info(f'Number of cases for training: {n_train}')
            self.log.info(f'Train index: {train_idx}')
            self.log.info(f'Number of cases for validation: {n_validation}')
            self.log.info(f'Validation index: {validation_idx}')

            self.log.info('---------------------------------------------------------------')
            self._generating_previous_node_rois(case_index_list)

            self.log.info('---------------------------------------------------------------')
            self._build_training_set(train_idx, validation_idx)
            self.log.info('---------------------------------------------------------------')
            self.log.info(f'Success to build a training set with time {timeit.default_timer() - tic} s')
            self.log.info('---------------------------------------------------------------')

            """No previous input"""
            with open(input_hist_path, 'a') as hist_input_f:
                lock_file(hist_input_f)
                summary_process_creation = f'[{process_user}|{process_id}|{datetime.now()}] Done creating input with tag {input_tag}\n'
                hist_input_f.write(summary_process_creation)
                hist_input_f.flush()
                unlock_file(hist_input_f)
                self.log.info(summary_process_creation)
                sys.stdout.flush()
    
    def load_training_set(self, workers=1, use_multiprocessing=False):
        """Load a training set

        TODO: multiprocessing

        Returns
        -------
        train_idx : ndarray
        validation_idx : ndarray
        """
        
        tic = timeit.default_timer()

        self.log.info('---------------------------------------------------------------')
        self.log.info('Loading training set')
        self.log.info('---------------------------------------------------------------')
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        hdf5_dir = os.path.join(working_dir, 'input')
        # train_idx_path = os.path.join(hdf5_dir, f'train_{input_tag}.csv')
        # validation_idx_path = os.path.join(hdf5_dir, f'valid_{input_tag}.csv')
        previous_node_rois = os.path.join(hdf5_dir, 'previous_node_rois', input_tag)
        self.previous_node_rois = previous_node_rois

        """avoid input racing conditions"""
        input_hist_path = os.path.join(hdf5_dir, f'history_input_{input_tag}')
        
        """Construct training and validation list"""
        """Load train_list.csv"""
        train_list_path = os.path.join(working_dir, "train_list.csv")
        self.case_dict_list = np.array(list(csv.DictReader(open(train_list_path,'r'))))
        case_index_list = np.arange(self.case_dict_list.shape[0])

        process_id =  os.getpid()
        process_user = _get_owner(process_id)
        process_cmd = ' '.join(_get_original_cmd(process_id))
        status_update_input = False

        if os.path.exists(input_hist_path):
            """previously created input exists"""
            while True:
                try:
                    with open(input_hist_path, 'a') as hist_input_f:
                        lock_file(hist_input_f)
                        summary_process_loading = f'[{process_user}|{process_id}|{datetime.now()}] Start loading input with tag {input_tag}\n'
                        hist_input_f.write(summary_process_loading)
                        hist_input_f.flush()
                        self.log.debug(f'Available input founded from {hdf5_dir}. Load the input...')
                        self.log.info('---------------------------------------------------------------')
                        self.log.info(summary_process_loading)
                        sys.stdout.flush()
                        
                        train_idx, validation_idx = self._load_training_set()
                        self.log.info('---------------------------------------------------------------')
                        self.log.info(f'Success to load a training set with time {timeit.default_timer() - tic} s')
                        self.log.info('---------------------------------------------------------------')
                        sys.stdout.flush()
                        
                        summary_process_loading = f'[{process_user}|{process_id}|{datetime.now()}] Done loading input with tag {input_tag}\n'
                        hist_input_f.write(summary_process_loading)
                        hist_input_f.flush()
                        unlock_file(hist_input_f)
                        self.log.info(summary_process_loading)
                        sys.stdout.flush()
                    break
                except OSError or BlockingIOError as e:
                    if not status_update_input:
                        summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start waiting the locked input with tag {input_tag} with process_cmd:\n' + \
                                f'\t{process_cmd}\n\t{e}\n'
                        self.log.info(summary_process_start)
                        sys.stdout.flush()
                    status_update_input = True
                    time.sleep(self.check_frequency_s)
        return train_idx, validation_idx
    
    def get_mini_batch(self, idxs):
        """Return a mini-batch [x,y] containing given {idxs} cases for training

        Parameters
        ----------
        idxs: ndarray
            ndarray of index of the cases for construting mini-batch
        
        Returns
        -------
        tuple of ndarray
            mini-batch [x,y] for training
        """
        x = []; y = []
        for idx in idxs:
            try:
                inputs = self._get_processed_inputs(idx)
                x.append(inputs[0])
                y.append(inputs[1])
            except Exception as e:
                self.log.debug(f'Fail to load {idx}. Will ignore {idx}.\n Error:\n{traceback.format_exc()}\n')
                pass
        x = np.array(x); y = np.array(y)
        return (x,y)
    
    def get_predict_mini_batch(self, idxs):
        """Return a mini-batch [x] containing given {idxs} cases for testing

        Parameters
        ----------
        idx: int
            index of the mini-batch
        F
        Returns
        -------
        ndarray
            mini-batch [x] for training
        """
        x = []
        for idx in idxs:
            try:
                inputs = self._get_processed_inputs(idx)
                x.append(inputs[0])
            except Exception as e:
                self.log.debug(f'Fail to load {idx}. Will ignore {idx}.\n Error:\n{traceback.format_exc()}\n')
                pass
        x = np.array(x)
        return x

    def _get_processed_inputs(self, idx):
        """Return processed inputs from case {idx}
        Parameters
        ----------
        idx : int
            index of case to process the image
        
        Returns
        -------
        img_arr : ndarray
            preprocessed image from case {idx}
        roi_arr : ndarray
            processed roi from case {idx}
        """
        image_path = self.case_dict_list[idx]['image'].strip()
        try: ref_roi_path = self.case_dict_list[idx]['ref_roi'].strip()
        except: ref_roi_path = None
        if self.previous_node_rois:
            previous_node_roi_folder = os.path.join(self.previous_node_rois, str(idx)) # previous_node_roi_name = str(idx)
            # previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'{self.node_name}.roi')
            previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'search_area_{self.node_name}.roi')
        else:
            previous_seg_roi_file = None
        
        if self.use_ph_area:
            ph_area_file = os.path.join(os.path.dirname(previous_seg_roi_file), f'ph_area_{self.node_name}.roi')
        else:
            ph_area_file = None

        img, img_arr, roi, roi_arr, bb, ph_area_image_array, ph_area_mask_array = self._load(image_path, ref_roi_path, previous_seg_roi_file, ph_area_file)

        self.log.debug(f'Done loading images, rois, bounding boxe and ph_area information.')
        
        """Pre-processing images"""
        """1. resizing images"""
        target_shape = self.get_target_shape()
        # self.log.debug(f'Original image shape: {img_arr.shape}')
        # self.log.debug(f'Target image shape: {target_shape}')
        # self.log.debug(f'{np.min(roi_arr)}, {np.max(roi_arr)}')
        # self.log.debug(f'{np.sum(roi_arr==0)}, {np.sum(roi_arr==1)}')
        # self.log.debug(f'{np.sum(roi_arr==1)/np.sum(np.ones_like(roi_arr))}')
        if self.search_area_attention_type:
            img_arr = np.concatenate([np.expand_dims(self.resize(img_arr[...,j], target_shape), axis=-1) for j in range(img_arr.shape[-1])], axis=-1)
        else:
            img_arr = self.resize(img_arr, target_shape)
        # self.log.debug(f'Resized image shape: {img_arr.shape}')
        # self.log.debug(f'Original roi shape: {roi_arr.shape}')
        # self.log.debug(f'Target roi shape: {target_shape}')
        # self.log.debug(f'{np.min(roi_arr)}, {np.max(roi_arr)}')
        # self.log.debug(f'{np.sum(roi_arr==0)}, {np.sum(roi_arr==1)}')
        # self.log.debug(f'{np.sum(roi_arr==1)/np.sum(np.ones_like(roi_arr))}')
        # if ph_area_file: 
        #     self.log.debug(f'Ph_area image array shape: {ph_area_image_array.shape}')
        #     ph_area_image_array = self.resize(ph_area_image_array, target_shape)
        original_img = copy.copy(img_arr)
        if ph_area_mask_array is not None: 
            ph_area_mask_array = self.resize_roi(ph_area_mask_array, target_shape)
            ph_area_mask_array[ph_area_mask_array > 0.] = 1.
            ph_area_mask_array = ph_area_mask_array.astype(np.int32)
        
        """2. intensity normalization"""
        intensity_norm = self.get_intensity_norm()
        self.log.debug(f'len(intensity_norm) : {len(intensity_norm)}')
        img_arr = self.normalization(img_arr, intensity_norm, ph_area_image_array)
        img_arr = img_arr.astype(np.float32)
        
        """3. convert roi to make refernce"""
        if roi is not None:
            """
            TODO
            ----
            check...resizing make some issues..
            """
            roi_arr = self.resize_roi(roi_arr, target_shape)
            # self.log.debug(f'Resized roi shape: {roi_arr.shape}')
            # roi_arr = (roi_arr - np.min(roi_arr))/ (np.max(roi_arr) - np.min(roi_arr))
            # self.log.debug(f'{np.min(roi_arr)}, {np.max(roi_arr)}')
            # self.log.debug(f'{np.sum(roi_arr==0)}, {np.sum(roi_arr==1)}')
            # # self.log.debug(f'{np.sum(roi_arr==1)/np.sum(np.ones_like(roi_arr))}')
            roi_arr[roi_arr > 0.] = 1.
            # roi_arr[roi_arr >= 0.5] = 1.
            # roi_arr[roi_arr < 0.5] = 0.
            # self.log.debug(f'{np.min(roi_arr)}, {np.max(roi_arr)}')
            # self.log.debug(f'{np.sum(roi_arr==0)}, {np.sum(roi_arr==1)}')
            # self.log.debug(f'{np.sum(roi_arr==1)/np.sum(np.ones_like(roi_arr))}')
                
            """TODO: update to use keras.to_cagegorical"""
            roi_arr = np.expand_dims(roi_arr, axis=-1) # for binary class segmentation
            roi_arr = roi_arr.astype(np.int32)

        """4. draw png of normalized input image for review purpose"""
        not_skip_anyway = (not self.png_skip)
        if self.mode == 'train':
            not_skip_in_training = (not self.png_skip_training)
        else:
            not_skip_in_training = ((not self.png_skip_training) or (not self.training_child))
        if not_skip_anyway and not_skip_in_training:
            if img_arr.shape[0] > 20: c=3
            elif img_arr.shape[0] > 50: c=2
            elif img_arr.shape[0] > 100: c=1
            else: c = 4
            if self.mode == 'train':
                if self.previous_node_rois is not None:
                    review_fig_dir = os.path.join(self.previous_node_rois, str(idx)) # previous_node_roi_name = str(idx)
                else:
                    raise ValueError('no place to save png. need update the scripts.')
            # review_fig_dir = os.path.join(config_model['path_info']['working_directory'].strip(), 
            #                 'input', 'png',
            #                 config_model['chromosome_info']['input_tag'])
            else:
                review_fig_dir = self.config_model['path_info']['output_directory'].strip()
            self.draw_fig(review_fig_dir, img_arr, intensity_norm, roi_arr=roi_arr, ph_area_mask_array=ph_area_mask_array, original_img=original_img, c=c, png_overwrite=self.png_overwrite)
        return img_arr, roi_arr
        
    def _load(self, image_path, roi_path=None, previous_seg_roi_file=None, ph_area_file=None):
        """Private function for loading single image (and roi)

        Parameters
        ----------
        image_path : str
            path of the image to load
        roi_path : str (default = None)
            path of the roi to load
            if None, only image will be loaded.
        previous_seg_roi_file : str
            path of the roi output file of previous miu node
        Returns
        -------
        image : qia.common.img.image object
            loaded image
        image_array : ndarray
            numpy array of loaded image, croped based on bounding box configuration.
            crop area (points) will be controled by MIU. Without any miu node, 
            it will use the whole image area.
        roi : qia.common.img.image object
            loaded roi. If roi_path is not given, return None
        roi_array : ndarray
            numpy array of loaded roi, croped based on bounding box configuration.
            If roi_path is not given, return None
            crop area (points) will be controled by MIU. Without any miu node, 
            it will use the whole image area.
        bb : list of int
            bounding box information from segmentations of previous node and zitter
        """
        
        try:
            """bounding box"""
            if previous_seg_roi_file:
                try:
                    assert (os.path.exists(previous_seg_roi_file))
                    # self.log.debug(f'Search area for prediction: {previous_seg_roi_file}')
                except:
                    """TODO:temp detour for prediction. (go up one directory) Later, need for revising"""
                    previous_seg_roi_file_dir = os.path.dirname(os.path.dirname(previous_seg_roi_file))
                    previous_seg_roi_file_name = os.path.basename(previous_seg_roi_file)
                    previous_seg_roi_file = os.path.join(previous_seg_roi_file_dir, previous_seg_roi_file_name)
                    # self.log.debug(f'Search area for prediction: {previous_seg_roi_file}')
                    assert (os.path.exists(previous_seg_roi_file))
                    
                previous_seg_roi = qimage.read(previous_seg_roi_file)
                # previous_seg_roi = previous_seg_roi.get_alias(min_point=(0, 0, 0))
                minpoint, maxpoint = previous_seg_roi.find_region(1,1)
                tlx, tly, tlz = minpoint[0], minpoint[1], minpoint[2]
                brx, bry, brz =  maxpoint[0], maxpoint[1], maxpoint[2]
                bb = [tlx, tly, tlz, brx, bry, brz]
            else:
                bb = copy.copy(self.bb)

            """TODO: roll-back to assume seri for dicom series only?"""
            try:
                image = qimage.read(image_path) #our inhouse image object format
            except Exception as e:
                if 'seri' in image_path.split('.')[-1]:
                    with open(image_path) as f:
                        img_path_list = f.readlines()
                        image = qimage.read(img_path_list[0])
                        # try:
                            # self.image = qimage.read(img_path_list[0])
                        # except:
                            # dcm = pydicom.read_file(img_path_list[0])
                else:
                    traceback.print_exc()
                    raise ValueError(f'Image loading failed for {image_path} with exception {e}')        
            
            if roi_path is not None:
                if 'roi' in roi_path.split('.')[-1]:
                    roi = qimage.cast(image)
                    roi.fill_with_roi(roi_path)
                else:
                    roi = qimage.read(roi_path)
            else:
                roi = None
                roi_array = None
                            
            """Centering, formatting"""
            image = image.get_alias(min_point=(0, 0, 0))
            image_array = image.get_array()
            image_array = image_array.astype(np.float)
            original_img_shape = image_array.shape
            if roi_path is not None:
                roi = roi.get_alias(min_point=(0, 0, 0))
                roi_array = roi.get_array()
                roi_array = roi_array.astype(np.float32)
                if 'roi' not in roi_path.split('.')[-1]:
                    """not inhouse-reference roi."""
                    if not (len(np.unique(roi_array))==1 and np.unique(roi_array)[0]==0):
                        roi_array = (roi_array - np.min(roi_array))/ (np.max(roi_array) - np.min(roi_array))
                    roi_array[roi_array >= 0.5] = 1.
                    roi_array[roi_array < 0.5] = 0.

            """TODO:check image orientation for 3D image input"""
            # if len(self.get_target_shape()) > 2:
            #     """Note: for 3D images open with .seri file
            #     @Youngwon
            #     flip on z-axis bases on
            #     https://gitlab.cvib.ucla.edu/qia/pcl/-/blob/master/include/pcl/image_io/DicomImageSeriesReader.h#L91
            #     """
            #     image_orientation = image.get_orientation()
            #     image_orientation_rounded = np.round(image_orientation, 1)
            #     if image_orientation_rounded[-1] < 0:
            #         self.log.debug(f'orientation: {image_orientation}')
            #         self.log.debug(f'orientation (rounded): {image_orientation_rounded}')
            #         image_array = image_array[::-1]
            #         self.log.debug(f'Image is flipped on z-axis.')
            #         if roi_path is not None:
            #             roi_array = roi_array[::-1]
            #             self.log.debug(f'ROI is flipped on z-axis.')

            """bounding box"""
            if self.search_area_attention_type is not None and not self.search_area_bounding_box:
                tlx, tly, tlz, brx, bry, brz = 0, 0, 0, original_img_shape[2]-1, original_img_shape[1]-1, original_img_shape[0]-1
            else:
                tlx, tly, tlz, brx, bry, brz = bb

            """ph area image bounding box"""
            if ph_area_file is not None:    
                self.log.debug(f'The ph area should available in this case. Checking {ph_area_file} ...')
                try:
                    assert (os.path.exists(ph_area_file))
                except:
                    """TODO:temp detour for prediction. (go up one directory) Later, need for revising"""
                    ph_area_file_dir = os.path.dirname(os.path.dirname(ph_area_file))
                    ph_area_file_name = os.path.basename(ph_area_file)
                    ph_area_file = os.path.join(ph_area_file_dir, ph_area_file_name)
                    # self.log.debug(f'ph area for prediction: {previous_seg_roi_file}')
                    try:
                        assert (os.path.exists(ph_area_file))
                    except:
                        self.log.debug(f'Missing the ph area {ph_area_file} for normalizing iamge {image_path}.')
                        ph_area_file = None
            if ph_area_file is not None:
                ph_area_roi = qimage.read(ph_area_file)
                self.log.debug(f'The ph area {ph_area_file} will be used for normalizing iamge {image_path}.')
                # if 'roi' in ph_area_file.split('.')[-1]:
                #     ph_area_roi = qimage.cast(image)
                #     ph_area_roi.fill_with_roi(ph_area_file)
                # else:
                #     ph_area_roi = qimage.read(ph_area_file)
                ph_area_image_array = copy.copy(image_array)
                ph_minpoint, ph_maxpoint = ph_area_roi.find_region(1,1)
                ph_minpoint = np.max(list(zip(minpoint, ph_minpoint)), axis=1)
                ph_maxpoint = np.min(list(zip(maxpoint, ph_maxpoint)), axis=1)
                ph_tlx, ph_tly, ph_tlz = ph_minpoint[0], ph_minpoint[1], ph_minpoint[2]
                ph_brx, ph_bry, ph_brz =  ph_maxpoint[0], ph_maxpoint[1], ph_maxpoint[2]  
                self.log.debug(f'The ph area bb: {ph_tlx, ph_tly, ph_tlz, ph_brx, ph_bry, ph_brz}.')

                # ph_mask = ph_area_array <= 0
                # ph_area_image_conv = np.ma.masked_array(image_array, mask=ph_mask)
                # self.log.debug(f'The image shape after masking ph_area: {ph_area_image_conv.shape}.')
                if len(self.get_target_shape()) > 2:
                    """3D input """
                    ph_area_image_array = ph_area_image_array[ph_tlz:(ph_brz+1), ph_tly:(ph_bry+1), ph_tlx:(ph_brx+1)]
                else:
                    """2D input. TODO: is 3D input way working?"""
                    ph_area_image_array = ph_area_image_array[0, ph_tly:(ph_bry+1), ph_tlx:(ph_brx+1)]
                self.log.debug(f'The image shape after cropping with ph area: {ph_area_image_array.shape}.')
                
                ph_area_mask_roi = qimage.cast(image)
                ph_area_mask_roi.fill_with_roi(ph_area_file)
                ph_area_mask_roi = ph_area_mask_roi.get_alias(min_point=(0, 0, 0))
                ph_area_mask_array_raw = ph_area_mask_roi.get_array()
                ph_area_mask_array = copy.copy(ph_area_mask_array_raw) # prevent the segmentation fault error outside this ftn (pcl qimage object seems pointing same location with image or something?)
            else:
                ph_area_image_array = None
                ph_area_mask_array = None
                
            """crop
            TODO: check if 3d implementation also work for 2d
            """
            if len(self.get_target_shape()) > 2:
                """3D input"""
                image_array = image_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
                if roi_path: roi_array = roi_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
                # if ph_area_file: ph_area_array = ph_area_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
            else:
                """2D input"""
                image_array = image_array[0, tly:(bry+1), tlx:(brx+1)]
                if roi_path: roi_array = roi_array[0, tly:(bry+1), tlx:(brx+1)]
                # if ph_area_file: ph_area_array = ph_area_array[0, tly:(bry+1), tlx:(brx+1)]
            self.log.debug(f'The image shape after cropping with search area: {image_array.shape}.')
            

            """Search area attention"""
            if self.search_area_attention_type is not None:
                try:
                    assert (os.path.exists(previous_seg_roi_file))
                    search_area_roi = qimage.cast(image)
                    search_area_roi.fill_with_roi(previous_seg_roi_file)
                    """Centering, formatting"""
                    search_area_roi = search_area_roi.get_alias(min_point=(0, 0, 0))
                    search_area_roi_array = search_area_roi.get_array()
                    """TODO:check image orientation for 3D image input"""
                    if len(self.get_target_shape()) > 2:
                        """3D input"""
                        search_area_roi_array = search_area_roi_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
                    else:
                        """2D input"""
                        search_area_roi_array = search_area_roi_array[0, tly:(bry+1), tlx:(brx+1)]
                except:
                    self.log.debug(f'Fail to load search area from {previous_seg_roi_file}. Use whole area as a search area. \n Error:\n{traceback.format_exc()}\n')
                    search_area_roi_array = np.ones_like(image_array)
                # self.log.debug(f'image_array: {image_array.shape}')
                # self.log.debug(f'search_area_roi_array: {search_area_roi_array.shape}')
                image_array = np.concatenate([np.expand_dims(image_array, axis=-1), np.expand_dims(search_area_roi_array, axis=-1)], axis=-1)
                # self.log.debug(f'image_array: {image_array.shape}')

            return image, image_array, roi, roi_array, bb, ph_area_image_array, ph_area_mask_array

        except Exception as e:
            traceback.print_exc()
            raise ValueError(f'Image loading failed for {image_path} with exception {e}')

class png_reader(base_reader):
    """
    Image reader class for loading png images and masks.
    For now, only used by bin/tutorial/hello_miu
    """
    def __init__(self, config_model, config_resource, log):
        super(png_reader,self).__init__(config_model, config_resource, log)
        
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        data_csv_path = os.path.join(working_dir, "data.csv")
        if not os.path.exists(data_csv_path):
            raise ValueError(f"expecting data.csv to be at location: {data_csv_path}")
        '''
        expected cvs content:
        kind,image_path,mask_path
        train,myimage000.png,mymask000.png
        validation,myimage001.png,mymask001.png
        test,myimage002.png,mymask002.png
        '''
        self.df = pd.read_csv(data_csv_path)

    def build_training_set(self, overwrite_input=False):
        train_idx = np.asarray(self.df[self.df.kind=='train'].index)
        validation_idx = np.asarray(self.df[self.df.kind=='validation'].index)
        return train_idx, validation_idx

    def get_mini_batch(self, idxs):
        x = []; y = []
        for idx in idxs:
            inputs = self._get_processed_inputs(idx)
            x.append(inputs[0])
            y.append(inputs[1])
        x = np.array(x)
        y = np.array(y)
        return (x,y)

    def get_predict_mini_batch(self, idxs):
        x, y = self.get_mini_batch(idxs)
        return x # why test batch return only x? and not y?

    def _get_processed_inputs(self, idx):
        
        target_shape = self.get_target_shape()
        image_path = self.df.at[idx,'image_path']
        mask_path = self.df.at[idx,'mask_path']
        
        img_arr = np.array(imageio.imread(image_path)).astype(np.float)
        img_arr = self.resize(img_arr, target_shape)

        roi_arr = np.array(imageio.imread(mask_path).astype(np.float))
        roi_arr = self.resize_roi(roi_arr, target_shape)

        """intensity normalization"""
        intensity_norm = self.get_intensity_norm()
        # self.log.debug(f'len(intensity_norm) : {len(intensity_norm)}')
        img_arr = self.normalization(img_arr, intensity_norm)
        img_arr = img_arr.astype(np.float32)
        
        """roi processing"""
        roi_arr[roi_arr > 0] = 1
        roi_arr = np.expand_dims(roi_arr, axis=-1)

        return img_arr, roi_arr
