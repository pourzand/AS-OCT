#include "ModelKS.h"
#include <pcl/misc/FileNameTokenizer.h>

float ModelMapperS(Blackboard& bb)
{
	float score=0.0;
	if ((bb.num_sol_elements()==0) && (bb.num_act_recs()==0))
		score = 0.9;
	return score;
}

/**
@name Features
@memo Vocabulary terms which are mapped to Feature Attributes.
@memo Add _B to the end of a term name to create a bi-directional relationship.
*/
//@{
/**
@name AnteriorTo_PlanarCentroid_mm
@memo AnteriorTo_PlanarCentroid_mm rel_entity_name [fuzzy]. Expected distance anteriorally (mm), based on the y-coordinates of centroids of the entity and the related entity. The centroid of the related entity is computed using only pixels in the image plane (slice) of the centroid of the entity in question.
*/
/**
@name Area_AllXYplanes_cm2
@memo Area_AllXYplanes_cm2 [fuzzy]. Expected total area in xy-plane(s) specified in cm2.
*/
/**
@name Area_AllXYplanes_mm2
@memo Area_AllXYplanes_mm2 [fuzzy]. Expected total area in xy-plane(s) specified in mm2.
*/
/**
@name Area_PerXYplane_cm2
@memo Area_PerXYplane_cm2 [fuzzy]. Area per slice. Expected total area in xy-plane(s) divided by the number of slices between the first and last slice in which the object appears. Units are cm2.
*/
/**
@name Area_MaxXYplane_mm2
@memo Area_MaxXYplane_mm2 [fuzzy]. Maximum area in a single slice. Expected maximum area in a single xy-plane. Units are mm2. Fuzzy membership can use genetic algorithm.
*/
/**
@name Area_PerXYplane_mm2
@memo Area_PerXYplane_mm2 [fuzzy]. Area per slice. Expected total area in xy-plane(s) divided by the number of slices between the first and last slice in which the object appears. Units are mm2.
*/
/**
@name Circularity_AtMaxDiameter_perc
@memo Circularity_AtMaxDiameter_perc [fuzzy]. Percentage of area of bounding circle occupied by region. Circularity = 100*(area of region)/(area of bounding circle). Circularity is computed on the plane where the region has maximum diameter. Radius of bounding circle is the maximum distance (in mm) from planar-centroid to any point in the region (in the plane).
*/
/**
@name Circularity_MinAllXYplanes_perc
@memo Circularity_MinAllXYplanes_perc [fuzzy]. Percentage of area of bounding circle occupied by region. Circularity = 100*(area of region)/(area of bounding circle). Circularity is computed on a plane-by-plane basis, with the final result being the minimum circularity from any plane. Radius of bounding circle is the maximum distance (in mm) from planar-centroid to any point in the region (in the current plane). Fuzzy membership can be optimized by GA.
*/
/**
@name DistanceFrom_Centroid_mm
@memo DistanceFrom_Centroid_mm rel_entity_name [fuzzy]. Expected distance (3D) between the coordinates of centroids of the entity and the related entity expressed in mm.
*/
/**
@name InContactWith
@memo InContactWith rel_entity_name. Gives a confidence of 1.0 if the entity in question is in contact with the related entity, confidence of 0 otherwise. Contact is determined in 3D. Two ROIs are in contact if they overlap or if they have one or more voxels that are adjacent by 26-point connectivity.
*/
/**
@name InContactWith_2D
@memo InContactWith_2D rel_entity_name. Gives a confidence of 1.0 if the entity in question is in contact with the related entity, confidence of 0 otherwise. Contact is determined in 2D. Two ROIs are in contact if they overlap or if they have one or more voxels that are adjacent by 8-point connectivity.
*/
/**
@name InferiorToTip_Centroid_mm
@memo InferiorToTip_Centroid_mm rel_entity_name [fuzzy]. Expected distance (inferiorly using z-coordindates) between the coordinates of centroids of the entity and the related entity expressed in mm. A negative argument indicates a centroid superior to the inferior tip of the related entity.
*/
/**
@name Inside_Centroid
@memo Inside_Centroid rel_entity_name. Gives confidence of 1 if the centroid of the entity in question is inside the related entity. Centroid is computed in 3D.
*/
/**
@name LeftOf_Centroid_mm
@memo LeftOf_Centroid_mm rel_entity_name [fuzzy]. Expected distance to the left (mm), based on the x-coordinates of centroids of the entity and the related entity.
*/
/**
@name LeftOf_PlanarCentroid_mm
@memo LeftOf_PlanarCentroid_mm rel_entity_name [fuzzy]. Expected distance to the left (mm), based on the x-coordinates of centroids of the entity and the related entity. The centroid of the related entity is computed using only pixels int the image plane (slice) nearest to the centroid of the entity in question.
*/
/**
@name Length_AvgDiameter_mm 
@memo Length_AvgDiameter_mm [fuzzy]. Longest diameter and its perpendicular are computed in a 2D slice. The length value is the average of the orthogonal diameters in mm.
*/
/**
@name Length_AvgDiameter_SliceThicknesses 
@memo Length_AvgDiameter_SliceThicknesses [fuzzy]. Longest diameter and its perpendicular are computed in a 2D slice. The length value is the average of the orthogonal diameters in mm divided by the slice thicknesses, i.e., the average diameter in slice thicknesses.
*/
/**
@name Length_MaxBBox_mm 
@memo Length_MaxBBox_mm [fuzzy]. 3D bounding box is computed. The length value is the longest of any of the three dimensions of the bounding box in mm.
*/
/**
@name Length_MaxDiameter_SliceThicknesses 
@memo Length_MaxDiameter_SliceThicknesses [fuzzy]. Longest diameter and its perpendicular are computed in a 2D slice. The length value is the longest diameter in mm divided by the slice thicknesses, i.e., the maximum diameter in slice thicknesses.
*/
/**
@name Length_ShortAxis_Pixels
@memo Length_ShortAxis_Pixels [fuzzy]. Longest diameter and its perpendicular are computed in a 2D slice. The short axis is the length of perpendicular in pixels. for 3-D ROIs all slices are checked to find the longest diameter and the perpendicular (short axis) is computed on the same slice.
*/
/**
@name Length_ShortAxis_SliceThicknesses 
@memo Length_ShortAxis_SliceThicknesses [fuzzy]. Longest diameter and its perpendicular are computed in a 2D slice. The length value is the perpendicular diameter in mm divided by the slice thicknesses, i.e., the short axis length in slice thicknesses.
*/
/**
@name Length_Z_SliceThicknesses 
@memo Length_Z_SliceThicknesses [fuzzy]. The distance between the top and bottom slices is computed based on DICOM slice location (difference between first and last slice). The length value is the distance in mm divided by the slice thicknesses, i.e., the longitudinal dimension in slice thicknesses.
*/
/**
@name NearX_Centroid_mm
@memo NearX_Centroid_mm rel_entity_name [fuzzy]. Expected difference between the x-coordinates of centroids of the entity and the related entity expressed in mm.
*/
/**
@name NearX_PlanarCentroid_mm
@memo NearX_PlanarCentroid_mm rel_entity_name [fuzzy]. Expected difference between the x-coordinates of centroids of the entity and the related entity expressed in mm. The centroid of the related entity is computed using only pixels int the image plane (slice) nearest to the centroid of the entity in question.
*/
/**
@name NearY_Centroid_mm
@memo NearY_Centroid_mm rel_entity_name [fuzzy]. Expected difference between the y-coordinates of centroids of the entity and the related entity expressed in mm.
*/
/**
@name NearY_PlanarCentroid_mm
@memo NearY_PlanarCentroid_mm rel_entity_name [fuzzy]. Expected difference between the y-coordinates of centroids of the entity and the related entity expressed in mm. The centroid of the related entity is computed using only pixels int the image plane (slice) nearest to the centroid of the entity in question.
*/
/**
@name NearZ_Centroid_mm
@memo NearZ_Centroid_mm rel_entity_name [fuzzy]. Expected difference between the z-coordinates of centroids of the entity and the related entity expressed in mm. Difference is z-coordinate of entity minus z-coordinate of related entity. If the difference in z-coordinates is negative (superior for a chest CT scan) then the value in mm is a negative number.
*/
/**
@name NoOverlap
@memo NoOverlap rel_entity_name. Gives a confidence of 0.0 if the entity in question overlaps the related entity, confidence of 1.0 otherwise. Overlap is determined in 3D.
*/
/**
@name NotInContactWith
@memo NotInContactWith rel_entity_name. Converse of InContactWith.
*/
/**
@name NotInContactWith_2D
@memo NotInContactWith_2D rel_entity_name. Converse of InContactWith_2D.
*/
/**
@name Outside_Centroid
@memo Outside_Centroid rel_entity_name. Gives confidence of 1 if the centroid of the entity in question is not inside the related entity. Centroid is computed in 3D.
*/
/**
@name Overlaps
@memo Overlaps rel_entity_name. Gives a confidence of 1.0 if the entity in question overlaps the related entity, confidence of 0 otherwise. Overlap is determined in 3D.
*/
/**
@name RightOf_Centroid_mm
@memo RightOf_Centroid_mm rel_entity_name [fuzzy]. Expected distance to the right (mm), based on the x-coordinates of centroids of the entity and the related entity.
*/
/**
@name Sphericity
@memo Sphericity [fuzzy]. Percentage of volume of bounding sphere occupied by region.Sphericity = 100*(volume of region)/(volume of bounding sphere). Radius of bounding sphere is the maximum distance (in mm) from centroid to any point in the region.
*/
/**
@name SurfaceContact_Percentage
@memo SurfaceContact_Percentage rel_entity_name [fuzzy]. The surface of the entity in question is determined by dilating its ROI by a single voxel. The percentage (0 - 100) of voxels in this surface that overlap with the related entity is assessed.
*/
/**
@name Volume_cc
@memo Volume_cc [fuzzy]. Expected volume specified in cc.
*/
/**
@name Volume_mm3
@memo Volume_mm3 [fuzzy]. Expected volume specified in mm3.
*/
//@}

