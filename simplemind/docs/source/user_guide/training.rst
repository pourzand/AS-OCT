.. highlight:: shell

************************************************
Training CNN Nodes
************************************************

If you want to train CNN nodes while executing your SM model, 
you need to prepare additional prerequisities related to the CNN training.

Prerequisites for CNN Node Training and Executing SM Model
===============================================================

You will need:

1. **SM model** (See `SM Model`_),

2. **Image file** for execution after all CNN training is finished, 

3. **Working directory** for saving any requirements and temporary output files for calculation,

4. **List of samples for training** each CNN nodes.


Optionally, you can set **resource directory** for GPU/CPU resource control. 
Without **resource directory**, SM will run through single-core CPU by default.

SM Model
------------------------------------------------
Now we have some CNN nodes that we want to train.
Let's assume your example SM model file ``example_model``:

.. code-block:: console
    
    Model: example_model
    1
    node_5
    cnn_node_4
    node_3
    cnn_node_2
    node_1
    End: example_model;

is placed under the directory ``\your\SM\model\directory``.
We need the corresponding SM nodes under this directory. 

If you have a specific CNN node that you want to use pre-trained weights 
for prediction without training,
place your pre-trained weight file in this directory too.
*Without the weight file, SM will train that CNN Node.*

See `SM Model`_ section for the detailed explanations.


Image File
---------------

This is an input image file to calculate the ROI after all the CNN training is finished.
SM supports various 2D / 3D image input format.
(See `Executing SM Model`_)

Working Directory
-------------------------

If you have CNN nodes that need training within your SM model,
you need to prepare the working directory with a specific structure.
Please follow the step for each CNN Node. If your CNN node has the name ``{cnn_node}``, 

1. Make the directory ``{cnn_node}_KerasModel``
2. Make a list of samples for training under each directory ``{cnn_node}_KerasModel``. Here is an example:

.. code-block:: console

    example_working_dir
    ㄴcnn_node_2_KerasModel
      ㄴtrain_list.csv
    ㄴcnn_node_4_KerasModel
      ㄴtrain_list.csv

List of Samples for Training
-----------------------------
The file of the list of samples for training ``train_list.csv`` should be placed 
at the CNN node directory under the working directory.
This file will let SM know about the path of images and ROIs to train the CNN node.

This file ``train_list.csv`` should follow the specific format. 
Below is the example of ``train_list.csv``.

.. csv-table:: Example of training list ``train_list.csv``
    :class: longtable
    :file: ../data/train_list.csv
    :header-rows: 1


By convention we store the image .seri files and reference ROIs in a data subfolder, and the input ROIs in an inp_roi folder

Instead of a reference ROI, (x, y) reference coordinates can also be provided. If the reference is coordinates rather than masks (and the CNN prediction will be coordinates) then those will be stored in the train_list.csv

* cnn_train.py determines what type of reference to expect base on the first line of train_list.csv - it looks for ‘ref_roi’ in this first line
* If the reference is to indicate no object coordinates in the image (e.g., no ET tip exists) then -1, -1 is to be used
* The training code will convert this to -0.5, -0.5 relative position with the intent that the CNN output is to be considered no detection if the coordinates are outside the image range (< 0)
* In the event that after cropping, based on the search area, the reference coordinates are outside the cropped image, then -0.5, -0.5 is also used in the training data file to indicate the tip is not present in the cropped training image


(Optional) Resource Directory
------------------------------------
If you want to control GPU/CPU resources, you can add **resource directory** option (``-u``).
(See `Executing SM Model`_)

In the example SM model file ``example_model``, we have two CNN nodes.

If you want to use **resource directory** option, the example of the resource directory ``example_resource_dir`` should have the structure below:

.. code-block:: console
  :emphasize-lines: 7-9

    example_working_dir
    ㄴcnn_node_2_KerasModel
      ㄴtrain_list.csv
    ㄴcnn_node_4_KerasModel
      ㄴtrain_list.csv

    example_resource_dir
    ㄴcnn_node_2_resource.ini
    ㄴcnn_node_4_resource.ini

Here, the required format of a resource configuration file is:

