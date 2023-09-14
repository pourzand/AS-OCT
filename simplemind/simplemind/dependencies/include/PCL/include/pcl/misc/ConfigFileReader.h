#ifndef PCL_CONFIG_FILE_READER
#define PCL_CONFIG_FILE_READER

#include <pcl/exception.h>
#include <pcl/misc/FileNameTokenizer.h>
#include <pcl/misc/StringTokenizer.h>
#include <pcl/macro.h>
#include <boost/smart_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <fstream>
#include <ostream>
#include <vector>

namespace pcl
{
	namespace misc
	{
	
		/*
			A class for extracting information (in the form of strings) from config files.

			The format for a field is in the form:
				NAME = VALUE
			The whole line must start with NAME and end with a new line. Leading and trailing spaces of VALUE is removed by default.
			Note that the only symbol allowed in NAME is underscore.
			
			Below is the format to substitute the value of a field with value from a source field:
				$(NAME)
			where NAME is the name of the source field. Below is an example:
				Name1 = Hello
				Name2 = $(Name1) World
			The field "Name2" will contain the value "Hello World".
			
			Comment style is similar with that of C++, with the exception that comment symbol must be declared at the beginning of a line.
			The ending of a block comment is also only valid if is the last two character in a line.

			Additionally, comments can also be done by starting the line with a # character.
			
			There are three commands, which is indicated by starting a line with @:
			@include_abs FILENAME
				Include a config file via absolute path.
			
			@include_rel FILENAME
				Include a config file via a path relative to current config file.
				
			@group GROUP_NAME {
				NAME1 = VALUE1
				NAME2 = VALUE2
				...
			}
				Indicates group of fields. Note that the enclosing bracket must be in a line by itself.
				The individual field in the group are referrable as GROUP_NAME.FIELD_NAME.
				For example, if GROUP_NAME is "Test", and NAME1 is "ElementBase", then the field of NAME1 is referable with "Test.ElementBase".
				Note that groups are not allowed to span across files.

			By default, given substituion via "$()" will be done within a group. In the case where the field cannot be found, the search will move to the upper group.
			To indicate search from the root, prefix the name with ":".
		 */
		 
		 namespace config_reader_details
		 {
			class ElementBase
			{
			public:
				typedef ElementBase Self;
				typedef boost::shared_ptr<Self> Pointer;
				typedef boost::shared_ptr<const Self> ConstantPointer;
			
				const std::string& getName() const
				{
					return m_Name;
				}

				std::string getFullName() const
				{
					if (m_Group) return m_Group->getFullName()+"."+m_Name;
					return m_Name;
				}

				const ConstantPointer& getGroup() const
				{
					return m_Group;
				}

				virtual bool isGroup() const = 0;
				virtual void print(std::ostream& os, bool use_file_format, int indent) const = 0;
			protected:
				std::string m_Name;
				ConstantPointer m_Group;

				ElementBase(const std::string& name, const ConstantPointer& group): m_Name(name), m_Group(group) {}
			};


			class Field: public ElementBase
			{
			public:
				typedef Field Self;
				typedef boost::shared_ptr<Self> Pointer;
				typedef boost::shared_ptr<const Self> ConstantPointer;
				
				template <class GroupPointer>
				static Pointer New(const std::string& name, const GroupPointer& group)
				{
					Pointer obj(new Self(name, group));
					if (group) group->addMember(obj);
					return obj;
				}
				
				const std::string& getValue() const
				{
					return m_Value;
				}
				
				void setValue(const std::string& value)
				{
					m_Value = value;
				}

				virtual bool isGroup() const
				{
					return false;
				}
				
				virtual void print(std::ostream& os, bool use_file_format, int indent) const
				{
					if (use_file_format) {
						for (int i=0; i<indent; ++i) os << "\t";
						os << getName() << " = " << m_Value << std::endl;
					} else os << getFullName() << " = " << m_Value << std::endl;
				}
			protected:
				std::string m_Value;

				Field(const std::string& name, const ElementBase::ConstantPointer& group): ElementBase(name, group) {}
			};
			
			
			class Group: public ElementBase
			{
			public:
				typedef Group Self;
				typedef boost::shared_ptr<Self> Pointer;
				typedef boost::shared_ptr<const Self> ConstantPointer;
				
				template <class GroupPointer>
				static Pointer New(const std::string& name, const GroupPointer& group)
				{
					Pointer obj(new Self(name, group));
					if (group) group->addMember(obj);
					return obj;
				}
				
				int size() const
				{
					return static_cast<int>(m_Member.size());
				}
				
				const ElementBase::ConstantPointer& getMember(int i) const
				{
					return m_Member[i];
				}
				const std::vector<ElementBase::ConstantPointer>& getMember() const
				{
					return m_Member;
				}
				
