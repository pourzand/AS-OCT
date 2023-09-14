#ifndef PCL_FEATURE_FEATURE_BASE
#define PCL_FEATURE_FEATURE_BASE

#include <vector>
#include <string>
#include <pcl/geometry.h>


namespace pcl
{
	namespace feature
	{

		template <class T>
		class FeatureBase
		{
		public:
			typedef T ValueType;
			typedef boost::shared_ptr<FeatureBase<T>> Pointer;

			size_t size() const
			{
				return m_Size;
			}

			virtual void forceNextCompute()=0;
			
			virtual void compute(const Point3D<int>& p, long i)=0;
			virtual T getResult(size_t i=0) const=0;
			virtual void populateResult(std::vector<T>& result) const=0;
			virtual void populateResult(std::vector<T>& result, size_t offset) const=0;

			virtual std::string getFeatureName(size_t i=0) const=0;

		protected:
			size_t m_Size;

			FeatureBase()
			{}
		};
	}
}

#endif