# main_sachi_ver_fix.py

import os
from os import listdir #for iteration
import matplotlib.pyplot as plt
from skimage.io import imread, imshow
from skimage.color import rgb2hsv
import pandas as pd

# for i in count:
# 	open(f"iris_cnn_a ($i)")
#	open("cornea_cnn_a (i)")
# open file command
# 	then use
# 	csv.imread
# would be a lot easier

# how to find the length of the dataset
# iterate through each image in the dataset
# make two datasets, one for iris, one for cornea
# then do the rest of the code

# use entire file path

count = 0
dir_path = 'C:/Users/sachi/Documents/GitHub/AS-OCT/SM_cornea'
for path in os.listdir(dir_path):
	if os.path.isfile(os.path.join(dir_path, path)):
		count += 1
print('File count:', count)

irislist = [files for files in os.listdir("C:/Users/sachi/Documents/GitHub/AS-OCT/SM_iris")]
irislist_dict = { ind: name for (ind, name) in enumerate(irislist) }

cornealist = [files for files in os.listdir("C:/Users/sachi/Documents/GitHub/AS-OCT/SM_cornea")]
cornealist_dict = { ind: name for (ind, name) in enumerate(irislist) }

for image in range(count):
	with open(irislist_dict[image]) as iris:
		scanIris = plt.imread(iris)
	with open(cornealist.dict[image]) as cornea:
		scanCornea = plt.imread(cornea)
	
	print("converting to hsv")
	scanIris_hsv = rgb2hsv(scanIris[...,0:3]) # since the png images are 4 channels 
	scanCornea_hsv = rgb2hsv(scanCornea[...,0:3])
	
	print("separating into halves")
	height, width, _ = scanCornea.shape
	half_width = width // 2
	
	print("making new halves images")
	corneaImg_left_half = imgCornea[:, :half_width]
	corneaImg_right_half = imgCornea[:, half_width:]
	irisImg_left_half = imgIris[:, :half_width]
	irisImg_right_half = imgIris[:, half_width:]