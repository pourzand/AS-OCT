"""TF functions for loss and metrics

"""
import numpy as np
from keras.callbacks import Callback
from sklearn.metrics import confusion_matrix, f1_score, precision_score, recall_score
from medpy import metric as medmetric

import tensorflow as tf
import keras.backend as K

from keras.losses import mean_squared_error, mean_absolute_error, binary_crossentropy
from keras.metrics import binary_accuracy

def MTL_carina_ettip(y_true, y_pred):
#     prf_carina, pcf_carina = roi_arr[0]
#     prf_ettip, pcf_ettip = roi_arr[1]
    pred_carina = y_pred[0]
    pred_ettip = y_pred[1]
    true_carina = y_true[0]
    true_ettip = y_true[1]

    carina_mae = mean_absolute_error(true_carina, pred_carina)
    ettip_mae = mean_absolute_error(true_ettip, pred_ettip)
    mtl_loss = (carina_mae * 250) + (ettip_mae * 250)
    return mtl_loss

def MTL_l2_distance_carina(y_true, y_pred):
#     prf_carina, pcf_carina = roi_arr[0]
#     prf_ettip, pcf_ettip = roi_arr[1]
    pred_carina = y_pred[0]
    true_carina = y_true[0]
    carina_l2 = l2_distance(true_carina, pred_carina)
    return carina_l2

def MTL_l2_distance_ettip(y_true, y_pred):
#     prf_carina, pcf_carina = roi_arr[0]
#     prf_ettip, pcf_ettip = roi_arr[1]
    pred_ettip = y_pred[1]
    true_ettip = y_true[1]
    ettip_l2 = l2_distance(true_ettip, pred_ettip)
    return ettip_l2

# def MTL_carina_ettip_dist(y_true, y_pred):
# #     prf_carina, pcf_carina = roi_arr[0]
# #     prf_ettip, pcf_ettip = roi_arr[1]
#     pred_y_carina, pred_x_carina = y_pred[0]
#     pred_y_ettip, pred_x_ettip = y_pred[1]
#     true_y_carina, true_x_carina = y_true[0]
#     true_y_ettip, true_x_ettip = y_true[1]

#     carina_mae = mean_absolute_error((true_y_carina, true_x_carina), (pred_y_carina, pred_x_carina))
#     ettip_mae = mean_absolute_error((true_y_ettip, true_x_ettip), (pred_y_ettip, pred_x_ettip))

#     mtl_loss = (carina_mae * 250) + (ettip_mae * 250)
#     return mtl_loss

def weighted_binary_crossentropy(y_true, y_pred):
    y_pred_clip = K.clip(y_pred, 1e-10, 1-1e-10)
    w = K.sum(1-y_pred_clip) / K.sum(y_pred_clip)
    return -K.mean(w*y_true*K.log(y_pred_clip) + (1-y_true)*K.log(1-y_pred_clip))

def precision(y_true, y_pred):
    return K.sum(y_true*y_pred)/(K.sum(y_true*y_pred) + K.sum((1-y_true)*y_pred) + K.epsilon())

def recall(y_true, y_pred):
    return K.sum(y_true*y_pred)/(K.sum(y_true*y_pred) + K.sum(y_true*(1-y_pred)) + K.epsilon())

def dice(y_true, y_pred):
    smoothing_factor = K.epsilon()
    # y_pred_clip = K.clip(y_pred, 1e-10, 1-1e-10)
    # y_true_clip = K.clip(y_true, 1e-10, 1-1e-10)
    # return (2. * K.sum(y_true_clip * y_pred_clip) + smoothing_factor)/ (K.sum(y_true_clip) + K.sum(y_pred_clip) + smoothing_factor)
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)
    return (2.0 *  K.sum(y_true_f * y_pred_f) + smoothing_factor) / (K.sum(y_true_f) + K.sum(y_pred_f) + smoothing_factor)

def negative_dice(y_true, y_pred):
    return -dice(y_true, y_pred)

def bce_dice_loss(y_true, y_pred):
    return binary_crossentropy(y_true, y_pred) - dice(y_true, y_pred)

def bce_dice_loss_weighted(y_true, y_pred):
    alpha = 0.7
    return alpha*binary_crossentropy(y_true, y_pred) - (1-alpha)*dice(y_true, y_pred)