/**
@name Segmentation Parameters
@memo Vocabulary terms which are mapped to SegParam Attributes.
*/
//@{
/**
@name AddMatchedCandidates
@memo AddMatchedCandidates rel_entity_name rel_entity_name. If this attribute is defined then system will copy the matched image candidates from the related entity to use as candidates for the current entity. There may be one or more related entities specified in which case the matched candidates from each of them are included.
*/
/**
@name DistanceMap2D_Thresh_Perc3DLocalMax
@memo DistanceMap2D_Thresh_Perc3DLocalMax rel_entity_name dist_thresh percentage hu_diff_thresh max_z_dist. When the local EDM maximum is found this threshold is applied. If the local maximum is below this threshold (in mm) no candidate is formed (this stops candidates being formed that are only a couple of voxels thick at their widest from noise). For other contiguous voxels to be included in the candidate they must have a distance map value above the given percentage of the local maximum. Also, to be included, a voxel must have an absolute difference in HU with the HU-value at local (distance) maximum that is within a specified range (hu_diff_thresh). See SegmentationKS.h for further documentation.
*/
//@{
/**
@name DistanceMapRegionGrowing
@memo DistanceMapRegionGrowing rel_entity_name dist_thresh percentage.
*/
//@{
/**
@name DistanceMapWatershed
@memo DistanceMapWatershed rel_entity_name dist_thresh.
*/
/**
@name DoNotFormCandsWith_NumVoxelsLessThan
@memo DoNotFormCandsWith_NumVoxelsLessThan Min_Num_Voxels. If this attribute is defined then system will require that regions should have the specified minimum number of voxels before forming a candidate (if there are no other overriding segmentation parameters). Integers can be set by chromosome {chromosome start bit #, end bit #, low value, high value}.
*/
/**
@name GrowPartSolid
@memo GrowPartSolid rel_entity_name percentage. This segmentation KS expects to receive a related entity comprising solid portions of lung nodules, and will attempt to grow any non-solid components around them. An adaptive threshold is used to generate an ROI for each part solid nodule and a distance map transformation is performed. Using the original seed point from the solid component new distance threshold bounds are computed using the percentage parameter and region growing is performed on the distance map to include contingious voxels within this range.
*/
/**
@name HUrange
@memo HUrange Lower_HU Upper_HU. Lower and upper threshold values must be integers. Integers can be set by chromosome {chromosome start bit #, end bit #, low value, high value}. Used for threshold based region-growing. Only used if image sequence on the blackboard is a CT.
*/
/**
@name IncludeAllVoxels
@memo IncludeAllVoxels. If this attribute is defined then system will attempt to include all voxels in the search area in a single candidate, respecting HUrange and 2D segmentation constraints where applicable.
*/
/**
@name IntensitySubregion
@memo IntensitySubregion rel_entity_name. Some organs have subregions with different intensity/contrast profiles. The rel_entity defines an organ subregion with a particular intensity/contrast profile. The subregion is not intended to be a separate component, but rather a subregion of a smoothly varying organ with a different intensity/contrast profile. A corresponding SegParam attribute is created that can be used by the NeuralNetKeras segmentation agent. For image pre-processing the histogram will be derived from the region (ph_area) and used as the basis for deriving the gray level lookups, but the pre-processing (normalization/enhancement) will be done for the entire search area (if intensities are above or below in the SA then they will be truncated when performing the pre-processing). This approach enables training of a DNN for segmenting an entire organ (to preserve full context in the task), but allows image preprocessing (normalization / enhancement) to be optimized to the intensity/contrast of a particular subregion.
*/
/**
@name LineToDots
@memo LineToDots axis spacing check_distance. Axis is "x" or "y". Spacing and check_distance are in mm. Tracks a smooth curve (provided by the search area) at intervals specified by a spacing (and searching ahead up to check_distance if there are discontinuities), starting in the axis direction specified. If the direction is x, the leftmost point is used as the start, and if the direction is y, the top point is used. The algorithm operates in 2D and processes each slice independently, finding one curve per slice.
*/
/**
@name MaxCostPath
@memo MaxCostPath. If this attribute is defined then system will generate image candidates as 2D maximum cost paths. One path (candidate) for each slice in which the search area exists. The path is in the y-direction. The computation of the path respects the UseSampled attribute. The image from which the path is computed is formed as follows: voxels inside the search area retain their but are capped at -100HU (to avoid too much attraction to sternum); voxels outside take the minimum value of any voxels inside; all voxels are inverted by subtracting their value from 4095. Checks minimum number of voxels requirement if specified in MinNumVoxels SegParam.
*/
/**
@name NeuralNetKeras
@memo NeuralNetKeras cnn_arch image_rows image_cols. cnn_arch = unet_5block; rows and columns must be integers (they are the input image size for the CNN).
*/
/**
@name NeuralNet_LearningRate
@memo NeuralNet_LearningRate learning_rate_value learning_rate_exponent. The learning rate = value*10^exp, e.g., parameters of 1.0 and -3.0 would yield a learning rate of 0.001. The NeuralNetKeras attribute, that specifies the CNN architecture must be provided before this one. Both parameters are floats and accept chromosomes.
*/
/**
@name NeuralNet_Normalization
@memo NeuralNet_Normalization channel_number num_processing_options process_number [ordered list of processing methods and hyper parameters (first process is number 0, then number 1, etc]. channel_number is an integer starting at 0, if multiple attributes have the same channel number the processing operations will be applied to the image in sequence during normalization. num_processing_options must be equal to the number of methods specified in the ordered list. process_number must be an integer [0, num_processing_options) and determines which processing will be applied (only one from the list will be executed). Processing method options are: mean0_std1, minmax, histo_eq, clahe XX YY (where XX is a float with clip limit and YY is an integer number of bins), denoise, no_channel (means that no channel will be created for this channel number). The NeuralNetKeras attribute, that specifies the CNN architecture must be provided before this one. 
*/
/**
@name NeuralNet_Parameter
@memo NeuralNet_Parameter group name type value. Permitted argument examples are as follows (any float, int, or exp may be included in the chromosome): 
NeuralNet_Parameter model_info optimizer string Adam;
optimizer   = Adam
    ; Optimizer name: str
    ;     Optimizers supported by Keras
NeuralNet_Parameter model_info decay exp 1 10 -4;
	; exp 1 10 -4 = 1*10^-4 = 0.0001
	; any of three numerical components of the exp can be set by the chromosome (most likely it is the exponent that would vary)
NeuralNet_Parameter model_info loss string dice;
    ; Name of the loss functions: str
    ;     should be inluded in cnn_loss_and_metrics.py
NeuralNet_Parameter model_info metrics string precision;
NeuralNet_Parameter model_info metrics string recall;
NeuralNet_Parameter model_info metrics string dice;
    ; Name of the metrics: str
    ;     should be inluded in cnn_loss_and_metrics.py
    ;     more than one can be specified by including multiple parameters
NeuralNet_Parameter training_info epochs int 50;
NeuralNet_Parameter training_info batch_size int 32;
NeuralNet_Parameter training_info validation_size float 0.2;
NeuralNet_Parameter training_info sequential boolean false;
NeuralNet_Parameter training_info replace boolean false;
NeuralNet_Parameter training_info augment boolean true;
NeuralNet_Parameter validation_info batch_size int 4;
NeuralNet_Parameter validation_info sequential boolean false;
NeuralNet_Parameter validation_info replace boolean false;
NeuralNet_Parameter validation_info augment boolean false;
NeuralNet_Parameter test_info batch_size int 4;
NeuralNet_Parameter test_info mode string evaluation;
NeuralNet_Parameter test_info sequential boolean false;
NeuralNet_Parameter test_info replace boolean false;
NeuralNet_Parameter test_info augment boolean false;
*/
/**
@name PlatenessRegionGrowing
@memo PlatenessRegionGrowing low high. When performing region growing, voxels will be included if the plateness values are >= low and <= high (which are floats). The range of the plateness values are [0.0, 3.0] obtained by summing the 2D plateness values from three orthogonal planes. The function reads a raw plateness file from the ROI directory.
*/
/**
@name SameCandidatesAs
@memo SameCandidatesAs rel_entity_name rel_entity_name. If this attribute is defined then system will copy the same image candidates as the related entity to use as candidates for the current entity. There may be one or more related entities specified in which case the candidates from each of them are included.
*/
/**
@name SegmentIn2D
@memo SegmentIn2D. If this attribute is defined then system will attempt to use 2D segmentation routines (the default is 3D).
*/
/**
@name UseSubsampledImage
@memo UseSubsampledImage x_step y_step z_step. If this attribute is defined then system will attempt to use a subsampled version of the image data and related image primitives when performing segmentation.
*/
//@}

