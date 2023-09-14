#ifndef PCL_PERCENTILE_CALCULATOR
#define PCL_PERCENTILE_CALCULATOR

//#include <macro.h>
#include <math.h>
#include <map>
#include <numeric>
#include <boost/math/special_functions/fpclassify.hpp>

#define pcl_ForEach(source, item) for (auto item=(source).begin(), m_end_##item=(source).end(); item!=m_end_##item; ++item)

//namespace pcl
//{
//	namespace statistics
//	{

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
					} else if (cur_cummulative>target_cummulative) {
						next_key = item->first;
						next_val = cur_cummulative;
						break;
					}
				}
				//Handle special case for rounding error
				if (boost::math::isnormal(prev_val) && !boost::math::isnormal(next_val)) return prev_key;
				else if (!boost::math::isnormal(prev_val) && boost::math::isnormal(next_val)) return next_key;
				else if (!boost::math::isnormal(prev_val) && !boost::math::isnormal(next_val)) return std::numeric_limits<double>::quiet_NaN();
				//Computing interpolation
				if (boost::math::isnormal(prev_val)) prev_val = (prev_val-0.5)/m_Num;
				if (boost::math::isnormal(next_val)) next_val = (next_val-0.5)/m_Num;
				double s = (val-prev_val)/(next_val-prev_val);
				return (1-s)*prev_key + s*next_key;
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

//	}
//}

#endif