def CFDL2(y_true, y_pred):
    '''
    Generalized Dice Loss:
    Dice Loss weighted by a **sqaured** inverse class frequency term
    '''
 
    smoothing_factor = K.epsilon()
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)

    # Define the Inverse Frequency Weight term
    y_true_f = K.cast(y_true_f, dtype='int32')
    label_count = tf.compat.v1.bincount(y_true_f, dtype=tf.dtypes.float32)

    weights_inv = 1 / K.square(label_count)

    # Normalize the weights 
    weights_inv /= K.sum(weights_inv)

    y_true_f = K.cast(y_true_f, y_pred_f.dtype)

    l_numerator = 2.0 * K.sum(weights_inv[1] * K.sum(y_true_f * y_pred_f) + 
                            weights_inv[0] * K.sum((1 - y_true_f) * (1 - y_pred_f)))
    l_denominator = K.sum(weights_inv[1] * K.sum(y_true_f + y_pred_f) +  
                            weights_inv[0] * K.sum((1 - y_true_f) + (1 - y_pred_f)))
    return 1 - (l_numerator + smoothing_factor) / (l_denominator + smoothing_factor)

def CFDL3(y_true, y_pred):
    '''
    Generalized Dice Loss:
    Dice Loss weighted by a **cubed** inverse class frequency term
    '''
 
    smoothing_factor = K.epsilon()
    y_true_f = K.flatten(y_true)
    y_pred_f = K.flatten(y_pred)

    # Define the Inverse Frequency Weight term
    y_true_f = K.cast(y_true_f, dtype='int32')
    label_count = tf.compat.v1.bincount(y_true_f, dtype=tf.dtypes.float32)

    weights_inv = 1 / (label_count * label_count * label_count)

    # Normalize the weights 
    weights_inv /= K.sum(weights_inv)

    y_true_f = K.cast(y_true_f, y_pred_f.dtype)

    l_numerator = 2.0 * K.sum(weights_inv[1] * K.sum(y_true_f * y_pred_f) + 
                            weights_inv[0] * K.sum((1 - y_true_f) * (1 - y_pred_f)))
    l_denominator = K.sum(weights_inv[1] * K.sum(y_true_f + y_pred_f) +  
                            weights_inv[0] * K.sum((1 - y_true_f) + (1 - y_pred_f)))
    return 1 - (l_numerator + smoothing_factor) / (l_denominator + smoothing_factor)

def CF_Dice(power_p):
    #TODO: Currently not working; need to fix the wrapper function
    '''
    A wrapper function that
    1. Takes a power parameter as an input
    2. Pass it to a class frequency weighted dice loss function (CFDL)
    3. Assigns the power parameter to the weight term
    
    Input: power_p; the power parameter
    Output: CFDL
    '''
    def CFDL(y_true, y_pred):
        '''
        Class frequency weighted dice loss function:
        Dice Loss weighted by an inverse class frequency term
        '''
    
        smoothing_factor = K.epsilon()
        y_true_f = K.flatten(y_true)
        y_pred_f = K.flatten(y_pred)

        # Define the Inverse Frequency Weight term
        y_true_f = K.cast(y_true_f, dtype='int32')
        label_count = tf.compat.v1.bincount(y_true_f, dtype=tf.dtypes.float32)
        
        # The first element of weights_inv indicates the number of background pixels 
        weights_inv = 1 / K.pow(label_count, power_p)
        
        # Normalize the weights 
        weights_inv /= K.sum(weights_inv)               

        y_true_f = K.cast(y_true_f, y_pred_f.dtype)

        l_numerator = 2.0 * (weights_inv[1] * K.sum(y_true_f * y_pred_f) + weights_inv[0] * K.sum((1 - y_true_f) * (1 - y_pred_f)))
        l_denominator = weights_inv[1] * K.sum(y_true_f + y_pred_f) + weights_inv[0] * K.sum((1 - y_true_f) + (1 - y_pred_f))
        
        return 1 - (l_numerator + smoothing_factor) / (l_denominator + smoothing_factor)
    return CFDL

def sensitivity(y_true, y_pred):
#     y_pred = K.round(y_pred)
#     neg_y_pred = 1 - y_pred
#     true_positive = K.sum(y_true * y_pred)
#     false_negative = K.sum(y_true * neg_y_pred)
#     return (true_positive) / (true_positive + false_negative + K.epsilon())
    y_pred = K.cast(K.greater(K.clip(y_pred, 0, 1), 0.5), K.floatx())
    neg_y_pred = 1 - y_pred
    true_positive = K.round(K.sum(K.clip(y_true * y_pred, 0, 1)))
    false_negative = K.round(K.sum(K.clip(y_true * neg_y_pred, 0, 1)))
    return (true_positive) / (true_positive + false_negative + K.epsilon())

def specificity(y_true, y_pred):
#     y_pred = K.round(y_pred)
#     neg_y_true = 1 - y_true
#     neg_y_pred = 1 - y_pred
#     false_positive = K.sum(neg_y_true * y_pred)
#     true_negative = K.sum(neg_y_true * neg_y_pred)
#     return (true_negative) / (false_positive + true_negative + K.epsilon())
    y_pred = K.cast(K.greater(K.clip(y_pred, 0, 1), 0.5), K.floatx())
    neg_y_true = 1 - y_true
    neg_y_pred = 1 - y_pred
    false_positive = K.round(K.sum(K.clip(neg_y_true * y_pred, 0, 1)))
    true_negative = K.round(K.sum(K.clip(neg_y_true * neg_y_pred, 0, 1)))
    return (true_negative) / (false_positive + true_negative + K.epsilon())


