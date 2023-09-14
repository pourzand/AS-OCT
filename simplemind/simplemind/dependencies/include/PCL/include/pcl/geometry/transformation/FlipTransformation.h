#ifndef PCL_FLIP_TRANSFORMATION
#define PCL_FLIP_TRANSFORMATION

#include <pcl/geometry/Point.h>
#include <pcl/geometry/Region3D.h>

namespace pcl
{
	namespace geometry
	{
		using namespace pcl;

		class FlipTransformation: private boost::noncopyable
		{
		public:
			typedef FlipTransformation Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

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
				for (int i=0; i<3; i++) {
					m_Sign[i] = 1;
					m_Translation[i] = 0;
				}
				return this;
			}

			template <class PT>
			Self* translate(const PT& p)
			{
				for (int i=0; i<3; i++) {
					m_Translation[i] += p[i];
				}
				return this;
			}

			Self* translate(int i, double val)
			{
				m_Translation[i] += val;
				return this;
			}

			Self* flipAxis(int i)
			{
				m_Sign[i] = -m_Sign[i];
				m_Translation[i] = -m_Translation[i];
				return this;
			}

			template <class RT>
			Self* flipRegion(int i, const RT& region)
			{
				m_Sign[i] = -m_Sign[i];
				m_Translation[i] = -m_Translation[i];
				m_Translation[i] += ( region.getMinPoint()[i] + region.getMaxPoint()[i] );
				return this;
			}

			/****************** Coordinate transformation methods ******************/
			template <class PointType>
			inline Point3D<double> toTransformed(const PointType& p) const
			{
				return Point3D<double>(
					p.x()*m_Sign[0] + m_Translation[0],
					p.y()*m_Sign[1] + m_Translation[1],
					p.z()*m_Sign[2] + m_Translation[2]
				);
			}

			template <class ReturnPointType, class PointType>
			inline ReturnPointType toTransformed(const PointType& p) const
			{
				return ReturnPointType(
					p.x()*m_Sign[0] + m_Translation[0],
					p.y()*m_Sign[1] + m_Translation[1],
					p.z()*m_Sign[2] + m_Translation[2]
				);
			}
			
			/****************** Duplication methods ******************/
			Pointer getCopy() const
			{
				Pointer result(new Self);
				for (int i=0; i<3; i++) {
					result->m_Sign[i] = m_Sign[i];
					result->m_Translation[i] = m_Translation[i];
				}
				return result;
			}

			Pointer getInverse() const
			{
				Pointer result(new Self);
				for (int i=0; i<3; i++) {
					result->m_Sign[i] = m_Sign[i];
					result->m_Translation[i] = -m_Translation[i]*m_Sign[i];
				}
				return result;
			}

			/****************** Data access ******************/
			const char* getSign() const
			{
				return m_Sign;
			}
			const char getSign(int i) const
			{
				return m_Sign[i];
			}

			const double* getTranslation() const
			{
				return m_Translation;
			}
			const double getTranslation(int i) const
			{
				return m_Translation[i];
			}

		protected:
			char m_Sign[3];
			double m_Translation[3]; 

			FlipTransformation() {};
		};

	}
}

#endif