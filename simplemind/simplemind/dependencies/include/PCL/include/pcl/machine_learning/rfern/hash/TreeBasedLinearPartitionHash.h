#ifndef PCL_TREE_BASED_LINEAR_PARTITION_HASH
#define PCL_TREE_BASED_LINEAR_PARTITION_HASH

#include <pcl/exception.h>
#include <pcl/math.h>
#include <pcl/misc/StringTokenizer.h>
#include <pcl/tree/BinaryTree.h>
#include <pcl/tree/TreeIoHelper.h>
#include <pcl/tree/UnorderedIterator.h>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/random.hpp>
#include <math.h>
#include <time.h>

namespace pcl
{
	namespace rfern
	{

		class TreeBasedLinearPartitionHash
		{
		public:
			struct Parameter 
			{};

			TreeBasedLinearPartitionHash() 
			{}

			void setNodeNum(int num)
			{
				m_NodeNum = num;
			}

			void initialize(const Parameter& param) 
			{}

			template <class TrainingDataListType>
			void train(const TrainingDataListType& training_data_list) 
			{
				m_FeatureNum = (*training_data_list.begin()).size();
				boost::random::mt19937 rnd_gen;
				rnd_gen.seed(static_cast<int>(std::time(NULL)));
				std::queue<int> queue;
				//Initializing tree with the first partition
				{
					Data data(m_FeatureNum);
					data.randomizeCoefficient(training_data_list, rnd_gen);
					queue.push(m_Tree.add(-1, std::move(data)));
				}
				//Adding the rest of node (or partition)
				for (int num=3; num<=m_NodeNum; ++num) {
					Data data(m_FeatureNum);
					data.randomizeCoefficient(training_data_list, rnd_gen);
					while (true) {
						int id = m_Tree.add(queue.front(), std::move(data));
						if (id<0) {
							queue.pop();
						} else {
							queue.push(id);
							break;
						}
					}
				}
				finalizeTree();
			}

			void printCoefficients()
			{
				pcl::tree::BreadthFirstIterator<TreeType> iter(m_Tree);
				pcl_ForIterator(iter) {
					auto node = iter.get();
					std::cout << node.id() << " (->" << node.parentId() << ") ";
					if (node.childrenNum()!=0)  {
						std::cout << node.data().offset << " -";
						pcl_ForEach(node.data().coefficient, item) std::cout << " " << *item;
					} else std::cout << node.data().index;
					std::cout << std::endl;
				}
			}

			template <class DataType>
			int computeIndex(DataType& obj) const
			{
				auto node = m_Tree.getRoot();
				while (node.childrenNum()!=0) {
					int index = node.data().getDivision(obj);
					node = node.child(index);
				}
				return node.data().index;
			}

