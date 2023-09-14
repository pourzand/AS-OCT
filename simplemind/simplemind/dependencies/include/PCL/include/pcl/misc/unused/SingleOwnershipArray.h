#ifndef SINGLE_OWNERSHIP_ARRAY
#define SINGLE_OWNERSHIP_ARRAY

template <class Type>
class SingleOwnershipArray
{
public:
	SingleOwnershipArray()
	{
		m_Pointer = NULL;
		m_Own = false;
	}
	SingleOwnershipArray(Type* pointer, bool own=true) 
	{
		m_Pointer = pointer;
		if (m_Pointer==NULL) m_Own = false;
		else m_Own = own;
	}
	SingleOwnershipArray(const SingleOwnershipArray& obj) {
		m_Pointer = obj.m_Pointer;
		m_Own = false;
	}
	~SingleOwnershipArray() 
	{
		clean();
	}

	inline Type* get() const
	{
		return m_Pointer;
	}

	inline Type& operator*() const 
	{
		return *m_Pointer;
	}

	inline Type* operator->() const
	{
		return m_Pointer;
	}

	inline Type& operator[](long i) const
	{
		return m_Pointer[i];
	}

	SingleOwnershipArray& operator=(const SingleOwnershipArray& obj)
	{
		m_Pointer = obj.m_Pointer;
		m_Own = false;
	}

	bool isOwner() 
	{
		return m_Own;
	}

	void reset() {
		clean();
		m_Pointer = NULL;
		m_Own = false;
	}
	void reset(Type* pointer, bool own=true) 
	{
		if (m_Pointer==pointer) return;
		clean();
		m_Pointer = pointer;
		if (m_Pointer==NULL) m_Own = false;
		else m_Own = own;
	}
	void reset(const SingleOwnershipArray& obj) 
	{
		if (m_Pointer==obj.m_Pointer) return;
		clean();
		m_Pointer = pointer;
		m_Own = false;
	}

	void release() 
	{
		m_Pointer = NULL;
		m_Own = false;
	}

	bool transferOwnershipTo(SingleOwnershipArray& obj)
	{
		if (!m_Own || &obj==this) return false;
		if (m_Pointer!=obj.m_Pointer) return false;
		m_Own = false;
		obj.m_Own = true;
	}

protected:
	Type *m_Pointer;
	bool m_Own;

	inline void clean() {
		if (m_Own) delete[] m_Pointer;
	}
};

#endif