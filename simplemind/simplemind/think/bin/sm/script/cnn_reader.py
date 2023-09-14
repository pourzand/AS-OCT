"""Reader subclasses family for specific tasks including CXR, MRI


Spec
----
CXRImageReader

TODO 
----
@youngwonchoi
    1. CXRImageReader: test
    1. MRIImageReader (needed?)
"""

import os
import numpy as np
import pandas as pd
import traceback

# from sklearn.preprocessing import scale
# from skimage import exposure, img_as_float
from skimage.transform import resize as skresize
import shutil

import os, sys

import pydicom

import __qia__
import qia.common.img.image as qimage
import qia.common.dicom.obj as qdicom
from cnn_reader_base import *
import cnntools as cnntls
from cnn_configurator import Configurator

from skimage.segmentation import mark_boundaries

def save_idx_npy(i_idx, npy_train_dir_path, _get_processed_inputs):
    i, idx = i_idx
    npy_train_img_path = os.path.join(npy_train_dir_path, f'img_{idx}.npy')
    npy_train_roi_path = os.path.join(npy_train_dir_path, f'roi_{idx}.npy')
    if (not os.path.exists(npy_train_img_path)) and (not os.path.exists(npy_train_roi_path)):
        img_arr, roi_arr = _get_processed_inputs(idx)
        np.save(npy_train_img_path, img_arr)
        np.save(npy_train_roi_path, roi_arr)
        print(f'{i}: sample {idx} was written. Final image array shape: {img_arr.shape}')

def load_idx_npy(i_idx, npy_train_dir_path):
    i, idx = i_idx
    npy_train_img_path = os.path.join(npy_train_dir_path, f'img_{idx}.npy')
    npy_train_roi_path = os.path.join(npy_train_dir_path, f'roi_{idx}.npy')
    img_arr = np.load(npy_train_img_path)
    roi_arr = np.load(npy_train_roi_path)
    # print(f'{i}: {img_arr.shape}, {roi_arr.shape}')
    return [img_arr, roi_arr]

