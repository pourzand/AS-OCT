"""Examples

input_prediction_from_secure_data
- PROMISE12 SM model with pretrained multiple CNNs:
    promise12_dict = {
            'prefix':'promise12', 
            'input_name':'Case20.mhd', 
            'model_name':'sm_model_promise12/prostate_model',
            'final_roi_name':'prostate_whole_cnn.roi'
            }
"""

import pytest

import sys, os
import numpy as np

try:
    import importlib.resources as pkg_resources
except ImportError:
    import importlib_resources as pkg_resources
from pkg_resources import resource_filename
import tarfile

# @pytest.fixture(scope="session")
# def input_prediction(request) -> input_prediction_from_secure_files(*request):
@pytest.fixture(scope="session")
def input_prediction_from_secure_files(tmp_path_factory, request):
    data_prefix, sn_prefix, input_name, model_name, final_roi_name = request.param
    input_data_tar_path = f'.secure_files/{data_prefix}_data.tar.gz'
    input_sn_tar_path = f'.secure_files/{sn_prefix}.tar.gz'
    ref_output_tar_path = f'.secure_files/{sn_prefix}_ref_output.tar.gz'

    temp_dir_path = tmp_path_factory.mktemp('shared_data_tmp')

    input_data_path = temp_dir_path
    with tarfile.open(input_data_tar_path) as f:
        f.extractall(input_data_path)
    image_path = f'{input_data_path}/{data_prefix}_data/{input_name}'

    input_model_path = os.path.join(temp_dir_path, f'{sn_prefix}')
    with tarfile.open(input_sn_tar_path) as f:
        f.extractall(input_model_path)
    sm_model = f'{input_model_path}/{model_name}'
    working_dir = f'{input_model_path}/working_dir'

    ref_output_dir = temp_dir_path
    with tarfile.open(ref_output_tar_path) as f:
        f.extractall(ref_output_dir)
    ref_prediction_roi_path = f'{ref_output_dir}/{final_roi_name}'

    output_dir = os.path.join(temp_dir_path, f'{sn_prefix}_ref_output')
    prediction_roi_file = f'{output_dir}/{final_roi_name}'
    return image_path, sm_model, output_dir, working_dir, prediction_roi_file, ref_prediction_roi_path