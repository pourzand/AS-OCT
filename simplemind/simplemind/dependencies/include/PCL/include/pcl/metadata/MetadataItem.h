#ifndef PCL_METADATA_ITEM
#define PCL_METADATA_ITEM

#include <pcl/metadata/MetadataItemBase.h>
#include <sstream>

namespace pcl
{

	template <class Type>
	class MetadataItem: public MetadataItemBase
	{
	public:
		typedef MetadataItem Self;
		typedef MetadataItemBase Parent;
		typedef boost::shared_ptr<Self> Pointer;
		typedef boost::shared_ptr<const Self> ConstantPointer;

		static Pointer New(int num, int data_identifier=0)
		{
			return Pointer(new Self(num, data_identifier));
		}

		static Pointer New(int num, const Type* data, int data_identifier=0)
		{
			Pointer result(new Self(num, data_identifier));
			for (int i=0; i<num; i++) {
				result->get(i) = data[i];
			}
			return result;
		}

		static Pointer New(int num, const Type& data, int data_identifier=0)
		{
			Pointer result(new Self(num, data_identifier));
			const Type* ptr = &data;
			for (int i=0; i<num; i++) {
				result->get(i) = ptr[i];
			}
			return result;
		}

		Type& get(int i=0)
		{
			return m_Data[i];
		}
		const Type& get(int i=0) const 
		{
			return m_Data[i];
		}

		virtual const std::type_info& type() const
		{
			return typeid(Type);
		}

		virtual int size() const
		{
			return m_Num;
		}

		virtual std::string toString(char delimiter=' ') const
		{
			std::stringstream ss;
			ss << m_Data[0];
			for (int i=1; i<m_Num; i++) {
				ss << delimiter << m_Data[i];
			}
			return ss.str();
		}

		virtual void print(std::ostream& stream) const
		{
			stream << "Data type: " << type().name() << std::endl;
			stream << "Element num: " << m_Num << std::endl;
			stream << "Data: " << toString() << std::endl;
		}

		virtual Parent::Pointer getCopy() const
		{
			return New(m_Num, m_Data[0]);
		}

	protected:
		int m_Num;
		boost::scoped_array<Type> m_Data;
		
		MetadataItem(int num, int data_identifier):MetadataItemBase(data_identifier)
		{
			m_Num = num;
			m_Data.reset(new Type[m_Num]);
		}
	};

}

#endif