class simple_image_reader(image_reader):
    """Basic Image reader class for loading image formats in CVIB.
    This class supports most of the image formats (2D/3D; 
    single/multi-channel) in CVIB.
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    It assume roi_reference is given. For dealing with x,y coordinates,
    use cxr_image_reader.
    We recommand you to use this reader only for small size of dataset.
    For larger dataset, we recommand you to use image_reader instead.

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
        whether to do augmentation on mini-batch for traiing
    self.previous_node_rois : str
        base directory to save rois from previous miu nodes
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False)
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
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
        super(simple_image_reader,self).__init__(config_model, config_resource, log, mode)
        self._processed_set = None
    
    def _build_npy(self, train_idx, npy_train_dir_path):
        os.makedirs(npy_train_dir_path, exist_ok=True)
        target_shape = self.get_target_shape()
        x_shape = copy.copy(target_shape)
        if self.search_area_attention_type is not None:
            x_shape.append(self.get_image_channels() + 1)
        else:
            x_shape.append(self.get_image_channels())
        
        if 'ref_roi' not in self.case_dict_list[0].keys():
            self.log.debug('No reference ROI is detected from the train_list.csv. y has shape (2)')
            y_shape = [2]
        else:
            y_shape = copy.copy(target_shape)
            y_shape.append(1)
        self.log.info(f'Build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')

        if 'ref_roi' not in self.case_dict_list[0].keys():
            self.log.debug('No reference ROI is detected from the train_list.csv. Assume the reference has a formmat of (x,y) point.')
            _get_processed_inputs = self._get_processed_inputs_without_roi
        else:
            _get_processed_inputs = self._get_processed_inputs
        try:
            self.log.info(f'Start writing npy input files under {npy_train_dir_path}')

            if self.use_multiprocessing:
                self.log.debug(f'Start computing npy with {self.workers} number of CPU cores.')
                pool = Pool(processes=self.workers)
                result = pool.map_async(partial(save_idx_npy,
                                                npy_train_dir_path= npy_train_dir_path, 
                                                _get_processed_inputs = _get_processed_inputs), 
                                        enumerate(train_idx)).get()
                pool.close()
                pool.join()
                self.log.debug(f'Finished computing npy with {self.workers} number of CPU cores.')
            else:
                self.log.debug(f'Start computing npy with a single CPU core.')
                result=[]
                for i, idx in enumerate(train_idx):
                    result.append(save_idx_npy((i,idx), npy_train_dir_path, _get_processed_inputs))
                self.log.debug(f'Finished computing npy using a single CPU core.')
            self.log.info(f'Writing finished: {npy_train_dir_path}')
        except Exception as e:
            raise ValueError(f'input generation failed with exception: {e}.\n Error:\n{traceback.format_exc()}\n')

    def _build_hdf5(self, train_idx, hdf5_train_path):
        """TODO: temp way not to overwrite hdf5 again. 
        think about better way later
        """
        if not os.path.exists(hdf5_train_path):
            target_shape = self.get_target_shape()
            x_shape = copy.copy(target_shape)
            y_shape = copy.copy(target_shape)
            """TODO: current version is for 
            single search area attention only"""
            if self.search_area_attention_type is not None:
                x_shape.append(self.get_image_channels() + 1)
            else:
                x_shape.append(self.get_image_channels())
            """TODO: current version is for binary only."""
            y_shape.append(1)
            self.log.info(f'Build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')
            labels = (self.config_model['model_info']['node_name'].split('_')[0],)
            try:
                self.log.info(f'Start writing hdf5 input files: {hdf5_train_path}')
                self.log.debug(f'Information for writer: xdims={x_shape}, ydims={y_shape}, labels={labels}')
                with Writer(hdf5_train_path, x_shape, labels, ydims=y_shape, overwrite=True) as writer:
                    for i, idx in enumerate(train_idx):
                        img_arr, roi_arr = self._get_processed_inputs(idx)
                        writer.add(img_arr, roi_arr)
                        self.log.debug(f'{i}: sample {idx} was written. Final image array shape: {img_arr.shape}')
                self.log.info(f'Writing finished: {hdf5_train_path}')
            except Exception as e:
                raise ValueError(f'input generation failed with exception: {e}.\n Error:\n{traceback.format_exc()}\n')
    
    def _build_training_set(self, train_idx, validation_idx):
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        input_dir = os.path.join(working_dir, 'input')
        os.makedirs(input_dir, exist_ok=True)
        train_idx_path = os.path.join(input_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(input_dir, f'valid_{input_tag}.csv')
        # hdf5_train_path = os.path.join(input_dir, f'train_{input_tag}.hdf5')
        # hdf5_validation_path = os.path.join(input_dir, f'valid_{input_tag}.hdf5')
        npy_train_dir_path = os.path.join(input_dir, f'train_{input_tag}_npy')
        npy_validation_dir_path = os.path.join(input_dir, f'valid_{input_tag}_npy')

        """Generate train and validation hdf5 files, index csv files"""
        # self._build_hdf5(train_idx, hdf5_train_path)
        self._build_npy(train_idx, npy_train_dir_path)
        self.log.info(f'Build training set from {len(train_idx)} cases under {npy_train_dir_path}')
        self._build_npy(validation_idx, npy_validation_dir_path)
        self.log.info(f'Build validation set from {len(validation_idx)} cases under {npy_validation_dir_path}')

        """Save train and validation idx"""
        np.savetxt(train_idx_path, train_idx, delimiter=",")
        self.log.info(f'Writing train index finished: {train_idx_path}')
        np.savetxt(validation_idx_path, validation_idx, delimiter=",")
        self.log.info(f'Writing validation index finished: {validation_idx_path}')
    
    def _load_training_set(self):
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        input_dir = os.path.join(working_dir, 'input')
        train_idx_path = os.path.join(input_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(input_dir, f'valid_{input_tag}.csv')
        # hdf5_train_path = os.path.join(input_dir, f'train_{input_tag}.hdf5')
        # hdf5_validation_path = os.path.join(input_dir, f'valid_{input_tag}.hdf5')
        npy_train_dir_path = os.path.join(input_dir, f'train_{input_tag}_npy')
        npy_validation_dir_path = os.path.join(input_dir, f'valid_{input_tag}_npy')

        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        """Load training and validation list"""
        train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Train index: {train_idx}')
        validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Validation index: {validation_idx}')
        
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        
        self.log.info(f'Start loading npy input files under {npy_train_dir_path}')
        self.log.info(f'Train index: {train_idx}')
        sys.stdout.flush()

        if self.use_multiprocessing:
            self.log.debug(f'Start computing npy with {self.workers} number of CPU cores.')
            pool = Pool(processes=self.workers)
            result = pool.map_async(partial(load_idx_npy,
                                            npy_train_dir_path= npy_train_dir_path), 
                                    enumerate(train_idx)).get()
            pool.close()
            pool.join()
            self.log.debug(f'Finished computing npy with {self.workers} number of CPU cores.')
        else:
            self.log.debug(f'Start computing npy with a single CPU core.')
            result=[]
            for i, idx in enumerate(train_idx):
                result.append(load_idx_npy((i,idx), npy_train_dir_path))
            self.log.debug(f'Finished computing npy using a single CPU core.')
        self.log.info(f'Writing finished: {npy_train_dir_path}')

        train_set = {}
        train_set['X'] = np.array([case[0] for case in result])
        train_set['Y'] = np.array([case[1] for case in result])
        x_shape = train_set['X'].shape
        y_shape = train_set['Y'].shape
        self.log.info(f'Finised loading npy input files under {npy_train_dir_path}')
        self.log.info(f'Training set: x={x_shape}, y={y_shape}')
        
        np.savetxt(train_idx_path, train_idx, delimiter=",")
        sys.stdout.flush()

        if (validation_idx is not None):
            self.log.info(f'Start loading npy input files under {npy_validation_dir_path}')
            self.log.info(f'Validation index: {validation_idx}')
            sys.stdout.flush()

            if self.use_multiprocessing:
                self.log.debug(f'Start computing npy with {self.workers} number of CPU cores.')
                pool = Pool(processes=self.workers)
                val_result = pool.map_async(partial(load_idx_npy,
                                                npy_train_dir_path= npy_validation_dir_path), 
                                        enumerate(validation_idx)).get()
                pool.close()
                pool.join()
                self.log.debug(f'Finished computing npy with {self.workers} number of CPU cores.')
            else:
                self.log.debug(f'Start computing npy with a single CPU core.')
                val_result=[]
                for i, idx in enumerate(validation_idx):
                    val_result.append(load_idx_npy((i,idx), npy_validation_dir_path))
                self.log.debug(f'Finished computing npy using a single CPU core.')
            self.log.info(f'Loading finished: {npy_validation_dir_path}')

            validation_set = {}
            validation_set['X'] = np.array([case[0] for case in val_result])
            validation_set['Y'] = np.array([case[1] for case in val_result])
            x_shape = validation_set['X'].shape
            y_shape = validation_set['Y'].shape
            self.log.info(f'Finised loading npy input files under {npy_validation_dir_path}')
            self.log.info(f'Validationing set: x={x_shape}, y={y_shape}')
            np.savetxt(validation_idx_path, validation_idx, delimiter=",")
            sys.stdout.flush()

        """Load processed inputs (image, roi) from hdf5"""
        # self.log.info(f'Start loading hdf5 input files: {hdf5_train_path}')
        # train_set = h5py.File(hdf5_train_path, 'r')
        # # train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
        # self.log.info(f'Train index: {train_idx}')
        # x_shape = train_set['X'].shape
        # y_shape = train_set['Y'].shape
        # self.log.info(f'Finised loading hdf5 input files: {hdf5_train_path}')
        # self.log.info(f'Training set: x={x_shape}, y={y_shape}')

        # # if (validation_idx is not None): 
        # if os.path.exists(hdf5_validation_path): 
        #     self.log.info(f'Start loading hdf5 input files: {hdf5_validation_path}')
        #     validation_set = h5py.File(hdf5_validation_path, 'r')
        #     # validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
        #     self.log.info(f'Validation index: {validation_idx}')
        #     x_shape = validation_set['X'].shape
        #     y_shape = validation_set['Y'].shape
        #     self.log.info(f'Finised loading hdf5 input files: {hdf5_validation_path}')
        #     self.log.info(f'Validation set: x={x_shape}, y={y_shape}')
        #     np.savetxt(validation_idx_path, validation_idx, delimiter=",")
        #     self.log.info(f'Writing validation index finished: {validation_idx_path}')
        
        """Build a private _processed_set
        TODO: warning! this is only for binary roi"""
        total_x = np.concatenate([np.array(train_set['X']), np.array(validation_set['X'])])
        total_y = np.concatenate([np.array(train_set['Y']), np.array(validation_set['Y'])])
        total_idx = np.concatenate([train_idx, validation_idx])
        total_x = total_x[total_idx].astype(np.float32)
        total_y = total_y[total_idx].astype(np.int32)
        self._processed_set = {'X':total_x, 'Y':total_y}
        self.log.debug(self._processed_set['X'].shape)
        self.log.debug(self._processed_set['Y'].shape)
        self.log.info(f'Finish build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')
        self.log.info(f'Finish build validation set from {len(validation_idx)} idxs: x={x_shape}, y={y_shape}')
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        return train_idx, validation_idx

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
        revise with build_trainig_set. so ugly..
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
        
        total_x = []
        total_y = []
        for idx in range(len(self.case_dict_list)):
            img_arr, roi_arr = self._get_processed_inputs(idx)
            total_x.append(img_arr)
            total_y.append(roi_arr)
        total_x = np.array(total_x)
        total_y = np.array(total_y)
        self._processed_set = {'X':total_x, 'Y':total_y}
    
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
        x = self._processed_set['X'][idxs]
        y = self._processed_set['Y'][idxs]
        return (x,y)
    
    def get_predict_mini_batch(self, idxs):
        """Return a mini-batch [x] containing given {idxs} cases for testing

        Parameters
        ----------
        idx: int
            index of the mini-batch
        
        Returns
        -------
        ndarray
            mini-batch [x] for training
        """
        x = self._processed_set['X'][idxs]
        return x

class cxr_image_reader(simple_image_reader):
    """CXR Image Reader class for loading CXR images with .seri formats (2D; 
    single-channel) in CVIB. This class supports the specific CXR tasks.
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    When the roi_reference is given, it will process the reference data
    for segmentation. If not, it will deal with x,y coordinates.
    
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
        whether to do augmentation on mini-batch for traiing
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self._task_type: int
        private
        0: segmentation with given roi
        1: segmentation without given roi
        TODO : using this
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False)
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
        write roi instnace from qia.common.image based on prediction probability
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
        Private function returning qia.common.img.image and numpy array of
        loaded image (and roi) from image_path (and roi_path)
    _load_search_area(self, image_path, previous_seg_roi_file)
        Private function for loading search area from previos segmentation roi
    _get_image_batch(self, idxs)
        Privait function returning one image batch containing given {idxs} cases.
    _get_reference_batch(self, idxs)
        Private function returning one reference batch containing given {idxs} cases.
    _get_processed_inputs_with_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is given.
    _get_processed_inputs_without_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is not given.

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
            - .seri: in-house seri file listing DICOM image paths 
    
    Supporting image shapes: 2D

    Supporting image channels: single-channel
    
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
        super(cxr_image_reader, self).__init__(config_model, config_resource, log, mode)

    # def _build_hdf5(self, train_idx, hdf5_train_path):
    #     if 'ref_roi' not in self.case_dict_list[0].keys():
    #         self.log.debug('No reference ROI is detected from the train_list.csv. Assume the reference has a formmat of (x,y) point.')
    #         self._get_processed_inputs = self._get_processed_inputs_without_roi
    #         self._build_hdf5_without_roi(train_idx, hdf5_train_path)
    #     else:
    #         super(cxr_image_reader, self)._build_hdf5(train_idx, hdf5_train_path)
    
    # def _build_hdf5_without_roi(self, train_idx, hdf5_train_path):
    #     """TODO: temp way not to overwrite hdf5 again. 
    #     think about better way later
    #     """
    #     if not os.path.exists(hdf5_train_path):
    #         target_shape = self.get_target_shape()
    #         x_shape = copy.copy(target_shape)
    #         y_shape = copy.copy([2])
    #         """TODO: current version is for 
    #         single search area attention only"""
    #         if self.search_area_attention_type is not None:
    #             x_shape.append(self.get_image_channels() + 1)
    #         else:
    #             x_shape.append(self.get_image_channels())
    #         # """TODO: current version is for binary only."""
    #         # y_shape.append(1)
    #         self.log.info(f'Build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')
    #         labels = (self.config_model['model_info']['node_name'].split('_')[0],)
    #         try:
    #             self.log.info(f'Start writing hdf5 input files: {hdf5_train_path}')
    #             self.log.debug(f'Information for writer: xdims={x_shape}, ydims={y_shape}, labels={labels}')
    #             with Writer(hdf5_train_path, x_shape, labels, ydims=y_shape, overwrite=True) as writer:
    #                 for i, idx in enumerate(train_idx):
    #                     img_arr, roi_arr = self._get_processed_inputs(idx)
    #                     writer.add(img_arr, roi_arr)
    #                     self.log.debug(f'{i}: sample {idx} was written. Final image array shape: {img_arr.shape}')
    #             self.log.info(f'Writing finished: {hdf5_train_path}')
    #         except Exception as e:
    #             """TODO: removing the failed input"""
    #             raise ValueError(f'input generation failed with exception: {e}.\n Error:\n{traceback.format_exc()}\n')

    # def _build_npy_without_roi(self, train_idx, hdf5_train_path):
    #     """TODO: temp way not to overwrite hdf5 again. 
    #     think about better way later
    #     """
    #     if not os.path.exists(hdf5_train_path):
    #         target_shape = self.get_target_shape()
    #         x_shape = copy.copy(target_shape)
    #         y_shape = copy.copy([2])
    #         """TODO: current version is for 
    #         single search area attention only"""
    #         if self.search_area_attention_type is not None:
    #             x_shape.append(self.get_image_channels() + 1)
    #         else:
    #             x_shape.append(self.get_image_channels())
    #         # """TODO: current version is for binary only."""
    #         # y_shape.append(1)
    #         self.log.info(f'Build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')
    #         labels = (self.config_model['model_info']['node_name'].split('_')[0],)
    #         try:
    #             self.log.info(f'Start writing hdf5 input files: {hdf5_train_path}')
    #             self.log.debug(f'Information for writer: xdims={x_shape}, ydims={y_shape}, labels={labels}')
    #             with Writer(hdf5_train_path, x_shape, labels, ydims=y_shape, overwrite=True) as writer:
    #                 for i, idx in enumerate(train_idx):
    #                     img_arr, roi_arr = self._get_processed_inputs(idx)
    #                     writer.add(img_arr, roi_arr)
    #                     self.log.debug(f'{i}: sample {idx} was written. Final image array shape: {img_arr.shape}')
    #             self.log.info(f'Writing finished: {hdf5_train_path}')
    #         except Exception as e:
    #             """TODO: removing the failed input"""
    #             raise ValueError(f'input generation failed with exception: {e}.\n Error:\n{traceback.format_exc()}\n')

    def _get_processed_inputs_without_roi(self, idx): #, miu_temp_path=None):
        """Return processed inputs from case {idx} when reference roi 
        is not given.
        Parameters
        ----------
        idx : int
            index of case to process the image
        miu_temp_path : str
            same the tmp output of lung segmentation

        Returns
        -------
        img_arr : ndarray
            preprocessed image from case {idx}
        roi_arr : ndarray
            processed roi from case {idx}
        """
        if self.previous_node_rois is not None:
            previous_node_roi_folder = os.path.join(self.previous_node_rois, str(idx)) # previous_node_roi_name = str(idx)
            previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'search_area_{self.node_name}.roi')
        else:
            previous_seg_roi_file = None

        image_path = self.case_dict_list[idx]['image']
        pr = int(self.case_dict_list[idx]['y'])
        pc = int(self.case_dict_list[idx]['x'])
        self.log.debug(f'pr = {pr}, pc = {pc}')
        self.log.debug(f'{image_path}, {previous_seg_roi_file}')

        if self.use_ph_area:
            ph_area_file = os.path.join(os.path.dirname(previous_seg_roi_file), f'ph_area_{self.node_name}.roi')
        else:
            ph_area_file = None

        img, img_arr, _, _, bb, ph_area_image_array, ph_area_mask_array = self._load(image_path, None, None, previous_seg_roi_file, ph_area_file) #, idx)
        
        """Pre-processing images"""
        """1. resizing images"""
        target_shape = self.get_target_shape()
        if self.search_area_attention_type is not None:
            img_arr = np.concatenate([np.expand_dims(self.resize(img_arr[...,j], target_shape), axis=-1) for j in range(img_arr.shape[-1])], axis=-1)
        else:
            img_arr = self.resize(img_arr, target_shape)
        # if ph_area_file: ph_area_image_array = self.resize(ph_area_image_array, target_shape)
        original_img = copy.copy(img_arr)
        if ph_area_mask_array is not None:
            ph_area_mask_array = self.resize_roi(ph_area_mask_array, target_shape)
            ph_area_mask_array[ph_area_mask_array > 0.] = 1.
            ph_area_mask_array = ph_area_mask_array.astype(np.int32)

        """2 intensity normalization"""
        intensity_norm = self.get_intensity_norm()
        # self.log.debug(f'len(intensity_norm) : {len(intensity_norm)}')
        img_arr = self.normalization(img_arr, intensity_norm)
        img_arr = img_arr.astype(np.float32)

        """Create point roi in row/column coordinates appropriately translated and scaled
        TODO
        ----
        Need check
        """
        prf = -0.5
        pcf = -0.5
        tlx, tly, _, brx, bry, _ = bb
        self.bb = bb
        self.log.debug(f'bounding box = {bb}')

        # if (pr >= 0) and (pc >= 0):
        if (tly <= pr <= (bry+1)) and (tlx <= pc <= (brx+1)):
            pr = pr-tly
            pc = pc-tlx
            prf = pr/((bry+1)-tly) 
            pcf = pc/((brx+1)-tlx)
        # added by MD 12/3/21
        else:
            raise ValueError(f'Reference marking ({pc},{pr}) is not within the bouding box {bb}.')
        roi_arr = np.array(([prf],[pcf]))
        roi_arr = np.squeeze(roi_arr)
        self.log.debug(f'roi_arr of {idx}: {roi_arr.shape}, {roi_arr}')
        self.log.debug(f'pr, pc: {pr}, {pc}')
        # self.log.debug(f'roi_arr = {roi_arr}')
        # roi_arr = roi_arr.astype(np.int32)
        # self.log.debug(f'roi_arr casting = {roi_arr}')

        """"3. draw png of normalized input image for review purpose"""

        # if ((not self.png_skip) and (self.mode != 'train')) or ((not self.png_skip_training) and (self.training_child)):
        if (not self.png_skip) and ((not self.png_skip_training) or (not self.training_child)):
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
            self.draw_fig(review_fig_dir, img_arr, intensity_norm, roi_arr=roi_arr, ph_area_mask_array=ph_area_mask_array, original_img=original_img)
        return img_arr, roi_arr
    
    def _load(self, image_path, roi_path=None, input_roi_folder=None, previous_seg_roi_file=None, ph_area_file=None): #, tmp_roi_name=None):
        """Private function for loading single image (and roi)

        Parameters
        ----------
        image_path : str
            path of the image to load
        roi_path : str (default = None)
            path of the roi to load
            if None, only image will be loaded.
        input_roi_folder : str (default = None)
            path of the input roi to be loaded
            if None, will be ignored.
        previous_seg_roi_file : str
            path of the roi output file of previous miu node

        Returns
        -------
        image : qia.common.img.image object
            loaded image
        image_array : ndarray
            numpy array of loaded image, cropped based on bounding box configuration.
            crop area (points) will be controled by MIU. Without any miu node, 
            it will use the whole image area.
        roi : qia.common.img.image object
            loaded roi. If roi_path is not given, return None
        roi_array : ndarray
            numpy array of loaded roi, cropped based on bounding box configuration.
            If roi_path is not given, return None
            crop area (points) will be controled by MIU. Without any miu node, 
            it will use the whole image area.
        bb : list of int
            bounding box information from segmentations of previous node and zitter
        """
        
        try:
            """Previous node Segmentation
            TODO
            ----
            1. handle with zitter
                jitt_perc = 0.1
                : The percentage of the bounding box dimensions 
                in the x,y direction that is applied to generate 
                the 9 samples from each case. This is only applied
                to the training set (not validation). Set <= 0.0 if 
                no jitter is desired. Currently only implemented 
                for gen_roi_hdf5 (not generate_hdf5 coordinates).

                if zitter
                    tlx, tly, tlz, brx, bry, brz = self.bb     
                    d1 = int(((bry+1) - tly)*jitt_perc)
                    d0 = int(((brx+1) - tlx)*jitt_perc)

                    generate 4 gittered points (tag tl, tr, bl, br)
                        draw each case sample with tag
                    and treat each zitter-case as one sample.
                    for each zitter-case
                        tly = max(0, tly)
                        bry = min(bry+1, img_arr.shape[0])-1
                        tlx = max(0, tlx)
                        brx = min(brx+1, img_arr.shape[1])-1
                        bb = [tlx, tly, None, brx, bry, None]
            """
            if previous_seg_roi_file is not None:
                try:
                    assert (os.path.exists(previous_seg_roi_file))
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
                """TODO: for prediction. need to check and revise""" 
                bb = copy.copy(self.bb)

            # try:
            image = qimage.read(image_path) #our inhouse image object format
            # except Exception as e:
            #     if 'seri' in image_path.split('.')[-1]:
            #         with open(image_path) as f:
            #             img_path_list = f.readlines()
            #             image = qimage.read(img_path_list[0])
            #             try:
            #                 self.image = qimage.read(img_path_list[0])
            #             except:
            #                 self.image = pydicom.read_file(img_path_list[0])
            #     else:
            #         traceback.print_exc()
            #         raise ValueError(f'Image loading failed for {image_path} with exception {e}')        
            
            if roi_path is not None:
                if 'roi' in roi_path.split('.')[-1]:
                    roi = qimage.cast(image)
                    roi.fill_with_roi(roi_path)
                else:
                    roi = qimage.read(roi_path)
            else:
                roi = None
                roi_array = None
            
            if ph_area_file:    
                ph_area_mask_roi = qimage.cast(image)
                ph_area_mask_roi.fill_with_roi(ph_area_file)
            
            """Centering
            TODO: ask what this means"""
            image = image.get_alias(min_point=(0, 0, 0))
            image_array = image.get_array()
            original_img_shape = image_array.shape
            if roi_path is not None:
                roi = roi.get_alias(min_point=(0, 0, 0))
                roi_array = roi.get_array()
                if 'roi' not in roi_path.split('.')[-1]:
                    if not (len(np.unique(roi_array))==1 and np.unique(roi_array)[0]==0):
                        roi_array = (roi_array - np.min(roi_array))/ (np.max(roi_array) - np.min(roi_array))
                    roi_array[roi_array >= 0.5] = 1
                    roi_array[roi_array < 0.5] = 0

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
                        ph_area_file = None
                        self.log.debug(f'Missing the ph area {ph_area_file} for normalizing iamge {image_path}.')
            if ph_area_file is not None:    
                ph_area_roi = qimage.read(ph_area_file)
                self.log.debug(f'The ph area {ph_area_file} will be used for normalizing iamge {image_path}.')
                ph_area_roi = qimage.read(ph_area_file)
                # if 'roi' in ph_area_file.split('.')[-1]:
                #     ph_area_roi = qimage.cast(image)
                #     ph_area_roi.fill_with_roi(ph_area_file)
                # else:
                #     ph_area_roi = qimage.read(ph_area_file)
                # ph_area_roi = ph_area_roi.get_alias(min_point=(0, 0, 0))
                # ph_area_array = ph_area_roi.get_array()
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
                
            """Crop
            TODO: ugly...update by switching with input type attributes
            move +1 things to initialize self.bb too.
            """
            if len(self.get_target_shape()) > 2:
                """3D input"""
                image_array = image_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
                if roi_path is not None: roi_array = roi_array[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)]
            else:
                """2D input"""
                image_array = image_array[0, tly:(bry+1), tlx:(brx+1)]
                if roi_path is not None: roi_array = roi_array[0, tly:(bry+1), tlx:(brx+1)]

            """CXR specific: invert (if necessary)"""
            try: dcmp = cnntls.dcm_dir(image_path)
            except: dcmp = image_path
            # try:
            #     obj = qdicom.read(dcmp)
            # except:
            #     # isdicomfile=False
            #     obj = pydicom.dcmread(dcmp)
            obj = qdicom.read(dcmp)
            photometric = obj['PhotometricInterpretation'].value
            if photometric=='MONOCHROME1':
                # self.log.debug('*** inverting image before normalizing')
                image_array = np.invert(image_array)
            
            """Search area attention"""
            if self.search_area_attention_type is not None:
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
                image_array = np.concatenate([image_array, search_area_roi_array], axis=-1)
            
            return image, image_array, roi, roi_array, bb, ph_area_image_array, ph_area_mask_array

        except Exception as e:
            traceback.print_exc()
            raise ValueError(f'Image loading failed for {image_path} with exception {e}.\n Error:\n{traceback.format_exc()}\n')

    def _load_training_set(self):
        working_dir = self.config_model['path_info']['working_directory']
        working_dir = os.path.join(working_dir, f'{self.node_name}_KerasModel')
        input_tag = self.config_model['chromosome_info']['input_tag']
        input_dir = os.path.join(working_dir, 'input')
        train_idx_path = os.path.join(input_dir, f'train_{input_tag}.csv')
        validation_idx_path = os.path.join(input_dir, f'valid_{input_tag}.csv')
        # hdf5_train_path = os.path.join(input_dir, f'train_{input_tag}.hdf5')
        # hdf5_validation_path = os.path.join(input_dir, f'valid_{input_tag}.hdf5')
        npy_train_dir_path = os.path.join(input_dir, f'train_{input_tag}_npy')
        npy_validation_dir_path = os.path.join(input_dir, f'valid_{input_tag}_npy')

        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        """Load training and validation list"""
        train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Train index: {train_idx}')
        validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
        self.log.info(f'Validation index: {validation_idx}')
        
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        
        self.log.info(f'Start loading npy input files under {npy_train_dir_path}')
        self.log.info(f'Train index: {train_idx}')
        sys.stdout.flush()

        if self.use_multiprocessing:
            self.log.debug(f'Start computing npy with {self.workers} number of CPU cores.')
            pool = Pool(processes=self.workers)
            result = pool.map_async(partial(load_idx_npy,
                                            npy_train_dir_path= npy_train_dir_path), 
                                    enumerate(train_idx)).get()
            pool.close()
            pool.join()
            self.log.debug(f'Finished computing npy with {self.workers} number of CPU cores.')
        else:
            self.log.debug(f'Start computing npy with a single CPU core.')
            result=[]
            for i, idx in enumerate(train_idx):
                result.append(load_idx_npy((i,idx), npy_train_dir_path))
            self.log.debug(f'Finished computing npy using a single CPU core.')
        self.log.info(f'Writing finished: {npy_train_dir_path}')

        train_set = {}
        train_set['X'] = np.array([case[0] for case in result])
        train_set['Y'] = np.array([case[1] for case in result])
        x_shape = train_set['X'].shape
        y_shape = train_set['Y'].shape
        self.log.info(f'Finised loading npy input files under {npy_train_dir_path}')
        self.log.info(f'Training set: x={x_shape}, y={y_shape}')
        
        np.savetxt(train_idx_path, train_idx, delimiter=",")
        sys.stdout.flush()

        if (validation_idx is not None):
            self.log.info(f'Start loading npy input files under {npy_validation_dir_path}')
            self.log.info(f'Validation index: {validation_idx}')
            sys.stdout.flush()

            if self.use_multiprocessing:
                self.log.debug(f'Start computing npy with {self.workers} number of CPU cores.')
                pool = Pool(processes=self.workers)
                val_result = pool.map_async(partial(load_idx_npy,
                                                npy_train_dir_path= npy_validation_dir_path), 
                                        enumerate(validation_idx)).get()
                pool.close()
                pool.join()
                self.log.debug(f'Finished computing npy with {self.workers} number of CPU cores.')
            else:
                self.log.debug(f'Start computing npy with a single CPU core.')
                val_result=[]
                for i, idx in enumerate(validation_idx):
                    val_result.append(load_idx_npy((i,idx), npy_validation_dir_path))
                self.log.debug(f'Finished computing npy using a single CPU core.')
            self.log.info(f'Loading finished: {npy_validation_dir_path}')

            validation_set = {}
            validation_set['X'] = np.array([case[0] for case in val_result])
            validation_set['Y'] = np.array([case[1] for case in val_result])
            x_shape = validation_set['X'].shape
            y_shape = validation_set['Y'].shape
            self.log.info(f'Finised loading npy input files under {npy_validation_dir_path}')
            self.log.info(f'Validationing set: x={x_shape}, y={y_shape}')
            np.savetxt(validation_idx_path, validation_idx, delimiter=",")
            sys.stdout.flush()

        """Load processed inputs (image, roi) from hdf5"""
        # self.log.info(f'Start loading hdf5 input files: {hdf5_train_path}')
        # train_set = h5py.File(hdf5_train_path, 'r')
        # # train_idx = np.array(pd.read_csv(train_idx_path, header=None).iloc[:,0]).astype(np.int)
        # self.log.info(f'Train index: {train_idx}')
        # x_shape = train_set['X'].shape
        # y_shape = train_set['Y'].shape
        # self.log.info(f'Finised loading hdf5 input files: {hdf5_train_path}')
        # self.log.info(f'Training set: x={x_shape}, y={y_shape}')

        # # if (validation_idx is not None): 
        # if os.path.exists(hdf5_validation_path): 
        #     self.log.info(f'Start loading hdf5 input files: {hdf5_validation_path}')
        #     validation_set = h5py.File(hdf5_validation_path, 'r')
        #     # validation_idx = np.array(pd.read_csv(validation_idx_path, header=None).iloc[:,0]).astype(np.int)
        #     self.log.info(f'Validation index: {validation_idx}')
        #     x_shape = validation_set['X'].shape
        #     y_shape = validation_set['Y'].shape
        #     self.log.info(f'Finised loading hdf5 input files: {hdf5_validation_path}')
        #     self.log.info(f'Validation set: x={x_shape}, y={y_shape}')
        #     np.savetxt(validation_idx_path, validation_idx, delimiter=",")
        #     self.log.info(f'Writing validation index finished: {validation_idx_path}')
        
        """Build a private _processed_set"""
        total_x = np.concatenate([np.array(train_set['X']), np.array(validation_set['X'])])
        total_y = np.concatenate([np.array(train_set['Y']), np.array(validation_set['Y'])])
        total_idx = np.concatenate([train_idx, validation_idx])
        total_x = total_x[total_idx].astype(np.float32)
        total_y = total_y[total_idx].astype(np.float32)
        self._processed_set = {'X':total_x, 'Y':total_y}
        self.log.debug(self._processed_set['X'].shape)
        self.log.debug(self._processed_set['Y'].shape)
        self.log.info(f'Finish build training set from {len(train_idx)} idxs: x={x_shape}, y={y_shape}')
        self.log.info(f'Finish build validation set from {len(validation_idx)} idxs: x={x_shape}, y={y_shape}')
        self.log.info(total_y)
        self.log.info('---------------------------------------------------------------')
        sys.stdout.flush()
        return train_idx, validation_idx

    def write_roi(self, pred, base_img, threshold=0.5, norm_img_arr = None):
        """Specific ROI writing function for CXR tasks.

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

        original_img_shape = base_img.get_size()
        # original_img_shape = self.get_image_shape(base_img.get_array()[0])
        # self.log.debug(f'{pred.shape}, {len(pred.shape)}')
        if len(pred.shape)==2:
            """Check if the CNN output is a 2D point and convert it to an image

            The pred output is required to be (rows, colums)
            When scaling/translating the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
            self.log.debug(f'Using point ROI writing function with (y, x) = {pred[0]}.')
            pred_rescale = np.zeros((1, original_img_shape[1], original_img_shape[0]))
            #self.log.debug("CNN pred[0][0] = ", pred[0][0])
            #self.log.debug("CNN pred[0][1] = ", pred[0][1])
            if (0<=pred[0][0]<=1) and (0<=pred[0][1]<=1):
                tlx, tly, _, brx, bry, _ = self.bb
                r = (int)(pred[0][0]*(bry-tly)) + tly
                c = (int)(pred[0][1]*(brx-tlx)) + tlx
                pred_rescale[0][r][c] = 1
                # count = 1
        else: 
            """CNN output is a 2D image
            """
            # pred_img = os.path.join(args.output_path, 'cnn_pred_output_image.png')
            """Check the comment from previous code & change it by self.resize:
            Drop the last axis because it is for the numbers of samples, 
            and we only have 1 sample
            When scaling the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
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

            pred_cast = pred.astype(np.uint8)
            pred_rescale = np.zeros((original_img_shape[0], original_img_shape[1], original_img_shape[2]))
            pred_rescale[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)] = pred_cast   
            # self.log.debug(f'pred_rescale.shape={pred_rescale.shape}')

        # roi_sp = os.path.join(output_path,'pred_'+se_name+'.roi')
        # pred_cast = pred_rescale.astype(np.uint8)
        # pred_image = qimage.from_array(pred_cast, base_img)
        pred_image = qimage.from_array(pred_rescale, template=base_img)

        self.log.debug(f'Written roi size: {pred_image.get_size()}')
        return [pred_image], ['pred'], None, None


class cxr_cvc_image_reader(cxr_image_reader):
    """CXR CVC Image Reader class for loading CXR images with .seri formats (2D; 
    single-channel) in CVIB. This class supports the specific CXR tasks.
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    When the roi_reference is given, it will process the reference data
    for segmentation. If not, it will deal with x,y coordinates.
    
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
        whether to do augmentation on mini-batch for traiing
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self._task_type: int
        private
        0: segmentation with given roi
        1: segmentation without given roi
        TODO : using this
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False)
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
        write roi instnace from qia.common.image based on prediction probability
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
        Private function returning qia.common.img.image and numpy array of
        loaded image (and roi) from image_path (and roi_path)
    _load_search_area(self, image_path, previous_seg_roi_file)
        Private function for loading search area from previos segmentation roi
    _get_image_batch(self, idxs)
        Privait function returning one image batch containing given {idxs} cases.
    _get_reference_batch(self, idxs)
        Private function returning one reference batch containing given {idxs} cases.
    _get_processed_inputs_with_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is given.
    _get_processed_inputs_without_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is not given.

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
            - .seri: in-house seri file listing DICOM image paths 
    
    Supporting image shapes: 2D

    Supporting image channels: single-channel
    
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
        super(cxr_cvc_image_reader,self).__init__(config_model, config_resource, log, mode)


