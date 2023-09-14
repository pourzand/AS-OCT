#ifndef PCL_ANN
#define PCL_ANN

#include <ANN.h>
#include <pcl/exception.h>
#include <pcl/machine_learning/ann/VoteProbabilityCalculator.h>
#include <pcl/misc/ClassInstanceTracker.h>
#include <pcl/macro.h>
#include <pcl/machine_learning/FeatureIoHelper.h>

namespace pcl
{
	namespace ann
	{

		namespace details 
		{
			struct AnnDestructor
			{
				AnnDestructor()
				{
					annClose();
				}
			};

			typedef ::pcl::misc::ClassInstanceTracker<void, void, AnnDestructor> AnnInstanceTracker;
		}


		template <class ProbabilityCalculator=VoteProbabilityCalculator, class InternalLabelType=char>
		class Ann: private details::AnnInstanceTracker
		{
		public:
			Ann(int k, double eps=0)
			{
				m_DataPts = NULL;
				m_K = k;
				m_Eps = eps;
				m_OwnDataPts = false;
			}
			Ann(std::istream& is)
			{
				m_DataPts = NULL;
				try {
					pcl::FeatureReader reader(is);
					{
						std::vector<double> ann_info;
						reader.read(ann_info);
						m_K = static_cast<int>(ann_info[0]);
						m_Eps = ann_info[1];
						m_FeatureSize = static_cast<int>(ann_info[2]);
						m_UniqueLabelNum = static_cast<int>(ann_info[3]);
					}
					reader.read(m_Label);
					m_KdTree.reset(new ANNkd_tree(is));
					m_DataPts = m_KdTree->thePoints();
					m_OwnDataPts = true;
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
				setupBuffer();
			}

			~Ann() 
			{ 
				if (m_DataPts!=NULL) {
					if (m_OwnDataPts) annDeallocPts(m_DataPts); 
					else delete[] m_DataPts; 
				} 
			}

			template <class TrainingDataType>
			void train(const TrainingDataType& training_data)
			{
				if (m_DataPts!=NULL) pcl_ThrowException(pcl::Exception(), "Classifier is already trained!");
				m_FeatureSize = training_data.featureSize();
				m_UniqueLabelNum = training_data.uniqueLabelSize();
				m_Label = training_data.labelIndexes<InternalLabelType>();
				setupDataPts(training_data.features());
				m_KdTree.reset(new ANNkd_tree(m_DataPts, m_Label.size(), m_FeatureSize));
				setupBuffer();
			}
			
			//TODO
			template <class FeatureDataType, class LabelDataType>
			void train(const FeatureDataType& training_data

			//************* Compute/apply method

			template <class T>
			std::vector<double> compute(const T& input) const
			{
				auto point_buffer = getAnnPointBuffer<T>(input);
				m_KdTree->annkSearch(point_buffer, m_K, m_IndexBuffer.get(), m_DistanceBuffer.get(), m_Eps);
				std::vector<double> result(m_UniqueLabelNum, 0);
				m_Calculator.process(result, m_IndexBuffer.get(), m_DistanceBuffer.get(), m_Label, m_K);
				return std::move(result);
			}

			//************* IO methods
			void write(std::ostream& os, bool use_binary=false) const
			{
				try {
					pcl::FeatureWriter writer(os, use_binary, ANNcoordPrec);
					{
						std::vector<double> ann_info;
						ann_info.push_back(m_K);
						ann_info.push_back(m_Eps);
						ann_info.push_back(m_FeatureSize);
						ann_info.push_back(m_UniqueLabelNum);
						writer.write(ann_info);
					}
					writer.write(m_Label);
					m_KdTree->Dump(ANNtrue, os);
				} catch (const std::ios_base::failure& e) {
					pcl_ThrowException(Exception(), e.what());
				}
			}

		protected:
			int m_K;
			double m_Eps;
			size_t m_FeatureSize, m_UniqueLabelNum;
			std::vector<InternalLabelType> m_Label;

			ProbabilityCalculator m_Calculator;
			
			ANNpoint *m_DataPts;
			bool m_OwnDataPts;
			
			mutable boost::scoped_array<ANNcoord> m_PointBuffer;
			boost::scoped_array<ANNidx> m_IndexBuffer;
			boost::scoped_array<ANNdist> m_DistanceBuffer;

			boost::scoped_ptr<ANNkd_tree> m_KdTree;

			void setupBuffer()
			{
				m_IndexBuffer.reset(new ANNidx[m_K]);
				m_DistanceBuffer.reset(new ANNdist[m_K]);
				m_PointBuffer.reset(new ANNcoord[m_FeatureSize]);
			}
			
			template <class T>
			typename boost::enable_if<boost::is_same<typename T::value_type, ANNcoord>, ANNpoint>::type getAnnPointBuffer(const T& input) const
			{
				return const_cast<ANNcoord*>(&input[0]);
			}
			template <class T>
			typename boost::disable_if<boost::is_same<typename T::value_type, ANNcoord>, ANNpoint>::type getAnnPointBuffer(const T& input) const
			{
				for (int i=0; i<m_FeatureSize; ++i) m_PointBuffer[i] = input[i];
				return m_PointBuffer.get();
			}

			template <class T>
			typename boost::enable_if<boost::is_same<typename T::value_type, ANNcoord>, void>::type setupDataPts(const T& training_data)
			{
				m_OwnDataPts = false;
				m_DataPts = new ANNpoint[training_data.size()];
				int count = 0;
				pcl_ForEach(training_data, item) {
					m_DataPts[count] = const_cast<ANNCoord*>(&(*item)[0]);
					++count;
				}
			}
			template <class T>
			typename boost::disable_if<boost::is_same<typename T::value_type, ANNcoord>, void>::type setupDataPts(const T& training_data)
			{
				m_OwnDataPts = true;
				m_DataPts = annAllocPts(training_data.size(), m_FeatureSize);
				int count = 0;
				pcl_ForEach(training_data, row) {
					int i = 0;
					pcl_ForEach(*row, item) {
						m_DataPts[count][i] = *item;
						++i;
					}
					++count;
				}
			}
		};

	}
}

#endif