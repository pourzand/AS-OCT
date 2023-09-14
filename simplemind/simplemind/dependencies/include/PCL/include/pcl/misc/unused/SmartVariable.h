#ifndef SMART_VARIABLE
#define SMART_VARIABLE

template <class Type>
class SmartVariable
{
public:
	SmartVariable()
	{
		m_IsDefined = false;
	}

	inline operator Type() const
	{ 
		return m_Variable; 
	}

	inline Type& operator=(const Type& val) 
	{
		m_Variable = val;
		m_IsDefined = true;
	}

	inline bool isDefined()
	{
		return m_IsDefined;
	}

protected:
	Type m_Variable;
	bool m_IsDefined;
};

/*************************************************************/

template <class Type>
class SmartConditionedVariable
{
public:
	SmartVariable()
	{
		m_IsInit = true;
	}

	inline void assign(bool condition, const Type& val) 
	{
		if (m_IsInit || condition) {
			m_IsInit = false;
			m_Variable = val;
		}
	}

	inline Type& getValue()
	{
		return m_Variable;
	}

	inline bool isDefined()
	{
		return !m_IsInit;
	}

protected:
	Type m_Variable;
	bool m_IsInit;
};



template <class Type1, class Type2>
class SmartConditionedVariable
{
public:
	SmartVariable()
	{
		m_IsInit = true;
	}

	inline void assign(bool condition, const Type1& val1, const Type2& val2) 
	{
		if (m_IsInit || condition) {
			m_IsInit = false;
			m_Variable1 = val1;
			m_Variable2 = val2;
		}
	}

	inline Type1& getValue1()
	{
		return m_Variable1;
	}

	inline Type2& getValue2()
	{
		return m_Variable2;
	}

	inline bool isDefined()
	{
		return !m_IsInit;
	}

protected:
	Type1 m_Variable1;
	Type2 m_Variable2;
	bool m_IsInit;
};

#endif