public:
	static Pointer New(long sx, long sy, long sz) {
		Pointer obj(new Self());
		obj->setupBuffer(sx, sy, sz);
		obj->m_Own = true;
		obj->setSize(sx, sy, sz);
		return obj;
	}

	static Pointer New(BufferType ptr, long sx, long sy, long sz, bool own=false) {
		Pointer obj(new Self());
		obj->m_Data = ptr;
		obj->m_Own = own;
		obj->setSize(sx, sy, sz);
		return obj;
	}
	
	/******** Reference retrieval ********/
	inline ReferenceValueType at(int x, int y, int z)
	{
		return at(x + y*m_OffsetTable[1] + z*m_OffsetTable[2]);
	}
	inline ReferenceValueType at(const Point3D<int>& p)
	{
		return at(p.x() + p.y()*m_OffsetTable[1] + p.z()*m_OffsetTable[2]);
	}
	
	inline ConstantValueType at(int x, int y, int z) const 
	{
		return at(x + y*m_OffsetTable[1] + z*m_OffsetTable[2]);
	}
	inline ConstantValueType at(const Point3D<int>& p) const
	{
		return at(p.x() + p.y()*m_OffsetTable[1] + p.z()*m_OffsetTable[2]);
	}

	/******** Buffer information retrieval ********/
	inline long toIndex(const Point3D<int>& p) const
	{
		return toIndex(p.x(), p.y(), p.z());
	}
	inline long toIndex(int x, int y, int z) const
	{
		return x + y*m_OffsetTable[1] + z*m_OffsetTable[2];
	}

	inline Point3D<int> toPoint(long index) const
	{
		int z = index/m_OffsetTable[2];
		index %= m_OffsetTable[2];
		int y = index/m_OffsetTable[1];
		index %= m_OffsetTable[1];
		return Point3D<int>(index, y, z);
	}

	inline const Point3D<int>& getSize() const
	{
		return m_Size;
	}

	inline long const* getOffsetTable() const
	{
		return m_OffsetTable;
	}

	/******** Local methods ********/
	BufferType getPointer()
	{
		return m_Data;
	}

	void dropOwnership()
	{
		m_Own = false;
	}

protected:
	Point3D<int> m_Size;
	long m_OffsetTable[4];
	BufferType m_Data;
	bool m_Own;

	ImageBuffer() {}

	void setSize(int sx, int sy, int sz) 
	{
		m_Size.set(sx, sy, sz);
		m_OffsetTable[0] = 1;
		m_OffsetTable[1] = m_Size.x();
		m_OffsetTable[2] = static_cast<long>(m_Size.x())*static_cast<long>(m_Size.y());
		m_OffsetTable[3] = m_OffsetTable[2]*static_cast<long>(m_Size.z());
	}