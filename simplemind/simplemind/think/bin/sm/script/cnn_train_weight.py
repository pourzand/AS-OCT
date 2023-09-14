"""Weight tranining for cnn training

This script is for training the a CNN model weight from miu.

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
"""

import os, sys
import glob
from shutil import copyfile
import logging
import timeit, time
from datetime import datetime
from argparse import ArgumentParser
import warnings
warnings.filterwarnings("ignore")

# import __qia__
from cnn_configurator import Configurator
from cnntools import _get_owner, _get_original_cmd, lock_file, unlock_file

def get_sampler_configuration(config_sampler):
    """helper function to get sampler attributes from model configuration
    
    Paramters
    ---------
    config_sampler

    Return
    ------
    batch_size : int
        batch_size
    sequential : bool
        sequential
    replace : bool
        replace
    subsets_per_epoch : int
        subsets_per_epoch
    steps_per_epoch : int
        steps_per_epoch
    """
    try:
        batch_size = config_sampler['batch_size']
        batch_size = int(batch_size.strip())
    except:
        batch_size = config_sampler['batch_size']
        log.critical(f'Batch size configuration is not readable: {batch_size}')
    try:
        if config_sampler['sequential'] == 'True': sequential = True
        else: sequential = True
    except: sequential = False
    try:
        if config_sampler['replace'] == 'True': replace = True
        else: replace = False
    except: replace = False
    try: subsets_per_epoch = int(config_sampler['subsets_per_epoch'])
    except: subsets_per_epoch = None
    try: steps_per_epoch = int(config_sampler['steps_per_epoch'])
    except: steps_per_epoch = None
    return batch_size, sequential, replace, subsets_per_epoch, steps_per_epoch

def train_weight(config_model, config_resource, log):
    process_id =  os.getpid()
    process_user = _get_owner(process_id)
    process_cmd = ' '.join(_get_original_cmd(process_id))

    """Train weight
    """
    node_name = config_model['model_info']['node_name']
    working_dir = config_model['path_info']['working_directory'].strip()
    working_dir = os.path.join(working_dir, f'{node_name}_KerasModel', 'weights')
    weight_tag = config_model['chromosome_info']['weights_tag']
    weight_hist_path = os.path.join(working_dir, f'history_weight_{node_name}_{weight_tag}')
    log.info('-----------------------------------------------------------------')
    log.info(f'Cnn node {node_name} computing start...')
    tic = timeit.default_timer()

    """1. Construct a reader class instance"""
    reader_class = getattr(cnn_reader, config_model['model_info']['reader_class'].strip())
    reader = reader_class(config_model, config_resource, log, 'train')
    target_shape = reader.get_target_shape()
    img_channels = reader.get_image_channels()
    
    train_idx, validation_idx = reader.load_training_set()

    """2. Construct generators"""
    """2.1 Construct a sampler class instance for train-generator"""
    # train_batch_size = reader.get_mini_batch_size()
    # validation_batch_size = reader.get_validation_batch_size()
    log.info('Construct training data sampler')
    train_sampler_class = getattr(cnn_sampler, config_model['model_info']['sampler_class'].strip())  
    config_sampler = config_model['training_info']
    train_info = get_sampler_configuration(config_sampler)
    batch_size, sequential, replace, subsets_per_epoch, steps_per_epoch = train_info
    train_sampler = train_sampler_class(train_idx, log, batch_size,
                                        sequential = sequential, replace = replace,
                                        subsets_per_epoch = subsets_per_epoch, 
                                        steps_per_epoch = steps_per_epoch)
    augmentator_class = getattr(cnn_augmentation, config_model['model_info']['augmentator_class'].strip())
    do_augment = config_model['training_info']['do_augment'].strip()
    if do_augment == 'true':
        augmentation_types = config_model['model_info']['augmentation'].split(',')
        augmentation_types = [x.strip() for x in augmentation_types]
    else:
        augmentation_types = []
    train_augmentator = augmentator_class(augmentation_types, target_shape, log)
    train_generator = cnn_sampler.batch_generator(reader, train_sampler, train_augmentator)
    
    """2.2 Construct a sampler class instance for valiation-generator"""
    config_sampler = config_model['validation_info']
    validation_info = get_sampler_configuration(config_sampler)
    batch_size, sequential, replace, subsets_per_epoch, steps_per_epoch = validation_info
    if batch_size is not None:
        log.info('Construct validation data sampler')
        validation_sampler_class = getattr(cnn_sampler, config_model['model_info']['sampler_class'].strip())  

        validation_sampler = validation_sampler_class(validation_idx, log, batch_size,
                                                    sequential = sequential, replace = replace,
                                                    subsets_per_epoch = subsets_per_epoch,
                                                    steps_per_epoch = steps_per_epoch)
        augmentator_class = getattr(cnn_augmentation, config_model['model_info']['augmentator_class'].strip())  
        do_augment = config_model['validation_info']['do_augment'].strip()
        if do_augment == 'true':
            augmentation_types = config_model['model_info']['augmentation'].split(',')
            augmentation_types = [x.strip() for x in augmentation_types]
        else:
            augmentation_types = []
        validation_augmentator = augmentator_class(augmentation_types, target_shape, log)
        validation_generator = cnn_sampler.batch_generator(reader, validation_sampler, validation_augmentator)
                        
    """3. Construct a network class instance"""
    network_class = getattr(cnn_network, config_model['model_info']['network_class']) 
    network = network_class(config_model, config_resource, log)
    network.set_network(target_shape, img_channels)

    """4. Compile the network class instance"""
    network.model_compile()
    sys.stdout.flush()
    network.save_model()
    
    """5. Training"""
    hist = network.fit_generator(train_generator, 
                                epochs=int(config_model['training_info']['epochs'].strip()),
                                validation_generator=validation_generator)
    sys.stdout.flush()
    network.save_history(hist.history)
    log.info(f'CNN training finished for node {node_name}')
    log.info(f'Cnn node {node_name} computing finished with time {timeit.default_timer() - tic}s.')
    log.info('-----------------------------------------------------------------')

    """TODO:
    bring a temporary solution for moving best file to root dir
    why have to use glob? because of condor-broken issues?
    """
    with open(weight_hist_path, 'a') as hist_weight_f:
        log.info('-----------------------------------------------------------------')
        print('-----------------------------------------------------------------')
        print(f'Weight training finished. Lock the trigger file: {hist_weight_f}.')
        log.info(f'Weight training finished. Lock the trigger file: {hist_weight_f}.')
        sys.stdout.flush()
        lock_file(hist_weight_f)
        summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start copying weight with tag {weight_tag} with process_cmd:\n' + \
                f'\t{process_cmd}\n'
        hist_weight_f.write(summary_process_start)
        hist_weight_f.flush()
        log.info(summary_process_start)
        print(summary_process_start)
        sys.stdout.flush()

        # weight_file = network.weight_path
        node_name = config_model['model_info']['node_name']
        hd5_bit_tag = config_model['chromosome_info']['weights_tag']
        weight_files = glob.glob(os.path.join(network.weight_dir, f'{node_name}_weights*.h5'))
        weight_files = sorted(weight_files, reverse=True, key=lambda t:os.stat(t).st_mtime)
        moving_path = os.path.join(os.path.dirname(network.weight_dir),
                                    f'{node_name}_weights_{hd5_bit_tag}.hd5')
        copyfile(weight_files[0], moving_path)
        log.debug(f'Best weight {weight_files[0]} copied to {moving_path}.')
        print(f'Best weight {weight_files[0]} copied to {moving_path}.')

        summary_process_copying = f'[{process_user}|{process_id}|{datetime.now()}] Done copying weight with tag {weight_tag}\n'
        hist_weight_f.write(summary_process_copying)
        hist_weight_f.flush()
        log.info(summary_process_copying)
        print(summary_process_copying)
        log.info('-----------------------------------------------------------------')
        print('-----------------------------------------------------------------')
        log.info(f'Cauculation all finished. Try unlock trigger file: {hist_weight_f}')
        print(f'Cauculation all finished. Try unlock trigger file: {hist_weight_f}')
        sys.stdout.flush()
        unlock_file(hist_weight_f)
        log.info(f'Done unlocking. All computations for train weight {hd5_bit_tag} finished.')
        print(f'Done unlocking. All computations for train weight {hd5_bit_tag} finished.')
        log.info('-----------------------------------------------------------------')
        print('-----------------------------------------------------------------')
        sys.stdout.flush()    

