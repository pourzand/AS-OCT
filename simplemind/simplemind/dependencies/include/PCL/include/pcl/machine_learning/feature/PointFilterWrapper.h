#ifndef PCL_FEATURE_POINT_FILTER_WRAPPER
#define PCL_FEATURE_POINT_FILTER_WRAPPER
#include <pcl/machine_learning/feature/FeatureBase.h>
#include <pcl/exception.h>

namespace pcl
{
	namespace feature
	{

		template <class T, class P>
		class PointFilterWrapper: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef P PointFilterPointer;
			typedef boost::shared_ptr<PointFilterWrapper<T,P>> Pointer;

			static Pointer New(const P& filter, const std::vector<size_t>& index, const std::vector<std::string>& name)
			{
				if (!name.empty() && name.size()!=index.size()) pcl_ThrowException(pcl::Exception(), "Invalid feature name vector size encountered!");
				return Pointer(new PointFilterWrapper<T,P>(filter, index, name));
			}

			virtual void forceNextCompute()
			{
				m_Filter->forceNextApply();
			}

			virtual void compute(const Point3D<int>& p, long i)
			{
				m_Filter->apply(p,i);
			}

			virtual T getResult(size_t i=0) const
			{
				return static_cast<T>(m_Filter->getResult(m_Index[i]));
			}

			virtual void populateResult(std::vector<T>& result) const
			{
				pcl_ForEach(m_Index, item) result.push_back(m_Filter->getResult(*item));
			}

			virtual void populateResult(std::vector<T>& result, size_t offset) const
			{
				pcl_ForEach(m_Index, item) {
					result[offset] = m_Filter->getResult(*item);
					++offset;
				}
			}

			virtual std::string getFeatureName(size_t i=0) const
			{
				if (m_Name.empty()) return "";
				return m_Name[i];
			}

		protected:
			PointFilterPointer m_Filter;
			std::vector<size_t> m_Index;
			std::vector<std::string> m_Name;

			PointFilterWrapper(const P& filter, const std::vector<size_t>& index, const std::vector<std::string>& name)
			{
				m_Filter = filter;
				m_Index = index;
				m_Name = name;
				this->m_Size = m_Index.size();
			}
		};


		class PointFilterWrapperHelper
		{
		public:
			template <class T, class P>
			static typename PointFilterWrapper<T,P>::Pointer New(const P& filter, const std::string& n="")
			{
				std::vector<size_t> index(1, 0);
				std::vector<std::string> name(1, n);
				return PointFilterWrapper<T,P>::New(filter, index, name);
			}
			
			template <class T, class P>
			static typename PointFilterWrapper<T,P>::Pointer New(const P& filter, size_t i, const std::string& n="")
			{
				std::vector<size_t> index(1, i);
				std::vector<std::string> name(1, n);
				return PointFilterWrapper<T,P>::New(filter, index, name);
			}

			template <class T, class P>
			static typename PointFilterWrapper<T,P>::Pointer New(const P& filter, const std::vector<size_t>& index, const std::vector<std::string>& name=std::vector<std::string>())
			{
				return PointFilterWrapper<T,P>::New(filter, index, name);
			}
		};

	}
}

#endif