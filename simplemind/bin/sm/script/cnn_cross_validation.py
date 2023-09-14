"""Cross Validation

This script will do cross-validation experiment.
    * main - the main function of the script

Parameters
----------
--miu: str
    miu_path
--working_dir: str
    working directory
--model_file: str
    model_file path
--output_dir: str
    output_dir path
--resource_config: str
    resource_config path
-v: int
    '--verbose': (optional) default=0
    set the logging level
        0: no logging (critical error only)
        1: info level
        2: debug level

Examples
--------
Using cnn_cross_validation.py:
    1. [mhd; single case; single channel; 3D]
    Ex) Prostate organ segmentation with a public data PROMISE12
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export script=$qia_home/bin/miu/script/cnn_cross_validation.py
    export miu=$qia_home/bin/miu/miu
    export base_dir=/scratch/youngwon/miu_examples/public_PROMISE12
    export working_dir=$base_dir/experiments/experiment_miu
    export model_file=$base_dir/experiments/miu_model_prostate/prostate_model
    export node_name=prostate_cnn
    export output_dir=$base_dir/experiments/experiment_miu/cv_output
    export resource_config=$working_dir/prostate_cnn_KerasModel/prostate_cnn_resource.ini
    export log_file=$working_dir/log_cnn_cv_single_channel.log
    python $script --miu=$miu --working_dir=$working_dir --model_file=$model_file --node_name=prostate_cnn --output_dir=$output_dir --resource_config=$resource_config -v 2 | tee $log_file
    ```

    Ex) Prostate organ segmentation with a in-house data
    ```
    export qia_home=/cvib2/apps/personal/youngwonchoi/project/QIA/dev_yw
    export script=$qia_home/bin/miu/script/cnn_cross_validation.py
    export miu=$qia_home/bin/miu/miu
    export base_dir=/scratch/youngwon/miu_examples/inhouse_10042
    export working_dir=$base_dir/experiments/experiment_miu
    export model_file=$base_dir/experiments/miu_model_prostate/prostate_model
    export node_name=prostate_cnn
    export output_dir=$base_dir/experiments/experiment_miu/cv_output
    export resource_config=$working_dir/prostate_cnn_KerasModel/prostate_cnn_resource.ini
    export log_file=$working_dir/log_cnn_cv_single_channel.log
    python $script --miu=$miu --working_dir=$working_dir --model_file=$model_file --node_name=prostate_cnn --output_dir=$output_dir --resource_config=$resource_config -v 2 | tee $log_file
    ```

    2. TODO [mhd; single case; multi-channel; 3D]
    Ex) Prostate organ segmentation with a in-house data
"""

import os
import sys
import logging
from argparse import ArgumentParser
import shutil
import distutils.dir_util as distdirutil
import glob
import subprocess
import timeit
import traceback
import warnings
warnings.filterwarnings("ignore")

import numpy as np
import csv
from sklearn.model_selection import KFold

# import __qia__
# import qia.common.img.image as qimage
# from cnn_configurator import Configurator

def execute(cmd):
    proc = subprocess.Popen(cmd, shell=False)
    return proc.wait()

