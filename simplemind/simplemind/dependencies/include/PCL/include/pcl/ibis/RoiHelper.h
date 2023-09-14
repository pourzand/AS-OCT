#ifndef PCL_ROI_HELPER
#define PCL_ROI_HELPER

#include <pcl/image.h>
#include <pcl/exception.h>
#include <pcl/misc/FileStreamHelper.h>
#include <iostream>
#include <vector>
#include <sstream>

namespace pcl
{
	namespace ibis
	{
		using namespace pcl;

		class RoiHelper
		{
		public:
			//================== Reading ROI ==================
			template <class ImageType>
			static typename ImageType::Pointer Read(std::istream& is, const typename ImageType::IoValueType& fg_val=1, const typename ImageType::IoValueType& bg_val=0)
			{
				std::vector<char> buffer;
				Region3D<int> region;
				region.reset();
				//Computing the region needed and storing the values for later
				std::vector<int> values;
				try {
					int slice_num = ReadTillNextSpace(is,buffer);
					values.push_back(slice_num);
					for (int s=0; s<slice_num; s++) {
						int z = ReadTillNextSpace(is,buffer), 
							line_num = ReadTillNextSpace(is,buffer);
						values.push_back(z);
						values.push_back(line_num);
						for (int l=0; l<line_num; l++) {
							int y = ReadTillNextSpace(is,buffer), 
								interval_num = ReadTillNextSpace(is,buffer);
							values.push_back(y);
							values.push_back(interval_num);
							for (int i=0; i<interval_num; i++) {
								int x_start = ReadTillNextSpace(is,buffer), 
									x_end = ReadTillNextSpace(is,buffer);
								if (x_start>x_end) pcl_Swap(x_start, x_end);
								values.push_back(x_start);
								values.push_back(x_end);
								region.add(Point3D<int>(x_start,y,z));
								region.add(Point3D<int>(x_end,y,z));
							}
						}
					}
				} catch (int) {
					pcl_ThrowException(pcl::Exception(), "Termination occured unexpectedly");
				}
				if (region.empty()) return typename ImageType::Pointer();
				//Constructing the actual image
				auto image = ImageType::New(region.getMinPoint(), region.getMaxPoint());
				ImageHelper::Fill(image, bg_val);
				auto values_iter = values.begin();
				int slice_num = *values_iter; ++values_iter;
				for (int s=0; s<slice_num; s++) {
					int z = *values_iter; ++values_iter;
					int line_num = *values_iter; ++values_iter;
					for (int l=0; l<line_num; l++) {
						int y = *values_iter; ++values_iter;
						int interval_num = *values_iter; ++values_iter;
						for (int i=0; i<interval_num; i++) {
							int x_start = *values_iter; ++values_iter;
							int x_end = *values_iter; ++values_iter;
							if (x_start>x_end) pcl_Swap(x_start, x_end);
							for (int x=x_start; x<=x_end; x++) {
								image->set(x,y,z, fg_val);
							}
						}
					}
				}
				return image;
			}
			template <class ImageType>
			static typename ImageType::Pointer Read(const std::string& file, const typename ImageType::IoValueType& fg_val=1, const typename ImageType::IoValueType& bg_val=0)
			{
				auto is = pcl::FileStreamHelper::CreateIfstream(file);
				auto result = Read<ImageType>(is, fg_val, bg_val);
				is.close();
				return result;
			}

			template <class ImagePointerType>
			static Region3D<int> Fill(const ImagePointerType& image, std::istream& is, const typename ImagePointerType::element_type::IoValueType& val=1)
			{
				Region3D<int> region;
				region.reset();
				std::vector<char> buffer;
				Point3D<int> temp;
				try {
					int slice_num = ReadTillNextSpace(is,buffer);
					for (int s=0; s<slice_num; s++) {
						int z = ReadTillNextSpace(is,buffer), 
							line_num = ReadTillNextSpace(is,buffer);
						for (int l=0; l<line_num; l++) {
							int y = ReadTillNextSpace(is,buffer), 
								interval_num = ReadTillNextSpace(is,buffer);
							for (int i=0; i<interval_num; i++) {
								int x_start = ReadTillNextSpace(is,buffer), 
									x_end = ReadTillNextSpace(is,buffer);
								if (x_start>x_end) pcl_Swap(x_start, x_end);
								region.add(temp.set(x_start,y,z));
								region.add(temp.set(x_end,y,z));
								for (int x=x_start; x<=x_end; x++) {
									if (image->contain(x,y,z)) {
										image->set(x,y,z, val);
									}
								}
							}
						}
					}
				} catch (int) {
					pcl_ThrowException(pcl::Exception(), "Termination occured unexpectedly");
				}
				return region;
			}
			template <class ImagePointerType>
			static Region3D<int> Fill(const ImagePointerType& image, const std::string& file, const typename ImagePointerType::element_type::IoValueType& val=1)
			{
				auto is = pcl::FileStreamHelper::CreateIfstream(file);
				Region3D<int> region = Fill(image, is, val);
				is.close();
				return region;
			}


