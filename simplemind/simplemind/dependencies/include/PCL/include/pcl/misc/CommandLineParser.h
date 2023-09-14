#ifndef PCL_COMMAND_LINE_PARSER
#define PCL_COMMAND_LINE_PARSER

#include <pcl/misc/StringTokenizer.h>
#include <pcl/exception.h>
#include <pcl/macro.h>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include <sstream>
#include <typeinfo>

#pragma warning ( push )
#pragma warning ( disable : 4101 )

namespace pcl
{

	namespace command_line_details
	{

		class DataElementBase: private boost::noncopyable
		{
		public:
			typedef DataElementBase Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;

			virtual int size() const = 0;
			virtual void add(const std::string& arg) = 0;
			virtual const std::type_info& getType() const = 0;
			virtual const std::string getTypeName() const = 0;
			virtual bool full() const = 0;

			virtual void print(std::ostream& os) const = 0;

			virtual std::string getString(int i) const = 0;

		protected:
			DataElementBase() {}
		};


		template <class Type>
		class DataElement: public DataElementBase
		{
		public:
			typedef DataElement Self;
			typedef DataElementBase Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;
			typedef Type ValueType;
			enum { Enum = false };

			static Pointer New(int num)
			{
				return Pointer(new Self(num));
			}

			virtual int size() const
			{
				return static_cast<int>(m_Data.capacity());
			}

			virtual bool full() const
			{
				return m_Data.size()==m_Data.capacity();
			}

			virtual void add(const std::string& arg)
			{
				if (m_Data.size()==m_Data.capacity()) pcl_ThrowException(pcl::Exception(), "Max data size exceeed!");
				m_Data.push_back(boost::lexical_cast<Type>(arg));
			}

			virtual const std::type_info& getType() const
			{
				return typeid(Type);
			}

			virtual const std::string getTypeName() const
			{
				return getSpecializedTypeName<Type>();
			}

			virtual void print(std::ostream& os) const
			{
				os << m_Data[0];
				for (int i=1; i<size(); ++i) os << " " << m_Data[i];
			}

			const std::vector<Type>& get() const
			{
				return m_Data;
			}
			
			const Type& get(int i) const
			{
				return m_Data[i];
			}

			virtual std::string getString(int i) const
			{
				return boost::lexical_cast<std::string>(m_Data[i]);
			}

		protected:
			std::vector<Type> m_Data;

			DataElement(int num)
			{
				m_Data.reserve(num);
			}

			template <class T>
			typename boost::enable_if<boost::is_same<T,std::string>, const std::string>::type getSpecializedTypeName() const
			{
				return "string";
			}
			template <class T>
			typename boost::disable_if<boost::is_same<T,std::string>, const std::string>::type getSpecializedTypeName() const
			{
				return typeid(T).name();
			}
		};


		template <class Type, bool CaseSensitive>
		class EnumDataElement: public DataElement<Type>
		{
		public:
			typedef EnumDataElement Self;
			typedef DataElement<Type> Parent;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<Self const> ConstantPointer;
			typedef boost::shared_ptr<const std::vector<Type>> EnumTableType;
			typedef Type ValueType;
			enum { Enum = true };
			enum { UseCaseInsensitiveCompare = boost::is_same<ValueType,std::string>::value && !CaseSensitive };

			static Pointer New(const EnumTableType& values, int num)
			{
				return Pointer(new Self(num, values));
			}

			virtual void add(const std::string& arg)
			{
				if (this->m_Data.size()==this->m_Data.capacity()) pcl_ThrowException(pcl::Exception(), "Max data size exceeed!");
				auto val = boost::lexical_cast<Type>(arg);
				if (compare<Self>(val)) this->m_Data.push_back(val);
				else throw boost::bad_lexical_cast();
			}

		protected:
			EnumTableType m_EnumValues;

			EnumDataElement(int num, const EnumTableType& values):DataElement<Type>(num), m_EnumValues(values) {}

			template <class S, class T>
			typename boost::disable_if_c<S::UseCaseInsensitiveCompare, bool>::type compare(const T& val) const
			{
				pcl_ForEach(*m_EnumValues, item) {
					if (val==*item) {
						return true;
						break;
					}
				}
				return false;
			}

