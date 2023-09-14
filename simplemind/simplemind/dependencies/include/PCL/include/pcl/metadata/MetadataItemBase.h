#ifndef PCL_METADATA_ITEM_BASE
#define PCL_METADATA_ITEM_BASE

#include <typeinfo>
#include <string>
#include <ostream>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>

namespace pcl
{

	class MetadataItemBase: private boost::noncopyable
	{
	public:
		typedef MetadataItemBase Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef boost::shared_ptr<const Self> ConstantPointer;

		virtual const std::type_info& type() const = 0;
		virtual int size() const = 0;

		operator std::string() const
		{
			return this->toString(' ');
		}
		virtual std::string toString(char delimiter=' ') const = 0;

		virtual void print(std::ostream& stream) const = 0;

		virtual Pointer getCopy() const = 0;

	protected:
		int m_DataIdentifier;

		MetadataItemBase(int data_identifier):m_DataIdentifier(data_identifier) 
		{}
	};

}

#endif