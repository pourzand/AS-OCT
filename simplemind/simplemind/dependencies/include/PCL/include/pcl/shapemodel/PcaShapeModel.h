#ifndef PCL_PCA_SHAPE_MODEL
#define PCL_PCA_SHAPE_MODEL

#include <pcl/misc/VnlSymmetricEigenSystemWrapper.h>
#include <pcl/misc/IndexedDataSorter.h>
#include <pcl/exception.h>
#include <pcl/macro.h>
#include <pcl/misc/FileStreamHelper.h>
#include <vnl/vnl_matrix.h>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <algorithm>

namespace pcl
{
	namespace shapemodel
	{

		class PcaShapeModel
		{
		public:
			PcaShapeModel()
			{}

			void write(const std::string& filename)
			{
				std::ofstream os(filename.c_str(), std::ofstream::binary);
				write(os);
			}
			void write(std::ostream& os)
			{
				try {
					auto exception_obj = pcl::StreamExceptionHelper::GetStreamExceptionObject(os, std::ios_base::failbit | std::ios_base::badbit);
					unsigned int size;
					//Writing mean
					size = m_Mean.size();
					os.write((char*)&size, sizeof(unsigned int));
					pcl_ForEach(m_Mean, item) {
						double val = *item;
						os.write((char*)&val, sizeof(double));
					}
					//Writing variance
					size = m_Variance.size();
					os.write((char*)&size, sizeof(unsigned int));
					pcl_ForEach(m_Variance, item) {
						double val = *item;
						os.write((char*)&val, sizeof(double));
					}
					//Writing model
					size = m_Model.rows();
					os.write((char*)&size, sizeof(unsigned int));
					size = m_Model.columns();
					os.write((char*)&size, sizeof(unsigned int));
					for (int r=0; r<m_Model.rows(); ++r) for (int c=0; c<m_Model.columns(); ++c) {
						double val = m_Model.get(r,c);
						os.write((char*)&val, sizeof(double));
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(pcl::Exception(), e.what());
				}
			}

			void read(const std::string& filename)
			{
				std::ifstream is(filename.c_str(), std::ifstream::binary);
				read(is);
			}
			void read(std::istream& is) 
			{
				unsigned int size;
				//Reading mean
				is.read((char*)&size, sizeof(unsigned int));
				m_Mean.resize(size);
				for (int i=0; i<size; ++i) {
					double val;
					is.read((char*)&val, sizeof(double));
					if (!is.good()) pcl_ThrowException(Exception(), "Termination occured unexpectedly");
					m_Mean[i] = val;
				}
				//Reading variance
				is.read((char*)&size, sizeof(unsigned int));
				m_Variance.resize(size);
				for (int i=0; i<size; ++i) {
					double val;
					is.read((char*)&val, sizeof(double));
					if (!is.good()) pcl_ThrowException(Exception(), "Termination occured unexpectedly");
					m_Variance[i] = val;
				}
				m_StdDev.resize(m_Variance.size());
				for (int i=0; i<m_Variance.size(); ++i) m_StdDev[i] = sqrt(m_Variance[i]);
				//Reading model
				unsigned int row, col;
				is.read((char*)&row, sizeof(unsigned int));
				is.read((char*)&col, sizeof(unsigned int));
				m_Model.set_size(row, col);
				for (int r=0; r<m_Model.rows(); ++r) for (int c=0; c<m_Model.columns(); ++c) {
					double val;
					is.read((char*)&val, sizeof(double));
					if (!is.good()) pcl_ThrowException(Exception(), "Termination occured unexpectedly");
					m_Model(r,c) = val;
				}
			}

			template <class ShapeList>
			void train(const ShapeList& list, double variance_ratio=0.95)
			{
				int size = list.begin()->size();
				m_Mean.resize(size);
				pcl_ForEach(m_Mean, item) *item = 0;
				pcl_ForEach(list, item) {
					for (int i=0; i<size; ++i) m_Mean[i] += item->at(i);
				}
				pcl_ForEach(m_Mean, item) *item = *item/list.size();

				vnl_matrix<double> covariance(size, size);
				for (int r=0; r<size; ++r) {
					for (int c=0; c<=r; ++c) {
						double result = 0;
						pcl_ForEach(list, item) result += (item->at(r)-m_Mean[r])*(item->at(c)-m_Mean[c]);
						result /= list.size();
						covariance(r,c) = result;
						if (r!=c) covariance(c,r) = result;
					}
				}

				pcl::misc::VnlSymmetricEigenSystemWrapper<double> eigen(covariance);
				pcl::misc::IndexedDataSorter<double, false> sorter;
				for (int i=0; i<size; ++i) sorter.add(pcl::abs(eigen->get_eigenvalue(i)));
				sorter.sort();
				
				double total_variance = 0;
				for (int i=0; i<size; ++i) total_variance += sorter.get(i);
				double target_variance = total_variance*variance_ratio;
				double cur_total = 0;
				m_Variance.clear();
				for (int i=0; i<size; ++i) {
					m_Variance.push_back(sorter.get(i));
					cur_total += sorter.get(i);
					if (cur_total>=target_variance) break;
				}

				m_Model.set_size(size, m_Variance.size());
				for (int c=0; c<m_Variance.size(); ++c)
					m_Model.set_column(c, eigen->get_eigenvector(sorter.getIndex(c)));

				m_StdDev.resize(m_Variance.size());
				for (int i=0; i<m_Variance.size(); ++i) m_StdDev[i] = sqrt(m_Variance[i]);
			}

			const std::vector<double>& getVariance() const
			{
				return m_Variance;
			}

			const std::vector<double>& getMean() const
			{
				return m_Mean;
			}

			const std::vector<double>& getStdDev() const
			{
				return m_StdDev;
			}

			template <class ShapeType>
			std::vector<double> computeShapeParam(const ShapeType& shape) const
			{
				if (shape.size()!=m_Model.rows()) pcl_ThrowException(
					Exception(), 
					std::string("Invalid input provided! Expected size is ")+boost::lexical_cast<std::string>(m_Model.rows())+" but found "+boost::lexical_cast<std::string>(shape.size())
					);

				std::vector<double> temp(m_Mean.size());
				for (int i=0; i<m_Mean.size(); ++i) temp[i] = shape[i] - m_Mean[i];
				return std::move(transposeMultiply(m_Model, temp));
			}

			template <class ShapeType>
			ShapeType computeShape(const std::vector<double>& param) const
			{
				if (param.size()!=m_Model.columns()) pcl_ThrowException(
					Exception(), 
					std::string("Invalid input provided! Expected size is ")+boost::lexical_cast<std::string>(m_Model.columns())+" but found "+boost::lexical_cast<std::string>(param.size())
					);

				auto temp = multiplyColumnVector(m_Model, param);
				ShapeType result;
				result.resize(m_Model.rows());
				for (int i=0; i<m_Model.rows(); ++i) result[i] = temp[i] + m_Mean[i];
				return std::move(result);
			}

			//Testing only
			void compare(const PcaShapeModel& obj) const
			{
				double error;

				error = 0;
				for (int i=0; i<m_Variance.size(); ++i) {
					error += square(m_Variance[i]-obj.m_Variance[i]);
				}
				std::cout << "Variance diff: " << error << std::endl;

				error = 0;
				for (int i=0; i<m_Mean.size(); ++i) {
					error += square(m_Mean[i]-obj.m_Mean[i]);
				}
				std::cout << "Mean diff: " << error << std::endl;

				error = 0;
				for (int r=0; r<m_Model.rows(); ++r) for (int c=0; c<m_Model.columns(); ++c) {
					error += square(m_Model(r,c)-obj.m_Model(r,c));
				}
				std::cout << "Model diff: " << error << std::endl;
			}

		protected:
			std::vector<double> m_Variance;
			vnl_matrix<double> m_Model;
			std::vector<double> m_Mean;
			std::vector<double> m_StdDev;

			double square(double v) const
			{
				return v*v;
			}

			template <class MatrixType, class VectorType>
			VectorType multiplyColumnVector(const MatrixType& matrix, const VectorType& vector) const //Vector is treated as column
			{
				VectorType result(matrix.rows());
				//if (vector.size()!=matrix.columns()) pcl_ThrowException(Exception(), "Invalid input size!");
				for (int r=0; r<matrix.rows(); ++r) {
					double val = 0;
					for (int c=0; c<matrix.columns(); ++c) val += matrix(r,c)*vector[c];
					result[r] = static_cast<typename VectorType::value_type>(val);
				}
				return std::move(result);
			}

			template <class MatrixType, class VectorType>
			VectorType transposeMultiply(const MatrixType& matrix, const VectorType& vector) const //Matrix is being transpose and vector is treated as column
			{
				VectorType result(matrix.columns());
				//if (vector.size()!=matrix.rows()) pcl_ThrowException(Exception(), "Invalid input size!");
				for (int c=0; c<matrix.columns(); ++c) {
					double val = 0;
					for (int r=0; r<matrix.rows(); ++r) val += matrix(r,c)*vector[r];
					result[c] = static_cast<typename VectorType::value_type>(val);
				}
				return std::move(result);
			}
		};

	}
}

#endif