			template <class S, class T>
			typename boost::enable_if_c<S::UseCaseInsensitiveCompare, bool>::type compare(const T& val) const
			{
				pcl_ForEach(*m_EnumValues, item) {
					if (boost::iequals(val, *item)) {
						return true;
						break;
					}
				}
				return false;
			}
		};
		/***************************************************************************************************/

		class ElementCreatorBase
		{
		public:
			typedef ElementCreatorBase Self;
			typedef std::unique_ptr<Self> Pointer;

			virtual DataElementBase::Pointer create() const = 0;
			virtual int size() const = 0;

		protected:
			ElementCreatorBase() {}
		};


		template <class ElementType, class Enable=void>
		class ElementCreator: public ElementCreatorBase
		{
		public:
			typedef ElementCreator Self;
			typedef std::unique_ptr<Self> Pointer;

			static Pointer New(int num)
			{
				return Pointer(new Self(num));
			}

			virtual DataElementBase::Pointer create() const
			{
				return ElementType::New(m_Num);
			}

			virtual int size() const
			{
				return m_Num;
			}

		protected:
			int m_Num;

			ElementCreator(int num):m_Num(num) {}
		};


		template <class ElementType>
		class ElementCreator<ElementType, typename boost::enable_if_c<ElementType::Enum>::type>: public ElementCreatorBase
		{
		public:
			typedef ElementCreator Self;
			typedef std::unique_ptr<Self> Pointer;
			typedef typename ElementType::EnumTableType EnumTableType;

			static Pointer New(const EnumTableType& enum_val, int num)
			{
				return Pointer(new Self(enum_val, num));
			}

			virtual DataElementBase::Pointer create() const
			{
				return ElementType::New(m_EnumValues, m_Num);
			}

			virtual int size() const
			{
				return m_Num;
			}

		protected:
			EnumTableType m_EnumValues;
			int m_Num;

			ElementCreator(const EnumTableType& enum_val, int num):m_EnumValues(enum_val), m_Num(num) {}
		};

		/***************************************************************************************************/

		class OptionBase: private boost::noncopyable
		{
		public:
			typedef OptionBase Self;
			typedef boost::shared_ptr<Self> Pointer;
			typedef boost::shared_ptr<const Self> ConstantPointer;

			virtual DataElementBase::Pointer createElement() = 0;
			virtual void addElement(const DataElementBase::Pointer& element) = 0;

			virtual int elementNum() const = 0;

			virtual bool isSwitch() const = 0;

			template <class Type>
			typename DataElement<Type>::ConstantPointer getElement(int i) const
			{
				typename DataElement<Type>::ConstantPointer res =  boost::dynamic_pointer_cast<const DataElement<Type>>(getElement(i));
				if (res) return res;
				pcl_ThrowException(pcl::Exception(), std::string("Cannot cast data element to type \"")+typeid(Type).name()+"\"");
			}
			virtual DataElementBase::ConstantPointer getElement(int i) const = 0;
			
			template <class Type>
			const std::vector<Type>& getElementData(int element=0) const
			{
				return getElement<Type>(element)->get();
			}

			template <class Type>
			const Type& getElementDatum(int element=0, int datum=0) const
			{
				return getElement<Type>(element)->get(datum);
			}
			const std::string getElementDatum(int element=0, int datum=0) const
			{
				return getElement(element)->getString(datum);
			}

			bool optional() const
			{
				return m_Optional;
			}

			bool declared() const
			{
				return m_Declared;
			}
			
			bool singular() const
			{
				return m_Singular;
			}

			void setDeclared()
			{
				m_Declared = true;
			}

			virtual bool help() const
			{
				return false;
			}

			const std::string& syntax() const
			{
				return m_Syntax;
			}

			const std::string& description() const
			{
				return m_Description;
			}

		protected:
			std::string m_Syntax;
			std::string m_Description;
			bool m_Optional;
			bool m_Declared;
			bool m_Singular;

			OptionBase(const std::string& syntax, const std::string& description, bool optional, bool singular): 
			m_Syntax(syntax), m_Description(description), m_Optional(optional), m_Singular(singular), m_Declared(false) {}
		};


		class DataOption: public OptionBase
		{
		public:
			typedef DataOption Self;
			typedef boost::shared_ptr<Self> Pointer;

