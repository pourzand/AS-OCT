#ifndef PCL_FEATURE_IMAGE_WRAPPER
#define PCL_FEATURE_IMAGE_WRAPPER
#include <boost/utility/enable_if.hpp>
#include <pcl/machine_learning/feature/FeatureBase.h>
#include <pcl/exception.h>

namespace pcl
{
	namespace feature
	{

		template <class T, class I, bool UseIndex>
		class ImageWrapper: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef I ImagePointer;
			typedef boost::shared_ptr<ImageWrapper<T,I,UseIndex>> Pointer;
			typedef ImageWrapper<T,I,UseIndex> Self;

			virtual void compute(const Point3D<int>& p, long i)
			{
				m_Value = static_cast<T>(m_Image->get(i));
			}

#include <pcl/machine_learning/feature/ImageWrapper.txx>
		};


		template <class T, class I>
		class ImageWrapper<T,I,false>: public FeatureBase<T>
		{
		public:
			typedef T ValueType;
			typedef I ImagePointer;
			typedef boost::shared_ptr<ImageWrapper<T,I,false>> Pointer;
			typedef ImageWrapper<T,I,false> Self;

			virtual void compute(const Point3D<int>& p, long i)
			{
				m_Value = static_cast<T>(m_Image->get(p));
			}

#include <pcl/machine_learning/feature/ImageWrapper.txx>
		};


		class ImageWrapperHelper
		{
		public:
			template <class T, class I>
			static typename ImageWrapper<T,I,true>::Pointer New(const I& image, const std::string& name="")
			{
				return ImageWrapper<T,I,true>::New(image, name);
			}

			template <class T, bool UseIndex, class I>
			static typename ImageWrapper<T,I, UseIndex>::Pointer New(const I& image, const std::string& name="")
			{
				return ImageWrapper<T,I,UseIndex>::New(image, name);
			}
		};

	}
}

#endif