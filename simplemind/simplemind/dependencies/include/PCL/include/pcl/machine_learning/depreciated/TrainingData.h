#ifndef PCL_TRAINING_DATA
#define PCL_TRAINING_DATA
#include <pcl/machine_learning/ShareableReadOnlyVector.h>
#include <pcl/machine_learning/ShareableReadOnlyUnorderedMap.h>

namespace pcl
{

	class TrainingDataHelper
	{
		template<class T, class A>
		static pcl::ShareableReadOnlyVector<T,A> GetSROV(const vector<T,A>& vec)
		{
			return pcl::ShareableReadOnlyVector<T,A>(vec);
		}

		template<class T, class A>
		static pcl::ShareableReadOnlyVector<T,A> GetSROV(vector<T,A>&& vec)
		{
			return pcl::ShareableReadOnlyVector<T,A>(std::move(vec));
		}

		template<class K, class M, class H, class P, class A>
		static pcl::ShareableReadOnlyUnorderedMap<K,M,H,P,A> GetSROUM(const boost::unordered_map<K,M,H,P,A>& map)
		{
			return pcl::ShareableReadOnlyUnorderedMap<K,M,H,P,A>(map);
		}

		template<class K, class M, class H, class P, class A>
		static pcl::ShareableReadOnlyUnorderedMap<K,M,H,P,A> GetSROUM(boost::unordered_map<K,M,H,P,A>&& map)
		{
			return pcl::ShareableReadOnlyUnorderedMap<K,M,H,P,A>(std::move(map));
		}
	};


	template <class F, class L> 
	struct TrainingData
	{
		typedef F FeatureType;
		typedef L LabelType;
		
		FeatureType feature;
		LabelType label;

		TrainingData()
		{}

		TrainingData(const FeatureType& f, cosnt LabelType& l)
		{
			feature = f;
			label = l;
		}
	};

}

#endif