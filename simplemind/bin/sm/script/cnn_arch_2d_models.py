"""Two-dimensional CNN architectures

Method
------
get_vgg_model
    VGG models
get_unet
    5 block 2D U-Net models
get_vggregnet
    VGG-ResNet models
"""
from keras.models import Model
from keras.layers import Input
from keras.layers import Dense
# from keras.layers import merge, Activation, Dropout
from keras.layers import Conv2D, MaxPooling2D, Flatten
from keras.layers.convolutional import UpSampling2D,Conv2DTranspose
from keras.layers.normalization import BatchNormalization
# from keras.layers.advanced_activations import PReLU
from keras.layers.merge import concatenate
from keras.utils import plot_model

def get_vgg_model(weight_file=None, rows=256, cols=256):
    dim_order = 'channels_first'
    if dim_order=='channels_last':
        inputs = Input(shape=(rows,cols,1))
        axis_concat = 3
    elif dim_order=='channels_first':
        inputs = Input(shape=( 1, rows, cols))
        axis_concat = 1
    dropout_p = 0.5
    # block1
    conv1 = Conv2D(64, (3, 3), padding="same", activation='relu', data_format=dim_order)(inputs)
    conv1 = Conv2D(64, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv1)
    pool1 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(conv1)

    # block2
    conv2 = Conv2D(128, (3, 3), padding="same", activation='relu', data_format=dim_order)(pool1)
    conv2 = Conv2D(128, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv2)
    pool2 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(conv2)

    # block3
    conv3 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(pool2)
    conv3 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv3)
    conv3 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv3)
    pool3 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(conv3)

    # block4
    conv4 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(pool3)
    conv4 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv4)
    conv4 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv4)
    pool4 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(conv4)

    # block5
    conv5 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(pool4)
    conv5 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv5)
    conv5 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv5)

    # deconv1
    up6 = concatenate([UpSampling2D(size=(2, 2),data_format=dim_order)(conv5), conv4], axis=axis_concat)
    conv6 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(up6)
    conv6 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv6)
    conv6 = Conv2D(512, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv6)
    # deconv2
    up7 = concatenate([UpSampling2D(size=(2, 2),data_format=dim_order)(conv6), conv3], axis=axis_concat)
    conv7 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(up7)
    conv7 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv7)
    conv7 = Conv2D(256, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv7)
    # deconv3
    up8 = concatenate([UpSampling2D(size=(2, 2),data_format=dim_order)(conv7), conv2], axis=axis_concat)
    conv8 = Conv2D(128, (3, 3), padding="same", activation='relu', data_format=dim_order)(up8)
    conv8 = Conv2D(128, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv8)
    # deconv4
    up9 = concatenate([UpSampling2D(size=(2, 2),data_format=dim_order)(conv8), conv1], axis=axis_concat)
    conv9 = Conv2D(64, (3, 3), padding="same", activation='relu', data_format=dim_order)(up9)
    conv9 = Conv2D(64, (3, 3), padding="same", activation='relu', data_format=dim_order)(conv9)
    # fully convolutional layer
    conv10 = Conv2D(1, (3, 3) ,activation='sigmoid', padding="same",data_format=dim_order)(conv9)
    
    model = Model(inputs=inputs, outputs=conv10)
    plot_model(model,to_file='keras2_model.png',show_shapes=True)
    if weight_file is not None:
        model.load_weights(weight_file)
    
    return model
    
    
