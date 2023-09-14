/* An extension of the standard UCLA PACS V5 format for XA and RF	*/
/* Version V5b, release 4 of August 7th, 1997				*/

#ifndef _UCLA_V5B_H_
#define _UCLA_V5B_H_

struct	PACS_HEADER  		/*  : UCLA PACS Header */
{

/*	--- header version --- */

  char	file_type[4];			/* header version [DI,RI,VI,V4 ...] */
					/* For UCLA V5b it is "V5b"	*/
  
  /*	--- image format --- */	
  
  long	img_dim;			/* image dimension [1,2,3,4]	*/
  long	img_rows;			/* num. x-pixels [256,512,1024,...] */
  long	img_cols;			/* num. y-pixels [256,512,1024,...] */
  long	no_img_in_file;			/* num. images in file.  eg. 23	*/
  
  
  /*	--- image storage --- */
  
  long	pix_bit_allocated;		/* e.g. 12 	*/
  long	pix_bit_stored;			/* e.g. 16	*/
  long	pix_bit_high_pos;		/* e.g. 12 (for Sun - 12th bit)	*/
  long	compr_flag;			/* 0 = none, 1=compression	*/
  char	compr_version[8];		/* e.g. FFBA_1, Huffman, ...	*/
  
  
  /*	--- voxel size	--- */

  float	x_pix_size;			/* in mm 	*/
  float	y_pix_size;			/* in mm 	*/
  float	study_slice_thickness;		/* in mm 	*/
  float	study_slice_spacing;		/* in mm 	*/
  char	field_of_view[16];		/* X mm x Y mm */
    
    
    /*	--- pointers to subheaders --- */
    
  long	subhdr_blk_size;		/* eg. 512 bytes		*/
  long	subhdr_blk_position;		/* start block of header	*/
  
  
  /*	--- acquisition machine references --- */

  char	rid_path[40];			/* path name on capture computer			*/
  char	rid_filename[48];		/* image name on acquisition machine		*/
  char	exam_reference_no[20];		/* exam reference number used by RID DBMS 	*/
  char	study_reference_no[20];		/* study reference number used by RID DBMS 	*/
  char	series_reference_no[20];	/* series reference number used by RID DBMS	*/

  
  /*	--- RIS and PACS study reference ID	--- */
  
  char	RIS_study_no[12];		/* study reference id used by RIS DBMS		*/
  char	PACS_study_no[12];		/* patient study number referenced by PACS	*/

  
  /*	--- patient information --- */
  
  char	pat_id[12];			/* 123-45-67	*/
  char	pat_name[48];			/* last, first	*/
  char	pat_birth_date[12];		/* 850930 	*/
  char	pat_sex[4];			/* M, F, O	*/
  char	pat_history[80];		/* pat presented w/ rhabnosarcoma stage 3	*/
  char	pat_status[4];			/* in, out, er, ref, unk 	*/

  
  /*	--- study information ---    */
  
  char	reason_for_request[80];		/* r/o pneumonia 	*/
  char	study_description[80];		/* ap hand x-ray (contains anatomy info)	*/
  char	study_subdescription[80];	/* bone age 	*/
  char	study_acq_date[12];		/* 900830	*/
  char	study_acq_time[12];		/* 143300	*/
  char	study_radiologist[40];		/* BOECHAT	*/
  char	study_ref_physician[40];	/* BRILL	*/
  char	study_modality[4];		/* CR,MR,CT,XA,...	*/
  char	study_misc_info[48];		
  char  pat_preparation[32];		/* contrast / sedation */
  
  /*	--- imaging device information ---    */
  
  char	inst_station_id[24];		/* e.g. pcr1.ped1		*/
  char	inst_acq_manuf[24];		/* e.g. PHILIPS			*/
  char	inst_acq_model[24];		/* e.g. PCR-901 UNIT A		*/
  char	inst_id[32];			/* UCLA MED PLAZA, MED CTR	*/
  
  /* === XA add-ons ===			*/
  
  char	study_id_number[8];		/* XA Sequence number		*/
  float img_fps;			/* XA Frame rate (fps)		*/

  int	img_plane_type;			/* XA Image Plane Type:  	*/
					/* 0 for a single-plane scan,	*/
					/* 1 for a frontal plane of a 	*/
					/*       biplanar study,	*/
					/* 2 for a lateral plane of a	*/
					/*       biplanar study.	*/

  int	img_table_motion;		/* XA table motion: 0 for none,	*/
					/* 1 for yes			*/

