#ifndef PCL_PHYSICAL_IMAGE_BASE
#define PCL_PHYSICAL_IMAGE_BASE

#include <pcl/image/ImageBase.h>
#include <pcl/exception.h>

#ifndef NO_VNL
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_inverse.h>
#endif

namespace pcl
{

	template <bool Flag>
	class ImagePhysicalLayer: public ImageBase
	{
	public:
		enum { UseOrientationMatrix = Flag };
		typedef ImagePhysicalLayer Self;
		typedef boost::shared_ptr< Self > Pointer;
		typedef boost::shared_ptr< Self const > ConstantPointer;
		typedef vnl_matrix_fixed<double,3,3> OrientationMatrixType;
		
		inline OrientationMatrixType getOrientationMatrix() const
		{
			OrientationMatrixType temp;
			temp.set_identity();
			return temp;
		}
		
		inline OrientationMatrixType getInverseOrientationMatrix() const
		{
			OrientationMatrixType temp;
			temp.set_identity();
			return temp;
		}
		
		inline Point3D<double> getImageAxis(int i) const
		{
			Point3D<double> result(0,0,0);
			result[i] = 1;
			return result;
		}
		
		inline Region3D<double> getPhysicalRegion() const
		{
			return Region3D<double>(toPhysicalCoordinate(getMinPoint()), toPhysicalCoordinate(getMaxPoint()));
		}
		
		inline Region3D<double> getPhysicalRegion(const Region3D<int>& region) const
		{
			return Region3D<double>(toPhysicalCoordinate(region.getMinPoint()), toPhysicalCoordinate(region.getMaxPoint()));
		}
		
		inline Region3D<double> getImageRegionFromPhysical(const Region3D<double>& region) const
		{
			return Region3D<double>(toImageCoordinate(region.getMinPoint()), toImageCoordinate(region.getMaxPoint()));
		}

		/******** Coordinates conversion ********/
		inline Point3D<double> toPhysicalVector(double x, double y, double z) const
		{
			return Point3D<double>(
				(x * this->getSpacing().x()),
				(y * this->getSpacing().y()),
				(z * this->getSpacing().z())
				);
		}
		template <class PT>
		inline Point3D<double> toPhysicalVector(const PT& p) const
		{
			return toPhysicalVector(p[0], p[1], p[2]);
		}
		inline Point3D<double> toPhysicalCoordinate(double x, double y, double z) const
		{
			return Point3D<double>(
				this->getOrigin().x() + (x * this->getSpacing().x()),
				this->getOrigin().y() + (y * this->getSpacing().y()),
				this->getOrigin().z() + (z * this->getSpacing().z())
				);
		}
		template <class PT>
		inline Point3D<double> toPhysicalCoordinate(const PT& p) const
		{
			return this->toPhysicalCoordinate(p[0], p[1], p[2]);
		}

		inline Point3D<double> toImageVector(double x, double y, double z) const
		{
			return Point3D<double>(
				x*this->m_OneOverSpacing.x(),
				y*this->m_OneOverSpacing.y(),
				z*this->m_OneOverSpacing.z()
				);
		}
		template <class PT>
		inline Point3D<double> toImageVector(const PT& p) const
		{
			return this->toImageVector(p[0], p[1], p[2]);
		}
		inline Point3D<double> toImageCoordinate(double x, double y, double z) const
		{
			return Point3D<double>(
				(x - this->getOrigin().x())*this->m_OneOverSpacing.x(),
				(y - this->getOrigin().y())*this->m_OneOverSpacing.y(),
				(z - this->getOrigin().z())*this->m_OneOverSpacing.z()
				);
		}
		template <class PT>
		inline Point3D<double> toImageCoordinate(const PT& p) const
		{
			return this->toImageCoordinate(p[0], p[1], p[2]);
		}

	protected:
		ImagePhysicalLayer() {}	
		
