/******** Virtual set methods from ImageBase ********/
public:

	using Parent::setValue;

	virtual bool setValue(long, double) 
	{
		return false;
	}

	virtual bool setValue(int, int, int, double) 
	{
		return false;
	}