			template <class CreatorPointer>
			static Pointer New(CreatorPointer&& ptr, std::string syntax, const std::string& description, bool optional, bool singular)
			{
				Pointer obj(new Self(syntax, description, optional, singular));
				obj->m_ElementCreator = std::move(ptr);
				return obj;
			}

			virtual DataElementBase::Pointer createElement()
			{
				return m_ElementCreator->create();
			}

			virtual void addElement(const DataElementBase::Pointer& element)
			{
				m_Element.push_back(element);
			}

			using OptionBase::getElement;
			virtual DataElementBase::ConstantPointer getElement(int i) const
			{
				return m_Element[i];
			}

			virtual int elementNum() const
			{
				return static_cast<int>(m_Element.size());
			}

			virtual bool isSwitch() const
			{
				return false;
			}
		protected:
			ElementCreatorBase::Pointer m_ElementCreator;
			std::vector<DataElementBase::Pointer> m_Element;

			DataOption(const std::string& syntax, const std::string& description, bool optional, bool singular): OptionBase(syntax, description, optional, singular) {}
		};


		class ArgumentOption: public DataOption
		{
		public:
			typedef ArgumentOption Self;
			typedef boost::shared_ptr<Self> Pointer;

			template <class CreatorPointer>
			static Pointer New(CreatorPointer&& ptr, std::string syntax, const std::string& description, bool optional, bool singular)
			{
				Pointer obj(new Self(syntax, description, optional, singular));
				if (ptr->size()>1) pcl_ThrowException(pcl::Exception(), "Template element must be of size 1!");
				obj->m_ElementCreator = std::move(ptr);
				return obj;
			}

			virtual void addElement(const DataElementBase::Pointer& element)
			{
				if (!m_Element.empty() && singular()) pcl_ThrowException(pcl::Exception(), "More than one element added!");
				m_Element.push_back(element);
			}

		protected:
			ArgumentOption(const std::string& syntax, const std::string& description, bool optional, bool singular): DataOption(syntax, description, optional, singular) {}
		};


		class SwitchOption: public OptionBase
		{
		public:
			typedef SwitchOption Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(std::string syntax, const std::string& description, bool optional)
			{
				return Pointer(new Self(syntax, description, optional));
			}

			virtual DataElementBase::Pointer createElement()
			{
				pcl_ThrowException(pcl::Exception(), "Invalid function call!");
			}

			virtual void addElement(const DataElementBase::Pointer& element)
			{
				pcl_ThrowException(pcl::Exception(), "Invalid function call!");
			}

			using OptionBase::getElement;
			virtual DataElementBase::ConstantPointer getElement(int i) const
			{
				pcl_ThrowException(pcl::Exception(), "Invalid function call!");
			}

			virtual int elementNum() const
			{
				return 0;
			}

			virtual bool isSwitch() const
			{
				return true;
			}
		protected:
			SwitchOption(const std::string& syntax, const std::string& description, bool optional): OptionBase(syntax, description, optional, false) {}
		};


		class HelpOption: public SwitchOption
		{
		public:
			typedef HelpOption Self;
			typedef boost::shared_ptr<Self> Pointer;

			static Pointer New(const std::string& syntax)
			{
				return Pointer(new Self(syntax));
			}

			virtual bool help() const
			{
				return true;
			}

		protected:
			HelpOption(const std::string& syntax): SwitchOption(syntax, "Display this help and exit", true) {}
		};

		/***************************************************************************************************/

		struct LookupKey
		{
			const std::string* pointer;

			LookupKey():pointer(NULL) {}
			LookupKey(const std::string* ptr): pointer(ptr) {}
			LookupKey(const LookupKey& obj): pointer(obj.pointer) {}
			template <class SmartPointerType>
			LookupKey(const SmartPointerType& ptr, typename boost::disable_if<boost::is_pointer<SmartPointerType>>::type* dummy=0): pointer(ptr.get()) {}

			const std::string& deref() const
			{
				return *pointer;
			}

			operator bool() const
			{
				return pointer!=NULL;
			}

			bool operator==(const LookupKey& obj) const
			{
				if (pointer==obj.pointer) return true;
				return *pointer == *(obj.pointer);
			}
		};

		struct LookupKeyHasher
		{
			std::size_t operator()(const LookupKey& string_ptr) const
			{
				boost::hash<std::string> hasher;
				return hasher(*(string_ptr.pointer));
			}
		};

	}


