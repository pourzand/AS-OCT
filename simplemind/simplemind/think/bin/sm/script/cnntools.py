import os
import sys
import __qia__
import qia.common.img.image as qimage
# import qia.common.dicom.obj as qdicom
#import matplotlib.pyplot as plt
import numpy as np
#import csv
from sklearn.preprocessing import scale
from skimage import exposure
from medpy.filter.smoothing import anisotropic_diffusion
#from skimage import img_as_float
#import matplotlib.pyplot as plt
#from argparse import ArgumentParser
#from unet import get_unet
#from scipy import ndimage
import SimpleITK as sitk
import psutil, pwd, getpass

def norm_upper_centile(arr, ph_area_image_array=None, centile0=0, centile1=100):
    if ph_area_image_array is not None: lookup_arr = ph_area_image_array
    else: lookup_arr = arr
    c0 = np.percentile(lookup_arr, centile0)
    c1 = np.percentile(lookup_arr, centile1)
    # print("min ", np.amin(arr))
    # print("max ", np.amax(arr))
    # print("c0 ", c0)
    # print("c1 ", c1)
    new_arr = ((arr-c0)/(c1-c0)).clip(0,1)
    # print("new_arr centile0 ", np.percentile(new_arr, centile0))
    # print("new_arr centile1 ", np.percentile(new_arr, centile1))
    return new_arr
	
def norm_mean_std(arr, ph_area_image_array=None):
    if ph_area_image_array is not None: lookup_arr = ph_area_image_array
    else: lookup_arr = arr
    #return (arr-np.percentile(arr, 75))/np.std(arr)
    return (arr-np.mean(lookup_arr))/np.std(lookup_arr)

def norm_min_max(arr, ph_area_image_array=None):
    if ph_area_image_array is not None: lookup_arr = ph_area_image_array
    else: lookup_arr = arr
    c0 = np.min(lookup_arr)
    c1 = np.max(lookup_arr)
    return ((arr-c0)/(c1-c0)).clip(0,1)

def norm_generic_CT(arr, ph_area_image_array=None, MINVAL=-1000, MAXVAL=1000):
    """Apply generic CT normalization to a numpy array.
    Scales the array from MINVAL HU and MAXVAL HU to 0 and 1 (values outside the limit are clipped).
    """
    RANGE = MAXVAL-MINVAL
    norm_arr = ((arr-MINVAL)/RANGE).clip(0,1)
    return norm_arr.astype(np.float16)

def dcm_dir(image_path):
    with open(image_path,'r') as f:
        dcmp = f.read().splitlines()[0]
        return dcmp
        
def get_subsampled(image,rows=256,cols=256):
    size = [rows, cols, image.get_size()[2]]
    spacing = [spc*sz/new_sz for spc, sz, new_sz in zip(image.get_spacing(), image.get_size(), size)]
    template = qimage.new(
        qimage.Type.dummy, (0,0,0), [i-1 for i in size],
        spacing=spacing,
        origin = image.to_physical_coordinates(image.get_min_point()),
        orientation = image.get_orientation()
    )
    ret = image.get_resampled(template)
    return ret

def n4_bias_field_correction(img_arr, ph_area_image_array=None):
    """N4_bias_correction
    https://simpleitk.readthedocs.io/en/master/link_N4BiasFieldCorrection_docs.html
    https://github.com/bigbigbean/N4BiasFieldCorrection/blob/master/N4BiasFieldCorrection.py
    """
    # print("[DEBUG|cnntools.py] N4 bias correction runs.")
    sys.stdout.flush()
    inputImage = sitk.GetImageFromArray(img_arr)
    maskImage = sitk.OtsuThreshold(inputImage,0,1,200)
    inputImage = sitk.Cast(inputImage,sitk.sitkFloat32)
    corrector = sitk.N4BiasFieldCorrectionImageFilter()
    output = corrector.Execute(inputImage,maskImage)
#     log_bias_field = corrector.GetLogBiasFieldAsImage(inputImage)
#     bias_field = inputImage / sitk.Exp(log_bias_field)
    output_arr = sitk.GetArrayFromImage(output)
    # print("[DEBUG|cnntools.py] N4 Bias Field Finished.")
    sys.stdout.flush()
    return output_arr #, output, bias_field

