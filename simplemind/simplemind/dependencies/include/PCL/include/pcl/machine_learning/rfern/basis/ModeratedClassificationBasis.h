#ifndef PCL_MODERATED_CLASSIFICATION_BASIS
#define PCL_MODERATED_CLASSIFICATION_BASIS

#include <pcl/exception.h>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace pcl
{
	namespace rfern
	{

		template <class ObservationType, class TrainingDataType>
		class ModeratedClassificationBasis 
		{
		public:
			struct Parameter 
			{
				int max_label;
				double Nr; //A regularization term

				Parameter() 
				{ 
					Nr = 1;
					max_label = 1;
				}
			};

			ModeratedClassificationBasis() {}

			void initialize(const Parameter& param) 
			{
				m_Count.clear();
				m_TrainingSetClassCount.clear();
				m_Nr = param.Nr;
				m_Count.resize(param.max_label+1, 0);
				m_TrainingSetClassCount.resize(param.max_label+1, 0);
			}

			void addTrainingData(const TrainingDataType& input, bool activate) 
			{
				m_TrainingSetClassCount[input.label()]++;
				if (activate) m_Count[input.label()]++;
			}

			void update()
			{
				m_Probability.clear();
				m_Probability.resize(m_Count.size());
				for (int i=0; i<m_Count.size(); i++) m_Probability[i] = ((double)m_Count[i] + m_Nr) / ((double)m_TrainingSetClassCount[i] + m_Count.size()*m_Nr);
				m_TrainingSetClassCount.clear();
				m_Count.clear();
			}

			ObservationType& compute(ObservationType& obj) const
			{
				obj.output()->resize(m_Probability.size());
				for (int i=0; i<m_Probability.size(); i++) {
					obj.output(i) = m_Probability[i];
				}
				return obj;
			}

			/************ IO methods ************/
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					if (!use_binary) {
						os << "TXT\n";
						os << m_Probability.size() << std::endl;
						os << m_Probability[0];
						for (int i=1; i<m_Probability.size(); i++) {
							os << " " << m_Probability[i];
						}
						os << std::endl;
					} else {
						os << "BIN\n";
						int size = static_cast<int>(m_Probability.size());
						os.write((char*)&size, sizeof(int));
						for (int i=0; i<m_Probability.size(); i++) {
							os.write((char*)&m_Probability[i], sizeof(double));
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
						readVector(is, m_Probability);
					} else if (strcmp(header, "BIN\n")==0) {
						int size;
						is.read((char*)&size, sizeof(int));
						m_Probability.resize(size);
						for (int i=0; i<m_Probability.size(); i++) {
							is.read((char*)&m_Probability[i], sizeof(double));
						}
					} else {
						pcl_ThrowException(Exception(), "Invalid header \"" + std::string(header) + "\" encountered!");
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

		protected:
			double m_Nr;
			std::vector<double> m_Probability;
			std::vector<long> m_Count; 
			std::vector<long> m_TrainingSetClassCount;

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