	using namespace command_line_details;

	struct CommandLineException: public pcl::Exception {};
	struct CommandLineHelpException: public CommandLineException 
	{
		virtual std::string getMessageHeader() const
		{
			return "";
		}
	};
	struct CommandLineErrorException: public CommandLineException 
	{
		virtual std::string getMessageHeader() const
		{
			return "Input argument error: ";
		}
	};


	class CommandLineParser {
	public:
		CommandLineParser(int argc, char *argv[]) {
			m_Argc = argc;
			m_Argv = argv;
		}

		static std::string GetHeader(const std::string& str)
		{
			return "== "+str+" ==\n\n";
		}

		void setOverallDescription(const std::string& description)
		{
			m_OverallDescription = description;
		}

		void setExtraDescription(const std::string& description)
		{
			m_ExtraDescription = description;
		}

		/************ Argument addition ************/

		template <class Type>
		void addArgument(const std::string& identifier, const std::string& syntax, const std::string& description, bool optional=false, bool singular=true)
		{
			auto lookup_key = addKey(identifier);
			auto creator = ElementCreator<DataElement<Type>>::New(1);
			m_ArgumentMap[lookup_key] = ArgumentOption::New(std::move(creator), syntax, description, optional, singular);
			m_ArgumentSequence.push_back(lookup_key);
		}
		template <class Type>
		void addArgument(const Type* ptr, int ptr_num, const std::string& identifier, const std::string& syntax, const std::string& description, bool optional=false, bool singular=true)
		{
			addArgument<Type>(boost::shared_ptr<std::vector<Type>>(new std::vector<Type>(ptr, ptr+ptr_num)), identifier, syntax, description, optional, singular);
		}
		template <class Type, bool CaseSensitive>
		void addArgument(const Type* ptr, int ptr_num, const std::string& identifier, const std::string& syntax, const std::string& description, bool optional=false, bool singular=true)
		{
			addArgument<Type, CaseSensitive>(boost::shared_ptr<std::vector<Type>>(new std::vector<Type>(ptr, ptr+ptr_num)), identifier, syntax, description, optional, singular);
		}
		template <class Type>
		void addArgument(boost::shared_ptr<std::vector<Type>>& enum_values, const std::string& identifier, const std::string& syntax, const std::string& description, bool optional=false, bool singular=true)
		{
			addArgument<Type,false>(enum_values, identifier, syntax, description, optional, singular);
		}
		template <class Type, bool CaseSensitive>
		void addArgument(boost::shared_ptr<std::vector<Type>>& enum_values, const std::string& identifier, const std::string& syntax, const std::string& description, bool optional=false, bool singular=true)
		{
			auto lookup_key = addKey(identifier);
			auto creator = ElementCreator<EnumDataElement<Type, CaseSensitive>>::New(enum_values, 1);
			std::string extra_description = "\nValid values";
			if (CaseSensitive) extra_description += " (case sensitive)";
			extra_description += ": ";
			extra_description += boost::lexical_cast<std::string>((*enum_values)[0]);
			for (int i=1; i<enum_values->size(); i++) extra_description += ", " + boost::lexical_cast<std::string>((*enum_values)[i]);
			m_ArgumentMap[lookup_key] = ArgumentOption::New(std::move(creator), syntax, description+extra_description, optional, singular);
			m_ArgumentSequence.push_back(lookup_key);
		}

		/************ Option addition ************/

		void addOption(const std::string& key, const std::string& description, bool optional=true)
		{
			auto lookup_key = addKey(key);
			m_OptionMap[lookup_key] = SwitchOption::New(key, description, optional);
			m_OptionSequence.push_back(lookup_key);
		}