def main(miu, working_dir, model_file, node_name, output_dir, resource_config, log, nfold=5):
    log.info('---------------------------------------------------------------')
    log.info('Configurations')
    log.info('---------------------------------------------------------------')
    log.info(f'MIU path: {miu}')
    log.info(f'Working directory: {working_dir}')
    log.info(f'Model file: {model_file}')
    log.info(f'Node name: {node_name}')
    log.info(f'Ourput direcory: {output_dir}')
    log.info(f'Resource configuration file: {resource_config}')
    log.info('---------------------------------------------------------------')
    log.info(f'Prepare directories for {nfold}-fold Cross-Validation')
    log.info('---------------------------------------------------------------')
    
    """Prepare output directorys for CV"""
    try:
        if os.path.exists(output_dir):
            """
            Assume each fold working dir, input, train, test list are exists
            
            TODO
            ----
            how to handle this..remove? similar to -f?
            """
            # shutil.rmtree(output_dir)
            # # for root, dirs, files in os.walk(output_dir, topdown=False):         
            # #     for name in dirs:            
            # #         os.rmdir(os.path.join(root, name))
            
            for fold in range(nfold):
                fold_working_dir = os.path.join(output_dir,f'{fold}_fold')
                # os.makedirs(fold_working_dir, exist_ok=True)
                
                """copy model"""
                model_dir = os.path.dirname(model_file)
                model_name = os.path.basename(model_file)
                fold_model_dir = os.path.join(fold_working_dir, os.path.basename(model_dir))
                fold_model_path = os.path.join(fold_model_dir, model_name)
                log.debug(f'Model directory to copy: {model_dir}')
                log.debug(f'Model directory to write: {model_dir}')
                distdirutil.copy_tree(model_dir, fold_model_dir)
                log.debug(f'{fold}-fold model path: {fold_model_path}')
                # if os.path.exists(output_dir):
                
                """copy resource_config"""
                fold_working_dir_cnn = os.path.join(fold_working_dir, f'{node_name}_KerasModel')
                os.makedirs(fold_working_dir_cnn, exist_ok=True)
                shutil.copy(resource_config, fold_working_dir_cnn)
                resource_config_filename = os.path.basename(resource_config)
                fold_resource_config =os.path.join(fold_working_dir_cnn, resource_config_filename)
                log.debug(f'{fold}-fold resource_config: {fold_resource_config}')
        else:
            os.makedirs(output_dir)

            """Read train_list.csv"""
            """TODO: ask convention"""
            train_list_path = os.path.join(os.path.join(working_dir, f'{node_name}_KerasModel'), 'train_list.csv')
            log.info(f'train_list file to read: {train_list_path}')
            train_list_dict = np.array(list(csv.DictReader(open(train_list_path,'r'))))

            """get CV index"""
            random_state = 12
            kf = KFold(n_splits=nfold, shuffle=True, random_state=random_state)
            cv_index = kf.split(range(train_list_dict.shape[0]))
            train_tot_idxs = []
            test_tot_idxs = []
            for fold, (train_idx, test_idx) in enumerate(cv_index):
                train_tot_idxs.append(np.array(train_idx))
                test_tot_idxs.append(np.array(test_idx))
            train_tot_idxs = np.array(train_tot_idxs)
            test_tot_idxs = np.array(test_tot_idxs)
            # train_tot_idxs_path = os.path.join(output_dir, 'cv_total_train_index.npy')
            # test_tot_idxs_path = os.path.join(output_dir, 'cv_total_test_index.npy')
            # np.save(train_tot_idxs_path, train_tot_idxs)
            # np.save(test_tot_idxs_path, test_tot_idxs)
            
            fold_dirs = []
            for fold in range(nfold):
                """make working directory"""
                fold_working_dir = os.path.join(output_dir,f'{fold}_fold')
                os.makedirs(fold_working_dir, exist_ok=True)
                fold_output_dir = os.path.join(fold_working_dir,'output')
                
                """copy model"""
                model_dir = os.path.dirname(model_file)
                model_name = os.path.basename(model_file)
                fold_model_dir = os.path.join(fold_working_dir, os.path.basename(model_dir))
                fold_model_path = os.path.join(fold_model_dir, model_name)
                log.debug(f'Model directory to copy: {model_dir}')
                log.debug(f'Model directory to write: {model_dir}')
                distdirutil.copy_tree(model_dir, fold_model_dir)
                log.debug(f'{fold}-fold model path: {fold_model_path}')
                # if os.path.exists(output_dir):
                
                """copy resource_config"""
                fold_working_dir_cnn = os.path.join(fold_working_dir, f'{node_name}_KerasModel')
                os.makedirs(fold_working_dir_cnn, exist_ok=True)
                shutil.copy(resource_config, fold_working_dir_cnn)
                resource_config_filename = os.path.basename(resource_config)
                fold_resource_config =os.path.join(fold_working_dir_cnn, resource_config_filename)
                log.debug(f'{fold}-fold resource_config: {fold_resource_config}')
                
                # """copy tensorboard config. TODO: fix"""
                # node_name = '_'.join(os.path.basename(resource_config).split('_')[:-1])
                # tb_config_filename = f'{node_name}_tensorboard.ini'
                # tensorboard_config = os.path.join(os.path.dirname(resource_config), tb_config_filename)
                # fold_tensorboard_config = os.path.join(fold_working_dir_cnn, tb_config_filename)
                # log.debug(f'tensorboard_config to copy: {tensorboard_config}')
                # shutil.copy(tensorboard_config, fold_working_dir_cnn)
                # log.debug(f'{fold}-fold tensorboard_config: {fold_tensorboard_config}')

                """make train_list.csv"""
                fold_train_idxs = train_tot_idxs[fold]
                fold_train_list_dict = train_list_dict[fold_train_idxs]
                log.debug(f'{fold}-fold sample for train: {len(fold_train_list_dict)}')
                fold_train_list_path = os.path.join(fold_working_dir_cnn, 'train_list.csv')
                log.info(f'{fold}-fold train_list file to write: {fold_train_list_path}')
                with open(fold_train_list_path, 'w', newline='') as csvfile:
                    fieldnames = fold_train_list_dict[0].keys()
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    writer.writeheader()
                    for fold_case in fold_train_list_dict:
                        writer.writerow(fold_case)
                log.info(f'Finished to write {fold}-fold train_list: {fold_train_list_path}')
                
                """make test_list.csv"""
                fold_test_idxs = test_tot_idxs[fold]
                fold_test_list_dict = train_list_dict[fold_test_idxs]
                log.debug(f'{fold}-fold sample for test: {len(fold_test_list_dict)}')
                fold_test_list_path = os.path.join(fold_working_dir_cnn, 'test_list.csv')
                log.info(f'{fold}-fold test_list file to write: {fold_test_list_path}')
                with open(fold_test_list_path, 'w', newline='') as csvfile:
                    fieldnames = fold_test_list_dict[0].keys()
                    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
                    writer.writeheader()
                    for fold_case in fold_test_list_dict:
                        writer.writerow(fold_case)
                log.info(f'Finished to write {fold}-fold test_list: {fold_test_list_path}')

                # fold_dict = {'fold_model_path':fold_model_path,
                #     'fold_model_dir':fold_model_dir,
                #     'fold_working_dir':fold_working_dir,
                #     'fold_output_dir':fold_output_dir,
                #     'fold_resource_config':fold_resource_config,
                #     'fold_tensorboard_config':fold_tensorboard_config,
                #     'fold_train_list_path':fold_train_list_path,
                #     'fold_test_list_path':fold_test_list_path}
                # fold_dirs.append(fold_dict)
                log.info('---------------------------------------------------------------')
    except OSError as e:
        raise ValueError(f'Error: {output_dir} : {e.strerror}')
    
    log.debug(f'Check the prepared {nfold}-fold cross-validation output directory:')
    log.debug(f'{output_dir}')
    log.info('---------------------------------------------------------------')
    for currentpath, folders, files in os.walk(output_dir):
        log.debug(f'{currentpath}, {folders}, {files}')

    fold_dirs = []
    for fold in range(nfold):
        fold_working_dir = os.path.join(output_dir,f'{fold}_fold')
        model_dir = os.path.dirname(model_file)
        model_name = os.path.basename(model_file)
        fold_model_dir = os.path.join(fold_working_dir, os.path.basename(model_dir))
        fold_model_path = os.path.join(fold_model_dir, model_name)
        fold_output_dir = os.path.join(fold_working_dir,'output')
        resource_config_filename = os.path.basename(resource_config)
        # node_name = '_'.join(os.path.basename(resource_config).split('_')[:-1])
        tb_config_filename = f'{node_name}_tensorboard.ini'
        # tensorboard_config = os.path.join(os.path.dirname(resource_config), tb_config_filename)
        fold_working_dir_cnn = os.path.join(fold_working_dir, f'{node_name}_KerasModel')
        fold_tensorboard_config = os.path.join(fold_working_dir_cnn, tb_config_filename)
        fold_resource_config =os.path.join(fold_working_dir_cnn, resource_config_filename)
        fold_train_list_path = os.path.join(fold_working_dir_cnn, 'train_list.csv')
        fold_test_list_path = os.path.join(fold_working_dir_cnn, 'test_list.csv')
        fold_dict = {'node_name':node_name,
                    'fold_model_path':fold_model_path,
                    'fold_model_dir':fold_model_dir,
                    'fold_working_dir':fold_working_dir,
                    'fold_output_dir':fold_output_dir,
                    'fold_resource_config':fold_resource_config,
                    'fold_tensorboard_config':fold_tensorboard_config,
                    'fold_train_list_path':fold_train_list_path,
                    'fold_test_list_path':fold_test_list_path}
        fold_dirs.append(fold_dict)
    log.info('---------------------------------------------------------------')
    log.info(f'Computing 5-fold CV')
    log.info('---------------------------------------------------------------')
    for fold in range(nfold):
        fold_dict = fold_dirs[fold]
        node_name = fold_dict['node_name']
        fold_model_path = fold_dict['fold_model_path']
        fold_model_dir = fold_dict['fold_model_dir']
        fold_working_dir = fold_dict['fold_working_dir']
        fold_output_dir = fold_dict['fold_output_dir']
        fold_resource_config = fold_dict['fold_resource_config']
        fold_tensorboard_config = fold_dict['fold_tensorboard_config']
        fold_train_list_path = fold_dict['fold_train_list_path']
        fold_test_list_path = fold_dict['fold_test_list_path']

        log.info(f'Computing {fold}th-fold')
        log.info(f'{fold} model_file: {fold_model_path}')
        log.info(f'{fold} working dir: {fold_working_dir}')
        log.info(f'{fold} output_dir: {fold_output_dir}')
        log.info(f'{fold} resource config: {fold_resource_config}')
        log.info(f'{fold} tensorbaord config: {fold_tensorboard_config}')
        log.info('---------------------------------------------------------------')
        log.info(f'{fold}-fold test_list file to read: {fold_test_list_path}')
        test_list_dict = np.array(list(csv.DictReader(open(fold_test_list_path,'r'))))
        log.info('---------------------------------------------------------------')
        log.info(f'{fold}-fold training start')
        log.info('---------------------------------------------------------------')
        fold_image_path = test_list_dict[0]['image']
        # log.debug(fold_image_path)
        cmd = [miu, fold_image_path, fold_model_path, fold_output_dir,
        '-d', fold_working_dir, '-f']
        execute(cmd)

        # fold_weight_dir = os.path.join(fold_working_dir, f'{node_name}_KerasModel', 'weights')
        # weight_name = os.path.basename(glob.glob(os.path.join(fold_weight_dir, '*.h5'))[0])
        # fold_best_weight_path = os.path.join(fold_weight_dir, weight_name)
        # new_weight_name = '.'.join(weight_name.split('.')[:-1]) + '.hd5'
        # copy_fold_best_weight = os.path.join(os.path.dirname(fold_weight_dir), new_weight_name)
        # # copy_fold_best_weight = os.path.join(fold_model_dir, new_weight_name)
        # if not os.path.exists(copy_fold_best_weight):
        # # if len(glob.glob(os.path.join(fold_model_dir, '*.hd5'))) == 0:
        #     log.info(f'{fold}-fold trained best weight {fold_best_weight_path}')
        #     log.info(f'copied into {fold_model_dir}')
        #     log.info(f'as {new_weight_name}')
        #     shutil.copy(fold_best_weight_path, copy_fold_best_weight)
        log.info('---------------------------------------------------------------')
        log.info(f'{fold}-fold prediction')
        log.info('---------------------------------------------------------------')
        fold_prediction_dir = os.path.join(fold_working_dir, 'test_predictions')
        os.makedirs(fold_prediction_dir, exist_ok=True)
        for fold_case_dict in test_list_dict:
            case = fold_case_dict['']
            fold_case_image_path = fold_case_dict['image']
            log.debug('---------------------------------------------------------------')
            log.debug(f'Predict case {case} with img: {fold_case_image_path}')
            log.debug('---------------------------------------------------------------')
            prediction_output_dir=os.path.join(fold_prediction_dir,case)
            cmd = [miu, fold_case_image_path, fold_model_path, prediction_output_dir,
                    '-d', fold_working_dir, '-f']
            execute(cmd)
            # output_prediction_path = os.path.join(fold_output_dir, f'{node_name}.roi')
            # copy_prediction_path = os.path.join(fold_prediction_dir, f'{node_name}_{case}.roi')
            # log.debug(output_prediction_path)
            # shutil.copy(output_prediction_path, copy_prediction_path)
            prediction_saved_path = os.path.join(prediction_output_dir, f'{node_name}.roi')
            log.debug(f'Prediction saved: {prediction_saved_path}')
            log.debug(f'Fishied to predict case {case}.')
            log.debug('---------------------------------------------------------------')

