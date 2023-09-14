"""Convolutional Layers: attention family

Spec
----
Search_Area_Attention
    attention_type = none
        ; type of attention: str
        ;     supported type: hard, soft, none
        ;     hard: hard (constant) attention.
        ;           The serach area will be used as a constant attention area.
        ;     soft: soft (trainable) attention.
        ;           Attention weight is trainable, and attention area can be changed from search area.
        ;           The attention weight will be constrained to be as same as the search area.
        ;     none: no attention will be added to the input layer
    attention_initial = none
        ; path of ndarray
        ;    For soft attention, initial weight can be set by attention_initial.
        ;    If none, "one" will be used as initial weight.
    intensity_value = none
        ; the intensity (weighting amount) value of whole attention: float (range within [0,1])
        ;     if intensity_trainable = True, this will be the initial value of the intentisy parameter
        ;     if none, random initial value will be used for trainable intensity_value. 
        ;     none initial value for constant intensity_value will raise error.
    intensity_trainable = false
        ; whether the intensity of attention is trainable or not (constant)
        ;     if false, intensity_value will be constidered as a constant hyper-parameter
        ;     if true, intensity_value will be trained. The input intensity_value will be set as a initial.

TODO 
---
@youngwonchoi

intensity_window_attention
local_attention
self_attention
"""

import os
import numpy as np
import tensorflow as tf
from keras.layers import Layer
from keras import initializers

class SearchAreaAttention(Layer):
    """Search Area Attention layer Class
    Search Area attention layer class inheriting the keras Layer class.
    
    Parameters
    ----------
    attention_initial: ndarray
        For soft attention, initial weight can be set by attention_initial.
        If none, "one" will be used as initial weight.
    intensity_trainable: bool
        whether the intensity of attention is trainable or not (constant)
        if false, intensity_value will be constidered as a constant hyper-parameter
        if true, intensity_value will be trained. The input intensity_value will be set as a initial.
    intensity_initial: float
        the intensity (weighting amount) value of whole attention: float (range within [0,1])
        if none, random initial value will be used for trainable intensity_value. 
        none initial value for constant intensity_value will raise error.

    Attributes
    ----------
    
    Methods
    -------
    
    Usage
    -----
    
    Examples
    --------

    """
    def __init__(self, attention_initial=None, intensity_trainable=False, intensity_initial=0.,
                 **kwargs):
        super(SearchAreaAttention, self).__init__(**kwargs)
        self.attention_initial = attention_initial
        self.intensity_trainable = intensity_trainable
        self.intensity_initial = intensity_initial
    
    def get_config(self):
        config = {
            'attention_initial': self.attention_initial,
            'intensity_trainable': self.intensity_trainable,
            'intensity_initial':self.intensity_initial
        }
        base_config = super(SearchAreaAttention, self).get_config()
        return dict(list(base_config.items()) + list(config.items()))


class SearchAreaAttentionHard(SearchAreaAttention):
    """Hard-type Search Area Attention layer Class
    hard search area attention layer class inheriting the Search Area attention layer class.
    
    Parameters
    ----------
    attention_initial: ndarray
        For soft attention, initial weight can be set by attention_initial.
        If none, "one" will be used as initial weight.
    intensity_trainable: bool
        whether the intensity of attention is trainable or not (constant)
        if false, intensity_value will be constidered as a constant hyper-parameter
        if true, intensity_value will be trained. The input intensity_value will be set as a initial.
    intensity_initial: float
        the intensity (weighting amount) value of whole attention: float (range within [0,1])
        if none, random initial value will be used for trainable intensity_value. 
        none initial value for constant intensity_value will raise error.

    Attributes
    ----------
    
    Methods
    -------
    
    Usage
    -----
    
    Examples
    --------
    """
    def __init__(self, attention_initial=None, intensity_trainable=False, intensity_initial=0., 
                **kwargs):
        super(SearchAreaAttentionHard, self).__init__(attention_initial, intensity_trainable, intensity_initial, **kwargs)
    
    def build(self, input_shape):
        from keras import backend as K
        self.intensity_weight = self.add_weight(name='intensity', 
                                      shape=(1,),
                                      initializer=initializers.constant(self.intensity_initial),
                                      trainable=self.intensity_trainable)
        if self.intensity_trainable:
            self.intensity = K.sigmoid(self.intensity_weight)
        else:
            self.intensity = self.intensity_weight
        super(SearchAreaAttentionHard, self).build(input_shape)

    def call(self, x):
        input_img = x[...,:-1]
        input_search_area = x[...,-1:]
        
        attention = self.intensity * input_search_area
        attentioned_img = input_search_area * input_img
        return_img = self.intensity * attentioned_img + (1.-self.intensity) * input_img
        return [return_img, attention]
        
    def compute_output_shape(self, input_shape):
        img_shape = tuple(list(input_shape[:-1]) + [input_shape[-1]-1])
        search_area_attention_shape = tuple(list(input_shape[:-1]) + [1])
        return [img_shape, search_area_attention_shape]
    
