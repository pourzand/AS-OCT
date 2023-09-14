"""Predictor

This script is for runing a CNN model prediction from miu.

    * main - the main function of the script

Parameters
----------
--model_config: str
    model configuration file path
--resource_config: str
    resource configuration file path
-v: int
    '--verbose': (optional) default=0
    set the logging level
        0: no logging (critical error only)
        1: info level
        2: debug level

Examples
--------
Using MIU executable:
    1. [single channel; 3D]
    Ex) Prostate organ segmentation with a public data PROMISE12 (.mhd)
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export miu=$qia_home/bin/miu/miu
    export base_dir=/apps/personal/youngwon/miu_examples/public_PROMISE12
    export working_dir=$base_dir/experiments/experiment_miu
    export image_file=$base_dir/data_source/training_all/Case00.mhd
    export model_file=$base_dir/experiments/miu_model_prostate/prostate_model
    export output_dir=$base_dir/experiments/experiment_miu/output
    export resource_config=$working_dir/prostate_cnn_KerasModel/prostate_cnn_resource.ini
    export log_file=$working_dir/log_miu_single_channel.log
    $miu $image_file $model_file $output_dir -d $working_dir -f | tee $log_file
    ```

    Ex) Prostate organ segmentation with a in-house data (.nii)
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export miu=$qia_home/bin/miu/miu
    export base_dir=/apps/personal/youngwon/miu_examples/inhouse_10042
    export working_dir=$base_dir/experiments/experiment_miu
    export image_file=$base_dir/data_source/total/10042_1_003TNQ2B/img_orig.nii
    export model_file=$base_dir/experiments/miu_model_prostate/prostate_model
    export output_dir=$base_dir/experiments/experiment_miu/output
    export resource_config=$working_dir/prostate_cnn_KerasModel/prostate_cnn_resource.ini
    export log_file=$working_dir/log_miu_single_channel.log
    $miu $image_file $model_file $output_dir -d $working_dir -f | tee $log_file
    ```
    
    2. [single channel; 2D]
    EX) In-house CXR trachea_cnn (.seri)
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export miu=$qia_home/bin/miu/miu
    export base_dir=/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea
    export working_dir=$base_dir/experiments/experiment_miu
    export image_file=$base_dir/data_images_carina/linux_1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0.seri
    # export input_roi=$base_dir/data_input_rois/1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0
    export input_roi=$base_dir/data_temp_input_rois/
    export model_file=$base_dir/experiments/miu_model_cxr_trachea/cxr_trachea_model
    export output_dir=$base_dir/experiments/experiment_miu/output
    export chromosome=1111010110001110011
    export resource_config=$working_dir/trachea_cnn_KerasModel/trachea_cnn_resource.ini
    export log_file=$working_dir/log_miu_cxr_single_channel.log
    $miu $image_file $model_file $output_dir -r $input_roi -d $working_dir -c $chromosome -f | tee $log_file
    ```

    3. [product level example]
    EX) In-house CXR v9 test_411 (.seri)
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export miu=$qia_home/bin/miu/miu
    export base_dir=/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea
    export working_dir=$base_dir/experiments/experiment_cxr_v9
    export image_file=$base_dir/data_images_carina/linux_1.2.392.200036.9125.3.32166252172248.64927082709.5016411_0.seri
    export input_roi=$base_dir/data_input_rois/1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0
    # export input_roi=$base_dir/data_temp_input_rois/1.2.392.200036.9125.3.32166252172248.64927082709.5016411_0
    export model_file=$base_dir/experiments/miu_model_cxr_v9/cxr_model
    export output_dir=$base_dir/experiments/experiment_cxr_v9/output_411_v9
    export log_file=$working_dir/log_miu_cxr_v9_411_channel.log
    $miu $image_file $model_file $output_dir -r $input_roi -d $working_dir -f | tee $log_file
    ```

    EX) In-house CXR v9 test_761 (.seri)
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export miu=$qia_home/bin/miu/miu
    export base_dir=/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea
    export working_dir=$base_dir/experiments/experiment_cxr_v9
    export image_file=$base_dir/data_images_carina/linux_1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0.seri
    export input_roi=$base_dir/data_input_rois/1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0
    # export input_roi=$base_dir/data_temp_input_rois/1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0
    export model_file=$base_dir/experiments/miu_model_cxr_v9/cxr_model
    export output_dir=$base_dir/experiments/experiment_cxr_v9/output_761_v9
    export log_file=$working_dir/log_miu_cxr_v9_761_channel.log
    $miu $image_file $model_file $output_dir -r $input_roi -d $working_dir -f | tee $log_file
    ```

    4. TODO [single case; multi-channel; 3D]
    Ex) Prostate organ segmentation with a in-house data

Using cnn_predict.py:
    1. [mhd; single case; single channel; 3D]
    Ex) Prostate organ segmentation with a public data PROMISE12
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export script=$qia_home/bin/miu/script/cnn_predict.py
    export base_dir=/apps/personal/youngwon/miu_examples/public_PROMISE12
    export working_dir=$base_dir/experiments/experiment_miu
    export model_config=$working_dir/output/prostate_cnn_config.ini
    export resource_config=$working_dir/prostate_cnn_resource.ini
    export log_file=$working_dir/log_cnn_pred_py_single_channel.log
    python $script --model_config=$model_config --resource_config=$resource_config -v 2 | tee $log_file
    ```

    Ex) Prostate organ segmentation with a in-house data
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export script=$qia_home/bin/miu/script/cnn_predict.py
    export base_dir=/apps/personal/youngwon/miu_examples/inhouse_10042
    export working_dir=$base_dir/experiments/experiment_miu
    export model_config=$working_dir/output/prostate_cnn_config.ini
    export resource_config=$working_dir/prostate_cnn_resource.ini
    export log_file=$working_dir/log_cnn_pred_py_single_channel.log
    python $script --model_config=$model_config --resource_config=$resource_config -v 2 | tee $log_file
    ```

    2. TODO [mhd; single case; multi-channel; 3D]
    Ex) Prostate organ segmentation with a in-house data
"""

