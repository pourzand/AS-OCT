#ifndef PCL_TERRAIN_MAP
#define PCL_TERRAIN_MAP

#include <pcl/image.h>
#include <vnl/vnl_matrix_fixed.h>

namespace pcl
{

	/*!
		A class for the representation of a terrain map. 
		Conversion to and fro from image coordinate, presented in the form of (x,y,h), to the physical coordinate is provided using the same interface as the common image class.
	*/
	template <class Type>
	class TerrainMap: public Image<Type, true>
	{
	public:
		typedef TerrainMap Self;
		typedef Image<Type,true> Parent;
		typedef boost::shared_ptr<Self> Pointer;
		typedef boost::shared_ptr<Self const> ConstantPointer;

		static Pointer New(int sx, int sy, const Point3D<int>& minp=Point3D<int>(0,0,0), const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0))
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(sx, sy, 1); 
			obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
			obj->setBuffer(BufferType::New(obj->m_Size.x(), obj->m_Size.y(), obj->m_Size.z())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			return obj; 
		}
		static Pointer New(int sx, int sy, const OrientationMatrixType& matrix, const Point3D<int>& minp=Point3D<int>(0,0,0), const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0))
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(sx, sy, 1); 
			obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
			obj->setBuffer(BufferType::New(obj->m_Size.x(), obj->m_Size.y(), obj->m_Size.z())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin);
			obj->setOrientationMatrix(matrix);
			return obj; 
		}

		static Pointer New(typename BufferType::Pointer& buffer, const Point3D<int>& minp=Point3D<int>(0,0,0), const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0))
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(buffer->getSize().x(), buffer->getSize().y(), 1); 
			obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
			obj->setBuffer(buffer);
			obj->setSpacing(spacing); 
			obj->setOrigin(origin);
			return obj; 
		}
		static Pointer New(typename BufferType::Pointer& buffer, const OrientationMatrixType& matrix, const Point3D<int>& minp=Point3D<int>(0,0,0), const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0))
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(buffer->getSize().x(), buffer->getSize().y(), 1); 
			obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
			obj->setBuffer(buffer);
			obj->setSpacing(spacing); 
			obj->setOrigin(origin);
			obj->setOrientationMatrix(matrix);
			return obj; 
		}

		template <class InputPointerType>
		static Pointer New(typename InputPointerType& input)
		{
			Pointer result = New(input->getBufferSize().x(), input->getBufferSize().y(), input->getOrientationMatrix(), Point3D<int>(0,0,0), input->getSpacing(), input->getOrigin());
			result->m_Size.set(input->getSize().x(), input->getSize().y(), 1);
			result->setMinPoint(input->getMinPoint(), input->toBufferCoordinate(input->getMinPoint()));
			return result;
		}

		/************** Misc helper **************/
		static OrientationMatrixType ConstructOrientationMatrixFromUnitVectors(const Point3D<double>& vec_x, const Point3D<double>& vec_y, const Point3D<double>& vec_z)
		{
			OrientationMatrixType matrix;
			matrix.set_column(0, &vec_x[0]);
			matrix.set_column(1, &vec_y[0]);
			matrix.set_column(2, &vec_z[0]);
			return matrix;
		}

		/************** Terrain map information related **************/
		inline Point3D<double> getOrientationMatrixColumn(int i) const
		{
			return Point3D<double>(m_OrientationMatrix.get(0,i), m_OrientationMatrix.get(1,i), m_OrientationMatrix.get(2,i));
		}
		
		template <class PointType>
		PointType getCoordinate(int x, int y) const
		{
			return PointType(x,y, get(x,y,getMinPoint().z()));
		}
		template <class PointType>
		PointType getCoordinate(int x, int y, long index) const
		{
			return PointType(x,y, get(index));
		}
		template <class PointType, class PT>
		PointType getCoordinate(const PT& point, long index) const
		{
			return PointType(point.x(), point.y(), get(index));
		}

		template <class PointType, class IterType>
		typename boost::enable_if<getPoint_exists<IterType>, PointType>::type getCoordinate(const IterType& iter) const
		{
			return getCoordinate<PointType>(iter.getPoint(), iter);
		}
		template <class PointType, class PT>
		typename boost::disable_if<getPoint_exists<PT>, PointType>::type getCoordinate(const PT& point) const
		{
			return getCoordinate<PointType>(point.x(), point.y());
		}
		
		using Parent::toPhysicalCoordinate;
		template <class PT>
		inline Point3D<double> toPhysicalCoordinate(const PT& p, double val) const
		{
			return toPhysicalCoordinate(p[0], p[1], val);
		}

		/************** Aliasing and subimage methods **************/
		typename Pointer getAlias(const Point3D<int>& minp, bool fix_physical)
		{
			if (fix_physical) {
				return getAlias(minp, this->getSpacing(), 
					toPhysicalCoordinate(this->getMinPoint().x(), this->getMinPoint().y(), 0) - toPhysicalVector(minp.x(), minp.y(), 0)
					);
			} else return getAlias(minp, this->getSpacing(), this->getOrigin());
		}
		typename Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing)
		{
			return getAlias(minp, spacing, this->getOrigin());
		}
		typename Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin)
		{
			Pointer obj(new Self());
			obj->m_Size = this->getSize(); 
			obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return obj;
		}
		typename Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix)
		{
			Pointer obj(new Self());
			obj->m_Size = this->getSize(); 
			obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(matrix);
			return obj;
		}
		
		typename ConstantPointer getAlias(const Point3D<int>& minp, bool fix_physical) const
		{
			if (fix_physical) {
				return getAlias(minp, this->getSpacing(), 
					toPhysicalCoordinate(this->getMinPoint().x(), this->getMinPoint().y(), 0) - toPhysicalVector(minp.x(), minp.y(), 0)
					);
			} else return getAlias(minp, this->getSpacing(), this->getOrigin());
		}
		typename ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing) const
		{
			return getAlias(minp, spacing, this->getOrigin());
		}
		typename ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin) const
		{
			Pointer obj(new Self());
			obj->m_Size = this->getSize(); 
			obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return getAlias(minp, spacing, origin, this->getBasisMatrix());
		}
		typename ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) const
		{
			Pointer obj(new Self());
			obj->m_Size = this->getSize(); 
			obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(matrix);
			return obj;
		}

		typename Pointer getSubImage(const Point3D<int>& minp, const Point3D<int>& maxp)
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1);
			obj->m_Size.min(this->getSize());
			obj->setMinPoint(minp, this->toBufferCoordinate(minp));			
			obj->setSpacing(spacing); 
			obj->setOrigin(origin); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return obj;
		}
		typename ConstantPointer getSubImage(const Point3D<int>& minp, const Point3D<int>& maxp) const
		{
			Pointer obj(new Self()); 
			obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1);
			obj->m_Size.min(this->getSize()); 
			obj->setMinPoint(minp, this->toBufferCoordinate(minp)); 
			obj->setSpacing(this->getSpacing()); 
			obj->setOrigin(this->toPhysicalCoordinate(minp)); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return obj;
		}
		
		typename Self::Pointer getWholeImage()
		{
			Pointer obj(new Self()); 
			obj->m_Size = getBufferSize();
			Point3D<int> minp = getMinPoint();
			minp -= toBufferCoordinate(minp);
			obj->setMinPoint(minp, Point3D<int>(0,0,0));
			obj->setSpacing(getSpacing()); 
			obj->setOrigin(getOrigin()); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return obj;
		}
		typename Self::ConstantPointer getWholeImage() const
		{
			Pointer obj(new Self()); 
			obj->m_Size = getBufferSize();
			Point3D<int> minp = getMinPoint();
			minp -= toBufferCoordinate(minp);
			obj->setMinPoint(minp, Point3D<int>(0,0,0));
			obj->setSpacing(getSpacing()); 
			obj->setOrigin(getOrigin()); 
			obj->setBuffer(this->m_Buffer);
			obj->setOrientationMatrix(getOrientationMatrix(), getInverseOrientationMatrix());
			return obj;
		}

	protected:
		TerrainMap() {}
	};

}

#endif