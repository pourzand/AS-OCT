#ifndef PCL_PERCENTILE_CALCULATOR
#define PCL_PERCENTILE_CALCULATOR

#include <pcl/macro.h>
#include <math.h>
#include <map>
#include <numeric>
#include <boost/math/special_functions/fpclassify.hpp>

#define JAVA_EPSILON 0.000000001

namespace pcl
{
	namespace statistics
	{

		template <class ValueType>
		class PercentileCalculator
		{
		public:
			PercentileCalculator() 
			{
				m_Num = 0;
			}

			void addValue(const ValueType& val, int count=1)
			{
				auto iter = m_Map.find(val);
				if (iter==m_Map.end()) m_Map[val] = count;
				else iter->second += count;
				m_Num += count;
			}
			template <class MapType>
			void addMap(const MapType& map)
			{
				pcl_ForEach(map, item) {
					addValue(item->first, item->second);
				}
			}

			/************************* Results related methods *************************/

			double getEntropy() const
			{
				double sum = 0;
				double log2 = log(2.0);
				pcl_ForEach(m_Map, item) {
					double prob = double(item->second)/double(m_Num);
					sum += prob*(log(prob)/log2);
				}
				return -sum;
			}
			
			double getJavaEntropy() const
			{
				double sum = 0;
				pcl_ForEach(m_Map, item) {
					double prob = double(item->second)/double(m_Num);
					sum += -prob*log(prob + JAVA_EPSILON);
				}
				return sum;
			}
			
			double getEnergy() const
			{
				double sum = 0;
				pcl_ForEach(m_Map, item) {
					double prob = double(item->second)/double(m_Num);
					sum += prob*prob;
				}
				return sum;
			}

			double getMedian(bool interpolate) const
			{
				if (interpolate) return getInterpolatedMedian();
				else return getMedian();
			}
			double getMedian() const
			{
				return getPercentile(0.5);
			}
			double getInterpolatedMedian() const
			{
				return getInterpolatedPercentile(0.5);
			}

			double getPercentile(double val, bool interpolate) const
			{
				if (interpolate) return getInterpolatedPercentile(val);
				else return getPercentile(val);
			}
			double getPercentile(double val) const
			{
				if (m_Num==0) return std::numeric_limits<double>::quiet_NaN();
				if (val<0) val = 0;
				else if (val>1) val = 1;
				double target_cummulative = m_Num*val;
				double cur_cummulative = 0;
				pcl_ForEach(m_Map, item) {
					cur_cummulative += item->second;
					if (cur_cummulative>=target_cummulative) return item->first;
				}
				return m_Map.rbegin()->first;
			}

			double getInterpolatedPercentile(double val) const
			{
				if (m_Num==0) return std::numeric_limits<double>::quiet_NaN();
                if (m_Map.size()==1) return m_Map.begin()->first;
				if (val<0) val = 0;
				else if (val>1) val = 1;
				double cur_cummulative = 0;
				double target_cummulative = (val*m_Num) + 0.5;
				double prev_key, prev_val = std::numeric_limits<double>::quiet_NaN(),
					next_key, next_val = std::numeric_limits<double>::quiet_NaN();
				pcl_ForEach(m_Map, item) {
					cur_cummulative += item->second;
					if (cur_cummulative<target_cummulative) {
						prev_key = item->first;
						prev_val = cur_cummulative;
					} else {
						next_key = item->first;
						next_val = cur_cummulative;
						break;
					}
				}
                
                //std::cout << "(val, prev_val, next_val, prev_key, next_key) = (" << val << ", " << prev_val << ", " << next_val << ", " << prev_key << ", " << next_key << ")" << std::endl;
                if (!boost::math::isnormal(prev_val)) return next_key;
                if (prev_val+1>target_cummulative) {
                    double s = target_cummulative-prev_val;
                    //std::cout << "s = " << s << std::endl;
                    return (1-s)*prev_key + s*next_key;
                } else return next_key;
			}
			
			template <class EvalFunc>
			long getNum(EvalFunc& eval_func) const
			{
				long num = 0;
				pcl_ForEach(m_Map, item) {
					if (eval_func(item->first)) num += item->second;
				}
				return num;
			}

			long getNum() const 
			{
				return m_Num;
			}

			double getMin() const 
			{
				return m_Map.begin()->first;
			}
			double getMax() const 
			{
				return m_Map.rbegin()->first;
			}

			const std::map<ValueType,long>& getMap() const
			{
				return m_Map;
			}

		protected:
			std::map<ValueType,long> m_Map;
			long m_Num;
		};

	}
}

#endif