### Before running this script, I renamed the image directory to "images", and segmentation directory to "segmentation"
image_dir_name = "images"       ### Update this!!!
seg_dir_name = "labels"   ### Update this!!!

# example_img="/radraid/apps/personal/wasil/USC/oct/images/SSA_003_OD_Scan3_1.pngout_0000.nii.gz"
# example_seg="/radraid/apps/personal/wasil/USC/oct/segmentation/SSA_003_OD_Scan3_1.pngblue.pngout_0000.nii.gz"

import glob, os, pandas as pd
#base_dir = "~/simplemind-applications/oct_quick_start" ### Update this!!!
base_dir = "/home/ipilab/simplemind-applications/fundus_quick_start"
img_search=os.path.join(base_dir, image_dir_name, "*.pngout_0000.nii.gz") ### change "images" if you named it differently


images = glob.glob(img_search)

blue_data_cnn_train_list = []
blue_data_apt_train_list = []
red_data_cnn_train_list = []
red_data_apt_train_list = []
oct_data_apt_train_list = []
for image_path in images:
    seg_id = os.path.basename(image_path).split(".")[0]


    ### 
    task_id = "optic_disk"
    seg_path = os.path.join(os.path.dirname(os.path.dirname(image_path)), seg_dir_name, "{}avg_ann.pngout_0000.nii.gz".format(seg_id))
    if os.path.exists(seg_path):
    	print(seg_id)
    	print(seg_path)

    	blue_data_cnn_train_list.append(list([image_path,seg_path]))
    	blue_data_apt_train_list.append(list([seg_id,task_id,image_path,seg_path]))


    task_id = "fundus"
    oct_data_apt_train_list.append(list([seg_id,task_id,image_path,os.path.dirname(seg_path)]))
    

blue_cnn_train_df = pd.DataFrame(blue_data_cnn_train_list, columns=['image','ref_roi'])
blue_apt_train_df = pd.DataFrame(blue_data_apt_train_list, columns=['id','dataset','image_file','reference'])
oct_apt_train_df = pd.DataFrame(oct_data_apt_train_list, columns=['id','dataset','image_file','reference'])


blue_cnn_train_df.to_csv(os.path.join(base_dir, "optic_disk_cnn_train.csv"))
blue_apt_train_df.to_csv(os.path.join(base_dir, "optic_disk_apt_train.csv")) ### this isn't used for now
oct_apt_train_df.to_csv(os.path.join(base_dir, "fundus_apt_train.csv"))