			/************ IO methods ************/
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					if (!use_binary) {
						os << "TXT\n";
						os << m_NodeNum << std::endl;
						os << m_FeatureNum << std::endl;
						pcl::tree::TreeIoHelper::WriteText(os, m_Tree, [](std::ostream& os, const Data& data) {
							data.write(os, false);
						});
					} else {
						os << "BIN\n";
						os.write((char*)&m_NodeNum, sizeof(int));
						os.write((char*)&m_FeatureNum, sizeof(int));
						pcl::tree::TreeIoHelper::WriteBinary(os, m_Tree, [](std::ostream& os, const Data& data) {
							data.write(os, true);
						});
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

			void read(std::istream& is)
			{
				try {
					//Reading main info
					char header[5];
					is.read(header, 4); header[4] = NULL;
					if (strcmp(header, "TXT\n")==0) {
						std::string buffer;
						std::getline(is, buffer);
						m_NodeNum = atoi(buffer.c_str());
						std::getline(is, buffer);
						m_FeatureNum = atoi(buffer.c_str());
						m_Tree = pcl::tree::TreeIoHelper::ReadText<TreeType>(is, [&](std::istream& is)->Data {
							Data result(m_FeatureNum);
							result.read(is,false);
							return std::move(result);
						});
					} else if (strcmp(header, "BIN\n")==0) {
						is.read((char*)&m_NodeNum, sizeof(int));
						is.read((char*)&m_FeatureNum, sizeof(int));
						m_Tree = pcl::tree::TreeIoHelper::ReadBinary<TreeType>(is, [&](std::istream& is)->Data {
							Data result(m_FeatureNum);
							result.read(is,true);
							return std::move(result);
						});
					} else {
						pcl_ThrowException(Exception(), "Invalid header \"" + std::string(header) + "\" encountered!");
					}
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

		protected:
			int m_NodeNum, m_FeatureNum;
			double m_Multiplier;

			struct Data 
			{
				std::vector<double> coefficient;
				double offset;
				int index; //Not saved

				Data()
				{}

				Data(int feature_num)
				{
					coefficient.resize(feature_num);
				}

				Data& operator=(const Data& obj) 
				{
					coefficient = obj.coefficient;
					offset = obj.offset;
					index = obj.index;
					return *this;
				}

				Data& operator=(Data&& obj) 
				{
					coefficient = std::move(obj.coefficient);
					offset = obj.offset;
					index = obj.index;
					return *this;
				}

				template <class DataType>
				double getProjectedValue(const DataType& data) const
				{
					double result = 0;
					for (int i=0; i<coefficient.size(); ++i) {
						result += coefficient[i]*data[i];
					}
					return result;
				}

				template <class DataType>
				int getDivision(const DataType& data) const
				{
					if (getProjectedValue(data)>offset) return 1;
					return 0;
				}

				void write(std::ostream& os, bool binary) const
				{
					if (binary) {
						if (coefficient.empty()) {
							char temp = 0;
							os.write((char*)&temp, sizeof(char));
							os.write((const char*)&index, sizeof(int));
						} else {
							char temp = 1;
							os.write((char*)&temp, sizeof(char));
							os.write((const char*)&offset, sizeof(double));
							pcl_ForEach(coefficient, item) os.write((const char*)&(*item), sizeof(double));
						}
					} else {
						if (coefficient.empty()) {
							os << 0 << " " << index << std::endl;
						} else {
							os << 1 << " " << offset;
							pcl_ForEach(coefficient, item) os << " " << *item;
							os << std::endl;
						}
					}
				}

				void read(std::istream& is, bool binary)
				{
					if (binary) {
						char type;
						is.read(&type, sizeof(char));
						if (type==0) {
							is.read((char*)&index, sizeof(int));
						} else {
							is.read((char*)&offset, sizeof(double));
							pcl_ForEach(coefficient, item) is.read((char*)&(*item), sizeof(double));
						}
					} else {
						std::string buffer;
						std::getline(is, buffer);
						pcl::misc::StringTokenizer tokenizer(buffer.c_str());
						tokenizer.begin(' ');
						if (tokenizer.end()) pcl_ThrowException(Exception(), "Insufficient data read!");
						int type = atoi(tokenizer.getToken().c_str());
						tokenizer.next(' ');
						if (tokenizer.end()) pcl_ThrowException(Exception(), "Insufficient data read!");
						if (type==0) {
							index = atoi(tokenizer.getToken().c_str());
						} else {
							offset = atof(tokenizer.getToken().c_str());
							pcl_ForEach(coefficient, item) {
								tokenizer.next(' ');
								if (tokenizer.end()) pcl_ThrowException(Exception(), "Insufficient data read!");
								*item = atof(tokenizer.getToken().c_str());
							}
						}
					}
				}

				template <class TrainingDataListType, class RandomGenerator>
				void randomizeCoefficient(const TrainingDataListType& training_data_list, RandomGenerator& rnd_gen)
				{
					boost::random::uniform_real_distribution<> rand_dist(-1,1);
					for (int i=0; i<coefficient.size(); ++i) {
						coefficient[i] = rand_dist(rnd_gen);
					}

					double min_range = std::numeric_limits<double>::infinity(), 
						max_range = -std::numeric_limits<double>::infinity();
					bool init = true;
					pcl_ForEach(training_data_list, item) {
						double val = getProjectedValue(*item);
						min_range = pcl_Min(min_range, val);
						max_range = pcl_Max(max_range, val);
					}
					boost::random::uniform_real_distribution<> range_dist(min_range, max_range);
					offset = range_dist(rnd_gen);
				}
			};

			typedef pcl::tree::BinaryTree<Data> TreeType;
			TreeType m_Tree;

			void finalizeTree()
			{
				std::vector<int> leaf;
				pcl::tree::UnorderedIterator<TreeType> iter(m_Tree);
				pcl_ForIterator(iter) {
					auto node = iter.get();
					if (node.childrenNum()<2) leaf.push_back(node.id());
				}
				int index = 0;
				pcl_ForEach(leaf, item) {
					int child_num = m_Tree.getChildrenNum(*item);
					for (int i=child_num; i<2; ++i) {
						Data temp;
						temp.index = index;
						m_Tree.add(*item, std::move(temp));
						++index;
					}
				}
			}

		};

	}
}

#endif