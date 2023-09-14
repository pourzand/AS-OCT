#ifndef PCL_FERN
#define PCL_FERN

#include <pcl/macro.h>
#include <boost/utility.hpp>
#include <iostream>
#include <vector>

namespace pcl
{
	namespace rfern
	{

		template <class OType, class TDType, class HashType, template<class,class> class BasisClass>
		class Fern 
		{
			typedef OType ObservationType;
			typedef TDType TrainingDataType;
			typedef BasisClass<ObservationType,TrainingDataType> BasisType;

			Fern(int node_num) 
			{
				m_Basis.resize(node_num);
				m_Hash.setNodeNum(node_num);
			}

			void intialize(const typename HashType::Parameter& h_param, const typename BasisType::Parameter& b_param) 
			{
				m_Hash.initialize(h_param);
				pcl_ForEach(m_Basis, item) {
					item->initialize(b_param);
				}
			}

			template <class TrainingDataListType>
			void train(const TrainingDataListType& training_data_list)
			{
				m_Hash.train(training_data);
				pcl_ForEach(training_data_list, item) {
					int index = m_Hash.computeIndex(*item);
					for (int i=0; i<m_Basis.size(); i++) {
						m_Basis[i].addTrainingData(*item, index==i);
					}
				}
				for (int i=0; i<m_Basis.size(); i++) m_Basis[i].update();
			}

			ObservationType& compute(ObservationType& observation) const
			{
				int index = m_Hash.computeIndex(observation);
				m_Basis[index].compute(observation);
				return observation;
			}

			/************ IO methods ************/
			void write(std::ostream& os, bool use_binary=false) const
			{
				m_Hash.write(os, use_binary);
				for (int i=0; i<m_Basis.size(); i++) {
					m_Basis[i].write(os, use_binary);
				}
			}

			void read(std::istream& is)
			{
				m_Hash.read(is);
				for (int i=0; i<m_Basis.size(); i++) {
					m_Basis[i].read(is);
				}
			}

		protected:
			std::vector<BasisType> m_Basis;
			HashType m_Hash;
		};

	}
}

#endif