/**
@name Search Areas
@memo Vocabulary terms which are mapped to SearchArea Attributes.
*/
//@{
/**
@name AnteriorTo_Range_cm AnteriorTo_Range_cm_OR
@memo AnteriorTo_Range_cm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within an offset range of the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in cm. All voxels in the x and z directions are included in the search area.
*/
/**
@name AnteriorToPosteriorTip_RangePlanar_mm
@memo AnteriorToPosteriorTip_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Y-offset range of the anterior tip of the related entity. The set of voxels is computed on a plane-by-plane basis, and the anterior tip of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. The anterior tip of a planar ROI is the midpoint of the first contiguous x-interval of pixels encountered (in raster order). Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name AnteriorToTip_RangePlanar_mm
@memo AnteriorToTip_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Y-offset range of the anterior tip of the related entity. The set of voxels is computed on a plane-by-plane basis, and the anterior tip of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. The anterior tip of a planar ROI is the midpoint of the first contiguous x-interval of pixels encountered (in raster order). Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name AnteriorToTip_RangePlanar_mm_OR
@memo AnteriorToTip_RangePlanar_mm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within a Y-offset range of the anterior tip of the related entity. The set of voxels is computed on a plane-by-plane basis, and the anterior tip of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. The anterior tip of a planar ROI is the midpoint of the first contiguous x-interval of pixels encountered (in raster order). Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name Between_X
@memo Between_X rel_entity_name. The search area includes voxels which lie between (in the x-direction) voxels of the related entity. It does not include voxels in the related entity.
*/
/**
@name Clear
@memo Clear. Clears the current search area (makes it empty). Used for threshold based region-growing.
*/
/**
@name Contract_Erode
@memo Contract_Erode structuring_element. Morphological erosion is applied to the current search area. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name ConvexHull
@memo ConvexHull rel_entity_name. Search area is set of voxels that are enclosed (2D) by the convex hulls formed from boundaries of the related primitive on each slice. The algorithm starts with the largest boundaries, and ignores boundaries that are already inside a convex hull, i.e. ignores boundaries associated with holes inside a region.
*/
/**
@name Crop_TlPropDiff_mm
@memo Crop_TlPropDiff_mm xsize ysize zsize tlxprop tlyprop tlzprop. Creates a rectangular search area with dimensions in mm of xsize ysize zsize. For each the parameter must be > 0.0, if it is <= 0 then the region extends over the entire image along that dimension. The tlxprop tlyprop tlzprop parameters indicate the proportion of the difference between the image size and the crop size along a dimension that should be assigned when positioning the top left (tl) corner of the crop box. The prop parameters should be [0.0, 1.0]. For example 0,0,0 positions the crop at the top left, 0.5,0.5,0.5 positions it in the middle of the image. If a prop parameter is outside of the allowed range then the entire image along the corresponding axis is included. If the resulting crop region extends beyond the boundary of the image then the search area is constrained to be within the image. Assumes constant pixel size in all planes, but not constant slice spacing. tlxprop tlyprop tlzprop can accept chromosome bits.
*/
/**
@name DistanceMap2D_Thresh_mm
@memo DistanceMap2D_Thresh_mm rel_entity_name distance_threshold. The search area is obtained by thresholding a 2D Euclidean distance map of a related image primitive. The distance map is computed in mm and the threshold is given in the same units. Values in the distance map must be greater than or equal to the threshold to be included in the resulting search area.
*/
/**
@name Expand_Dilate
@memo Expand_Dilate structuring_element. Morphological dilation is applied to the current search area. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name Found
@memo Found rel_entity_name. Must be called as Found_E since if the related entity is not found the attribute will be ignored. If the related entity has not been found then the search area is made empty (cleared).
*/
/**
@name InferiorTo_Range_cm InferiorTo_Range_cm_OR
@memo InferiorTo_Range_cm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within an offset range of the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in cm. All voxels in the x and y directions are included in the search area.
*/
/**
@name InferiorToTip_Range_cm
@memo InferiorToTip_Range_cm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Z-offset range of the inferior tip of the related entity (positive offset is in the inferior direction for a chest CT). The inferior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in cm. All voxels in the x and y directions are included in the search area.
*/
/**
@name InferiorToTip_Range_cm_OR
@memo InferiorToTip_Range_cm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within a Z-offset range of the inferior tip of the related entity (positive offset is in the inferior direction for a chest CT). The inferior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in cm. All voxels in the x and y directions are included in the search area.
*/
/**
@name InferiorToTip_Range_mm
@memo InferiorToTip_Range_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Z-offset range of the inferior tip of the related entity (positive offset is in the inferior direction for a chest CT). The inferior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in mm. All voxels in the x and y directions are included in the search area.
*/
/**
@name InferiorToTip_Range_mm_OR
@memo InferiorToTip_Range_mm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within a Z-offset range of the inferior tip of the related entity (positive offset is in the inferior direction for a chest CT). The inferior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in mm. All voxels in the x and y directions are included in the search area.
*/
/**
@name Inside_2D
@memo Inside_2D rel_entity_name. The name of the enclosing entity must be supplied as an argument. The search area includes all voxels that are fully enclosed (in the 2D imaging plane) by the related entity. Used for threshold based region-growing.
*/
/**
@name NearCentroid_Range_mm
@memo NearCentroid_Range_mm rel_entity_name x_lower_bound x_upper_bound y_lower_bound y_upper_bound z_lower_bound z_upper_bound. The search area includes the set of voxels that are within an X,Y,Z-offset range of the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset ranges are given in mm. If any of the lower bounds are > the upper bounds then that dimension is ignored and all voxels are included from that dimension in the search area.
*/
/**
@name NearX_RangePlanar_mm
@memo NearX_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within an X-offset range of the centroid of the related entity. The set of voxels is computed on a plane-by-plane basis, and the centroid of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the y direction are included in the search area.
*/
/**
@name NearY_RangePlanar_mm
@memo NearY_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Y-offset range of the centroid of the related entity. The set of voxels is computed on a plane-by-plane basis, and the centroid of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name NearY_RangePlanar_mm_OR
@memo NearY_RangePlanar_mm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within a Y-offset range of the centroid of the related entity. The set of voxels is computed on a plane-by-plane basis, and the centroid of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name NearZ_Range_mm
@memo NearZ_Range_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Z-offset range of the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x and y directions are included in the search area.
*/
/**
@name NotFound
@memo NotFound rel_entity_name. If the related entity has been found then the search area is made empty (cleared).
*/
/**
@name NotPartOf
@memo NotPartOf rel_entity_name. The search area excludes (subtracts) the voxels that are part of the related entity. Used for threshold based region-growing.
*/
/**
@name PartOf
@memo PartOf rel_entity_name. The search area includes exactly the voxels that are part of the related entity. Used for threshold based region-growing.
*/
/**
@name PartOf_OR
@memo PartOf_OR rel_entity_name. The search area is expanded (OR) by including the voxels that are part of the related entity. Used for threshold based region-growing.
*/
/**
@name PartOfContracted_Eroded
@memo PartOfContracted_Eroded rel_entity_name structuring_element. The search area includes the voxels that are part of the related entity after morphological erosion is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfContracted_TowardPlanarCentroid_mm
@memo PartOfContracted_TowardPlanarCentroid_mm rel_entity_name distance to be contracted. Used for threshold based region-growing. This relation is faster to compute for relatively large expansion distances, otherwise PartOfExpanded_Dilated is probably faster.
*/
/**
@name PartOfContracted_Opened
@memo PartOfContracted_Opened rel_entity_name structuring_element. The search area includes the voxels that are part of the related entity after morphological opening is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfExpanded_Closed
@memo PartOfExpanded_Closed rel_entity_name structuring_element. The search area includes the voxels that are part of the related entity after morphological closing is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfExpanded_Closed_OR
@memo PartOfExpanded_Closed_OR rel_entity_name structuring_element. The search area is expanded (OR) by including the set of voxels that are part of the related entity after morphological closing is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfExpanded_Dilated
@memo PartOfExpanded_Dilated rel_entity_name [structuring_element]. The search area includes the voxels that are part of the related entity after morphological dilation is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfExpanded_Dilated_OR
@memo PartOfExpanded_Dilated_OR rel_entity_name [structuring_element]. The search area is expanded (OR) by including the set of voxels that are part of the related entity after morphological dilation is applied. The structuring element must be specified in one of the prescribed formats. Used for threshold based region-growing.
*/
/**
@name PartOfExpanded_FromPlanarCentroid_mm
@memo PartOfExpanded_FromPlanarCentroid_mm rel_entity_name distance to be contracted. Used for threshold based region-growing. This relation is faster to compute for relatively large expansion distances, otherwise PartOfExpanded_Dilated is probably faster.
*/
/**
@name PosteriorToAnteriorTip_RangePlanar_mm
@memo PosteriorToAnteriorTip_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Y-offset range of the anterior tip of the related entity. The set of voxels is computed on a plane-by-plane basis, and the anterior tip of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. The anterior tip of a planar ROI is the midpoint of the first contiguous x-interval of pixels encountered (in raster order). Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name PosteriorToTip_RangePlanar_mm
@memo PosteriorToTip_RangePlanar_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Y-offset range of the posterior tip of the related entity. The set of voxels is computed on a plane-by-plane basis, and the posterior tip of the related entity is computed for each plane, considering only those voxels in that plane. Consequently the search area includes only those planes in which the related entity exists. The posterior tip of a planar ROI is the midpoint of the last contiguous x-interval of pixels encountered (in raster order). Used for threshold based region-growing. The lower and upper bound of the offset range are given in mm. All voxels in the x direction are included in the search area.
*/
/**
@name PosteriorTo_Range_cm PosteriorTo_Range_cm_OR
@memo PosteriorTo_Range_cm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within an offset range of the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in cm. All voxels in the x and z directions are included in the search area.
*/
/**
@name SuperiorTo_Range_cm
@memo SuperiorTo_Range_cm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within an offset range (above) the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in cm. All voxels in the x and y directions are included in the search area.
*/
/**
@name SuperiorTo_Range_cm_OR
@memo SuperiorTo_Range_cm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within an offset range (above) the centroid of the related entity. Used for threshold based region-growing. The lower and upper bound of the offset range are given in cm. All voxels in the x and y directions are included in the search area.
*/
/**
@name SuperiorToTip_Range_mm
@memo SuperiorToTip_Range_mm rel_entity_name lower_bound upper_bound. The search area includes the set of voxels that are within a Z-offset range of the superior tip of the related entity. The superior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in mm (+ve number is superior direction). All voxels in the x and y directions are included in the search area.
*/
/**
@name SuperiorToTip_Range_mm_OR
@memo SuperiorToTip_Range_mm_OR rel_entity_name lower_bound upper_bound. The search area is expanded (OR) by including the set of voxels that are within a Z-offset range of the superior tip of the related entity. The superior tip is determined by the point with minimum z-coordinate. The lower and upper bound of the offset range are given in mm (+ve number is superior direction). All voxels in the x and y directions are included in the search area.
*/
//@}

/**
@name Structuring Elements (used in Seach Area terms)
@memo Format for structuring element descriptors.
*/
//@{
/**
@name Circle
@memo Circle radius. The radius is a floating point number with units of mm. To convert this to a radius in pixels, the row pixel spacing of the first image in the sequence is used, i.e. the row and column spacings are assumed equal and the spacings on all image slices are assumed equal.
*/
/**
@name Line_X
@memo Line_X length. Represents a line in the x-direction, with a given length, centered at the origin. The length is a floating point number with units of mm. To convert this to a length in pixels, the column pixel spacing of the first image in the sequence is used, i.e. the spacings on all image slices are assumed equal.
*/
/**
@name Line_Y
@memo Line_Y length. Represents a line in the y-direction, with a given length, centered at the origin. The length is a floating point number with units of mm. To convert this to a length in pixels, the row pixel spacing of the first image in the sequence is used, i.e. the spacings on all image slices are assumed equal.
*/
/**
@name Line_Z
@memo Line_Z length. Represents a line in the z-direction, with a given length, centered at the origin. The length is a floating point number with units of mm. To convert this to a length in pixels, the spacing between the first and second images in the sequence is used, i.e. the SPACING BETWEEN ALL IMAGE SLICES ARE ASSUMED EQUAL. If there is only 1 slice in the sequence the length is set to 0.
*/
/**
@name Box
@memo Box (top_left_x, top_left_y, top_left_z) (bottom_right_x, bottom_right_y, bottom_right_z). Represents a box with top left and bottom right vertices as given. The coordinates are given as distances from the origin in mm (floats). To convert the x and y coordinates to pixels, the row and column pixel spacing of the first image in the sequence is used, i.e. the spacings on all image slices are assumed equal. TRo convert the z-coordinate to pixels, the spacing between the first and second images in the sequence is used, i.e. the SPACING BETWEEN ALL IMAGE SLICES ARE ASSUMED EQUAL. If there is only 1 slice in the sequence the top_left_z and bottom_right_z are both set to 0.
*/
//@}


/**
@name Inferencing Parameters
@memo Vocabulary terms which are mapped to InfParam Attributes.
*/
//@{
/**
@name MatchAllCandsWithConfAbove
@memo MatchAllCandsWithConfAbove confidence_threshold. The confidence must be a float in the range [0.0, 1.0]. All candidates are matched that have a confidence score >= the given threshold.
*/
//@}


/**
@name Memory Management Parameters
@memo Vocabulary terms which are mapped to MemParam Attributes.
*/
//@{
/**
@name RetainCandidates
@memo RetainCandidates. Candidates are not deleted (freed) after a match has been made to the solution element.
*/
//@}


/*
Reads a structuring element and generates an ROI.
The ROI supplied as argument is assumed to be empty and is then modified.
Returns 1 if successful, 0 otherwise and prints a warning message.
Advances the std::string index to beyond the structuring element description.
Sets the se_descr equal to the part of the std::string used to derive the structuring element.
*/
/*
int read_struct_el(const string& s, const MedicalImageSequence& mis, ROI& se, string& se_descr, int& i)
{
    int j=i, done=0;

    std::string se_type;
    if (read_word(s, se_type, i)) {
	if (se_type.compare("circle")==0) {
		float rad_mm;
		if (read_float(s, rad_mm, i) && (rad_mm>0)) {
			float rad_pix = rad_mm/mis.row_pixel_spacing(0);
			se.add_circle(rad_pix, 0, 0);

			advance_to(s, ' ', i);
			done = 1;
		}
	}
    }

    if (!done)
	cerr << "WARNING: Unable to read structuring element" << endl;
    else
	se_descr.assign(s, j, (i-j));

    return done;
}
*/


/*
bool readMatchedRoi(SolElement& se, std::string roiPath, Blackboard& bb) {
	bool done = false;
	ifstream roiFile(roiPath+"/"+se.name()+".roi");
	if (roiFile) {
		cout << "Found ROI for " << se.name() << endl;
		done = true;

		MedicalImageSequence& medseq = bb.med_im_seq();

		ROI r;
		roiFile >> r;
		//cout << r.num_pix() << endl;

		ImageRegion* ir = new ImageRegion (r, medseq);
		se.add_candidate(ir);

		se.append_matched_cand_index(0);
		se.create_matched_prim(medseq);	

		se.free_candidates();

		// Add activation record so that it doesn't try to segment
		bb.append_act_rec("ReadMatchedRoi", "SegmentationKS");
	}
	roiFile.close();
	return done;
}
*/


