public:
	static Pointer New(const I& image, const std::string& name)
	{
		return Pointer(new Self(image, name));
	}

	virtual void forceNextCompute()
	{}

	virtual T getResult(size_t i=0) const
	{
		return m_Value;
	}

	virtual void populateResult(std::vector<T>& result) const
	{
		result.push_back(m_Value);
	}

	virtual void populateResult(std::vector<T>& result, size_t offset) const
	{
		result[offset] = m_Value;
	}

	virtual std::string getFeatureName(size_t i=0) const
	{
		return m_Name;
	}

protected:
	ImagePointer m_Image;
	std::string m_Name;
	T m_Value;

	Self(const I& image, const std::string& name)
	{
		m_Image = image;
		m_Name = name;
		this->m_Size = 1;
	}