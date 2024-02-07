dict_output = {'pixdim[1]':1,
                'pixdim[2]':1,
                'pixdim[3]':1,
                'qoffset_x':0,
                'qoffset_y':0,
                'qoffset_z':0}
def my_dice(img1,img2):
 intersection = np.logical_and(img1, img2)
 union = np.logical_or(img1, img2)
 dice = (2*np.sum(intersection))/(np.sum(union)+np.sum(intersection))
 return dice
