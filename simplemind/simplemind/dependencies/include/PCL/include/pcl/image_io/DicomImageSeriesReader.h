#ifndef PCL_DICOM_IMAGE_SERIES_READER
#define PCL_DICOM_IMAGE_SERIES_READER

#include <itkGDCMImageIO.h>
#include <pcl/image_io/ItkImageReader.h>
#include <pcl/misc/StringTokenizer.h>
#include <pcl/statistics.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <list>
#include <map>

namespace pcl
{
	using namespace pcl::image_io;
	namespace boost_file = boost::filesystem;
	
	template <class ImageType>
	class DicomImageSeriesReader: private boost::noncopyable
	{
	public:
		enum SortType { INSTANCE, POSITION, ADDSEQ };

		typedef DicomImageSeriesReader Self;
		typedef boost::shared_ptr<Self> Pointer;

		static Pointer New()
		{
			return Pointer(new Self);
		}

		void addFile(const std::string& file)
		{
			m_File.push_back(file);
		}
		int addFilesInDirectory(const std::string& directory)
		{
			boost_file::path path(directory); 
			if (!boost_file::is_directory(path)) return 0;

			int count = 0;
			boost_file::directory_iterator end_itr;
			for (boost_file::directory_iterator itr(path); itr!=end_itr; itr++) {
				if (!boost_file::is_directory(itr->status())) {
					m_File.push_back(itr->path().string());
					count++;
				}
			}
			return count;
		}

