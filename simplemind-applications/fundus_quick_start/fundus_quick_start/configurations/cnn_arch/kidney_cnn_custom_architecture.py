"""Three-dimensional CNN architectures for kidney

Method
------
get_cnn_arch
    CNN architecture descriptor
    for loading a custom architecture


Custom Architectures
--------------------
get_3d_kidney_model_v0
    3D U-Net model v0 for kidney
```
pip install git+https://www.github.com/keras-team/keras-contrib.git
"""
from keras.models import Model
from keras.layers import Input
from keras.layers import Activation, BatchNormalization
from keras.layers.convolutional import Conv3DTranspose
from keras.layers import Dropout
from keras.layers import Conv3D
from keras.layers.convolutional import MaxPooling3D
from keras.layers.merge import concatenate
# from keras_contrib.layers.normalization.instancenormalization import InstanceNormalization
from keras.layers import Layer, InputSpec
from keras import initializers, regularizers, constraints
from keras import backend as K

class InstanceNormalization(Layer): #Stolen from https://github.com/keras-team/keras-contrib/blob/master/keras_contrib/layers/normalization/instancenormalization.py
    """Instance normalization layer.
    Normalize the activations of the previous layer at each step,
    i.e. applies a transformation that maintains the mean activation
    close to 0 and the activation standard deviation close to 1.
    # Arguments
        axis: Integer, the axis that should be normalized
            (typically the features axis).
            For instance, after a `Conv2D` layer with
            `data_format="channels_first"`,
            set `axis=1` in `InstanceNormalization`.
            Setting `axis=None` will normalize all values in each
            instance of the batch.
            Axis 0 is the batch dimension. `axis` cannot be set to 0 to avoid errors.
        epsilon: Small float added to variance to avoid dividing by zero.
        center: If True, add offset of `beta` to normalized tensor.
            If False, `beta` is ignored.
        scale: If True, multiply by `gamma`.
            If False, `gamma` is not used.
            When the next layer is linear (also e.g. `nn.relu`),
            this can be disabled since the scaling
            will be done by the next layer.
        beta_initializer: Initializer for the beta weight.
        gamma_initializer: Initializer for the gamma weight.
        beta_regularizer: Optional regularizer for the beta weight.
        gamma_regularizer: Optional regularizer for the gamma weight.
        beta_constraint: Optional constraint for the beta weight.
        gamma_constraint: Optional constraint for the gamma weight.
    # Input shape
        Arbitrary. Use the keyword argument `input_shape`
        (tuple of integers, does not include the samples axis)
        when using this layer as the first layer in a Sequential model.
    # Output shape
        Same shape as input.
    # References
        - [Layer Normalization](https://arxiv.org/abs/1607.06450)
        - [Instance Normalization: The Missing Ingredient for Fast Stylization](
        https://arxiv.org/abs/1607.08022)
    """
    def __init__(self,
                 axis=None,
                 epsilon=1e-3,
                 center=True,
                 scale=True,
                 beta_initializer='zeros',
                 gamma_initializer='ones',
                 beta_regularizer=None,
                 gamma_regularizer=None,
                 beta_constraint=None,
                 gamma_constraint=None,
                 **kwargs):
        super(InstanceNormalization, self).__init__(**kwargs)
        self.supports_masking = True
        self.axis = axis
        self.epsilon = epsilon
        self.center = center
        self.scale = scale
        self.beta_initializer = initializers.get(beta_initializer)
        self.gamma_initializer = initializers.get(gamma_initializer)
        self.beta_regularizer = regularizers.get(beta_regularizer)
        self.gamma_regularizer = regularizers.get(gamma_regularizer)
        self.beta_constraint = constraints.get(beta_constraint)
        self.gamma_constraint = constraints.get(gamma_constraint)

    def build(self, input_shape):
        ndim = len(input_shape)
        if self.axis == 0:
            raise ValueError('Axis cannot be zero')

        if (self.axis is not None) and (ndim == 2):
            raise ValueError('Cannot specify axis for rank 1 tensor')

        self.input_spec = InputSpec(ndim=ndim)

        if self.axis is None:
            shape = (1,)
        else:
            shape = (input_shape[self.axis],)

        if self.scale:
            self.gamma = self.add_weight(shape=shape,
                                         name='gamma',
                                         initializer=self.gamma_initializer,
                                         regularizer=self.gamma_regularizer,
                                         constraint=self.gamma_constraint)
        else:
            self.gamma = None
        if self.center:
            self.beta = self.add_weight(shape=shape,
                                        name='beta',
                                        initializer=self.beta_initializer,
                                        regularizer=self.beta_regularizer,
                                        constraint=self.beta_constraint)
        else:
            self.beta = None
        self.built = True

    def call(self, inputs, training=None):
        input_shape = K.int_shape(inputs)
        reduction_axes = list(range(0, len(input_shape)))

        if self.axis is not None:
            del reduction_axes[self.axis]

        del reduction_axes[0]

        mean = K.mean(inputs, reduction_axes, keepdims=True)
        stddev = K.std(inputs, reduction_axes, keepdims=True) + self.epsilon
        normed = (inputs - mean) / stddev

        broadcast_shape = [1] * len(input_shape)
        if self.axis is not None:
            broadcast_shape[self.axis] = input_shape[self.axis]

        if self.scale:
            broadcast_gamma = K.reshape(self.gamma, broadcast_shape)
            normed = normed * broadcast_gamma
        if self.center:
            broadcast_beta = K.reshape(self.beta, broadcast_shape)
            normed = normed + broadcast_beta
        return normed

    def get_config(self):
        config = {
            'axis': self.axis,
            'epsilon': self.epsilon,
            'center': self.center,
            'scale': self.scale,
            'beta_initializer': initializers.serialize(self.beta_initializer),
            'gamma_initializer': initializers.serialize(self.gamma_initializer),
            'beta_regularizer': regularizers.serialize(self.beta_regularizer),
            'gamma_regularizer': regularizers.serialize(self.gamma_regularizer),
            'beta_constraint': constraints.serialize(self.beta_constraint),
            'gamma_constraint': constraints.serialize(self.gamma_constraint)
        }
        base_config = super(InstanceNormalization, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))
