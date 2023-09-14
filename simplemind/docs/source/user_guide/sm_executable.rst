.. highlight:: shell

************************************************
SM Runner
************************************************

**SM runner** has the structure of:

.. code-block:: console
    
    sm.runner(image_path, sm_model, output_dir, 
            working_directory="", user_resource_directory="", chromosome="",
            skip_png_prediction=False, skip_png_training=True, skip_tensorboard=False)

:image_path: A path of image file to segment,
             e.g., ``/scratch/mbrown/CXR/1.2.276.0.7230010.3.1.3.898003985.12164.1524075050.761_0.seri``
:sm_model: A text file listing the nodes of the segmentation model,
             e.g., ``/radraid/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea/experiments/miu_model_trachea/cxr_trachea_model``
:output_dir: Directory where outputs (blackboard, ROIs, etc) will be stored,
                   e.g., ``/radraid/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea/experiments/experiment_miu/output``

**optional arguments**

======================================  ===============================================================================================
arguments                                 Explanation
======================================  ===============================================================================================
working_directory                       Directory where working files (e.g., EDM, CNN) are read from if they exist 
                                            (were computed previously) or are written otherwise.
user_resource_directory                 Directory where the resource configuration files for CNN nodes are stored. 
                                            If this is not specified, default resource configuration using single core CPU will be used.
chromosome                              Input chromosome (optional), e.g., ``010101010100011100101010101001010101010``
skip_png_prediction                     Whether to skip generating ``.png`` file for reviewing the normalized input image or not. 
                                            If ``-i`` is used, it will skip generating input image review. This option has a priority 
                                            whether node of MIU model has skip_png argument or not.
skip_png_training                       Whether to skip generating ``.png`` file for reviewing the normalized input image for training
                                            phase. If ``-it`` is used, it will skip generating input image review for training, but the 
                                            input image review for prediction will still be generated.
skip_tensorboard                        Whether to skip generating tensorboard log file or not.
                                            If ``-t`` is used, it will skip generating tensorboard log file.
======================================  ===============================================================================================