#ifndef PCL_AFFINE_TRANSFORMATION
#define PCL_AFFINE_TRANSFORMATION

#ifndef NO_VNL

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Region3D.h>
#include <pcl/geometry/transformation/FlipTransformation.h>
#include <pcl/math.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_rotation_matrix.h>
#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_inverse.h>

namespace pcl
{
	namespace geometry
	{
		using namespace pcl;

		class AffineTransformation: private boost::noncopyable
		{
		public:
			typedef AffineTransformation Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;
			typedef vnl_matrix_fixed<double,3,3> MatrixType;
			typedef vnl_matrix_fixed<double,3,1> VectorType;


			enum Axis
			{
				X = 0, 
				Y = 1, 
				Z = 2
			};

			static Pointer New()
			{
				Pointer result(new Self);
				result->reset();
				return result;
			}

			Self* reset() 
			{
				m_Transformation.set_identity();
				for (int i=0; i<3; i++) {
					m_Translation(i,0) = 0;
				}
				return this;
			}

			template <class PT>
			Self* addTranslation(const PT& p)
			{
				for (int i=0; i<3; i++) {
					m_Translation(i,0) += p[i];
				}
				return this;
			}

			Self* addTranslation(int i, double val)
			{
				m_Translation(i,0) += val;
				return this;
			}
			
			Self* addScaling(double s)
			{
				return addScaling(Point3D<double>(s,s,s));
			}
			
			template <class PT>
			typename boost::enable_if<boost::is_base_of<PointBase,PT>, Self>::type* addScaling(const PT& s)
			{
				for (int i=0; i<3; i++) {
					m_Translation(i,0) *= s[i];
					for (int c=0; c<3; c++) {
						m_Transformation(i,c) *= s[i];
					}
				}
				return this;
			}

			Self* addScaling(int i, double val)
			{
				for (int c=0; c<3; c++) {
					m_Transformation(i,c) *= val;
				}
				m_Translation(i,0) *= val;
				return this;
			}
			
			template <class PointType>
			Self* addRotation(const PointType& axis, double radian)
			{
				vnl_vector_fixed<double,3> temp;
				for (int i=0; i<3; i++) temp[i] = axis[i]*radian;
				MatrixType &rot_mat = vnl_rotation_matrix(temp);
				addTransformation(rot_mat);
				return this;
			}

			template <class PointType, class OriginPointType>
			Self* addRotation(const PointType& axis, double radian, const OriginPointType& rotation_origin)
			{
				addTranslation(-rotation_origin);
				addRotation(axis, radian);
				addTranslation(rotation_origin);
				return this;
			}
			
			Self* addTransformation(const MatrixType& mat)
			{
				m_Transformation = mat*m_Transformation;
				m_Translation = mat*m_Translation;
				return this;
			}

			Self* addTransformation(const Pointer transform)
			{
				return addTransformation(transform.get());
			}
			
			Self* addTransformation(const Self* transform)
			{
				addTransformation(transform->getTransformation());
				addTranslation(transform->getTranslation());
				return this;
			}
			
			Self* addTransformation(const FlipTransformation::Pointer flip_transform)
			{
				return addTransformation(flip_transform.get());
			}

			Self* addTransformation(const FlipTransformation* flip_transform)
			{
				MatrixType mat;
				mat.set_identity();
				for (int i=0; i<3; i++) mat(i,i) *= flip_transform->getSign(i);
				addTransformation(mat);
				Point3D<double> trans(flip_transform->getTranslation(0), flip_transform->getTranslation(1), flip_transform->getTranslation(2));
				addTranslation(trans);
				return this;
			}
			
			template <class ImagePointer>
			Self* addPhysicalToImageTransformation(const ImagePointer& image)
			{
				addTranslation(-image->getOrigin());
				if (ImagePointer::element_type::UseOrientationMatrix) addTransformation(image->getInverseOrientationMatrix());
				addScaling(image->getOneOverSpacing());
				return this;
			}
			
			template <class ImagePointer>
			Self* addImageToPhysicalTransformation(const ImagePointer& image)
			{
				addScaling(image->getSpacing());
				if (ImagePointer::element_type::UseOrientationMatrix) addTransformation(image->getOrientationMatrix());
				addTranslation(image->getOrigin());
				return this;
			}

			/****************** Coordinate transformation methods ******************/
			template <class ReturnPointType, class PointType>
			inline typename boost::disable_if<boost::is_integral<typename ReturnPointType::ValueType>, ReturnPointType>::type toTransformed(const PointType& p) const
			{
				return ReturnPointType(
					m_Transformation(0,0)*p[0] + m_Transformation(0,1)*p[1] + m_Transformation(0,2)*p[2] + m_Translation(0,0),
					m_Transformation(1,0)*p[0] + m_Transformation(1,1)*p[1] + m_Transformation(1,2)*p[2] + m_Translation(1,0),
					m_Transformation(2,0)*p[0] + m_Transformation(2,1)*p[1] + m_Transformation(2,2)*p[2] + m_Translation(2,0)
				);
			}
			
			template <class ReturnPointType, class PointType>
			inline typename boost::enable_if<boost::is_integral<typename ReturnPointType::ValueType>, ReturnPointType>::type toTransformed(const PointType& p) const
			{
				return ReturnPointType(
					pcl::round(m_Transformation(0,0)*p[0] + m_Transformation(0,1)*p[1] + m_Transformation(0,2)*p[2] + m_Translation(0,0)),
					pcl::round(m_Transformation(1,0)*p[0] + m_Transformation(1,1)*p[1] + m_Transformation(1,2)*p[2] + m_Translation(1,0)),
					pcl::round(m_Transformation(2,0)*p[0] + m_Transformation(2,1)*p[1] + m_Transformation(2,2)*p[2] + m_Translation(2,0))
				);
			}

			/****************** Duplication methods ******************/
			Pointer getCopy() const
			{
				Pointer result(new Self);
				result->m_Transformation = m_Transformation;
				result->m_Translation = m_Translation;
				return result;
			}

			Pointer getInverse() const
			{
				Pointer result(new Self);
				result->m_Transformation = vnl_inverse(m_Transformation);
				result->m_Translation = result->m_Transformation*(-m_Translation);
				return result;
			}

			/****************** Data access ******************/
			inline const MatrixType& getTransformation() const
			{
				return m_Transformation;
			}
			
			inline Point3D<double> getTranslation() const
			{
				return Point3D<double>(getTranslation(0), getTranslation(1), getTranslation(2));
			}
			inline double getTranslation(int i) const
			{
				return m_Translation(i,0);
			}

		protected:
			MatrixType m_Transformation;
			VectorType m_Translation; 

			AffineTransformation() {};
		};

	}
}

#endif

#endif