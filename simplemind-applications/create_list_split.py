import random

### Before running this script, I renamed the image directory to "images", and segmentation directory to "segmentation"
image_dir_name = "AveragedImages"       ### Update this!!!
seg_dir_name = "segmentations"   ### Update this!!!

# example_img="/radraid/apps/personal/wasil/USC/oct/images/SSA_003_OD_Scan3_1.pngout_0000.nii.gz"
# example_seg="/radraid/apps/personal/wasil/USC/oct/segmentation/SSA_003_OD_Scan3_1.pngblue.pngout_0000.nii.gz"

import glob, os, pandas as pd
#base_dir = "~/simplemind-applications/oct_quick_start" ### Update this!!!
base_dir = "/home/ipilab/simplemind-applications/oct_quick_start"
img_search=os.path.join(base_dir, image_dir_name, "*.jpegout_0000.nii.gz") ### change "images" if you named it differently


images = glob.glob(img_search)

blue_data_cnn_train_list = []
blue_data_apt_train_list = []
red_data_cnn_train_list = []
red_data_apt_train_list = []
oct_data_apt_train_list = []
for image_path in images:
    seg_id = os.path.basename(image_path).split(".")[0]
    
    if random.uniform(0,1) < 0.7:
    	put_in_training = True
    else:
    	put_in_training = False
    
    if put_in_training == True:
	    #################### Training set #########################
	    ### blue = cornea
	    task_id = "cornea"
	    seg_path = os.path.join(os.path.dirname(os.path.dirname(image_path)), seg_dir_name, "{}.pngblue.pngout_0000.nii.gz".format(seg_id))
	    if os.path.exists(seg_path):
	    	print(seg_id)
	    	print(seg_path)

	    	blue_data_cnn_train_list.append(list([image_path,seg_path]))
	    	blue_data_apt_train_list.append(list([seg_id,task_id,image_path,seg_path]))


	    ### red = iris
	    task_id = "iris"
	    seg_path = os.path.join(os.path.dirname(os.path.dirname(image_path)), seg_dir_name, "{}.pngred.pngout_0000.nii.gz".format(seg_id))
	    if os.path.exists(seg_path):
	    	print(seg_path)

	    	red_data_cnn_train_list.append(list([image_path,seg_path]))
	    	red_data_apt_train_list.append(list([seg_id,task_id,image_path,seg_path]))

    #################### Testing set #########################
    else:
	    task_id = "oct"
	    oct_data_apt_train_list.append(list([seg_id,task_id,image_path,os.path.dirname(seg_path)]))
    

blue_cnn_train_df = pd.DataFrame(blue_data_cnn_train_list, columns=['image','ref_roi'])
blue_apt_train_df = pd.DataFrame(blue_data_apt_train_list, columns=['id','dataset','image_file','reference'])
red_cnn_train_df = pd.DataFrame(red_data_cnn_train_list, columns=['image','ref_roi'])
red_apt_train_df = pd.DataFrame(red_data_apt_train_list, columns=['id','dataset','image_file','reference'])
oct_apt_train_df = pd.DataFrame(oct_data_apt_train_list, columns=['id','dataset','image_file','reference'])


blue_cnn_train_df.to_csv(os.path.join(base_dir, "cornea_cnn_train.csv"))
blue_apt_train_df.to_csv(os.path.join(base_dir, "cornea_apt_train.csv")) ### this isn't used for now
red_cnn_train_df.to_csv(os.path.join(base_dir, "iris_cnn_train.csv"))
red_apt_train_df.to_csv(os.path.join(base_dir, "iris_apt_train.csv")) ### this isn't used for now
oct_apt_train_df.to_csv(os.path.join(base_dir, "oct_apt_train.csv"))







