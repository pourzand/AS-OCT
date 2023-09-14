#ifndef PCL_COMPUTE_ON_DEMAND_BASE
#define PCL_COMPUTE_ON_DEMAND_BASE

#include <pcl/image.h>
#include <pcl/exception.h>
#include <boost/numeric/conversion/bounds.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

namespace pcl
{
	namespace cod
	{
		using namespace pcl;
		
		struct IncompatibleCodException: public pcl::Exception {};

		template <bool> class CodBase;

		namespace details
		{
			template <class T>
			class has_element_type
			{
				typedef char yes;
				typedef short no;

				template <class C> static yes test(typename C::element_type*);
				template <class C> static no test(...);
			public:
				enum { value = sizeof(test<T>(0)) == sizeof(yes) };
			};

			template <class T, bool Enable=has_element_type<T>::value>
			struct is_base_of_CodBase
			{
				enum { value = boost::is_base_of<CodBase<true>,typename T::element_type>::value || boost::is_base_of<CodBase<false>,typename T::element_type>::value };
			};

			template <class T>
			struct is_base_of_CodBase<T, false>
			{
				enum { value = boost::is_base_of<CodBase<true>,T>::value || boost::is_base_of<CodBase<false>,T>::value };
			};
		}

		template <bool Flag>
		class CodBase: private boost::noncopyable
		{
		public:
			enum { ReferableViaIndex = Flag };
			typedef boost::shared_ptr<CodBase> Pointer;

			template <class ImagePointerType>
			inline typename boost::disable_if<details::is_base_of_CodBase<ImagePointerType>, bool>::type isCompatible(const ImagePointerType& img) const
			{
				if (isUnbounded()) return true;
				if (m_MinPointIndex==img->localToIndex(img->getMinPoint()) && m_MaxPointIndex==img->localToIndex(img->getMaxPoint()) && m_Region==img->getRegion()) return true;
				return false;
			}
			
			template <class CodPointerType>
			inline typename boost::enable_if<details::is_base_of_CodBase<CodPointerType>, bool>::type isCompatible(const CodPointerType& obj) const
			{
				if (isUnbounded() || obj->isUnbounded()) return true;
				if (m_MinPointIndex==obj->getMinPointIndex() && m_MaxPointIndex==obj->getMaxPointIndex() && m_Region==obj->getRegion()) return true;
				return false;
			}
			
			bool referableViaIndex() const
			{
				return ReferableViaIndex;
			}

			inline bool isUnbounded() const
			{
				if (m_MinPointIndex==boost::numeric::bounds<long>::lowest()) return true;
				return false;
			}

			inline long getMinPointIndex() const
			{
				return m_MinPointIndex;
			}
			
			inline long getMaxPointIndex() const
			{
				return m_MaxPointIndex;
			}

			inline const Region3D<int>& getRegion() const
			{
				return m_Region;
			}

		protected:
			void setRegionInfo() //Indicate the the operator is unbounded (as in not limited by a buffer)
			{
				m_MinPointIndex = boost::numeric::bounds<long>::lowest();
				m_Region.reset();
			}

			void setRegionInfo(long minp_index, long maxp_index, const Region3D<int>& region)
			{
				m_MinPointIndex = minp_index;
				m_MaxPointIndex = maxp_index;
				m_Region = region;
			}
			
			template <class ImagePointerType>
			typename boost::disable_if<details::is_base_of_CodBase<ImagePointerType>, void>::type setRegionInfo(const ImagePointerType& image)
			{
				m_MinPointIndex = image->localToIndex(image->getMinPoint());
				m_MaxPointIndex = image->localToIndex(image->getMaxPoint());
				m_Region = image->getRegion();
			}
			
			template <class CodPointerType>
			typename boost::enable_if<details::is_base_of_CodBase<CodPointerType>, void>::type setRegionInfo(const CodPointerType& obj)
			{
				if (obj->isUnbounded()) {
					setRegionInfo();
					return;
				}
				m_MinPointIndex = obj->getMinPointIndex();
				m_MaxPointIndex = obj->getMaxPointIndex();
				m_Region = obj->getRegion();
			}

			CodBase() {}

		private:
			long m_MinPointIndex, m_MaxPointIndex;
			Region3D<int> m_Region;
		};

	}
}

#endif