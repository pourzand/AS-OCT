#ifndef ML_PAGED_IMAGE_WRAPPER
#define ML_PAGED_IMAGE_WRAPPER

#include <pcl/image/ImagePhysicalLayer.h>
#include <pcl/image/ImageAlgorithmObject.h>
#include <mlTVirtualVolume.h>

namespace pcl
{
	namespace mevislab
	{

		template <class Type, bool Flag=true>
		class MlPagedImageWrapper: public ImagePhysicalLayer<Flag>
		{
		public:
			enum {
				ItkAliasable = false
			};
			typedef MlPagedImageWrapper Self;
			typedef ImagePhysicalLayer<Flag> Parent;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;
			typedef Type ValueType;
			typedef ValueType ConstantValueType;
			typedef ValueType IoValueType;
			typedef typename Parent::OrientationMatrixType OrientationMatrixType;

			static ConstantPointer New(ml::PagedImage* img, const Point3D<int>& minp=Point3D<int>(0,0,0), /*bool correct_svs=true,*/ int u=0, int t=0, int c=0)
			{
				ml::MedicalImageProperties props;
				props.setVoxelToWorldMatrix(img->getVoxelToWorldMatrix());
				props.translateVoxelToWorldMatrix(ml::Vector3(0.5, 0.5, 0.5));
				ml::Matrix4 mat = props.getVoxelToWorldMatrix();
				Point3D<double> origin, spacing;
				OrientationMatrixType orientation_matrix;
				for (int i=0; i<3; ++i) {
					Point3D<double> temp;
					for (int j=0; j<3; ++j) temp[j] = mat[j][i];
					spacing[i] = temp.getNorm();
					temp /= spacing[i];
					for (int j=0; j<3; ++j) orientation_matrix(j,i) = temp[j];
					origin[i] = mat[i][3];
				}

				return New(img, minp, spacing, origin, orientation_matrix, u, t, c);
			}

			static ConstantPointer New(ml::PagedImage* img, const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& orientation_matrix, int u=0, int t=0, int c=0)
			{
				return ConstantPointer(new MlPagedImageWrapper(img, minp, spacing, origin, orientation_matrix, u, t, c));
			}

			/******** Aliasing and subimage methods ********/
			typename Self::ConstantPointer getAlias(const Point3D<int>& minp, bool fix_physical) const
			{
				if (fix_physical) {
					return getAlias(minp, this->getSpacing(), 
						this->toPhysicalCoordinate(this->getMinPoint()) - this->toPhysicalVector(minp)
						);
				} else return getAlias(minp, this->getSpacing(), this->getOrigin());
			}
			typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing) const
			{
				return this->getAlias(minp, spacing, this->getOrigin());
			}
			typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin) const
			{
				Pointer obj(new Self()); 
				obj->m_Size = this->getSize(); 
				obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
				obj->setSpacing(spacing); 
				obj->setOrigin(origin); 
				obj->setVirtualVolume(this);
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
				return obj;
			}
			typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix)
			{
				Pointer obj(new Self()); 
				obj->m_Size = this->getSize(); 
				obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
				obj->setSpacing(spacing); 
				obj->setOrigin(origin); 
				obj->setVirtualVolume(this);
				obj->setOrientationMatrix(matrix);
				return obj;
			}

			typename Self::ConstantPointer getSubImage(const Point3D<int>& input_minp, const Point3D<int>& input_maxp) const
			{
				Pointer obj(new Self()); 
				Point3D<int> minp(input_minp), maxp(input_maxp);
				minp.min(this->getMaxPoint()).max(this->getMinPoint());
				maxp.min(this->getMaxPoint()).max(this->getMinPoint());
				obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1);
				obj->setMinPoint(minp, this->toBufferCoordinate(minp)); 
				obj->setSpacing(this->getSpacing()); 
				obj->setOrigin(this->getOrigin()); 
				obj->setVirtualVolume(this);
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
				return obj;
			}

			typename Self::ConstantPointer getWholeImage() const
			{
				Pointer obj(new Self()); 
				obj->m_Size = this->getCod()->getRegion().getSize();
				Point3D<int> cod_minp = this->m_CodBuffer->getCodBufferMinPoint();
				Point3D<int> minp = this->getMinPoint();
				minp -= this->toBufferCoordinate(minp);
				minp += cod_minp; //Compensate for coverage of COD object 
				obj->setMinPoint(minp, cod_minp);
				obj->setSpacing(this->getSpacing()); 
				obj->setOrigin(this->getOrigin()); 
				obj->setVirtualVolume(this);
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
				return obj;
			}

			/******** Value get/set methods ********/
			template <class Iterator>
			inline typename boost::disable_if_c<getPoint_exists<Iterator>::value, ConstantValueType>::type get(const Iterator& iter) const
			{
				return get(this->localToPoint(iter));
			}
			template <class Iterator>
			inline typename boost::enable_if_c<getPoint_exists<Iterator>::value, ConstantValueType>::type get(const Iterator& iter) const
			{
				return get(iter.getPoint());
			}

			inline ConstantValueType get(int x, int y, int z) const
			{
				const Point3D<int>& coord = this->toBufferCoordinate(x,y,z);
				ml::ImageVector p(coord.x(),coord.y(),coord.z(),m_U,m_T,m_C);
				return m_TVirtualVolume->getValue(p);
			}
			inline ConstantValueType get(const Point3D<int>& p) const
			{
				return this->get(p.x(), p.y(), p.z());
			}
			inline ConstantValueType get(long index) const
			{
				return get(this->localToPoint(index));
			}
			