if __name__=='__main__':
    """Parsing the input arguments"""
    parser = ArgumentParser(description='Script to cross-validation experiment using MIU.')
    parser.add_argument('-s', '--miu', type=str, dest='miu',
                        help="miu_path")
    parser.add_argument('-d', '--working_dir', type=str, dest='working_dir', 
                        help="working directory")
    parser.add_argument('-m', '--model_file', type=str, dest='model_file', 
                        help="model_file path")
    parser.add_argument('-n', '--node_name', type=str, dest='node_name', 
                        help="node name")
    parser.add_argument('-o', '--output_dir', type=str, dest='output_dir', 
                        help="output_dir path")
    parser.add_argument('-r', '--resource_config', type=str, dest='resource_config', 
                        help="resource_config path")
    parser.add_argument('-v', '--verbose', type=int, dest='verbose', default=0,
                        help="logging level")
    args = parser.parse_args()

    """logging: using basic logger from logging
    TODO: set color
    https://stackoverflow.com/questions/45923290/how-to-get-the-current-log-level-in-python-logging-module
    """
    logging.basicConfig(format = '[%(name)-10s|%(levelname)-8s|%(filename)-20s:%(lineno)-3s] %(message)s',
                    level=logging.DEBUG)
    log = logging.getLogger()
    if args.verbose >= 2: log.setLevel(logging.DEBUG)
    elif args.verbose >= 1: log.setlevel(logging.INFO)
    else: log.setlevel(logging.WARNING) # logging.ERROR / logging.CRITICAL
    
    log.info('---------------------------------------------------------------')
    log.info('Python Environments')
    log.info('---------------------------------------------------------------')
    log.info('python v. %d.%d.%d' % (sys.version_info[:3]))
    log.info(f'numpy v. {np.__version__}')
    
    main(args.miu, args.working_dir, args.model_file, args.node_name, args.output_dir, args.resource_config, log)