
************************************************
Neural Network Embedding
************************************************
This section describes how convolutional neural networks (NNs) are embedded within the semantic network (SN). 
An attribute (``SegParam``) is needed to provide parameters to be used by a neural network to perform segmentation, 
i.e., generate candidates. Its parameters are used for both CNN training and testing - 
the input images for both must of course be prepared the same way.

Neural Network Segmentation Agent
============================================
A segmentation agent (agent of type ``SegmentationKS``) is activated when this ``SegParam`` is present and if necessary calls a training script (if the neural network model weights do not already exist). 
The training and prediction scripts use a common library of normalization methods (an imported python script).
The ``“NeuralNetKeras”`` ``SegmentationKS`` uses a C++ system call to run the necessary python scripts.

When activated, this agent (through function *NeuralNetKerasA*) checks if there is a CNN weight file or not; if not it then calls a training python script (see CNN Model Training section below).

The training and prediction python scripts accept the intensity normalization from the *SegParam* as an argument and then call the appropriate normalization function when creating the CNN input image.

*cnn_arch.py* has all of the architectures and a *get_cnn_arch function* that accepts the name of the architecture requested (then training and predict code can import this same script)

* Each architecture is named with format of descriptor_subdescriptor(s), e.g., unet_5block
* The number of channels input to the architecture is implied by the number of normalization descriptor arguments


To embed a new CNN architecture
============================================
Update *cnn_arch.py*:

#. Add the function that returns the model (e.g., get_vggregnet)

   * No need to load the weights as this is handled in the *get_cnn_arch* function
   
#. Add a conditional in get_cnn_arch to call the model loading function

   * if cnn_arch==’vgg_reg’: model = get_vggregnet(img_rows, img_cols, img_channels)
   * The architecture descriptor (’vgg_reg’) is the one to be used in the neural network attribute (see below for further information on attributes)

If the weights are not being learned via cnn_train.py (they already exist) then don’t forget to add the weights (hd5) file to the model directory following the node-based naming convention described below.

.. seealso:: For the further information about the SM attribute (``SegParam``), see `SM Model Attributes`_ section.


NN Model Training
============================================
The User Guide section on Training CNN Nodes describes:

* Control of GPU/CPU resources via a configuration file.
* The working directory (EDM directory on the SM Blackboard).
* The training set defined within the working directory.

During training the CNN working subdirectory named *NodeName_KerasModel* is created and is accessed by the agent.

We need to store and process different hdf5 input files and different hd5 weight files for different chromosomes

* This is done by appending relevant bits of the chromosome to the hdf5 and hd5 file names, i.e., appending a “bit tag” that is comprised of the number of relevant bits and those chromosome bit values in hex format
* Determining which bits are relevant is described below and is important since we do not want to retrain unless bits changed that may affect the result

In SM Attribute.h a private member was added: std::vector<bool> _chromosome_bits_used;

* Indicates which chromosome bit numbers of the were used in setting this attributes values
* The vector is NOT guaranteed to be the length of the chromosome, any bits not included in the vector are assumed to be False, 
* i.e., if the vector is empty we assume no chromosome bits were used
* In SM ModelKS.cc a Attribute.set_chromosome_bit_used is called where applicable while the model is being read in
* If any upstream bits changed that impact the search area or if the normalization bits change then the hdf5 files need to updated

  * NeuralNetKeras has members like the base class for normalization bits

In SM SegParam.cc, method *NeuralNetKerasA* computes the bit_tag strings on demand (could be different for hdf5 and hd5)

* In the node with the CNN only bits relating to the search area and in the Neural Net attribute itself should be included in the weight file name
* Added code to identify nodes involved in SearchArea attributes, and determine all ancestor nodes (upstream from them) using Blackboard::add_ancestors method

  * Any bit used in an ancestor node should be included 
  
* The bit tag format is “X_Y” where X is the number of chromosome bits involved, and Y is a hex representation of those bits.