def get_unet(img_rows=256, img_cols=256, img_channels=1):

    inputs = Input((img_rows, img_cols, img_channels))
    conv1 = Conv2D(32, (3, 3), activation='relu', padding='same')(inputs)
    conv1 = Conv2D(32, (3, 3), activation='relu', padding='same')(conv1)
    pool1 = MaxPooling2D(pool_size=(2, 2))(conv1)

    conv2 = Conv2D(64, (3, 3), activation='relu', padding='same')(pool1)
    conv2 = Conv2D(64, (3, 3), activation='relu', padding='same')(conv2)
    pool2 = MaxPooling2D(pool_size=(2, 2))(conv2)

    conv3 = Conv2D(128, (3, 3), activation='relu', padding='same')(pool2)
    conv3 = Conv2D(128, (3, 3), activation='relu', padding='same')(conv3)
    pool3 = MaxPooling2D(pool_size=(2, 2))(conv3)

    conv4 = Conv2D(256, (3, 3), activation='relu', padding='same')(pool3)
    conv4 = Conv2D(256, (3, 3), activation='relu', padding='same')(conv4)
    pool4 = MaxPooling2D(pool_size=(2, 2))(conv4)

    conv5 = Conv2D(512, (3, 3), activation='relu', padding='same')(pool4)
    conv5 = Conv2D(512, (3, 3), activation='relu', padding='same')(conv5)

    up6 = concatenate([Conv2DTranspose(256, (2, 2), strides=(2, 2), padding='same')(conv5), conv4], axis=3)
    conv6 = Conv2D(256, (3, 3), activation='relu', padding='same')(up6)
    conv6 = Conv2D(256, (3, 3), activation='relu', padding='same')(conv6)

    up7 = concatenate([Conv2DTranspose(128, (2, 2), strides=(2, 2), padding='same')(conv6), conv3], axis=3)
    conv7 = Conv2D(128, (3, 3), activation='relu', padding='same')(up7)
    conv7 = Conv2D(128, (3, 3), activation='relu', padding='same')(conv7)

    up8 = concatenate([Conv2DTranspose(64, (2, 2), strides=(2, 2), padding='same')(conv7), conv2], axis=3)
    conv8 = Conv2D(64, (3, 3), activation='relu', padding='same')(up8)
    conv8 = Conv2D(64, (3, 3), activation='relu', padding='same')(conv8)

    up9 = concatenate([Conv2DTranspose(32, (2, 2), strides=(2, 2), padding='same')(conv8), conv1], axis=3)
    conv9 = Conv2D(32, (3, 3), activation='relu', padding='same')(up9)
    conv9 = Conv2D(32, (3, 3), activation='relu', padding='same')(conv9)

    conv10 = Conv2D(1, (1, 1), activation='sigmoid')(conv9)
    model = Model(inputs=[inputs], outputs=[conv10])

    return model

def get_vggregnet(rows=256, cols=256):
    dim_order = 'channels_last'
    if dim_order=='channels_last':
        inputs = Input(shape=(rows,cols,1))
        #inputs = Input((rows,cols,1))
        axis_concat = 3
    elif dim_order=='channels_first':
        inputs = Input(shape=( 1, rows, cols))
        axis_concat = 1

    kernel_initializer = 'glorot_uniform'
    # block1
    conv1 = Conv2D(32, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(inputs)
    norm1 = BatchNormalization()(conv1)

    
    conv1 = Conv2D(32, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm1)
    norm1 = BatchNormalization()(conv1)
    pool1 = MaxPooling2D(pool_size=(3, 3), data_format=dim_order)(norm1)
    
    # block2
    conv2 = Conv2D(64, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(pool1)
    norm2 = BatchNormalization()(conv2)
    
    conv2 = Conv2D(64, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm2)
    norm2 = BatchNormalization()(conv2)
    pool2 = MaxPooling2D(pool_size=(3, 3), data_format=dim_order)(norm2)
    
    # block3
    conv3 = Conv2D(128, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(pool2)
    norm3 = BatchNormalization()(conv3)
    
    conv3 = Conv2D(128, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm3)
    norm3 = BatchNormalization()(conv3)
    
    conv3 = Conv2D(128, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm3)
    norm3 = BatchNormalization()(conv3)
    pool3 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(norm3)
    
    # block4
    conv4 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(pool3)
    norm4 = BatchNormalization()(conv4)
    
    conv4 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm4)
    norm4 = BatchNormalization()(conv4)
    
    conv4 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm4)
    norm4 = BatchNormalization()(conv4)
    pool4 = MaxPooling2D(pool_size=(2, 2), data_format=dim_order)(norm4)
    
    # block5
    conv5 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(pool4)
    norm5 = BatchNormalization()(conv5)
    
    conv5 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm5)
    norm5 = BatchNormalization()(conv5)
    
    conv5 = Conv2D(256, (3, 3), padding="same", activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order)(norm5)
    norm5 = BatchNormalization()(conv5)
    
    conv5 = Conv2D(128, (1,1), activation='relu', kernel_initializer=kernel_initializer, data_format=dim_order, name='block5_conv4')(norm5)
    norm5 = BatchNormalization()(conv5)
    
    # fully convolutional output layer
    fc = Flatten(name='flatten')(norm5)
    #fc = BatchNormalization()(fc)
    fc = Dense(128, activation='relu', kernel_initializer=kernel_initializer, name='fc1')(fc)
    fc = BatchNormalization()(fc)
    fc = Dense(128, activation='relu', kernel_initializer=kernel_initializer, name='fc2')(fc)
    fc = BatchNormalization()(fc)
    fc = Dense(2, kernel_initializer='glorot_uniform', name='predictions')(fc)
        
    model = Model(inputs=inputs, outputs=fc)
    
    return model