# class SearchAreaAttentionSoft(SearchAreaAttention):
#     """Hard Search Area Attention layer Class
#     hard search area attention layer class inheriting the Search Area attention layer class.
    
#     Parameters
#     ----------
#     attention_initial: ndarray
#         For soft attention, initial weight can be set by attention_initial.
#         If none, "one" will be used as initial weight.
#     intensity_trainable: bool
#         whether the intensity of attention is trainable or not (constant)
#         if false, intensity_value will be constidered as a constant hyper-parameter
#         if true, intensity_value will be trained. The input intensity_value will be set as a initial.
#     intensity_initial: float
#         the intensity (weighting amount) value of whole attention: float (range within [0,1])
#         if none, random initial value will be used for trainable intensity_value. 
#         none initial value for constant intensity_value will raise error.

#     Attributes
#     ----------
    
#     Methods
#     -------
    
#     Usage
#     -----
    
#     Examples
#     --------
#     """
#     def build(self, input_shape):
#         self.kernelf = self.add_weight(name='convf', 
#                                       shape=(1,1,1,input_shape[-1],1),
#                                       initializer='glorot_uniform',
#                                       trainable=True)
# #         self.kernelg = self.add_weight(name='convg', 
# #                                       shape=(1,1,1,input_shape[-1],input_shape[-1]//4),
# #                                       initializer='glorot_uniform',
# #                                       trainable=True)
#         self.kernelh = self.add_weight(name='convh', 
#                                       shape=(1,1,1,input_shape[-1],input_shape[-1]),
#                                       initializer='glorot_uniform',
#                                       trainable=True)
# #         self.gamma_1 = self.add_weight(name='gamma_1', 
# #                                       shape=(1,),
# #                                       initializer='zeros',
# #                                       trainable=True)
# #         self.gamma_2 = self.add_weight(name='gamma_2', 
# #                                       shape=(1,),
# #                                       initializer='ones',
# #                                       trainable=True)
#         self.gamma = self.add_weight(name='gamma', 
#                                       shape=(1,),
#                                       initializer='zeros',
#                                       trainable=True)
#         super(SoftAttention, self).build(input_shape)

#     def call(self, x):
#         import keras.backend as k
#         import tensorflow as tf
#         def hw_flatten(x): return tf.reshape(x,[-1, k.shape(x)[1]*k.shape(x)[2]*k.shape(x)[3], k.shape(x)[4]])
        
#         f = k.conv3d(x, kernel=self.kernelf, padding='same') # [bs, t, h, w, c']
# #         g = k.conv3d(x, kernel=self.kernelg, padding='same') # [bs, t, h, w, c']
#         h = k.conv3d(x, kernel=self.kernelh, padding='same') # [bs, t, h, w, c]
# #         s = k.sum(f*g, axis=4, keepdims=True)  # [bs, t , h , w] 
#         s = f
#         beta = k.sigmoid(s)  # attention map [bs, t , h , w]
#         o = beta * h  # [bs, t, h, w, c]
# #         gamma = (self.gamma_1)/(self.gamma_1 + self.gamma_2+k.epsilon())
#         x = self.gamma  * o + (1-self.gamma) * x
#         return [x, beta]

#     def compute_output_shape(self, input_shape):
#         return [input_shape, (input_shape[0], input_shape[1], input_shape[2], input_shape[3], 1)]