In SM SegmentationKS.cc (NeuralNetKerasA method), the hd5 file paths in the model directory and the working directory are computed using the bit string

* If the hd5 file is not found in the model directory it is search for in the working directory (weights subfolder)

If not found in either place the NeuralNetKerasA method calls cnn_train.py, before calling cnn_predict.py

* The bit tag is also passed to cnn_train.py

Training occurs via cnn_train.py

* Computes the hdf5 and hd5 paths
* If not provided the default tag is “NA”
* Check whether the hdf5 file already exists and generate if not

During training SM_train and SM_valid subfolders are created (with further subfolders based on the chromosome)

* These folders contain SM results as part of determining the input ROI for the CNN training.
* It includes prepared (cropped and normalized) input images/reference in a png format that allows review

The hdf5 input training and validation files (prepared images+ref) are written to an input subfolder and tagged based on the chromosome.

* They may not be re-computed with every training run, it depends on whether the chromosome changes affected relevant nodes.

The trained CNN model weights (hd5 file) are written to a weights subfolder and tagged based on the chromosome.

During evolution we may try many options in the working directory, but once a final chromosome is selected we could copy the associated weights file to the model folder so that it can be committed.

Training python script - \\\\dingo\\scratch\\mbrown\\script\\cnn_train.py

* Calls miu_nod.exe multiple times

  * SM is called with the STOP_AT argument set to the name of the node with the CNN being trained
  * The stop-at node name is stored on the Blackboard
  * When computing priority scores for each group, NextGroupA (SchedulerKS) sets all group priorities to -1.0 if the stop-at solution element is part of the current group (just processed). This effectively stops all processing.
  
    * We stop at the group rather than the NextSolEl level since we want to make sure it fires enough times to let it be processed fully within the group.

* When calling SM during training, the Working Directory argument is not set and under these conditions NeuralNetKerasA just generates the search area as the matched primitive (rather than trying to learn the CNN model, which does not exist) - this prevents recursion ad infinitum.
* Saves a cropped, normalized output ROI as a png image in the SM output folder with file name: node_name+'_resized_input_image.png'
* To view training images generated by SM, for example, for a carina_cnn node, in Windows folder \\\\dingo\\scratch\\mbrown\\CXR\\working\\carina_cnn_KerasModel\\miu search for carina_cnn_resized_input_image.png and display Extra Large Icons
* The hdf5 training file is stored in the “training_path” argument folder and within that folder the training.hdf5 and valid.hdf5 stored
* The split between training validation is a parameter internal to cnn_train.py and is currently 0.8 training, 0.2 validation

To generate the training hd5s using SM for et_tube_2_cnn, for example, we run it with a dummy input image (since the weights file does not yet exist it will launch cnn_train.py)

* Outputs to \\\\dingo\\scratch\\mbrown\\CXR\\working\\et_tube_2_cnn_KerasModel

.. code-block:: console

   M:\apps\personal\mdaly\conda\condabin\activate.bat
   python \\dingo\scratch\mbrown\SENN\miu.py \\dingo\scratch\mbrown\temp\1.2.392.200036.9125.3.32166252204190.64913841641.239621_0.seri \\dingo\scratch\mbrown\temp\roi \\dingo\scratch\mbrown\temp\seg --model \\thorimage9\apps\all\seg_model\cxr_model.7\cxr_model --working \\dingo\scratch\mbrown\CXR\working --rerun

To view CNN input images, search in windows explorer (et_tube_2_cnn*.png) with extra large icons

To view an individual case output from SM: 


.. code-block:: console
    
   C:\Python34\python.exe M:\DEVELOPMENT\MEDVIEW\dev3.4\test\miu_viewer.py \\dingo\scratch\mbrown\CXR\working\et_tip_cnn_KerasModel\miu\000000100

Currently training of the CNN using the hdf5 input files need to be initiated manually by running \\\\dingo\\scratch\\mbrown\\SENN\\script\\train.py

