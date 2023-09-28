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

#PUT YOUR LOCAL PATH HERE
corneaImg = cv2.imread('/Users/seena/Downloads/DEFAULT/SSA004_OS_Scan3_11/eval/image/cornea_cnn_a.png')
IrisImg = cv2.imread('/Users/seena/Downloads/DEFAULT/SSA004_OS_Scan3_11/eval/image/IRIS_a.png')

# Get the height and width of the images
height, width, _ = corneaImg.shape # Underscore just let's us ignore the values

# Split the images into two halves along the y-axis
half_width = width // 2 # Floor division
corneaImg_left_half = corneaImg[:, :half_width]
corneaImg_right_half = corneaImg[:, half_width:]
IrisImg_left_half = IrisImg[:, :half_width]
IrisImg_right_half = IrisImg[:, half_width:]

# Convert the images to the HSV color space
hsv_corneaImg_left_half = cv2.cvtColor(corneaImg_left_half, cv2.COLOR_BGR2HSV)
hsv_corneaImg_right_half = cv2.cvtColor(corneaImg_right_half, cv2.COLOR_BGR2HSV)
hsv_IrisImg_left_half = cv2.cvtColor(IrisImg_left_half, cv2.COLOR_BGR2HSV)
hsv_IrisImg_right_half = cv2.cvtColor(IrisImg_right_half, cv2.COLOR_BGR2HSV)

# Define the lower and upper HSV values for the red color
lower_red = np.array([0, 100, 100])
upper_red = np.array([10, 255, 255])

# Create binary masks for the red regions in both left halves
mask1_left_half = cv2.inRange(hsv_corneaImg_left_half, lower_red, upper_red)
mask2_left_half = cv2.inRange(hsv_IrisImg_left_half, lower_red, upper_red)

# Create binary masks for the red regions in both right halves
mask1_right_half = cv2.inRange(hsv_corneaImg_right_half, lower_red, upper_red)
mask2_right_half = cv2.inRange(hsv_IrisImg_right_half, lower_red, upper_red)

# Find the common regions (intersection) between the left halves
intersection_mask_left = cv2.bitwise_and(mask1_left_half, mask2_left_half)

# Find the common regions (intersection) between the right halves
intersection_mask_right = cv2.bitwise_and(mask1_right_half, mask2_right_half)

# Find the coordinates (points) where the left halves intersect
intersection_points_left = np.column_stack(np.where(intersection_mask_left > 0))

# Find the coordinates (points) where the right halves intersect
intersection_points_right = np.column_stack(np.where(intersection_mask_right > 0))

intersectionArr = [intersection_points_left,intersection_mask_right]

# for i in range(2):
#     print("intersectionArr shape")
#     print(intersectionArr[i].shape)
#     # print("intersectionArr contents")
#     # print(intersectionArr[i][0][:])
#     # print("intersectionArr sum")
#     # print(sum(intersectionArr[i][0][:]))

#     if(sum(intersectionArr[i][0][:]) == 0):
        
#         # meaning no intersection found, we must approximate
#         # Find the closest points between the two segmentations within the same half
#         if(i == 0): # means left half
#             distances = np.linalg.norm(mask1_left_half[:, np.newaxis] - mask2_left_half, axis=2)
#         else: 
#             distances = np.linalg.norm(mask1_right_half[:, np.newaxis] - mask2_right_half, axis=2)
        
#         print("distances shape")
#         print(distances.shape)

#         min_indices = np.unravel_index(np.argmin(distances), distances.shape)
#         closest_point_left_half = intersection_points_left[min_indices[0]]
#         closest_point_right_half = intersection_points_right[min_indices[1]]
#         midpoint_left_half = ((closest_point_left_half[0] + closest_point_right_half[0]) // 2,
#                             (closest_point_left_half[1] + closest_point_right_half[1]) // 2)

#         # Define the size of the region to approximate (e.g., a small rectangle)
##         region_size = (20, 20)  # You can adjust the size as needed

#         # Create a mask for the region around the midpoint
##         region_mask_left = np.zeros_like(mask1_left_half)
##         region_mask_left[
#             midpoint_left_half[0] - region_size[0] // 2:midpoint_left_half[0] + region_size[0] // 2,
#             midpoint_left_half[1] - region_size[1] // 2:midpoint_left_half[1] + region_size[1] // 2
#         ] = 255

#         # Overlay the region mask on the left half of image1
#         image1_left_half_with_region = cv2.bitwise_and(image1_left_half, image1_left_half, mask=region_mask_left)

# Overlay the points of intersection on the original images
for point in intersection_points_left:
    cv2.circle(corneaImg_left_half, tuple(reversed(point)), 5, (0, 255, 0), -1)
    cv2.circle(IrisImg_left_half, tuple(reversed(point)), 5, (0, 255, 0), -1)

for point in intersection_points_right:
    cv2.circle(corneaImg_right_half, tuple(reversed(point)), 5, (0, 255, 0), -1)
    cv2.circle(IrisImg_right_half, tuple(reversed(point)), 5, (0, 255, 0), -1)


plt.figure(figsize=(12, 12), dpi=200)

# Display the images with intersection points for the left and right halves
plt.subplot(221), plt.imshow(cv2.cvtColor(corneaImg_left_half, cv2.COLOR_BGR2RGB))
plt.title('Cornea Left Half ')
plt.subplot(222), plt.imshow(cv2.cvtColor(IrisImg_left_half, cv2.COLOR_BGR2RGB) )
plt.title('Iris Left Half')
plt.subplot(223), plt.imshow(cv2.cvtColor(corneaImg_right_half, cv2.COLOR_BGR2RGB) )
plt.title('Cornea Right Half')
plt.subplot(224), plt.imshow(cv2.cvtColor(IrisImg_right_half, cv2.COLOR_BGR2RGB) )
plt.title('Iris Right Half')

plt.tight_layout()

plt.show()



# anshu and brent suggest that I use the furthest aka last point of intersection
# so for the left half i choose the left most point of overlap.
