"""Augmentation classes

common augmentation techniques that will be applied on-the-fly after pre-processing

Spec
----
basic_augmentator
"""
import os, sys
import logging
from matplotlib.animation import ImageMagickBase
import numpy as np
from functools import partial
import matplotlib.pyplot as plt 

from tensorflow.keras.preprocessing.image import apply_affine_transform, apply_brightness_shift

class basic_augmentator(object):
    """Basic Augmentator Class
    Common augmentation techniques that will be applied on-the-fly after pre-processing

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

    """
    def __init__(self, augmentation_types, target_shape, log, seed=None):
        """
        Parameters
        ----------
        augmentation_types : list
            dictionary of model configuration information
        target_shape : tuple
            target_shape = reader.get_target_shape()
            For 3D images, target size of z, x, y dimension.
            For 2D images, target size of x, y dimension.
        log : object
            logging class instance
        """
        self.log = log
        self.augmentation_types = augmentation_types
        self.log.info(f'--------------------------------------------------------------------------')
        self.log.info(f'Configure Augmentation Types')
        self.log.info(f'--------------------------------------------------------------------------')
        self.log.info(f'Augmentation_types: {self.augmentation_types}')
        if self.augmentation_types:
            self.augmentation_function = self._prepare_augmentation_function_stack(target_shape)
            if self.augmentation_function is None:
                self.augment = self._no_augment
                self.log.info(f'No Augmentation based on parsing the given augmentation types.')
            else:
                self.augment = self._augment
                self.log.info(f'Augmentation function assigned.')
        else:
            self.augment = self._no_augment
            self.log.info(f'No Augmentation since no augmentation types are provided.')
        self.log.info(f'--------------------------------------------------------------------------')
        self.img_num = 0
    
    def on_training_start(self):
        pass

    def on_epoch_end(self):
        pass

    def _no_augment(self, mini_batches):
        return mini_batches

    def _augment(self, mini_batches):
        x_augmented_mini_batches = []
        y_augmented_mini_batches = []
        x_mini_batches, y_mini_batches = mini_batches
        for img, ref in zip(x_mini_batches, y_mini_batches):
            # TODO: make base augmentator & make augmentator for point marking / landmark detection
            if len(ref.shape) < 2:
                # point marking
                # For point marking, ref = (prf, pcf) = (point_y, point_x)
                ref_arr = np.zeros_like(img).astype(np.int)
                ref_x = int(ref[1] * img.shape[1])
                ref_y = int(ref[0] * img.shape[0])
                ref_arr[ref_y, ref_x] = 1
                batch = (img,ref_arr)
            elif len(ref.shape) == 2:
                # landmark detection, order : (pcf, prf) = (x, y)
                # ref = [(pcf_1, prf_2), (pcf, prf), ...] = [(point_x_1, point_y_1), (point_x_2, point_y_2), ...]
                ref_lm = ref[0]
                ref_arr = np.zeros_like(img).astype(np.int)
                ref_x = int(ref_lm[0] * img.shape[1])
                ref_y = int(ref_lm[1] * img.shape[0])
                ref_arr[ref_y, ref_x] = 1
                batch = (img,ref_arr)
                # ref_arr_tot = []
                # for ref_lm in ref:
                #     ref_arr = np.zeros_like(img).astype(np.int)
                #     ref_x = int(ref_lm[1] * img.shape[1])
                #     ref_y = int(ref_lm[0] * img.shape[0])
                #     ref_arr[ref_y, ref_x] = 1
                #     ref_arr_tot.append(ref_arr_tot)

            else:
                batch = (img,ref)
            # sys.stdout.flush()
            aug_batch = self.augmentation_function(batch)
            if self._is_marking_exist(aug_batch[-1]):
                if len(ref.shape) < 2:
                    # point marking
                    aug_ref_y, aug_ref_x = np.argwhere(aug_batch[1][:,:,0])[0]
                    aug_ref_x = aug_ref_x / (1.*img.shape[1])
                    aug_ref_y = aug_ref_y / (1.*img.shape[0])
                    x_augmented_mini_batches.append(aug_batch[0])
                    y_augmented_mini_batches.append([aug_ref_y, aug_ref_x])
                elif len(ref.shape) == 2:
                    # landmark detection, order : (pcf, prf) = (x, y)
                    aug_ref_y, aug_ref_x = np.argwhere(aug_batch[1][:,:,0])[0]
                    aug_ref_x = aug_ref_x / (1.*img.shape[1])
                    aug_ref_y = aug_ref_y / (1.*img.shape[0])
                    x_augmented_mini_batches.append(aug_batch[0])
                    y_augmented_mini_batches.append([[aug_ref_x, aug_ref_y]])
                else:
                    x_augmented_mini_batches.append(aug_batch[0])
                    y_augmented_mini_batches.append(aug_batch[1])
            else:
                x_augmented_mini_batches.append(img)
                y_augmented_mini_batches.append(ref)
            
            """save image and marking for debugging point marking problem
            """
            # if len(ref.shape) < 2:
            #     png_org_batch_x = img
            #     # png_org_y, png_org_x = ref[1], ref[0]
            #     png_org_y, png_org_x = int(ref[0] * img.shape[0]), int(ref[1] * img.shape[1])
            #     png_conv_org_y, png_conv_org_x = np.argwhere(ref_arr[:,:,0])[0]
            #     png_aug_batch_x = aug_batch[0]
            #     if self._is_marking_exist(aug_batch[-1]):
            #         # print(aug_batch[1].shape)
            #         # print(np.argwhere(aug_batch[1][:,:,0]))
            #         png_aug_y, png_aug_x = np.argwhere(aug_batch[1][:,:,0])[0]
            #         png_conv_aug_y, png_conv_aug_x = int(aug_ref_y * img.shape[0]), int(aug_ref_x * img.shape[1])
            #     else:
            #         png_aug_y, png_aug_x = int(ref[0] * img.shape[0]), int(ref[1] * img.shape[1])
            #         png_conv_aug_y, png_conv_aug_x = int(ref[0] * img.shape[0]), int(ref[1] * img.shape[1])
            #     fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(10,10))
            #     axes[0].imshow(png_org_batch_x[:,:,0], cmap='gray', vmin=-1, vmax=1)
            #     axes[0].scatter(png_org_x, png_org_y, s=30, c='red', marker='o')
            #     axes[0].scatter(png_conv_org_x, png_conv_org_y, s=30, c='blue', marker='x')
            #     axes[0].set_axis_off()
            #     axes[0].text(0, 0, f'Image', color='black', bbox=dict(facecolor='white', alpha=1))
            #     axes[1].imshow(png_aug_batch_x[:,:,0], cmap='gray', vmin=-1, vmax=1)
            #     axes[1].scatter(png_aug_x, png_aug_y, s=30, c='red', marker='o')
            #     axes[1].scatter(png_conv_aug_x, png_conv_aug_y, s=30, c='blue', marker='x')
            #     axes[1].set_axis_off()
            #     axes[1].text(0, 0, f'Augmented Image', color='black', bbox=dict(facecolor='white', alpha=1))
            #     # fig.show()
            #     fig.savefig(f'/scratch2/personal/mdaly/CXR/miccai2022/experiments/baseline/carina/carina_baseline_v4/augmentation_ss/{self.img_num}.png')
            #     fig.clear()
            #     self.img_num += 1
            #     self.log.debug('DEBUG: saved last image and ref, and augmented img and ref of this mini batch')
            #     sys.stdout.flush()
        return (np.array(x_augmented_mini_batches), np.array(y_augmented_mini_batches))
    
    def _prepare_augmentation_ftn(self, aug_descriptor, target_shape):
        augd_split = aug_descriptor.split('_')
        self.log.debug(f'Add type {aug_descriptor}: {augd_split}')
        i = 0
        ftn = None
        while i < len(augd_split):
            if augd_split[i] == 'xaxisflipping':
                ftn = partial(self._x_axis_flipping)
            elif augd_split[i] == 'yaxisflipping':
                ftn = partial(self._y_axis_flipping)
            elif augd_split[i] == 'zaxisflipping':
                ftn = partial(self._z_axis_flipping)
            elif augd_split[i] == 'randomzoom':
                i += 1
                if i < len(augd_split):
                    zoom_min = float(augd_split[i])            
                    i += 1
                    if i < len(augd_split):
                        zoom_max = float(augd_split[i])
                        if len(target_shape) > 2:
                            # 3D
                            ftn = partial(self._3d_random_zooming, zoom_range=[zoom_min,zoom_max], 
                                        row_axis=0, col_axis=1, channel_axis=2, # per each z slice (batch[i])
                                        fill_mode='nearest', cval=0., interpolation_order=1)
                        else:
                            # 2D
                            ftn = partial(self._2d_random_zooming, zoom_range=[zoom_min,zoom_max], 
                                        row_axis=0, col_axis=1, channel_axis=2,
                                        fill_mode='nearest', cval=0., interpolation_order=1)
            elif augd_split[i] == 'randomshift':
                i += 1
                if i < len(augd_split):
                    wrg = float(augd_split[i])            
                    i += 1
                    if i < len(augd_split):
                        hrg = float(augd_split[i])
                        if len(target_shape) > 2:
                            # 3D
                            ftn = partial(self._3d_random_shift, wrg=wrg, hrg=hrg,
                                        row_axis=0, col_axis=1, channel_axis=2, # per each z slice (batch[i])
                                        fill_mode='nearest', cval=0., interpolation_order=1)
                        else:
                            # 2D
                            ftn = partial(self._2d_random_shift, wrg=wrg, hrg=hrg,
                                        row_axis=0, col_axis=1, channel_axis=2,
                                        fill_mode='nearest', cval=0., interpolation_order=1)
            elif augd_split[i] == 'randombrightness':
                i += 1
                if i < len(augd_split):
                    umin = float(augd_split[i])            
                    i += 1
                    if i < len(augd_split):
                        umax = float(augd_split[i])
                        if len(target_shape) > 2:
                            # 3D
                            ftn = partial(self._3d_random_brightness, brightness_range=[umin, umax])
                        else:
                            # 2D
                            ftn = partial(self._2d_random_brightness, brightness_range=[umin, umax])
            elif augd_split[i] == 'pointjitter':
                i += 1
                if i < len(augd_split):
                    wp = int(augd_split[i])            
                    i += 1
                    if i < len(augd_split):
                        hp = int(augd_split[i])
                        if len(target_shape) > 2:
                            # 3D
                            ftn = partial(self._3d_point_jitter, wp=wp, hp=hp,
                                        row_axis=0, col_axis=1, channel_axis=2, # per each z slice (batch[i])
                                        fill_mode='nearest', cval=0., interpolation_order=1)
                        else:
                            # 2D
                            ftn = partial(self._2d_point_jitter, wp=wp, hp=hp,
                                        row_axis=0, col_axis=1, channel_axis=2,
                                        fill_mode='nearest', cval=0., interpolation_order=1)
            elif augd_split[i] == 'randomrotation':
                i += 1
                if i < len(augd_split):
                    rg = float(augd_split[i])
                    if len(target_shape) > 2:
                        # 3D
                        ftn = partial(self._3d_random_rotation, rg=rg, 
                                    row_axis=0, col_axis=1, channel_axis=2, # per each z slice (batch[i])
                                    fill_mode='nearest', cval=0., interpolation_order=1)
                    else:
                        # 2D
                        ftn = partial(self._2d_random_rotation, rg=rg, 
                                    row_axis=0, col_axis=1, channel_axis=2,
                                    fill_mode='nearest', cval=0., interpolation_order=1)
            elif augd_split[i]!='none': #then the syntax is not valid
                i = len(augd_split)
            i += 1
        if i==(len(augd_split)+1):
            self.log.error('ERROR: failed to parse: ', aug_descriptor)
            sys.stdout.flush()
        return ftn

    def _prepare_augmentation_function_stack(self, target_shape):
        # list_functions = [self._pass]
        list_functions = []
        self.log.debug(f'Parsing {self.augmentation_types}')
        for aug_descriptor in self.augmentation_types:
            ftn = self._prepare_augmentation_ftn(aug_descriptor, target_shape)
            if ftn is not None: list_functions.append(ftn)
        # self.log.debug('Check list of augmentation functions')
        # self.log.debug(list_functions)
        if len(list_functions):
            def augmentation_function(x):
                for func in list_functions:
                    x = func(x)
                return x
            return augmentation_function
        else:
            return None

    def _is_marking_exist(self, x):
        return np.sum(x) > 0

    def _x_axis_flipping(self, batch):
        # self.log.debug('check x_axis_flipping')
        if np.random.choice([True, False]):
            # self.log.debug('x_axis_flipping')
            # sys.stdout.flush()
            augmented = []
            if len(batch[0].shape) > 3:
                for arr in batch:
                    """3D input"""
                    augmented.append(arr[:,::-1,:])
            else:
                for arr in batch:
                    """2D input"""
                    augmented.append(arr[::-1,:])
        else: augmented = batch
        return augmented

    def _y_axis_flipping(self, batch):
        # self.log.debug('check y_axis_flipping')
        if np.random.choice([True, False]):
            # self.log.debug(f'{len(batch)}, {batch[0].shape}, {batch[1].shape}')
            # self.log.debug('y_axis_flipping')
            # sys.stdout.flush()
            augmented = []
            if len(batch[0].shape) > 3:
                for arr in batch:
                    """3D input"""
                    augmented.append(arr[:,:,::-1])
            else:
                for arr in batch:
                    """2D input"""
                    augmented.append(arr[:,::-1])
        else: augmented = batch
        return augmented
    
    def _z_axis_flipping(self, batch):
        # self.log.debug('check z_axis_flipping')
        if np.random.choice([True, False]):
            # self.log.debug('z_axis_flipping')
            sys.stdout.flush()
            augmented = []
            if len(batch[0].shape) > 3:
                for arr in batch:
                    """3D input"""
                    augmented.append(arr[::-1,:,:])
            else:
                for arr in batch:
                    """2D input"""
                    raise ValueError('No Z axis flipping for 2D')
        else: augmented = batch
        return augmented

    def _2d_random_zooming(self, batch, zoom_range, row_axis=0, col_axis=1, channel_axis=2,
        fill_mode='nearest', cval=0., interpolation_order=1):
        # https://github.com/keras-team/keras-preprocessing/blob/1.1.2/keras_preprocessing/image/affine_transformations.py#L121-L156
        if len(zoom_range) != 2:
            raise ValueError('`zoom_range` should be a tuple or list of two'
                            ' floats. Received: %s' % (zoom_range,))
        if zoom_range[0] == 1 and zoom_range[1] == 1:
            zx, zy = 1, 1
        else:
            # zx, zy = np.random.uniform(zoom_range[0], zoom_range[1], 2)
            zx = np.random.uniform(zoom_range[0], zoom_range[1])
            zy = zx
        # self.log.debug(f'Random zooming with zx={zx}, zy={zy}')
        # sys.stdout.flush()
        augmented = []
        for arr in batch:
            aug_arr = apply_affine_transform(arr, zx=zx, zy=zy, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                fill_mode=fill_mode, cval=cval,
                                order=interpolation_order)
            augmented.append(aug_arr)
        return augmented

    def _3d_random_zooming(self, batch, zoom_range, row_axis=0, col_axis=1, channel_axis=2,
        fill_mode='nearest', cval=0., interpolation_order=1):
        # https://github.com/keras-team/keras-preprocessing/blob/1.1.2/keras_preprocessing/image/affine_transformations.py#L121-L156
        if len(zoom_range) != 2:
            raise ValueError('`zoom_range` should be a tuple or list of two'
                            ' floats. Received: %s' % (zoom_range,))
        if zoom_range[0] == 1 and zoom_range[1] == 1:
            zx, zy = 1, 1
        else:
            # zx, zy = np.random.uniform(zoom_range[0], zoom_range[1], 2)
            zx = np.random.uniform(zoom_range[0], zoom_range[1])
            zy = zx
        # self.log.debug(f'Random zooming with zx={zx}, zy={zy}')
        # print(f'Random zooming with zx={zx}, zy={zy}')
        augmented = []
        for arr in batch:
            aug_tot = []
            for arr_slice in arr:
                aug_arr_slice = apply_affine_transform(arr_slice, zx=zx, zy=zy, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                    fill_mode=fill_mode, cval=cval,
                                    order=interpolation_order)
                aug_tot.append(aug_arr_slice)
            augmented.append(aug_tot)
        return augmented

    def _2d_random_rotation(self, batch, rg, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        # https://github.com/keras-team/keras-preprocessing/blob/1.1.2/keras_preprocessing/image/affine_transformations.py#L121-L156
        # self.log.debug('check _2d_random_rotation')
        if np.random.choice([True, False]):
            theta = np.random.uniform(-rg, rg)
            # self.log.debug(f'Random rotation with theta={theta}')
            # sys.stdout.flush()
            augmented = []
            for arr in batch:
                aug_arr =  apply_affine_transform(arr, theta=theta, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                    fill_mode=fill_mode, cval=cval,
                                    order=interpolation_order)
                augmented.append(aug_arr)
        else: augmented = batch
        return augmented
    
    def _3d_random_rotation(self, batch, rg, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        # https://github.com/keras-team/keras-preprocessing/blob/1.1.2/keras_preprocessing/image/affine_transformations.py#L121-L156
        # self.log.debug('check _3d_random_rotation')
        if np.random.choice([True, False]):
            theta = np.random.uniform(-rg, rg)
            # self.log.debug(f'Random rotation with theta={theta}')
            # sys.stdout.flush()
            augmented = []
            for arr in batch:
                aug_tot = []
                for arr_slice in arr:
                    aug_arr_slice = apply_affine_transform(arr_slice, theta=theta, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                            fill_mode=fill_mode, cval=cval,
                                            order=interpolation_order)
                    aug_tot.append(aug_arr_slice)
                augmented.append(aug_tot)
        else: augmented = batch
        return augmented

    def _2d_random_shift(self, batch, wrg, hrg, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        """Performs a random spatial shift of a Numpy image tensor.
        # Arguments
            x: Input tensor. Must be 3D.
            wrg: Width shift range, as a float fraction of the width.
            hrg: Height shift range, as a float fraction of the height.
            row_axis: Index of axis for rows in the input tensor.
            col_axis: Index of axis for columns in the input tensor.
            channel_axis: Index of axis for channels in the input tensor.
            fill_mode: Points outside the boundaries of the input
                are filled according to the given mode
                (one of `{'constant', 'nearest', 'reflect', 'wrap'}`).
            cval: Value used for points outside the boundaries
                of the input if `mode='constant'`.
            interpolation_order: int, order of spline interpolation.
                see `ndimage.interpolation.affine_transform`
        # Returns
            Shifted Numpy image tensor.
        #https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_shift
        """
        # self.log.debug('check _2d_random_shift')
        if np.random.choice([True, False]):
            h, w = batch[0].shape[row_axis], batch[0].shape[col_axis]
            tx = np.random.uniform(-wrg, wrg) * w
            ty = np.random.uniform(-hrg, hrg) * h
            # self.log.debug(f'Random 2d shift with tx={tx}, ty={ty}')
            # sys.stdout.flush()
            augmented = []
            for arr in batch:
                aug_arr =  apply_affine_transform(arr, tx=tx, ty=ty, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                    fill_mode=fill_mode, cval=cval,
                                    order=interpolation_order)
                augmented.append(aug_arr)
        else: augmented = batch
        return augmented

    def _3d_random_shift(self, batch, wrg, hrg, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        """Performs a random spatial shift of a Numpy image tensor.
        # Arguments
            x: Input tensor. Must be 4D.
            wrg: Width shift range, as a float fraction of the width.
            hrg: Height shift range, as a float fraction of the height.
            row_axis: Index of axis for rows in the input tensor.
            col_axis: Index of axis for columns in the input tensor.
            channel_axis: Index of axis for channels in the input tensor.
            fill_mode: Points outside the boundaries of the input
                are filled according to the given mode
                (one of `{'constant', 'nearest', 'reflect', 'wrap'}`).
            cval: Value used for points outside the boundaries
                of the input if `mode='constant'`.
            interpolation_order: int, order of spline interpolation.
                see `ndimage.interpolation.affine_transform`
        # Returns
            Shifted Numpy image tensor.
        #https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_shift
        """
        # self.log.debug('check _3d_random_shift')
        if np.random.choice([True, False]):
            h, w = batch[0][0].shape[row_axis], batch[0][0].shape[col_axis]
            tx = np.random.uniform(-wrg, wrg) * w
            ty = np.random.uniform(-hrg, hrg) * h
            # self.log.debug(f'Random 3d shift with tx={tx}, ty={ty}')
            # sys.stdout.flush()
            augmented = []
            for arr in batch:
                aug_tot = []
                for arr_slice in arr:
                    aug_arr_slice = apply_affine_transform(arr_slice, tx=tx, ty=ty, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                        fill_mode=fill_mode, cval=cval,
                                        order=interpolation_order)
                    aug_tot.append(aug_arr_slice)
                augmented.append(aug_tot)
        else: augmented = batch
        return augmented
        
    def _2d_point_jitter(self, batch, wp, hp, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        """Performs a random spatial shift of a Numpy image tensor.
        # Arguments
            x: Input tensor. Must be 3D.
            hp: Width shift pixel, as a intiger number of the hight pixel.
            wp: Height shift range, as a intiger number of the width pixel.
            row_axis: Index of axis for rows in the input tensor.
            col_axis: Index of axis for columns in the input tensor.
            channel_axis: Index of axis for channels in the input tensor.
            fill_mode: Points outside the boundaries of the input
                are filled according to the given mode
                (one of `{'constant', 'nearest', 'reflect', 'wrap'}`).
            cval: Value used for points outside the boundaries
                of the input if `mode='constant'`.
            interpolation_order: int, order of spline interpolation.
                see `ndimage.interpolation.affine_transform`
        # Returns
            Shifted Numpy image tensor.
        #https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_shift
        """
        # self.log.debug('check _2d_point_jitter')
        if np.random.choice([True, False]): 
            wpr = np.random.choice(range(0,wp))
            hpr = np.random.choice(range(0,hp))
            # self.log.debug(f'{len(batch)}, {batch[0].shape}')
            # self.log.debug(f'Random 2d point jitter with wpr={wpr}, hpr={hpr}')
            # sys.stdout.flush()
            augmented = [batch[0]]
            for arr in batch[1:]:
                aug_arr =  apply_affine_transform(arr, tx=wpr, ty=hpr, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                    fill_mode=fill_mode, cval=cval,
                                    order=interpolation_order)
                augmented.append(aug_arr)
        else: augmented = batch
        return augmented
    
    def _3d_point_jitter(self, batch, wp, hp, row_axis=0, col_axis=1, channel_axis=2, fill_mode='nearest',
                        cval=0.0, interpolation_order=1):
        """Performs a random spatial shift of a Numpy image tensor.
        # Arguments
            x: Input tensor. Must be 4D.
            hp: Width shift pixel, as a intiger number of the hight pixel.
            wp: Height shift range, as a intiger number of the width pixel.
            row_axis: Index of axis for rows in the input tensor.
            col_axis: Index of axis for columns in the input tensor.
            channel_axis: Index of axis for channels in the input tensor.
            fill_mode: Points outside the boundaries of the input
                are filled according to the given mode
                (one of `{'constant', 'nearest', 'reflect', 'wrap'}`).
            cval: Value used for points outside the boundaries
                of the input if `mode='constant'`.
            interpolation_order: int, order of spline interpolation.
                see `ndimage.interpolation.affine_transform`
        # Returns
            Shifted Numpy image tensor.
        #https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_shift
        """
        # self.log.debug('check _3d_point_jitter')
        if np.random.choice([True, False]):
            wpr = np.random.choice(range(0,wp))
            hpr = np.random.choice(range(0,hp))
            # self.log.debug(f'Random 3d point jitter with wpr={wpr}, hpr={hpr}')
            # sys.stdout.flush()
            augmented = [batch[0]]
            for arr in batch[1:]:
                aug_tot = []
                for arr_slice in arr:
                    aug_arr_slice = apply_affine_transform(arr_slice, tx=wpr, ty=hpr, row_axis=row_axis, col_axis=col_axis, channel_axis=channel_axis,
                                        fill_mode=fill_mode, cval=cval,
                                        order=interpolation_order)
                    aug_tot.append(aug_arr_slice)
                augmented.append(aug_tot)
        else: augmented = batch
        return augmented

    def _2d_random_brightness(self, batch, brightness_range):
        """Performs a random brightness shift.
        # Arguments
            x: Input tensor. Must be 3D.
            brightness_range: Tuple of floats; brightness range.
            channel_axis: Index of axis for channels in the input tensor.
        # Returns
            Numpy image tensor.
        # Raises
            ValueError if `brightness_range` isn't a tuple.
        https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_brightness
        """
        # self.log.debug(f'Random 2d brightness with brightness_range={brightness_range}')
        # self.log.debug('check _2d_random_brightness')
        if np.random.choice([True, False]):
            if len(brightness_range) != 2:
                raise ValueError(
                    '`brightness_range should be tuple or list of two floats. '
                    'Received: %s' % (brightness_range,))
            u = np.random.uniform(brightness_range[0], brightness_range[1])
            aug_x = apply_brightness_shift(batch[0], u)
            return [aug_x] + batch[1:]
        else: return batch
    
    def _3d_random_brightness(self, batch, brightness_range):
        """Performs a random brightness shift.
        # Arguments
            x: Input tensor. Must be 4D.
            brightness_range: Tuple of floats; brightness range.
            channel_axis: Index of axis for channels in the input tensor.
        # Returns
            Numpy image tensor.
        # Raises
            ValueError if `brightness_range` isn't a tuple.
        https://www.tensorflow.org/api_docs/python/tf/keras/preprocessing/image/random_brightness
        """
        # self.log.debug(f'Random 3d brightness with brightness_range={brightness_range}')
        # self.log.debug('check _3d_random_brightness')
        if np.random.choice([True, False]):
            if len(brightness_range) != 2:
                raise ValueError(
                    '`brightness_range should be tuple or list of two floats. '
                    'Received: %s' % (brightness_range,))
            u = np.random.uniform(brightness_range[0], brightness_range[1])
            aug_x_tot = []
            for aug_arr_slice in batch[0]:
                aug_x = apply_brightness_shift(aug_arr_slice, u)
                aug_x_tot.append(aug_x)
            return [aug_x_tot] + batch[1:]
        else: return batch

    # def _random_contrast(self, batch):
    #     self.log.debug('Try random contrast')
    #     return batch
    
    # def _random_saturation(self, batch):
    #     self.log.debug('Try random saturation')
    #     return batch



class cxr_multitask_augmentator(basic_augmentator):
    """CXR Multitask Augmentator Class
    Common augmentation techniques that will be applied on-the-fly after pre-processing

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

    """
    def __init__(self, augmentation_types, target_shape, log, seed=None):
        """
        Parameters
        ----------
        augmentation_types : list
            dictionary of model configuration information
        target_shape : tuple
            target_shape = reader.get_target_shape()
            For 3D images, target size of z, x, y dimension.
            For 2D images, target size of x, y dimension.
        log : object
            logging class instance
        """
        super(cxr_multitask_augmentator,self).__init__(augmentation_types, target_shape, log, seed=seed)
        
    def _augment(self, mini_batches):
        x_augmented_mini_batches = []
        carina_y_augmented_mini_batches = []
        ettip_y_augmented_mini_batches = []
        x_mini_batches, y_mini_batches = mini_batches
        carina_y_mini_batches = y_mini_batches[0]
        ettip_y_mini_batches = y_mini_batches[1]
        for img, carina_ref, ettip_ref in zip(x_mini_batches, carina_y_mini_batches, ettip_y_mini_batches):
            if len(carina_ref.shape) < 2:
                # point marking
                # For point marking, ref = (prf, pcf) = (point_y, point_x)
                carina_ref_arr = np.zeros_like(img).astype(np.int)
                carina_ref_x = int(carina_ref[1] * img.shape[1])
                carina_ref_y = int(carina_ref[0] * img.shape[0])
                carina_ref_arr[carina_ref_y, carina_ref_x] = 1
                ettip_ref_arr = np.zeros_like(img).astype(np.int)
                ettip_ref_x = int(ettip_ref[1] * img.shape[1])
                ettip_ref_y = int(ettip_ref[0] * img.shape[0])
                ettip_ref_arr[ettip_ref_y, ettip_ref_x] = 1
                batch = (img, carina_ref_arr, ettip_ref_arr)
            else:
                batch = (img, carina_ref, ettip_ref)
            # self.log.debug(f'carina ref: {self._is_marking_exist(carina_ref_arr)}')
            # self.log.debug(f'ettip ref: {self._is_marking_exist(ettip_ref_arr)}')
            # sys.stdout.flush()
            aug_imaage, carina_aug_arr, ettip_aug_arr = self.augmentation_function(batch)
            # sys.stdout.flush()
            # self.log.debug(f'carina aug: {self._is_marking_exist(carina_aug_arr)}')
            # sys.stdout.flush()
            # self.log.debug(f'ettip aug: {self._is_marking_exist(ettip_aug_arr)}')
            # sys.stdout.flush()
            if (self._is_marking_exist(carina_aug_arr)) and (self._is_marking_exist(ettip_aug_arr)):
                if len(carina_ref.shape) < 2:
                    # point marking
                    carina_aug_ref_y, carina_aug_ref_x = np.argwhere(carina_aug_arr[:,:,0])[0]
                    carina_aug_ref_x = carina_aug_ref_x / (1.*img.shape[1])
                    carina_aug_ref_y = carina_aug_ref_y / (1.*img.shape[0])

                    ettip_aug_ref_y, ettip_aug_ref_x = np.argwhere(ettip_aug_arr[:,:,0])[0]
                    ettip_aug_ref_x = ettip_aug_ref_x / (1.*img.shape[1])
                    ettip_aug_ref_y = ettip_aug_ref_y / (1.*img.shape[0])
                    x_augmented_mini_batches.append(aug_imaage)
                    carina_y_augmented_mini_batches.append([carina_aug_ref_y, carina_aug_ref_x])
                    ettip_y_augmented_mini_batches.append([ettip_aug_ref_y, ettip_aug_ref_x])
                    # self.log.debug('augmentation for point marking')
                    # sys.stdout.flush()
                else:
                    x_augmented_mini_batches.append(aug_imaage)
                    carina_y_augmented_mini_batches.append(carina_aug_arr)
                    ettip_y_augmented_mini_batches.append(ettip_aug_arr)
                    # self.log.debug('No point marking')
                    # sys.stdout.flush()
            else:
                x_augmented_mini_batches.append(img)
                carina_y_augmented_mini_batches.append(carina_ref)
                ettip_y_augmented_mini_batches.append(ettip_ref)
            #     self.log.debug('augmentation fail for point marking')
            #     sys.stdout.flush()
            # self.log.debug('-----------------------------------------------------------------------------------')
            # sys.stdout.flush()
            
        # """save image and marking for debugging point marking problem
        # """
        # fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(10,10))
        # png_org_batch_x = img
        # axes[0].imshow(png_org_batch_x[:,:,0], cmap='gray', vmin=-1, vmax=1)
        # for i, (ref, ref_arr, ref_name, col) in enumerate(zip([carina_ref, ettip_ref], [carina_ref_arr, ettip_ref_arr], ['carina', 'ettip'], ['red','blue'])):
        #     png_org_y, png_org_x = int(ref[0] * img.shape[0]), int(ref[1] * img.shape[1])
        #     axes[0].scatter(png_org_x, png_org_y, s=10, c=col, marker='o', label=f'{ref_name}_org')
        #     png_conv_org_y, png_conv_org_x = np.argwhere(ref_arr[:,:,0])[0]
        #     axes[0].scatter(png_conv_org_x, png_conv_org_y, s=10, c=col, marker='x', label=f'{ref_name}_conv_org')
        # axes[0].set_axis_off()
        # axes[0].text(0, 0, f'Image', color='black', bbox=dict(facecolor='white', alpha=1))

        # png_aug_batch_x = x_augmented_mini_batches[-1]
        # carina_aug_ref = carina_y_augmented_mini_batches[-1]
        # ettip_aug_ref = ettip_y_augmented_mini_batches[-1]
        # axes[1].imshow(png_aug_batch_x[:,:,0], cmap='gray', vmin=-1, vmax=1)
        # for i, (aug_ref, aug_arr, ref_name, col) in enumerate(zip([carina_aug_ref, ettip_aug_ref], [carina_aug_arr, ettip_aug_arr], ['carina', 'ettip'], ['red', 'blue'])):
        #     png_aug_y, png_aug_x = int(aug_ref[0] * img.shape[0]), int(aug_ref[1] * img.shape[1])
        #     axes[1].scatter(png_aug_x, png_aug_y, s=10, c=col, marker='o', label=f'{ref_name}_aug')
        #     png_aug_conv_y, png_aug_conv_x = np.argwhere(aug_arr[:,:,0])[0]
        #     axes[1].scatter(png_aug_conv_x, png_aug_conv_y, s=10, c=col, marker='x', label=f'{ref_name}_conv_aug')
        # axes[1].set_axis_off()
        # axes[1].text(0, 0, f'Augmented Image', color='black', bbox=dict(facecolor='white', alpha=1))
        # fig.legend()
        # # fig.show()
        # fig.savefig(f'/scratch2/personal/mdaly/CXR/miccai2022/experiments/multitask/multitask_v0/augmentation_ss/{self.img_num}.png')
        # fig.clear()
        # self.img_num += 1
        # self.log.debug('------------------------------------------------------------------------------------')
        # self.log.debug('DEBUG: saved last image and ref, and augmented img and ref of this mini batch')
        # self.log.debug('------------------------------------------------------------------------------------')
        # sys.stdout.flush()

        return (np.array(x_augmented_mini_batches), [np.array(carina_y_augmented_mini_batches), np.array(ettip_y_augmented_mini_batches)])
    