* Currently cnn_train.py just generates the hdf5 file, it does not actually do the training
* After running train.py, the resulting weight file (.hd5) copied to the model folder with name format node_name+'_weights.hd5'

To train et_tube_2_cnn, for example, on supernova:

* Check which GPU core is free and set it in train_config.py
* ssh mbrown@REDLRADADM23589.ad.medctr.ucla.edu 

.. code-block:: console
    
    screen -S cnn (argument is name of job, so that my job keeps going even if I end the session)
    docker run --mount type=bind,source=/apps,target=/apps --mount type=bind,source=/cvib2,target=/cvib2 --mount type=bind,source=/scratch,target=/scratch --mount type=bind,source=/scratch2,target=/scratch2 --mount type=bind,source=/dingo_data,target=/dingo_data -it registry.rip.ucla.edu/deep_med /bin/bash
    python /scratch/mbrown/SENN/script/train.py
    Ctrl+a d (to detach a session and keep it running)
    screen -x cnn (to return to the session)


NN Model Prediction
============================================

The SegmentationKS, *NeuralNetKerasA*, runs the CNN prediction via a system call to a python script, cnn_predict.py.

Necessary arguments are passed from *NeuralNetKerasA* to cnn_predict.py

* CNN architecture descriptor

  * The *get_cnn_arch function* in cnn_arch.py is used to create the CNN architecture based on the descriptor
  * Whenever a new model architecture is being introduced a descriptor must be defined and the necessary Keras model function added to cnn_arch.py

* Weight file path

  * *NeuralNetKerasA* forms the weight file path based on the assumption that the weights are in the model folder with name format node_name+\'_weights.hd5\'

* Necessary arguments to form the transformed (cropped, normalized, etc) input image

  * Top left (TL) and bottom right (BR) cropping box coordinates
  * Normalization descriptor

    * For multichannel image inputs our current thinking is that the multiple channels will be different normalizations with same dimensions/crop
    * Cnn_predict.py creates an array of images by iterating through each normalization parameter
    * We pass in a list of normalizations from NeuralNetKerasA using the -i arg flag when making the system call to cnn_predict.py

* cnn_predict.py now does image preparation more like cnn_train.py

  * Using resize instead of subsampled may have some effect on trachea_cnn performance, but once we retrain using hdf5 generated by SM it should be fine

The CNN will output a mask image or point which needs to go through a reverse transformation to undo the crop/scale when converting it to ROI format

* img = img.get_alias(min_point=(0, 0, 0))

  * It seems that qimage starts indexing the first image from 1 after reading the DICOM (while ROIs assume the first image is z=0) => this moves the first index to z=0
  * Also need to make sure that this works for cases where the image instance numbers may not be continuous - it might be fine depending on how the transformation at the end of SM works


* The python script that runs the model will be doing the crop/scale to form the input image on the fly, so after getting the mask result it should be able to do the inverse transformation 
* There is also a post-processing step that turns everything back into ROI format and outputs a file named pred.roi into a path that is provided as arg to the script
  * *NeuralNetKerasA* passes this arg as bb.temp_file_path(), so the roi file path is bb.temp_file_path()+\'pred.roi\'

*NeuralNetKerasA* takes the CNN output ROI and forms a single candidate. 

* The candidate will then be subject to node features to determine if it becomes the matched prim. Connected component analysis can be applied at the next dependent node if necessary.

Speeding Up CNN Model Prediction

* cnn_predict.py has a function def prerun_cnns that works only for cxr_model.7 for the purposes of speeding up computation

  * It precomputes all CNN ROIs (that do not depend on other nodes) in one script so that we don’t have the overhead of initializing the Keras environment and reading and pre-processing the image multiple times
  * If called it writes files of format pred_se_name.roi to the SM input roi directory

* NeuralNet seg KS checks to see if this roi file exists in the input roi directory, and if so it just reads it when doing the prediction


