# segmentation.py
# Wasil's code improvement

import numpy as np
import cv2
from skimage.io import imread, imshow
from skimage.color import rgb2hsv
import matplotlib.pyplot as plt

print("Loading image..")
img = 'SSA002_OD_Scan2_5.png'
scan = plt.imread(img)
print("Convering to hsv")
scan_hsv = rgb2hsv(scan)

# blue segmentation
print("blue mask starting")

lower_mask = scan_hsv[:,:,0] > 0.6
upper_mask = scan_hsv[:,:,0] < 0.7
saturation_mask = scan_hsv[:,:,1] > 0.3
mask = upper_mask*lower_mask*saturation_mask

# just saving the mask
from PIL import Image
new_p = Image.fromarray(mask)
new_p = new_p.convert("L")
new_p.save("blue.jpeg")

red = scan_hsv[:,:,0]*mask
green = scan_hsv[:,:,1]*mask
blue = scan_hsv[:,:,2]*mask
scan_blue = np.dstack((red,green,blue))
imshow(scan_blue)
print("blue mask ending")

# red segmentation
print("red mask starting")
lower_mask1 = scan_hsv[:,:,0] > -2 #or 0.0
upper_mask1 = scan_hsv[:,:,0] < 0.2 #or 0.1 (neither work)
saturation_mask1 = scan_hsv[:,:,1] > 0.1
mask1 = upper_mask1*lower_mask1*saturation_mask1

# just saving the mask
new_p = Image.fromarray(mask1)
new_p = new_p.convert("L")
new_p.save("red.jpeg")