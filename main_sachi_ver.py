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

# shift tab to tab backwards

print("Loading image..")

path = "C:/Users/sachi/Pictures/SM_image_database"
iris = "C:/Users/sachi/Pictures/SM_iris"
cornea = "C:/Users/sachi/Pictures/SM_cornea"

im1 = os.listdir(iris)
im2 = os.listdir(cornea)
images = [im1, im2]
print(im1)
'''
def segment(im1: str, im2: str) -> None:
	print("Loading image..")
	img = input_filename
	scan = plt.imread(img)
	
folder_dir = "C:/Users/sachi/Documents/IPILAB/Completed_Averaged Images"
for images in os.listdir(folder_dir):
	# check if the image ends with png
	if (images.endswith(".png")):
		segment(images)
		print(images)
'''


############# INSERT MAIN FUNCTION ############
count = 0
for i in im1:
	corneaImg = cv2.imread(i)
	#irisImg = cv2.imread(im2)
	if i == 1:
		print(corneaImg)

'''
# Define the lower and upper HSV values for the red color
lower_red = np.array([0, 100, 100])
upper_red = np.array([10, 255, 255])

# Get the height and width of the images
height, width, _ = corneaImg.shape # Underscore just let's us ignore the values

# Find half-width to split images along y-axis
half_width = width // 2 # Floor division

################## CORNEA ######################
# Split Image
corneaImg_left_half = corneaImg[:, :half_width]
corneaImg_right_half = corneaImg[:, half_width:]
# Convert to HSV Color Space
hsv_corneaImg_left_half = cv2.cvtColor(corneaImg_left_half, cv2.COLOR_BGR2HSV)
hsv_corneaImg_right_half = cv2.cvtColor(corneaImg_right_half, cv2.COLOR_BGR2HSV)
# Binary Masks
mask1_left_half = cv2.inRange(hsv_corneaImg_left_half, lower_red, upper_red)
mask1_right_half = cv2.inRange(hsv_corneaImg_right_half, lower_red, upper_red)
######## RETURN ORIGINAL HALVES & MASKS ########

################## IRIS ########################
# Split Image
irisImg_left_half = irisImg[:, :half_width]
irisImg_right_half = irisImg[:, half_width:]
# Convert to HSV Color Space
hsv_irisImg_left_half = cv2.cvtColor(irisImg_left_half, cv2.COLOR_BGR2HSV)
hsv_irisImg_right_half = cv2.cvtColor(irisImg_right_half, cv2.COLOR_BGR2HSV)
# Binary Masks
mask2_left_half = cv2.inRange(hsv_irisImg_left_half, lower_red, upper_red)
mask2_right_half = cv2.inRange(hsv_irisImg_right_half, lower_red, upper_red)
######## RETURN ORIGINAL HALVES & MASKS ########

# Find the common regions (intersection) between the left halves
intersection_mask_left = cv2.bitwise_and(mask1_left_half, mask2_left_half)

# Find the common regions (intersection) between the right halves
intersection_mask_right = cv2.bitwise_and(mask1_right_half, mask2_right_half)

# Find the coordinates (points) where the left halves intersect
intersection_points_left = np.column_stack(np.where(intersection_mask_left > 0))

# Find the coordinates (points) where the right halves intersect
intersection_points_right = np.column_stack(np.where(intersection_mask_right > 0))

# Overlay the points of intersection on the original images
for point in intersection_points_left:
	cv2.circle(corneaImg_left_half, tuple(reversed(point)), 5, (0, 255, 0), -1)
	cv2.circle(irisImg_left_half, tuple(reversed(point)), 5, (0, 255, 0), -1)

for point in intersection_points_right:
	cv2.circle(corneaImg_right_half, tuple(reversed(point)), 5, (0, 255, 0), -1)
	cv2.circle(irisImg_right_half, tuple(reversed(point)), 5, (0, 255, 0), -1)

box_size = (150, 150)  # Adjust the size of the bounding box as needed

# Determine the bottom leftmost point for the left half
if len(intersection_points_left) > 0:
	bottom_leftmost_point_left = tuple(intersection_points_left[np.argmax(intersection_points_left[:, 0])])
	cv2.circle(corneaImg_left_half, tuple(reversed(bottom_leftmost_point_left)), 5, (255, 0, 0), -1)
	cv2.circle(irisImg_left_half, tuple(reversed(bottom_leftmost_point_left)), 5, (255, 0, 0), -1)

	top_left = (bottom_leftmost_point_left[1] - box_size[1] // 2, bottom_leftmost_point_left[0] - box_size[0] // 2)
	bottom_right = (bottom_leftmost_point_left[1] + box_size[1] // 2, bottom_leftmost_point_left[0] + box_size[0] // 2)

	# Draw the rectangular bounding box in blue for rn
	cv2.rectangle(corneaImg_left_half, top_left, bottom_right, (255, 0, 0), 2)  # (0, 0, 255) for red, 2 for thickness
	cv2.rectangle(irisImg_left_half, top_left, bottom_right, (255, 0, 0), 2)  # (0, 0, 255) for red, 2 for thickness

else:
	print("No intersection points found in the left half.")

# Determine the bottom rightmost point for the right half
if len(intersection_points_right) > 0:
	bottom_rightmost_point_right = tuple(intersection_points_right[np.argmax(intersection_points_right[:, 0])])
	cv2.circle(corneaImg_right_half, tuple(reversed(bottom_rightmost_point_right)), 5, (255, 0, 0), -1)
	cv2.circle(irisImg_right_half, tuple(reversed(bottom_rightmost_point_right)), 5, (255, 0, 0), -1)


	top_left = (bottom_rightmost_point_right[1] - box_size[1] // 2, bottom_rightmost_point_right[0] - box_size[0] // 2)
	bottom_right = (bottom_rightmost_point_right[1] + box_size[1] // 2, bottom_rightmost_point_right[0] + box_size[0] // 2)

	# Draw the rectangular bounding box in blue for rn
	cv2.rectangle(corneaImg_right_half, top_left, bottom_right, (255, 0, 0), 2)  # (0, 0, 255) for red, 2 for thickness
	cv2.rectangle(irisImg_right_half, top_left, bottom_right, (255, 0, 0), 2)  # (0, 0, 255) for red, 2 for thickness

else:
	print("No intersection points found in the right half.")
	count = count + 1


plt.figure(figsize=(12, 12), dpi=200)

# Display the images with intersection points for the left and right halves
plt.subplot(221), plt.imshow(cv2.cvtColor(corneaImg_left_half, cv2.COLOR_BGR2RGB))
plt.title('Cornea Left Half ')
plt.subplot(222), plt.imshow(cv2.cvtColor(irisImg_left_half, cv2.COLOR_BGR2RGB) )
plt.title('Iris Left Half')
plt.subplot(223), plt.imshow(cv2.cvtColor(corneaImg_right_half, cv2.COLOR_BGR2RGB) )
plt.title('Cornea Right Half')
plt.subplot(224), plt.imshow(cv2.cvtColor(irisImg_right_half, cv2.COLOR_BGR2RGB) )
plt.title('Iris Right Half')

plt.tight_layout()

plt.show()

print(count)

################# ITERATE AGAIN ####################

# Anshu and Trent suggested to use the furthest point on the cornea to find intersection if the iris and cornea do not overlap. 
# For this code, we simply count when there isn't an intersection to test how many images this code can run on.
'''