		void read(SortType sort_type=INSTANCE, bool ignore_duplicates=false)
		{
			m_MetadataArray.clear();
			m_SliceExists.clear();
			std::map<int, double> instance_pos_lookup;
			std::map<double, ImageInfo> image_map = getImageInfoMap(sort_type, ignore_duplicates, instance_pos_lookup);

			//Special case where only 1 slice is available
			if (image_map.size()==1) {
				auto iter = image_map.begin();
				m_OutputImage = ImageType::New(iter->second.image);
				m_SliceExists.resize(image_map.size()); m_SliceExists[0] = true;
				m_MetadataArray.resize(image_map.size()); m_MetadataArray[0] = iter->second.image->getMetadata();
				pcl::ImageIterator img_iter(iter->second.image);
				pcl_ForIterator(img_iter) m_OutputImage->set(img_iter, iter->second.image->get(img_iter));
				pcl::Point3D<int> minp(0, 0, instance_pos_lookup.begin()->first);
				m_OutputImage = m_OutputImage->getAlias(minp, true);
				return;
			}

			//Getting instance based min point if is available
			pcl::Point3D<int> minp(0, 0, instance_pos_lookup.begin()->first);
			//Getting a corrected orientation matrix based on whether the slice positions are increasing or decreasing
			auto orientation_matrix = image_map.begin()->second.image->getOrientationMatrix();
			double orientation_matrix_multiplier = 1;
			{
				auto iter = image_map.begin();
				double pos1 = iter->second.slice_position;
				while (pcl::abs(pos1-iter->second.slice_position)<0.001) {
					++iter;
					if (iter==image_map.end()) pcl_ThrowException(pcl::Exception(), "Unable to find valid second slice!");
				}
				double pos2 = iter->second.slice_position;
				if (pos1>pos2) {
					orientation_matrix(0,2) = -orientation_matrix(0,2);
					orientation_matrix(1,2) = -orientation_matrix(1,2);
					orientation_matrix(2,2) = -orientation_matrix(2,2);
					orientation_matrix_multiplier = -1;
				}
			}

			//Special case where sorting is based on sequence added
			if (sort_type==ADDSEQ) {
				pcl::Point3D<double> spacing = image_map.begin()->second.image->getSpacing();
				{
					auto iter = image_map.begin();
					double pos1 = iter->second.slice_position;
					++iter;
					double pos2 = iter->second.slice_position;
					spacing[2] = pcl::abs(pos1-pos2);
				}
				pcl::Point3D<int> size = pcl::Point3D<int>(image_map.begin()->second.image->getSize().x(),
					image_map.begin()->second.image->getSize().y(),
					image_map.size());
				m_OutputImage = ImageType::New(minp, minp+size-1, spacing, image_map.begin()->second.image->getOrigin(), orientation_matrix);
				m_SliceExists.resize(image_map.size());
				m_MetadataArray.resize(image_map.size());
				pcl::Region3D<int> region = m_OutputImage->getRegion();
				region.getMaxPoint().z() = region.getMinPoint().z();
				pcl::ImageIterator iter1(m_OutputImage);
				int count = 0;
				pcl_ForEach(image_map, item) {
					iter1.setRegion(region);
					pcl::ImageIterator iter2(item->second.image);
					pcl_ForIterator2(iter1, iter2) {
						m_OutputImage->set(iter1, item->second.image->get(iter2));
					}
					m_SliceExists[count] = true;
					m_MetadataArray[count] = item->second.image->getMetadata();
					++count;
					++region.getMinPoint().z();
					++region.getMaxPoint().z();
				}
				return;
			}
			//Computing median distance between key and slice position
			double median_disp;
			double median_physical_disp;
			{
				pcl::statistics::PercentileCalculator<double> disp, phydisp;
				double prev_val;
				double prev_physical_pos;
				bool init = true;
				pcl_ForEach(instance_pos_lookup, item) {
					if (init) {
						prev_val = item->first;
						prev_physical_pos = item->second;
						init = false;
					} else {
						disp.addValue(pcl::abs(item->first-prev_val));
						phydisp.addValue(pcl::abs(item->second-prev_physical_pos));
						prev_val = item->first;
						prev_physical_pos = item->second;
					}
				}
				median_disp = disp.getMedian(false);
				median_physical_disp = phydisp.getMedian(false);
			}
			if (sort_type==POSITION) median_disp = median_physical_disp;

			//Setting up the image storage
			pcl::Point3D<double> spacing = image_map.begin()->second.image->getSpacing();
			spacing[2] = median_physical_disp;
			int slice_num = pcl::round(((image_map.rbegin()->first - image_map.begin()->first)/median_disp)+1);

			/*std::cout << image_map.rbegin()->first << std::endl;
			std::cout << image_map.begin()->first << std::endl;
			std::cout << median_disp << std::endl;*/
			pcl::Point3D<int> size = pcl::Point3D<int>(image_map.begin()->second.image->getSize().x(),
				image_map.begin()->second.image->getSize().y(),
				slice_num);
			auto output_image = InternalImageType::New(pcl::Point3D<int>(0,0,0), size-1, spacing, image_map.begin()->second.image->getOrigin(), orientation_matrix);

			/*std::cout << output_image->getOrigin() << std::endl;
			std::cout << output_image->getOrientationMatrix() << std::endl;
			std::cout << output_image->toPhysicalCoordinate(output_image->getMinPoint()) << std::endl;

			std::cout << "# " << output_image->getSpacing() << " " << image_map.begin()->second.image->getSpacing() << std::endl;*/

			output_image = output_image->getAlias(minp, true);

			/*std::cout << output_image->toPhysicalCoordinate(output_image->getMinPoint()) << std::endl;

			std::cout << std::endl;
			std::cout << output_image->getPhysicalRegion() << std::endl;
			{
				pcl::Region3D<double> reg; reg.reset();
				pcl_ForEach(image_map, item) reg.add(item->second.image->getPhysicalRegion());
				std::cout << reg << std::endl;
				std::cout << output_image->getRegion() << " " <<  image_map.size() << std::endl;
			}
			std::cout << std::endl;*/

			m_SliceExists.resize(slice_num);
			m_MetadataArray.resize(slice_num);
			for (int i=0; i<slice_num; ++i) {
				m_SliceExists[i] = false;
				m_MetadataArray[i].reset();
			}
			//Assigning storage slices to image info
			pcl::Point3D<double> vec(output_image->getOrientationMatrix()(0,2)*orientation_matrix_multiplier,
				output_image->getOrientationMatrix()(1,2)*orientation_matrix_multiplier,
				output_image->getOrientationMatrix()(2,2)*orientation_matrix_multiplier);
			{
				for (int z=output_image->getMinPoint().z(); z<=output_image->getMaxPoint().z(); ++z) {
					double cur_slice_pos;
					switch (sort_type) {
					case INSTANCE:
						cur_slice_pos = z;
						break;
					case POSITION:
						cur_slice_pos = output_image->toPhysicalCoordinate(0,0,z).getDotProduct(vec);
						break;
					}
					pcl_ForEach(image_map, item) {
						item->second.set(pcl::abs(item->first-cur_slice_pos), z);
					}
				}
				/*std::cout << std::endl << std::endl;
				pcl_ForEach(image_map, item) std::cout << item->first << "|" << item->second.instance << " ";
				std::cout << std::endl;*/
			}

			//Checks for possible error in assigning storage slices to image info
#ifndef NO_WARNING
			{
				std::string error_msg;
				double max_diff;
				if (sort_type==POSITION) max_diff = output_image->getSpacing().z()/2;
				else max_diff = 0.5;
				pcl_ForEach(image_map, item) if (item->second.diff>max_diff) {
					error_msg += "Slice " +
						boost::lexical_cast<std::string>(item->second.instance) +
						" at " + 
						boost::lexical_cast<std::string>(item->first) + 
						" matched to ";
					if (sort_type==POSITION) error_msg += boost::lexical_cast<std::string>(output_image->toPhysicalCoordinate(0,0,item->second.storage_z).getDotProduct(vec));
					else error_msg += boost::lexical_cast<std::string>(item->second.storage_z);
					error_msg += " in output volume\n";
				}
				if (!error_msg.empty()) {
					std::cout << "Warning at DicomImageSeriesReader: Slice matching exceeds allowed tolerance (" << max_diff << ") at the following slices:" << std::endl;
					std::cout << error_msg;
				}
			}
#endif
			//Filling storage slices with corresponding image info
			pcl::Region3D<int> region = output_image->getRegion();
			pcl::ImageIterator iter1(output_image);
			int min_z = region.getMinPoint().z();
			pcl_ForEach(image_map, item) {
				if (!m_SliceExists[item->second.storage_z-min_z]) {
					auto cur_item = item;
					pcl_ForEach(image_map, sub_item) {
						if (cur_item->second.storage_z==sub_item->second.storage_z) {
							if (pcl::abs(cur_item->second.diff-sub_item->second.diff)<0.001) {
								if (cur_item->second.instance<sub_item->second.instance) cur_item = sub_item; //Retain the smaller instance slice if position is same
							} else if (cur_item->second.diff>sub_item->second.diff) cur_item = sub_item;
						}
					}
					region.getMinPoint().z() = region.getMaxPoint().z() = cur_item->second.storage_z;
					iter1.setRegion(region);
					pcl::ImageIterator iter2(cur_item->second.image);
					pcl_ForIterator2(iter1, iter2) {
						output_image->set(iter1, cur_item->second.image->get(iter2));
					}
					m_SliceExists[cur_item->second.storage_z-min_z] = true;
					m_MetadataArray[cur_item->second.storage_z-min_z] = cur_item->second.image->getMetadata();
				}
			}

			m_OutputImage = ImageType::NewAlias(output_image, false);
#ifndef NO_WARNING
			if (!output_image->getOrientationMatrix().is_identity(0.0001) && !m_OutputImage->UseOrientationMatrix) {
				std::cout << "Warning at DicomImageSeriesReader: UseOrientationMatrix is false despite reading a series with non-identity orientation matrix!" << std::endl;
			}
#endif
		}