if __name__=='__main__':
    """Parsing the input arguments"""
    parser = ArgumentParser(description='Script to run a CNN model training from miu')
    
    parser.add_argument('--cm', '--model_config', type=str, dest='model_config',
                        help="model configuration file path")
    parser.add_argument('--cr', '--resource_config', type=str, dest='resource_config', 
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
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    if args.verbose >= 2: log.setLevel(logging.DEBUG)
    elif args.verbose >= 1: log.setLevel(logging.INFO)
    else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    # resource thing: no gpu option in
    import numpy as np
    import tensorflow as tf
    tf.get_logger().setLevel('ERROR') # tensorflow logs too much.
    import keras
    import cnn_reader
    import cnn_sampler
    import cnn_network
    import cnn_augmentation

    """Model configuration maps"""
    config_model_class = Configurator(args.model_config, log)
    config_model_class.set_config_map(config_model_class.get_section_map())
    config_model = config_model_class.get_config_map()

    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))
    log.info(f'numpy v. {np.__version__}')
    log.info(f'tensorflow v. {tf.__version__}')
    log.info(f'keras v. {keras.__version__}')

    """Resource configuration maps"""
    log.info('---------------------------------------------------------------')
    log.info('Resource Configuration')
    log.info('---------------------------------------------------------------')
    if os.path.exists(args.resource_config):
        resource_config_path = args.resource_config
    else:
        resource_config_path = os.path.join(os.path.dirname(__file__), "ref_resource.ini")
        log.info('No resource information is detected. Reference resource configuration is used instead.')
        log.info('MIU will use CPU resources only.')
        log.info('---------------------------------------------------------------')
    log.debug(f'Resource configuration path: {resource_config_path}')
    config_resource_class = Configurator(resource_config_path, log)
    config_resource_class.set_config_map(config_resource_class.get_section_map())
    config_resource = config_resource_class.get_config_map()
    config_resource_class.print_config_map()
    log.debug(config_resource)

    train_weight(config_model, config_resource, log)



