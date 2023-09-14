#ifndef PCL_VARIABLE_WIDTH_HISTOGRAM
#define PCL_VARIABLE_WIDTH_HISTOGRAM

#include <pcl/macro.h>
#include <boost/smart_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/array.hpp>
#include <vector>

#pragma warning ( push )
#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4267 )

namespace pcl
{

	/*
	A histogram class that supports variable bin width defined based on edges. A bin i is defined as [edge(i), edge(i+1)).
	*/
	class VariableWidthHistogram
	{
	public:
		typedef VariableWidthHistogram Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef boost::shared_ptr<const Self> ConstantPointer;

		template <class ListType>
		static Pointer New(const ListType& edge)
		{
			Pointer obj(new Self);
			obj->m_Edge.reserve(edge.size());
			obj->m_Bin.resize(edge.size()-1);
			pcl_ForEach(edge, item) {
				obj->m_Edge.push_back(*item);
			}
			pcl_ForEach(obj->m_Bin, item) {
				*item = 0;
			}
			return obj;
		}

		bool addValue(double val, double amount=1)
		{
			int bin = findBin(val);
			if (bin==-1) return false;
			m_Bin[bin] += amount;
			return true;
		}

		int findBin(double val) const
		{
			if (val<m_Edge[0] || val>m_Edge[m_Edge.size()-1]) return -1;
			int min_ind = 0, max_ind = m_Bin.size()-1;
			int pivot = m_Bin.size()*0.5;
			bool go_left;
			while(true) {
				if (val>=m_Edge[pivot] && val<m_Edge[pivot+1]) return pivot;
				if (m_Edge[pivot]>val) { //lies at right side
					max_ind = pivot;
					go_left = true;
				} else {
					min_ind = pivot;
					go_left = false;
				}
				pivot = min_ind + (max_ind-min_ind)*0.5;
				if ((max_ind-min_ind)==1 && !go_left) pivot++;
			}
		}

		double& getBin(int i)
		{
			return m_Bin[i];
		}
		double getBin(int i) const
		{
			return m_Bin[i];
		}

		boost::tuple<double, double> getEdge(int i) const
		{
			return boost::tuple<double, double>(m_Edge[i], m_Edge[i+1]);
		}

		int size() const
		{
			return m_Bin.size();
		}

		const std::vector<double>& edge() const
		{
			return m_Edge;
		}

		const std::vector<double>& bin() const
		{
			return m_Bin;
		}

		double getTotalWeight() const
		{
			double result = 0;
			pcl_ForEach(m_Bin, item) result += *item;
			return result;
		}

		void normalize()
		{
			double total = getTotalWeight();
			pcl_ForEach(m_Bin, item) *item /= total;
		}

		Pointer getCopy() const
		{
			Pointer obj(new Self);
			obj->m_Edge = m_Edge;
			obj->m_Bin = m_Bin;
			return obj;
		}

	protected:
		std::vector<double> m_Edge;
		std::vector<double> m_Bin; //Note: Size of bins is always one less than the size of edges!

		VariableWidthHistogram() {}
	};

}

#pragma warning ( pop )
#endif