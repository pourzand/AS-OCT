"""Neural Network classes family for CNN

Spec
----
ABC neural_network
segmentation_network

TODO 
---
@youngwonchoi
    1. neural_network
        - warm-start
    1. segmentation_network
    1. tube_detection_network ? cxr_network?
    1. detection_network ? nodule..
    1. classification_network
"""

import os, sys
import json
import timeit, time
import random
import numpy as np
from numpy.core.numeric import True_
import tensorflow as tf
import keras.callbacks
from keras.utils import multi_gpu_model
# if not tf.__version__.startswith('2'):
#     import keras.backend as K

from keras.layers import Input
from keras.models import Model

"""For Python3.5+"""
import importlib.util

# import __qia__
# import cnntools as cnntls
from cnn_arch import get_cnn_arch
import cnn_loss_and_metrics
import cnn_tensorboard_utils
import cnn_layer_attention_family

import os
os.environ["HDF5_USE_FILE_LOCKING"] = "FALSE"

class neural_network(object):
    """Neural Network Class
    Abstract base class of a class family of neural networks.
    Primal example of the child calss is segmentation_network class. 
    This subclass supports most of the segmentation tasks
    various image inputs (2D/3D; single/multi-channel) in CVIB.

    Parameters
    ----------
    config_model : dict
        dictionary of model configuration information
    log : object
        logging class instance

    Attributes
    ----------
    self.config_model : dict
        dictionary of model configuration information
    self.config_resource : dict
        dictionary of resource configuration information
    self.log : object
        logging class instance
    self.network : keras.model.Model
        CNN model for calculation
        keras.model.Model instance
    self.multi_gpu_network : object
        keras multi-gpu model
        keras.utils.multi_gpu_model output
    self.number_of_gpu : int
        number of gpus for calculation
    self.max_queue_size : int
        number of mini-batches to be queued when
        waiting for GPU calculation
    self.workers : int
        number of cpu workers
    self.use_multiprocessing : bool
        whether to use multiprocessing
    self.verbose : int
        logging level based on the self.log.level
    
    Methods
    -------
    set_network(self, img_shape=(None, None), img_channels=1)
        Call CNN architecture
    predict(self, x, batch_size = 1)
        Prediction without generator
    predict_generator(self, sampler, max_queue_size=10, workers=1, use_multiprocessing=False)
        Prediction with generator

    
    Usage
    -----
    You can inherit this class to implement a new 
    neural network class with a own loss and/or prediction
    function, if you need to specify a loss function for
    specific tasks or formats.

    Examples
    --------

    """
    def __init__(self, config_model, config_resource, log):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        """
        self.config_model = config_model
        self.config_resource = config_resource
        self.log = log
        self.network = None

        simple_tb = self.config_model['tensorboard_info']['simple_tb'].strip().lower()
        if simple_tb == 'true': self.simple_tb = True
        else: self.simple_tb = False
        self.TB = cnn_tensorboard_utils.TensorBoardSimpleWrapper
        
        if self.log.level <= 10: self.verbose = 2 # debug
        elif self.log.level <= 20: self.verbose = 1 # info
        else: self.verbose = 0 # warning

        node_name = self.config_model['model_info']['node_name']
        h5_bit_tag = self.config_model['chromosome_info']['input_tag']
        hd5_bit_tag = self.config_model['chromosome_info']['weights_tag']
        weight_path = self.config_model['path_info']['weights_file'].strip()
        working_dir = self.config_model['path_info']['working_directory'].strip()

        """Search area attention"""
        search_area_attention_type=self.config_model['search_area_attention_info']['attention_type'].strip().lower()
        if search_area_attention_type !="none":
            self.log.info('---------------------------------------------------------------')
            self.log.info('Architecture: search area attention')
            self.log.info('---------------------------------------------------------------')
            self.search_area_attention_type = f'SearchAreaAttention{search_area_attention_type.capitalize()}'
            self.log.info(f'Search area attention type: {self.search_area_attention_type}')
            try:
                self.search_area_attention_class = getattr(cnn_layer_attention_family, self.search_area_attention_type)
                self.log.debug(f'Search area attention class: {self.search_area_attention_class}')
                if not self.simple_tb:
                    self.TB = cnn_tensorboard_utils.TensorBoardSegmentationAttentionWrapper
                self.log.debug(f'Tensorboard for Search area attention will be used.')
            except Exception as e:
                self.search_area_attention_type = None
                self.log.info(f'Fail to find the proper search area attention class with error {e}.')
                self.log.info('Search area attention will be ignored.')
                self.log.info(f'Search area attention type: {self.search_area_attention_type}')
            """Search area attention: attention_initial"""            
            attention_initial_path = self.config_model['search_area_attention_info']['attention_initial'].strip()
            if attention_initial_path !="none":
                self.log.debug(f'Numpy array for attention_initial will be loaded from {attention_initial_path}.')
                self.attention_initial = np.load(attention_initial_path)
                # try: shape check
                #     assert (self.attention_initial.shape == )
                # except:
                #     self.attention_initial = None
                self.log.debug(f'Initial weight for search area attention is specified with shape: {self.attention_initial.shape}.')
            else:
                self.attention_initial = None
                self.log.debug(f'Initial weight for search area attention is not specified. "one" will be used.')
            self.log.debug(f'Numpy array for attention_initial will be loaded from {attention_initial_path}.')

            """Search area attention: intensity"""            
            intensity_value=self.config_model['search_area_attention_info']['intensity_value'].strip()
            try:
                self.intensity_value = float(intensity_value)
                assert ((self.intensity_value >= 0.) and (self.intensity_value <= 1.))
            except: 
                self.log.info(f'Wrong intensity value input: {self.intensity_value}.')
                self.log.info('Intensity value should be a float and within a range [0., 1.].')
                self.log.info('Intensity value will be ignored.')
                self.intensity_value = None
            self.log.info(f'Search area attention intensity value: {self.intensity_value}')
            intensity_trainable=self.config_model['search_area_attention_info']['intensity_trainable'].strip()
            if intensity_trainable == 'true': 
                self.intensity_trainable = True
            else: 
                self.intensity_trainable = False
            self.log.info(f'Search area attention intensity trainable: {self.intensity_trainable}')
        else: 
            self.search_area_attention_type = None
            self.log.debug("No search area attention.")
            self.log.debug(f'Search area attention type: {self.search_area_attention_type}')
        
        """trained weight"""
        if os.path.exists(weight_path):
            self.weight_path = weight_path
            self.saved_weight_path = weight_path
        else:
            self.saved_weight_path = None
            if os.path.exists(working_dir):
                working_dir = os.path.join(working_dir, f'{node_name}_KerasModel')
                os.makedirs(working_dir, exist_ok=True)
                os.makedirs(os.path.join(working_dir, 'weights'), exist_ok=True)
                self.weight_dir = os.path.join(working_dir, 'weights', hd5_bit_tag)
                weight_file_name = f'{node_name}_weights_{hd5_bit_tag}.h5'
                # weight_file_name = "weights.{epoch:02d}-{val_loss:1.4f}.h5" # change to .hd5 file for now
                self.weight_path = os.path.join(self.weight_dir, weight_file_name)
                
                self.logdir = os.path.join(working_dir, 'logs')
                self.csv_logpath = os.path.join(self.logdir, f'{node_name}_{hd5_bit_tag}.csv')
                self.hist_logpath = os.path.join(self.logdir, f'{node_name}_{hd5_bit_tag}.json')
                skip_tb = self.config_model['switch_from_miu_info']['skip_tb'].strip().lower()
                self.log.debug(f'Skip tensorboard logging from miu argument: {skip_tb}')
                if (skip_tb == 'true'): self.skip_tb = True
                else: self.skip_tb = False
                self.log.debug(f'Skip tensorboared logging: {self.skip_tb}')
                if self.skip_tb:
                    self.tensorboard_logdir = None
                else:
                    self.tensorboard_logdir = os.path.join(working_dir, 'tb_logs', f'{node_name}_{hd5_bit_tag}_{h5_bit_tag}')
            else:
                raise ValueError(f'Can not find both the weight file:{weight_path}\n\
                        and working directory:{working_dir}\n\
                        You should specifiy at least one of them.')
            
            warm_start_model=self.config_model['path_info']['warm_start_model'].strip()
            if os.path.exists(working_dir) and warm_start_model !="":
                self.warm_start_model = warm_start_model
                self.warm_start=True
            else:
                self.warm_start_model = None
                self.warm_start=False
        
        """Resource control"""
        """1. GPU control parameters"""
        self.log.info('---------------------------------------------------------------')
        self.log.info('GPU resources')
        self.log.info('---------------------------------------------------------------')
        gpu_cores =config_resource['GPU']['gpu_cores'].strip()
        memory_growth = config_resource['GPU']['memory_growth'].strip().lower()=='true'
        try: memory_limit = float(config_resource['GPU']['memory_limit'])
        except: memory_limit = None
        self.log.info(f'Given gpu_cores number: {gpu_cores}')
        self.log.info(f'memory_growth: {memory_growth}')
        self.log.info(f'memory_limit: {memory_limit}')
        
        if len(gpu_cores) > 0:
            gpu_config = tf.compat.v1.config
            self.log.info(f'Finding GPU cores {gpu_cores}...')
            available_gpus = tf.config.experimental.list_physical_devices('GPU')
            self.log.info(f'All Available GPUs: {available_gpus}')
            catched_gpus = [catched for catched in available_gpus if catched.name.split(':')[-1] in gpu_cores and 'GPU' in catched.name]
            if memory_growth:
                gpu_config.experimental.set_visible_devices(catched_gpus, 'GPU')
                for gpu in catched_gpus:
                    gpu_config.experimental.set_memory_growth(gpu, True)
            else:
                for gpu in catched_gpus:
                    gpu_config.experimental.set_virtual_device_configuration(gpu, [tf.config.experimental.VirtualDeviceConfiguration(memory_limit=memory_limit)])
                    self.log.debug(f'Set the virtual device: {gpu_config.experimental.get_virtual_device_configuration(gpu)}')
                gpu_config.experimental.set_visible_devices(catched_gpus, 'GPU')
        else:
            os.environ["CUDA_VISIBLE_DEVICES"] = "-1"
            gpu_config = tf.compat.v1.config
            # gpu_config.experimental.set_visible_devices([], 'GPU')
            self.log.info(f'Using no GPU cores....')
        logical_gpus = gpu_config.experimental.list_logical_devices('GPU')
        # logical_devices = gpu_config.experimental.list_logical_devices()
        self.number_of_gpu = len(logical_gpus)
        self.log.info(f'Choosed gpu resouces: {logical_gpus}')
        self.log.info(f'Choosed number of GPU: {self.number_of_gpu}')
        self.multi_gpu_network = None

        """2. CPU control parameters"""
        self.log.info('---------------------------------------------------------------')
        self.log.info('CPU resources')
        self.log.info('---------------------------------------------------------------')
        self.max_queue_size=int(config_resource['CPU']['max_queue_size'])
        self.workers=int(config_resource['CPU']['num_cpu_core'])
        self.use_multiprocessing=config_resource['CPU']['use_multiprocessing'].strip().lower()=='true'
        self.log.info(f'max_queue_size: {self.max_queue_size}')
        self.log.info(f'use_multiprocessing: {self.use_multiprocessing}')
        self.log.info(f'num_cpu_core: {self.workers}')
        
        """
        TODO: fix the way only saving best model
        check @wasilwahi branch
        """
        self.best_model_save = False
        
        """3. Set seed
        TODO: dicsussion needed how to handle seed num.
        """
        # 1. Set seed
        seed_value = 123
        # 1. Set `PYTHONHASHSEED` environment variable at a fixed value
        os.environ['PYTHONHASHSEED']=str(seed_value)
        # 2. Set `python` built-in pseudo-random generator at a fixed value
        random.seed(seed_value)
        # 3. Set `numpy` pseudo-random generator at a fixed value
        np.random.seed(seed_value)
        # 4. Set `tensorflow` pseudo-random generator at a fixed value
        if tf.__version__.startswith('2'): tf.random.set_seed(seed_value)
        else: tf.set_random_seed(seed_value)
        
    def _get_callbacks(self, validation_data=None):
        """private function to get callbacks
        """
        try:
            callback_list = self.config_model['callbacks_info']['callbacks'].split(',')
            callbacks = [cb.strip() for cb in callback_list]
            self.log.debug(f'Callback list: {callbacks}')
        except:
            callbacks = []

        """TODO
        Ugly.... find better way..
        """

        for idx, callback_name in enumerate(callback_list):
            if "CSVLogger".lower() in callback_name:
                if not os.path.exists(self.logdir):
                    os.makedirs(self.logdir)
                self.log.info(f'CSVLogger will log the training in {self.csv_logpath}')
                csv_logger = getattr(keras.callbacks, 'CSVLogger')(self.csv_logpath)
                callbacks[idx] = csv_logger
            elif "EarlyStopping".lower() in callback_name:
                monitor = self.config_model['callbacks_info']['monitor']
                mode = self.config_model['callbacks_info']['callback_mode']
                patience = int(self.config_model['callbacks_info']['patience'])
                min_delta = float(self.config_model['callbacks_info']['min_delta'])                
                callback = getattr(keras.callbacks, 'EarlyStopping')
                callbacks[idx] = callback(monitor=monitor, mode=mode,
                                          patience=patience,
                                          min_delta=min_delta,
                                          verbose=self.verbose)
            elif "ModelCheckpoint".lower() in callback_name:
                self.log.info(f'ModelCheckpoint will update the weights at {self.weight_path}')
                self.best_model_save = True
                monitor = self.config_model['callbacks_info']['monitor']
                mode = self.config_model['callbacks_info']['callback_mode']
                callback = getattr(keras.callbacks, 'ModelCheckpoint')
                callbacks[idx] = callback(filepath=self.weight_path,
                                          monitor=monitor, mode=mode,
                                          save_best_only=True,
                                          save_weights_only=True,
                                          verbose=self.verbose)
            elif "ReduceLROnPlateau".lower() in callback_name:
                monitor = self.config_model['callbacks_info']['monitor']
                factor = float(self.config_model['callbacks_info']['lr_factor'])
                patience = int(self.config_model['callbacks_info']['lr_patience'])
                cooldown = 2
                min_lr = 1e-12
                callback = getattr(keras.callbacks, 'ReduceLROnPlateau')
                reduce_lr = callback(monitor=monitor, factor=factor, 
                                    patience=patience, cooldown=cooldown, min_lr=min_lr,
                                    verbose=self.verbose)
                self.log.info('Reducing learning rate on plateau.')
                callbacks[idx] = reduce_lr
            else: callbacks[idx] = getattr(keras.callbacks, callback_name)()
        if (self.tensorboard_logdir is not None) and (validation_data is not None):
            self.log.info(f'Tensorboard data will be logged in {self.tensorboard_logdir}')
            callbacks.append(self._get_tensorboard_callbacks(validation_data))
        else:
            self.log.info(f'Tensorboard logging is turned off.')
            self.log.debug(f'skip_tb={self.skip_tb}')
        return callbacks

    
    def _get_tensorboard_callbacks(self, validation_data=None):
        if not os.path.exists(self.tensorboard_logdir):
            os.makedirs(self.tensorboard_logdir)
        
        histogram_freq=int(self.config_model['tensorboard_info']['histogram_frequency'])
        try: zcut = [int(zc.strip()) for zc in self.config_model['tensorboard_info']['zcut'].split(',')]
        except: zcut= [0,0]
        try: downsampling_scale = float(self.config_model['tensorboard_info']['downsampling_scale'].strip())
        except: downsampling_scale = 1.
        callback = self.TB(log_dir=self.tensorboard_logdir,
                           validation_data = validation_data,
                           histogram_freq=histogram_freq,
                           batch_size=int(self.config_model['validation_info']['batch_size']),
                           write_graph=self.config_model['tensorboard_info']['write_graph'].lower()=='true',
                           write_grads=self.config_model['tensorboard_info']['write_grads'].lower()=='true',
                           write_images=self.config_model['tensorboard_info']['write_images'].lower()=='true',
                           write_weights_histogram=self.config_model['tensorboard_info']['write_weights_histogram'].lower()=='true', 
                           write_weights_images=self.config_model['tensorboard_info']['write_weights_images'].lower()=='true',
                           embeddings_freq=int(self.config_model['tensorboard_info']['embeddings_freq']),
                           embeddings_metadata='metadata.tsv',
                           tb_data_steps=1,
                           zcut=zcut, downsampling_scale=downsampling_scale)
        return callback

    def set_network(self, img_shape=(None, None), img_channels=1):
        """call CNN architecture
        
        Parameters
        ----------
        img_shape: tuple of int (optional. default=(None, None))
            By default, assume arbitrary size of 2D single channel image
        """
        cnn_arch = self.config_model['model_info']['architecture']
        self.log.debug(f'CNN architecture descriptor: {cnn_arch}')
        self.log.debug(f'Path of CNN model weights: {self.saved_weight_path}')
        
        """Call CNN architecture"""
        # TODO: check logging level of get_cnn_arch
        try:
            # self.network = get_cnn_arch(cnn_arch, weight_file=self.saved_weight_path,
            #                             img_shapes=img_shape, img_channels=img_channels)
            self.network = get_cnn_arch(cnn_arch,
                                        img_shapes=img_shape, img_channels=img_channels)
        except Exception as e:
            self.log.info(f'Fail to load {cnn_arch} from default architectures.')
            custom_path_list = cnn_arch.split('/')
            # self.log.debug(custom_path_list)
            working_dir = self.config_model['path_info']['working_directory']
            node_name = self.config_model['model_info']['node_name']
            custom_arch_module_path = os.path.join(working_dir, f'{node_name}_KerasModel', '/'.join(custom_path_list[:-1]))
            assert '.py' in custom_arch_module_path
            custom_cnn_arch = custom_path_list[-1]
            self.log.info(f'Checking the custom architecture {custom_cnn_arch} from custom module {custom_arch_module_path}')
            
            spec = importlib.util.spec_from_file_location("custom_arch_modeul", custom_arch_module_path)
            custom_arch_modeul = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(custom_arch_modeul)
            self.network = custom_arch_modeul.get_cnn_arch(custom_cnn_arch,
                                                            img_shapes=img_shape, img_channels=img_channels)
            # self.network = custom_arch_modeul.get_cnn_arch(custom_cnn_arch, weight_file=self.saved_weight_path,
            #                                                 img_shapes=img_shape, img_channels=img_channels)
            self.log.info(f'The custom architecture {custom_cnn_arch} from custom module {custom_arch_module_path} is loaded.')
        
        """Set search area attention
        TODO
        ----
        @youngwon: panelty function for soft attention (consider score within layer class)
        """
        if self.search_area_attention_type is not None:
            self.log.info('---------------------------------------------------------------')
            self.log.info(f'Build network with search area attention: {self.search_area_attention_type}')
            self.log.info('---------------------------------------------------------------')
            self.main_network = self.network
            main_model_input_shape = list(self.main_network.input_shape[1:])
            input_shape_with_sa_attention = tuple(main_model_input_shape[:-1] + [main_model_input_shape[-1]+1])
            x_attention_input = Input(shape=input_shape_with_sa_attention, name='input_with_search_area_attention')
            x_attentioned, learned_search_area_attention = self.search_area_attention_class(intensity_trainable=self.intensity_trainable,
                                                                     intensity_initial=self.intensity_value)(x_attention_input)
            p_hat_with_attention = self.main_network(inputs=[x_attentioned])
            self.network = Model(inputs=[x_attention_input], outputs=[p_hat_with_attention], name='model_with_search_area_attention')
            self.network.summary(line_length=150, print_fn=self.log.info)

        """Set weight"""
        if self.saved_weight_path:
            self.network.load_weights(self.saved_weight_path)
            
        """Set multi-GPU model"""
        if self.number_of_gpu > 0:
            try:
                self.multi_gpu_network = multi_gpu_model(self.network, 
                                                        gpus=self.number_of_gpu)
                self.log.info(f'Calculation using {self.number_of_gpu} GPUs')
            except:
                self.multi_gpu_network = self.network
                self.log.info('Calculation using 1 GPU')
        else:
            self.multi_gpu_network = self.network
            self.log.info('Calculation using CPU cores')

    
    def model_compile(self):
        self.log.info('---------------------------------------------------------------')
        self.log.info('Compile Network')
        self.log.info('---------------------------------------------------------------')
        loss = getattr(cnn_loss_and_metrics, self.config_model['model_info']['loss'])
        dict_optimizer = {'adam':'Adam', 'adadelta':'Adadelta', 'adagrad':'Adagrad', 'adamax':'Adamax', 'rmsprop':'RMSprop', 'sgd':'SGD'}
        optimizer_desc = dict_optimizer[self.config_model['model_info']['optimizer'].strip().lower()]
        optimizer = getattr(keras.optimizers, optimizer_desc)
        lr = float(self.config_model['model_info']['lr'])
        decay=float(self.config_model['model_info']['decay'])
        beta_1=float(self.config_model['model_info']['beta_1'])
        metrics=[getattr(cnn_loss_and_metrics, metric.strip()) for metric in self.config_model['model_info']['metrics'].split(',')]
        self.log.info('Optimizer = {}'.format(self.config_model['model_info']['optimizer']))
        self.log.info('Loss = {}'.format(self.config_model['model_info']['loss']))
        self.log.info('Metrics = {}'.format(self.config_model['model_info']['metrics']))

        if 'adam' in optimizer_desc:
            optimizer_instance = optimizer(lr = lr, decay = decay, beta_1 = beta_1)
        else:
            optimizer_instance = optimizer(lr = lr, decay = decay)
        
        self.multi_gpu_network.compile(loss=loss, optimizer=optimizer_instance, metrics=metrics)

    def fit(self, x, y, epochs=1, batch_size = 1,
            # warm_start=False, warm_start_model=None, hist_path = None,
            initial_epoch=0):
        """training without generator
        
        Parameters
        ----------
        x: ndarray
            input numpy array
        y: ndarray
            reference numpy array

        Returns
        -------
        dict
            dictionary of training history
        """
        """set initial weight"""
        if self.warm_start:
            # with open('./%s/%s' % (os.path.join(os.path.split(warm_start_model)[:-1]), hist_path), 'r') as f:
            #     history = json.load(f)
            # try:
            #     trained_epoch = int(history['epochs'][-1])
            #     if np.isnan(trained_epoch):
            #         trained_epoch = int(history['epochs'][-2])
            # except:
            #     trained_epoch = len(list(history.values())[0])
            trained_epoch = 0

            epochs += trained_epoch
            epoch = initial_epoch+trained_epoch
            self.network.load_weights(self.warm_start_model)
            self.log.info('Load %d epoch trained weights from %s' % (trained_epoch, self.warm_start_model))
        else:
            epoch = initial_epoch
        
        self.log.info('Training start:')
        tic = timeit.default_timer()
        callbacks = self._get_callbacks(None)
        validation_split = int(self.network_info['training_info']['validation_size'].strip())
        hist = self.multi_gpu_network.fit(x, y, batch_size=batch_size,
                              epochs=epochs, 
                              verbose=1, 
                              callbacks=callbacks, 
                              validation_split=validation_split,
                              initial_epoch=epoch,                                        
                              max_queue_size=self.max_queue_size, workers=self.workers,
                              use_multiprocessing=self.use_multiprocessing)
        
        if self.best_model_save:
            self.log.info('Load best model')
            self.network.load_weights(self.weight_path)
        self.log.info(f'Training finished with time {timeit.default_timer() - tic}s.')
        return hist

    def fit_generator(self, train_generator, epochs=1, validation_generator=None, 
                    #   warm_start=False, warm_start_model=None, hist_path = None,
                      initial_epoch=0):
        """training with generator
        
        Parameters
        ----------
        train_generator: keras.utils.Sequence
            generator of training data
        validation_generator: keras.utils.Sequence
            generator of training data
        epochs : int
            Maximum number of epochs to train

        Returns
        -------
        dict
            dictionary of training history

        TODO
        ----
        warm-start
        """
        if self.warm_start:
            # with open('./%s/%s' % (os.path.join(os.path.split(warm_start_model)[:-1]), hist_path), 'r') as f:
            #     history = json.load(f)
            # try:
            #     trained_epoch = int(history['epochs'][-1])
            #     if np.isnan(trained_epoch):
            #         trained_epoch = int(history['epochs'][-2])
            # except:
            #     trained_epoch = len(list(history.values())[0])
            trained_epoch = 0

            epochs += trained_epoch
            epoch = initial_epoch+trained_epoch
            self.network.load_weights(self.warm_start_model)
            self.log.info('Load %d epoch trained weights from %s' % (trained_epoch, self.warm_start_model))
        else:
            epoch = initial_epoch

        self.log.info('Training start:')
        tic = timeit.default_timer()
        callbacks = self._get_callbacks(validation_generator)
        
        hist = self.multi_gpu_network.fit_generator(train_generator,
                                        steps_per_epoch=len(train_generator),
                                        epochs=epochs, 
                                        verbose=1, 
                                        initial_epoch=epoch,
                                        callbacks=callbacks, 
                                        validation_data=validation_generator,
                                        validation_steps=len(validation_generator),
                                        max_queue_size=self.max_queue_size, 
                                        workers=self.workers, use_multiprocessing=self.use_multiprocessing)
        
        if self.best_model_save:
            self.log.info('Load best model')
            self.network.load_weights(self.weight_path)
        self.log.info(f'Training finished with time {timeit.default_timer() - tic}s.')
        return hist
    
    def save_model(self):
        """save_model
        """
        # serialize model to YAML
        model_yaml = self.network.to_yaml()
        os.makedirs(self.weight_dir, exist_ok=True) 
        with open(os.path.join(self.weight_dir, "model.yaml"), "w") as yaml_file:
            yaml_file.write(model_yaml)
    
    def save_weight(self):
        """save weight
        """
        os.makedirs(self.weight_dir, exist_ok=True)
        self.network.save_weights(self.weight_path)

    def save_history(self, history):
        """save_history
        """
        if not os.path.exists(self.logdir):
            os.makedirs(self.logdir)
        try:
            with open(self.hist_logpath, 'w+') as f:
                json.dump(history, f)
        except:
            with open(self.hist_logpath, 'w+') as f:
                hist = dict([(ky, np.array(val).astype(np.float).tolist()) for (ky, val) in history.items()])
                json.dump(hist, f)

    """TODO"""
    # def evaluate(self, x, y, batch_size):

    def predict_generator(self, sampler, max_queue_size=10, workers=1, use_multiprocessing=False):
        """Prediction with generator
        
        Parameters
        ----------
        sampler: object
            sampler class instance
        max_queue_size: int (default=10)
            maximum number of mini-batch to queue in the CPU memory
        workers: int (default=1)
            number of cpu workers for generating mini-batch
        use_multiprocessing: bool (default=False)

        Returns
        -------
        ndarray
            numpy array of prediction
        """
        self.log.info('Prediction start:')
        tic = timeit.default_timer()
        pred = self.multi_gpu_network.predict_generator(sampler, 
                                            steps=len(sampler),
                                            max_queue_size=max_queue_size, 
                                            workers=workers, 
                                            use_multiprocessing=use_multiprocessing,
                                            verbose=self.verbose)
        self.log.info(f'Prediction finished with time {timeit.default_timer() - tic}s.')
        return pred

    def predict(self, x, batch_size = 1):
        """Prediction without generator
        
        Parameters
        ----------
        x: ndarray
            input numpy array

        Returns
        -------
        ndarray
            numpy array of prediction
        """
        self.log.info('Prediction start:')
        tic = timeit.default_timer()
        pred = self.multi_gpu_network.predict(x, batch_size = batch_size,
                                            verbose=self.verbose)
        self.log.info(f'Prediction finished with time {timeit.default_timer() - tic}s.')
        return pred
    
    def predict_generator(self, sampler, max_queue_size=10, workers=1, use_multiprocessing=False):
        """Prediction with generator
        
        Parameters
        ----------
        sampler: object
            sampler class instance
        max_queue_size: int (default=10)
            maximum number of mini-batch to queue in the CPU memory
        workers: int (default=1)
            number of cpu workers for generating mini-batch
        use_multiprocessing: bool (default=False)

        Returns
        -------
        ndarray
            numpy array of prediction
        """
        self.log.info('Prediction start:')
        tic = timeit.default_timer()
        pred = self.multi_gpu_network.predict_generator(sampler, 
                                            steps=len(sampler),
                                            max_queue_size=max_queue_size, 
                                            workers=workers, 
                                            use_multiprocessing=use_multiprocessing,
                                            verbose=self.verbose)
        self.log.info(f'Prediction finished with time {timeit.default_timer() - tic}s.')
        return pred


class segmentation_network(neural_network):
    # __doc__ = neural_network.__doc__ + __doc__ # TODO: docstring inheritation
    def __init__(self, config_model, config_resource, log):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        """
        super(segmentation_network,self).__init__(config_model, config_resource, log)
        if not self.simple_tb: self.TB = cnn_tensorboard_utils.TensorBoardSegmentationWrapper


