#ifndef PCL_LINEAR_PARTITION_HASH
#define PCL_LINEAR_PARTITION_HASH

#include <pcl/exception.h>
#include <pcl/math.h>
#include <pcl/misc/StringTokenizer.h>
#include <iostream>
#include <boost/random.hpp>
#include <math.h>
#include <time.h>

namespace pcl
{
	namespace rfern
	{

		class LinearPartitionHash
		{
		public:
			struct Parameter 
			{};

			LinearPartitionHash() 
			{}

			void setNodeNum(int num)
			{
				m_NodeNum = num;
				m_PartitionNum = static_cast<int>(ceil(log(static_cast<double>(num))/log(2.0)));
				m_Multiplier = static_cast<double>(m_NodeNum) / (pow(2.0,m_PartitionNum)-1);
				m_Coefficient.resize(m_PartitionNum);
				m_Offset.resize(m_PartitionNum);
			}

			void initialize(const Parameter& param) 
			{}

			template <class TrainingDataListType>
			void train(const TrainingDataListType& training_data_list) 
			{
				m_FeatureNum = (*training_data_list.begin()).size();
				boost::random::mt19937 rnd_gen;
				rnd_gen.seed(static_cast<int>(std::time(NULL)));
				for (int i=0; i<m_PartitionNum; ++i) {
					std::vector<double> coefficient(m_FeatureNum);
					randomizeCoefficient(coefficient, m_Offset[i], training_data_list, rnd_gen);
					m_Coefficient[i] = std::move(coefficient);
				}
			}

			void printCoefficients()
			{
				for (int i=0; i<m_PartitionNum; i++) {
					pcl_ForEach(m_Coefficient[i], item) std::cout << *item << " ";
					std::cout << "-> " << m_Offset[i] << std::endl;
				}
			}

			template <class DataType>
			int computeIndex(DataType& obj) const
			{
				int result = 0;
				for (int i=0; i<m_PartitionNum; ++i) {
					if (getProjectedValue(obj, m_Coefficient[i])>=m_Offset[i]) {
						int temp = 1;
						temp <<= pcl::round(i*m_Multiplier);
						result |= temp;
					}
				}
				return result;
			}

			/************ IO methods ************/
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					if (!use_binary) {
						os << "TXT\n";
						os << m_NodeNum << std::endl;
						os << m_FeatureNum << std::endl;
						pcl_ForEach(m_Coefficient, item) {
							const std::vector<double> &coef = *item;
							os << coef[0];
							for (int i=1; i<m_FeatureNum; ++i) os << " " << coef[i];
							os << std::endl;
						}
						os << m_Offset[0];
						for (int i=1; i<m_PartitionNum; ++i) os << " " << m_Offset[i];
						os << std::endl;
					} else {
						os << "BIN\n";
						os.write((char*)&m_NodeNum, sizeof(int));
						os.write((char*)&m_FeatureNum, sizeof(int));
						pcl_ForEach(m_Coefficient, item) {
							const std::vector<double> &coef = *item;
							for (int i=0; i<m_FeatureNum; ++i) os.write((char*)&coef[i], sizeof(double));
						}
						for (int i=0; i<m_PartitionNum; ++i) os.write((char*)&m_Offset[i], sizeof(double));
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
						int node_num = atoi(buffer.c_str());
						setNodeNum(node_num);
						std::getline(is, buffer);
						m_FeatureNum = atoi(buffer.c_str());
						pcl_ForEach(m_Coefficient, item) {
							std::vector<double> &coef = *item;
							coef.resize(m_FeatureNum);
							readVector(is, coef);
						}
						readVector(is, m_Offset);
					} else if (strcmp(header, "BIN\n")==0) {
						int node_num;
						is.read((char*)&node_num, sizeof(int));
						setNodeNum(node_num);
						is.read((char*)&m_FeatureNum, sizeof(int));
						pcl_ForEach(m_Coefficient, item) {
							std::vector<double> &coef = *item;
							coef.resize(m_FeatureNum);
							for (int i=0; i<m_FeatureNum; ++i) is.read((char*)&coef[i], sizeof(double));
						}
						for (int i=0; i<m_PartitionNum; ++i) is.read((char*)&m_Offset[i], sizeof(double));
					} else {
						pcl_ThrowException(Exception(), "Invalid header \"" + std::string(header) + "\" encountered!");
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

		protected:
			int m_NodeNum, m_PartitionNum, m_FeatureNum;
			double m_Multiplier;
			std::vector< std::vector<double> > m_Coefficient;
			std::vector<double> m_Offset;

			template <class DataType>
			double getProjectedValue(const DataType& data, const std::vector<double>& coefficient) const
			{
				double result = 0;
				for (int i=0; i<coefficient.size(); ++i) {
					result += coefficient[i]*data[i];
				}
				return result;
			}

			template <class TrainingDataListType, class RandomGenerator>
			void randomizeCoefficient(std::vector<double>& coefficient, double& offset, const TrainingDataListType& training_data_list, RandomGenerator& rnd_gen)
			{
				boost::random::uniform_real_distribution<> rand_dist(-1,1);
				for (int i=0; i<coefficient.size(); ++i) {
					coefficient[i] = rand_dist(rnd_gen);
				}

				double min_range = std::numeric_limits<double>::infinity(), 
					max_range = -std::numeric_limits<double>::infinity();
				bool init = true;
				pcl_ForEach(training_data_list, item) {
					double val = getProjectedValue(*item, coefficient);
					min_range = pcl_Min(min_range, val);
					max_range = pcl_Max(max_range, val);
				}
				boost::random::uniform_real_distribution<> range_dist(min_range, max_range);
				offset = range_dist(rnd_gen);
			}

			void readVector(std::istream& is, std::vector<double>& vec)
			{
				std::string buffer;
				std::getline(is, buffer);
				pcl::misc::StringTokenizer tokenizer(buffer.c_str());
				int count = 0;
				for (tokenizer.begin(' '); !tokenizer.end(); tokenizer.next(' ')) {
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