				const ElementBase::ConstantPointer findMemberWithName(const std::string& name) const
				{
					size_t pos = name.find('.');
					if (pos==std::string::npos) { //Searching locally
						return findLocalMemberWithName(name);
					}
					//Searching subgroups
					std::string new_name = name.substr(pos+1);
					pcl_ForEach(m_Member, item) if ((*item)->isGroup()) {
						Group::ConstantPointer group = boost::static_pointer_cast<const Group>(*item);
						ElementBase::ConstantPointer member = group->findMemberWithName(new_name);
						if (member) return member;
					}
					return ElementBase::ConstantPointer();
				}

				void addMember(const ElementBase::ConstantPointer& member)
				{
					m_Member.push_back(member);
				}

				virtual bool isGroup() const
				{
					return true;
				}

				virtual void print(std::ostream& os, bool use_file_format, int indent) const
				{
					if (use_file_format) {
						for (int i=0; i<indent; ++i) os << "\t";
						os << "@group " << getName() << std::endl;
						for (int i=0; i<indent; ++i) os << "\t";
						os << "{" << std::endl;
					}
					pcl_ForEach(m_Member, item) {
						(*item)->print(os, use_file_format, indent+1);
					}
					if (use_file_format) {
						for (int i=0; i<indent; ++i) os << "\t";
						os << "}" << std::endl;
					}
				}
			protected:
				std::vector<ElementBase::ConstantPointer> m_Member;

				Group(const std::string& name, const ElementBase::ConstantPointer& group): ElementBase(name, group) {}

				const ElementBase::ConstantPointer findLocalMemberWithName(const std::string& name) const
				{
					pcl_ForEach(m_Member, item) if ((*item)->getName()==name) {
						return *item;
					}
					return ElementBase::ConstantPointer();
				}
			};
		 }
		 
		 using namespace pcl::misc::config_reader_details;

		 typedef boost::error_info<struct config_file_, std::string> throw_config_file;
		 typedef boost::error_info<struct config_file_line_, int> throw_config_file_line;
		 
		 struct ConfigFileError: public pcl::Exception 
		 {
			virtual std::string getMessageHeader() const
			{
				return "Config file error: ";
			}

			virtual std::ostream& print(std::ostream& os) const
			{
				if ( auto msg = boost::get_error_info<pcl::exception_details::throw_message>(*this) ) {
					std::cout << this->getMessageHeader() << *msg << std::endl;
				}
				if ( auto msg = boost::get_error_info<throw_config_file_line>(*this) ) {
					std::cout << "At line: " << *msg << std::endl;
				}
				if ( auto msg = boost::get_error_info<throw_config_file>(*this) ) {
					std::cout << "In file: " << *msg << std::endl;
				}
				return os;
			}
		 };

#define pcl_ThrowConfigFileError(msg, line, file) boost::throw_exception( ::boost::enable_error_info(ConfigFileError()) << \
	::pcl::exception_details::throw_message(msg) << \
	throw_config_file_line(line) << \
	throw_config_file(file))

		 class ConfigFileReader
		 {
		 public:
			ConfigFileReader() {}
			ConfigFileReader(const std::string& file) 
			{
				read(file);
			}
			
			void read(const std::string& file)
			{
				m_Sequence.clear();
				m_File.clear();
				m_Lookup.clear();
				parseFile(file, Group::Pointer());
			}

			bool empty() const
			{
				return m_Lookup.empty();
			}
			
			bool exist(const std::string& name) const
			{
				return m_Lookup.find(name)!=m_Lookup.end();
			}

			ElementBase::ConstantPointer get(const std::string& name, bool no_throw=false) const
			{
				auto iter = m_Lookup.find(name);
				if (iter==m_Lookup.end()) {
					if (no_throw) return ElementBase::Pointer();
					else pcl_ThrowException(pcl::Exception(), "Cannot find field or group with name \""+name+"\"");
				}
				return iter->second;
			}
			
			const std::string& getFieldValue(const std::string& name) const
			{
				auto iter = m_Lookup.find(name);
				if (iter==m_Lookup.end() || iter->second->isGroup()) pcl_ThrowException(pcl::Exception(), "Cannot find field with name \""+name+"\"");
				return boost::static_pointer_cast<const Field>(iter->second)->getValue();
			}
			
			const std::vector<std::string>& getLoadedFiles() const
			{
				return m_File;
			}
			
			std::ostream& printFormated(std::ostream& os) const
			{
				pcl_ForEach(m_Sequence, item) {
					(*item)->print(os, true, 0);
				}
				return os;
			}
			
