#ifndef COD_TO_IMAGE_CONVERTER
#define COD_TO_IMAGE_CONVERTER

#include <pcl/image/ImageBase.h>
#include <pcl/image/ImageAlgorithm.h>

#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4800 )

namespace pcl
{
	namespace cod
	{
		using namespace pcl;
		
		namespace details
		{
			/**
				A wrapper to make a COD object behaves similary to a buffer object (mainly in terms of the coordinate system)
			**/
			template <class CodType>
			class CodBuffer
			{
			public:
				typedef CodBuffer Self;
				typedef boost::shared_ptr< Self > Pointer;
				typedef boost::shared_ptr< Self const > ConstantPointer;
				typedef typename CodType::ReturnType ReturnType;
			
				template <class ImagePointerType>
				static Pointer New(const typename CodType::Pointer& cod, const ImagePointerType& image, const long*offset_table) 
				{
					Pointer obj(new Self);
					obj->m_Cod = cod;
					if (cod->isUnbounded()) {
						obj->m_BufferToCodOffset.set(0,0,0);
					} else {
						obj->m_BufferToCodOffset = cod->getRegion().getMinPoint();
						obj->m_BufferToCodOffset -= image->toBufferCoordinate(image->localToPoint(cod->getMinPointIndex()));
					}
					for (int i=0; i<4; ++i) obj->m_OffsetTable[i] = offset_table[i];
					return obj;
				}
				
				/*template <class Iterator>
				inline typename boost::disable_if_c<getPoint_exists<Iterator>::value, ReturnType>::type get(const Iterator& iter) const
				{
					return this->getViaIndex<CodType>(iter);
				}
				template <class Iterator>
				inline typename boost::enable_if_c<getPoint_exists<Iterator>::value, ReturnType>::type get(const Iterator& iter) const
				{
					return m_Cod->get(Point3D<int>(iter.getPoint())+=m_BufferToCodOffset, iter);
				}*/

				inline ReturnType get(int x, int y, int z) const
				{
					return m_Cod->get(Point3D<int>(x,y,z)+=m_BufferToCodOffset, toIndex(x,y,z));
				}

				inline ReturnType get(const Point3D<int>& point, long index) const
				{
					return m_Cod->get(Point3D<int>(point)+=m_BufferToCodOffset, index);
				}

				inline ReturnType get(long index) const
				{
					return getViaIndex<CodType>(index);
				}
				
				inline const typename CodType::Pointer& getCod() const
				{
					return m_Cod;
				}
				
				inline Point3D<int> toPoint(long index) const
				{
					int z = index/this->m_OffsetTable[2];
					index %= this->m_OffsetTable[2];
					int y = index/this->m_OffsetTable[1];
					index %= this->m_OffsetTable[1];
					return Point3D<int>(index,y,z);
				}
				
				inline long toIndex(int x, int y, int z) const
				{
					return x + y*this->m_OffsetTable[1] + z*this->m_OffsetTable[2];
				}
				
				inline Point3D<int> getCodBufferMinPoint() const
				{
					return m_Cod->getRegion().getMinPoint() - m_BufferToCodOffset;
				}

				inline long const* getOffsetTable() const
				{
					return m_OffsetTable;
				}

			protected:
				typename CodType::Pointer m_Cod;
				Point3D<int> m_BufferToCodOffset;
				long m_OffsetTable[4];
				
				template <class T>
				inline typename boost::enable_if_c<T::ReferableViaIndex, ReturnType>::type getViaIndex(long index) const
				{
					return m_Cod->get(index);
				}
				template <class T>
				inline typename boost::disable_if_c<T::ReferableViaIndex, ReturnType>::type getViaIndex(long index) const
				{
					return m_Cod->get(indexToCodCoordinate(index), index);
				}
				
				inline Point3D<int> indexToCodCoordinate(long index) const
				{
					return (toPoint(index) += this->m_BufferToCodOffset);
				}
			};
		}

		using namespace pcl::cod::details;
		

		template <class CodType, bool Flag>
		class CodToImageConverter: public ImagePhysicalLayer<Flag>
		{
		public:
			enum {
				ItkAliasable = false
			};
			typedef CodToImageConverter Self;
			typedef ImagePhysicalLayer<Flag> Parent;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;
			typedef typename CodType::ReturnType ValueType;
			typedef typename CodType::ReturnType ConstantValueType;
			typedef typename CodType::ReturnType IoValueType;
			typedef typename Self::OrientationMatrixType OrientationMatrixType;

			template <class ImagePointerType>
			static ConstantPointer New(const typename CodType::Pointer& cod, const ImagePointerType& image)
			{
				if (!cod->isCompatible(image)) pcl_ThrowException(IncompatibleCodException(), "COD object is not compatible with template image");
				Pointer obj(new Self);
				obj->m_Size = image->getSize();
				obj->setMinPoint(image->getMinPoint(), image->toBufferCoordinate(image->getMinPoint()));
				obj->setSpacing(image->getSpacing());
				obj->setOrigin(image->getOrigin());
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(image->getOrientationMatrix(), image->getInverseOrientationMatrix());
				obj->setBuffer(cod, image);
				return obj;
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
				obj->setBuffer(this);
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
				obj->setBuffer(this);
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
				obj->setBuffer(this);
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
				obj->setBuffer(this);
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
				return obj;
			}

			/******** Value get/set methods ********/
			template <class Iterator>
			inline typename boost::disable_if_c<getPoint_exists<Iterator>::value, ConstantValueType>::type get(const Iterator& iter) const
			{
				return m_CodBuffer->get(iter);
			}
			template <class Iterator>
			inline typename boost::enable_if_c<getPoint_exists<Iterator>::value, ConstantValueType>::type get(const Iterator& iter) const
			{
				return m_CodBuffer->get(this->toBufferCoordinate(iter.getPoint()), iter);
			}

			inline ConstantValueType get(int x, int y, int z) const
			{
				const Point3D<int>& coord = this->toBufferCoordinate(x,y,z);
				return m_CodBuffer->get(coord.x(), coord.y(), coord.z());
			}
			inline ConstantValueType get(const Point3D<int>& p) const
			{
				return this->get(p.x(), p.y(), p.z());
			}
			inline ConstantValueType get(long index) const
			{
				return m_CodBuffer->get(index);
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
				const Point3D<int> &p = this->toBufferCoordinate(x,y,z);
				return m_CodBuffer->toIndex(p.x(), p.y(), p.z());
			}

			inline Point3D<int> localToPoint(long index) const
			{
				return m_CodBuffer->toPoint(index) -= this->m_Offset;
			}
			
			inline const typename CodType::Pointer& getCod() const
			{
				return m_CodBuffer->getCod();
			}

		protected:
			typename CodBuffer<CodType>::Pointer m_CodBuffer;
			Point3D<int> m_BufferSize;

			CodToImageConverter() {}
			
			void setBuffer(const Self* obj)
			{
				this->m_CodBuffer = obj->m_CodBuffer;
				this->m_BufferSize = obj->m_BufferSize;
				this->m_OffsetTable = this->m_CodBuffer->getOffsetTable();
			}
			
			template <class ImagePointerType>
			void setBuffer(const typename CodType::Pointer& cod, const ImagePointerType& image)
			{
				this->m_BufferSize = image->getSize();
				this->m_CodBuffer = CodBuffer<CodType>::New(cod, image, image->getOffsetTable());
				this->m_OffsetTable = this->m_CodBuffer->getOffsetTable();
			}
		};

	}
}

#pragma warning ( pop )
#endif