def negative_minimum_sensitivity_specificity(y_true, y_pred):
    neg_y_pred = 1 - y_pred
    true_positive = K.sum(y_true * y_pred)
    false_negative = K.sum(y_true * neg_y_pred)
    smooth_sensitivity =  (true_positive) / (true_positive + false_negative + K.epsilon())

    neg_y_true = 1 - y_true
    neg_y_pred = 1 - y_pred
    false_positive = K.sum(neg_y_true * y_pred)
    true_negative = K.sum(neg_y_true * neg_y_pred)
    smooth_specificity = (true_negative) / (false_positive + true_negative + K.epsilon())
    
    return -K.minimum(smooth_sensitivity, smooth_specificity)

def l2_distance(a,b):
    """
    Euclidean distance (L2)
    Need to correct with true distence weight (from image offset)
    """
    return K.sqrt(K.sum(K.square(a-b)))

def mean_squared_error(a,b):
    """
    Euclidean distance
    Need to correct with true distence weight (from image offset)
    """
    return K.sum(K.square(a-b))


def mean_squared_error_scaled(a,b):
    """
    Euclidean distance
    Need to correct with true distence weight (from image offset)
    """
    return K.sum(K.square(a-b))*250

# def mean_absolute_error(a,b):
#     return mean_absolute_error(a,b)

def mean_absolute_error_scaled(a,b):
    return mean_absolute_error(a,b)*250


    
def where3D(a):
    a_shape = K.shape(a)
    z = K.repeat(K.arange(a_shape[0]), a_shape[1]*a_shape[2]).reshape(a_shape[0],a_shape[1],a_shape[2])
    x = K.repeat(K.array([K.repeat(K.arange(a_shape[1]), a_shape[2]).reshape(a_shape[1],a_shape[2])]),
                  a_shape[0], axis=0)
    y = K.repeat(K.array([K.repeat(K.array([K.arange(a_shape[2])]), a_shape[1], axis=0)]), a_shape[0], axis=0)
    return K.stack([z[a], x[a], y[a]]).transpose()
    
def asd(A_in, B_in):
    """
    Average surface distance (ASD)
    """
    # TODO : fix (to be surface version)
    A = K.cast(where3D(K.not_equal(A_in, 0)), 'float32')
    B = K.cast(where3D(K.not_equal(B_in, 0)), 'float32')
    def dist_fn(x): return K.min(K.map_fn(lambda y: distance(x[0],y), x[1]))
    return K.mean(K.map_fn(lambda x: dist_fn((x,B)), A))
    

def assd(y_true, y_pred):
    """Average symmetric surface distance (ASSD)"""
    return (asd(y_true,y_pred)+asd(y_pred,y_true)) * 0.5
    
def assd_round(y_true, y_pred_in):
    """Average symmetric surface distance (ASSD)"""
    y_pred = K.round(y_pred_in)
    return (asd(y_true,y_pred)+asd(y_pred,y_true)) * 0.5
    
def hd_nonsym(A_in, B_in):
    A = K.cast(where3D(K.not_equal(A_in, 0)), 'float32')
    B = K.cast(where3D(K.not_equal(B_in, 0)), 'float32')
    def dist_fn(x): return K.min(K.map_fn(lambda y: distance(x[0],y), x[1]))
    return K.max(K.map_fn(lambda x: dist_fn((x,B)), A))

def hd(y_true, y_pred):
    """Hausdorff distance (HD)"""
    return K.maximum(hd_nonsym(y_true, y_pred),hd_nonsym(y_pred,y_true))

def hd_round(y_true, y_pred_in):
    """Hausdorff distance (HD)"""
    y_pred = K.round(y_pred_in)
    return K.maximum(hd_nonsym(y_true, y_pred),hd_nonsym(y_pred,y_true))

        
