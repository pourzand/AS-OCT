/******** Virtual get methods from ImageBase ********/
protected:
	pcl::image_base_details::ToDouble m_DoubleValueConverter;

public:

	using Parent::getValue;

	virtual double getValue(long index) const
	{
		return m_DoubleValueConverter(this->get(index));
	}

	virtual double getValue(int x, int y, int z) const
	{
		return m_DoubleValueConverter(this->get(x,y,z));
	}
