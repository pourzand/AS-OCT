#ifndef PCL_NAIVE_BAYES_ACCUMULATOR
#define PCL_NAIVE_BAYES_ACCUMULATOR

namespace pcl
{
	namespace rfern
	{

		template <class ObservationType>
		class NaiveBayesAccumulator
		{
		public:
			NaiveBayesAccumulator()
			{}

			template <class ObservationList>
			ObservationType& compute(ObservationType& observation, ObservationList& list) const
			{
				observation.getOutput().resize((*list.begin()).output_size());
				for (int i=0; i<observation.output_size(); i++) observation.output() = 1;

				pcl_ForEach(list, item) {
					ObservationType& cur = *item;
					for (int i=0; i<observation.output_size(); i++) {
						observation.output() *= item.output();
					}
				}

				return observation;
			}
		};

	}
}

#endif