# Returns empty array if norm_descriptor does match a known type of normalization
def intensity_normalization(img_arr, norm_descriptor, ph_area_image_array=None):
    norm_img_arr = img_arr
    nd_split = norm_descriptor.split('_')
    nd_split = [nd.strip() for nd in nd_split]
    # print('[DEBUG|cnntools.py] nd_split', nd_split)
    i = 0
    while i < len(nd_split):    
        # print('[DEBUG|cnntools.py] nd_split[i]', i, nd_split[i])
        if nd_split[i]=='mean0std1':
            # print('[DEBUG|cnntools.py] mean0std1')
            norm_img_arr = norm_mean_std(norm_img_arr, ph_area_image_array)
        elif nd_split[i]=='minmax':
            # print('[DEBUG|cnntools.py] minmax')
            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
        elif nd_split[i]=='histoeq':
            # print('[DEBUG|cnntools.py] histoeq')
                if len(norm_img_arr.shape) > 2:
                    """3D"""
                    # print(f'[DEBUG|cnntools.py] 3D inputs: {norm_img_arr.shape}')
                    try:
                        norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, nbins=1024) \
                            for norm_img_arr_2d in norm_img_arr])
                    except:
                        norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                        norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, nbins=1024) \
                            for norm_img_arr_2d in norm_img_arr])
                else:
                    """2D"""
                    try: norm_img_arr = exposure.equalize_adapthist(norm_img_arr, nbins=1024)
                    except:
                        norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                        norm_img_arr = exposure.equalize_adapthist(norm_img_arr, nbins=1024)
        elif nd_split[i]=='centile':
            # print('[DEBUG|cnntools.py] centile')
            i += 1
            if  i < len(nd_split):
                c0 = float(nd_split[i])
                # print('normalization clip 0 ', c0)
                i += 1
                if i < len(nd_split):
                    c1 = float(nd_split[i])
                    # print('normalization clip 1 ', c1)
                    norm_img_arr = norm_upper_centile(norm_img_arr, ph_area_image_array, c0, c1,)
        elif nd_split[i]=='clahe':
            # print('[DEBUG|cnntools.py] clahe')
            i += 1
            if  i < len(nd_split):
                cl = float(nd_split[i])
                # print('normalization clip limit ', cl)
                i += 1
                if i < len(nd_split):
                    nb = int(nd_split[i])
                    # print('normalization num bins ', nb)
                    if len(norm_img_arr.shape) > 2:
                        """3D"""
                        # print(f'[DEBUG|cnntools.py] 3D inputs: {norm_img_arr.shape}')
                        try:
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                    else:
                        """2D"""
                        try:
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
        elif nd_split[i]=='denoise':
            # print('[DEBUG|cnntools.py] denoise')
            norm_img_arr = anisotropic_diffusion(norm_img_arr, niter=10, kappa=50, gamma=0.1, voxelspacing=None, option=1)
        elif nd_split[i]=='normclahe':
            # print('[DEBUG|cnntools.py] normclahe')
            i += 1
            if i < len(nd_split):
                cl = float(nd_split[i])
                # print('normalization clip limit ', cl)              
                i += 1
                if i < len(nd_split):
                    nb = int(nd_split[i])
                    # print('normalization num bins ', nb)
                    norm_img_arr = norm_mean_std(exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb))
                    if len(norm_img_arr.shape) > 2:
                        """3D"""
                        # print(f'[DEBUG|cnntools.py] 3D inputs: {norm_img_arr.shape}')
                        try:
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                    else:
                        """2D"""
                        try:
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
                    norm_img_arr = norm_mean_std(norm_img_arr)
        elif nd_split[i]=='denoiseclahe':
            # print('[DEBUG|cnntools.py] denoiseclahe')
            i += 1
            if i < len(nd_split):
                cl = float(nd_split[i])
                # print('normalization clip limit ', cl)              
                i += 1
                if i < len(nd_split):
                    nb = int(nd_split[i])
                    # print('normalization num bins ', nb)
                    if len(norm_img_arr.shape) > 2:
                        """3D"""
                        # print(f'[DEBUG|cnntools.py] 3D inputs: {norm_img_arr.shape}')
                        try:
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = np.array([exposure.equalize_adapthist(norm_img_arr_2d, clip_limit=cl, nbins=nb) \
                                for norm_img_arr_2d in norm_img_arr])
                    else:
                        """2D"""
                        try:
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
                        except:
                            norm_img_arr = norm_min_max(norm_img_arr, ph_area_image_array)
                            norm_img_arr = exposure.equalize_adapthist(norm_img_arr, clip_limit=cl, nbins=nb)
                    norm_img_arr = anisotropic_diffusion(norm_img_arr, niter=10, kappa=50, gamma=0.1, voxelspacing=None, option=1)
        elif nd_split[i]=='n4biascorr':
            # print('[DEBUG|cnntools.py] n4biascorr')
            norm_img_arr = n4_bias_field_correction(norm_img_arr, ph_area_image_array)
        elif nd_split[i]=='normgenericCT':
            # print('[DEBUG|cnntools.py] normgenericCT')
            i += 1
            if  i < len(nd_split):
                MINVAL = float(nd_split[i])
                # print('normalization genericCT MINVAL ', MINVAL)              
                i += 1
                if i < len(nd_split):
                    MAXVAL = float(nd_split[i])
                    # print('normalization genericCT MAXVAL ', MAXVAL)
                    norm_img_arr = norm_generic_CT(norm_img_arr, ph_area_image_array, MINVAL=MINVAL, MAXVAL=MAXVAL)
        elif nd_split[i]!='none': #then the syntax is not valid
            # print('[DEBUG|cnntools.py] none')
            i = len(nd_split)
        i += 1
        
    if i==(len(nd_split)+1):
        print('ERROR: failed to parse: ', norm_descriptor)
        sys.stdout.flush()
    # sys.stdout.flush()
    return norm_img_arr
    
