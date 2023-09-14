"""Input creation for cnn training

This script is for creating input-related files for a CNN model training from miu.

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
import logging
from argparse import ArgumentParser

# import __qia__
from cnn_configurator import Configurator

def input_creation(config_model, config_resource, log):
    """Input creation
    """

    """1.1 Construct a reader class instance"""
    reader_class = getattr(cnn_reader, config_model['model_info']['reader_class'].strip())
    reader = reader_class(config_model, config_resource, log, 'train')
    target_shape = reader.get_target_shape()
    img_channels = reader.get_image_channels()
    
    """1.2 Build a training set """
    try:
        reader.build_training_set()
    except:
        log.debug('Error occurs while building a training set. Restart to generate input file from the scratch.')
        reader.build_training_set(overwrite_input=True)

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

    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))
    log.info(f'numpy v. {np.__version__}')
    log.info(f'tensorflow v. {tf.__version__}')
    log.info(f'keras v. {keras.__version__}')

    """Model configuration maps"""
    config_model_class = Configurator(args.model_config, log)
    config_model_class.set_config_map(config_model_class.get_section_map())
    config_model = config_model_class.get_config_map()

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

    input_creation(config_model, config_resource, log)