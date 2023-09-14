#ifndef PCL_METADATA
#define PCL_METADATA

#include <pcl/metadata/MetadataItemBase.h>
#include <pcl/metadata/MetadataItem.h>
#include <pcl/macro.h>
#include <pcl/iterator/IteratorMacro.h>
#include <pcl/exception.h>
#include <map>

#ifndef NO_ITK
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#endif

namespace pcl
{

	class Metadata: private boost::noncopyable
	{
	public:
		typedef Metadata Self;
		typedef boost::shared_ptr<Self> Pointer;
		typedef boost::shared_ptr<const Self> ConstantPointer;
		typedef std::map<std::string, MetadataItemBase::Pointer>::iterator Iterator;
		typedef std::map<std::string, MetadataItemBase::Pointer>::const_iterator ConstantIterator;

		static Pointer New()
		{
			return Pointer(new Self);
		}

#ifndef NO_ITK
		static Pointer New(const itk::MetaDataDictionary& itk_meta)
		{
			typedef itk::MetaDataObject<std::string> MetaDataType;
			Pointer result(new Self);
			itk::MetaDataDictionary::ConstIterator end = itk_meta.End();
			for (itk::MetaDataDictionary::ConstIterator item = itk_meta.Begin(); item!=end; item++) {
				MetaDataType::Pointer value = dynamic_cast<MetaDataType*>( item->second.GetPointer() ) ;
				if (value.IsNotNull()) result->add(item->first, MetadataItem<std::string>::New(1, value->GetMetaDataObjectValue()));
			}
			return result;
		}
#endif

		bool add(const std::string& key, const MetadataItemBase::Pointer& value)
		{
			if (!value) return false;
			if (hasKey(key)) return false;
			m_Data[key] = value;
			return true;
		}

		bool hasKey(const std::string& key) const
		{
			return find(key)!=end();
		}
		
		template <class Type>
		Type getValue(const std::string& key, int i=0) const
		{
			ConstantIterator res = find(key);
			if (res==end()) pcl_ThrowException(pcl::Exception(), "Invalid key \""+key+"\" provided");
			return getValue<Type>(res, i);
		}
		template <class Type>
		Type getValue(ConstantIterator& iter, int i=0) const
		{
			if (iter==end()) pcl_ThrowException(pcl::Exception(), "Invalid iterator provided");
			return boost::static_pointer_cast<const MetadataItem<Type>, const MetadataItemBase>(iter->second)->get(i);
		}
		template <class Type>
		Type getValue(Iterator& iter, int i=0)
		{
			if (iter==end()) pcl_ThrowException(pcl::Exception(), "Invalid iterator provided");
			return boost::static_pointer_cast<const MetadataItem<Type>, const MetadataItemBase>(iter->second)->get(i);
		}

		MetadataItemBase::Pointer get(const std::string& key)
		{
			Iterator res = find(key);
			if (res==end()) return MetadataItemBase::Pointer();
			return res->second;
		}
		MetadataItemBase::ConstantPointer get(const std::string& key) const
		{
			ConstantIterator res = find(key);
			if (res==end()) return MetadataItemBase::Pointer();
			return res->second;
		}

		template <class MetadataItemType>
		typename MetadataItemType::Pointer get(const std::string& key) 
		{
			Iterator res = find(key);
			if (res==end()) return MetadataItemType::Pointer();
			return boost::dynamic_pointer_cast<MetadataItemType, MetadataItemBase>(res->second);
		}
		template <class MetadataItemType>
		typename MetadataItemType::ConstantPointer get(const std::string& key) const
		{
			ConstantIterator res = find(key);
			if (res==end()) return MetadataItemType::Pointer();
			return boost::dynamic_pointer_cast<const MetadataItemType, const MetadataItemBase>(res->second);
		}

		template <class MetadataItemType>
		typename MetadataItemType::Pointer getStatic(const std::string& key) 
		{
			Iterator res = find(key);
			if (res==end()) return MetadataItemType::Pointer();
			return boost::static_pointer_cast<MetadataItemType, MetadataItemBase>(res->second);
		}
		template <class MetadataItemType>
		typename MetadataItemType::ConstantPointer getStatic(const std::string& key) const
		{
			ConstantIterator res = find(key);
			if (res==end()) return MetadataItemType::Pointer();
			return boost::static_pointer_cast<const MetadataItemType, const MetadataItemBase>(res->second);
		}

		size_t size() const
		{
			return m_Data.size();
		}

		//Assignment
		void copy(const Metadata::ConstantPointer& obj)
		{
			m_Data.clear();
			pcl_ForEach(*obj, item) {
				m_Data[item->first] = item->second->getCopy();
			}
		}

		//Itk conversion methods
#ifndef NO_ITK
		void populate(itk::MetaDataDictionary& itk_meta) const
		{
			typedef itk::MetaDataObject<std::string> MetaDataType;
			pcl_ForEach(*this, item) {
				MetaDataType::Pointer entryvalue = MetaDataType::New();
				entryvalue->SetMetaDataObjectValue(item->second->toString());
				itk_meta[item->first] = entryvalue;
			}
		}
#endif

		//Meant for STL like behavior
		inline Iterator begin()
		{
			return m_Data.begin();
		}
		inline ConstantIterator begin() const
		{
			return m_Data.begin();
		}

		inline Iterator end()
		{
			return m_Data.end();
		}
		inline ConstantIterator end() const
		{
			return m_Data.end();
		}

		inline Iterator find(const std::string& key)
		{
			return m_Data.find(key);
		}
		inline ConstantIterator find(const std::string& key) const
		{
			return m_Data.find(key);
		}

	protected:
		std::map<std::string, MetadataItemBase::Pointer> m_Data;

		Metadata() {}
	};

}

#endif