		protected:
			static int ReadTillNextSpace(std::istream& is, std::vector<char>& buffer)
			{
				buffer.clear();
				
				//TODO pending testing.
				//is.get();
				char c = const_cast<std::istream&>(is).get();
				
				//Initializing (skipping spaces)
				while (is.good()) {
					if (c!=' ') {
						buffer.push_back(c);
						break;
					}
					c = is.get();
				}
				//Actual reading
				if (!is.good()) {
					throw 1;
				}
				c = is.get();
				while (!is.eof()) {
					if (c==' ') break;
					buffer.push_back(c);
					c = is.get();
				}
				buffer.push_back(0);
				return atoi(&buffer[0]);
			}

		public:
			//================== Writing ROI ==================
			template <class ImagePointerType>
			static std::ostream& Write(std::ostream& os, const ImagePointerType& image) 
			{
				return Write(os, image, [&](const PointIndexObject& pi)->bool {
					return image->get(pi);
				});
			}
			template <class ImagePointerType>
			static void Write(const std::string& file, const ImagePointerType& image) 
			{
				auto os = pcl::FileStreamHelper::CreateOfstream(file);
				Write(os, image);
				os.close();
			}

			template <class ImagePointerType>
			static std::ostream& Write(std::ostream& os, const ImagePointerType& image, const Region3D<int>& process_region) 
			{
				return Write(os, image, process_region, [&](const PointIndexObject& pi)->bool {
					return image->get(pi);
				});
			}
			template <class ImagePointerType>
			static void Write(const std::string& file, const ImagePointerType& image, const Region3D<int>& process_region) 
			{
				auto os = pcl::FileStreamHelper::CreateOfstream(file);
				Write(os, image, process_region);
				os.close();
			}

			template <class ImagePointerType, class DecisionFunc>
			static std::ostream& Write(std::ostream& os, const ImagePointerType& image, DecisionFunc func) 
			{
				return Write(os, image, image->getRegion(), func);
			}
			template <class ImagePointerType, class DecisionFunc>
			static void Write(const std::string& file, const ImagePointerType& image, DecisionFunc func) 
			{
				Write(file, image, image->getRegion(), func);
			}

			template <class ImagePointerType, class DecisionFunc>
			static std::ostream& Write(std::ostream& os, const ImagePointerType& image, const Region3D<int>& process_region, DecisionFunc func) 
			{
				if (!image) {
					os << "0";
					return os;
				}

				Region3D<int> region(process_region);
				region.setIntersect(image->getRegion());

				int x_offset = image->getOffsetTable()[0],
					y_offset = image->getOffsetTable()[1],
					z_offset = image->getOffsetTable()[2];

				PointIndexObject pi;
				int slice_count = 0;
				std::stringstream final_buffer;
				try {
					auto exception_obj = pcl::StreamExceptionHelper::GetStreamExceptionObject(os, std::ios_base::failbit | std::ios_base::badbit);
					if (!region.empty()) {
						int z_index = image->localToIndex(region.getMinPoint());
						for (int z=region.getMinPoint().z(); z<=region.getMaxPoint().z(); z++) {
							std::stringstream z_buffer;
							int line_count = 0;
							int y_index = z_index;
							for (int y=region.getMinPoint().y(); y<=region.getMaxPoint().y(); y++) {
								std::stringstream y_buffer;
								int interval_count = 0;
								bool start_found = false;
								int x_index = y_index;
								for (int x=region.getMinPoint().x(); x<=region.getMaxPoint().x(); x++) {
									pi.point.set(x,y,z);
									pi.index = x_index;
									if (func(pi)) {
										if (!start_found) {
											start_found = true;
											y_buffer << " " << x;
											interval_count++;
										}
									} else {
										if (start_found) {
											start_found = false;
											y_buffer << " " << x-1;
										}
									}
									x_index += x_offset;
								}
								if (start_found) {
									y_buffer << " " << region.getMaxPoint().x();
								}
								if (interval_count>0) {
									z_buffer << " " << y << " " << interval_count << y_buffer.str();
									line_count++;
								}
								y_index += y_offset;
							}
							if (line_count>0) {
								final_buffer << " " << z << " " << line_count << z_buffer.str();
								slice_count++;
							}
							z_index += z_offset;
						}
					}
					os << slice_count << final_buffer.str();
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(pcl::Exception(), e.what());
				} 
				return os;
			}
			template <class ImagePointerType, class DecisionFunc>
			static void Write(const std::string& file, const ImagePointerType& image, const Region3D<int>& process_region, DecisionFunc func) 
			{
				auto os = pcl::FileStreamHelper::CreateOfstream(file);
				Write(os, image, process_region, func);
				os.close();
			}
		};

	}
}

#endif