			friend std::ostream& operator<<(std::ostream& os, const ConfigFileReader& obj)
			{
				pcl_ForEach(obj.m_Sequence, item) {
					(*item)->print(os, false, 0);
				}
				return os;
			}

		 protected:
			std::vector<std::string> m_File;
			std::vector<ElementBase::ConstantPointer> m_Sequence;
			boost::unordered_map<std::string, ElementBase::Pointer> m_Lookup;

			void addFile(const std::string& file)
			{
				pcl_ForEach(m_File, item) if (file==*item) {
					throw 1;
				}
				m_File.push_back(file);
			}
			
			void addElementBaseToCollection(const ElementBase::Pointer& field)
			{
				if (!field->getGroup()) m_Sequence.push_back(field);
				m_Lookup[field->getFullName()] = field;
			}
			
			int getStringStartPos(const std::string& str)
			{
				int start_pos = -1;
				for (int i=0; i<str.size(); ++i) if (!std::isspace(str[i])) {
					start_pos = i;
					break;
				}
				return start_pos;
			}
			
			void testName(const std::string& str, const std::string& filename, int line)
			{
				for (int i=0; i<str.size(); ++i) if (!std::isalnum(str[i]) && str[i]!='_') {
					pcl_ThrowConfigFileError("Invalid name \""+str+"\" found", line, filename);
				}
			}
			
			Group::Pointer createGroupIfNeeded(const std::string& name, const Group::Pointer& group, const std::string& filename, int line)
			{
				std::string fullname;
				if (group) fullname = group->getFullName()+"."+name;
				else fullname = name;
				Group::Pointer cur_group;
				auto iter = m_Lookup.find(fullname);
				if (iter!=m_Lookup.end()) {
					if (iter->second->isGroup()) cur_group = boost::static_pointer_cast<Group>(iter->second);
					else pcl_ThrowConfigFileError("Group name \""+iter->second->getFullName()+"\" is already being used by a field", line, filename);
				} else {
					cur_group = Group::New(name, group);
					addElementBaseToCollection(cur_group);
				}
				return cur_group;
			}
			
			Field::Pointer createFieldIfNeeded(const std::string& name, const Group::Pointer& group, const std::string& filename, int line)
			{
				std::string fullname;
				if (group) fullname = group->getFullName()+"."+name;
				else fullname = name;
				Field::Pointer cur_field;
				auto iter = m_Lookup.find(fullname);
				if (iter!=m_Lookup.end()) {
					if (!(iter->second->isGroup())) cur_field = boost::static_pointer_cast<Field>(iter->second);
					else pcl_ThrowConfigFileError("Field name \""+cur_field->getFullName()+"\" is already being used by a group", line, filename);
				} else {
					cur_field = Field::New(name, group);
					addElementBaseToCollection(cur_field);
				}
				return cur_field;
			}
			
			int hasCharacter(const char* str, char target, const std::string& filename, int line)
			{
				int i=0;
				while(str[i]!=NULL) {
					if (str[i]==target) return i;
					else if (!std::isspace(str[i])) {
						pcl_ThrowConfigFileError("Invalid token \""+std::string(&str[i])+"\" found", line, filename);
					}
					++i;
				}
				return -1;
			}
			
			std::string advanceUntilCharacter(int& file_line, std::ifstream& is, char target, const std::string& filename)
			{
				while (!is.eof()) {
					std::string buffer;
					++file_line;
					std::getline(is, buffer);
					int start_pos = getStringStartPos(buffer);
					if (start_pos!=-1) {
						if (buffer[start_pos]==target) {
							return buffer.substr(start_pos);
						} else if (buffer[start_pos]=='#');
						else pcl_ThrowConfigFileError("Invalid token \""+std::string(&buffer[start_pos])+"\" found", file_line, filename);
					}
				}
				return std::string();
			}

			std::string ascendingSearch(const std::string& name, const Group::ConstantPointer& group)
			{
				if (!group) return getFieldValue(name);
				else if (name[0]==':') return getFieldValue(name.substr(1)); 
				else {
					ElementBase::ConstantPointer field = group->findMemberWithName(name);
					if (field && field->isGroup()==false) {
						return boost::static_pointer_cast<const Field>(field)->getValue();
					}
					return ascendingSearch(name, boost::static_pointer_cast<const Group>(group->getGroup()));
				}
			}