  int	img_positioner_motion;		/* XA Positioner arm motion: 	*/
					/* 0 for none, 1 for yes	*/
  float img_prim_angle;			/* XA Positioner primary angle	*/
					/* (in equatorial plane)	*/
  float img_sec_angle;			/* XA Positioner arm secondary	*/
					/* angle (in sagittal plane)	*/

  char	field_of_view_shape[10];	/* ROUND/RECTANGLE		*/
  

  /*	--- Curve Presence Flag ---	*/
  
  int	num_curves_enclosed;		/* Number of curves in the file	*/

  /*	--- extra filler space ---	*/
  
  char	free_text[28];			/* for future use		*/
};



struct	PACS_SUBHEADER_MR  		/*  : UCLA PACS MR Sub-Header */
{
  
  /*	--- source -> patient -> detector geometry --- */
  
  char	mr_pat_orientation[32];	/* Suspine, Prone, Decubitus Lt, Decubitus Rt	*/
  
  
  /*	--- sequence view ---	*/
  
  char	mr_plane_name[16];		/* axial, sagittal, coronal, 	*/
					/* oblique			*/
  char	mr_position[16];		/* 0 = head first, 1 = feet first*/
  char	mr_longit_ref[32];		/* Supra Obital, Orbital Meatus, etc.. 	*/
  char	mr_vertic_ref[32];		/* Supra Obital, Infra Orbital, etc.. 	*/
  float	mr_start_x;			/* min rt position w/r to ref pt.	*/
  float	mr_end_x;			/* max right position "		*/
  float	mr_start_y;			/* min anterior view  "		*/
  float	mr_end_y;			/* max anterior view  "		*/
  float	mr_start_z;			/* min superior view  "		*/
  float	mr_end_z;			/* max superior view  "		*/
  float	mr_image_location;		/* image location relative to landmark	*/
  
  
  /*	--- imaging technique	---	*/
  
  char	mr_pulse_seq_name[24];		/* Spin Echo, Inv Rec, Mult Echo, ...	*/
  char	mr_gating_name[8];	/* 0=none, 1=ECG, 2=RESP, 3=ECG/R, 4=RCOMP	*/
  
  
  /*	--- source characteristics ---	 */
  
  long	mr_gauss;			/* magnetic field strength		*/
  char	mr_coil_name[8];		/* 0=head, 1=body, 2=surface		*/
  
  
  /*	--- pulse sequence technique	--- */
  
  float	mr_repitition_recovery_time;	/* Time of recovery, microsec.	*/
  float	mr_scan_time;			/* Active scan time		*/
  float	mr_echo_delay;			/* Echo delay, microseconds	*/
  float	mr_inversion_time;		/* Inversion recovery time,  	*/
					/* microseconds			*/
  long	mr_no_echos;			/* Number of echos		*/
  float	mr_no_excitations;		/* Number of excitations	*/
  long	mr_flip_angle;			/* Flip angle prescribed for 	*/
					/* GRASS			*/

  char	mr_traverse_time[4] ;		/*	T1/T2	*/


  
  /*	--- display parameters --- */
  
  long	mr_dflt_window;			/* recomm default windows 	*/
  long	mr_dflt_level;
  
  
  /*	--- Max/Min gray level in study  ---	*/
  
  long	study_min;			/* minimum z-value in study	*/
  long	study_max;			/* maximum z-value in study	*/
  
  unsigned char	free_text[264] ;
};



struct	PACS_SUBHEADER_CT  		/*  : UCLA PACS CT Sub-Header 	*/
{
  
  unsigned char ct_series_name[20];     /* series type 			*/


  /* --- source characteristics	--- 	*/
  
  unsigned char ct_expo_time[8];	/* time of exposure in msec	*/
  unsigned char ct_expo_rate[8];	/* rate of x-ray exposure	*/
  unsigned char ct_kvp[8];		/* tube voltage in Kvp		*/
  unsigned char ct_scan_seq[20];	/* scan sequence		*/

  
  /*	--- source -> patient -> detector geometry --- */
  
  unsigned char ct_pat_orient[12];	/* patient orientation		*/
  unsigned char ct_src_det_dist[16];	/* tube to detector distance	*/
  unsigned char ct_src_pat_dist[16];	/* tube to patient distance	*/

  
  /*	--- series view ---	*/
  
  unsigned char ct_posit_ref[8];
  unsigned char ct_slice_loc[16];

  
  /* --- reconstruction parameters ---	*/
  
  unsigned char ct_recon_diam[8];	/* field of view, reconst diam	*/
  unsigned char ct_conv_kern[16];	/* smoothing kernel used	*/

  
  /* --- calibration parameters	---	*/
  
