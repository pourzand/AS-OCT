"""CNN Architectures
Parsing given CNN architecture desciptor

Method
------
get_cnn_arch(cnn_arch, weight_file=None, 
             img_shapes=(256, 256, 22), img_channels=1)
    parsing given CNN architecture desciptor, 
    and return the proper architecture.

Examples
--------

"""
import cnn_arch_2d_resnet as rsnt
from cnn_arch_2d_models import *
from cnn_arch_3d_models import *
	
def get_cnn_arch(cnn_arch, weight_file=None, img_shapes=(256, 256), img_channels=1):
    """CNN architecture desciptor
    
    Attributes
    ----------
    cnn_arch : str
        CNN architecture descriptor
        Available 2D models:
            'unet_5block', 
            'vgg_reg',
            'resnet_18', 'resnet_34', 'resnet_50', 'resnet_101', 'resnet_152'
        Available 3D models: 
            'unet_3d_arb', 'unet_3d_small_arb'
            'unet_3d_2d_small_arb'
    weight_file: str
        default=None
    img_shapes: tuple of int
        default=(256, 256) # assume 2D by default
        For 3D input, use a tuple with . For example, you can use img_shapes=(256, 256, None)
    img_channels: int
        default=1
    
    Usage
    -----
    You can add more architecture by adding new model output here.

    Examples
    --------
    ```
    import cnn_arch
    model2D = cnn_arch.get_cnn_arch('unet_5block', weight_file=None,
                                    img_shape=(320, 320), img_channels=2)
    model3D = cnn_arch.get_cnn_arch('unet_3d_arb', weight_file=None,
                                    img_shape=(320, 320), img_channels=2)
    ```
    """
    
    model = None
    if len(img_shapes) == 2:
        # 2D architecture. please find better way...
        img_rows, img_cols = img_shapes
        if cnn_arch=='unet_5block':
            model = get_unet(img_rows, img_cols, img_channels)
        elif cnn_arch=='vgg_reg':
            model = get_vggregnet(None, img_rows, img_cols)
        elif 'resnet' in cnn_arch:
            arch_split = cnn_arch.split('_', 2)
            if arch_split[1]=='18':
                model = rsnt.ResNet18(input_shape=(img_rows,img_cols,img_channels), classes=2, dropout=None)
            elif arch_split[1]=='34':
                model = rsnt.ResNet34(input_shape=(img_rows,img_cols,img_channels), classes=2, dropout=None)
            elif arch_split[1]=='50':
                model = rsnt.ResNet50(input_shape=(img_rows,img_cols,img_channels), classes=2)
            elif arch_split[1]=='101':
                model = rsnt.ResNet101(input_shape=(img_rows,img_cols,img_channels), classes=2)
            elif arch_split[1]=='152':
                model = rsnt.ResNet152(input_shape=(img_rows,img_cols,img_channels), classes=2)
        else:
            raise ValueError(f'No architecture name {cnn_arch} for 2D input. \
                               \nYou can update the cnn_arch.py script \
                               \nto use a new architecture.')
    elif len(img_shapes) == 3:
        # img_rows, img_cols, img_slices = img_shapes
        img_slices, img_rows, img_cols = img_shapes
        if cnn_arch=='unet_3d_arb':
            model = get_3d_arb_unet(img_rows, img_cols, img_slices, img_channels)
        elif cnn_arch=='unet_3d_small_arb':
            model = get_3d_arb_unet(img_rows, img_cols, img_slices, img_channels)
        elif cnn_arch=='unet_3d_2d_small_arb':
            model = get_3d_arb_unet(img_rows, img_cols, img_slices, img_channels)
        else:
            raise ValueError(f'No architecture name {cnn_arch} for 3D input. \
                               \nYou can update the cnn_arch.py script \
                               \nto use a new architecture.')

    if model and weight_file:
        model.load_weights(weight_file)
		
    return model