		bool checkPhysicalCoordinatesValidity() const
		{
			std::cout << m_OutputImage->getRegion() << std::endl;
			Point3D<int> minp = m_OutputImage->getMinPoint();
			int min_z = m_OutputImage->getMinPoint().z(),
				max_z = m_OutputImage->getMaxPoint().z();
			for (int z=min_z; z<=max_z; ++z) {
				int index = z-min_z;
				if (sliceExists(index)) {
					minp.z() = z;
					Point3D<double> slice_org = m_OutputImage->toPhysicalCoordinate(minp);
					Point3D<double> meta_org = getImagePositionFromMetadata(index);
					if (!slice_org.epsilonEqual(meta_org)) return false;
				}
			}
			return true;
		}

		typename ImageType::Pointer getOutputImage()
		{
			return m_OutputImage;
		}

		std::vector<int> getMissingZCoordinates() const
		{
			std::vector<int> missing_z;
			for (int i=0; i<m_SliceExists.size(); ++i) {
				if (m_SliceExists[i]==false) missing_z.push_back(i+m_OutputImage->getMinPoint().z());
			}
			return missing_z;
		}

		const std::vector<Metadata::Pointer>& getMetadataArray() const
		{
			return m_MetadataArray;
		}

		const Metadata::Pointer& getMetadata(int z) const
		{
			return m_MetadataArray[z-m_OutputImage->getMinPoint().z()];
		}

