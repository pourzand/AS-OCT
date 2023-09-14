"""Three-dimensional CNN architectures

Method
------
get_3d_arb_unet
    3D U-Net model with arbitrary size input.
"""
from keras.models import Model
from keras.layers import Input
from keras.layers import Activation
# from keras.layers import Dense, Dropout, merge, Flatten
from keras.layers.convolutional import Conv3DTranspose
from keras.layers import Conv3D
# from keras.layers.convolutional import UpSampling3D, MaxPooling3D
from keras.layers.normalization import BatchNormalization
from keras.layers.merge import Concatenate
# from tensorflow.keras.models import Model
# from tensorflow.keras.layers import Input
# from tensorflow.keras.layers import Activation
# # from tensorflow.keras.layers import Dense, Dropout, merge, Flatten
# from tensorflow.keras.layers import Conv3DTranspose
# from tensorflow.keras.layers import Conv3D
# # from tensorflow.keras.layers import UpSampling3D, MaxPooling3D
# from tensorflow.keras.layers import BatchNormalization
# from tensorflow.keras.layers import Concatenate
from keras.utils import plot_model

def get_3d_arb_unet(img_rows=None, img_cols=None, img_slices=None, img_channels=1):
    """
    TODO: 
    1. make the architecture works for odd number of z slices
    1. add architecture input shape in ref_config
    """
    # patch_size = (img_slices, img_rows, img_cols, img_channels)
    patch_size = (None, None, None, img_channels)
    
    """Encoder"""
    x_input = Input(shape=patch_size, name='input')
    e_1 = Conv3D(filters=8, # TODO: chech when the number of filters is 16
                 kernel_size=(3,5,5), strides=(1,2,2), 
                 padding='same', kernel_initializer='he_normal', name='e1_conv')(x_input)
    e_1 = BatchNormalization(name='e1_bn')(e_1)
    e_1 = Activation('relu', name='e1_activation')(e_1)
    # e_1 = Dropout(0.5, name='e1_dropout')(e_1)

    e_2 = Conv3D(filters=16, kernel_size=(1,3,3), 
                 padding='same', kernel_initializer='he_normal', name='e2_conv')(e_1)
    e_2 = BatchNormalization(name='e2_bn')(e_2)
    e_2 = Activation('relu', name='e2_activation')(e_2)
    # e_2 = Dropout(0.5, name='e2_dropout')(e_2)

    e_3 = Conv3D(filters=32, kernel_size=(1,3,3), strides=(1,2,2), 
                    padding='same', kernel_initializer='he_normal', name='e3_conv')(e_2)
    e_3 = BatchNormalization(name='e3_bn')(e_3)
    e_3 = Activation('relu', name='e3_activation')(e_3)
    # e_3 = Dropout(0.5, name='e3_dropout')(e_3)

    e_4 = Conv3D(filters=32, kernel_size=(1,3,3), 
                    padding='same', kernel_initializer='he_normal', name='e4_conv')(e_3)
    e_4 = BatchNormalization(name='e4_bn')(e_4)
    e_4 = Activation('relu', name='e4_activation')(e_4)
    # e_4 = Dropout(0.5, name='e4_dropout')(e_4)

    e_5 = Conv3D(filters=64, kernel_size=(1,3,3), strides=(2,2,2), 
                    padding='same', kernel_initializer='he_normal', name='e5_conv')(e_4)
    e_5 = BatchNormalization(name='e5_bn')(e_5)
    e_5 = Activation('relu', name='e5_activation')(e_5)
    # e_5 = Dropout(0.5, name='e5_dropout')(e_5)

    ################################################################################################################
    # center : encoder to decoder

    etd = Conv3D(filters=64, kernel_size=(2,5,5), 
                    padding='same', kernel_initializer='he_normal', name='center_conv')(e_5)
    etd = BatchNormalization(name='center_bn')(etd)
    etd = Activation('relu', name='center_activation')(etd)
    # etd = Dropout(0.5, name='center_dropout')(etd)

    """Decoder"""
    d_5 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), strides=(2,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d5_convT')(etd)
    d_5 = BatchNormalization(name='d5_bn')(d_5)
    d_5 = Activation('relu', name='d5_activation')(d_5)
    # d_5 = Dropout(0.5, name='d5_dropout')(d_5)

    d_4 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d4_convT')(d_5)
    d_4 = BatchNormalization(name='d4_bn')(d_4)
    d_4 = Activation('relu', name='d4_activation')(d_4)
    # d_4 = Dropout(0.5, name='d4_dropout')(d_4)
    d_4 = Concatenate(name='d4_concat')([d_4, e_3])

    d_3 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d3_convT')(d_4)
    d_3 = BatchNormalization(name='d3_bn')(d_3)
    d_3 = Activation('relu', name='d3_activation')(d_3)
    # d_3 = Dropout(0.5, name='d3_dropout')(d_3)

    d_2 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d2_convT')(d_3)
    d_2 = BatchNormalization(name='d2_bn')(d_2)
    d_2 = Activation('relu', name='d2_activation')(d_2)
    d_2 = Concatenate(name='d2_concat')([d_2, e_1])

    d_1 = Conv3DTranspose(filters=8, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d1_convT')(d_2)
    d_1 = BatchNormalization(name='d1_bn')(d_1)
    d_1 = Activation('relu', name='d1_activation')(d_1)
    # d_1 = Dropout(0.5, name='d1_dropout')(d_1)

    dtp = Conv3D(filters=8, kernel_size=(3,5,5), 
                    padding='same', kernel_initializer='he_normal', name='d0_conv')(d_1)
    dtp = BatchNormalization(name='d0_bn')(dtp)
    dtp = Activation('relu', name='d0_activation')(dtp)
    # dtp = Dropout(0.5, name='d0_dropout')(dtp)
    dtp = Concatenate(name='d0_concat')([dtp, x_input])

    last_h = Conv3D(filters=1, kernel_size=(3,5,5), padding='same', 
                    kernel_initializer='he_normal', name='p_conv')(dtp)
    p_hat = Activation('sigmoid', name='p_hat')(last_h)

    model = Model(inputs=[x_input], outputs=[p_hat])
    # plot_model(model,to_file='3D_Unet_arb.png',show_shapes=True)
    return model



def get_3d_small_arb_unet(img_rows=None, img_cols=None, img_slices=None, img_channels=1):
    """
    TODO: 
    1. make the architecture works for odd number of z slices
    1. add architecture input shape in ref_config
    """
    # patch_size = (img_slices, img_rows, img_cols, img_channels)
    patch_size = (None, None, None, img_channels)
    
    """Encoder"""
    x_input = Input(shape=patch_size, name='input')
    e_1 = Conv3D(filters=16, # TODO: chech when the number of filters is 16
                kernel_size=(3,3,3), strides=(2,2,2), 
                padding='same', kernel_initializer='he_normal', name='e1_conv')(x_input)
    e_1 = BatchNormalization(name='e1_bn')(e_1)
    e_1 = Activation('relu', name='e1_activation')(e_1)
    # e_1 = Dropout(0.5, name='e1_dropout')(e_1)

    e_2 = Conv3D(filters=16, kernel_size=(1,3,3), 
                padding='same', kernel_initializer='he_normal', name='e2_conv')(e_1)
    e_2 = BatchNormalization(name='e2_bn')(e_2)
    e_2 = Activation('relu', name='e2_activation')(e_2)
    # e_2 = Dropout(0.5, name='e2_dropout')(e_2)

    e_3 = Conv3D(filters=32, kernel_size=(1,3,3), strides=(1,2,2), 
                    padding='same', kernel_initializer='he_normal', name='e3_conv')(e_2)
    e_3 = BatchNormalization(name='e3_bn')(e_3)
    e_3 = Activation('relu', name='e3_activation')(e_3)
    # e_3 = Dropout(0.5, name='e3_dropout')(e_3)

    e_4 = Conv3D(filters=32, kernel_size=(1,3,3), 
                    padding='same', kernel_initializer='he_normal', name='e4_conv')(e_3)
    e_4 = BatchNormalization(name='e4_bn')(e_4)
    e_4 = Activation('relu', name='e4_activation')(e_4)
    # e_4 = Dropout(0.5, name='e4_dropout')(e_4)

    e_5 = Conv3D(filters=64, kernel_size=(1,3,3), 
                    padding='same', kernel_initializer='he_normal', name='e5_conv')(e_4)
    e_5 = BatchNormalization(name='e5_bn')(e_5)
    e_5 = Activation('relu', name='e5_activation')(e_5)
    # e_5 = Dropout(0.5, name='e5_dropout')(e_5)

    ################################################################################################################
    # center : encoder to decoder

    etd = Conv3D(filters=64, kernel_size=(1,5,5), 
                    padding='same', kernel_initializer='he_normal', name='center_conv')(e_5)
    etd = BatchNormalization(name='center_bn')(etd)
    etd = Activation('relu', name='center_activation')(etd)
    # etd = Dropout(0.5, name='center_dropout')(etd)

    ################################################################################################################
    """Decoder"""
    d_5 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d5_convT')(etd)
    d_5 = BatchNormalization(name='d5_bn')(d_5)
    d_5 = Activation('relu', name='d5_activation')(d_5)
    # d_5 = Dropout(0.5, name='d5_dropout')(d_5)

    d_4 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d4_convT')(d_5)
    d_4 = BatchNormalization(name='d4_bn')(d_4)
    d_4 = Activation('relu', name='d4_activation')(d_4)
    # d_4 = Dropout(0.5, name='d4_dropout')(d_4)
    d_4 = Concatenate(name='d4_concat')([d_4, e_3])

    d_3 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d3_convT')(d_4)
    d_3 = BatchNormalization(name='d3_bn')(d_3)
    d_3 = Activation('relu', name='d3_activation')(d_3)
    # d_3 = Dropout(0.5, name='d3_dropout')(d_3)

    d_2 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d2_convT')(d_3)
    d_2 = BatchNormalization(name='d2_bn')(d_2)
    d_2 = Activation('relu', name='d2_activation')(d_2)
    d_2 = Concatenate(name='d2_concat')([d_2, e_1])

    d_1 = Conv3DTranspose(filters=16, kernel_size=(3,3,3), strides=(2,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d1_convT')(d_2)
    d_1 = BatchNormalization(name='d1_bn')(d_1)
    d_1 = Activation('relu', name='d1_activation')(d_1)
    # d_1 = Dropout(0.5, name='d1_dropout')(d_1)

    dtp = Conv3D(filters=16, kernel_size=(3,3,3), 
                    padding='same', kernel_initializer='he_normal', name='d0_conv')(d_1)
    dtp = BatchNormalization(name='d0_bn')(dtp)
    dtp = Activation('relu', name='d0_activation')(dtp)
    # dtp = Dropout(0.5, name='d0_dropout')(dtp)
    dtp = Concatenate(name='d0_concat')([dtp, x_input])

    last_h = Conv3D(filters=1, kernel_size=(3,3,3), padding='same', 
                    kernel_initializer='he_normal', name='p_conv')(dtp)
    p_hat = Activation('sigmoid', name='p_hat')(last_h)

    model = Model(inputs=[x_input], outputs=[p_hat])

    model.summary()
    return model


def get_3d_2d_small_arb_unet(img_rows=None, img_cols=None, img_slices=None, img_channels=1):
    """
    TODO: 
    1. add architecture input shape in ref_config
    """
    # patch_size = (img_slices, img_rows, img_cols, img_channels)
    patch_size = (None, None, None, img_channels)
    
    """Encoder"""
    x_input = Input(shape=patch_size, name='input')
    e_1 = Conv3D(filters=8, # TODO: chech when the number of filters is 16
                kernel_size=(1,5,5), strides=(1,2,2), 
                padding='same', kernel_initializer='he_normal', name='e1_conv')(x_input)
    e_1 = BatchNormalization(name='e1_bn')(e_1)
    e_1 = Activation('relu', name='e1_activation')(e_1)
    # e_1 = Dropout(0.5, name='e1_dropout')(e_1)

    e_2 = Conv3D(filters=16, kernel_size=(1,3,3), 
                padding='same', kernel_initializer='he_normal', name='e2_conv')(e_1)
    e_2 = BatchNormalization(name='e2_bn')(e_2)
    e_2 = Activation('relu', name='e2_activation')(e_2)
    # e_2 = Dropout(0.5, name='e2_dropout')(e_2)

    e_3 = Conv3D(filters=32, kernel_size=(1,3,3), strides=(1,2,2), 
                    padding='same', kernel_initializer='he_normal', name='e3_conv')(e_2)
    e_3 = BatchNormalization(name='e3_bn')(e_3)
    e_3 = Activation('relu', name='e3_activation')(e_3)
    # e_3 = Dropout(0.5, name='e3_dropout')(e_3)

    e_4 = Conv3D(filters=32, kernel_size=(1,3,3), 
                    padding='same', kernel_initializer='he_normal', name='e4_conv')(e_3)
    e_4 = BatchNormalization(name='e4_bn')(e_4)
    e_4 = Activation('relu', name='e4_activation')(e_4)
    # e_4 = Dropout(0.5, name='e4_dropout')(e_4)

    e_5 = Conv3D(filters=64, kernel_size=(1,3,3), strides=(1,2,2), 
                    padding='same', kernel_initializer='he_normal', name='e5_conv')(e_4)
    e_5 = BatchNormalization(name='e5_bn')(e_5)
    e_5 = Activation('relu', name='e5_activation')(e_5)
    # e_5 = Dropout(0.5, name='e5_dropout')(e_5)

    ################################################################################################################
    # center : encoder to decoder

    etd = Conv3D(filters=64, kernel_size=(1,5,5), 
                    padding='same', kernel_initializer='he_normal', name='center_conv')(e_5)
    etd = BatchNormalization(name='center_bn')(etd)
    etd = Activation('relu', name='center_activation')(etd)
    # etd = Dropout(0.5, name='center_dropout')(etd)

    """Decoder"""
    d_5 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d5_convT')(etd)
    d_5 = BatchNormalization(name='d5_bn')(d_5)
    d_5 = Activation('relu', name='d5_activation')(d_5)
    # d_5 = Dropout(0.5, name='d5_dropout')(d_5)

    d_4 = Conv3DTranspose(filters=32, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d4_convT')(d_5)
    d_4 = BatchNormalization(name='d4_bn')(d_4)
    d_4 = Activation('relu', name='d4_activation')(d_4)
    # d_4 = Dropout(0.5, name='d4_dropout')(d_4)
    d_4 = Concatenate(name='d4_concat')([d_4, e_3])

    d_3 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d3_convT')(d_4)
    d_3 = BatchNormalization(name='d3_bn')(d_3)
    d_3 = Activation('relu', name='d3_activation')(d_3)
    # d_3 = Dropout(0.5, name='d3_dropout')(d_3)

    d_2 = Conv3DTranspose(filters=16, kernel_size=(1,3,3), 
                            padding='same', kernel_initializer='he_normal', name='d2_convT')(d_3)
    d_2 = BatchNormalization(name='d2_bn')(d_2)
    d_2 = Activation('relu', name='d2_activation')(d_2)
    d_2 = Concatenate(name='d2_concat')([d_2, e_1])

    d_1 = Conv3DTranspose(filters=8, kernel_size=(1,3,3), strides=(1,2,2), 
                            padding='same', kernel_initializer='he_normal', name='d1_convT')(d_2)
    d_1 = BatchNormalization(name='d1_bn')(d_1)
    d_1 = Activation('relu', name='d1_activation')(d_1)
    # d_1 = Dropout(0.5, name='d1_dropout')(d_1)

    dtp = Conv3D(filters=8, kernel_size=(1,5,5), 
                    padding='same', kernel_initializer='he_normal', name='d0_conv')(d_1)
    dtp = BatchNormalization(name='d0_bn')(dtp)
    dtp = Activation('relu', name='d0_activation')(dtp)
    # dtp = Dropout(0.5, name='d0_dropout')(dtp)
    dtp = Concatenate(name='d0_concat')([dtp, x_input])

    last_h = Conv3D(filters=1, kernel_size=(1,5,5), padding='same', 
                    kernel_initializer='he_normal', name='p_conv')(dtp)
    p_hat = Activation('sigmoid', name='p_hat')(last_h)

    model = Model(inputs=[x_input], outputs=[p_hat])
    return model