"""Format for using custom architecture"""
def get_cnn_arch(cnn_arch, weight_file=None, img_shapes=(512, 512), img_channels=1):
    """CNN architecture desciptor
    
    Attributes
    ----------
    cnn_arch : str
        CNN architecture descriptor
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
        raise ValueError(f'No architecture name {cnn_arch} for 2D input. \
                            \nYou can update the cnn_arch.py script \
                            \nto use a new architecture.')
    elif len(img_shapes) == 3:
        img_slices, img_rows, img_cols = img_shapes
        if cnn_arch=='kidney_3d_model_v0':
            model = get_kidney_3d_model_v0(img_rows, img_cols, img_slices, img_channels)
        else:
            raise ValueError(f'No architecture name {cnn_arch} for 3D input. \
                               \nYou can update the cnn_arch.py script \
                               \nto use a new architecture.')

    if model and weight_file:
        model.load_weights(weight_file)
		
    return model

def convblock(input, kernelsize, filters):
    fx = Conv3D(filters, kernelsize, kernel_initializer='he_normal', data_format = 'channels_last',padding='same')(input)
    fx = InstanceNormalization(axis = -1)(fx)
    out = Activation(activation = 'relu')(fx)
    
    return out

def get_kidney_3d_model_v0(img_rows=None, img_cols=None, img_slices=None, img_channels=1):
    inputs = Input((img_slices, img_rows, img_cols, img_channels))

    x =8;
    c1 = convblock(inputs,(3, 3, 3), x)
    c1 = convblock(c1,(3, 3, 3), x)
    c1 = Dropout(0.1) (c1)

    p2 = Conv3D(x*2, (2,2,2), strides = (2,2,2),  kernel_initializer='he_normal', data_format = 'channels_last', padding='valid') (c1)
    p2 = InstanceNormalization(axis = -1)(p2)
    p2 = Activation(activation = 'relu')(p2)
    c2 = convblock(p2, (3,3,3), x*2)
    c2 = Dropout(0.1) (c2)

    p3 = Conv3D(x*4, (2,2,2), strides = (2,2,2),  kernel_initializer='he_normal', data_format = 'channels_last', padding='valid') (c2)
    p3 = InstanceNormalization(axis = -1)(p3)
    p3 = Activation(activation = 'relu')(p3)
    c3 = convblock(p3, (3,3,3), x*4)
    c3 = Dropout(0.2) (c3)

    p4 = Conv3D(x*8, (2,2,2), strides = (2,2,2),  kernel_initializer='he_normal', data_format = 'channels_last', padding='valid') (c3)
    p4 = InstanceNormalization(axis = -1)(p4)
    p4 = Activation(activation = 'relu')(p4)
    c4 = convblock(p4, (3,3,3), x*8)
    c4 = Dropout(0.2) (c4)

    p5 = Conv3D(x*16, (2,2,2), strides = (2,2,2),  kernel_initializer='he_normal', data_format = 'channels_last', padding='valid') (c4)
    p5 = InstanceNormalization(axis = -1)(p5)
    p5 = Activation(activation = 'relu')(p5)
    c5 = convblock(p5, (3,3,3), 320)
    c5 = Dropout(0.2) (c5)

    #Bottleneck
    p6 = Conv3D(x*16, (2,2,2), strides = (2,2,2),  kernel_initializer='he_normal', data_format = 'channels_last', padding='valid') (c5)
    p6 = InstanceNormalization(axis = -1)(p6)
    p6 = Activation(activation = 'relu')(p6)
    c6 = convblock(p6, (3,3,3), x*16)
    c6 = Dropout(0.2) (c6)

    #Synthesis
    u7 = Conv3DTranspose(x*16, (2,2,2), strides=(2,2,2), padding='same') (c6)
    u7 = concatenate([u7, c5])
    c7 = convblock(u7, (3,3,3), x*16)
    c7 = convblock(c7, (3,3,3), x*16)
    c7 = Dropout(0.2) (c7)

    u8 = Conv3DTranspose(x*8, (2,2,2), strides=(2,2,2), padding='same') (c7)
    u8 = concatenate([u8, c4])
    c8 = convblock(u8, (3,3,3), x*8)
    c8 = convblock(c8, (3,3,3), x*8)
    c8 = Dropout(0.2) (c8)

    u9 = Conv3DTranspose(x*4, (2,2,2), strides=(2,2,2), padding='same') (c8)
    u9 = concatenate([u9, c3])
    c9 = convblock(u9, (3,3,3), x*4)
    c9 = convblock(c9, (3,3,3), x*4)
    c9 = Dropout(0.1) (c9)

    u10 = Conv3DTranspose(x*2, (2,2,2), strides=(2,2,2), padding='same') (c9)
    u10 = concatenate([u10, c2])
    c10 = convblock(u10, (3,3,3), x*2)
    c10 = convblock(c10, (3,3,3), x*2)
    c10 = Dropout(0.1) (c10)

    u11 = Conv3DTranspose(x, (2,2,2), strides=(2,2,2), padding='same') (c10)
    u11 = concatenate([u11, c1])
    c11 = convblock(u11, (3,3,3), x)
    c11 = convblock(c11, (3,3,3), x)
    c11 = Dropout(0.1) (c11)

    outputs = Conv3D(1, (1, 1, 1), activation='sigmoid') (c11)
    
    model = Model(inputs=[inputs], outputs=[outputs])
    print('----------------------------------------------------')
    print('Custom model:')
    print('----------------------------------------------------')
    model.summary()
    print('----------------------------------------------------')
    return model