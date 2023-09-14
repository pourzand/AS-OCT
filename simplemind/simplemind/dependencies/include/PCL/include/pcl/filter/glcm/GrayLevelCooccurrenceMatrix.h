#ifndef PCL_GRAY_LEVEL_COOCCURRENCE_MATRIX
#define PCL_GRAY_LEVEL_COOCCURRENCE_MATRIX

#include <vnl/vnl_matrix.h>
#include <boost/lexical_cast.hpp>
#include <ios>
#include <iomanip>

namespace pcl
{
	namespace filter
	{

		class GrayLevelCooccurrenceMatrix
		{
		public:
			GrayLevelCooccurrenceMatrix(unsigned size, bool symmetric=false)
			{
				m_Symmetric = symmetric;
				m_Matrix.set_size(size, size);
				m_Matrix.fill(0);
				m_Num = 0;
			}

			GrayLevelCooccurrenceMatrix(const GrayLevelCooccurrenceMatrix& obj)
			{
				m_Symmetric = obj.m_Symmetric;
				m_Matrix = obj.m_Matrix;
				m_Num = obj.m_Num;
			}

			void clear()
			{
				m_Matrix.fill(0);
				m_Num = 0;
			}

			unsigned size() const
			{
				return m_Matrix.rows();
			}

			long getNum() const
			{
				return m_Num;
			}

			vnl_matrix<double>& getMatrix()
			{
				return m_Matrix;
			}

			const vnl_matrix<double>& getMatrix() const
			{
				return m_Matrix;
			}

			GrayLevelCooccurrenceMatrix& operator=(const GrayLevelCooccurrenceMatrix& obj)
			{
				m_Matrix = obj.m_Matrix;
				m_Num = obj.m_Num;
				return *this;
			}

			void add(unsigned row, unsigned col)
			{
				++m_Matrix(row,col);
				++m_Num;
				if (m_Symmetric) {
					++m_Matrix(col,row);
					++m_Num;
				}
			}

			double operator()(unsigned row, unsigned col) const
			{
				return m_Matrix(row, col);
			}

			vnl_matrix<double>& getNormalizedMatrix(vnl_matrix<double>& mat) const
			{
				long num = m_Num;
				mat = m_Matrix;
				for (unsigned r=0; r<mat.rows(); ++r) for (unsigned c=0; c<mat.columns(); ++c)
					mat(r,c) /= num;
				return mat;
			}

			vnl_matrix<double> getNormalizedMatrix() const
			{
				vnl_matrix<double> result;
				return getNormalizedMatrix(result);
			}


            void debug_print(std::ostream &out, int width=4) const
            {
                using std::setw;
                using std::endl;

                if (width < 4) width = 4;

                out << setw(width) << "glcm";
                for (int j = 0; j < size(); j++)
                    out << setw(width) << boost::lexical_cast<std::string>(j) + ":";
                out << endl;
                for (int i = 0; i < size(); i++)
                {
                    out << setw(width) << boost::lexical_cast<std::string>(i) + ":";
                    for (int j = 0; j < size(); j++)
                    {
                        out << setw(width) << m_Matrix(i, j);
                    }
                    out << endl;
                }
            }

		protected:
			bool m_Symmetric;
			vnl_matrix<double> m_Matrix;
			long m_Num;
		};

	}
}

#endif