class pointmarking_network(neural_network):
    # __doc__ = neural_network.__doc__ + __doc__ # TODO: docstring inheritation
    def __init__(self, config_model, config_resource, log):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        """
        super(segmentation_network,self).__init__(config_model, config_resource, log)
        if not self.simple_tb: self.TB = cnn_tensorboard_utils.TensorBoardPointDetectionWrapper

class pointmarking_multitask_network(neural_network):
    # __doc__ = neural_network.__doc__ + __doc__ # TODO: docstring inheritation
    def __init__(self, config_model, config_resource, log):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        """
        super(segmentation_network,self).__init__(config_model, config_resource, log)
        if not self.simple_tb: self.TB = cnn_tensorboard_utils.TensorBoardPointDetectionMultiTaskWrapper


    def model_compile(self):
        self.log.info('---------------------------------------------------------------')
        self.log.info('Compile Multi Task Learning Network')
        self.log.info('---------------------------------------------------------------')
        loss = getattr(cnn_loss_and_metrics, self.config_model['model_info']['loss'])
        dict_optimizer = {'adam':'Adam', 'adadelta':'Adadelta', 'adagrad':'Adagrad', 'adamax':'Adamax', 'rmsprop':'RMSprop', 'sgd':'SGD'}
        optimizer_desc = dict_optimizer[self.config_model['model_info']['optimizer'].strip().lower()]
        optimizer = getattr(keras.optimizers, optimizer_desc)
        lr = float(self.config_model['model_info']['lr'])
        decay=float(self.config_model['model_info']['decay'])
        beta_1=float(self.config_model['model_info']['beta_1'])
        metrics=[getattr(cnn_loss_and_metrics, metric.strip()) for metric in self.config_model['model_info']['metrics'].split(',')]
        self.log.info('Optimizer = {}'.format(self.config_model['model_info']['optimizer']))
        self.log.info('Loss = {}'.format(self.config_model['model_info']['loss']))
        self.log.info('Metrics = {}'.format(self.config_model['model_info']['metrics']))

        if 'adam' in optimizer_desc:
            optimizer_instance = optimizer(lr = lr, decay = decay, beta_1 = beta_1)
        else:
            optimizer_instance = optimizer(lr = lr, decay = decay)
        
        self.multi_gpu_network.compile(loss={'carina':loss, 'ettip':loss}, 
                                        loss_weights = {'carina':1, 'ettip':1},
                                         optimizer=optimizer_instance, metrics=metrics)

class classification_network(neural_network):
    # __doc__ = neural_network.__doc__ + __doc__ # TODO: docstring inheritation
    def __init__(self, config_model, config_resource, log):
        """
        Parameters
        ----------
        config_model : dict
            dictionary of model configuration information
        config_resource : dict
            dictionary of resource configuration information
        log : object
            logging class instance
        """
        super(segmentation_network,self).__init__(config_model, config_resource, log)
        if not self.simple_tb: self.TB = cnn_tensorboard_utils.TensorBoardClassificationWrapper
