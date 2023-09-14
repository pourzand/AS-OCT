#ifndef SINGLE_OWNERSHIP_POINTER
#define SINGLE_OWNERSHIP_POINTER

template <class Type>
class SingleOwnershipPointer
{
public:
	SingleOwnershipPointer()
	{
		m_Pointer = NULL;
		m_Own = false;
	}
	SingleOwnershipPointer(Type* pointer, bool own=true) 
	{
		m_Pointer = pointer;
		if (m_Pointer==NULL) m_Own = false;
		else m_Own = own;
	}
	SingleOwnershipPointer(const SingleOwnershipPointer& obj) {
		m_Pointer = obj.m_Pointer;
		m_Own = false;
	}
	~SingleOwnershipPointer() 
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

	SingleOwnershipPointer& operator=(const SingleOwnershipPointer& obj)
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
	void reset(const SingleOwnershipPointer& obj) 
	{
		if (m_Pointer==obj.m_Pointer) return;
		clean();
		m_Pointer = pointer;
		m_Own = false;
	}

	void release() 
	{
		m_Own = false;
		clean();
		m_Pointer = NULL;
		m_Own = false;
	}

	bool transferOwnershipTo(SingleOwnershipPointer& obj)
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
		if (m_Own) delete m_Pointer;
	}
};

#endif