#ifndef PCL_IMAGE_FILE_INFO
#define PCL_IMAGE_FILE_INFO

#include <pcl/misc/FileNameTokenizer.h>
#include <typeinfo>
#include <ostream>

namespace pcl
{

	struct ImageFileInfo
	{
		const std::type_info& type;
		int dimension;
		int component_size;
		std::string extension;
		
		template <class ComponentType>
		static ImageFileInfo New(const std::string& filename, int dimension)
		{
			FileNameTokenizer fname(filename);
			return ImageFileInfo(typeid(ComponentType), dimension, sizeof(ComponentType), FileNameTokenizer(filename).getExtensionWithoutDot());
		}

        ImageFileInfo(const std::type_info& t, int d, int s, const std::string& e): type(t), dimension(d), component_size(s), extension(e) {}

		friend std::ostream& operator<<(std::ostream& os, const ImageFileInfo& obj)
		{
			os << "Extension: " << obj.extension << std::endl;
			os << "Type: " << obj.type.name() << std::endl;
			os << "Dimension: " << obj.dimension << std::endl;
			os << "Component size: " << obj.component_size << std::endl;
			return os;
		}
	};

}

#endif
