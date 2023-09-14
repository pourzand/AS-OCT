.. highlight:: shell

We consider "Users" to be those that want to run an existing SM model and those that want to develop and validate an SM model. 

************************************************
SM Model
************************************************

SM model consists of **the SM model file** and corresponding **SM nodes**.

The SM model file and corresponding nodes should be placed in the same directory.

SM Model File
============================================
SM model file is a text file that has a list of SM nodes, 
and the relationships between those nodes.

Example of SM model file ``trachea_model``:

.. code-block:: console

    Model: trachea_model
    1
    trachea_final
    trachea_finetune_cnn
    trachea_smooth
    trachea_cnn
    trachea_input
    End: trachea_model;

    Comments section

.. hint:: (WIP) You can see the real example of ``trachea_model`` here: ``/radraid/apps/personal/youngwon/miu_examples/inhouse_cxr_trachea/experiments/miu_model_trachea/``

SM Nodes and SM Model Diretory
============================================

Every corresponding SM nodes listed in the SM model file (e.g., ``trachea_model``) should 
be placed under the same directory that the SM model file is exist.

Structure of the SM Model Directory
----------------------------------------

Example of the SM model directory ``trachea_model_directory`` that contains the SM model file ``trachea_model``:

.. code-block:: console

    trachea_model_directory
     ㄴ trachea_cnn
     ㄴ trachea_final
     ㄴ trachea_finetune_cnn
     ㄴ trachea_input
     ㄴ trachea_model
     ㄴ trachea_smooth

* To execute the CNN nodes with pre-trained weights:
    * If the weights already exist and you want to use those weights, 
      please add the weights (``hd5``) file to the model directory 
      following the node-based naming convention described below.
    * Below is the example of the SM model directory ``trachea_model_directory``, if you want 
      to use the pre-trained weight for ``trachea_cnn`` node. Note the ``trachea_cnn_weights_NA.hd5``.
      
      .. code-block:: console
        :emphasize-lines: 3
        
        trachea_model_directory
        ㄴ trachea_cnn
        ㄴ trachea_cnn_weights_NA.hd5
        ㄴ trachea_final
        ㄴ trachea_finetune_cnn
        ㄴ trachea_input
        ㄴ trachea_model
        ㄴ trachea_smooth


SM Nodes
-----------------------------------

You can use the SM Model Attributes to write each SM node listed in the SM model file. 
You must write each SM node in a separate file with
the file name the same as the node name. 

.. seealso:: Please see the `SM Model Attributes`_ for other possible SM grammar.

Here are some example nodes listed in ``trachea_model``.

* Example of ``trachea_input``:
  
  .. code-block:: console
      
      AnatPathEntity: trachea_input;
      IncludeAllVoxels;
      Crop_TlPropDiff_mm 750 {0, 3, 150, 750} 750 {0, 3, 150, 750} -1 0.5 0.0 0.0;
      End: trachea_input;

      Comments section


* Example of the cnn node ``trachea_cnn``:

  .. code-block:: console
      
      AnatPathEntity: trachea_cnn;
      NeuralNetKeras unet_5block 512 512;
      NeuralNet_Parameter model_info reader_class string cxr_image_reader;
      NeuralNet_Normalization 0 1 0 clahe 0.03 {4, 6, 0.01, 0.08} 256 {7, 9, 64, 512};
      NeuralNet_Normalization 0 1 0 mean0_std1;
      NeuralNet_LearningRate 1.0 -2 {10, 13, -15, 0};
      NeuralNet_Parameter model_info optimizer string adam;
      NeuralNet_Parameter model_info decay float 0.0001;
      NeuralNet_Parameter model_info loss string negative_dice;
      NeuralNet_Parameter model_info metrics string dice;
      NeuralNet_Parameter model_info metrics string precision;
      NeuralNet_Parameter model_info metrics string recall;
      NeuralNet_Parameter training_info epochs int 50;
      NeuralNet_Parameter training_info batch_size int 64;
      NeuralNet_Parameter training_info sequential boolean false;
      NeuralNet_Parameter training_info replace boolean false;
      NeuralNet_Parameter training_info augment boolean false;
      NeuralNet_Parameter validation_info batch_size int 8;
      NeuralNet_Parameter callbacks_info monitor string val_loss;
      NeuralNet_Parameter callbacks_info callback_mode string min;
      NeuralNet_Parameter tensorboard_info downsampling_scale float 1.;
      PartOf_E trachea_input;
      End: trachea_cnn;

      Comments section

  .. hint:: See `Real Examples`_ Section for SM model example for various problems.


  .. todo:: Documantation needs explanations about default configuration for CNN models.
    
  .. todo:: Documantation needs explanations about the chromosome for GA.

.. _SM Model Attributes: ./attributes.rst
.. _Real Examples: ./examples.rst
