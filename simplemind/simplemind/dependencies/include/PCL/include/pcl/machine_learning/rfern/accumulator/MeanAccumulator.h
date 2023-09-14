#ifndef PCL_MEAN_ACCUMULATOR
#define PCL_MEAN_ACCUMULATOR

namespace pcl
{
	namespace rfern
	{

		template <class ObservationType>
		class MeanAccumulator
		{
		public:
			MeanAccumulator()
			{
				m_IsWeighted = false;
			}

			template <class ObservationList>
			ObservationType& compute(ObservationType& observation, ObservationList& list) const
			{
				observation.getOutput().resize((*list.begin()).output_size());
				for (int i=0; i<observation.output_size(); i++) observation.output() = 0;

				if (m_IsWeighted) {
					double total_weight = 0;
					pcl_ForEach(list, item) {
						ObservationType& cur = *item;
						for (int i=0; i<observation.output_size(); i++) {
							observation.output() += cur.output()*cur.weight();
						}
						total_weight += cur.weight();
					}
					for (int i=0; i<observation.output_size(); i++) {
						observation.output() /= total_weight;
					}
				} else {
					pcl_ForEach(list, item) {
						ObservationType& cur = *item;
						for (int i=0; i<observation.output_size(); i++) {
							observation.output() += item.output();
						}
					}
					for (int i=0; i<observation.output_size(); i++) {
						observation.output() /= list.size();
					}
				}

				return observation;
			}

		protected:
			bool m_IsWeighted;
		};

	}
}

#endif