		const bool sliceExists(int z) const
		{
			return m_SliceExists[z-m_OutputImage->getMinPoint().z()];
		}

		const Point3D<double> getSliceImagePosition(int z) const
		{
			return getImagePositionFromMetadata(z-m_OutputImage->getMinPoint().z());
		}

		void fillMissingSlices(double val)
		{
			ImageIterator iter(m_OutputImage);
			Region3D<int> region(m_OutputImage->getRegion());
			for (int z=m_OutputImage->getMinPoint().z(); z<=m_OutputImage->getMaxPoint().z(); ++z) {
				if (!sliceExists(z)) {
					region.getMinPoint()[2] = region.getMaxPoint()[2] = z;
					iter.setRegion(region);
					pcl_ForIterator(iter) m_OutputImage->set(iter, val);
				}
			}
		}

		void interpolateMissingSlices()
		{
			for (int z=m_OutputImage->getMinPoint().z(); z<=m_OutputImage->getMaxPoint().z(); ++z) {
				if (!sliceExists(z)) linearInterpolateSlice(z);
			}
		}

	protected:
		typedef pcl::Image<typename ImageType::ValueType, true> InternalImageType;
		std::list<std::string> m_File;
		std::vector<Metadata::Pointer> m_MetadataArray;
		typename ImageType::Pointer m_OutputImage;
		std::vector<bool> m_SliceExists;

		struct ImageInfo
		{
			typename InternalImageType::Pointer image;
			std::string filename;
			double slice_position;
			int instance;

			double diff;
			int storage_z;

			ImageInfo()
			{
				diff = std::numeric_limits<double>::infinity();
			}

			ImageInfo(const typename InternalImageType::Pointer& i, const std::string& f, double pos, int ins)
			{
				image = i;
				filename = f;
				slice_position = pos;
				instance = ins;
				diff = std::numeric_limits<double>::infinity();
			}

			void set(double d, int z)
			{
				if (d<diff) {
					diff = d;
					storage_z = z;
				}
			}
		};

		DicomImageSeriesReader() {}

		Point3D<double> getImagePositionFromMetadata(int i) const
		{
			Point3D<double> return_val;
			std::string data = m_MetadataArray[i]->getValue<std::string>("0020|0032");
			misc::StringTokenizer tokenizer(data.c_str());
			tokenizer.begin((char)!boost::algorithm::is_any_of("+-1234567890."));
			return_val[0] = atof(tokenizer.getToken().c_str());
			tokenizer.next((char)!boost::algorithm::is_any_of("+-1234567890."));
			return_val[1] = atof(tokenizer.getToken().c_str());
			tokenizer.next((char)!boost::algorithm::is_any_of("+-1234567890."));
			return_val[2] = atof(tokenizer.getToken().c_str());
			return return_val;
		}

		void linearInterpolateSlice(int z)
		{
			int prev_z, next_z;
			for (int j=z-1; j>=m_OutputImage->getMinPoint().z(); --j) if (sliceExists(j)) {
				prev_z = j;
				break;
			}
			for (int j=z+1; j<=m_OutputImage->getMaxPoint().z(); ++j) if (sliceExists(j)) {
				next_z = j;
				break;
			}

			double s = (static_cast<double>(z-prev_z))/(static_cast<double>(next_z-prev_z)),
				one_minus_s = 1-s;
			ImageIterator prev_iter(m_OutputImage), iter(m_OutputImage), next_iter(m_OutputImage);
			Region3D<int> region(m_OutputImage->getRegion());
			region.getMinPoint()[2] = prev_z; region.getMaxPoint()[2] = prev_z; prev_iter.setRegion(region);
			region.getMinPoint()[2] = z; region.getMaxPoint()[2] = z; iter.setRegion(region);
			region.getMinPoint()[2] = next_z; region.getMaxPoint()[2] = next_z; next_iter.setRegion(region);

			pcl_ForIterator3(prev_iter, iter, next_iter) {
				m_OutputImage->set(iter, m_OutputImage->get(prev_iter)*one_minus_s + m_OutputImage->get(next_iter)*s);
			}
		}

		double getSlicePosition(const typename InternalImageType::Pointer& img)
		{
			auto mat = img->getOrientationMatrix();
			return mat(0,2)*img->getOrigin().x() + mat(1,2)*img->getOrigin().y() + mat(2,2)*img->getOrigin().z();
		}

