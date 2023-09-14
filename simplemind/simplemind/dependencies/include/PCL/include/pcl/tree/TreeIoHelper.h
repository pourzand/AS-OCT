#ifndef PCL_TREE_IO_HELPER
#define PCL_TREE_IO_HELPER

#include <pcl/tree/BreadthFirstIterator.h>
#include <pcl/exception.h>
#include <iostream>
#include <fstream>

namespace pcl
{
	namespace tree
	{

		class TreeIoHelper
		{
		public:
			//Binary writer

			template <class TreeType, class DataWriteFunc>
			static void WriteBinary(std::ostream& os, const TreeType& tree, DataWriteFunc& data_writer)
			{
				BreadthFirstIterator<TreeType> iter(tree);
				pcl_ForIterator(iter) {
					auto node = iter.get();
					int temp;
					temp = node.id();
					os.write((char*)(&temp), sizeof(int));
					temp = node.parentId();
					os.write((char*)(&temp), sizeof(int));
					data_writer(os, node.data());
					if (!os.good()) pcl_ThrowException(Exception(), "Failure occured during IO!");
				}
				int temp = -1;
				os.write((char*)(&temp), sizeof(int)); //End of stream indicator
			}

			template <class TreeType, class DataWriteFunc>
			static void WriteBinary(const std::string& filename, const TreeType& tree, DataWriteFunc& data_writer)
			{
				std::ofstream os(filename.c_str(), std::iostream::binary);
				WriteBinary(os, tree, data_writer);
				os.close();
			}

			//Text writer

			template <class TreeType, class DataWriteFunc>
			static void WriteText(std::ostream& os, const TreeType& tree, DataWriteFunc& data_writer)
			{
				BreadthFirstIterator<TreeType> iter(tree);
				pcl_ForIterator(iter) {
					auto node = iter.get();
					os << node.id() << std::endl;
					os << node.parentId() << std::endl;
					data_writer(os, node.data());
					if (!os.good()) pcl_ThrowException(Exception(), "Failure occured during IO!");
				}
				os << -1 << std::endl; //End of stream indicator
			}

			template <class TreeType, class DataWriteFunc>
			static void WriteText(const std::string& filename, const TreeType& tree, DataWriteFunc& data_writer)
			{
				std::ofstream os(filename.c_str(), std::iostream::binary);
				WriteText(os, tree, data_writer);
				os.close();
			}

			//Auto writer

			template <class TreeType, class DataBinaryWriter, class DataTextWriter>
			static void Write(const std::string& filename, const TreeType& tree, DataBinaryWriter& bin_writer, DataTextWriter& txt_writer, bool binary)
			{
				std::ofstream os(filename.c_str(), std::iostream::binary);
				Write(os, tree, bin_writer, txt_writer, binary);
				os.close();
			}

			template <class TreeType, class DataBinaryWriter, class DataTextWriter>
			static void Write(std::ostream& os, const TreeType& tree, DataBinaryWriter& bin_writer, DataTextWriter& txt_writer, bool binary)
			{
				if (binary) {
					os << "BIN\n";
					WriteBinary(os, tree, bin_writer);
				} else {
					os << "TXT\n";
					WriteText(os, tree, txt_writer);
				}
			}

			//Binary reader

			template <class TreeType, class DataReadFunc>
			static TreeType ReadBinary(std::istream& is, DataReadFunc& data_reader)
			{
				TreeType result;
				bool end_encountered = false;
				while (is.good()) {
					int id, parent;
					is.read((char*)&id, sizeof(int));
					if (id==-1) {
						end_encountered = true;
						break;
					}
					is.read((char*)&parent, sizeof(int));
					result.customAdd(id, parent, data_reader(is));
				}
				if (!is.good()) pcl_ThrowException(Exception(), "Failure occured during IO!");
				if (!end_encountered) pcl_ThrowException(Exception(), "Stream ended unexpectedly!");
				return std::move(result);
			}

			template <class TreeType, class DataReaderFunc>
			static TreeType ReadBinary(const std::string& filename, DataReaderFunc& data_reader)
			{
				std::ifstream is(filename.c_str(), std::iostream::binary);
				TreeType result = ReadBinary(is, data_writer);
				is.close();
				return std::move(result);
			}

			//Text reader

			template <class TreeType, class DataReadFunc>
			static TreeType ReadText(std::istream& is, DataReadFunc& data_reader)
			{
				TreeType result;
				bool end_encountered = false;
				while (is.good()) {
					std::string buffer;
					std::getline(is, buffer);
					int id = atoi(buffer.c_str());
					if (id==-1) {
						end_encountered = true;
						break;
					}
					std::getline(is, buffer);
					int parent = atoi(buffer.c_str());
					result.customAdd(id, parent, data_reader(is));
				}
				if (!is.good()) pcl_ThrowException(Exception(), "Failure occured during IO!");
				if (!end_encountered) pcl_ThrowException(Exception(), "Stream ended unexpectedly!");
				return std::move(result);
			}

			template <class TreeType, class DataReaderFunc>
			static TreeType ReadText(const std::string& filename, DataReaderFunc& data_reader)
			{
				std::ifstream is(filename.c_str(), std::iostream::binary);
				TreeType result = ReadText(is, data_writer);
				is.close();
				return std::move(result);
			}
			
			//Auto reader

			template <class TreeType, class DataBinaryReader, class DataTextReader>
			static TreeType Read(const std::string& filename, DataBinaryReader& bin_reader, DataTextReader& text_reader)
			{
				std::ifstream is(filename.c_str(), std::iostream::binary);
				TreeType result = Read(is, bin_reader, text_reader);
				is.close();
				return std::move(result);
			}

			template <class TreeType, class DataBinaryReader, class DataTextReader>
			static TreeType Read(std::istream& is, DataBinaryReader& bin_reader, DataTextReader& text_reader)
			{
				TreeType result;
				char header[5];
				is.read(header, 4); header[4] = NULL;
				if (strcmp(header, "TXT\n")==0) result = ReadText<TreeType>(is, text_reader);
				else if (strcmp(header, "BIN\n")==0) result = ReadBinary<TreeType>(is, bin_reader);
				else pcl_ThrowException(Exception(), "Invalid header \"" + std::string(header) + "\" encountered!");
				return std::move(result);
			}
		};

	}
}

#endif