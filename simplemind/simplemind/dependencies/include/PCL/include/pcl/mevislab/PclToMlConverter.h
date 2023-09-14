#ifndef PCL_TO_ML_CONVERTER
#define PCL_TO_ML_CONVERTER

#include <mlModuleIncludes.h>
#include <pcl/image.h>
#include <pcl/mevislab/exception.h>

namespace pcl
{
	namespace mevislab
	{

		class PclToMlConverter
		{
		public:
			PclToMlConverter()
			{}

			void reset()
			{
				m_Image.reset();
			}

			template <class ImagePointerType>
			void reset(const ImagePointerType& image, bool correct_svs=true)
			{
				if (!image) {
					reset();
					return;
				}

				MLDataType datatype;
				if (image->isType(typeid(char))) datatype = MLint8Type;
				else if (image->isType(typeid(unsigned char))) datatype = MLuint8Type;
				else if (image->isType(typeid(short))) datatype = MLint16Type;
				else if (image->isType(typeid(unsigned short))) datatype = MLuint16Type;
				else if (image->isType(typeid(int))) datatype = MLint32Type;
				else if (image->isType(typeid(unsigned int))) datatype = MLuint32Type;
				else if (image->isType(typeid(long))) datatype = MLint64Type;
				else if (image->isType(typeid(unsigned long))) datatype = MLuint64Type;
				else if (image->isType(typeid(float))) datatype = MLfloatType;
				else if (image->isType(typeid(double))) datatype = MLdoubleType;
				else pcl_ThrowException(UnsupportedTypeException(), std::string("Unsupported image type ")+typeid(*image).name()+" encountered!");

				pcl::ImageIteratorWithPoint iter(image);
				auto minmax = pcl::ImageHelper::GetMinMax(image, iter);
				setMinValue(minmax.get<0>());
				setMaxValue(minmax.get<1>());

				if (image->getMinPoint()==Point3D<int>(0,0,0)) set(image, datatype);
				else set(image->getAlias(pcl::Point3D<int>(0,0,0), true), datatype);
			}

			void setMinValue(double val)
			{
				m_MinValue = val;
			}

			void setMaxValue(double val)
			{
				m_MaxValue = val;
			}

			double getMinValue() const
			{
				return m_MinValue;
			}

			double getMaxValue() const
			{
				return m_MaxValue;
			}

			bool setOutputImageProperties(ml::PagedImage* outputImage, MLDataType datatype)
			{
				if (!m_Image) return false;

				outputImage->getImagePropertyContainer().clear();
				outputImage->setDataType(datatype);
				outputImage->setImageExtent(m_Image->getSize().x(), m_Image->getSize().y(), m_Image->getSize().z(), 1, 1, 1);

				outputImage->setMinVoxelValue(m_MinValue);
				outputImage->setMaxVoxelValue(m_MaxValue);
				outputImage->setVoxelToWorldMatrix(m_WorldMatrix);

				outputImage->getCDimensionInfos().clear();
				outputImage->getCDimensionInfos().push_back(ML_LUMINANCE);
				return true;
			}

			bool setOutputImageProperties(ml::PagedImage* outputImage)
			{
				return setOutputImageProperties(outputImage, m_DataType);
			}

			const pcl::ImageBase::ConstantPointer& getImage() const
			{
				return m_Image;
			}

			MLDataType getDataType() const
			{
				return m_DataType;
			}

		protected:
			pcl::ImageBase::ConstantPointer m_Image;
			ml::Matrix4 m_WorldMatrix;
			MLDataType m_DataType;
			double m_MinValue, m_MaxValue;

			template <class ImagePointerType>
			void set(const ImagePointerType& image, MLDataType type/*, bool correct_svs*/)
			{
				m_Image = image;
				m_DataType = type;

				// Get location, voxel scaling and orientation of itk image or importer.
				const auto &spacing = image->getSpacing();
				const auto &origin = image->toPhysicalCoordinate(image->getMinPoint());
				const auto &orientation = image->getOrientationMatrix();

				// Create the ML world matrix as id and set up scaling, orientation and origin provided by the itk image or importer.
				ml::Matrix4 mat = ml::Matrix4::getIdentity();

				// Compose scale, directionCosines and translation to the new ML world matrix.
				mat[0][0] = orientation(0,0)*spacing[0];
				mat[1][0] = orientation(1,0)*spacing[0];
				mat[2][0] = orientation(2,0)*spacing[0];
				mat[3][0] = 0;

				mat[0][1] = orientation(0,1)*spacing[1];
				mat[1][1] = orientation(1,1)*spacing[1];
				mat[2][1] = orientation(2,1)*spacing[1];
				mat[3][1] = 0;

				mat[0][2] = orientation(0,2)*spacing[2];
				mat[1][2] = orientation(1,2)*spacing[2];
				mat[2][2] = orientation(2,2)*spacing[2];
				mat[3][2] = 0;

				mat[0][3] = origin[0];
				mat[1][3] = origin[1];
				mat[2][3] = origin[2];
				mat[3][3] = 1;

				ml::MedicalImageProperties props;
				props.setVoxelToWorldMatrix(mat);

				// Subtract half voxel shift - in MedicalImageProperties the matrix considers
				// voxel positions at the corner and not in the center.
				//if (correct_svs) props.translateVoxelToWorldMatrix(ml::Vector3(-0.5, -0.5, -0.5));
				props.translateVoxelToWorldMatrix(ml::Vector3(-0.5, -0.5, -0.5));

				m_WorldMatrix = props.getVoxelToWorldMatrix();
			}
		};

	}
}

#endif