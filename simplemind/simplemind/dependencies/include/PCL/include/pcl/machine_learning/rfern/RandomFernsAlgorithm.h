#ifndef PCL_RANDOM_FERNS_ALGORITHM
#define PCL_RANDOM_FERNS_ALGORITHM

#include <pcl/exception.h>
#include <pcl/machine_learning/rfern/Fern.h>
#include <pcl/machine_learning/ClassificationTrainingObservation.h>
#include <pcl/machine_learning/Observation.h>
#include <boost/smart_ptr.hpp>

namespace pcl
{
	namespace rfern
	{

		template <class ObservationType, class TrainingType, class HashType, template<class,class> class BasisClass, template<class> class AccumulatorClass>
		class RandomFernsAlgorithm
		{
		protected:
			typedef Fern<ObservationType,TrainingType,HashType,BasisClass> FernType;
			typedef boost::scoped_ptr<FernType> FernPointerType;
		public:
			typedef typename FernType::HashType::Parameter HashParamType;
			typedef typename FernType::BasisType::Parameter BasisParamType;
			typedef typename TrainingType TrainingDataType;
			typedef typename ObservationType ResultType;
			typedef typename AccumulatorClass<ObservationType> AccumulatorType;

			RandomFernsAlgorithm() {}
			RandomFernsAlgorithm(int fern_num, int node_num)
			{
				setFernNum(fern_num, node_num);
			}

			//************* Setup methods
			void setFernNum(int fern_num, int node_num)
			{
				m_FernNum = fern_num;
				m_NodeNum = node_num;
				m_Fern.reset(new FernPointerType[m_FernNum]);
				for (int i=0; i<m_FernNum; i++) m_Fern[i].reset(new FernType(m_NodeNum));
			}

			template <class TrainingDataListType>
			void train(const typename HashParamType& h_param, const typename BasisParamType& b_param, const TrainingDataListType& training_data_list)
			{
				for (int i=0; i<m_FernNum; i++) {
					m_Fern[i]->initialize(h_param, b_param);
					m_Fern[i]->train(training_data_list);
				}
			}

			template <class TrainingDataListType>
			void train(const TrainingDataListType& training_data_list)
			{
				train(HashParamType(), BasisParamType(), training_data_list);
			}

			AccumulatorType& getAccumulator()
			{
				return m_Accumulator;
			}

			//************* Compute method
			ObservationType& compute(ObservationType& observation) const
			{
				std::vector<ObservationType> list;
				list.reserve(m_NodeNum);
				for (int i=0; i<m_NodeNum; i++) {
					ObservationType cur(observation.feature(), ObservationType.CreateFeatureVector(), 1);
					m_Fern[i]->compute(cur);
					list.push_back(cur);
				}
				return m_Accumulator.compute(observation, list);
			}

			//************* IO methods
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					if (!use_binary) {
						os << "TXT\n";
						os << m_FernNum << std::endl;
						os << m_NodeNum << std::endl;
					} else {
						os << "BIN\n";
						os.write((char*)&m_FernNum, sizeof(m_FernNum));
						os.write((char*)&m_NodeNum, sizeof(m_NodeNum));
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
				for (int i=0; i<m_FernNum; i++) {
					m_Fern[i]->write(os, use_binary);
				}
			}

			void read(std::istream& is)
			{
				try {
					char header[5];
					is.read(header, 4); header[4] = NULL;
					if (strcmp(header, "TXT\n")==0) {
						std::string buffer;
						std::getline(is, buffer); m_FernNum = atoi(buffer.c_str());
						std::getline(is, buffer); m_NodeNum = atoi(buffer.c_str());
					} else if (strcmp(header, "BIN ")==0) {
						is.read((char*)&m_FernNum, sizeof(m_FernNum));
						is.read((char*)&m_NodeNum, sizeof(m_NodeNum));
					} else {
						pcl_ThrowException(Exception(), "Invalid header \"" + header + "\" encountered");
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
				m_Fern.reset(new FernPointerType[m_FernNum]);
				for (int i=0; i<m_FernNum; i++) {
					m_Fern[i].reset(new FernType(m_NodeNum));
					m_Fern[i]->read(is, use_binary);
				}
			}

		protected:
			boost::scoped_array<FernPointerType> m_Fern;
			AccumulatorType m_Accumulator;
			int m_FernNum, m_NodeNum;
		};

	}
}

#endif