#include <pcl/image/image_base_virtual_get.txx>
#include <pcl/image/image_base_virtual_set_dummy.txx>

			/******** Virtual methods from parent ********/
			using Parent::toIndex;
			virtual long toIndex(int x, int y, int z) const
			{
				return this->localToIndex(x,y,z);
			}

			virtual Point3D<int> toPoint(long index) const
			{
				return this->localToPoint(index);
			}

			virtual const Point3D<int>& getBufferSize() const
			{
				return this->m_BufferSize;
			}

			virtual bool isType(const std::type_info& t) const
			{
				return t==typeid(ValueType);
			}

			/******** Local methods ********/
			inline long localToIndex(const Point3D<int>& p) const
			{
				return this->localToIndex(p.x(), p.y(), p.z());
			}
			inline long localToIndex(int x, int y, int z) const
			{
				return (m_Offset.x()+x) + (m_Offset.y()+y)*m_OffsetTable[1] + (m_Offset.z()+z)*m_OffsetTable[2];
			}

			inline Point3D<int> localToPoint(long index) const
			{
				int z = index/m_OffsetTable[2];
				index %= m_OffsetTable[2];
				int y = index/m_OffsetTable[1];
				index %= m_OffsetTable[1];
				return Point3D<int>(index-m_Offset.x(), y-m_Offset.y(), z-m_Offset.z());
			}
			
		protected:
			Point3D<int> m_BufferSize;
			long m_ActualOffset[4];
			mutable boost::shared_ptr<ml::TVirtualVolume<Type>> m_TVirtualVolume;
			int m_U, m_T, m_C;
			pcl::Region3D<int> m_MevisRegion;

			MlPagedImageWrapper()
			{}

			MlPagedImageWrapper(ml::PagedImage* img, const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& orientation_matrix, int u=0, int t=0, int c=0)
			{
				m_U = u;
				m_T = t;
				m_C = c;
				m_TVirtualVolume.reset(new ml::TVirtualVolume<Type>(img));
				ml::SubImageBox box = m_TVirtualVolume->getVirtualVolume().getBox();
				setBufferSize(box.v2.x-box.v1.x+1, box.v2.y-box.v1.y+1, box.v2.z-box.v1.z+1);
				this->m_Size.set(box.v2.x-box.v1.x+1, box.v2.y-box.v1.y+1, box.v2.z-box.v1.z+1);
				this->setMinPoint(minp, Point3D<int>(box.v1.x, box.v1.y, box.v1.z));
				this->setSpacing(spacing);
				this->setOrigin(origin);
				this->setOrientationMatrix(orientation_matrix);

				m_MevisRegion.set(
					pcl::Point3D<int>(box.v1.x,box.v1.y,box.v1.z),
					pcl::Point3D<int>(box.v2.x,box.v2.y,box.v2.z)
				);
			}

			void setBufferSize(int sx, int sy, int sz) 
			{
				this->m_BufferSize.set(sx, sy, sz);
				m_ActualOffset[0] = 1;
				m_ActualOffset[1] = m_BufferSize.x();
				m_ActualOffset[2] = static_cast<long>(m_BufferSize.x())*static_cast<long>(m_BufferSize.y());
				m_ActualOffset[3] = m_ActualOffset[2]*static_cast<long>(m_BufferSize.z());
				this->m_OffsetTable = m_ActualOffset;
			}

			void setVirtualVolume(const Self* obj)
			{
				m_U = obj->m_U;
				m_T = obj->m_T;
				m_C = obj->m_C;
				m_TVirtualVolume = obj->m_TVirtualVolume;
				setBufferSize(obj->m_BufferSize.x(), obj->m_BufferSize.y(), obj->m_BufferSize.z());
			}
		};

	}
}

#endif