		template <class Type>
		void addOption(int num, const std::string& key, const std::string& syntax, const std::string& description, bool optional=true, bool singular=true)
		{
			auto lookup_key = addKey(key);
			auto creator = ElementCreator<DataElement<Type>>::New(num);
			m_OptionMap[lookup_key] = DataOption::New(std::move(creator), syntax, description, optional, singular);
			m_OptionSequence.push_back(lookup_key);
		}
		template <class Type>
		void addOption(const Type* ptr, int ptr_num, int num, const std::string& key, const std::string& syntax, const std::string& description, bool optional=true, bool singular=true)
		{
			addOption<Type>(boost::shared_ptr<std::vector<Type>>(new std::vector<Type>(ptr, ptr+ptr_num)), num, key, syntax, description, optional, singular);
		}
		template <class Type, bool CaseSensitive>
		void addOption(const Type* ptr, int ptr_num, int num, const std::string& key, const std::string& syntax, const std::string& description, bool optional=true, bool singular=true)
		{
			addOption<Type, CaseSensitive>(boost::shared_ptr<std::vector<Type>>(new std::vector<Type>(ptr, ptr+ptr_num)), num, key, syntax, description, optional, singular);
		}
		template <class Type>
		void addOption(const boost::shared_ptr<std::vector<Type>>& enum_values, int num, const std::string& key, const std::string& syntax, const std::string& description, bool optional=true, bool singular=true)
		{
			addOption<Type,false>(enum_values, num, key, syntax, description, optional, singular);
		}
		template <class Type, bool CaseSensitive>
		void addOption(const boost::shared_ptr<std::vector<Type>>& enum_values, int num, const std::string& key, const std::string& syntax, const std::string& description, bool optional=true, bool singular=true)
		{
			auto lookup_key = addKey(key);
			auto creator = ElementCreator<EnumDataElement<Type, CaseSensitive>>::New(enum_values, num);
			std::string extra_description = "\nValid values";
			if (CaseSensitive) extra_description += " (case sensitive)";
			extra_description += ": ";
			extra_description += boost::lexical_cast<std::string>((*enum_values)[0]);
			for (int i=1; i<enum_values->size(); i++) extra_description += ", " + boost::lexical_cast<std::string>((*enum_values)[i]);
			m_OptionMap[lookup_key] = DataOption::New(std::move(creator), syntax, description+extra_description, optional, singular);
			m_OptionSequence.push_back(lookup_key);
		}

		void addHelpOption(const std::string& key)
		{
			auto lookup_key = addKey(key);
			m_OptionMap[lookup_key] = HelpOption::New(key);
			m_OptionSequence.push_back(lookup_key);
		}

		/************ Misc ************/

		void update()
		{
			checkArgument();
			resetArgument();

			int argv_count = 1;
			auto end_iter = m_OptionMap.end();
			while (argv_count<m_Argc) {
				std::string argv(m_Argv[argv_count]);
				auto iter = m_OptionMap.find(LookupKey(&argv));
				argv_count++;
				if (iter==end_iter) {
					addToArgument(argv);
				} else {
					auto& option = iter->second;
					option->setDeclared();
					if (option->help()) throwHelp();
					else if (!option->isSwitch()) {
						auto element = option->createElement();
						int num = element->size();
						for (int i=0; i<num; ++i) {
							if (argv_count>=m_Argc) pcl_ThrowSimpleException(CommandLineErrorException(), "Arguments terminated unexpectedly at option "+iter->first.deref()+getHelpDirections());
							std::string cur_argv(m_Argv[argv_count]);
							try {
								element->add(cur_argv);
							} catch (const boost::bad_lexical_cast& e) {
								pcl_ThrowSimpleException(CommandLineErrorException(), "Invalid input \""+std::string(cur_argv)+"\" encountered at option "+iter->first.deref()+getHelpDirections());
							}
							argv_count++;
						}
						if (option->elementNum()==1 && option->singular()) {
							std::cout << "Warning: Redundant entry \"" << iter->first.deref() << " ";
							element->print(std::cout);
							std::cout << "\" ignored!" << std::endl;
						} else option->addElement(element);
					}
				}
			}

			checkCompletion();
		}

		OptionBase::ConstantPointer getArgument(const std::string& identifier) const
		{
			auto iter = m_ArgumentMap.find(LookupKey(&identifier));
			if (iter==m_ArgumentMap.end()) pcl_ThrowException(pcl::Exception(), "Cannot find \""+identifier+"\" in argument list");
			return iter->second;
		}

		OptionBase::ConstantPointer getOption(const std::string& key) const
		{
			auto iter = m_OptionMap.find(LookupKey(&key));
			if (iter==m_OptionMap.end()) pcl_ThrowException(pcl::Exception(), "Cannot find \""+key+"\" in option list");
			return iter->second;
		}

