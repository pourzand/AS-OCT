#ifndef PCL_OBSERVATION
#define PCL_OBSERVATION

#include <boost/smart_ptr.hpp>
#include <vector>

namespace pcl
{

	template <class Type=double, class LType=char>
	class Observation
	{
	public:
		typedef Observation Self;
		typedef Type ValueType;
		typedef LType LabelType;
		typedef boost::shared_ptr<std::vector<ValueType> > FeatureVector;
		typedef boost::shared_ptr<const std::vector<ValueType> > ConstantFeatureVector;

		static FeatureVector CreateFeatureVector(int size)
		{
			return FeatureVector(new std::vector<Type>(size));
		}
		
		static FeatureVector CreateFeatureVector()
		{
			return FeatureVector(new std::vector<Type>());
		}

		Observation() 
		{
			m_Feature.reset(new std::vector<Type>());
			m_Output.reset(new std::vector<Type>());
		}
		Observation(int feature_size, int output_size) 
		{
			m_Feature.reset(new std::vector<Type>(feature_size));
			m_Output.reset(new std::vector<Type>(output_size));
			m_Weight = 0;
		}
		Observation(const Self& obj)
		{
			copy(obj);
		}
		
		Observation(const FeatureVector& feature, const FeatureVector& output, double weight, LType label=0)
		{
			m_Feature = feature;
			m_Output = output;
			m_Weight = weight;
			m_Label = label;
		}

		int size() const
		{
			return static_cast<int>(m_Feature->size());
		}

		int output_size() const 
		{
			return static_cast<int>(m_Output->size());
		}

		/** Feature access **/
		ValueType& operator[](int i)
		{
			return (*m_Feature)[i];
		}
		ValueType operator[](int i) const
		{
			return (*m_Feature)[i];
		}
		
		FeatureVector feature()
		{
			return m_Feature;
		}
		ConstantFeatureVector feature() const
		{
			return m_Feature;
		}

		ConstantFeatureVector feature(int i)
		{
			return (*m_Feature)[i];
		}
		ValueType feature(int i) const
		{
			return (*m_Feature)[i];
		}
		
		/** output access **/
		FeatureVector output()
		{
			return m_Output;
		}
		ConstantFeatureVector output() const
		{
			return *m_Output;
		}
		
		ValueType& output(int i)
		{
			return (*m_Output)[i];
		}
		ValueType output(int i) const
		{
			return (*m_Output)[i];
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
			*(result.m_Output) = *(this->m_Output);
			result.m_Weight = this->m_Weight;
			result.m_Label = this->m_Label;
			return result;
		}
		Self getCopy(const std::vector<int>& index) const
		{
			Self result(index.size());
			for (int i=0; i<index.size(); i++) {
				result[i] = (*this)[index[i]];
			}
			*(result.m_Output) = *(this->m_Output);
			result.m_Weight = this->m_Weight;
			result.m_Label = this->m_Label;
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
			if (sizeof(Type)==1) {
				os << (int)(*m_Feature)[0];
				for (int i=1; i<m_Feature->size(); i++) {
					os << " " << (int)(*m_Feature)[i];
				}
				os << " | ";
				os << (int)(*m_Output)[0];
				for (int i=1; i<m_Output->size(); i++) {
					os << " " << (int)(*m_Output)[i];
				}
			} else {
				os << (*m_Feature)[0];
				for (int i=1; i<m_Feature->size(); i++) {
					os << " " << (*m_Feature)[i];
				}
				os << " | ";
				os << (*m_Output)[0];
				for (int i=1; i<m_Output->size(); i++) {
					os << " " << (*m_Output)[i];
				}
			}
			os << " | " << m_Weight;
			if (sizeof(LType)==1) os << " | " << (int)m_Label;
			else os << " | " << m_Label;
			return os;
		}

		friend std::ostream& operator<<(std::ostream& os, const Self& obj)
		{
			return obj.print(os);
		}

	protected:
		FeatureVector m_Feature;
		FeatureVector m_Output;
		LabelType m_Label;
		double m_Weight; //Only used by regression

		void copy(const Self& obj)
		{
			m_Feature = obj.m_Feature;
			m_Output = obj.m_Output;
			m_Weight = obj.m_Weight;
			m_Label = obj.m_Label;
		}
	};

}

#endif