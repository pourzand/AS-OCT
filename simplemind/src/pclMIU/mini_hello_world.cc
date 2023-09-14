// #include "miniIni.h"
// #include <iostream>

// using namespace std;

// int main()
// {

//     ///////////////////////////////////////////////////////////////////////////////
//     /*
//     * Snippet for writing config file: 1. writing from scratch
//     * Using mINI library (https://github.com/pulzed/mINI):
//     *    a tiny, header only C++ library for manipulating INI files.
//     */
//     // create a file instance
//     char configpath[5000];
//     // sprintf (configpath, "%s%s", bb.temp_file_path(), "/test_config.inii");
//     sprintf(configpath, "%s%s", "/scratch/youngwon/data/prostate/mhd_organ/output_miu", "/test_config.ini");
//     mINI::INIFile configfile(configpath);

//     // create a data structure
//     mINI::INIStructure configini;

//     // populate the structure
//     //// e.g. model information
//     configini["model_info"]["reader_class"] = "ImageReader";
    
//     char imgshape[2000];
//     // sprintf(imgshape, "%d, %d, %d", input_image_rows, input_image_cols, input_image_slices);
//     sprintf(imgshape, "%d, %d, %d", 320, 320, 22);
//     configini["model_info"]["img_shape"] = imgshape;
    
//     char boundingbox[2000];
//     //sprintf(imgshape, "%d, %d, %d, %d, %d, %d", tl.x, tl.y, tl.z, br.x, br.y, br.z);
//     sprintf(boundingbox, "%d, %d, %d, %d, %d, %d", 0, 0, 0, 511, 511, 46);
//     configini["model_info"]["bounding_box"] = boundingbox;
    
//     // configini["model_info"]["architecture"] = trgl->cnn_arch().c_str();
//     configini["model_info"]["architecture"] = "unet_3d_arb";
    
//     configini["model_info"]["optimizer"] = "Adam";
//     configini["model_info"]["lr"] = "0.01";
//     configini["model_info"]["decay"] = "0.001";
//     configini["model_info"]["loss"] = "dice";
//     configini["model_info"]["metrics"] = "precision, recall, dice";
//     configini["model_info"]["intensity_norm"] = "mean0_std1";

//     //// e.g. path information
//     // configini["path_info"]["image"] = bb.image_path();
//     configini["path_info"]["image"] = "/scratch/youngwon/data/prostate/PROMISE12/training_all/Case00.mhd";
//     // configini["path_info"]["output_path"] = bb.temp_file_path();
//     configini["path_info"]["output_path"] = "/scratch/youngwon/data/prostate/mhd_organ/output_miu";
    
//     //// e.g. training information
//     configini["training_info"]["max_epochs"] = "50";
//     configini["training_info"]["batch_size"] = "32";
//     configini["training_info"]["augmentation"] = "true";

//     //// e.g. validation information
//     configini["validation_info"]["batch_size"] = "4";

//     //// e.g. test information
//     configini["test_info"]["batch_size"] = "8";

//     // generate an INI file (overwrites any previous file)
//     bool genSuccess = configfile.generate(configini, true);
//     cout << "Generated configuration ini path: " << configpath << "with return value " << genSuccess << endl;
    
//     ///////////////////////////////////////////////////////////////////////////////
//     /*
//     * Snippet for writing config file: 2. copying and editing from existing file
//     * Using mINI library (https://github.com/pulzed/mINI):
//     *    a tiny, header only C++ library for manipulating INI files.
//     */
//     // create a reference file instance
//     char referenceconfigpath[5000];
//     sprintf (referenceconfigpath, "%s", "/scratch/youngwon/data/prostate/mhd_organ/output_miu/ref_config.ini");
//     mINI::INIFile referenceconfigfile(referenceconfigpath);
    
//     // create a reference structrue
//     mINI::INIStructure refconfigini;

//     // To read from a file:
//     bool readSuccess = referenceconfigfile.read(refconfigini);
//     cout << "Read reference configuration ini path: " << referenceconfigpath << "with return value " << readSuccess << endl;
    
//     // update the lr from 0.01 to 0.001
//     if (refconfigini.has("model_info"))
//     {
//         // we have section, we can access it safely without creating a new one
//         auto& collection = refconfigini["model_info"];
//         if (collection.has("lr"))
//         {
//             // we have key, we can access it safely without creating a new one
//             // auto& value = collection["lr"];
            
//             // we have key, we can update it safely without creating a new one
//             refconfigini["model_info"]["lr"] = "0.001";
//         }
//     }
    
    
//     // change all values in the ""validation_info" section to "0"
//     for (auto const& it : refconfigini)
//     {
//         auto const& section = it.first;
//         auto const& collection = it.second;
//         if ("validation_info" == section)
//         {
//             for (auto const& it2 : collection)
//             {
//                 auto const& key = it2.first;
//                 refconfigini[section][key] = "0";
//             }
//         }   
//     }


//     // create a new file instance
//     char newconfigpath[5000];
//     sprintf (newconfigpath, "%s", "/scratch/youngwon/data/prostate/mhd_organ/output_miu/copied_config.ini");
//     mINI::INIFile newconfigfile(newconfigpath);
    
//     // To write new file with edited reference info
//     bool copySuccess = newconfigfile.generate(refconfigini, true);
    
//     cout << "Generated configuration ini path: " << newconfigpath << "with return value " << copySuccess << endl;
//     ///////////////////////////////////////////////////////////////////////////////
// }
