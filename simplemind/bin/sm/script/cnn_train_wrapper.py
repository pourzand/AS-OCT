"""Trainer

This script is for runing a CNN model training from miu.

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
"""

import os, sys
import copy
import configparser
import logging
import timeit, time
import subprocess
from datetime import datetime
from argparse import ArgumentParser
import traceback
from termcolor import colored
import warnings
warnings.filterwarnings("ignore")

# import __qia__
from cnn_configurator import Configurator
from cnntools import _get_owner, _get_original_cmd, lock_file, unlock_file


def _execute(cmd):
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

def main(config_model, log, model_config_path, resource_config_path, verbose):
    """Main function for cnn training

    Parameters
    ----------
    config_model : dict
    log : logging instance
    """

    """Configuration for file locking system"""
    check_frequency_s=5

    """avoid weight racing conditions"""
    node_name = config_model['model_info']['node_name']
    working_dir = config_model['path_info']['working_directory'].strip()
    weight_tag = config_model['chromosome_info']['weights_tag']
    input_tag = config_model['chromosome_info']['input_tag']
    input_dir = os.path.join(working_dir, f'{node_name}_KerasModel', 'input')
    os.makedirs(input_dir, exist_ok=True)
    input_avaialble_path = os.path.join(input_dir, f'available_input_{input_tag}')
    input_hist_path = os.path.join(input_dir, f'history_input_{input_tag}')
    weight_dir = os.path.join(working_dir, f'{node_name}_KerasModel', 'weights')
    os.makedirs(weight_dir, exist_ok=True)
    weight_avaialble_path = os.path.join(weight_dir, f'available_weight_{node_name}_{weight_tag}')
    weight_hist_path = os.path.join(weight_dir, f'history_weight_{node_name}_{weight_tag}')
    condor_job_path = config_model['path_info']['condor_job_directory'].strip()
    if (condor_job_path == ''): condor_job_path = None
    else: log.debug(f'MIU will be run by condor. Condor related files will be saved in {condor_job_path}')

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

    cpu_config_resource_class = configparser.ConfigParser()
    cpu_config_resource_class['CPU'] = config_resource['CPU']
    cpu_config_resource_class.add_section('log')
    # cpu_config_resource_class.set('log', 'output_dir', config_model['path_info']['output_directory'])
    cpu_config_resource_class['log']['output_dir'] = config_model['path_info']['output_directory']


    gpu_config_resource_class = configparser.ConfigParser()
    gpu_config_resource_class['CPU'] = config_resource['CPU']
    gpu_config_resource_class['CPU']['num_cpu_core'] = '1'
    gpu_config_resource_class['CPU']['use_multiprocessing'] = 'false'
    gpu_config_resource_class['GPU'] = config_resource['GPU']
    gpu_config_resource_class.add_section('log')
    gpu_config_resource_class['log']['output_dir'] = config_model['path_info']['output_directory']
    
    process_id =  os.getpid()
    process_user = _get_owner(process_id)
    process_cmd = ' '.join(_get_original_cmd(process_id))
    status_update_weight = False
    check_frequency_s_weight = 5
    check_frequency_s_input = 5
    check_frequency_s_done = 30
    while True:
        try:
            with open(weight_avaialble_path, 'a') as available_weight_f:
                lock_file(available_weight_f)
                summary_process_check = f'[{process_user}|{process_id}|{datetime.now()}] Check an available weight with tag {weight_tag}\n'
                available_weight_f.write(summary_process_check)
                available_weight_f.flush()
                log.info(summary_process_check)                    
                sys.stdout.flush()

                if os.path.exists(weight_hist_path):
                    """previously trained weight exists"""
                    log.info('Previously trained weight exists.')
                    pass
                else:
                    try:
                        log.info('---------------------------------------------------------------')
                        log.info('Computation start')
                        log.info('---------------------------------------------------------------')

                        status_update_input = False
                        """1. Input creation"""
                        while True:
                            try:
                                with open(input_avaialble_path, 'a') as available_input_f:
                                    lock_file(available_input_f)
                                    """start creating input"""
                                    summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start checking the available input with {input_tag} with process_cmd:\n' + \
                                            f'\t{process_cmd}\n'
                                    available_input_f.write(summary_process_start)
                                    available_input_f.flush()
                                    log.debug(f'Start checking the available input with {input_tag}')
                                    log.info('---------------------------------------------------------------')
                                    log.info(summary_process_start)
                                    sys.stdout.flush()
                                    
                                    if os.path.exists(input_hist_path):
                                        """previously created input exists"""
                                        log.info('Previously created input exists.')
                                        pass
                                    else:
                                        """start creating input"""
                                        summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] No input founded. Generate new input with {input_tag} with process_cmd:\n' + \
                                                f'\t{process_cmd}\n'
                                        available_input_f.write(summary_process_start)
                                        available_input_f.flush()
                                        log.debug('No input founded. Generate new input...')
                                        log.info('---------------------------------------------------------------')
                                        log.info(summary_process_start)
                                        sys.stdout.flush()

                                        input_script_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'cnn_train_input_creation.py')
                                        cmd = ['python', input_script_path, '--model_config', model_config_path, '--resource_config', resource_config_path, '--verbose', str(verbose)]
                                        # cmd = ["python -c", "'from simplemind.think_module.bin.sm.script.cnn_train_input_creation import imported_main;", "imported_main(\"{}\", \"{}\", {});".format(model_config_path ,resource_config_path, str(verbose)), "'"]
                                        if condor_job_path is not None:
                                            os.makedirs(condor_job_path, exist_ok=True)
                                            input_condor_resource = os.path.join(condor_job_path, f'{node_name}_input_{input_tag}.resource')
                                            with open(input_condor_resource, 'w') as f:
                                                cpu_config_resource_class.write(f)
                                            input_condor_job = os.path.join(condor_job_path, f'{node_name}_input_{input_tag}.job')
                                            with open(input_condor_job, 'w') as f:
                                                f.write(' '.join(cmd))
                                            with open(input_condor_job, 'r') as f:
                                                print('Executing: \n\t' + f.read() + '\n')
                                                sys.stdout.flush()
                                            input_condor_done = os.path.join(condor_job_path, f'{node_name}_input_{input_tag}.done')
                                            execute_status = False
                                            while True:
                                                if os.path.exists(input_condor_done):
                                                    break
                                                # elif os.path.exists(input_condor_error): exit()
                                                else:
                                                    if not execute_status:
                                                        process_cmd = ' '.join(cmd)
                                                        summary_executing = f'[{process_user}|{process_id}|{datetime.now()}] Waiting for the condor job to finish input creation with input tag {input_tag} with process_cmd:\n' + \
                                                                f'\t{process_cmd}\n'
                                                        log.info(summary_executing)
                                                        sys.stdout.flush()
                                                    execute_status = True
                                                    time.sleep(check_frequency_s_done)
                                        else:
                                            log.info('Executing: \n\t' + ' '.join(cmd))
                                            sys.stdout.flush()
                                            _execute(cmd)
                                
                                    available_input_f.write(f'[{process_user}|{process_id}|{datetime.now()}] Done checking input with tag {input_tag}\n')
                                    available_input_f.flush()
                                    unlock_file(available_input_f)
                                    log.info(f'[{process_user}|{process_id}|{datetime.now()}] Done checking input with tag {input_tag}\n')
                                    sys.stdout.flush()
                                break
                            except OSError or BlockingIOError as e:
                                if not status_update_input:
                                    summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start waiting the locked input with tag {input_tag} with process_cmd:\n' + \
                                            f'\t{process_cmd}\n\t{e}\n'
                                    log.info(summary_process_start)
                                    sys.stdout.flush()
                                status_update_input = True
                                time.sleep(check_frequency_s_input)

                        """2. Weight training"""
                        """start train weight"""
                        weight_script_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'cnn_train_weight.py')
                        cmd = ['python', weight_script_path, '--model_config', model_config_path, '--resource_config', resource_config_path, '--verbose', str(verbose)]
                        # cmd = ["python -c", "'from simplemind.think_module.bin.sm.script.cnn_train_weight import imported_main;", "imported_main(\"{}\", \"{}\", {});".format(model_config_path ,resource_config_path, str(verbose)), "'"]
                        if condor_job_path is not None:
                            os.makedirs(condor_job_path, exist_ok=True)
                            weight_condor_resource = os.path.join(condor_job_path, f'{node_name}_weight_{weight_tag}.resource')
                            with open(weight_condor_resource, 'w') as f:
                                gpu_config_resource_class.write(f)
                            weight_condor_job = os.path.join(condor_job_path, f'{node_name}_weight_{weight_tag}.job')
                            with open(weight_condor_job, 'w') as f:
                                f.write(' '.join(cmd))
                            with open(weight_condor_job, 'r') as f:
                                print('Executing: \n\t' + f.read() + '\n')
                                sys.stdout.flush()
                            weight_condor_done = os.path.join(condor_job_path, f'{node_name}_weight_{weight_tag}.done')
                            execute_status = False
                            while True:
                                if os.path.exists(weight_condor_done):            
                                    break
                                # elif os.path.exists(weight_condor_error): exit()
                                else:
                                    if not execute_status:
                                        process_cmd = ' '.join(cmd)
                                        summary_executing = f'[{process_user}|{process_id}|{datetime.now()}] Waiting for the condor job to finish training weight with weight tag {weight_tag} with process_cmd:\n' + \
                                                f'\t{process_cmd}\n'
                                        log.info(summary_executing)
                                        sys.stdout.flush()
                                    execute_status = True
                                    time.sleep(check_frequency_s_done)
                        else: 
                            log.info('Executing: \n\t' + ' '.join(cmd))
                            sys.stdout.flush()
                            _execute(cmd)
                            
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
                        file_error_str = f"{type(e).__name__} at line {e.__traceback__.tb_lineno} of {__file__}: {e}"
                        file_error_str += '-------------------------------------------------------------------------------------------------\n'
                        file_error_str += 'Full error traceback:\n'
                        file_error_str += '-------------------------------------------------------------------------------------------------\n'
                        file_error_str += f'{traceback.format_exc()}'
                        file_error_str += '-------------------------------------------------------------------------------------------------\n'
                        with open(error_log_path, 'a') as f:
                            log.info(c_error_str + error_str + log_error_str)
                            f.write(error_str + file_error_str)
                            sys.stdout.flush()
                        working_dir = os.path.join(config_model['path_info']['working_directory'], f'{node_name}_KerasModel')
                        os.makedirs(os.path.join(working_dir, f'error_log'), exist_ok=True)
                        error_working_path = os.path.join(working_dir, f'error_log', f'error_{node_name}_{weight_tag}_{input_tag}.log')
                        log_error_str = f'See error log at \n    {error_working_path}\n'
                        log_error_str += '-------------------------------------------------------------------------------------------------\n'
                        with open(error_working_path, 'a') as f:
                            f.write(error_str + file_error_str)
                        log.info(log_error_str)
                        sys.stdout.flush()
                        raise ValueError('cnn_train.py failed with exception ', e)

                    available_weight_f.write(f'[{process_user}|{process_id}|{datetime.now()}] Done checking weight with tag {weight_tag}\n')
                    available_weight_f.flush()
                    unlock_file(available_weight_f)
                    print(f'[{process_user}|{process_id}|{datetime.now()}] Done checking weight with tag {weight_tag}\n')
                    sys.stdout.flush()
                break
        except OSError or BlockingIOError as e:
            if not status_update_weight:
                summary_process_start = f'[{process_user}|{process_id}|{datetime.now()}] Start waiting the locked weight with tag {weight_tag} with process_cmd:\n' + \
                        f'\t{process_cmd}\n\t{e}\n'
                log.info(summary_process_start)
                sys.stdout.flush()
            status_update_weight = True
            time.sleep(check_frequency_s_weight)
            # break
    
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
    formatter = logging.Formatter('[%(name)-6s|%(levelname)-6s|%(filename)-20s:%(lineno)-3s] %(message)s')
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)
    if args.verbose >= 2: log.setLevel(logging.DEBUG)
    elif args.verbose >= 1: log.setLevel(logging.INFO)
    else: log.setLevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    """Model configuration maps"""
    config_model_class = Configurator(args.model_config, log)
    config_model_class.set_config_map(config_model_class.get_section_map())
    config_model = config_model_class.get_config_map()

    """Sanity check for the current model and cnn_train.py"""
    node_name = config_model['model_info']['node_name']
    input_tag = config_model['chromosome_info']['input_tag']
    weight_tag = config_model['chromosome_info']['weights_tag']
    working_dir = os.path.join(config_model['path_info']['working_directory'], f'{node_name}_KerasModel')
    error_working_path = os.path.join(working_dir, f'error_log', f'error_{node_name}_{input_tag}_{weight_tag}.log')
    if (os.path.exists(error_working_path)):
        with open(error_working_path) as f:
            error_string = f.read()
        check_error_string = f'Error Log for {node_name} from {os.path.basename(os.path.abspath(__file__))}'
        if error_string.count(check_error_string) >= 3:
            log.info('---------------------------------------------------------------')
            log.info(f'Error is recursively repeated. Stop to train {node_name}')
            log.info('---------------------------------------------------------------')
            sys.stdout.flush()
            raise ValueError(f'Error is recursively repeated. Stop to train {node_name}')

    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))

    log.info('---------------------------------------------------------------')
    log.info('Model Configuration')
    log.info('---------------------------------------------------------------')
    log.debug(f'Model configuration path: {args.model_config}')
    config_model_class.print_config_map()
    log.debug(config_model)
    
    main(config_model, log, args.model_config, args.resource_config, args.verbose)

    