import os
import sys
import logging
import numpy as np
from argparse import ArgumentParser
import timeit, time
from datetime import datetime
import traceback
from termcolor import colored
import warnings
warnings.filterwarnings("ignore")
import tensorflow as tf
tf.get_logger().setLevel('ERROR') # tensorflow logs too much.
import keras

import __qia__
import qia.common.img.image as qimage
from cnn_configurator import Configurator
import cnn_reader
import cnn_network
from cnntools import _get_owner, _get_original_cmd, lock_file, unlock_file
logging.getLogger('matplotlib').setLevel(logging.ERROR)
import matplotlib.pyplot as plt

def main(config_model, config_resource, log):
    try:
        """
        TODO: @youngwonchoi
        1. Set seed
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
        """

        log.info('---------------------------------------------------------------')
        log.info('Computation start')
        log.info('---------------------------------------------------------------')
        """1. Construct a reader class instance
        Notes:
        If we want to choose the reader class based on the file format, maybe
        that operation should be done in here.
        """
        reader_class = getattr(cnn_reader, config_model['model_info']['reader_class'])
        reader = reader_class(config_model, config_resource, log, 'prediction')
        target_shape = reader.get_target_shape()
        img_channels = reader.get_image_channels()
        
        """2. Construct a network class instance
        """
        network_class = getattr(cnn_network, config_model['model_info']['network_class']) 
        network = network_class(config_model, config_resource, log)
        
        """Configuration for file locking system"""
        check_frequency_s=1
        """avoid weight racing conditions"""
        weight_path = network.weight_path
        working_dir = os.path.dirname(weight_path)
        node_name = config_model['model_info']['node_name']
        weight_tag = config_model['chromosome_info']['weights_tag']
        weight_hist_path = os.path.join(working_dir, f'history_weight_{node_name}_{weight_tag}')

        process_id =  os.getpid()
        process_user = _get_owner(process_id)
        process_cmd = ' '.join(_get_original_cmd(process_id))
        status_update_weight = False
        
        network.set_network(target_shape, img_channels)

        # tic = timeit.default_timer()
        # while True:
        #     try:
        #         with open(weight_hist_path, 'a') as hist_weight_f:
        #             lock_file(hist_weight_f)
        #             summary_process_loading = f'[{process_user}|{process_id}|{datetime.now()}] Start loading the weight with tag {weight_tag}\n'
        #             hist_weight_f.write(summary_process_loading)
        #             hist_weight_f.flush()
                    
        #             network.set_network(target_shape, img_channels)

        #             summary_process_loading = f'[{process_user}|{process_id}|{datetime.now()}] Done loading the weight with tag {weight_tag}\n'
        #             hist_weight_f.write(summary_process_loading)
        #             hist_weight_f.flush()
        #             unlock_file(hist_weight_f)
        #             log.info(summary_process_loading)
        #             sys.stdout.flush()
        #             unlock_file(hist_weight_f)
        #         break
        #     except OSError or BlockingIOError as e:
        #         toc = timeit.default_timer()
        #         if (toc-tic) > 100 * check_frequency_s: 
        #             """Timeout"""
        #             break
        #         else:
        #             if not status_update_weight:
        #                 summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start waiting the locked weight with tag {weight_tag} with process_cmd:\n' + \
        #                         f'\t{process_cmd}\n\t{e}\n'
        #                 print(summary_process_start)
        #                 sys.stdout.flush()
        #             status_update_weight = True
        #             time.sleep(check_frequency_s)
                
        
        """3. Processing batches"""
        tic = timeit.default_timer()
        image_path = config_model['path_info']['image_file']
        previous_node_rois = os.path.join(config_model['path_info']['output_directory'])
        reader.previous_node_rois = previous_node_rois
        # if os.path.exists(os.path.join(previous_node_rois, f'search_area_{reader.node_name}.roi')):
        #     log.debug(f'Search Area saved in = {reader.previous_node_rois}')
        reader.set_case_dict_list([image_path])
        img_arr = reader.get_predict_mini_batch([0])
        
        """4. Predict ROIs"""
        pred = network.predict(img_arr, batch_size=1)
        # log.debug(f'prediction: {pred}')
        """5. Write ROIs.
        TODO
        ----
        revise this for encapsulation
        need some wrapper in reader class...
        should not using qimage.load outside the reader..
        """
        base_img = qimage.read(image_path)
        # try:
        #     base_img = qimage.read(image_path) #our inhouse image object format
        # except Exception as e:
        #     if 'seri' in image_path.split('.')[-1]:
        #         with open(image_path) as f:
        #             img_path_list = f.readlines()
        #             base_img = qimage.read(img_path_list[0])
        #             # try:
        #                 # self.image = qimage.read(img_path_list[0])
        #             # except:
        #                 # dcm = pydicom.read_file(img_path_list[0])
        #     else:
        #         traceback.print_exc()
        #         raise ValueError(f'Image loading failed for {image_path} with exception {e}') 
        base_img = base_img.get_alias(min_point=(0, 0, 0))
        pred_image_list, pred_name_list, fig, fig_norm = reader.write_roi(pred, base_img, threshold=0.5, norm_img_arr=img_arr[...,-1])
        for pred_image, name in zip(pred_image_list, pred_name_list):
            roi_path = os.path.join(config_model['path_info']['output_directory'],
                                    f'{name}.roi')
            pred_image.write(roi_path)
            log.info(f'ROI is written in {roi_path}')
        node_name = config_model['model_info']['node_name']
        log.info(f'CNN prediction finished for node {node_name}')
        if fig is not None:
            png_name = '_'.join(pred_name_list)
            fig_path = os.path.join(config_model['path_info']['output_directory'],
                                    f'pred_{png_name}_on_original.png')
            fig.savefig(fig_path)
            log.info(f'CNN prediction on original image saved at {fig_path}')

        if fig_norm is not None:
            png_name = '_'.join(pred_name_list)
            fig_path = os.path.join(config_model['path_info']['output_directory'],
                                    f'pred_{png_name}_on_norm.png')
            fig_norm.savefig(fig_path)
            log.info(f'CNN prediction on normalized image saved at {fig_path}')
        log.info('-----------------------------------------------------------------')
        
    except Exception as e:
        # traceback.print_exc()
        output_dir = config_model['path_info']['output_directory']
        node_name = config_model['model_info']['node_name']
        error_log_path = os.path.join(output_dir, f'error_log_{node_name}.log')
        
        tstamp = datetime.now().strftime('%Y-%m-%d-%H-%m-%s')
        c_error_str = colored('\n-------------------------------------------------------------------------------------------------\n', 'red')
        colored_warning = f'Error from {os.path.basename(os.path.abspath(__file__))}\n'
        c_error_str += colored(colored_warning, 'red')
        c_error_str += colored('-------------------------------------------------------------------------------------------------\n', 'red')
        error_str = '-------------------------------------------------------------------------------------------------\n'
        error_str += f'Error Log for {node_name} from {os.path.basename(os.path.abspath(__file__))} (time stamp: {tstamp})\n'
        error_str += f'    Error from {os.path.abspath(__file__)}\n'
        error_str += '-------------------------------------------------------------------------------------------------\n'
        log_error_str = f'See error log at \n    {error_log_path}\n'
        log_error_str += '-------------------------------------------------------------------------------------------------\n'
        # file_error_str = f'{repr(e)}\n'
        file_error_str = f"{type(e).__name__} at line {e.__traceback__.tb_lineno} of {__file__}: {e}\n"
        file_error_str += '-------------------------------------------------------------------------------------------------\n'
        file_error_str += 'Full error traceback:\n'
        file_error_str += '-------------------------------------------------------------------------------------------------\n'
        file_error_str += f'{traceback.format_exc()}\n'
        file_error_str += '-------------------------------------------------------------------------------------------------\n'
        with open(error_log_path, 'a') as f:
            log.info(c_error_str + error_str + log_error_str)
            f.write(error_str + file_error_str)
            sys.stdout.flush()
        raise ValueError('cnn_train.py failed with exception ', e)
    # finally:

