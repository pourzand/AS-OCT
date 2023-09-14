#ifndef PCL_TERRAIN_MAP_NORMAL_COD_CONVERTER
#define PCL_TERRAIN_MAP_NORMAL_COD_CONVERTER

#include <pcl/cod/CodBase.h>
#include <pcl/terrain_map.h>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;

		template <class ImageType>
		class TerrainMapNormalCodConverter: public CodBase<false>
		{
		public:
			typedef TerrainMapNormalCodConverter Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef Point3D<double> ReturnType;

			static Pointer New(const typename ImageType::ConstantPointer& image)
			{
				if (!image) {
#ifndef NO_WARNING
					std::cout << "Warning at TerrainMapNormalCodConverter::New(): Empty image object provided, a NULL pointer is returned!" << std::endl;
#endif
					return Pointer();
				}
				Pointer obj(new Self);
				obj->setImage(image);
				return obj;
			}

			template <class IteratorType>
			inline ReturnType get(const IteratorType& iter) const
			{
				return m_Image->getTerrainPhysicalCoordinate(iter.getPoint(), iter);
			}

			inline ReturnType get(const Point3D<int>& p) const
			{
				return m_Image->getTerrainPhysicalCoordinate(p);
			}

			inline ReturnType get(const Point3D<int>& p, long index) const
			{
				return m_Image->getTerrainPhysicalCoordinate(p, index);
			}

			DummyImage::ConstantPointer getTemplateImage() const
			{
				return DummyImage::New(m_Image);
			}

		protected:
			typename ImageType::ConstantPointer m_Image;

			TerrainMapNormalCodConverter() {}

			void setImage(const typename ImageType::ConstantPointer& image) 
			{
				m_Image = image;
				setRegionInfo(m_Image->toIndex(m_Image->getMinPoint()), m_Image->getRegion());
			}
		};

	}
}

#endif