			std::string parseValue(std::string& value, const Group::ConstantPointer& group, const std::string& filename, int line)
			{
				std::string result;
				size_t pos = 0,
					prev_pos = 0;
				while ((pos = value.find("$(", pos))!=std::string::npos) {
					result.append(value, prev_pos, pos-prev_pos);
					pos += 2;
					size_t end_pos = value.find(")", pos);
					if (end_pos==std::string::npos) pcl_ThrowConfigFileError("Cannot find corresponding \")\" for \"$("+value.substr(pos)+"\"", line, filename);
					std::string name = value.substr(pos, end_pos-pos);
					try {
						result += ascendingSearch(name, group);
					} catch (const pcl::Exception&) {
						pcl_ThrowConfigFileError("Cannot find field $("+name+")", line, filename);
					}
					pos = end_pos+1;
					prev_pos = pos;
				}
				if (result.empty()) return std::move(value);
				else {
					result += value.substr(prev_pos);
					return std::move(result);
				}
			}

			void parseGroup(int& file_line, std::ifstream& is, const Group::Pointer& group, bool test_end, const std::string& filename, int start_line)
			{
				while (!is.eof()) {
					std::string buffer;
					++file_line;
					std::getline(is, buffer);
					int start_pos = getStringStartPos(buffer);
					if (start_pos!=-1) {
						if (buffer[start_pos]=='#'); //Skip if it is a comment
						else if (buffer[start_pos]=='/' && buffer[start_pos+1]=='/'); //Skip if it is a comment
						else if (buffer[start_pos]=='/' && buffer[start_pos+1]=='*') {
							//Searching for end of block
							std::string temp;
							if ((start_pos+2)<buffer.size()) temp = buffer.substr(start_pos+2);
							while (temp.find("*/")==std::string::npos) {
								++file_line;
								std::getline(is, temp);
							}
						} else if (buffer[start_pos]=='@') { //Processing commands
							StringTokenizer tokenizer(&buffer[start_pos+1]);
							tokenizer.begin(boost::is_space());
							if (tokenizer.getToken()=="group") {
								tokenizer.setRemoveDelimiter(false);
								tokenizer.next(boost::is_space()||(!boost::is_alnum() && !boost::is_any_of("_")));
								testName(tokenizer.getToken(), filename, file_line);
								auto cur_group = createGroupIfNeeded(tokenizer.getToken(), group, filename, file_line);
								int group_start_line = file_line;
								//Finding the starting curly bracket
								if (!tokenizer.end() && hasCharacter(tokenizer.getRemainder(), '{', filename, file_line)!=-1) {} 
								else {
									if (advanceUntilCharacter(file_line, is, '{', filename).empty()) 
										pcl_ThrowConfigFileError("Unexpected end at group \""+cur_group->getFullName()+"\"", group_start_line, filename);
								}
								//Parsing group
								parseGroup(file_line, is, cur_group, true, filename, group_start_line);
							} else if (tokenizer.getToken()=="include_abs") {
								std::string param = std::string(tokenizer.getRemainder());
								boost::trim(param);
								try {
									parseFile(param, group);
								} catch (const int&) {
									pcl_ThrowConfigFileError("File \""+param+"\" added more than once", file_line, filename);
								}
							} else if (tokenizer.getToken()=="include_rel") {
								std::string param = std::string(tokenizer.getRemainder());
								boost::trim(param);
								try {
									parseFile(param, filename, group);
								} catch (const int&) {
									pcl_ThrowConfigFileError("File \""+param+"\" added more than once", file_line, filename);
								}
							}
						} else if (buffer[start_pos]=='}') {
							if (group && test_end) return; //End of group reached
							else pcl_ThrowConfigFileError("Invalid token \""+std::string(&buffer[start_pos])+"\" found", file_line, filename);
						} else { //Processing field
							StringTokenizer tokenizer(&buffer[start_pos]);
								tokenizer.begin('=');
							if (!tokenizer.delimiterFound()) pcl_ThrowConfigFileError("Invalid token \""+std::string(&buffer[start_pos])+"\" found", file_line, filename);
							std::string name = boost::trim_copy(tokenizer.getToken());
							testName(name, filename, file_line);
							auto cur_field = createFieldIfNeeded(name, group, filename, file_line);
							std::string value = tokenizer.getRemainder();
							boost::trim(value);
							cur_field->setValue(parseValue(value, group, filename, file_line));
						}
					}
				}
				if (group && test_end) {
					pcl_ThrowConfigFileError("Cannot find matching closing bracket for group \""+group->getFullName()+"\"", start_line, filename);
				}
			}

			void parseFile(const std::string& file, const std::string& source, const Group::Pointer& group)
			{
				FileNameTokenizer fname(source);
				parseFile(fname.getPath()+file, group);
			}
			void parseFile(const std::string& file, const Group::Pointer& group)
			{
				addFile(file);
				std::ifstream file_stream(file.c_str(), std::ifstream::in);
				int file_line = 0;
				parseGroup(file_line, file_stream, group, false, file, -1);
			}
		 };
	}
}

#endif