		std::map<double, ImageInfo> getImageInfoMap(SortType sort_type, bool ignore_duplicates, std::map<int, double>& instance_pos_lookup)
		{
			std::map<double, ImageInfo> image_map;
			int count = 0;
			itk::ImageIOBase::Pointer image_io = itk::GDCMImageIO::New();
			Point3D<int> initial_size;
			pcl_ForEach(m_File, item) {
				try {
					typename InternalImageType::Pointer raw_img = ItkImageReader<InternalImageType>::ReadImage(*item, image_io);
					std::vector<typename InternalImageType::Pointer> raw_img_vec;
					if (raw_img->getSize().z()>1) {
						auto region = raw_img->getRegion();
						for (int z=raw_img->getMinPoint().z(); z<raw_img->getMaxPoint().z(); ++z) {
							region.getMinPoint().z() = region.getMaxPoint().z() = z;
							auto img = pcl::ImageHelper::GetCroppedAuto(raw_img, region);
							img = img->getAlias(pcl::Point3D<int>(0,0,0), true);
							img->setMetadata(raw_img->getMetadata());
							raw_img_vec.push_back(img);
							if (sort_type==INSTANCE) {
								std::cout << "Warning: Multiple slices found and only first slice is retained due to INSTANCE sorting in " << *item << std::endl;
								break;
							}
						}
					} else {
						raw_img_vec.push_back(raw_img);
					}

					pcl_ForEach(raw_img_vec, img_item) {
						auto &img = *img_item;
						if (count==0) {
							initial_size = img->getSize();
						} else {
							if (img->getSize()!=initial_size) {
								std::stringstream ss;
								ss << *item << " contains an image of different size compared to the first item " << initial_size.toString();
								pcl_ThrowException(ImageReaderException(), ss.str());
							}
						}

						int instance;
						if (sort_type!=ADDSEQ) {
							try {
								Metadata::Iterator iter = img->getMetadata()->find("0020|0013");
								if (iter==img->getMetadata()->end()) {
									if (sort_type==INSTANCE) {
										std::cout << "Cannot find instance number tag (0020,0013) in " << *item << std::endl;
										/*std::stringstream ss;
										ss << *item << " cannot find instance number tag (0020,0013) in " << *item;
										pcl_ThrowException(ImageReaderException(), ss.str());*/
										throw 1;
									} else instance = count;
								} else {
									std::string str= std::string(img->getMetadata()->template getValue<std::string>(iter).c_str()); //This is to remove possible null character in the end of a DICOM string
									boost::trim(str);
									try {
										instance = boost::lexical_cast<int>(str);
									} catch (...) {
										std::cout << "Failure to convert \"" << str << "\" to int" << std::endl;
										throw;
									}
								}
							} catch (...) {
								std::cout << "Assuming -1 instance number due to exception encountered when reading tag (0020,0013) in " << *item << std::endl;
								instance = -1;
							}
						}

						double key;
						double pos = getSlicePosition(img);
						switch (sort_type)
						{
						case INSTANCE:
							key = instance;
							break;
						case POSITION:
							key = pos;
							break;
						case ADDSEQ:
							key = count;
							break;
						}

						if (image_map.find(key)!=image_map.end()) {
							std::string type;
							switch (sort_type)
							{
							case INSTANCE:
								type = "instance";
								break;
							case POSITION:
								type = "position";
								break;
							}
							if (!ignore_duplicates) {
								pcl_ThrowException(ImageReaderException(), 
									"ERROR: File " + image_map.find(key)->second.filename + " and " + *item + " shared the same " + type + " " + boost::lexical_cast<std::string>(key));
							} else {
								std::cout << "Warning: File " 
									<< image_map.find(key)->second.filename + 
									" skipped as it shared the same " << type << " of " << boost::lexical_cast<std::string>(key) << " with " << *item 
									<< std::endl;
							}
						} else {
							image_map[key] = ImageInfo(img, *item, pos, instance);
							instance_pos_lookup[instance] = pos;
						}

						++count;
					}
				} catch (ImageReaderException e) {
					throw;
				} catch (...) {
					std::cout << "Skipping: Unhandled exception occured when reading " << *item << std::endl;
				}
			}
			return image_map;
		}
	};

}

#endif