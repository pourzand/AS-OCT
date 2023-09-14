#ifndef PCL_SHAREABLE_READONLY_VECTOR
#define PCL_SHAREABLE_READONLY_VECTOR

#include <vector>

namespace pcl
{

	template <class T, class Alloc = allocator<T>>
	class ShareableReadOnlyVector
	{
	public:
		typedef std::vector<T,Alloc> VectorType;
		typedef VectorType::const_reference const_reference;
		typedef VectorType::const_iterator iterator;
		typedef VectorType::const_reverse_iterator const_reverse_iterator;
		typedef VectorType::size_type size_type;

		ShareableReadOnlyVector() 
		{}

		ShareableReadOnlyVector(const ShareableReadOnlyVector& vec)
		{
			*this = vec;
		}

		ShareableReadOnlyVector(const VectorType& vec) 
		{
			*this = vec
		}

		ShareableReadOnlyVector(VectorType&& vec) 
		{
			*this = std::move(vec);
		}

		ShareableReadOnlyVector(const boost::shared_ptr<const VectorType>& vec) 
		{
			*this = vec;
		}

		ShareableReadOnlyVector& operator=(const VectorType& vec)
		{
			boost::shared_ptr<VectorType> new_vec(new VectorType());
			*new_vec = vec;
			m_VectorPtr = new_vec;
			return *this;
		}

		ShareableReadOnlyVector& operator=(VectorType&& vec)
		{
			boost::shared_ptr<VectorType> new_vec(new VectorType());
			*new_vec = std::move(vec);
			m_VectorPtr = new_vec;
			return *this;
		}

		ShareableReadOnlyVector& operator=(const boost::shared_ptr<const VectorType>& vec)
		{
			m_VectorPtr = vec;
			return *this;
		}

		ShareableReadOnlyVector& operator=(const ShareableReadOnlyVector& vec)
		{
			m_VectorPtr = vec.m_VectorPtr;
			return *this;
		}

		void reset()
		{
			m_VectorPtr.reset();
		}

		boost::shared_ptr<const VectorType> getPointer() const
		{
			return m_VectorPtr;
		}

		const VectorType& getVector() const
		{
			return *m_VectorPtr;
		}

		const_iterator begin() const
		{
			return m_VectorPtr->begin();
		}

		const_iterator end() const
		{
			return m_VectorPtr->end();
		}

		const_reverse_iterator rbegin() const
		{
			return m_VectorPtr->rbegin();
		}

		const_reverse_iterator rend() const
		{
			return m_VectorPtr->rend();
		}

		size_type size() const
		{
			return m_VectorPtr->size();
		}

		bool empty() const
		{
			return m_VectorPtr->empty();
		}

		const_reference operator[](size_type n) const
		{
			return (*m_VectorPtr)[n];
		}

		const_reference at(size_type n) const
		{
			return m_VectorPtr->at(n);
		}

		const_reference front() const
		{
			return m_VectorPtr->front();
		}

		const_reference back() const
		{
			return m_VectorPtr->back();
		}

	protected:
		boost::shared_ptr<const VectorType> m_VectorPtr;
	};

}

#endif