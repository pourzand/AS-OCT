#ifndef PCL_TRAINING_DATA
#define PCL_TRAINING_DATA

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <pcl/exception.h>
#include <pcl/macro.h>

namespace pcl
{

	template <class V, class L, class I=char> 
	class TrainingData
	{
	public:
		typedef V ValueType;
		typedef std::vector<ValueType> FeatureType;
		typedef L LabelType;
		typedef I LabelIndexType;

		TrainingData()
		{
			m_FeatureSize = -1;
			m_Update = false;
		}
		
		void push(const FeatureType& feat, const LabelType& label)
		{
			if (m_FeatureSize==-1) m_FeatureSize = feat.size();
			else if (m_FeatureSize!=feat.size()) pcl_ThrowException(pcl::Exception(), "Invalid feature num encountered!");
			m_Feature.push_back(feat);
			m_Label.push_back(label);
			m_LabelValueLookup[label] = 0;
			m_Update = false;
		}
		
		void push(FeatureType&& feat, const LabelType& label)
		{
			if (m_FeatureSize==-1) m_FeatureSize = static_cast<int>(feat.size());
			else if (m_FeatureSize!=feat.size()) pcl_ThrowException(pcl::Exception(), "Invalid feature num encountered!");
			m_Feature.push_back(std::move(feat));
			m_Label.push_back(label);
			m_LabelValueLookup[label] = 0;
			m_Update = false;
		}

		template <class ListType, class KeyListType>
		void push(const ListType& input, const KeyListType& keys, const LabelType& label)
		{
			if (m_FeatureSize==-1) m_FeatureSize = keys.size();
			else if (m_FeatureSize!=keys.size()) pcl_ThrowException(pcl::Exception(), "Invalid feature num encountered!");
			FeatureType feat;
			feat.reserve(m_FeatureSize);
			pcl_ForEach(keys, item) feat.push_back(input.at(*item));
			push(std::move(feat), label);
		}

		void update()
		{
			m_InverseLabelValueLookup.clear();
			m_InverseLabelValueLookup.reserve(m_LabelValueLookup.size());
			pcl_ForEach(m_LabelValueLookup, items) m_InverseLabelValueLookup.push_back(items->first);
			std::sort(m_InverseLabelValueLookup.begin(), m_InverseLabelValueLookup.end());
			for (int i=0; i<m_InverseLabelValueLookup.size(); ++i) m_LabelValueLookup[m_InverseLabelValueLookup[i]] = static_cast<LabelIndexType>(i);
			m_Update = true;
		}

		size_t size() const
		{
			return m_Feature.size();
		}

		size_t featureSize() const
		{
			return m_FeatureSize;
		}

		size_t uniqueLabelSize() const
		{
			return m_InverseLabelValueLookup.size();
		}

		std::vector<FeatureType>& features()
		{
			return m_Feature;
		}

		const std::vector<FeatureType>& features() const
		{
			return m_Feature;
		}

		std::vector<LabelType>& labels()
		{
			return m_Label;
		}

		const std::vector<LabelType>& labels() const
		{
			return m_Label;
		}

		template <class T>
		std::vector<T> labelIndexes() const
		{
			if (!m_Update) pcl_ThrowException(pcl::Exception(), "Training data is not updated!");
			std::vector<T> result;
			result.resize(m_Label.size());
			for (int i=0; i<size(); ++i) result[i] = toLabelIndex(m_Label[i]);
			return std::move(result);
		}

		const LabelType& toLabel(LabelIndexType i) const
		{
			return m_InverseLabelValueLookup[i];
		}

		LabelIndexType toLabelIndex(const LabelType& label) const
		{
			return m_LabelValueLookup.at(label);
		}

	protected:
		size_t m_FeatureSize;
		std::vector<FeatureType> m_Feature;
		std::vector<LabelType> m_Label;
		boost::unordered_map<LabelType,LabelIndexType> m_LabelValueLookup;
		std::vector<LabelType> m_InverseLabelValueLookup;
		bool m_Update;
	};

}

#endif