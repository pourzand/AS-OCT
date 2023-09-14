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

	~FastLookupImageBuffer()
	{
		if (m_Own) delete[] m_Data;
	}

	/******** Reference retrieval ********/
	inline ReferenceValueType at(int x, int y, int z)
	{
		return at(x + m_LookupTableY[y] + m_LookupTableZ[z]);
	}
	inline ReferenceValueType at(const Point3D<int>& p)
	{
		return at(p.x() + m_LookupTableY[p.y()] + m_LookupTableZ[p.z()]);
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
		int z = index%m_OffsetTable[2];
		index -= z*m_OffsetTable[2];
		int y = index%m_OffsetTable[1];
		index -= y*m_OffsetTable[1];
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
	long m_OffsetTable[3];
	BufferType m_Data;
	bool m_Own;
	boost::scoped_array<long> m_LookupTableY, m_LookupTableZ;

	FastLookupImageBuffer() {}

	void setSize(int sx, int sy, int sz) 
	{
		m_Size.set(sx, sy, sz);
		m_OffsetTable[0] = 1;
		m_OffsetTable[1] = m_Size.x();
		m_OffsetTable[2] = m_Size.x()*m_Size.y();

		m_LookupTableY.reset(new long[m_Size.y()]);
		for (int i=0; i<m_Size.y(); i++) {
			m_LookupTableY[i] = i*m_OffsetTable[1];
		}

		m_LookupTableZ.reset(new long[m_Size.z()]);
		for (int i=0; i<m_Size.z(); i++) {
			m_LookupTableZ[i] = i*m_OffsetTable[2];
		}
	}