class cxr_multi_task_image_reader(cxr_image_reader):
    """CXR image reader class for Multi-task learning.
    It load CXR images with .seri formats (2D; single-channel) in CVIB.
    Current version only assume the point marking problem.
    For the referncfe, two point markings are given via train_list.csv.
    This class supports the multiple CXR point marking tasks.
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    When the roi_reference is given, it will process the reference data
    for segmentation. If not, it will deal with x,y coordinates.
    
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
        whether to do augmentation on mini-batch for traiing
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self._task_type: int
        private
        0: segmentation with given roi
        1: segmentation without given roi
        TODO : using this
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False)
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
        write roi instnace from qia.common.image based on prediction probability
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
        Private function returning qia.common.img.image and numpy array of
        loaded image (and roi) from image_path (and roi_path)
    _load_search_area(self, image_path, previous_seg_roi_file)
        Private function for loading search area from previos segmentation roi
    _get_image_batch(self, idxs)
        Privait function returning one image batch containing given {idxs} cases.
    _get_reference_batch(self, idxs)
        Private function returning one reference batch containing given {idxs} cases.
    _get_processed_inputs_with_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is given.
    _get_processed_inputs_without_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is not given.

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
            - .seri: in-house seri file listing DICOM image paths 
    
    Supporting image shapes: 2D

    Supporting image channels: single-channel
    
    Usage
    -----


    Examples
    --------

    TODO
    -----
    multiple roi input

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
        super(cxr_multi_task_image_reader, self).__init__(config_model, config_resource, log, mode)
    
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
        x = self._processed_set['X'][idxs]
        raw_y = self._processed_set['Y'][idxs]
        carina_y = raw_y[:,0,:]
        ettip_y = raw_y[:,1,:]
        return (x,[carina_y, ettip_y])

    # def get_mini_batch(self, idxs):
    #     """Return a mini-batch [x,y] containing given {idxs} cases for training

    #     Parameters
    #     ----------
    #     idxs: ndarray
    #         ndarray of index of the cases for construting mini-batch
        
    #     Returns
    #     -------
    #     tuple of ndarray
    #         mini-batch [x,y] for training
    #     """
    #     x = []; y_carina = []; y_ettip = []
    #     for idx in idxs:
    #         try:
    #             inputs = self._get_processed_inputs(idx)
    #             # print(inputs[0].shape)
    #             print(inputs[1])
    #             sys.stdout.flush()
    #             x.append(inputs[0])
    #             y_carina.append(inputs[1][0])
    #             y_ettip.append(inputs[1][1])
    #             self.log.debug(f'ids: {idx}, carina: {y_carina[-1]}, ettip: {y_ettip[-1]}, x.shape={x[-1].shape}')
    #             sys.stdout.flush()
    #         except Exception as e:
    #             self.log.debug(f'Fail to load {idx}. Will ignore {idx}.\n Error:\n{traceback.format_exc()}\n')
    #             sys.stdout.flush()
    #             pass
    #     x = np.array(x); y = [np.array(y_carina), np.array(y_ettip)]
    #     return (x,y)

    def _get_processed_inputs_without_roi(self, idx): #, miu_temp_path=None):
        """Return processed inputs from case {idx} when reference roi 
        is not given.
        Parameters
        ----------
        idx : int
            index of case to process the image
        miu_temp_path : str
            same the tmp output of lung segmentation

        Returns
        -------
        img_arr : ndarray
            preprocessed image from case {idx}
        roi_arr : ndarray
            processed roi from case {idx}
        """
        if self.previous_node_rois is not None:
            previous_node_roi_folder = os.path.join(self.previous_node_rois, str(idx)) # previous_node_roi_name = str(idx)
            previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'search_area_{self.node_name}.roi')
        else:
            previous_seg_roi_file = None

        image_path = self.case_dict_list[idx]['image']

        pr_carina = int(self.case_dict_list[idx]['y_carina'])
        pc_carina = int(self.case_dict_list[idx]['x_carina'])
        self.log.debug(f'pr_carina = {pr_carina}, pc_carina = {pc_carina}')
        pr_ettip = int(self.case_dict_list[idx]['y_ettip'])
        pc_ettip = int(self.case_dict_list[idx]['x_ettip'])
        self.log.debug(f'pr_ettip = {pr_ettip}, pc_ettip = {pc_ettip}')

        self.log.debug(f'{image_path}, {previous_seg_roi_file}')

        if self.use_ph_area:
            ph_area_file = os.path.join(os.path.dirname(previous_seg_roi_file), f'ph_area_{self.node_name}.roi')
        else:
            ph_area_file = None

        img, img_arr, _, _, bb, ph_area_image_array, ph_area_mask_array = self._load(image_path, None, None, previous_seg_roi_file, ph_area_file) #, idx)
        
        """Pre-processing images"""
        """1. resizing images"""
        target_shape = self.get_target_shape()
        if self.search_area_attention_type is not None:
            img_arr = np.concatenate([np.expand_dims(self.resize(img_arr[...,j], target_shape), axis=-1) for j in range(img_arr.shape[-1])], axis=-1)
        else:
            img_arr = self.resize(img_arr, target_shape)
        # if ph_area_file: ph_area_image_array = self.resize(ph_area_image_array, target_shape)
        original_img = copy.copy(img_arr)
        if ph_area_mask_array is not None: 
            ph_area_mask_array = self.resize_roi(ph_area_mask_array, target_shape)
            ph_area_mask_array[ph_area_mask_array > 0.] = 1.
            ph_area_mask_array = ph_area_mask_array.astype(np.int32)

        """2 intensity normalization"""
        intensity_norm = self.get_intensity_norm()
        # self.log.debug(f'len(intensity_norm) : {len(intensity_norm)}')
        img_arr = self.normalization(img_arr, intensity_norm, ph_area_image_array)
        img_arr = img_arr.astype(np.float32)

        """Create point roi in row/column coordinates appropriately translated and scaled
        TODO
        ----
        Need check
        """
        prf = -0.5
        pcf = -0.5
        tlx, tly, _, brx, bry, _ = bb
        self.bb = bb
        self.log.debug(f'bounding box = {bb}')

        """Carina"""
        if (tly <= pr_carina <= (bry+1)) and (tlx <= pc_carina <= (brx+1)):
            pr_carina = pr_carina-tly
            pc_carina = pc_carina-tlx
            prf_carina = pr_carina/((bry+1)-tly) # float \in [0,1]
            pcf_carina = pc_carina/((brx+1)-tlx) # float \in [0,1]
        else:
            raise ValueError(f'Reference carina marking ({pc_carina},{pr_carina}) is not within the bouding box {bb}.')
        
        """ettip"""
        if (tly <= pr_ettip <= (bry+1)) and (tlx <= pc_ettip <= (brx+1)):
            pr_ettip = pr_ettip-tly
            pc_ettip = pc_ettip-tlx
            prf_ettip = pr_ettip/((bry+1)-tly) # float \in [0,1]
            pcf_ettip = pc_ettip/((brx+1)-tlx) # float \in [0,1]
        else:
            raise ValueError(f'Reference ettip marking ({pc_ettip},{pr_ettip}) is not within the bouding box {bb}.')


        roi_arr = np.array([[[prf_carina],[pcf_carina]],[[prf_ettip],[pcf_ettip]]])
        roi_arr = np.squeeze(roi_arr)
        self.log.debug(f'roi_arr of {idx}: {roi_arr.shape}, {roi_arr}')
        self.log.debug(f'\t[tly, bry] = [{tly},{bry}], [tlx, brx] = [{tlx},{brx}]')
        self.log.debug(f'\tpr_carina, pc_carina: {pr_carina}, {pc_carina}')
        self.log.debug(f'\tprf_carina, pcf_carina: {prf_carina}, {pcf_carina}')

        """"3. draw png of normalized input image for review purpose"""
        # if ((not self.png_skip) and (self.mode != 'train')) or ((not self.png_skip_training) and (self.training_child)):
        if (not self.png_skip) and ((not self.png_skip_training) or (not self.training_child)):
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
            self.draw_fig(review_fig_dir, img_arr, intensity_norm, roi_arr=roi_arr, ph_area_mask_array=ph_area_mask_array, original_img=original_img)
        return img_arr, roi_arr

    def draw_fig(self, review_fig_dir, img_norm_list, intensity_norm, roi_arr=None, ph_area_mask_array = None, original_img=None,c=4, alhpa=0.7, png_overwrite=False):
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
                            if len(roi_arr.shape) < 3:
                                """point roi (roi_arr) = (y, x) or [(y, x), (y, x)]"""
                                rows, cols = img_display.shape
                                prf_carina, pcf_carina = roi_arr[0]
                                prf_ettip, pcf_ettip = roi_arr[1]
                                # if (0<=scaled_prf<=1) and (0<=scaled_pcf<=1):
                                #     tlx, tly, _, brx, bry, _ = self.bb
                                #     prf = (int)(scaled_prf*(bry-tly)) + tly
                                #     pcf = (int)(scaled_pcf*(brx-tlx)) + tlx
                                #     self.log.debug(f'(prs, pcs)={(prf, pcf)}, bb = {self.bb}')

                                if (prf_carina>=0) and (pcf_carina>=0):
                                    prs_carina = round(rows*prf_carina).astype(int)
                                    pcs_carina = round(cols*pcf_carina).astype(int)
                                
                                if (prf_ettip>=0) and (pcf_ettip>=0):
                                    prs_ettip = round(rows*prf_ettip).astype(int)
                                    pcs_ettip = round(cols*pcf_ettip).astype(int)

                                    # for m in range(max(prs-7, 0), min(prs+8, cols)):
                                    #     for n in range(max(pcs-7, 0), min(pcs+8, rows)):
                                    #         img_display[m][n] = 1
                                    # for m in range(max(prs-5, 0), min(prs+6, cols)):
                                    #     for n in range(max(pcs-5, 0), min(pcs+6, rows)):
                                    #         img_display[m][n] = 0
                                axes[i].imshow(img_display, cmap=plt.cm.gray)
                                axes[i].scatter([pcs_carina], [prs_carina], c='red', label='Carina')
                                axes[i].scatter([pcs_ettip], [prs_ettip], c='blue', label='ET tip')
                                axes[i].set_title('2D slice with ROI\n[on preprocessed image]',y=-0.3, ha="center")
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
                        fig.legend()
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

    def write_roi(self, pred, base_img, threshold=0.5, norm_img_arr = None):
        """Specific ROI writing function for CXR tasks.

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

        original_img_shape = base_img.get_size()
        # original_img_shape = self.get_image_shape(base_img.get_array()[0])
        # self.log.debug(f'{pred[0].shape}, {len(pred[0].shape)}')
        if len(pred[0].shape) < 3:
            """Check if the CNN output is a 2D point and convert it to an image

            The pred output is required to be (rows, colums)
            When scaling/translating the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
            self.log.debug(f'Using point ROI writing function with [(carina_y, carina_x), (ettip_y, ettip_x)] = {pred[0], pred[1]}.')
            pred_carina_rescale = np.zeros((1, original_img_shape[1], original_img_shape[0]))
            pred_carina = pred[0][0]
            carina_r = None
            carina_c = None
            if (0<=pred_carina[0]<=1) and (0<=pred_carina[1]<=1):
                tlx, tly, _, brx, bry, _ = self.bb
                carina_r = (int)(pred_carina[0]*(bry-tly)) + tly
                carina_c = (int)(pred_carina[1]*(brx-tlx)) + tlx
                pred_carina_rescale[0][carina_r][carina_c] = 1

            pred_ettip_rescale = np.zeros((1, original_img_shape[1], original_img_shape[0]))
            pred_ettip = pred[1][0]
            ettip_r = None
            ettip_c = None
            if (0<=pred_ettip[0]<=1) and (0<=pred_ettip[1]<=1):
                tlx, tly, _, brx, bry, _ = self.bb
                ettip_r = (int)(pred_ettip[0]*(bry-tly)) + tly
                ettip_c = (int)(pred_ettip[1]*(brx-tlx)) + tlx
                pred_ettip_rescale[0][ettip_r][ettip_c] = 1

            pred_carina_image = qimage.from_array(pred_carina_rescale, template=base_img)
            pred_ettip_image = qimage.from_array(pred_ettip_rescale, template=base_img)

            self.log.debug(f'Written roi size for carina: {pred_carina_image.get_size()}')
            self.log.debug(f'Written roi size for ettip: {pred_ettip_image.get_size()}')
            pred_image = [pred_carina_image, pred_ettip_image]

            """plt for marking"""
            c=4; alhpa=0.7; nrow = 1
            fig, ax = plt.subplots()
            img_arr = base_img.get_array()
            ax.imshow(img_arr[0], cmap='gray')
            if carina_c is not None and carina_r is not None:
                ax.scatter([carina_c], [carina_r], c='red', label='Carina Prediction')
            if ettip_c is not None and ettip_r is not None:
                ax.scatter([ettip_c], [ettip_r], c='blue', label='ET Tip Prediction')
            ax.legend()
            fig.tight_layout()

            if norm_img_arr is not None:
                fig_norm, ax_norm = plt.subplots()
                ax_norm.imshow(norm_img_arr[0], cmap='gray')

                rows, cols = norm_img_arr[0].shape
                prf_carina, pcf_carina = pred_carina
                prf_ettip, pcf_ettip = pred_ettip
                prs_carina = None; pcs_carina = None
                prs_ettip = None; pcs_ettip = None
                if (prf_carina>=0) and (pcf_carina>=0):
                    prs_carina = round(rows*prf_carina).astype(int)
                    pcs_carina = round(cols*pcf_carina).astype(int)
                
                if (prf_ettip>=0) and (pcf_ettip>=0):
                    prs_ettip = round(rows*prf_ettip).astype(int)
                    pcs_ettip = round(cols*pcf_ettip).astype(int)

                if pcs_carina is not None and prs_carina is not None:
                    ax_norm.scatter([pcs_carina], [prs_carina], c='red', label='Carina Prediction')
                if pcs_ettip is not None and prs_ettip is not None:
                    ax_norm.scatter([pcs_ettip], [prs_ettip], c='blue', label='ET Tip Prediction')
                ax_norm.legend()
            fig_norm.tight_layout()

            return pred_image, ['carina', 'ettip'], fig, fig_norm
        else: 
            self.log.debug(f'{pred.shape}, {len(pred.shape)}')
            """CNN output is a 2D image
            """
            # pred_img = os.path.join(args.output_path, 'cnn_pred_output_image.png')
            """Check the comment from previous code & change it by self.resize:
            Drop the last axis because it is for the numbers of samples, 
            and we only have 1 sample
            When scaling the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
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

            pred_cast = pred.astype(np.uint8)
            pred_rescale = np.zeros((original_img_shape[0], original_img_shape[1], original_img_shape[2]))
            pred_rescale[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)] = pred_cast   
            # self.log.debug(f'pred_rescale.shape={pred_rescale.shape}')

            # roi_sp = os.path.join(output_path,'pred_'+se_name+'.roi')
            # pred_cast = pred_rescale.astype(np.uint8)
            # pred_image = qimage.from_array(pred_cast, base_img)
            pred_image = qimage.from_array(pred_rescale, template=base_img)

            self.log.debug(f'Written roi size: {pred_image.get_size()}')
            return [pred_image], ['pred'], None


class cxr_landmark_detection_image_reader(cxr_image_reader):
    """CXR Landmark Detection Image Reader class for loading CXR images with .seri formats (2D; 
    single-channel) in CVIB. This class supports the specific CXR tasks.
    Instead of change the point range [0,1] from cxr_image_reader, this reader will use point range [-1,1].
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    When the roi_reference is given, it will process the reference data
    for segmentation. If not, it will deal with x,y coordinates.
    
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
        whether to do augmentation on mini-batch for traiing
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self._task_type: int
        private
        0: segmentation with given roi
        1: segmentation without given roi
        TODO : using this
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False)
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
        write roi instnace from qia.common.image based on prediction probability
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
        Private function returning qia.common.img.image and numpy array of
        loaded image (and roi) from image_path (and roi_path)
    _load_search_area(self, image_path, previous_seg_roi_file)
        Private function for loading search area from previos segmentation roi
    _get_image_batch(self, idxs)
        Privait function returning one image batch containing given {idxs} cases.
    _get_reference_batch(self, idxs)
        Private function returning one reference batch containing given {idxs} cases.
    _get_processed_inputs_with_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is given.
    _get_processed_inputs_without_roi(self, idx)
        Private function return processed inputs from case {idx} when reference roi 
        is not given.

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
            - .seri: in-house seri file listing DICOM image paths 
    
    Supporting image shapes: 2D

    Supporting image channels: single-channel
    
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
        super(cxr_landmark_detection_image_reader,self).__init__(config_model, config_resource, log, mode)

    
    def _get_processed_inputs_without_roi(self, idx): #, miu_temp_path=None):
        """Return processed inputs from case {idx} when reference roi 
        is not given.
        Parameters
        ----------
        idx : int
            index of case to process the image
        miu_temp_path : str
            same the tmp output of lung segmentation

        Returns
        -------
        img_arr : ndarray
            preprocessed image from case {idx}
        roi_arr : ndarray
            processed roi from case {idx}
        """
        if self.previous_node_rois is not None:
            previous_node_roi_folder = os.path.join(self.previous_node_rois, str(idx)) # previous_node_roi_name = str(idx)
            previous_seg_roi_file = os.path.join(previous_node_roi_folder, f'search_area_{self.node_name}.roi')
        else:
            previous_seg_roi_file = None

        image_path = self.case_dict_list[idx]['image']
        pr = int(self.case_dict_list[idx]['y'])
        pc = int(self.case_dict_list[idx]['x'])
        self.log.debug(f'pr = {pr}, pc = {pc}')
        self.log.debug(f'{image_path}, {previous_seg_roi_file}')

        if self.use_ph_area:
            ph_area_file = os.path.join(os.path.dirname(previous_seg_roi_file), f'ph_area_{self.node_name}.roi')
        else:
            ph_area_file = None

        img, img_arr, _, _, bb, ph_area_image_array, ph_area_mask_array = self._load(image_path, None, None, previous_seg_roi_file, ph_area_file) #, idx)
        
        """Pre-processing images"""
        """1. resizing images"""
        target_shape = self.get_target_shape()
        if self.search_area_attention_type is not None:
            img_arr = np.concatenate([np.expand_dims(self.resize(img_arr[...,j], target_shape), axis=-1) for j in range(img_arr.shape[-1])], axis=-1)
        else:
            img_arr = self.resize(img_arr, target_shape)
        # if ph_area_file: ph_area_image_array = self.resize(ph_area_image_array, target_shape)
        original_img = copy.copy(img_arr)
        if ph_area_mask_array is not None:
            ph_area_mask_array = self.resize_roi(ph_area_mask_array, target_shape)
            ph_area_mask_array[ph_area_mask_array > 0.] = 1.
            ph_area_mask_array = ph_area_mask_array.astype(np.int32)

        """2 intensity normalization"""
        intensity_norm = self.get_intensity_norm()
        # self.log.debug(f'len(intensity_norm) : {len(intensity_norm)}')
        img_arr = self.normalization(img_arr, intensity_norm)
        img_arr = img_arr.astype(np.float32)

        """Create point roi in row/column coordinates appropriately translated and scaled
        TODO
        ----
        Need check
        """
        prf = -0.5
        pcf = -0.5
        tlx, tly, _, brx, bry, _ = bb
        self.bb = bb
        self.log.debug(f'bounding box = {bb}')

        # if (pr >= 0) and (pc >= 0):
        if (tly <= pr <= (bry+1)) and (tlx <= pc <= (brx+1)):
            bb_length = (bry+1)-tly
            bb_width = (brx+1)-tlx
            prf = 2.*(pr-tly)/(bb_length) - 1. # prf in [-1, 1]
            pcf = 2.*(pc-tlx)/(bb_width) - 1. # pcf in [-1, 1]
        # added by MD 12/3/21
        else:
            raise ValueError(f'Reference marking ({pc},{pr}) is not within the bouding box {bb}.')
        # roi_arr = np.array(([prf],[pcf]))
        roi_arr = np.array([[pcf,prf]]) # single landmark, shape = (1, 2)
        # roi_arr = np.squeeze(roi_arr)
        self.log.debug(f'roi_arr of {idx}: {roi_arr.shape}, {roi_arr}')
        self.log.debug(f'pr, pc: {pr}, {pc}')
        self.log.debug(f'prf, pcf: {prf}, {pcf}')
        self.log.debug(f'bry, tly: {bry}, {tly}')
        self.log.debug(f'brx, tlx: {brx}, {tlx}')
        # self.log.debug(f'roi_arr = {roi_arr}')
        # roi_arr = roi_arr.astype(np.int32)
        # self.log.debug(f'roi_arr casting = {roi_arr}')

        """"3. draw png of normalized input image for review purpose"""

        # if ((not self.png_skip) and (self.mode != 'train')) or ((not self.png_skip_training) and (self.training_child)):
        if (not self.png_skip) and ((not self.png_skip_training) or (not self.training_child)):
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
            self.draw_fig(review_fig_dir, img_arr, intensity_norm, roi_arr=roi_arr, ph_area_mask_array=ph_area_mask_array, original_img=original_img)
        return img_arr, roi_arr
    

    def write_roi(self, pred_landmark, base_img, threshold=0.5, norm_img_arr = None):
        """Specific ROI writing function for CXR tasks.

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

        original_img_shape = base_img.get_size()
        # original_img_shape = self.get_image_shape(base_img.get_array()[0])
        # self.log.debug(f'{pred.shape}, {len(pred.shape)}')
        pred = pred_landmark[0]
        if len(pred.shape)==2:
            """Check if the CNN output is a 2D point and convert it to an image

            The pred output is required to be (rows, colums)
            When scaling/translating the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
            self.log.debug(f'Using point ROI writing function with (y, x) = {pred[0]}.')
            pred_rescale = np.zeros((1, original_img_shape[1], original_img_shape[0]))
            #self.log.debug("CNN pred[0][0] = ", pred[0][0])
            #self.log.debug("CNN pred[0][1] = ", pred[0][1])
            if (-1<=pred[0][0]<=1) and (-1<=pred[0][1]<=1):
                # bb_length = (bry+1)-tly
                # bb_width = (brx+1)-tlx
                # prf = 2.*(pr-tly)/(bb_length) - 1. # prf in [-1, 1]
                # pcf = 2.*(pc-tlx)/(bb_width) -1. # pcf in [-1, 1]
                tlx, tly, _, brx, bry, _ = self.bb
                bb_length = bry-tly
                bb_width = brx-tlx
                # r = (int)(0.5*(pred[0][0] + 1.) * bb_length) + tly
                # c = (int)(0.5*(pred[0][1] + 1.) * bb_width) + tlx
                r = (int)(0.5*(pred[0][1] + 1.) * bb_length) + tly
                c = (int)(0.5*(pred[0][0] + 1.) * bb_width) + tlx
                pred_rescale[0][r][c] = 1
                # count = 1
        else: 
            """CNN output is a 2D image
            """
            # pred_img = os.path.join(args.output_path, 'cnn_pred_output_image.png')
            """Check the comment from previous code & change it by self.resize:
            Drop the last axis because it is for the numbers of samples, 
            and we only have 1 sample
            When scaling the CNN output back to image dimensions, 
            note that numpy (pred) is z,y,x and image_size is x,y,z
            """
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

            pred_cast = pred.astype(np.uint8)
            pred_rescale = np.zeros((original_img_shape[0], original_img_shape[1], original_img_shape[2]))
            pred_rescale[tlz:(brz+1), tly:(bry+1), tlx:(brx+1)] = pred_cast   
            # self.log.debug(f'pred_rescale.shape={pred_rescale.shape}')

        # roi_sp = os.path.join(output_path,'pred_'+se_name+'.roi')
        # pred_cast = pred_rescale.astype(np.uint8)
        # pred_image = qimage.from_array(pred_cast, base_img)
        pred_image = qimage.from_array(pred_rescale, template=base_img)

        self.log.debug(f'Written roi size: {pred_image.get_size()}')
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
                            if len(roi_arr.shape) == 2:
                                """point roi (roi_arr) = (y, x)"""
                                """TODO
                                check img_display.shape ordering is (y,x) or (x,y)
                                """
                                rows, cols = img_display.shape
                                # prf, pcf = roi_arr
                                pcf, prf = roi_arr[0] # sinle landmark
                                # if (0<=scaled_prf<=1) and (0<=scaled_pcf<=1):
                                #     tlx, tly, _, brx, bry, _ = self.bb
                                #     prf = (int)(scaled_prf*(bry-tly)) + tly
                                #     pcf = (int)(scaled_pcf*(brx-tlx)) + tlx
                                #     self.log.debug(f'(prs, pcs)={(prf, pcf)}, bb = {self.bb}')

                                axes[i].imshow(img_display, cmap=plt.cm.gray)
                                if (-1 <= prf <= 1) and (-1 <= pcf <= 1):
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