def cropped(qimage_obj, prcnt_top, image_size, crop_size):
    img_arr = qimage_obj.get_array()[0]
    img_spacing = qimage_obj.get_spacing()
    x_spacing = img_spacing[0]
    print("x_spacing:", x_spacing)
    y_spacing = img_spacing[1]
    print("y_spacing:", y_spacing)
    orig_rows = image_size[1]
    orig_cols = image_size[0]
    
    h = orig_rows * y_spacing
    w = orig_cols * x_spacing
    print(h, w)
    
    diff_h = h - crop_size
    diff_w = w - crop_size
    if diff_h < 0 or diff_w < 0:
        raise RuntimeError("Crop size to large.")
    
    diff_rows = diff_h / y_spacing
    diff_cols = diff_w / x_spacing
    
    top = round(0 + diff_rows*prcnt_top)
    bottom = round(orig_rows - diff_rows*(1-prcnt_top))
    left = round(0 + diff_cols*0.5)
    right = round(orig_cols - diff_cols*0.5)
    cropped = img_arr[top:bottom, left:right]
    return top, left, cropped


def _get_owner(pid):
    """
    TODO
    ----
    username within condor
    """
    # # the /proc/PID is owned by process creator
    # proc_stat_file = os.stat("/proc/%d" % pid)
    # # get UID via stat call
    # uid = proc_stat_file.st_uid
    # # look up the username from uid
    # username = pwd.getpwuid(uid)[0]
    # username = getpass.getuser()
    username = psutil.Process().username()
    return username

def _get_original_cmd(pid):
    """
    TODO
    ----
    condor job id? or pid enough?
    """
    pid_process = psutil.Process(pid)
    return pid_process.cmdline()

"""Note
ref: https://stackoverflow.com/questions/489861/locking-a-file-in-python

TODO: might require atomic open!!!

`fcntl` for Posix based file locking (Linux, Ubuntu, MacOS, etc.)
Only allows locking on writable files, might cause
strange results for reading.

you create a lock record on the file at filesystem level including process id.
If the process dies or closes any filedescriptor to this file, the lock record gets removed by the system.
simply: fnctl locks work as a Process <--> File relationship, ignoring filedescriptors
(See https://stackoverflow.com/questions/29611352/what-is-the-difference-between-locking-with-fcntl-and-flock)

`msvcrt` for Windows file locking
"""
# try: 
#     import psutil
# except:
#     import subprocess
#     subprocess.check_call([sys.executable, "-m", "pip", "install", "psutil", "--user"])
# try: 
#     import fcntl
# except:
#     try: # Posix based 
#         subprocess.check_call([sys.executable, "-m", "pip", "install", "fcntl", "--user"])
#     except: # Windows
#         subprocess.check_call([sys.executable, "-m", "pip", "install", "msvcrt", "--user"])
try:
    import fcntl
    def lock_file(f):
        if f.writable(): fcntl.lockf(f, fcntl.LOCK_EX | fcntl.LOCK_NB)
    def unlock_file(f):
        if f.writable(): fcntl.lockf(f, fcntl.LOCK_UN)
except ModuleNotFoundError:
    raise ValueError('Not yet implemented for Windows file locking')
    # import msvcrt
    # def file_size(f):
    #     return os.path.getsize( os.path.realpath(f.name) )
    # def lock_file(f):
    #     msvcrt.locking(f.fileno(), msvcrt.LK_RLCK, file_size(f))
    # def unlock_file(f):
    #     msvcrt.locking(f.fileno(), msvcrt.LK_UNLCK, file_size(f))