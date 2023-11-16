#iteratorTest

import os
from os import listdir #for iteration
import pathlib

import numpy as np
import cv2
from skimage.io import imread, imshow
from skimage.color import rgb2hsv
import matplotlib.pyplot as plt #for segmentation

from skimage.segmentation import flood_fill # for region grow

from PIL import Image #for saving image

from pathlib import Path
from osgeo import gdal

def bound(s2: str, s1: str) -> None:
    print("Function start...")
    iris = s1
    cornea = s2
    print(iris) #these are arrays
    print(cornea)
    print("function end")

for cornea_file in Path("C:/Users/sachi/Pictures/SM_results").glob("cornea_cnn_a (*).png"):
    print("for loop start")
    s2 = cornea_file.as_posix()
    s1 = cornea.replace("cornea", "iris")
    if not (Path("C:/Users/sachi/Pictures/SM_results")/s1).exists():
       print(f"Cannot find before file for {after_file}")
    print("for loop end")
    