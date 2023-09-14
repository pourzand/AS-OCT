#ifndef PCL_VOTE_PROBABILITY_CALCULATOR
#define PCL_VOTE_PROBABILITY_CALCULATOR

#include <ANN.h>

namespace pcl
{
	namespace ann
	{

		class VoteProbabilityCalculator
		{
		public:
			VoteProbabilityCalculator() {}

			template <class ResultType, class LabelType>
			void process(ResultType& result, ANNidx* indexes, ANNdist* distance, const LabelType& label, int k) const
			{
				for (int i=0; i<k; ++i) {
					result[label[indexes[i]]] += 1;
					//std::cout << (int)label[indexes[i]];
				}
				//std::cout << std::endl;
				for (int i=0; i<result.size(); ++i) result[i] /= k;
			}
		};

	}
}

#endif