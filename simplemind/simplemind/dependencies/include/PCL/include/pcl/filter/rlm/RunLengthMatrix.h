#ifndef PCL_RUN_LENGTH_MATRIX
#define PCL_RUN_LENGTH_MATRIX

#include <vnl/vnl_matrix.h>
#include <ios>
#include <iomanip>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace filter
	{

		class RunLengthMatrix
		{
		public:
            RunLengthMatrix() {}

			RunLengthMatrix(const RunLengthMatrix &obj)
			{
				m_Matrix = obj.m_Matrix;
			}

			void clear()
			{
				m_Matrix.fill(0);
			}
            
            void setSize(int numGraylevels, int maxRunLength)
            {
                m_Matrix.set_size(numGraylevels, maxRunLength + 1);
                m_Matrix.fill(0);
            }

            unsigned int getNumGraylevels() const
            {
                return m_Matrix.rows();
            }

            unsigned int getMaxRunLength() const
            {
                return m_Matrix.cols() - 1;
            }

			vnl_matrix<double>& getMatrix()
			{
				return m_Matrix;
			}

            /*
             * Return a reference to the underlying matrix object.
             * There is no concept of a "normalized matrix" for RLM.
             */
			const vnl_matrix<double>& getMatrix() const
			{
				return m_Matrix;
			}

			RunLengthMatrix& operator=(const RunLengthMatrix& obj)
			{
				m_Matrix = obj.m_Matrix;
				//m_Num = obj.m_Num;
				return *this;
			}

            /*
             * By convention, graylevels are indexed [0, maxGraylevel - 1], while runlengths are indexed [0, maxRunLength] (where 0 is a dummy column)
             */
			void add(unsigned graylevel, unsigned runLength)
			{
				m_Matrix(graylevel, runLength)++;
			}

            /*
             * By convention, graylevels are indexed [0, maxGraylevel - 1], while runlengths are indexed [0, maxRunLength] (where 0 is a dummy column)
             */
			double operator()(unsigned graylevel, unsigned runLength) const
			{
				return m_Matrix(graylevel, runLength);
			}

            void debug_print(std::ostream &out, int width=4) const
            {
                using std::setw;
                using std::endl;

                if (width < 4) width = 4;

                out << setw(width) << "rlm";
                for (int j = 0; j <= getMaxRunLength(); j++)
                    out << setw(width) << boost::lexical_cast<std::string>(j) + ":";
                out << endl;
                for (int i = 0; i < getNumGraylevels(); i++)
                {
                    out << setw(width) << boost::lexical_cast<std::string>(i) + ":";
                    for (int j = 0; j <= getMaxRunLength(); j++)
                    {
                        out << setw(width) << m_Matrix(i, j);
                    }
                    out << endl;
                }
            }

		protected:
			vnl_matrix<double> m_Matrix;
			long m_sum;
		};

	}
}

#endif