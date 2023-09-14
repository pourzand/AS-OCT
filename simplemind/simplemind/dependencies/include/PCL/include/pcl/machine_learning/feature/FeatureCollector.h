#ifndef PCL_FEATURE_FEATURE_COLLECTOR
#define PCL_FEATURE_FEATURE_COLLECTOR
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include <pcl/machine_learning/feature/FeatureBase.h>
#include <pcl/exception.h>
#include <pcl/macro.h>

namespace pcl
{
	namespace feature
	{
		
		template <class T>
		class SubFeature: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef boost::shared_ptr<SubFeature<T>> Pointer;
			
			struct FeatureInfo
			{
				size_t ind;
				size_t pos;
				
				FeatureInfo()
				{}
				
				FeatureInfo(size_t i, size_t p)
				{
					ind = i;
					pos = p;
				}
			};
			
			static Pointer New(std::vector<const std::vector<typename FeatureBase<T>::Pointer>>& feature, const boost::unordered_map<size_t, std::vector<FeatureInfo>>& lookup, const std::vector<std::string>& name)
			{
				return Pointer(new SubFeature<T>(feature, lookup, name));
			}
			
			virtual void forceNextCompute()
			{
				pcl_ForEach(m_Lookup, item) {
					m_Feature[item->first]->forceNextCompute();
				}
			}

			virtual void compute(const Point3D<int>& p, long i)
			{
				pcl_ForEach(m_Lookup, item) {
					auto &feat = m_Feature[item->first];
					feat->compute(p,i);
					pcl_ForEach(item->second, info) {
						m_Value[info->pos] = feat->getResult(info->ind);
					}
				}
			}

			virtual T getResult(size_t i=0) const
			{
				return m_Value[i];
			}

			virtual void populateResult(std::vector<T>& result) const
			{
				pcl_ForEach(m_Value, item) result.push_back(*item);
			}

			virtual void populateResult(std::vector<T>& result, size_t offset) const
			{
				pcl_ForEach(m_Value, item) {
					result[offset] = *item;
					++offset;
				}
			}

			virtual std::string getFeatureName(size_t i=0) const
			{
				return m_Name[i];
			}
			
			const std::vector<T>& getResultVector() const
			{
				return m_Value;
			}
			
		protected:
			std::vector<typename FeatureBase<T>::Pointer> m_Feature;
			boost::unordered_map<size_t, std::vector<FeatureInfo>> m_Lookup;
			std::vector<std::string> m_Name;
			std::vector<T> m_Value;
			
			SubFeature(const std::vector<typename FeatureBase<T>::Pointer>& feature, const boost::unordered_map<size_t, std::vector<FeatureInfo>>& lookup, const std::vector<std::string>& name)
			{
				m_Feature = feature;
				m_Lookup = lookup;
				m_Name = name;
				this->m_Size = m_Name.size();
				m_Value.resize(this->m_Size);
			}
		};

		template <class T>
		class FeatureCollector: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef boost::shared_ptr<FeatureCollector<T>> Pointer;

			static Pointer New()
			{
				return Pointer(new FeatureCollector<T>());
			}

			void add(const FeatureBase<T>::Pointer& feature)
			{
				m_Feature.push_back(feature);
				this->m_Size += feature->size();
				for (size_t i=0; i<feature->size(); ++i) {
					m_Info.push_back(FeatureInfo(m_Feature.size()-1, i, feature->getFeatureName(i)));
					if (m_Lookup.find(feature->getFeatureName(i))!=m_Lookup.end()) {
						pcl_ThrowException(pcl::Exception(), "Duplicate feature name \""+feature->getFeatureName(i)+"\" encountered!");
					}
					m_Lookup[feature->getFeatureName(i)] = m_Info.size()-1;
				}
				m_Value.resize(this->m_Size);
			}

			virtual void forceNextCompute()
			{
				pcl_ForEach(m_Feature, item) (*item)->forceNextCompute();
			}

			virtual void compute(const Point3D<int>& p, long i)
			{
				size_t offset = 0;
				pcl_ForEach(m_Feature, item) {
					(*item)->compute(p,i);
					(*item)->populateResult(m_Value, offset);
					offset += (*item)->size();
				}
			}

			virtual T getResult(size_t i=0) const
			{
				return m_Value[i];
			}

			virtual void populateResult(std::vector<T>& result) const
			{
				pcl_ForEach(m_Value, item) result.push_back(*item);
			}

			virtual void populateResult(std::vector<T>& result, size_t offset) const
			{
				pcl_ForEach(m_Value, item) {
					result[offset] = *item;
					++offset;
				}
			}

			virtual std::string getFeatureName(size_t i=0) const
			{
				return m_Info[i].name;
			}
			
			const std::vector<T>& getResultVector() const
			{
				return m_Value;
			}

			template <class ListType>
			typename SubFeature<T>::Pointer getSubFeature(const ListType& name_list)
			{
				typedef typename SubFeature<T>::FeatureInfo Info;
				boost::unordered_map<int, std::vector<Info>> lookup;
				std::vector<std::string> name;
				name.reserve(name_list.size());
				pcl_ForEach(name_list, item) {
					auto info = m_Lookup.find(*item);
					if (info==m_Lookup.end()) {
						pcl_ThrowException(pcl::Exception(), "Cannot find feature name \""+*item+"\"!");
					}
					name.push_back(*item);
					auto entry = lookup.find(info->second.ind);
					if (entry==lookup.end()) {
						std::vector<Info> temp;
						temp.push_back(Info(info->second.sub, name.size()-1));
						lookup[info->second.ind] = temp;
					} else {
						entry->second.push_back(Info(info->second.sub, name.size()-1));
					}
				}
				return SubFeature<T>::New(m_Feature, lookup, name);
			}

		protected:
			struct FeatureInfo{
				size_t ind;
				size_t sub;
				std::string name;
				
				FeatureInfo()
				{}
				
				FeatureInfo(size_t i, size_t s, const std::string& n)
				{
					ind = i;
					sub = s;
					name = n;
				}
			};
			std::vector<FeatureInfo> m_Info;
			boost::unordered_map<std::string, size_t> m_Lookup;
			std::vector<typename FeatureBase<T>::Pointer> m_Feature;
			std::vector<T> m_Value;

			FeatureCollector()
			{
				this->m_Size = 0;
			}			
		};
	}
}

#endif