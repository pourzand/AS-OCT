#ifndef PCL_WEIGHTED_REGRESSION_BASIS
#define PCL_WEIGHTED_REGRESSION_BASIS

#include <pcl/exception.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace rfern
	{

		template <class ObservationType, class TrainingDataType>
		class WeightedRegressionBasis 
		{
		public:
			struct Parameter 
			{};

			WeightedRegressionBasis() {}

			void initialize(const Parameter& param) 
			{
				m_Weight = 0;
				m_Count = 0;
			}

			void addTrainingData(const TrainingDataType& input, bool activate) 
			{
				if (m_Count==0) m_Coefficient.resize(input.output_size(), 0);
				if (activate) {
					for (int i=0; i<m_Coefficient.size(); ++i) m_Coefficient[i] += input.output(i);
					++m_Weight;
				}
				++m_Count;
			}

			void update()
			{
				for (int i=0; i<m_Coefficient.size(); ++i) m_Coefficient[i] /= m_Weight;
				m_Weight /= m_Count;
			}

			ObservationType& compute(ObservationType& obj) const
			{
				obj.output()->resize(m_Coefficient.size());
				for (int i=0; i<m_Coefficient.size(); i++) {
					obj.output(i) = m_Coefficient[i];
				}
				obj.weight() = m_Weight;
				return obj;
			}

			/************ IO methods ************/
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					if (!use_binary) {
						os << "TXT\n";
						os << Coefficient.size() << std::endl;
						os << m_Weight << std::endl;
						os << Coefficient[0];
						for (int i=1; i<Coefficient.size(); i++) {
							os << " " << Coefficient[i];
						}
						os << std::endl;
					} else {
						os << "BIN\n";
						int size = static_cast<int>(Coefficient.size());
						os.write((char*)&size, sizeof(int));
						os.write((char*)&m_Weight, sizeof(double));
						for (int i=0; i<Coefficient.size(); i++) {
							os.write((char*)&Coefficient[i], sizeof(double));
						}
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

			void read(std::istream& is)
			{
				try {
					char header[5];
					is.read(header, 4); header[4] = NULL;
					if (strcmp(header, "TXT\n")==0) {
						std::string buffer;
						std::getline(is, buffer);
						int size = atoi(buffer.c_str());
						m_Probability.resize(size);
						std::getline(is, buffer);
						m_Weight = atof(buffer.c_str());
						readVector(is, m_Coefficient);
					} else if (strcmp(header, "BIN\n")==0) {
						int size;
						is.read((char*)&size, sizeof(int));
						m_Probability.resize(size);
						is.read((char*)&m_Weight, sizeof(double));
						for (int i=0; i<m_Coefficient.size(); i++) {
							is.read((char*)&m_Coefficient[i], sizeof(double));
						}
					} else {
						pcl_ThrowException(Exception(), "Invalid header \"" + std::string(header) + "\" encountered!");
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

		protected:
			std::vector<double> m_Coefficient;
			double m_Weight;
			int m_Count;

			void readVector(std::istream& is, std::vector<double>& vec)
			{
				std::string buffer;
				std::getline(is, buffer);
				pcl::misc::StringTokenizer tokenizer(buffer.c_str());
				int count = 0;
				for (tokenizer.begin(' '); !tokenizer.end(); tokenizer.next(' ')) {
					if (count>=vec.size()) pcl_ThrowException(Exception(), "Data overflow!");
					vec[count] = atof(tokenizer.getToken().c_str());
					count++;
				}
				if (count!=vec.size()) {
					pcl_ThrowException(Exception(), "Insufficient data read!");
				}
			}
		};

	}
}

#endif