.. code-block:: ini

    [GPU]
    gpu_cores=0
        ; GPU cores to use
        ; e.g., gpu_cores=3
        ;    This option will let SM to use GPU number 3.
        ; e.g., gpu_cores=2,3
        ;    This option will let SM to use multiple GPUs with number 2 and 3.
        ; e.g., gpu_cores=
        ;    This option will limit SM not to use any GPUs.
    memory_growth=True
        ; whether the process can use un-fixed size of GPU memory 
        ; or will use fixed persentage of GPU memory for each core
        ; If false then the GPU memory will be limited by 
        ; the given memory_percentage value
    memory_limit=1024
        ; Maximum memory (in MB) to allocate on the virtual device. Currently only supported for GPUs.
        ; will be ignored if memory_growth=True

    [CPU]
    max_queue_size=10
        ; maximum number of mini-batch to queue in the CPU memory
    num_cpu_core=20
        ; number of cpu workers
    use_multiprocessing=True
        ; whether to use multiprocessing

.. hint::  See `Real Examples`_ section for the example for various problems.


Call SM Executable
=====================================================
..  (Locally)

1. Prepare the prerequisities. For example, you have
   
   .. code-block:: console

       - SM_model_dir
         ㄴ{example_model}
         ㄴ{node_1}
         ㄴ{cnn_node_2}
         ㄴ{node_3}
         ㄴ{cnn_node_4}
         ㄴ{node_5}
       - Image_file
       - example_working_dir
         ㄴ{cnn_node_2}_KerasModel
           ㄴtrain_list.csv
         ㄴ{cnn_node_4}_KerasModel
           ㄴtrain_list.csv
       - example_resource_dir
         ㄴ{cnn_node_2}_resource.ini
         ㄴ{cnn_node_4}_resource.ini

2. Run docker with the cmd:
   
   .. code-block:: bash

       docker run -it --rm -v $PWD:/workdir -w /workdir \
       --name executing_SM sm_release:latest bash


   .. attention:: You should need to mount the volume that your SM model, image files and other prerequisities to access them inside the docker container. 
     For example, you can add the option ``-v /scratch:/scratch`` to access ``/scratch`` inside your docker container.

     .. code-block:: bash
       :emphasize-lines: 4

       docker run -it --rm -v $PWD:/workdir -w /workdir \
       -v /scratch:/scratch \
       --name executing_SM sm_release:latest bash


