#ifndef PCL_CLASSIFICATION_TRAINING_SAMPLE
#define PCL_CLASSIFICATION_TRAINING_SAMPLE

#include <boost/smart_ptr.hpp>
#include <vector>

namespace pcl
{

	template <class FType=double, class LType=char>
	class ClassificationTrainingObservation
	{
	public:
		typedef ClassificationTrainingObservation Self;
		typedef FType FeatureType;
		typedef LType LabelType;
		typedef boost::shared_ptr<std::vector<FeatureType> > FeatureVector;
		typedef boost::shared_ptr<const std::vector<FeatureType> > ConstantFeatureVector;

		static FeatureVector CreateFeatureVector(int size)
		{
			return FeatureVector(new std::vector<Type>(size));
		}
		
		static FeatureVector CreateFeatureVector()
		{
			return FeatureVector(new std::vector<Type>());
		}

		ClassificationTrainingObservation() 
		{
			m_Feature.reset(new std::vector<FeatureType>());
		}
		ClassificationTrainingObservation(int feature_size) 
		{
			m_Feature.reset(new std::vector<FeatureType>(feature_size));
		}
		ClassificationTrainingObservation(const Self& obj)
		{
			copy(obj);
		}
		ClassificationTrainingObservation(const FeatureVector& feature, int label, double weight=1)
		{
			m_Feature = feature;
			m_Label = label;
			m_Weight = 1;
		}

		int size() const
		{
			return static_cast<int>(m_Feature->size());
		}

		/** Feature access **/
		FeatureType& operator[](int i)
		{
			return (*m_Feature)[i];
		}
		FeatureType operator[](int i) const
		{
			return (*m_Feature)[i];
		}

		FeatureVector feature()
		{
			return m_Feature;
		}
		ConstantFeatureVector feature()const
		{
			return *m_Feature;
		}
		
		FeatureType& feature(int i)
		{
			return (*m_Feature)[i];
		}
		FeatureType feature(int i) const
		{
			return (*m_Feature)[i];
		}

		/** Label access **/
		LabelType& label()
		{
			return m_Label;
		}
		LabelType label() const
		{
			return m_Label;
		}
		
		/** weight access **/
		double& weight() 
		{
			return m_Weight;
		}
		
		double weight() const
		{
			return m_Weight;
		}

		/** Assignment methods **/
		Self& operator=(const Self& obj)
		{
			copy(obj);
			return *this;
		}

		Self getCopy() const
		{
			Self result(size());
			*(result.m_Feature) = *(this->m_Feature);
			result.m_Label = this->m_Label;
			result.m_Weight = this->m_Weight;
			return result;
		}
		Self getCopy(const std::vector<int>& index) const
		{
			Self result(index.size());
			for (int i=0; i<index.size(); i++) {
				result[i] = (*this)[index[i]];
			}
			result.m_Label = this->m_Label;
			result.m_Weight = this->m_Weight;
			return result;
		}
		
		/** Print methods **/
		inline std::string toString() const 
		{
			std::stringstream buffer;
			print(buffer);
			return buffer.str();
		}

		inline operator std::string () const
		{
			return toString();
		}

		inline std::ostream& print(std::ostream& os) const
		{
			if (sizeof(LType)==1) os << (int)m_Label << " | ";
			else os << m_Label << " | ";
			if (sizeof(Type)==1) {
				os << (int)(*m_Feature)[0];
				for (int i=1; i<m_Feature->size(); i++) {
					os << " " << (int)(*m_Feature)[i];
				}
			} else {
				os << m_Feature[0];
				for (int i=1; i<m_Feature->size(); i++) {
					os << " " << (*m_Feature)[i];
				}
			}
			os << " | " << m_Weight;
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const Self& obj)
		{
			return obj.print(os);
		}

	protected:
		FeatureVector m_Feature;
		LabelType m_Label;
		double m_Weight;

		void copy(const Self& obj)
		{
			m_Feature = obj.m_Feature;
			m_Label = obj.m_Label;
			m_Weight = obj.m_Weight;
		}
	};

}

#endif