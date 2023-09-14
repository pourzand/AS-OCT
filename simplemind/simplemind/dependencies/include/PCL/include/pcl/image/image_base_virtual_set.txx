/******** Virtual set methods from ImageBase ********/
protected:
	pcl::image_base_details::SetDouble m_DoubleValueSetter;

public:

	using Parent::setValue;

	virtual bool setValue(long index, double value) 
	{
		return m_DoubleValueSetter(this, index, value);
	}

	virtual bool setValue(int x, int y, int z, double value) 
	{
		return m_DoubleValueSetter(this, x,y,z, value);
	}