void ModelMapperA(Blackboard& bb)
{
	int i, j;
	const Model& model = bb.model();

	/// Vector of flags to indicate whether each chromosome bit has been used
  	//std::vector<bool> chromosome_bit_used;
	//for (int i=0; i<model.chromosome().length(); i++) chromosome_bit_used.push_back(false);

	for(i=0; i<model.n(); i++) {
		const AnatPathEntity& ape = model.entity(i);
		bb.add_sol_element(ape.name());
	}

	for(i=0; i<model.n(); i++) {
		const AnatPathEntity& ape = model.entity(i);
		SolElement& se = bb.sol_element(i);

		//if (!readMatchedRoi(se, model.roi_directory(), bb)) {
		ifstream roiFile(bb.roi_directory()+"/"+se.name()+".roi");
		if (roiFile) {
			// Create MatchAllCandidates and ignore other descriptors

			cout << "Found ROI for " << se.name() << endl;
			cout << "Adding MatchAboveConf descriptor and ignoring others" << endl;

			MatchAboveConf* mac = new MatchAboveConf (0.0);
			se.add_attribute(mac);
		}
		else {
			// Read other descriptors

		for(j=0; j<ape.n(); j++) {
			const std::string& descr = ape.descriptor(j);

			int string_index=0;
      std::string att_name;
			read_word(descr, att_name, string_index);

			int b_flag = (att_name.length()>=2) && !att_name.compare(att_name.length()-2, 2, "_B");
			if (b_flag)
				att_name.assign(att_name.substr(0, att_name.length()-2));

			int e_flag = (att_name.length()>=2) && !att_name.compare(att_name.length()-2, 2, "_E");
			if (e_flag)
				att_name.assign(att_name.substr(0, att_name.length()-2));

			int ok=0;
            
            if (!att_name.compare("AddMatchedCandidates")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index);

				if (ok) {
					Darray<int> rs_inds (1);
					Darray<std::string*> re_names (1);

					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						rs_inds.push_last(rel_solel_ind);
						re_names.push_last(new std::string (rel_entity_name));
					}
                    
                    std::string* ren = new std::string ("");
					while (read_word(descr, *ren, string_index)) {
						rel_solel_ind = bb.sol_element_index(*ren);
						if (rel_solel_ind!=-1) {
							rs_inds.push_last(rel_solel_ind);
							re_names.push_last(ren);
						}
						ren = new std::string ("");
					}
					delete ren;
					if (rs_inds.N()==1) {
						AddMatchedCandidates* sac = new AddMatchedCandidates (rel_entity_name, rel_solel_ind);
						se.add_attribute(sac);
					}
					else if (rs_inds.N()>1) {
						AddMatchedCandidates* sac = new AddMatchedCandidates (re_names, rs_inds);
						se.add_attribute(sac);
						for(int kk=0; kk<re_names.N(); kk++)
							delete re_names[kk];
					}
				}
			}
            
			else if (!att_name.compare("AnteriorTo_Range_cm") || !att_name.compare("AnteriorTo_Range_cm_OR") || !att_name.compare("PosteriorTo_Range_cm") || !att_name.compare("PosteriorTo_Range_cm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelCentroid* brc = 0;
						if (!att_name.compare("AnteriorTo_Range_cm") || !att_name.compare("AnteriorTo_Range_cm_OR")) {
							brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, 0, -10*(low+up)/2, 0, -1, 10*(up-low), -1, e_flag);
						}
						else if (!att_name.compare("PosteriorTo_Range_cm") || !att_name.compare("PosteriorTo_Range_cm_OR")) {
							brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, 0, 10*(low+up)/2, 0, -1, 10*(up-low), -1, e_flag);
						}
						if (brc && (!att_name.compare("AnteriorTo_Range_cm_OR") || !att_name.compare("PosteriorTo_Range_cm_OR")))
							brc->set_or_flag();
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("AnteriorTo_PlanarCentroid_mm")) {
        std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						f.scale_x(-1);
						PlanarCentroidOffsetY* c = new PlanarCentroidOffsetY (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("AnteriorToPosteriorTip_RangePlanar_mm")) {
        std::string rel_entity_name;
				float low, up;
        std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarYmax* br = new BoxRelPlanarYmax(rel_entity_name, rel_solel_ind, 0, -(low+up)/2.0, -1, up-low, e_flag);
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("AnteriorToTip_RangePlanar_mm") || !att_name.compare("AnteriorToTip_RangePlanar_mm_OR")) {
        std::string rel_entity_name;
				float low, up;
        std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarYmin* br = new BoxRelPlanarYmin(rel_entity_name, rel_solel_ind, 0, -(low+up)/2.0, -1, up-low, e_flag);
						if (!att_name.compare("AnteriorToTip_RangePlanar_mm_OR"))
							br->set_or_flag();
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("Area_AllXYplanes_cm2")) {
				Fuzzy f;
				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					f.scale_x(100);

					Area_XY* a = new Area_XY (f);
					se.add_attribute(a);
				}
			}

			else if (!att_name.compare("Area_AllXYplanes_mm2")) {
				Fuzzy f;
				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					Area_XY* a = new Area_XY (f);
					se.add_attribute(a);
				}
			}

			else if (!att_name.compare("Area_PerXYplane_cm2")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					f.scale_x(100);

					Area_perXY* a = new Area_perXY (f);
					se.add_attribute(a);
				}
			}

			else if (!att_name.compare("Area_MaxXYplane_mm2")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					//cout << f <<endl;
					Area_maxXY* a = new Area_maxXY (f);
					a->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(a);
				}
			}


			else if (!att_name.compare("Area_PerXYplane_mm2")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) && //read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					Area_perXY* a = new Area_perXY (f);
					a->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(a);
				}
			}

			else if (!att_name.compare("Between_X")) {
        std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BetweenX* bx = new BetweenX (rel_entity_name, rel_solel_ind, e_flag);
						se.add_attribute(bx);
					}
				}
			}

			else if (!att_name.compare("Circularity_AtMaxDiameter_perc")) {
				Fuzzy f;
				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					CircularityAtMaxDia* cm = new CircularityAtMaxDia (f);
					se.add_attribute(cm);
				}
			}

			else if (!att_name.compare("Circularity_MinAllXYplanes_perc")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					CircularityMin* cm = new CircularityMin (f);
					cm->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(cm);
				}
			}

			else if (!att_name.compare("Clear")) {
				ok = !skip_blanks(descr, string_index);

				if (ok) {
					Clear* c = new Clear ();
					se.add_attribute(c);
				}
			}

			else if (!att_name.compare("Contract_Erode")||!att_name.compare("Expand_Dilate")) {
				ROI struct_el;
				std::string struct_el_descr;
				//int start_of_struct_el, end_of_struct_el;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_roi_descr_genetic(descr, struct_el_descr, string_index, model.chromosome(), chromosome_bit_used);
				//(descr[string_index]=='[') &&
				//skip_blanks(descr, ++string_index);

				//start_of_struct_el = string_index;
				//ok = ok && advance_to(descr, ']', string_index);
				//end_of_struct_el = string_index-1;
				//string_index++;

				ok = ok && !skip_blanks(descr, string_index);

				if (ok) {
					//std::string struct_el_descr;
					//struct_el_descr.assign(descr, start_of_struct_el, end_of_struct_el-start_of_struct_el+1);
					if (!att_name.compare("Contract_Erode")) {
						MorphErode* me = new MorphErode (struct_el_descr, e_flag);
						me->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(me);
					}
					else if (!att_name.compare("Expand_Dilate")) {
						MorphDilate* md = new MorphDilate (struct_el_descr, e_flag);
						md->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(md);
					}

				}
			}

			else if (!att_name.compare("ConvexHull")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						ConvexHull* ch = new ConvexHull (rel_entity_name, rel_solel_ind, e_flag);
						se.add_attribute(ch);
					}
				}
			}
            
			else if (!att_name.compare("Crop_TlPropDiff_mm") || !att_name.compare("Crop_TlPropDiff_mm_OR")) {
				float xsize, ysize, zsize, tlxprop, tlyprop, tlzprop;
				std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) && read_float(descr, xsize, string_index);
				ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
				if (ok && (descr[string_index]=='{')) {
					ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, xsize, string_index);
					ok = ok && advance_to(descr, ' ', string_index);
				}
				ok = ok && skip_blanks(descr, string_index) && read_float(descr, ysize, string_index);
				ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
				if (ok && (descr[string_index]=='{')) {
					ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, ysize, string_index);
					ok = ok && advance_to(descr, ' ', string_index);
				}
				ok = ok && skip_blanks(descr, string_index) && read_float(descr, zsize, string_index);
				ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
				if (ok && (descr[string_index]=='{')) {
					ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, zsize, string_index);
					ok = ok && advance_to(descr, ' ', string_index);
				}
				ok = ok &&
				skip_blanks(descr, string_index) &&
				read_float(descr, tlxprop, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, tlyprop, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, tlzprop, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					BoxSizeTlProp* brc = new BoxSizeTlProp(xsize, ysize, zsize, tlxprop, tlyprop, tlzprop);
					if (brc && !att_name.compare("Crop_TlPropDiff_mm_OR"))
						brc->set_or_flag();
					brc->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(brc);
				}
			}

			else if (!att_name.compare("DistanceMap2D_Thresh_mm") || !att_name.compare("DistanceMap2D_Thresh_mm_OR")) {
        std::string rel_entity_name;
				float thresh;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, thresh, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						DistanceMap2D* br = new DistanceMap2D(rel_entity_name, rel_solel_ind, thresh, e_flag);
						if (!att_name.compare("DistanceMap2D_Thresh_mm_OR"))
							br->set_or_flag();
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("DistanceMap2D_Thresh_Perc3DLocalMax")) {
				std::string rel_entity_name;
				float dthresh;
				float perc;
				int hu_diff_thresh;
				float mzd;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, dthresh, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, perc, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_integer(descr, hu_diff_thresh, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, mzd, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						DistanceMap25DPercMax* br = new DistanceMap25DPercMax(rel_entity_name, rel_solel_ind, dthresh, perc, hu_diff_thresh, mzd);
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("DistanceMapRegionGrowing")) {
				std::string rel_entity_name;
				float dthresh;
				float perc;

				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, dthresh, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, perc, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						DistanceMapRegionGrowing* br = new DistanceMapRegionGrowing(rel_entity_name, rel_solel_ind, dthresh, perc);
						se.add_attribute(br);
					}
				}
			}
			
			else if (!att_name.compare("DistanceMapWatershed")) {
				std::string rel_entity_name;
				float dthresh;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, dthresh, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						DistanceMapWatershed* br = new DistanceMapWatershed(rel_entity_name, rel_solel_ind, dthresh);
						se.add_attribute(br);
					}
				}
			}
			
			else if (!att_name.compare("DistanceFrom_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						CentroidDistance* c = new CentroidDistance (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("DoNotFormCandsWith_NumVoxelsLessThan")) {
				int min_num;
				std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_integer(descr, min_num, string_index);
				if (ok && advance_to(descr, ' ', string_index)) {
				 ok = read_gene_integer(descr, model.chromosome(), chromosome_bit_used, min_num, string_index);
				}
				//!advance_to(descr, ' ', string_index);

				if (ok && (min_num<0)) {
					cerr << "WARNING: Argument invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				if (ok) {
					MinNumVoxels* mnv = new MinNumVoxels (min_num);
					mnv->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(mnv);
				}
			}

			else if (!att_name.compare("Found")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Found* nf = new Found (rel_entity_name, rel_solel_ind, e_flag, 1);
						se.add_attribute(nf);
					}
				}
			}

			else if (!att_name.compare("GrowPartSolid")) {
				std::string rel_entity_name;
				float perc;

				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, perc, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						GrowPartSolid* br = new GrowPartSolid(rel_entity_name, rel_solel_ind, perc);
						se.add_attribute(br);
					}
				}
			}
			
			else if (!att_name.compare("IntensitySubregion")) {

				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						IntensitySubregion* p = new IntensitySubregion (rel_entity_name, rel_solel_ind);
						se.add_attribute(p);
					}
				}
			}
			
			else if (!att_name.compare("HUrange")) {
			    if (!strcmp(bb.med_im_seq().modality(), "CT")) {
				int low_HU, up_HU, low_GL, up_GL;
				std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
//cout << descr << endl;

				ok = skip_blanks(descr, string_index) &&
				read_integer(descr, low_HU, string_index) &&
				advance_to(descr, ' ', string_index) &&
				read_gene_integer(descr, model.chromosome(), chromosome_bit_used, low_HU, string_index) &&
				advance_to(descr, 't', string_index) &&
				read_word(descr, to_str, string_index) &&
				!to_str.compare("to") &&
				read_integer(descr, up_HU, string_index);
				if (ok && advance_to(descr, ' ', string_index)) {
				 ok = read_gene_integer(descr, model.chromosome(), chromosome_bit_used, up_HU, string_index);
				}

				if (ok && (up_HU<low_HU)) {
					cerr << "WARNING: HU range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				// Make sure modality is CT
				// Convert from HU to GL
				if (ok && HU_to_GL(low_HU, low_GL, bb.med_im_seq()) && HU_to_GL(up_HU, up_GL, bb.med_im_seq())) {
					ThreshRangeGL* trgl = new ThreshRangeGL (low_GL, up_GL);
					trgl->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(trgl);
				}
			   }
			   else
				ok=1;
			}

			else if (!att_name.compare("IncludeAllVoxels")) {

				ok = !skip_blanks(descr, string_index);

				if (ok) {
					IncludeAllVoxels* s = new IncludeAllVoxels ();
					se.add_attribute(s);
				}
			}

			else if (!att_name.compare("InContactWith") || !att_name.compare("InContactWith_2D")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Fuzzy f;
						FPoint p1(0, 0), p2(1, 1);
						f.append(p1);
						f.append(p2);
						int threed_flag = 1;
						if (!att_name.compare("InContactWith_2D")) threed_flag = 0;
						
						Contacts* c = new Contacts (rel_entity_name, rel_solel_ind, f, b_flag, e_flag, threed_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("InferiorTo_Range_cm") || !att_name.compare("InferiorTo_Range_cm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelCentroid* brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, 0, 0, 10*(low+up)/2, -1, -1, 10*(up-low), e_flag);
						if (!att_name.compare("InferiorTo_Range_cm_OR"))
							brc->set_or_flag();
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("InferiorToTip_Range_cm") || !att_name.compare("InferiorToTip_Range_cm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelZmax* br = new BoxRelZmax(rel_entity_name, rel_solel_ind, 0, 0, 10.0*(low+up)/2.0, -1, -1, 10.0*(up-low), e_flag);
						if (!att_name.compare("InferiorToTip_Range_cm_OR"))
							br->set_or_flag();
						se.add_attribute(br);
					}
				}
			}


			else if (!att_name.compare("InferiorToTip_Range_mm") || !att_name.compare("InferiorToTip_Range_mm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelZmax* br = new BoxRelZmax(rel_entity_name, rel_solel_ind, 0, 0, (low+up)/2.0, -1, -1, (up-low), e_flag);
						if (!att_name.compare("InferiorToTip_Range_mm_OR"))
							br->set_or_flag();
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("InferiorToTip_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						f.scale_x(-1.0);
						CentroidOffsetMaxZ* c = new CentroidOffsetMaxZ (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if ((!att_name.compare("Inside_2D"))||(!att_name.compare("Inside_2D_OR"))) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Inside2D* i2d = new Inside2D (rel_entity_name, rel_solel_ind, e_flag);
						if (!att_name.compare("Inside_2D_OR")) i2d->set_or_flag();		
						se.add_attribute(i2d);
					}
				}
			}

			else if (!att_name.compare("Inside_Centroid")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Fuzzy f;
						FPoint p1(0, 0), p2(1, 1);
						f.append(p1);
						f.append(p2);
						CentroidInside* c = new CentroidInside (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("LeftOf_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						CentroidOffsetX* c = new CentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}


			else if (!att_name.compare("LeftOf_PlanarCentroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						PlanarCentroidOffsetX* c = new PlanarCentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("Length_AvgDiameter_mm")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					AvgDiameter* s = new AvgDiameter (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}
			
			else if (!att_name.compare("Length_AvgDiameter_SliceThicknesses")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					AvgDiameterInSliceThicknesses* s = new AvgDiameterInSliceThicknesses (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}			

			else if (!att_name.compare("Length_MaxBBox_mm")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					MaxBBoxLength* s = new MaxBBoxLength (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}
            
			else if (!att_name.compare("Length_MaxDiameter_SliceThicknesses")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					MaxDiameterInSliceThicknesses* s = new MaxDiameterInSliceThicknesses (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}

			else if (!att_name.compare("Length_ShortAxis_Pixels")) {
				cout << "Mapping Length_ShortAxis_Pixels" << endl;

				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					ShortAxisInPixels* s = new ShortAxisInPixels (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}

			else if (!att_name.compare("Length_ShortAxis_SliceThicknesses")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					PerpDiameterInSliceThicknesses* s = new PerpDiameterInSliceThicknesses (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}
			
			else if (!att_name.compare("Length_Z_SliceThicknesses")) {
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					LengthInSliceThicknesses* s = new LengthInSliceThicknesses (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}


			else if (!att_name.compare("LineToDots")) {
				float spacing, check_distance;
				std::string direction;

				ok = read_word(descr, direction, string_index) &&
				read_float(descr, spacing, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, check_distance, string_index) &&
				!advance_to(descr, ' ', string_index);

				if ((direction.compare("x")!=0) && (direction.compare("y")!=0)) {
					cerr << "WARNING: LineToDots direction must be x or y for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				if (spacing <= 0) {
					cerr << "WARNING: LineToDots spacing must be > 0 " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				if (ok) {
						LineToDots* ltd = new LineToDots (direction, spacing, check_distance);
						se.add_attribute(ltd);
				}
			}


			else if (!att_name.compare("MatchAllCandsWithConfAbove")) {
				float conf_thresh;
				std::string dummy;
	
				ok = skip_blanks(descr, string_index) &&
				read_float(descr, conf_thresh, string_index) &&
				read_word(descr, dummy, string_index) &&
				!skip_blanks(descr, string_index);;

				if (ok && ((conf_thresh<0) || (conf_thresh>1))) {
					cerr << "WARNING: Confidence threshold invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				if (ok) {
					MatchAboveConf* mac = new MatchAboveConf (conf_thresh);
					se.add_attribute(mac);
				}
			}
			
			else if (!att_name.compare("MaxCostPath")) {

				ok = !skip_blanks(descr, string_index);

				if (ok) {
					MaxCostPath* s = new MaxCostPath ();
					se.add_attribute(s);
				}
			}
			
			else if (!att_name.compare("Median_HU")) {
				Fuzzy f;
				MedicalImageSequence mis = bb.med_im_seq();

				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok && strcmp(mis.modality(),"CT")) {
					cerr << "WARNING: Median_HU only valid for CT" << endl;
					ok = 0;
				}

				
				if (ok) {
					//const Image& im = mis.image_const(0);
					//cout << "***** intercept=" << im.rescale_intercept() << endl;
					//cout << "***** slope=" << im.rescale_slope() << endl;
					//f.offset_x(-im.rescale_intercept());
					//f.scale_x(1/im.rescale_slope());
				
					MedianHU* c = new MedianHU (f);
					se.add_attribute(c);
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}
			
			else if (!att_name.compare("NearCentroid_Range_mm")) {
				std::string rel_entity_name;
				float xlow, xup;
				float ylow, yup;
				float zlow, zup;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, xlow, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, xup, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, ylow, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, yup, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, zlow, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, zup, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok) {
					if (xlow>xup) { xlow=1; xup=0; }
					if (ylow>yup) { ylow=1; yup=0; }
					if (zlow>zup) { zlow=1; zup=0; }
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelCentroid* brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, (xlow+xup)/2.0, (ylow+yup)/2.0, (zlow+zup)/2.0, xup-xlow, yup-ylow, zup-zlow, e_flag);
						se.add_attribute(brc);
					}
				}
			}


			else if (!att_name.compare("NearX_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						CentroidOffsetX* c = new CentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("NearX_PlanarCentroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						PlanarCentroidOffsetX* c = new PlanarCentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("NearX_RangePlanar_mm")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarCentroid* brc = new BoxRelPlanarCentroid(rel_entity_name, rel_solel_ind, (low+up)/2.0, 0, up-low, -1, e_flag);
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("NearY_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) && //read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						CentroidOffsetY* c = new CentroidOffsetY (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						c->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("NearY_PlanarCentroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						PlanarCentroidOffsetY* c = new PlanarCentroidOffsetY (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("NearY_RangePlanar_mm") || !att_name.compare("NearY_RangePlanar_mm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarCentroid* brc = new BoxRelPlanarCentroid(rel_entity_name, rel_solel_ind, 0, (low+up)/2.0, -1, up-low, e_flag);
						if (!att_name.compare("NearY_RangePlanar_mm_OR"))
							brc->set_or_flag();
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("NearZ_Range_mm")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
					read_word(descr, rel_entity_name, string_index) &&
					read_float(descr, low, string_index) &&
					advance_to(descr, ' ', string_index) && 
					skip_blanks(descr, string_index);
				if (ok && (descr[string_index] == '{')) {
					ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, low, string_index);
					ok = ok && advance_to(descr, ' ', string_index);
				}
				ok = ok && skip_blanks(descr, string_index) && read_float(descr, up, string_index);
				if (ok && advance_to(descr, ' ', string_index)) {
					ok = ok && advance_to(descr, '{', string_index);
					ok = ok && read_gene_float(descr, model.chromosome(), chromosome_bit_used, up, string_index);
					ok = ok && !advance_to(descr, ' ', string_index);
				}

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelCentroid* brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, 0, 0, (low+up)/2.0, -1, -1, up-low, e_flag);
						brc->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("NearZ_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						CentroidOffsetZ* c = new CentroidOffsetZ (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}
			
			else if (!att_name.compare("NeuralNetKeras")) {
				int image_rows, image_cols;
				//youngwon edited
				int image_slices = 0;
				std::string cnn_arch;
				
				//youngwon edited
				// ok = read_word(descr, cnn_arch, string_index) &&
				// skip_blanks(descr, string_index) &&
				// read_integer(descr, image_rows, string_index) &&
				// advance_to(descr, ' ', string_index) &&
				// skip_blanks(descr, string_index) &&
				// read_integer(descr, image_cols, string_index) &&
				// !advance_to(descr, ' ', string_index);
				ok = read_word(descr, cnn_arch, string_index) &&
				skip_blanks(descr, string_index) &&
				read_integer(descr, image_rows, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_integer(descr, image_cols, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_integer(descr, image_slices, string_index) &&
				!advance_to(descr, ' ', string_index);

				pcl::FileNameTokenizer mfname(model.file());
				std::string model_path = mfname.getPath();
				model_path.append(se.name());
				model_path.append("_weights.hd5");

				//youngwon edited TODO: I think this will causs some error. 
				// need to discuss the default descr that can handle 3D too.
				// if (ok && ((image_rows <= 0) || (image_cols <= 0))) {
				// 	cerr << "WARNING: image dimensions invalid for " << ape.name() << ": " << descr << endl;
				// 	ok = 0;
				// }
				// if (ok) {
				// 	NeuralNetKeras* nnk = new NeuralNetKeras (cnn_arch, model_path, image_rows, image_cols);
				// 	se.add_attribute(nnk);
				// }
				//youngwon TODO: allow 320 320 0?
				// if (ok && ((image_rows <= 0) || (image_cols <= 0)  || (image_slices <= 0))) {
				// 	cerr << "WARNING: image dimensions invalid for " << ape.name() << ": " << descr << endl;
				// 	ok = 0;
				// }
				if (ok && ((image_rows <= 0) || (image_cols <= 0))) {
					cerr << "WARNING: image dimensions invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				if (ok && (image_slices > 0)) {
					// cout << "read 3D input" << endl;
					NeuralNetKeras* nnk = new NeuralNetKeras (cnn_arch, model_path, image_rows, image_cols, image_slices);
					se.add_attribute(nnk);
				}
				if ((image_slices <= 0) && ((image_rows > 0) && (image_cols > 0))) {
					ok = 1;
					// cout << "read 2D input" << endl;
					NeuralNetKeras* nnk = new NeuralNetKeras (cnn_arch, model_path, image_rows, image_cols);
					se.add_attribute(nnk);
				}
			}

			else if (!att_name.compare("NeuralNet_LearningRate")) {
				std::string to_str;
				float lr_exp, learning_rate;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				std::vector<bool> normalization_bit_used;
				normalization_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) && read_float(descr, learning_rate, string_index);
				//cout << "learning_rate = " << learning_rate <<endl;
				ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
				//cout << descr[string_index] << endl;
				if (ok && (descr[string_index]=='{')) {
					ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, learning_rate, string_index);
					ok = ok && advance_to(descr, ' ', string_index);
				}
				ok = skip_blanks(descr, string_index) && read_float(descr, lr_exp, string_index);
				//cout << "lr_exp = " << lr_exp <<endl;
				if (advance_to(descr, ' ', string_index)) {
					ok = ok && advance_to(descr, '{', string_index);
					ok = ok && read_gene_float(descr, model.chromosome(), chromosome_bit_used, lr_exp, string_index);
					ok = ok && !advance_to(descr, ' ', string_index);
				}

				if (ok) {
					learning_rate = learning_rate*pow(10, lr_exp);
					//cout << "learning_rate = " << learning_rate <<endl;
					NeuralNetKeras* nnk = (NeuralNetKeras*) se.find_update_attribute("NeuralNetKeras");
					if (nnk) {
						nnk->set_learning_rate(learning_rate);
						nnk->set_chromosome_bits_used(chromosome_bit_used);
					}
					else {
						cerr << "WARNING: NeuralNetKeras attribute missing for " << ape.name() << ": " << descr << endl;
						ok = 0;
					}
				}
			}

			else if (!att_name.compare("NeuralNet_Normalization")) {
				int channel_index;
				int num_processing_options;
				//youngwon edited for debuging
				// int processing_index;
				int processing_index = 100;
				int j;
				std::string intensity_norm;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				std::vector<bool> normalization_bit_used;
				std::vector<bool> temp_norm_bit_used;
				normalization_bit_used.assign(model.chromosome().length(), false);
				temp_norm_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
					read_integer(descr, channel_index, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index) &&
					read_integer(descr, num_processing_options, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index) &&
					read_integer(descr, processing_index, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index);
				// youngwon edited
				// if (ok && (descr[string_index] == '{')) {
				// 	read_gene_integer(descr, model.chromosome(), normalization_bit_used, processing_index, string_index);
				// }
				if (ok && (descr[string_index] == '{')) {
					read_gene_integer(descr, model.chromosome(), normalization_bit_used, processing_index, string_index);
					ok = ok && advance_to(descr, ' ', string_index);	
				}
				//cout << "ok = " << ok << " descr = " << descr << " processing_index = " << processing_index << " string_index = " << string_index << endl;
				//cout << "channel_index = " << channel_index << "  num_processing_options = " << num_processing_options << "  processing_index = " << processing_index << endl;

				if (processing_index >= num_processing_options) {
					cerr << "WARNING: processing_index >= num_processing_options for " << ape.name() << ": " << descr << endl;
					cerr << "WARNING: insted of processing_index = " << processing_index << ", the last option with processing_index = " << num_processing_options - 1 << "will be used for " << ape.name() << ": " << descr << endl;
					processing_index = num_processing_options - 1;
					// ok = 0;
				}

				// youngwon edited
				// ok = ok && advance_to(descr, ' ', string_index);

				for(j=0; (j<num_processing_options) && (j<=processing_index) && ok; j++) {
					temp_norm_bit_used.assign(model.chromosome().length(), false);
					intensity_norm.clear();
					//ok = advance_to(descr, ' ', string_index) &&
					ok = skip_blanks(descr, string_index) &&
					read_word(descr, intensity_norm, string_index);

					// Remove any '_' characters from the processing name (since this is to be used as a separator)
					int jj = 0;
					while (jj<intensity_norm.length())
						if (intensity_norm[jj]=='_')
							intensity_norm.erase(jj, 1);
						else
							jj++;

					if (ok && (!intensity_norm.compare("clahe") || !intensity_norm.compare("normclahe"))) {
						float clip;
						int bins;
						ok = ok && skip_blanks(descr, string_index) && 
							read_float(descr, clip, string_index) && 
							advance_to(descr, ' ', string_index) &&
							read_gene_float(descr, model.chromosome(), temp_norm_bit_used, clip, string_index);
						//cout << "clip = " << clip << "   ok = " << ok << endl;
						ok = ok && skip_blanks(descr, string_index);
						ok = ok && read_integer(descr, bins, string_index);
						if (ok && advance_to(descr, ' ', string_index))
							ok = read_gene_integer(descr, model.chromosome(), temp_norm_bit_used, bins, string_index);
						//cout << "bins = " << bins << "   ok = " << ok << endl;
						if (ok) {
							char clip_buf[100];
							char bins_buf[100];
							sprintf (clip_buf, "%.3f", clip);
							sprintf (bins_buf, "%d", bins);
							intensity_norm = intensity_norm + "_" + clip_buf + "_" + bins_buf;
							//cout << "intensity_norm = " << intensity_norm << endl;

							if ((j==processing_index) || (j==(num_processing_options-1))) {
								int ii;
								for (ii=0; ii<temp_norm_bit_used.size(); ii++)
									normalization_bit_used[ii] = normalization_bit_used[ii] || temp_norm_bit_used[ii];
							}
						}
					}
					//youngwon edited
				// }
					if (ok && (!intensity_norm.compare("centile") || !intensity_norm.compare("normgenericCT"))) {
						float clip_0;
						float clip_1;
						ok = ok && skip_blanks(descr, string_index) && 
							read_float(descr, clip_0, string_index) && 
							advance_to(descr, ' ', string_index) &&
							read_gene_float(descr, model.chromosome(), temp_norm_bit_used, clip_0, string_index);
						//cout << "clip_0 = " << clip_0 << "   ok = " << ok << endl;
						ok = ok && skip_blanks(descr, string_index);
						ok = ok && read_float(descr, clip_1, string_index);
						if (ok && advance_to(descr, ' ', string_index))
							ok = read_gene_float(descr, model.chromosome(), temp_norm_bit_used, clip_1, string_index);
						//cout << "clip_1 = " << clip_1 << "   ok = " << ok << endl;
						if (ok) {
							char clip0_buf[100];
							char clip1_buf[100];
							sprintf (clip0_buf, "%.3f", clip_0);
							sprintf (clip1_buf, "%.3f", clip_1);
							intensity_norm = intensity_norm + "_" + clip0_buf + "_" + clip1_buf;
							//cout << "intensity_norm = " << intensity_norm << endl;

							if ((j == processing_index) || (j == (num_processing_options - 1))) {
								int ii;
								for (ii=0; ii<temp_norm_bit_used.size(); ii++)
									normalization_bit_used[ii] = normalization_bit_used[ii] || temp_norm_bit_used[ii];
							}
						}
					}
				}

				if (ok) {
					NeuralNetKeras* nnk = (NeuralNetKeras*) se.find_update_attribute("NeuralNetKeras");
					if (nnk) {
						nnk->add_to_channel(channel_index, intensity_norm); // Appends the processing to the channel by adding to the string with an '_' separator
						nnk->set_chromosome_bits_used(chromosome_bit_used);
						nnk->set_chromosome_bits_used(normalization_bit_used); // Union of bits used for normalization and other parameters
						nnk->set_normalization_bits_used(normalization_bit_used);
					}
					else {
						cerr << "WARNING: NeuralNetKeras attribute missing for " << ape.name() << ": " << descr << endl;
						ok = 0;
					}
				}
			}

			else if (!att_name.compare("NeuralNet_Augmentation")) {
				int stack_index;
				int num_augmentation_options;
				//youngwon edited for debuging
				// int augmentation_index;
				int augmentation_index = 100;
				int j;
				std::string augmentation;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				std::vector<bool> augmentation_bit_used;
				std::vector<bool> temp_aug_bit_used;
				augmentation_bit_used.assign(model.chromosome().length(), false);
				temp_aug_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
					read_integer(descr, stack_index, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index) &&
					read_integer(descr, num_augmentation_options, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index) &&
					read_integer(descr, augmentation_index, string_index) &&
					advance_to(descr, ' ', string_index) &&
					skip_blanks(descr, string_index);
				if (ok && (descr[string_index] == '{')) {
					read_gene_integer(descr, model.chromosome(), augmentation_bit_used, augmentation_index, string_index);
					ok = ok && advance_to(descr, ' ', string_index);	
				}
				//cout << "ok = " << ok << " descr = " << descr << " processing_index = " << processing_index << " string_index = " << string_index << endl;
				//cout << "stack_index = " << stack_index << "  num_processing_options = " << num_processing_options << "  processing_index = " << processing_index << endl;

				if (augmentation_index >= num_augmentation_options) {
					cerr << "WARNING: augmentation_index >= num_augmentation_options for " << ape.name() << ": " << descr << endl;
					cerr << "WARNING: insted of augmentation_index = " << augmentation_index << ", the last option with augmentation_index = " << num_augmentation_options - 1 << "will be used for " << ape.name() << ": " << descr << endl;
					augmentation_index = num_augmentation_options - 1;
					// ok = 0;
				}

				for(j=0; (j<num_augmentation_options) && (j<=augmentation_index) && ok; j++) {
					temp_aug_bit_used.assign(model.chromosome().length(), false);
					augmentation.clear();
					//ok = advance_to(descr, ' ', string_index) &&
					ok = skip_blanks(descr, string_index) &&
					read_word(descr, augmentation, string_index);

					// Remove any '_' characters from the augmentation name (since this is to be used as a separator)
					int jj = 0;
					while (jj<augmentation.length())
						if (augmentation[jj]=='_')
							augmentation.erase(jj, 1);
						else
							jj++;

					if (ok && (!augmentation.compare("randomzoom") || !augmentation.compare("randomshift") || !augmentation.compare("randombrightness"))) {
						float vmin, vmax;
						ok = ok && skip_blanks(descr, string_index) && 
							read_float(descr, vmin, string_index) && 
							advance_to(descr, ' ', string_index) &&
							read_gene_float(descr, model.chromosome(), temp_aug_bit_used, vmin, string_index);
						//cout << "vmin = " << vmin << "   ok = " << ok << endl;
						ok = ok && skip_blanks(descr, string_index);
						ok = ok && read_float(descr, vmax, string_index);
						if (ok && advance_to(descr, ' ', string_index))
							ok = read_gene_float(descr, model.chromosome(), temp_aug_bit_used, vmax, string_index);
						//cout << "vmax = " << vmax << "   ok = " << ok << endl;
						if (ok) {
							char vmin_buf[100];
							char vmax_buf[100];
							sprintf (vmin_buf, "%.3f", vmin);
							sprintf (vmax_buf, "%.3f", vmax);
							augmentation = augmentation + "_" + vmin_buf + "_" + vmax_buf;
							//cout << "augmentation = " << augmentation << endl;

							if ((j==augmentation_index) || (j==(num_augmentation_options-1))) {
								int ii;
								for (ii=0; ii<temp_aug_bit_used.size(); ii++)
									augmentation_bit_used[ii] = augmentation_bit_used[ii] || temp_aug_bit_used[ii];
							}
						}
					}
					if (ok && (!augmentation.compare("randomrotation"))) {
						float clip;
						ok = ok && skip_blanks(descr, string_index) && 
							read_float(descr, clip, string_index);
							// read_float(descr, clip, string_index) && 
							// advance_to(descr, ' ', string_index) &&
							// read_gene_float(descr, model.chromosome(), temp_aug_bit_used, clip, string_index);
						if (ok && advance_to(descr, ' ', string_index))
							ok = read_gene_float(descr, model.chromosome(), temp_aug_bit_used, clip, string_index);
						if (ok) {
							char clip_buf[100];
							sprintf (clip_buf, "%.3f", clip);
							augmentation = augmentation + "_" + clip_buf;
							if ((j == augmentation_index) || (j == (num_augmentation_options - 1))) {
								int ii;
								for (ii=0; ii<temp_aug_bit_used.size(); ii++)
									augmentation_bit_used[ii] = augmentation_bit_used[ii] || temp_aug_bit_used[ii];
							}
						}
					}
					if (ok && (!augmentation.compare("pointjitter"))) {
						int wp, hp;
						ok = ok && skip_blanks(descr, string_index) && 
							read_integer(descr, wp, string_index) && 
							advance_to(descr, ' ', string_index) &&
							read_gene_integer(descr, model.chromosome(), temp_aug_bit_used, wp, string_index);
						ok = ok && skip_blanks(descr, string_index);
						ok = ok && read_integer(descr, hp, string_index);
						if (ok && advance_to(descr, ' ', string_index))
							ok = read_gene_integer(descr, model.chromosome(), temp_aug_bit_used, hp, string_index);
						if (ok) {
							char wp_buf[100];
							char hp_buf[100];
							sprintf (wp_buf, "%d", wp);
							sprintf (hp_buf, "%d", hp);
							augmentation = augmentation + "_" + wp_buf + "_" + hp_buf;
							//cout << "augmentation = " << augmentation << endl;

							if ((j==augmentation_index) || (j==(num_augmentation_options-1))) {
								int ii;
								for (ii=0; ii<temp_aug_bit_used.size(); ii++)
									augmentation_bit_used[ii] = augmentation_bit_used[ii] || temp_aug_bit_used[ii];
							}
						}
					}
				}

				if (ok) {
					NeuralNetKeras* nnk = (NeuralNetKeras*) se.find_update_attribute("NeuralNetKeras");
					if (nnk) {
						nnk->add_to_augmentation(stack_index, augmentation); // Appends the processing to the channel by adding to the string with an '_' separator
						nnk->set_chromosome_bits_used(chromosome_bit_used);
						nnk->set_chromosome_bits_used(augmentation_bit_used); // Union of bits used for normalization and other parameters
						// nnk->set_normalization_bits_used(normalization_bit_used);
					}
					else {
						cerr << "WARNING: NeuralNetKeras attribute missing for " << ape.name() << ": " << descr << endl;
						ok = 0;
					}
				}
			}


			else if (!att_name.compare("NeuralNet_Parameter")) {
				std::string p_group;
				std::string p_name;
				std::string p_type;
				std::string p_value;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) && 
					read_word(descr, p_group, string_index) && 
					read_word(descr, p_name, string_index) &&
					read_word(descr, p_type, string_index);
				if (ok && !p_type.compare("string")) {
					ok = read_word(descr, p_value, string_index);
				}
				else if (ok && !p_type.compare("float")) {
					float fv;
					ok = read_float(descr, fv, string_index);
					if (ok && advance_to(descr, ' ', string_index)) {
						ok = advance_to(descr, '{', string_index) && 
							read_gene_float(descr, model.chromosome(), chromosome_bit_used, fv, string_index)&& 
							!advance_to(descr, ' ', string_index);
					}
					if (ok) {
						std::ostringstream ss;
						int iv = (int)fv;
						if (fv == iv)
							ss << iv;
						else 
							ss << fv;
						p_value = ss.str();
					}
				}
				else if (ok && !p_type.compare("exp")) {
					float coeff, base, exp, fv;

					ok = skip_blanks(descr, string_index) && read_float(descr, coeff, string_index);
					ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
					if (ok && (descr[string_index] == '{')) {
						ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, coeff, string_index);
						ok = ok && advance_to(descr, ' ', string_index);
					}
					ok = ok && skip_blanks(descr, string_index) && read_float(descr, base, string_index);
					ok = ok && advance_to(descr, ' ', string_index) && skip_blanks(descr, string_index);
					if (ok && (descr[string_index] == '{')) {
						ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, base, string_index);
						ok = ok && advance_to(descr, ' ', string_index);
					}
					ok = ok && skip_blanks(descr, string_index) && read_float(descr, exp, string_index);
					if (advance_to(descr, ' ', string_index)) {
						ok = ok && advance_to(descr, '{', string_index);
						ok = ok && read_gene_float(descr, model.chromosome(), chromosome_bit_used, exp, string_index);
						ok = ok && !advance_to(descr, ' ', string_index);
					}
					if (ok) {
						fv = coeff * pow(base, exp);
						std::ostringstream ss;
						int iv = (int)fv;
						if (fv == iv)
							ss << iv;
						else
							ss << fv;
						p_value = ss.str();
					}
				}
				else if (ok && !p_type.compare("int")) {
					int iv;
					ok = read_integer(descr, iv, string_index);
					if (ok && advance_to(descr, ' ', string_index)) {
						ok = advance_to(descr, '{', string_index) && 
							read_gene_integer(descr, model.chromosome(), chromosome_bit_used, iv, string_index)&& 
							!advance_to(descr, ' ', string_index);
					}
					if (ok) {
						std::ostringstream ss;
						ss << iv;
						p_value = ss.str();
					}
				}
				else if (ok && !p_type.compare("boolean")) {
					ok = read_word(descr, p_value, string_index);
					if (p_value.compare("true")!=0 && p_value.compare("false")!=0) {
						ok = 0;
						cerr << "WARNING: NeuralNetKeras boolean parameter expected true or false: " << descr << endl;
					}
				}
				else {
					ok = 0;
					cerr << "WARNING: NeuralNetKeras parameter type unknown: " << descr << endl;
				}

				if (ok) {
					NeuralNetKeras* nnk = (NeuralNetKeras*) se.find_update_attribute("NeuralNetKeras");
					if (nnk) {
						if (!p_group.compare("model_info") && !p_name.compare("metrics")) {
							//cout << "appending" << endl;
							nnk->append_parameter(p_group, p_name, p_value);
						}
						else
							nnk->set_parameter(p_group, p_name, p_value);
						nnk->set_chromosome_bits_used(chromosome_bit_used);
						cout << "NeuralNet_Parameter  " << p_group << "  " << p_name << "  " << p_value << endl;
					}
					else {
						cerr << "WARNING: NeuralNetKeras attribute missing for " << ape.name() << ": " << descr << endl;
						ok = 0;
					}
				}
			}

			else if (!att_name.compare("NotFound")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Found* nf = new Found (rel_entity_name, rel_solel_ind, e_flag, 0);
						se.add_attribute(nf);
					}
				}
			}

			else if (!att_name.compare("NotInContactWith") || !att_name.compare("NotInContactWith_2D")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Fuzzy f;
						FPoint p1(0, 1), p2(1, 0);
						f.append(p1);
						f.append(p2);
						int threed_flag = 1;
						if (!att_name.compare("NotInContactWith_2D")) threed_flag = 0;
						
						Contacts* c = new Contacts (rel_entity_name, rel_solel_ind, f, b_flag, e_flag, threed_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("NotPartOf")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						NotPartOf* np = new NotPartOf (rel_entity_name, rel_solel_ind, e_flag);
						se.add_attribute(np);
					}
				}
			}
			
			else if (!att_name.compare("Outside_Centroid")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Fuzzy f;
						FPoint p1(0, 1), p2(1, 0);
						f.append(p1);
						f.append(p2);
						CentroidInside* c = new CentroidInside (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}
			
			else if (!att_name.compare("Overlaps") || !att_name.compare("NoOverlap")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						Fuzzy f;
						FPoint p1(0, 0), p2(1, 1);
						f.append(p1);
						f.append(p2);
						if (!att_name.compare("Overlaps")) {
							Overlaps* c = new Overlaps (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
							se.add_attribute(c);
						}
						else {
							Fuzzy fno;
							FPoint p1no(0, 1), p2no(1, 0);
							fno.append(p1no);
							fno.append(p2no);
							Overlaps* c = new Overlaps (rel_entity_name, rel_solel_ind, fno, b_flag, e_flag);
							se.add_attribute(c);
						}
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}
			
			else if (!att_name.compare("PartOf") || !att_name.compare("PartOf_OR")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						PartOf* p = new PartOf (rel_entity_name, rel_solel_ind, e_flag);
						if (!att_name.compare("PartOf_OR"))
							p->set_or_flag();
						se.add_attribute(p);
					}
				}
			}

			else if (!att_name.compare("PartOfExpanded_Closed") || !att_name.compare("PartOfExpanded_Closed_OR")) {
				std::string rel_entity_name;
				//ROI struct_el;
				std::string struct_el_descr;
				//int start_of_struct_el, end_of_struct_el;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_roi_descr_genetic(descr, struct_el_descr, string_index, model.chromosome(), chromosome_bit_used);
				//cout << "struct_el_descr: " << struct_el_descr << endl;
				//cout << "string_index: " << string_index << endl;
				//cout << "ok: " << ok << endl;
				//advance_to(descr, '[', string_index) &&
				//skip_blanks(descr, ++string_index);

				//start_of_struct_el = string_index;
				//ok = ok && advance_to(descr, ']', string_index);
				//end_of_struct_el = string_index-1;
				//string_index++;

				ok = ok && !skip_blanks(descr, string_index);
				//cout << "string_index: " << string_index << endl;
				//cout << "ok: " << ok << endl;

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						//std::string struct_el_descr;
						//struct_el_descr.assign(descr, start_of_struct_el, end_of_struct_el-start_of_struct_el+1);
						MorphClose* mc = new MorphClose (rel_entity_name, rel_solel_ind, struct_el_descr, e_flag);
						//MorphClose* mc = new MorphClose (rel_entity_name, rel_solel_ind, struct_el, e_flag);
						if (!att_name.compare("PartOfExpanded_Closed_OR"))
							mc->set_or_flag();
						mc->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(mc);
					}
				}
			}

			else if (!att_name.compare("PartOfExpanded_Dilated") || !att_name.compare("PartOfExpanded_Dilated_OR")) {
				std::string rel_entity_name;
				std::string struct_el_descr;
				//ROI struct_el;
				//int start_of_struct_el, end_of_struct_el;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_roi_descr_genetic(descr, struct_el_descr, string_index, model.chromosome(), chromosome_bit_used);
				//advance_to(descr, '[', string_index) &&
				//skip_blanks(descr, ++string_index);

				//start_of_struct_el = string_index;
				//ok = ok && advance_to(descr, ']', string_index);
				//end_of_struct_el = string_index-1;
				//string_index++;

				ok = ok && !skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						//std::string struct_el_descr;
						//struct_el_descr.assign(descr, start_of_struct_el, end_of_struct_el-start_of_struct_el+1);
						MorphDilate* mc = new MorphDilate (rel_entity_name, rel_solel_ind, struct_el_descr, e_flag);
						if (!att_name.compare("PartOfExpanded_Dilated_OR"))
							mc->set_or_flag();
						mc->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(mc);
					}
				}
			}

			else if (!att_name.compare("PartOfContracted_Eroded")) {
				std::string rel_entity_name;
				std::string struct_el_descr;
				//ROI struct_el;
				//int start_of_struct_el, end_of_struct_el;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_roi_descr_genetic(descr, struct_el_descr, string_index, model.chromosome(), chromosome_bit_used);
				//advance_to(descr, '[', string_index) &&
				//skip_blanks(descr, ++string_index);

				//start_of_struct_el = string_index;
				//ok = ok && advance_to(descr, ']', string_index);
				//end_of_struct_el = string_index-1;
				//string_index++;

				ok = ok && !skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						//std::string struct_el_descr;
						//struct_el_descr.assign(descr, start_of_struct_el, end_of_struct_el-start_of_struct_el+1);
						MorphErode* me = new MorphErode (rel_entity_name, rel_solel_ind, struct_el_descr, e_flag);
						me->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(me);
					}
				}
			}

			else if (!att_name.compare("PartOfContracted_Opened")) {
				std::string rel_entity_name;
				std::string struct_el_descr;
				//ROI struct_el;
				//int start_of_struct_el, end_of_struct_el;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_roi_descr_genetic(descr, struct_el_descr, string_index, model.chromosome(), chromosome_bit_used);
				//advance_to(descr, '[', string_index) &&
				//skip_blanks(descr, ++string_index);

				//start_of_struct_el = string_index;
				//ok = ok && advance_to(descr, ']', string_index);
				//end_of_struct_el = string_index-1;
				//string_index++;

				ok = ok && !skip_blanks(descr, string_index);

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						//std::string struct_el_descr;
						//struct_el_descr.assign(descr, start_of_struct_el, end_of_struct_el-start_of_struct_el+1);
						MorphOpen* mo = new MorphOpen (rel_entity_name, rel_solel_ind, struct_el_descr, e_flag);
						mo->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(mo);
					}
				}
			}

			else if (!att_name.compare("PartOfContracted_TowardPlanarCentroid_mm") || !att_name.compare("PartOfContracted_TowardPlanarCentroid_mm_OR")) {
				std::string rel_entity_name;
				float dist;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, dist, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (dist<=0)) {
					cerr << "WARNING: distance invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						ExpandContractPlanar* ecp = new ExpandContractPlanar(rel_entity_name, rel_solel_ind, -dist, /*20,*/ e_flag);
						if (!att_name.compare("PartOfContracted_TowardPlanarCentroid_mm_OR"))
							ecp->set_or_flag();
						se.add_attribute(ecp);
					}
				}
			}

			else if (!att_name.compare("PartOfExpanded_FromPlanarCentroid_mm") || !att_name.compare("PartOfExpanded_FromPlanarCentroid_mm_OR")) {
				std::string rel_entity_name;
				float dist;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, dist, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (dist<=0)) {
					cerr << "WARNING: distance invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						ExpandContractPlanar* ecp = new ExpandContractPlanar(rel_entity_name, rel_solel_ind, dist, /*20,*/ e_flag);
						if (!att_name.compare("PartOfExpanded_FromPlanarCentroid_mm_OR"))
							ecp->set_or_flag();
						se.add_attribute(ecp);
					}
				}
			}

			else if (!att_name.compare("PlatenessRegionGrowing")) {
				float low, high;
				//std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				ok = skip_blanks(descr, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				read_gene_float(descr, model.chromosome(), chromosome_bit_used, low, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, high, string_index);
				if (ok && advance_to(descr, ' ', string_index)) {
				 ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, high, string_index);
				}

				if (ok) {
					PlatenessThreshRegGrow* pl = new PlatenessThreshRegGrow(low, high);
					pl->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(pl);
				}
			}

			else if (!att_name.compare("PosteriorToAnteriorTip_RangePlanar_mm")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarYmin* br = new BoxRelPlanarYmin(rel_entity_name, rel_solel_ind, 0, (low+up)/2.0, -1, up-low, e_flag);
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("PosteriorToTip_RangePlanar_mm")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);

				//ok = skip_blanks(descr, string_index) &&
				//read_word(descr, rel_entity_name, string_index) &&
				//read_float(descr, low, string_index) &&
				//advance_to(descr, ' ', string_index) &&
				//skip_blanks(descr, string_index) &&
				//read_float(descr, up, string_index) &&
				//!advance_to(descr, ' ', string_index);

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				read_gene_float(descr, model.chromosome(), chromosome_bit_used, low, string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index);
				if (ok && advance_to(descr, ' ', string_index)) {
				 ok = read_gene_float(descr, model.chromosome(), chromosome_bit_used, up, string_index);
				}

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelPlanarYmax* br = new BoxRelPlanarYmax(rel_entity_name, rel_solel_ind, 0, (low+up)/2.0, -1, up-low, e_flag);
						br->set_chromosome_bits_used(chromosome_bit_used);
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("RetainCandidates")) {
				ok = !skip_blanks(descr, string_index);

				if (ok) {
					RetainCands* rc = new RetainCands ();
					se.add_attribute(rc);
				}
			}

			else if (!att_name.compare("RightOf_Centroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						f.scale_x(-1);
						CentroidOffsetX* c = new CentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("RightOf_PlanarCentroid_mm")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						f.scale_x(-1);
						PlanarCentroidOffsetX* c = new PlanarCentroidOffsetX (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("SameCandidatesAs")) {
				std::string rel_entity_name;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index);

				if (ok) {
					Darray<int> rs_inds (1);
					Darray<std::string*> re_names (1);

					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						rs_inds.push_last(rel_solel_ind);
						re_names.push_last(new std::string (rel_entity_name));
					}
					//rel_entity_name.assign("");
          std::string* ren = new std::string ("");
					while (read_word(descr, *ren, string_index)) {
						rel_solel_ind = bb.sol_element_index(*ren);
						if (rel_solel_ind!=-1) {
							rs_inds.push_last(rel_solel_ind);
							re_names.push_last(ren);
						}
						ren = new std::string ("");
						//rel_entity_name.assign("");
					}
					delete ren;
					if (rs_inds.N()==1) {
						SameCandidatesAs* sac = new SameCandidatesAs (rel_entity_name, rel_solel_ind);
						se.add_attribute(sac);
					}
					else if (rs_inds.N()>1) {
						SameCandidatesAs* sac = new SameCandidatesAs (re_names, rs_inds);
						se.add_attribute(sac);
						for(int kk=0; kk<re_names.N(); kk++)
							delete re_names[kk];
					}
				}
			}

			else if (!att_name.compare("SegmentIn2D")) {

				ok = !skip_blanks(descr, string_index);

				if (ok) {
					SegmentIn2D* s = new SegmentIn2D ();
					se.add_attribute(s);
				}
			}
			else if (!att_name.compare("Sphericity")) {
				//cout << "Entering ModelKS Sphericity" << endl;
				Fuzzy f;
				std::vector<bool> chromosome_bit_used;
				chromosome_bit_used.assign(model.chromosome().length(), false);
				ok = read_fuzzy_genetic(descr, model.chromosome(), chromosome_bit_used, f, string_index) && //read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}
				//cout << f << endl;

				if (ok) {
					Sphericity* s = new Sphericity (f);
					s->set_chromosome_bits_used(chromosome_bit_used);
					se.add_attribute(s);
				}
			}

			else if (!att_name.compare("SuperiorTo_Range_cm") || !att_name.compare("SuperiorTo_Range_cm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelCentroid* brc = new BoxRelCentroid(rel_entity_name, rel_solel_ind, 0, 0, -10.0*(low+up)/2.0, -1, -1, 10.0*(up-low), e_flag);
						if (!att_name.compare("SuperiorTo_Range_cm_OR"))
							brc->set_or_flag();
						se.add_attribute(brc);
					}
				}
			}

			else if (!att_name.compare("SuperiorToTip_Range_mm") || !att_name.compare("SuperiorToTip_Range_mm_OR")) {
				std::string rel_entity_name;
				float low, up;
				std::string to_str;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_float(descr, low, string_index) &&
				advance_to(descr, ' ', string_index) &&
				skip_blanks(descr, string_index) &&
				read_float(descr, up, string_index) &&
				!advance_to(descr, ' ', string_index);

				if (ok && (up<=low)) {
					cerr << "WARNING: range invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						BoxRelZmin* br = new BoxRelZmin(rel_entity_name, rel_solel_ind, 0, 0, -(low+up)/2.0, -1, -1, up-low, e_flag);
						if (!att_name.compare("SuperiorToTip_Range_mm_OR"))
							br->set_or_flag();
						se.add_attribute(br);
					}
				}
			}

			else if (!att_name.compare("SurfaceContact_Percentage")) {
				std::string rel_entity_name;
				Fuzzy f;

				ok = skip_blanks(descr, string_index) &&
				read_word(descr, rel_entity_name, string_index) &&
				read_fuzzy(descr, f, string_index) &&
				!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					int rel_solel_ind = bb.sol_element_index(rel_entity_name);
					if (rel_solel_ind!=-1) {
						SurfaceContactPercentage* c = new SurfaceContactPercentage (rel_entity_name, rel_solel_ind, f, b_flag, e_flag);
						se.add_attribute(c);
					}
				}
				else
				{
					cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
				}
			}

			else if (!att_name.compare("UseSubsampledImage")) {
				std::string dummy;
				int x_step, y_step, z_step;

				ok = skip_blanks(descr, string_index) &&
				read_integer(descr, x_step, string_index) &&
				read_word(descr, dummy, string_index) &&
				read_integer(descr, y_step, string_index) &&
				read_word(descr, dummy, string_index) &&
				read_integer(descr, z_step, string_index) &&
				read_word(descr, dummy, string_index) &&
				!advance_to(descr, ' ', string_index);


				if (ok) {
					UseSubSampledImage* u = new UseSubSampledImage (x_step, y_step, z_step);
					se.add_attribute(u);
				}
			}

			else if (!att_name.compare("Volume_cc")) {
				Fuzzy f;
				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					// Convert from cc to mm3
					f.scale_x(1000);

					Volume* v = new Volume (f);
					se.add_attribute(v);
				}
			}

			else if (!att_name.compare("Volume_mm3")) {
				Fuzzy f;
				ok = 	read_fuzzy(descr, f, string_index) &&
					!skip_blanks(descr, string_index);

				if (ok && !f.valid()) {
					cerr << "WARNING: Fuzzy membership function invalid for " << ape.name() << ": " << descr << endl;
					ok = 0;
				}

				if (ok) {
					Volume* v = new Volume (f);
					se.add_attribute(v);
				}
			}

			if (!ok)
				cerr << "WARNING: Model Descriptor format incorrect for " << ape.name() << ": " << descr << endl;
		}
		}
	}
}
