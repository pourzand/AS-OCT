#ifndef PCL_FEATURE_STANDARDIZER
#define PCL_FEATURE_STANDARDIZER

#include <boost/noncopyable.hpp>
#include <pcl/macro.h>
#include <pcl/exception.h>
#include <iostream>
#include <pcl/machine_learning/FeatureIoHelper.h>

namespace pcl
{

	class FeatureStandardizer: private boost::noncopyable
	{
	public:
		FeatureStandardizer() {}

		template <class T>
		void train(const std::vector<std::vector<T>>& data)
		{
			m_Mean.clear();
			m_Std.clear();
			auto feature_size = data[0].size();
			m_Mean.resize(feature_size, 0);
			m_Std.resize(feature_size, 0);
			pcl_ForEach(data, item) {
				for (int i=0; i<feature_size; i++) {
					m_Mean[i] += (*item)[i];
					m_Std[i] += pcl_Square((*item)[i]);
				}
			}
			double item_num = static_cast<double>(data.size());
			for (int i=0; i<feature_size; i++) {
				m_Mean[i] /= item_num;
				m_Std[i] /= item_num;
				m_Std[i] -= pcl_Square(m_Mean[i]);
				m_Std[i] = sqrt(m_Std[i]);
			}
		}

		size_t size() const
		{
			return m_Mean.size();
		}

		template <class T>
		std::vector<T>& normalize(std::vector<T>& feature) const
		{
			if (feature.size()!=m_Mean.size()) pcl_ThrowException(
				pcl::Exception(),
				"Provided feature size ("+boost::lexical_cast<std::string>(feature.size())+") is invalid! (expected "+boost::lexical_cast<std::string>(m_Mean.size())+")"
				);
			for (int i=0; i<m_Mean.size(); i++) {
				feature[i] = (feature[i]-m_Mean[i])/m_Std[i];
			}
			return feature;
		}

		void assign(const std::vector<double>& mean, const std::vector<double>& std)
		{
			if (mean.size()!=std.size()) {
				pcl_ThrowException(Exception(), "Vectors provided is of different size!");
			}
			m_Mean = mean;
			m_Std = std;
		}

		/*********** IO methods ***********/
		void write(std::ostream& os, bool use_binary=false) const
		{
			try {
				FeatureWriter writer(os, use_binary);
				writer.write(m_Mean);
				writer.write(m_Std);
			} catch (const std::ios_base::failure& e) {
				pcl_ThrowException(Exception(), e.what());
			}
		}

		void read(std::istream& is) 
		{
			m_Mean.clear();
			m_Std.clear();
			try {
				FeatureReader reader(is);
				reader.read(m_Mean);
				reader.read(m_Std);
			} catch (const std::ios_base::failure& e) {
				pcl_ThrowException(Exception(), e.what());
			}
		}

		/*********** Data access methods ***********/
		const std::vector<double>& getMean() const
		{
			return m_Mean;
		}

		const std::vector<double>& getStd() const
		{
			return m_Std;
		}

	protected:
		std::vector<double> m_Mean, m_Std;
	};

}

#endif