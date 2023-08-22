# NEWsegmenter.py

import os
from os import listdir #for iteration

import numpy as np
import cv2
from skimage.io import imread, imshow
from skimage.color import rgb2hsv
import matplotlib.pyplot as plt #for segmentation

from PIL import Image #for saving image

def segment(input_filename: str) -> None:
	
	print("Loading image..")
	img = input_filename
	scan = plt.imread(img)
	print("Convering to hsv")
	scan_hsv = rgb2hsv(scan[...,0:3]) # since the png images are 4 channels 

	# blue segmentation
	print("blue mask starting")

	lower_mask = scan_hsv[:,:,0] > 0.6
	upper_mask = scan_hsv[:,:,0] < 0.7
	saturation_mask = scan_hsv[:,:,1] > 0.3
	mask = upper_mask*lower_mask*saturation_mask

	red = scan_hsv[:,:,0]*mask
	green = scan_hsv[:,:,1]*mask
	blue = scan_hsv[:,:,2]*mask
	scan_blue = np.dstack((red,green,blue))
	imshow(scan_blue)
	print("blue mask ending")

	# red segmentation
	print("red mask starting")
	lower_mask1 = scan_hsv[:,:,0] > -2 
	upper_mask1 = scan_hsv[:,:,0] < 0.3 
	saturation_mask1 = scan_hsv[:,:,1] > 0.1
	mask1 = upper_mask1*lower_mask1*saturation_mask1

	# just saving the mask
	blue_p = Image.fromarray(mask)
	blue_p = blue_p.convert("L")
	blue_p.save(input_filename + "blue.png")

	red_p = Image.fromarray(mask1)
	red_p = red_p.convert("L")
	red_p.save(input_filename + "red.png")
	
# iteration 
folder_dir = "C:/Users/sachi/Documents/IPILAB/Completed_Averaged Images"
for images in os.listdir(folder_dir):
	# check if the image ends with png
	if (images.endswith(".png")):
		
		segment(images)