		OptionBase::ConstantPointer get(const std::string& indentifier_key) const
		{
			try {
				auto iter = findItem(indentifier_key);
				return iter->second;
			} catch (int) {
				pcl_ThrowException(pcl::Exception(), "Cannot find \""+indentifier_key+"\" in both argument and option list");
			}
		}

		void printHelpMenu(std::ostream& os) const
		{
			os << "== Usage ==" << std::endl << std::endl;; 
			os << m_Argv[0] << " ";
			pcl_ForEach(m_ArgumentSequence, item) {
				auto option = m_ArgumentMap.find(*item)->second;
				if (option->optional()) os << "[" << option->syntax() << "] ";
				else os << option->syntax() << " ";
			}
			bool has_optional = false;
			pcl_ForEach(m_OptionSequence, item) {
				auto option = m_OptionMap.find(*item)->second;
				if (option->optional()) has_optional = true;
				else os << option->syntax() << " ";
			}
			if (has_optional) os << "[OPTION]";
			os << std::endl;
			os << std::endl;

			os << "== Description ==" << std::endl;
			if (!m_OverallDescription.empty()) os << std::endl << m_OverallDescription << std::endl;
			pcl_ForEach(m_ArgumentSequence, item) {
				auto option = m_ArgumentMap.find(*item)->second;
				os << std::endl;
				os << option->syntax() << std::endl;
				os << option->description() << std::endl;
			}
			pcl_ForEach(m_OptionSequence, item) {
				auto option = m_OptionMap.find(*item)->second;
				os << std::endl;
				os << option->syntax() << std::endl;
				os << option->description() << std::endl;
			}
			if (!m_ExtraDescription.empty()) os << std::endl << m_ExtraDescription << std::endl;
		}

