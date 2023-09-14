#ifndef PCL_DUMMY_IMAGE_TYPE
#define PCL_DUMMY_IMAGE_TYPE

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <pcl/image/ImagePhysicalLayer.h>
#include <pcl/misc/FileStreamHelper.h>
#include <pcl/misc/StringTokenizer.h>

#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )
#pragma warning ( disable : 4800 )

namespace pcl
{
	struct DummyImageSetException: public pcl::Exception {};
	
	namespace dummy_image {

		template <bool Flag>
		class DummyImageBase: public ImagePhysicalLayer<Flag>
		{
		public:
			enum {
				ItkAliasable = false
			};
			typedef DummyImageBase Self;
			typedef ImagePhysicalLayer<Flag> Parent;
			typedef boost::shared_ptr< Self > Pointer;
			typedef boost::shared_ptr< Self const > ConstantPointer;
			typedef typename Parent::OrientationMatrixType OrientationMatrixType;

			/******** New methods ********/
			static Pointer New(const Point3D<int>& size, const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
			{ 
				return New(Point3D<int>(0,0,0), size-1, spacing, origin); 
			}
			static Pointer New(const Point3D<int>& size, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
			{ 
				return New(Point3D<int>(0,0,0), size-1, spacing, origin, matrix); 
			}

			static Pointer New(const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
			{ 
				Pointer obj(new Self()); 
				obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
				obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
				obj->setBufferSize(obj->m_Size);
				obj->setSpacing(spacing); 
				obj->setOrigin(origin); 
				return obj; 
			}
			static Pointer New(const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
			{ 
				Pointer obj(new Self()); 
				obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
				obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
				obj->setBufferSize(obj->m_Size);
				obj->setSpacing(spacing); 
				obj->setOrigin(origin); 
				obj->setOrientationMatrix(matrix);
				return obj; 
			} 

			template <class ImagePointer>
			static Pointer New(const ImagePointer& ptr) 
			{
				Pointer result;
				if (Self::UseOrientationMatrix) result = New(Point3D<int>(0,0,0), ptr->getBufferSize()-1, ptr->getSpacing(), ptr->getOrigin(), ptr->getOrientationMatrix());
				else result = New(Point3D<int>(0,0,0), ptr->getBufferSize()-1, ptr->getSpacing(), ptr->getOrigin());
				result->setMinPoint(ptr->getMinPoint(), ptr->toBufferCoordinate(ptr->getMinPoint()));
				return result;
			}
			
			static Pointer New(const std::string& file)
			{
				Pointer obj(new Self());
				obj->read(file);
				return obj;
			}

			/******** Virtual methods from parent ********/
			using Parent::toIndex;
			virtual long toIndex(int x, int y, int z) const
			{
				return localToIndex(x,y,z);
			}

			virtual Point3D<int> toPoint(long index) const
			{
				return localToPoint(index);
			}

			virtual const Point3D<int>& getBufferSize() const
			{
				return m_BufferSize;
			}

			virtual bool isType(const std::type_info& t) const
			{
				return t==typeid(void);
			}

			using Parent::getValue;

            virtual double getValue(long /*index*/) const
			{
				return std::numeric_limits<double>::signaling_NaN();
			}

            virtual double getValue(int /*x*/, int /*y*/, int /*z*/) const
			{
				return std::numeric_limits<double>::signaling_NaN();
			}

			using Parent::setValue;

            virtual bool setValue(long /*index*/, double /*value*/)
			{
				pcl_ThrowException(DummyImageSetException(), "Attempt to set value to a dummy image");
			}

            virtual bool setValue(int /*x*/, int /*y*/, int /*z*/, double /*value*/)
			{
				pcl_ThrowException(DummyImageSetException(), "Attempt to set value to a dummy image");
			}

			/******** Local methods ********/
			long localToIndex(const Point3D<int>& p) const
			{
				return localToIndex(p.x(), p.y(), p.z());
			}
			long localToIndex(int x, int y, int z) const
			{
				const Point3D<int> &p = this->toBufferCoordinate(x,y,z);
				return bufferToIndex(p.x(), p.y(), p.z());
			}

			Point3D<int> localToPoint(long index) const
			{
				return bufferToPoint(index) -= this->m_Offset;
			}
			
			void write(const std::string& file) const
			{
				std::ofstream os(file.c_str(), std::iostream::out);
				if (os.fail()) pcl_ThrowException(FileStreamException(), "Error opening "+file+" for output");
				try {
					os.exceptions(std::ios_base::failbit | std::ios_base::badbit);
					os << "size ="; intPointOut(os, this->getSize()) << std::endl;
					os << "min_point ="; intPointOut(os, this->getMinPoint()) << std::endl;
					os << "buffer_min_point ="; intPointOut(os, this->getBufferOffset()+this->getMinPoint()) << std::endl;
					os << "buffer_size ="; intPointOut(os, this->m_BufferSize) << std::endl;
					os << "spacing ="; doublePointOut(os, this->getSpacing(), 10) << std::endl;
					os << "origin ="; doublePointOut(os, this->getOrigin(), 10) << std::endl;
					os << "orientation ="; doubleMatrixOut(os, this->getOrientationMatrix(), 10) << std::endl;
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(FileStreamException(), "Error encountered while writing! "+std::string(e.what()));
				}
			}

		protected:
			long m_ActualOffset[4];
			Point3D<int> m_BufferSize;

			DummyImageBase() {}

			template <class P>
			std::ostream& intPointOut(std::ostream& os, const P& point) const
			{
				os << " " << point[0] << " " << point[1] << " " << point[2];
				return os;
			}

			template <class P>
			std::ostream& doublePointOut(std::ostream& os, const P& point, int precision) const
			{
				os << " " << std::setprecision(precision) << point[0] 
					<< " " << std::setprecision(precision) << point[1] 
					<< " " << std::setprecision(precision) << point[2];
				return os;
			}

			template <class M>
			std::ostream& doubleMatrixOut(std::ostream& os, const M& matrix, int precision) const
			{
				for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) os << " " << std::setprecision(precision) << matrix[r][c];
				return os;
			}

			void setBufferSize(const Point3D<int>& buffer_size)
			{
				m_BufferSize = buffer_size;
				m_ActualOffset[0] = 1;
				m_ActualOffset[1] = m_BufferSize.x();
				m_ActualOffset[2] = static_cast<long>(m_BufferSize.x())*static_cast<long>(m_BufferSize.y());
				m_ActualOffset[3] = m_ActualOffset[2]*static_cast<long>(m_BufferSize.z());
				this->m_OffsetTable = m_ActualOffset;
			}

			inline long bufferToIndex(int x, int y, int z) const
			{
				return x + y*this->m_OffsetTable[1] + z*this->m_OffsetTable[2];
			}

			inline Point3D<int> bufferToPoint(long index) const
			{
				int z = index/this->m_OffsetTable[2];
				index %= this->m_OffsetTable[2];
				int y = index/this->m_OffsetTable[1];
				index %= this->m_OffsetTable[1];
				return Point3D<int>(index, y, z);
			}

			template <class P>
			void assignPointFromString(P& point, const std::string& str)
			{
				std::string temp = boost::trim_copy(str);
				pcl::misc::StringTokenizer tokenizer(temp.c_str());
				int count = 0;
				for (tokenizer.begin(' '); !tokenizer.end(); tokenizer.next(' ')) {
					point[count] = boost::lexical_cast<typename P::ValueType>(tokenizer.getToken());
					++count;
				}
			}

			template <class M>
			void assignOrientationMatrixFromString(M& matrix, const std::string& str)
			{
				std::string temp = boost::trim_copy(str);
				pcl::misc::StringTokenizer tokenizer(temp.c_str());
				tokenizer.begin(' ');
				for (int r=0; r<3; ++r) for (int c=0; c<3; ++c) {
					if (tokenizer.end()) pcl_ThrowException(Exception(), "Unexpected end encountered when reading orientation matrix!");
					matrix[r][c] = boost::lexical_cast<double>(tokenizer.getToken());
					tokenizer.next(' ');
				}
			}
			
			void read(const std::string& file)
			{
				Point3D<int> size, buffer_size, min_point, buffer_min_point;
				Point3D<double> origin, spacing;
				OrientationMatrixType orientation;
				
				enum { SZ, BSZ, MP, BMP, ORG, SPC, ORIENT };
				bool check[] = {false, false, false, false, false, false, false};

				std::ifstream is(file.c_str(), std::iostream::in);
				while (!is.eof()) {
					std::string str_buffer;
					std::getline(is, str_buffer, '\n');
					if (is.fail() && !is.eof()) pcl_ThrowException(Exception(), "Error occured when reading file "+file);
					boost::trim(str_buffer);
					if (!str_buffer.empty()) {
						pcl::misc::StringTokenizer tokenizer(str_buffer.c_str());
						tokenizer.begin('=');
						std::string header = boost::trim_copy(tokenizer.getToken());
						if (boost::iequals(header, "size")) {
							assignPointFromString(size, tokenizer.getRemainder());
							check[SZ] = true;
						} else if (boost::iequals(header, "min_point")) {
							assignPointFromString(min_point, tokenizer.getRemainder());
							check[MP] = true;
						} else if (boost::iequals(header, "buffer_min_point")) {
							assignPointFromString(buffer_min_point, tokenizer.getRemainder());
							check[BMP] = true;
						} else if (boost::iequals(header, "buffer_size")) {
							assignPointFromString(buffer_size, tokenizer.getRemainder());
							check[BSZ] = true;
						} else if (boost::iequals(header, "spacing")) {
							assignPointFromString(spacing, tokenizer.getRemainder());
							check[SPC] = true;
						} else if (boost::iequals(header, "origin")) {
							assignPointFromString(origin, tokenizer.getRemainder());
							check[ORG] = true;
						} else if (boost::iequals(header, "orientation")) {
							assignOrientationMatrixFromString(orientation, tokenizer.getRemainder());
							check[ORIENT] = true;
						}
					}
				}

				for (int i=0; i<7; ++i) if (!check[i]) {
					std::string header;
					switch(i) {
					case SZ:
						header = "size";
						break;
					case MP:
						header = "min_point";
						break;
					case BMP:
						header = "buffer_min_point";
						break;
					case BSZ:
						header = "buffer_size";
						break;
					case SPC:
						header = "spacing";
						break;
					case ORG:
						header = "origin";
						break;
					case ORIENT:
						header = "orientation";
						break;
					}
					pcl_ThrowException(Exception(), "Cannot find "+header+" when reading "+file);
				}
				
				this->m_Size = size;
				this->setMinPoint(min_point, buffer_min_point);
				this->setBufferSize(buffer_size);
				this->setSpacing(spacing);
				this->setOrigin(origin);
				this->setOrientationMatrix(orientation);
			}
		};

	}

	typedef dummy_image::DummyImageBase<true> DummyImage;
	typedef dummy_image::DummyImageBase<false> SimpleDummyImage;

}

#pragma warning ( pop )
#endif
