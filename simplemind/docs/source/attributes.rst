.. highlight:: shell

######################################
SM Model Attributes
######################################

An attribute (``SegParam``) is needed to provide parameters to be used by a neural network 
to perform segmentation, i.e., generate candidates. 
Its parameters are used for both CNN training and testing.

A bounding box is formed around the search area to define the input ROI, it is rescaled to the dimensions given as parameters (see below).

************************************************
Neural Network Node Attribute
************************************************

``“NeuralNetKeras”`` ``SegParam`` with arguments:
====================================================

* CNN Architecture - currently supported arguments are:
    * ``unet_5block``
        * multi channel supported
    * ``vgg_reg``
        * single channel only
    * ``resnet_{XX}``
        * ``{XX}`` = 18, 34, 50, 101, 152
        * 34 with single channel has been used so far (ResNet with 34 layers and v2 residual units)
* Input image dimensions
    * Support 2D image input - e.g., ``256, 256``
    * Support 3D image input - e.g., ``256, 256, 20```


``“NeuralNet_LearningRate”`` ``SegParam`` with arguments value and exponent:
================================================================================

* Both can be adapted by the GA, but we expect that typically just the exponent will be specified / optimized in the chromosome since learning rate should be explored logarithmically rather than linearly
* For example: ``value (float) = 1.0; exponent (float) = -2 {X, X+2, -7, 0}`` so that possible exponents range from 10^-7 and 10^0
* The NeuralNetKeras attribute, that specifies the CNN architecture must be provided before this one.

``“NeuralNet_Normalization”`` ``SegParam``:
================================================================================

* *channel_number**: an integer starting at 0, if multiple attributes have the same channel number the processing operations will be applied to the image in sequence during normalization.
* *num_processing_options*: must be equal to the number of methods specified in the ordered list 
* *process_number*: usually an integer [0, num_processing_options) and determines which processing will be applied (only one from the list will be executed)
    * If *process_number >= num_processing_options* then the last process option will be used
    * So when using the GA, if you want to favor a processing option, put it last or add it multiple times to the list
* ordered list of processing methods and hyper parameters (first process is number 0, then number 1, etc]. Processing method options are: 
    * ``mean0_std1``
    * ``minmax``
    * ``histo_eq``
    * ``clahe XX YY``
        * where ``XX`` is a float for the clip limit
        * where ``YY`` is an int for the number of gray level bins
        * to match the skimage.exposure.equalize_adapthist defaults use clahe_0.01_256
        * uses default kernel_size of 1/8 of image height by 1/8 of its width
    * ``denoise``
        * The denoising is applied with the following parameters: anisotropic_diffusion(img_arr, niter=10, kappa=50, gamma=0.1, voxelspacing=None, option=1)
    * ``none``
        * means no processing applied
    * ``no_channel``
        * means that no channel will be created for this channel number
    * ``normclahe XX YY`` (deprecated)
        * Instead use ``clahe XX YY`` followed by ``mean0_std1``
    * ``denoiseclahe_XX_YY``  (deprecated)
        * Instead use ``clahe_XX_YY`` followed by ``denoise``
* The ``NeuralNetKeras`` attribute, that specifies the CNN architecture must be provided before this one.
* For future extension of these options note that the three main types of pixel scaling techniques supported by the Keras ImageDataGenerator class are as follows:
    * Pixel Normalization: scale pixel values to the range 0-1.
    * Pixel Centering: scale pixel values to have a zero mean.
    * Pixel Standardization: scale pixel values to have a zero mean and unit variance.

.. In future we may also want to explicitly state the type of network output (currently it is determined by the network architecture)

.. * This would determine what is expected to be provided as reference for inclusion in the hdf5 training file: Mask or Point
.. * Point - (x,y,z) coordinates are to be learned
..     * can just ignore the z in a 2D image (e.g., used for carina detection in chest x-ray and a csv file with image path, x, y, z is expected)
..     * In cnn_predict.py this is converted to an image then ROI so that it can be handled appropriately in SM


.. todo:: attribute documentation from ``ModelKS.cc`` in the documentation.
