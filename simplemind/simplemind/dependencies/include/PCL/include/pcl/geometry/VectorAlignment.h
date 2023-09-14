#ifndef PCL_VECTOR_ALIGNMENT
#define PCL_VECTOR_ALIGNMENT

#include <pcl/geometry/Point.h>
#include <pcl/macro.h>
#include <pcl/constant.h>
#include <math.h>

namespace pcl
{
	namespace geometry
	{
		using namespace pcl;

		//Based on equations from http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/
		class VectorAlignment
		{
		public:
			VectorAlignment() {}

			VectorAlignment(const Point3D<double>& rotation_axis, const Point3D<double>& moving_vector)
			{
				set(rotation_axis, moving_vector);
			}

			void set(const Point3D<double>& rotation_axis, const Point3D<double>& moving_vector)
			{
				m_MovingVector = moving_vector;
				//Setting up the coefficients of the rotation matrix
				const char sign[3][3] = { {0,-1,1}, {1,0,-1}, {-1,1,0} };
				Coefficient coeff[3][3];
				for (int r=0; r<3; r++) {
					for (int c=0; c<3; c++) {
						Coefficient &cur_coeff = coeff[r][c];
						if (r==c) {
							cur_coeff.offset = pcl_Square(rotation_axis[r]);
							for (int i=0; i<3; i++) if (i!=r) {
								cur_coeff.cosine += pcl_Square(rotation_axis[i]);
							}
						} else {
							cur_coeff.offset = 1;
							for (int i=0; i<3; i++) {
								if (i==r || i==c) {
									cur_coeff.offset *= rotation_axis[i];
								} else {
									cur_coeff.sine = sign[r][c]*rotation_axis[i];
								}
							}
							cur_coeff.cosine = -cur_coeff.offset;
						}
					}
				}
				//Setting up coefficient of the moving vector
				for (int c=0; c<3; c++) {
					Coefficient &cur_coeff = m_MovingVectorCoeff[c];
					cur_coeff.reset();
					for (int r=0; r<3; r++) {
						Coefficient temp(coeff[r][c]);
						temp *= moving_vector[r];
						cur_coeff += temp;
					}
				}
			}

			void compute(const Point3D<double>& reference_vector)
			{
				Coefficient coeff;
				for (int i=0; i<3; i++) {
					Coefficient temp(m_MovingVectorCoeff[i]);
					temp *= reference_vector[i];
					coeff += temp;
				}
				m_RotationRadian = atan(coeff.sine/coeff.cosine);
				double second_order = -( coeff.sine*sin(m_RotationRadian) + coeff.cosine*cos(m_RotationRadian) );
				if (second_order>0) {
					m_RotationRadian += pcl::PI;
				}
				m_AlignedVector.set(
					m_MovingVectorCoeff[0].getValue(m_RotationRadian),
					m_MovingVectorCoeff[1].getValue(m_RotationRadian),
					m_MovingVectorCoeff[2].getValue(m_RotationRadian)
					);
			}

			const Point3D<double>& getAlignedVector() const
			{
				return m_AlignedVector;
			}

			double getRotationRadian() const
			{
				return m_RotationRadian;
			}

		protected:
			struct Coefficient {
				double offset;
				double cosine;
				double sine;

				Coefficient() 
				{
					reset();
				}

				void reset() 
				{
					offset = 0;
					cosine = 0;
					sine = 0;
				}

				Coefficient& operator+=(const Coefficient& obj)
				{
					offset += obj.offset;
					cosine += obj.cosine;
					sine += obj.sine;
					return *this;
				}
				Coefficient& operator*=(double val)
				{
					offset *= val;
					cosine *= val;
					sine *= val;
					return *this;
				}

				double getValue(double theta)
				{
					return offset + cosine*cos(theta) + sine*sin(theta);
				}

				std::string toString()
				{
					std::stringstream ss;
					ss << offset << " + " << cosine << "(cos) + " << sine << "(sin)";
					return ss.str();
				}
			};
			Coefficient m_MovingVectorCoeff[3];
			Point3D<double> m_MovingVector;
			double m_RotationRadian;
			Point3D<double> m_AlignedVector;
		};

	}
}

#endif