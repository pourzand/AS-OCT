#ifndef BUFFERED_COD_TO_IMAGE_CONVERTER
#define BUFFERED_COD_TO_IMAGE_CONVERTER

#include <pcl/cod/CodToImageConverter.h>
#include <boost/numeric/conversion/bounds.hpp>

#pragma warning ( push )


namespace pcl
{
	namespace cod
	{
		using namespace pcl;
		using namespace pcl::cod::details;

		template <class CodType, bool Flag, class Buffer=ImageBuffer<typename boost::remove_cv<typename CodType::ReturnType>::type>>
		class BufferedCodToImageConverter: public ImagePhysicalLayer<Flag>
		{
		public:
			enum {
				ItkAliasable = false
			};
			typedef BufferedCodToImageConverter Self;
			typedef ImagePhysicalLayer<Flag> Parent;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;
			typedef Buffer BufferType;
			typedef typename BufferType::ValueType ValueType;
			typedef typename BufferType::ConstantValueType ConstantValueType;
			typedef typename BufferType::IoValueType IoValueType;
			typedef typename BufferType::ReferenceValueType BufferReferenceValueType;
			typedef Image<typename BufferType::ValueType, Flag, BufferType> ReturnImageType;
			typedef typename Self::OrientationMatrixType OrientationMatrixType;

			template <class ImagePointerType>
			static ConstantPointer New(const typename CodType::Pointer& cod, const ImagePointerType& template_image)
			{
				if (!cod->isCompatible(template_image)) pcl_ThrowException(IncompatibleCodException(), "COD object is not compatible with template image");
				Pointer obj(new Self);
				obj->m_Size = template_image->getSize();
				obj->setMinPoint(template_image->getMinPoint(), template_image->toBufferCoordinate(template_image->getMinPoint()));
				obj->setSpacing(template_image->getSpacing());
				obj->setOrigin(template_image->getOrigin());
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(template_image->getOrientationMatrix(), template_image->getInverseOrientationMatrix());
				obj->setBuffer(cod, template_image);
				//Fill buffer with lowest value possible to indicate uncomputed
				long num = obj->m_Buffer->getOffsetTable()[3];
				for (long i=0; i<num; ++i) obj->m_Buffer->at(i) = boost::numeric::bounds<IoValueType>::lowest();
				return obj;
			}

			template <class ImagePointerType>
			static ConstantPointer NewWithTemplateImageAsBuffer(const typename CodType::Pointer& cod, const ImagePointerType& template_image)
			{
				if (!cod->isCompatible(template_image)) pcl_ThrowException(IncompatibleCodException(), "COD object is not compatible with template image");
				Pointer obj(new Self);
				obj->m_Size = template_image->getSize();
				obj->setMinPoint(template_image->getMinPoint(), template_image->toBufferCoordinate(template_image->getMinPoint()));
				obj->setSpacing(template_image->getSpacing());
				obj->setOrigin(template_image->getOrigin());
				if (Self::UseOrientationMatrix) obj->setOrientationMatrix(template_image->getOrientationMatrix(), template_image->getInverseOrientationMatrix());
				obj->setTemplateImageAsBuffer(cod, template_image);
				return obj;
			}

			/******** Aliasing and subimage methods ********/
			typename Self::ConstantPointer getAlias(const Point3D<int>& minp, bool fix_physical) const
			{
				if (fix_physical) {
					return getAlias(minp, this->getSpacing(), 
						this->toPhysicalCoordinate(this->getMinPoint()) - this->toPhysicalVector(minp)
						);
				} else return this->getAlias(minp, this->getSpacing(), this->getOrigin());
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
				BufferReferenceValueType value = m_Buffer->at(iter);
				if (value==boost::numeric::bounds<IoValueType>::lowest()) value = m_CodBuffer->get(iter);
				return value;
			}
			template <class Iterator>
			inline typename boost::enable_if_c<getPoint_exists<Iterator>::value, ConstantValueType>::type get(const Iterator& iter) const
			{
				BufferReferenceValueType value = m_Buffer->at(iter);
				if (value==boost::numeric::bounds<IoValueType>::lowest()) value = m_CodBuffer->get(this->toBufferCoordinate(iter.getPoint()), iter);
				return value;
			}

			inline ConstantValueType get(int x, int y, int z) const
			{
				long index = toIndex(x,y,z);
				BufferReferenceValueType value = m_Buffer->at(index);
				if (value==boost::numeric::bounds<IoValueType>::lowest()) value = m_CodBuffer->get(this->toBufferCoordinate(x,y,z), index);
				return value;
			}
			inline ConstantValueType get(const Point3D<int>& p) const
			{
				return this->get(p.x(), p.y(), p.z());
			}

			inline ConstantValueType get(long index) const
			{
				BufferReferenceValueType value = m_Buffer->at(index);
				if (value==boost::numeric::bounds<IoValueType>::lowest()) value = m_CodBuffer->get(index);
				return value;
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
			
			void resetBuffer() const
			{
				long num = m_Buffer->getOffsetTable()[3];
				for (long i=0; i<num; ++i) m_Buffer->at(i) = boost::numeric::bounds<IoValueType>::lowest();
			}

			typename ReturnImageType::Pointer getImage() const
			{
				if (Self::UseOrientationMatrix) return ReturnImageType::New(this->m_Buffer, this->toBufferCoordinate(this->getMinPoint()), this->getMinPoint(), this->getMaxPoint(), this->getSpacing(), this->getOrigin(), this->getOrientationMatrix());
				else return ReturnImageType::New(this->m_Buffer, this->toBufferCoordinate(this->getMinPoint()), this->getMinPoint(), this->getMaxPoint(), this->getSpacing(), this->getOrigin());
			}

			inline const typename CodType::Pointer& getCod() const
			{
				return m_CodBuffer->getCod();
			}

		protected:
			typename CodBuffer<CodType>::Pointer m_CodBuffer;
			long m_ActualOffset[4];
			Point3D<int> m_BufferSize;
			mutable typename BufferType::Pointer m_Buffer;

			BufferedCodToImageConverter() {}
			
			void setBuffer(const Self* obj)
			{
				this->m_CodBuffer = obj->m_CodBuffer;
				this->m_BufferSize = obj->m_BufferSize;
				this->m_OffsetTable = this->m_CodBuffer->getOffsetTable();
				this->m_Buffer = obj->m_Buffer;
			}
			
			template <class ImagePointerType>
			void setBuffer(const typename CodType::Pointer& cod, const ImagePointerType& image)
			{
				this->m_BufferSize = image->getSize();
				this->m_CodBuffer = CodBuffer<CodType>::New(cod, image, image->getOffsetTable());
				this->m_OffsetTable = this->m_CodBuffer->getOffsetTable();
				this->m_Buffer = BufferType::New(image->getBufferSize().x(), image->getBufferSize().y(), image->getBufferSize().z());
			}
			
			template <class ImagePointerType>
			void setTemplateImageAsBuffer(const typename CodType::Pointer& cod, const ImagePointerType& image)
			{
				this->m_BufferSize = image->getSize();
				this->m_CodBuffer = CodBuffer<CodType>::New(cod, image, image->getOffsetTable());
				this->m_OffsetTable = this->m_CodBuffer->getOffsetTable();
				this->m_Buffer = image->getBuffer();
			}
		};

	}
}

#pragma warning ( pop )
#endif