  unsigned char ct_rescal_inter[8];
  unsigned char ct_rescal_slope[8];
  unsigned char ct_gray_scale[4];
  
  
  /*	--- display parameters --- */
  
  long	ct_dflt_level;			/* recomm default windows */
  long	ct_dflt_window;
  
  
  /*	--- Max/Min gray level in study  ---	*/
  
  long	study_min;			/* minimum z-value in study	*/
  long	study_max;			/* maximum z-value in study	*/
  
  unsigned char	free_text[320];
};



struct	PACS_SUBHEADER_CR  		/*  : UCLA PACS CR Sub-Header */
{

  /* --- source characteristics 	---	*/

  char	cr_sensitivity[12];		/* CR S-value (HEX)	*/
  char	cr_range[12];			/* CR R-value (HEX)	*/


  /* --- detector_characteristics --- */
  
  char	cr_pre_sample_mode[16];		/* fixed, auto, senstivity mode, etc. 	*/ 


  /* --- image pre-processing parameters --- */
  char	preproc_parameters[64];		/* gamma lut's etc 		*/

  
  /*	--- display parameters --- */
  
  long	disp_win_center;		/* recomm default windows */
  long	disp_win_width;
  
  
  /*	--- Max/Min gray level in study  ---	*/
  
  long	study_min;			/* minimum z-value in study	*/
  long	study_max;			/* maximum z-value in study	*/

  
  unsigned char	free_text[392];

};



struct	PACS_SUBHEADER_XA  		/*  UCLA PACS XA Sub-Header */
{

  int 	xa_frame_number;		/* Frame number			*/

  /*  ---   Frame rate info	---	*/

  float xa_frame_interval;		/* Time interval between the 	*/
					/* previous frame and this one.	*/
					/* Expressed in msec.		*/

  /*  ---   XA Positioner info	---	*/

  float xa_prim_angle_increment;	/* Increment of XA Positioner 	*/
					/* primary angle for this frame	*/
  float xa_sec_angle_increment;		/* Increment of XA Positioner 	*/
					/* secondary angle - this frame	*/
  float xa_current_prim_angle;		/* Current value of Positioner	*/
					/* primary angle		*/
  float xa_current_sec_angle;		/* Current value of Positioner	*/
					/* secondary angle		*/
  float xa_source_to_patient;		/* Distance source to patient	*/
  float xa_source_to_detector;		/* Distance source to detector	*/

  /* ---    XA Table info	---	*/

  float xa_table_vertic_increment;	/* Vertical movement of the XA	*/
					/* table for this frame		*/
  float xa_table_lateral_increment;	/* Lateral movement of the XA	*/
					/* table for this frame		*/
  float xa_table_longit_increment;	/* Longitunal movement of the 	*/
					/* XA table for this frame	*/
   
  /*  ---   X-ray information	---	*/

  float xa_expo_kvp;			/* X-ray tube voltage in KVp	*/
  float xa_expo_current;		/* X-ray tube current in mA	*/
  float xa_expo_time;			/* exposure time, msec		*/
  float xa_expo_rate;			/* rate of X-ray exposure	*/

  /*    --- Extra space	 --- 	*/

  unsigned char free_text[452];
};



/* 
 * This is an appendix to the UCLA PACS header definitions.
 * PACS_SUBHEADER_IMG is an ad-hoc mechanism to pass some
 * image-specific information. This was devised primarily
 * to maintain parameters of the image that may be different
 * from that indicated in the main header. (GJ 09/22/93)
 */

struct	PACS_SUBHEADER_IMG  		/*  : UCLA PACS Image Sub-Header */
{

  int   img_number;         /* number of the image in the series */
  long	img_rows;			/* no x-pixels [64,128,256,512,1024,2048, ...]	*/
  long	img_cols;			/* no y-pixels [64,128,256,512,1024,2048, ...]	*/
  float	x_pix_size;			/* in mm 	*/
  float	y_pix_size;			/* in mm 	*/
  char	field_of_view[16];		/* X mm x Y mm */
  long	img_min;			/* minimum z-value of image */
  long	img_max;			/* maximum z-value of image */
  int   recon_algorithm;    /* reconstruction algorithm */
  float thick;              /* added by Hing on 6/23/95 @ the request of  */
                            /*   Mike McNitt-Gray */ 
};


/*
 * This is another appendix to the UCLA PACS header definitions.
 * Hwa Kho 08/18/95 
 */

struct PACS_SUBHEADER_IMGX
{
  char	version[8];		/* version name */
  char	window_level[16];	/* window level */
  char  window_width[16];	/* window width */
  char  mr_repetition_time[16]; /* TR value     */
  char  mr_echo_time[16];       /* TE value     */
};

#endif