        void setOrientationMatrix(const OrientationMatrixType& /*mat*/)
		{
/*#ifndef NO_WARNING
			std::cout << "Warning at ImagePhysicalLayer::setOrientationMatrix(): Called despite UseOrientationMatrix is false!\n";
#endif*/
		}
		
        void setOrientationMatrix(const OrientationMatrixType& /*mat*/, const OrientationMatrixType& /*inv_mat*/)
		{
/*#ifndef NO_WARNING
			std::cout << "Warning at ImagePhysicalLayer::setOrientationMatrix(): Called despite UseOrientationMatrix is false!\n";
#endif*/
		}
		
		template <class ImagePointerType>
		void copyInfo(const ImagePointerType& obj)
		{
			m_Spacing = obj->getSpacing();
			m_OneOverSpacing = obj->getOneOverSpacing();
			m_Origin = obj->getOrigin();
			m_Size = obj->getSize();
			m_Region = obj->getRegion();
			m_Offset = obj->getBufferOffset();
			m_OffsetTable = obj->getOffsetTable();
		}
	};
	
	template <>
	class ImagePhysicalLayer<true>: public ImageBase
	{
	public:
		enum { UseOrientationMatrix = true };
		typedef ImagePhysicalLayer Self;
		typedef boost::shared_ptr< Self > Pointer;
		typedef boost::shared_ptr< Self const > ConstantPointer;
		typedef vnl_matrix_fixed<double,3,3> OrientationMatrixType;

		inline const OrientationMatrixType& getOrientationMatrix() const
		{
			return m_OrientationMatrix;
		}
		
		inline const OrientationMatrixType& getInverseOrientationMatrix() const
		{
			return m_InverseOrientationMatrix;
		}
		
		inline Point3D<double> getImageAxis(int i) const
		{
			return Point3D<double>(
				m_OrientationMatrix(0,i),
				m_OrientationMatrix(1,i),
				m_OrientationMatrix(2,i)
				);
		}
		
		inline Region3D<double> getPhysicalRegion() const
		{
			auto points = getRegion().getCornerPoints();
			Region3D<double> result;
			result.reset();
			pcl_ForEach(points, p) result.add(toPhysicalCoordinate(*p));
			return result;
		}
		
		inline Region3D<double> getPhysicalRegion(const Region3D<int>& region) const
		{
			auto points = region.getCornerPoints();
			Region3D<double> result;
			result.reset();
			pcl_ForEach(points, p) result.add(toPhysicalCoordinate(*p));
			return result;
		}
		
		inline Region3D<double> getImageRegionFromPhysical(const Region3D<double>& region) const
		{
			auto points = region.getCornerPoints();
			Region3D<double> result;
			result.reset();
			pcl_ForEach(points, p) result.add(toImageCoordinate(*p));
			return result;
		}

		/******** Coordinates conversion ********/
		template <class PT>
		inline Point3D<double> toPhysicalVector(const PT& p) const
		{
			double tx = p[0]*this->getSpacing().x(),
				ty = p[1]*this->getSpacing().y(),
				tz = p[2]*this->getSpacing().z();
			return Point3D<double>(
				m_OrientationMatrix(0,0)*tx + m_OrientationMatrix(0,1)*ty + m_OrientationMatrix(0,2)*tz,
				m_OrientationMatrix(1,0)*tx + m_OrientationMatrix(1,1)*ty + m_OrientationMatrix(1,2)*tz,
				m_OrientationMatrix(2,0)*tx + m_OrientationMatrix(2,1)*ty + m_OrientationMatrix(2,2)*tz
				);
		}
		inline Point3D<double> toPhysicalVector(double x, double y, double z) const
		{
			return toPhysicalVector(Point3D<double>(x,y,z));
		}
		inline Point3D<double> toPhysicalCoordinate(double x, double y, double z) const
		{
			double tx = x*this->getSpacing().x(),
				ty = y*this->getSpacing().y(),
				tz = z*this->getSpacing().z();
			return Point3D<double>(
				this->getOrigin().x() + (m_OrientationMatrix(0,0)*tx + m_OrientationMatrix(0,1)*ty + m_OrientationMatrix(0,2)*tz),
				this->getOrigin().y() + (m_OrientationMatrix(1,0)*tx + m_OrientationMatrix(1,1)*ty + m_OrientationMatrix(1,2)*tz),
				this->getOrigin().z() + (m_OrientationMatrix(2,0)*tx + m_OrientationMatrix(2,1)*ty + m_OrientationMatrix(2,2)*tz)
				);
		}
		template <class PT>
		inline Point3D<double> toPhysicalCoordinate(const PT& p) const
		{
			return this->toPhysicalCoordinate(p[0], p[1], p[2]);
		}