def metric_test_simple(y_true, y_pred):
    # numpy metric_test
    # diceAB = 2 * np.sum(y_true * y_pred)/ (np.sum(y_true) + np.sum(y_pred) + 1e-7)
    # precisionAB = np.sum(y_true*y_pred)/(np.sum(y_true*y_pred) + np.sum((1-y_true)*y_pred) + 1e-7)
    # recallAB = np.sum(y_true*y_pred)/(np.sum(y_true*y_pred) + np.sum(y_true*(1-y_pred)) + 1e-7)
    
    # diceAB = 2 * (np.sum(y_true * y_pred) + 0.5*1e-7)/ (np.sum(y_true) + np.sum(y_pred) + 1e-7)
    # precisionAB = (np.sum(y_true*y_pred) + 1e-7)/(np.sum(y_true*y_pred) + np.sum((1-y_true)*y_pred) + 1e-7)
    # recallAB = (np.sum(y_true*y_pred) + 1e-7)/(np.sum(y_true*y_pred) + np.sum(y_true*(1-y_pred)) + 1e-7)
    
    diceAB = medmetric.dc(y_pred,y_true)
    precisionAB = medmetric.precision(y_pred,y_true)
    recallAB = medmetric.recall(y_pred,y_true)
    return diceAB, precisionAB, recallAB
        
def metric_test(y_true, y_pred, spacing):
    # numpy metric_test
    # diceAB = 2 * (np.sum(y_true * y_pred))/ (np.sum(y_true) + np.sum(y_pred) + 1e-7)
    
    # diceAB = 2 * (np.sum(y_true * y_pred) + 0.5*1e-7)/ (np.sum(y_true) + np.sum(y_pred) + 1e-7)
    # precisionAB = (np.sum(y_true*y_pred) + 1e-7)/(np.sum(y_true*y_pred) + np.sum((1-y_true)*y_pred) + 1e-7)
    # recallAB = (np.sum(y_true*y_pred) + 1e-7)/(np.sum(y_true*y_pred) + np.sum(y_true*(1-y_pred)) + 1e-7)
    
#     A = np.transpose(np.nonzero(y_true)).astype(np.float)
#     B = np.transpose(np.nonzero(y_pred)).astype(np.float)
#     if B.shape[0] == 0:
#         asdA = 0
#         asdB = 0
#         hdA = 0
#         hdB = 0
#     elif A.shape[0] == 0:
#         asdA = 0
#         asdB = 0
#         hdA = 0
#         hdB = 0
#     else:
#         AB = list(map(lambda x: np.min(np.sqrt(np.sum(np.square((x-B)*np.array(spacing)), axis=1))), A))
#         BA = list(map(lambda x: np.min(np.sqrt(np.sum(np.square((x-A)*np.array(spacing)), axis=1))), B))
#         asdA = np.mean(AB)
#         asdB = np.mean(BA)
#         hdA = np.max(AB)
#         hdB = np.max(BA)
    # return diceAB, 0.5*(asdA+asdB), max(hdA, hdB), precisionAB, recallAB
    
    diceAB = medmetric.dc(y_pred,y_true)
    precisionAB = medmetric.precision(y_pred,y_true)
    recallAB = medmetric.recall(y_pred,y_true)
    # asdAB = medmetric.asd(y_pred, y_true, voxelspacing=spacing)
    try:
        assdAB = medmetric.assd(y_pred, y_true, voxelspacing=spacing)
    except:
        assdAB = np.nan
    try:
        hdAB = medmetric.hd95(y_pred, y_true, voxelspacing=spacing)
    except:
        hdAB = np.nan
    return diceAB, assdAB, hdAB, precisionAB, recallAB

      
if __name__ == "__main__":
    print('Test loss functions (Dice / ASSD / HD / precision / recall)')
    y_true_set = np.array([[[0,0,0,0,0],
                            [0,0,0,0,0],
                            [0,1,1,0,0],
                            [1,1,1,0,0],
                            [0,1,0,0,0]]])
    y_pred_set = np.array([[[0,0,0,0,1],
                            [0,0,0,0,0],
                            [0,1,0.6,0,0],
                            [0,1,1,0,0],
                            [0,0.3,0,0,0]]])
    
    def test(acc, y_true_set, y_pred_set):
        sess = tf.Session()
        K.set_session(sess)
        with sess.as_default():
            return acc.eval(feed_dict={y_true: y_true_set, y_pred: y_pred_set})
    
    # tf
    y_true = tf.placeholder("float32", shape=(None,y_true_set.shape[1],y_true_set.shape[2])) 
    y_pred = tf.placeholder("float32", shape=(None,y_pred_set.shape[1],y_pred_set.shape[2]))

    #acc = keras.metrics.binary_crossentropy(y_true, y_pred)
    #acc = sum_binary_crossentropy(y_true, y_pred)
    metric_list = [dice(y_true, y_pred), 
                   assd(y_true, y_pred),
                   hd(y_true, y_pred),
                   precision(y_true, y_pred),
                   recall(y_true, y_pred)]

    # numpy
    print('Dice\t ASSD\t 95HD\t precision\t recall')
    print('tf : {}'.format([test(acc, y_true_set, y_pred_set) for acc in metric_list]))
    print('np : {}'.format(np.round(metric_test(y_true_set[0],y_pred_set[0]),8)))
