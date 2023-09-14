#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Tests for `simplemind` package.

From the project root:
```
docker run -it -u $(id -u):$(id -g) -v $PWD:/workdir -v ~/.local:/.local -w /workdir sm_release:latest bash -c "python setup.py develop --prefix=/.local; pytest -s --log-cli-level=10"
```
"""

import pytest

import os
import random
import logging
import tensorflow as tf
import numpy as np
import medpy.metric as medmetric

from simplemind import sm
# from simplemind import sm_model_summary
# will be changed to be open-souce image reader soon
from simplemind import __qia__
import qia.common.img.image as qimage

####################################################################################################################
# Simple test for the loss functions
####################################################################################################################
## Segmentation losses
# def test_loss_numpy():
#     y_true_set = np.array([[0,1],
#                             [1,0],
#                             [0,1],
#                             [1,0],
#                             [1,0]])
#     y_pred_set = np.array([[0,1],
#                             [0.1,0.9],
#                             [0.4,0.6],
#                             [0,1],
#                             [0.7,0.3]])
#     assert np.all(np.isclose(np.array([0.6, 0.6, 0.6, 0.6, 0.5833333333333333]), loss_and_metric.metric_test(y_true_set, y_pred_set)))
    

####################################################################################################################
# SM model summary tools
####################################################################################################################
# def test_model_summary():
#     sm_model = sm_model_summary.SMBaseModel(model_path, log)
#     G, fig = sm_model.get_summary_graph(figsize=(7,10), font_size=12)

####################################################################################################################
# Testing SM runner
# - Single-CNN Segmentation with PROMISE12
# - Multi-CNNs Segmentation with PROMISE12
# - Multi-CNNs CXR ettip placement
# TODO: resource parametrize
# - CPU
# - GPU
####################################################################################################################
promise12_predict = [
        'promise12', # data prefix
        'promise12_prediction', # model prefix
        'Case20.mhd', 
        'sn_promise12/prostate_model',
        'prostate_whole_cnn.roi'
        ]
promise12_complicated_predict = [
        'promise12', # data prefix
        'promise12_complicated_prediction', # model prefix
        'Case20.mhd', 
        'sn_promise12/prostate_model',
        'prostate_final.roi'
        ]
####################################################################################################################
# @pytest.mark.parametrize('input_config, resource_config', [ 
#     (promise12_predict, 'cpu')
#     ])
@pytest.mark.parametrize('input_prediction_from_secure_files', [ 
    # (promise12_predict),
    (promise12_complicated_predict),
    ], indirect=True)
def test_sm_runner(input_prediction_from_secure_files):
    '''
    Test simplemind sm.runner prediction
    '''
    seed_value = 123
    os.environ['PYTHONHASHSEED']=str(seed_value)
    random.seed(seed_value)
    np.random.seed(seed_value)
    tf.set_random_seed(seed_value)

    log = logging.getLogger()
    formatter = logging.Formatter('[%(asctime)s|%(name)-10s|%(levelname)-8s|%(filename)-25s:%(lineno)-3s] %(message)s')
    log.setLevel(logging.DEBUG)
    ch = logging.StreamHandler()
    ch.setFormatter(formatter)
    log.addHandler(ch)

    image_path, sn_path, output_dir, working_dir, prediction_roi_file, ref_prediction_roi_path = input_prediction_from_secure_files

    return_value = sm.runner(image_path = image_path, 
                            sn_entry_path = sn_path, 
                            output_dir = output_dir, 
                            working_directory=working_dir,
                            user_resource_directory="", 
                            chromosome="", 
                            skip_png_prediction=True, 
                            skip_png_training=True, 
                            skip_tensorboard=True) 
                            # logging path
    log.info('--------------------------------------------------------------------------------------------')
    log.info(f'SM runner execution finished.')
    log.info('--------------------------------------------------------------------------------------------')
    
    log.info('Generated output directory has:')
    log.info('\n\t'.join(os.listdir(output_dir)))
    log.info('--------------------------------------------------------------------------------------------')

    # qimage will be disregarded soon.
    # TODO: build wrapper sm_image_reader
    if np.all([os.path.exists(image_path), os.path.exists(ref_prediction_roi_path)]):
        log.info('Compare the prediction result with the reference prediction of the pretrained model')
    else:
        raise ValueError(f'Someting wrong with the test data. \
            \n\tImage file exist? {os.path.exists(image_path)} in here: {image_path} \
            \n\tRefernce prediction roi file exist? {os.path.exists(ref_prediction_roi_path)} in here: {ref_prediction_roi_path}')
    if os.path.exists(prediction_roi_file):
        image = qimage.read(image_path)
        # img = image.get_array()
        pred = qimage.cast(image)
        pred.fill_with_roi(prediction_roi_file)
        prediction_arr = pred.get_array()

        # ref_pred = qimage.read(ref_roi_path)
        ref_pred = qimage.cast(image)
        ref_pred.fill_with_roi(ref_prediction_roi_path)
        ref_prediction_arr = ref_pred.get_array()

        diceAB = medmetric.dc(prediction_arr, ref_prediction_arr)
        log.info(f'Dice(prediction, ref_prediction) = {diceAB}')
        assert np.isclose(diceAB, 1.)
        log.info(f'Direct comparison: {np.all(np.isclose(prediction_arr, ref_prediction_arr))}')
        assert np.all(np.isclose(prediction_arr, ref_prediction_arr))
    else:
        raise ValueError(f'Someting wrong with prediction. \n\tPrediction roi not exists in here: {prediction_roi_file}')
    log.info('--------------------------------------------------------------------------------------------')
    log.info('Generated input review figures:')
    log.info('\n\t'.join([x for x in os.listdir(output_dir) if '.png' in x]))
    log.info('--------------------------------------------------------------------------------------------')
    log.info('Finish testing simplemind sm.runner prediction')
    log.info('--------------------------------------------------------------------------------------------')
    

# def test_agent_cnn_prediction(input_value_sm_runner):