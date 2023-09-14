#ifndef PCL_STL_FORWARD_ITERATOR_MIMICKER
#define PCL_STL_FORWARD_ITERATOR_MIMICKER

namespace pcl
{

	template <class ImageType, class IteratorType>
	class StlForwardIteratorMimicker
	{
	public:
		StlForwardIteratorMimicker(const typename ImageType::Pointer& img, IteratorType& iter) 
		{
			m_Image = img;
			m_Iterator = &iter;
		}

		StlForwardIteratorMimicker(const StlForwardIteratorMimicker& obj) 
		{
			m_Image = obj.m_Image;
			m_Iterator = obj.m_Iterator;
		}
		
		inline StlForwardIteratorMimicker& begin()
		{
			m_Iterator->begin();
			return *this;
		}

		inline StlForwardIteratorMimicker end() //A fake function that does nothing!
		{
			return *this;
		}

		inline StlForwardIteratorMimicker& operator=(const StlForwardIteratorMimicker& obj)
		{
			if (this==&obj) return *this;
			m_Image = obj.m_Image;
			m_Iterator = obj.m_Iterator;
			return *this;
		}

		inline bool operator!=(const StlForwardIteratorMimicker& obj) const //This function is purely meant for checking end condition, and does not depend on the input parameter
		{
			return !m_Iterator->end();
		}
		
		inline bool operator==(const StlForwardIteratorMimicker& obj) const //This function is purely meant for checking end condition, and does not depend on the input parameter
		{
			return m_Iterator->end();
		}

		inline StlForwardIteratorMimicker& operator++()
		{
			m_Iterator->next();
			return *this;
		}
		
		friend StlForwardIteratorMimicker& operator++(StlForwardIteratorMimicker& obj)
		{
			m_Iterator->next();
			return *this;
		}

		inline typename typename ImageType::ReferenceValueType operator*()
		{
			return m_Image->at(*m_Iterator);
		}

	protected:
		typename ImageType::Pointer m_Image;
		IteratorType *m_Iterator;
	};


	template <class ImageType, class IteratorType>
	class StlConstantForwardIteratorMimicker
	{
	public:
		StlConstantForwardIteratorMimicker(const typename ImageType::ConstantPointer& img, IteratorType& iter) 
		{
			m_Image = img;
			m_Iterator = &iter;
		}

		StlConstantForwardIteratorMimicker(const StlForwardIteratorMimicker& obj) 
		{
			m_Image = obj.m_Image;
			m_Iterator = obj.m_Iterator;
		}
		
		inline StlConstantForwardIteratorMimicker& begin()
		{
			m_Iterator->begin();
			return *this;
		}

		inline StlConstantForwardIteratorMimicker& end() //A fake function that does nothing!
		{
			return *this;
		}

		inline StlConstantForwardIteratorMimicker& operator=(const StlConstantForwardIteratorMimicker& obj)
		{
			if (this==&obj) return *this;
			m_Image = obj.m_Image;
			m_Iterator = obj.m_Iterator;
			return *this;
		}

		inline bool operator!=(const StlConstantForwardIteratorMimicker& obj) const //This function is purely meant for checking end condition, and does not depend on the input parameter
		{
			return m_Iterator->end();
		}
		
		inline bool operator==(const StlConstantForwardIteratorMimicker& obj) const //This function is purely meant for checking end condition, and does not depend on the input parameter
		{
			return m_Iterator->end();
		}

		inline StlConstantForwardIteratorMimicker& operator++()
		{
			m_Iterator->next();
			return *this;
		}
		
		friend StlConstantForwardIteratorMimicker& operator++(StlConstantForwardIteratorMimicker& obj)
		{
			m_Iterator->next();
			return *this;
		}

		inline typename ImageType::ConstantValueType operator*()
		{
			return m_Image->get(*m_Iterator);
		}

	protected:
		typename ImageType::ConstantPointer m_Image;
		IteratorType *m_Iterator;
	};

}

#endif