		void print(std::ostream& os) const
		{
			pcl_ForEach(m_ArgumentSequence, item) {
				auto option = m_ArgumentMap.find(*item)->second;
				if (option->declared()) {
					os << item->deref() << "  <" << option->getElement(0)->getTypeName() << ">" << std::endl;
					for (int i=0; i<option->elementNum(); ++i) {
						os << "  ";
						option->getElement(i)->print(os);
						os << std::endl;
					}
					os << std::endl;
				}
			}
			pcl_ForEach(m_OptionSequence, item) {
				auto option = m_OptionMap.find(*item)->second;
				if (option->declared()) {
					os << item->deref();
					if (!option->isSwitch()) {
						os << "  <" << option->getElement(0)->getTypeName() << ">" << std::endl;
						for (int i=0; i<option->elementNum(); ++i) {
							os << "  ";
							option->getElement(i)->print(os);
							os << std::endl;
						}
					} else os << "  <Switch>";
					os << std::endl;
				}
			}
			if (m_UnusedArgument && m_UnusedArgument->elementNum()>0) {
				std::cout << "Unused arguments:";
				for (int i=0; i<m_UnusedArgument->elementNum(); ++i) {
					std::cout << " ";
					m_UnusedArgument->getElement(i)->print(os);
				}
				std::cout << std::endl;
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const CommandLineParser& parser)
		{
			parser.print(os);
			return os;
		}

	protected:
		int m_Argc;
		char **m_Argv;
		std::vector<std::unique_ptr<const std::string>> m_KeyStorage;
		std::string m_OverallDescription, m_ExtraDescription;
		int m_MinBaseArgumentNum, m_MaxBaseArgumentNum;
		boost::unordered_map<LookupKey, OptionBase::Pointer, LookupKeyHasher> m_ArgumentMap;
		std::list<LookupKey> m_ArgumentSequence;
		ArgumentOption::Pointer m_UnusedArgument;
		boost::unordered_map<LookupKey, OptionBase::Pointer, LookupKeyHasher> m_OptionMap;
		std::list<LookupKey> m_OptionSequence;

		std::list<LookupKey>::iterator m_ArgumentIter, m_ArgumentEndIter;
		OptionBase::Pointer m_CurrentArgument;

		boost::unordered_map<LookupKey, OptionBase::Pointer, LookupKeyHasher>::const_iterator findItem(const std::string& identifier_key) const
		{
			LookupKey lookup_key(&identifier_key);
			auto opt_iter = m_OptionMap.find(lookup_key);
			if (opt_iter!=m_OptionMap.end()) return opt_iter;
			auto arg_iter = m_ArgumentMap.find(lookup_key);
			if (arg_iter!=m_ArgumentMap.end()) return arg_iter;
			throw 1;
		}

		LookupKey addKey(const std::string& key)
		{
			std::unique_ptr<const std::string> ptr(new std::string(key));
			m_KeyStorage.push_back(std::move(ptr));
			return LookupKey(m_KeyStorage.back());
		}

		void checkArgument()
		{
			auto last_argument = m_ArgumentMap[m_ArgumentSequence.back()];
			pcl_ForEach(m_ArgumentSequence, item) {
				auto cur_argument = m_ArgumentMap[*item];
				if (cur_argument!=last_argument && !cur_argument->singular()) {
#ifndef NO_WARNING
					std::cout << "Warning: Found nonsingular argument " << item->deref() << " in the middle of the argument list!" << std::endl;
#endif
				}
			}
			if (last_argument->singular()) {
				auto creator = ElementCreator<DataElement<std::string>>::New(1);
				m_UnusedArgument = ArgumentOption::New(std::move(creator), "","",true,false);
			}
		}

		void resetArgument()
		{
			if (m_ArgumentSequence.empty()) {
				m_CurrentArgument = m_UnusedArgument;
				return;
			}
			m_ArgumentEndIter = m_ArgumentSequence.end();
			m_ArgumentIter = m_ArgumentSequence.begin();
			m_CurrentArgument = m_ArgumentMap[*m_ArgumentIter];
		}

		void addToArgument(const std::string& arg)
		{
			auto element = m_CurrentArgument->createElement();
			try {
				element->add(arg);
			} catch (const boost::bad_lexical_cast& e) {
				pcl_ThrowSimpleException(CommandLineErrorException(), "Invalid input \""+std::string(arg)+"\" used for "+m_CurrentArgument->syntax()+getHelpDirections());
			}
			m_CurrentArgument->addElement(element);
			m_CurrentArgument->setDeclared();
			if (m_CurrentArgument->singular()) {
				++m_ArgumentIter;
				if (m_ArgumentIter==m_ArgumentEndIter) {
					m_CurrentArgument = m_UnusedArgument;
				} else {
					m_CurrentArgument = m_ArgumentMap[*m_ArgumentIter];
				}
			}
		}

		void throwHelp()
		{
			std::stringstream msg;
			printHelpMenu(msg);
			pcl_ThrowSimpleException(CommandLineHelpException(), msg.str());
		}

		std::string getHelpDirections()
		{
			std::string help_option_str;
			int help_count = 0;
			pcl_ForEach(m_OptionMap, item) {
				if (item->second->help()) {
					if (help_count==0) {
						help_option_str += item->first.deref();
						++help_count;
					} else {
						help_option_str += " or ";
						help_option_str += item->first.deref();
						++help_count;
					}
				}
			}
			std::stringstream ss;
			ss << std::endl;
			if (help_count>0) {
				ss << "For help, use ";
				if (help_count>1) ss << "either ";
				ss << help_option_str << "." << std::endl;
			} else {
				printHelpMenu(ss);
			}
			return std::move(ss.str());
		}

		void checkCompletion()
		{
			bool error = false;
			std::stringstream ss;
			ss << "Following required inputs are missing:" << std::endl;
			pcl_ForEach(m_ArgumentSequence, item) {
				auto option = m_ArgumentMap[*item];
				if (!option->optional() && !option->declared()) {
					ss << "  " << option->syntax() << std::endl;
					error = true;
				}
			}
			pcl_ForEach(m_OptionSequence, item) {
				auto option = m_OptionMap[*item];
				if (!option->optional() && !option->declared()) {
					ss << "  " << option->syntax() << std::endl;
					error = true;
				}
			}
			if (error) {
				pcl_ThrowSimpleException(CommandLineErrorException(), ss.str()+getHelpDirections());
			}
			if (m_UnusedArgument && m_UnusedArgument->declared()) {
				std::cout << "Warning: Following unused arguments were found" << std::endl;
				for (int i=0; i<m_UnusedArgument->elementNum(); ++i) {
					std::cout << "  ";
					m_UnusedArgument->getElement(i)->print(std::cout);
					std::cout << std::endl;
				}
			}
		}
	};

}

#pragma warning ( pop )
#endif