		template <class PT>
		inline Point3D<double> toImageVector(const PT& p) const
		{
			return Point3D<double>(
				(m_InverseOrientationMatrix(0,0)*p[0] + m_InverseOrientationMatrix(0,1)*p[1] + m_InverseOrientationMatrix(0,2)*p[2])*this->getSpacing().x(),
				(m_InverseOrientationMatrix(1,0)*p[0] + m_InverseOrientationMatrix(1,1)*p[1] + m_InverseOrientationMatrix(1,2)*p[2])*this->getSpacing().y(),
				(m_InverseOrientationMatrix(2,0)*p[0] + m_InverseOrientationMatrix(2,1)*p[1] + m_InverseOrientationMatrix(2,2)*p[2])*this->getSpacing().z()
				);
		}
		inline Point3D<double> toImageVector(double x, double y, double z) const
		{
			return toImageVector(Point3D<double>(x,y,z));
		}
		inline Point3D<double> toImageCoordinate(double x, double y, double z) const
		{
			double tx = x - this->getOrigin().x(),
				ty = y - this->getOrigin().y(),
				tz = z - this->getOrigin().z();
			return Point3D<double>(
				(m_InverseOrientationMatrix(0,0)*tx + m_InverseOrientationMatrix(0,1)*ty + m_InverseOrientationMatrix(0,2)*tz)*this->m_OneOverSpacing.x(),
				(m_InverseOrientationMatrix(1,0)*tx + m_InverseOrientationMatrix(1,1)*ty + m_InverseOrientationMatrix(1,2)*tz)*this->m_OneOverSpacing.y(),
				(m_InverseOrientationMatrix(2,0)*tx + m_InverseOrientationMatrix(2,1)*ty + m_InverseOrientationMatrix(2,2)*tz)*this->m_OneOverSpacing.z()
				);
		}
		template <class PT>
		inline Point3D<double> toImageCoordinate(const PT& p) const
		{
			return this->toImageCoordinate(p[0], p[1], p[2]);
		}

	protected:
		OrientationMatrixType m_OrientationMatrix,
			m_InverseOrientationMatrix;
		
		ImagePhysicalLayer() 
		{
			m_OrientationMatrix.set_identity();
			m_InverseOrientationMatrix.set_identity();
		}
		
		void setOrientationMatrix(const OrientationMatrixType& mat) 
		{
			m_OrientationMatrix = mat;
			m_InverseOrientationMatrix = vnl_inverse(m_OrientationMatrix);
		}
		void setOrientationMatrix(const OrientationMatrixType& mat, const OrientationMatrixType& inv_mat) 
		{
			m_OrientationMatrix = mat;
			m_InverseOrientationMatrix = inv_mat;
		}
		
		template <class ImagePointerType>
		void copyInfo(const ImagePointerType& obj)
		{
			m_Spacing = obj->getSpacing();
			m_OneOverSpacing = obj->getOneOverSpacing();
			m_Origin = obj->getOrigin();
			m_Size = obj->getSize();
			m_Region = obj->getRegion();
			m_Offset = obj->getBufferOffset();
			m_OffsetTable = obj->getOffsetTable();
			m_OrientationMatrix = obj->getOrientationMatrix();
			m_InverseOrientationMatrix = obj->getInverseOrientationMatrix();
		}
	};

}

#endif
