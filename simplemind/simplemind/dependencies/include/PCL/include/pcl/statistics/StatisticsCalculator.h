#ifndef PCL_STATISTICS_CALCULATOR
#define PCL_STATISTICS_CALCULATOR

#include <pcl/macro.h>
#include <math.h>
#include <numeric>
#include <limits>

namespace pcl
{
	namespace statistics
	{

		//Based on the algorithms in http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Parallel_algorithm
		class StatisticsCalculator 
		{
		public:
			enum 
			{
				MEAN=1, VARIANCE=2, SKEWNESS=3, KURTOSIS=4
			};


			StatisticsCalculator() 
			{
				reset();
			}

			StatisticsCalculator(int highest_moment)
			{
				reset(highest_moment);
			}

			void reset(int highest_moment=KURTOSIS)
			{
				m_HighestMoment = highest_moment;
				n = 0;
				mean = 0;
				M2 = 0;
				M2_var = 0;
				M3 = 0;
				M4 = 0;
				m_Min = std::numeric_limits<double>::infinity();
				m_Max = -std::numeric_limits<double>::infinity();
			}

			void addValue(double val) {
				update(val);
				if (m_Min>val) m_Min = val;
				if (m_Max<val) m_Max = val;
			}
			
			template <class MapType>
			void addMap(const MapType& map)
			{
				pcl_ForEach(map, item) {
					long count = item->second;
					double val = item->first;
					for (long i=0; i<count; ++i) update(val);
				}
			}
			
			/************************* Results related methods *************************/

			double getMean() const
			{
				return mean;
			}

			double getStandardDeviation(bool bias_corrected) const
			{
				if (bias_corrected) return getBiasCorrectedStandardDeviation();
				else return getStandardDeviation();
			}
			double getStandardDeviation() const
			{
				return sqrt(getVariance());
			}
			double getBiasCorrectedStandardDeviation() const
			{
				return sqrt(getBiasCorrectedVariance());
			}

			double getVariance(bool bias_corrected) const
			{
				if (bias_corrected) return getBiasCorrectedVariance();
				else return getVariance();
			}
			double getVariance() const
			{
				return M2_var/n;
			}
			double getBiasCorrectedVariance() const
			{

				return M2_var/(n-1);
			}

			double getSkewness(bool bias_corrected) const
			{
				if (bias_corrected) return getBiasCorrectedSkewness();
				else return getSkewness();
			}
			double getSkewness() const
			{
				return sqrt(n)*M3/pow(M2,3.0/2.0);
			}
			double getBiasCorrectedSkewness() const
			{
				return getSkewness()*sqrt(n*(n-1))/(n-2);
			}

			double getKurtosis(bool bias_corrected) const
			{
				if (bias_corrected) return getBiasCorrectedKurtosis();
				else return getKurtosis();
			}
			double getKurtosis() const
			{    
				return (n*M4)/(M2*M2) - 3;
			}
			double getBiasCorrectedKurtosis() const
			{
				return ( (n-1)/( (n-2)*(n-3) ) ) * ( (n+1)*(getKurtosis()+3) - 3*(n-1) );
			}

			double getMin() const
			{ 
				return m_Min; 
			}
			double getMax() const
			{ 
				return m_Max; 
			}

			double getNum() const 
			{ 
				return n; 
			}

		protected:
			int m_HighestMoment;
			double n, mean, M2, M2_var, M3, M4;
			double delta, delta_n, delta_n2, term1;
			double m_Min, m_Max;

			void update(double val) {
				updateMean(val);
				if (m_HighestMoment<=1) return;
				updateVariance(val);
				if (m_HighestMoment==2) return;
				if (m_HighestMoment==3) {
					updateGeneral();
					updateSkewness();
				} else {        
					updateGeneral();
					updateKurtosis();
					updateSkewness();
				}
			}

			//Individual updates
			void updateMean(double x) 
			{
				n++;
				delta = x - mean;
				delta_n = delta / n;
				mean += delta_n;
			}

			void updateVariance(double x) 
			{
				M2_var += delta*(x - mean);
			}

			void updateGeneral() 
			{
				delta_n2 = delta_n * delta_n;
				term1 = delta * delta_n * (n-1);
			}

			void updateKurtosis() 
			{
				M4 += term1 * delta_n2 * (n*n - 3*n + 3) + 6 * delta_n2 * M2 - 4 * delta_n * M3;
			}

			void updateSkewness() 
			{
				M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
				M2 += term1;
			}
		};

	}
}

#endif