if __name__=='__main__':
    """Parsing the input arguments"""
    parser = ArgumentParser(description='Script to run a CNN model prediction from miu')
    parser.add_argument('-cm', '--model_config', type=str, dest='model_config',
                        help="model configuration file path")
    parser.add_argument('-cr', '--resource_config', type=str, dest='resource_config', 
                        help="resource configuration file path")
    parser.add_argument('-v', '--verbose', type=int, dest='verbose', default=0,
                        help="logging level")
    args = parser.parse_args()

    """logging: using basic logger from logging
    TODO: 
    set color
    https://stackoverflow.com/questions/45923290/how-to-get-the-current-log-level-in-python-logging-module
    make clean / add cnn_node_log file
    """
    # logging.basicConfig(format = '[%(name)-10s|%(levelname)-8s|%(filename)-20s:%(lineno)-3s] %(message)s',
    #                 level=logging.DEBUG)
    log = logging.getLogger()
    # if args.verbose >= 2: log.setLevel(logging.DEBUG)
    # elif args.verbose >= 1: log.setLevel(logging.INFO)
    # else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    # formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    formatter = logging.Formatter('[%(name)-6s|%(levelname)-6s|%(filename)-20s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    if args.verbose >= 2: log.setLevel(logging.DEBUG)
    elif args.verbose >= 1: log.setLevel(logging.INFO)
    else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    ch.setFormatter(formatter)
    log.addHandler(ch)
    
    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))
    log.info(f'numpy v. {np.__version__}')
    log.info(f'tensorflow v. {tf.__version__}')
    log.info(f'keras v. {keras.__version__}')

    """Model configuration maps"""
    log.info('---------------------------------------------------------------')
    log.info('Model Configuration')
    log.info('---------------------------------------------------------------')
    log.debug(f'Model configuration path: {args.model_config}')
    config_model_class = Configurator(args.model_config, log)
    config_model_class.set_config_map(config_model_class.get_section_map())
    config_model_class.print_config_map()
    config_model = config_model_class.get_config_map()
    log.debug(config_model)

    """Resource configuration maps"""
    log.info('---------------------------------------------------------------')
    log.info('Resource Configuration')
    log.info('---------------------------------------------------------------')
    is_predict_with_cpu = config_model['switch_from_miu_info']['predict_cpu_only'].strip().lower()
    # log.debug(f'Use a single-core CPU for predictions: {is_predict_with_cpu}')
    # if (is_predict_with_cpu == 'true'): predict_with_cpu = True
    # else: predict_with_cpu = False
    # if not predict_with_cpu:
    #     if os.path.exists(args.resource_config):
    #         resource_config_path = args.resource_config
    #     else:
    #         resource_config_path = os.path.join(os.path.dirname(__file__), "ref_resource.ini")
    #         log.info('No resource information is detected. Reference resource configuration is used instead.')
    #         log.info('MIU will use CPU resources only.')
    # else:
    #     resource_config_path = os.path.join(os.path.dirname(__file__), "ref_resource.ini")
    #     log.info('With the `-p` argument, MIU will use CPU resources only for prediction.')
    log.info('Before fixing the watcher problem for prediction, we will use CPU only prediction....')
    resource_config_path = os.path.join(os.path.dirname(__file__), "ref_resource.ini")
    log.info('---------------------------------------------------------------')
    config_resource_class = Configurator(resource_config_path, log)
    config_resource_class.set_config_map(config_resource_class.get_section_map())
    log.debug(f'Resource configuration path: {resource_config_path}')
    config_resource_class.print_config_map()
    config_resource = config_resource_class.get_config_map()
    log.debug(config_resource)

    main(config_model, config_resource, log)