class classification_image_reader(image_reader):
    """Basic Image reader class for loading image formats in CVIB.
    This class supports most of the image formats (2D/3D; 
    single/multi-channel) in CVIB.
    This image reader save processed whole dataset as hdf5, and it
    builds a training dataset by loading .hdf5 file to memory.
    It assume roi_reference is given. For dealing with x,y coordinates,
    use cxr_image_reader.
    We recommand you to use this reader only for small size of dataset.
    For larger dataset, we recommand you to use image_reader instead.

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
        whether to do augmentation on mini-batch for traiing
    self.previous_node_rois : str
        base directory to save rois from previous miu nodes
    self._processed_set: h5py dataset
        private
        processed data from h5py dataset
    self.search_area_attention_type : str
        type of search area attention

    Methods
    -------
    build_training_set(self, overwrite_input=False, workers=1, use_multiprocessing=False)
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
    _load(self, image_path, roi_path=None, input_roi_folder=None, miu_temp_path=None, tmp_roi_name=None)
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
        super(classification_image_reader,self).__init__(config_model, config_resource, log, mode)

    
    def _get_processed_inputs(self, idx):
        raise NotImplementedError()

    def _load(self, image_path, roi_path=None, previous_seg_roi_file=None):
        raise NotImplementedError()
    
    def draw_fig(self, review_fig_dir, img_norm_list, intensity_norm, roi_arr=None, c=4, alhpa=0.7, png_overwrite=False):
        raise NotImplementedError()

    def write_roi(self, pred, base_img, threshold=0.5):
        raise NotImplementedError()

    def _generating_previous_node_rois(self, case_index_list):
        pass

