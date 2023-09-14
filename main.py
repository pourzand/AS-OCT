# 72 x
# 358 y 

import os
from os import listdir #for iteration

import numpy as np
import cv2
from skimage.io import imread, imshow
from skimage.color import rgb2hsv
import matplotlib.pyplot as plt #for segmentation

from skimage.segmentation import flood_fill # for region grow

from PIL import Image #for saving image


# print("Loading image..")
# corneaSegmentationImg = "/Users/seena/Downloads/DEFAULT/SSA004_OS_Scan3_11/eval/image/cornea_cnn_a.png"

# scan = plt.imread(corneaSegmentationImg)
# plt.imshow(scan)
# print("Convering to hsv")
# scan_hsv = rgb2hsv(scan[...,0:3]) # since the png images are 4 channels 
# plt.imshow(scan_hsv)


# ---------------------------------
# old flood fill/region grow code 
# img = "/Users/seena/Desktop/OCT_image.png"
# scan = plt.imread(img)
# print("Convering to hsv")
# scan_hsv = rgb2hsv(scan[...,0:3]) # since the png images are 4 channels 
# filledImage = flood_fill(img,[72,358],50)
# imshow(filledImage)

# ---------------------------------

# Load the two images with red segmentations
image1 = cv2.imread('/Users/seena/Downloads/DEFAULT/SSA004_OS_Scan3_11/eval/image/cornea_cnn_a.png')
image2 = cv2.imread('/Users/seena/Downloads/DEFAULT/SSA004_OS_Scan3_11/eval/image/IRIS_a.png')

# Convert the images to the HSV color space
hsv_image1 = cv2.cvtColor(image1, cv2.COLOR_BGR2HSV)
hsv_image2 = cv2.cvtColor(image2, cv2.COLOR_BGR2HSV)

# Define the lower and upper HSV values for the red color
lower_red = np.array([0, 100, 100])
upper_red = np.array([10, 255, 255])

# Create binary masks for the red regions in both images
mask1 = cv2.inRange(hsv_image1, lower_red, upper_red)
mask2 = cv2.inRange(hsv_image2, lower_red, upper_red)

# Find the common regions (intersection) between the two masks
intersection_mask = cv2.bitwise_and(mask1, mask2)

# Find the coordinates (points) where the masks intersect
intersection_points = np.column_stack(np.where(intersection_mask > 0))

# Overlay the points of intersection on the original images
for point in intersection_points:
    cv2.circle(image1, tuple(reversed(point)), 5, (0, 255, 0), -1)
    cv2.circle(image2, tuple(reversed(point)), 5, (0, 255, 0), -1)

# Display the images with intersection points
plt.subplot(121), plt.imshow(cv2.cvtColor(image1, cv2.COLOR_BGR2RGB))
plt.title('Cornea with Intersection Points')
plt.subplot(122), plt.imshow(cv2.cvtColor(image2, cv2.COLOR_BGR2RGB))
plt.title('Iris with Intersection Points')
plt.show()