3. Run SM runner in python:
   
   .. code-block:: python
       :emphasize-lines: 4,6

       from simplemind import sm

       image_path="/your/image/file/path"
       sm_model="/your/SM/model/file/path"
       output_dir="/your/output/directory"
       working_directory="/your/working/directory"
       user_resource_directory="/your/resource/directory"
       sm.runner(image_path=image_path, sm_model=sm_model, output_dir=output_dir,  \
                working_directory=working_directory, \
                user_resource_directory=user_resource_directory)


   Since ``{cnn_node_2}`` and ``{cnn_node_4}`` have no pre-trained weight files in ``SM_model_dir`` nor working directory, 
   SM will start train ``{cnn_node_2}`` and ``{cnn_node_4}``. 

   After all the CNN trainings are finished, 
   SM will calculate the ROIs from ``$input_file`` with those trained weights. The results will be
   saved in ``$output_dir``.

   After executing SM, you will have the example output below. Highlighted lines will be generated by SM.
   
   .. code-block:: console
       :emphasize-lines: 12-37, 40-80
       
       - SM_model_dir
         ㄴ{example_model}
         ㄴ{node_1}
         ㄴ{cnn_node_2}
         ㄴ{node_3}
         ㄴ{cnn_node_4}
         ㄴ{node_5}
       - Image_file
       - example_working_dir
         ㄴ{cnn_node_2}_KerasModel
           ㄴtrain_list.csv
           ㄴconfig
             ㄴ{cnn_node_2}_{bit_tag}_config.ini
           ㄴlogs
             ㄴ{cnn_node_2}_{bit_tag}.csv
           ㄴweights
             ㄴ{cnn_node_2}_{bit_tag}.hd5
               ㄴ{bit_tag}
                 ㄴmodel.yaml
                   ㄴ{cnn_node_2}_{bit_tag}.hdf5
           ㄴtb_logs
             ㄴ{bit_tag}
               ㄴsome_tensorboard_logs
           ㄴinputs
             ㄴtrain_{bit_tag}.hdf5
             ㄴvalid_{bit_tag}.hdf5
             ㄴprevious_node_rois
               ㄴ{bit_tag}
                 ㄴ0
                 ㄴ1
                 ㄴ...
                 ㄴ{index in the train_list.csv}
                   ㄴ{cnn_node_2}_{normalization_1}_input_image.png
                   ㄴ{cnn_node_2}_{normalization_2}_input_image.png
                     ㄴancestor ROIs of {cnn_node_2} and other SM prediction outputs
                     ㄴ...
                 ㄴ{Total number of samples in train_list.csv}
         ㄴ{cnn_node_4}_KerasModel
           ㄴtrain_list.csv
           ㄴconfig
             ㄴ{cnn_node_4}_{bit_tag}_config.ini
           ㄴlogs
             ㄴ{cnn_node_4}_{bit_tag}.csv
           ㄴweights
             ㄴ{cnn_node_4}_{bit_tag}.hd5
               ㄴ{bit_tag}
                 ㄴmodel.yaml
                   ㄴ{cnn_node_4}_{bit_tag}.hdf5
           ㄴtb_logs
             ㄴ{bit_tag}
               ㄴsome_tensorboard_logs
           ㄴinputs
             ㄴtrain_{bit_tag}.hdf5
             ㄴvalid_{bit_tag}.hdf5
             ㄴprevious_node_rois
               ㄴ{bit_tag}
                 ㄴ0
                 ㄴ1
                 ㄴ...
                 ㄴ{index in the train_list.csv}
                   ㄴ{cnn_node_4}_{normalization}_input_image.png
                     ㄴancestor ROIs of {cnn_node_4} and other SM prediction outputs
                     ㄴ...
                 ㄴ{Total number of samples in train_list.csv}
       - output_dir
         ㄴ{node_1}.roi
         ㄴ{cnn_node_2}_config.ini
         ㄴ{cnn_node_2}.roi
         ㄴ{cnn_node_2}_{normalization_1}_input_image.png
         ㄴ{cnn_node_2}_{normalization_2}_input_image.png
         ㄴ{node_3}.roi
         ㄴ{cnn_node_4}_config.ini
         ㄴ{cnn_node_4}.roi
         ㄴ{cnn_node_4}_{normalization}_input_image.png
         ㄴ{node_5}.roi
         ㄴpred.roi
         ㄴsolution_info.txt
         ㄴsource_image.txr
         ㄴfile_list.txt
         ㄴblackboard.out
       - example_resource_dir
         ㄴ{cnn_node_2}_resource.ini
         ㄴ{cnn_node_4}_resource.ini

Outputs from CNN trainings within Working Directory
-----------------------------------------------------

**config**

* Configuration about the CNN training.
* About model architectures, epochs, batch_size, learning_rates, etc.
* Automatically generated from the NeuralNetKeras attributes in the CNN node

**inputs**

* Temporary files about pre-processed inputs for CNN training.
* Sub-structure under ``inputs`` directory can vary with the ``reader_class`` for CNN nodes.

**logs**

* Log file about the train/validation performances for each epoch.

**weights**

* Trained weights
* Files under the directory ``{bit_tag}`` are the temporary weights saved during the training.

**tb_logs**

* Files for tensorboard


Tensorboard
=====================================================

To run the tensorboard with the port number `6006`,

1. Run docker and launch the tensorboard:
   
   .. code-block:: bash

       export tb_port=6006
       export working_dir=/your/working/directory
       docker run -it --rm -p $tb_port:$tb_port -v $PWD:/workdir -w /workdir \ 
       --name tb_SM sm_release:latest bash -c "tensorboard --logdir=${working_dir} --port=${tb_port}"

2. Open the browser for the tensorboard

   .. code-block:: console

       http://localhost:6006


You can change the port number to what you want to use.

.. todo:: Explanation about the port number in the documentation



.. _SM Model: ./user_guide/SM_model.rst
.. _Real Examples: ./examples.rst
.. _Installation: ./installation.rst
.. _Executing SM Model: ./executing.rst
