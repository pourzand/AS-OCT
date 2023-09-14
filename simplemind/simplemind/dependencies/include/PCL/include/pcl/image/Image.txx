public:
	enum {
		ItkAliasable = Buffer::ItkAliasable
	};
	typedef Image Self;
	typedef ImagePhysicalLayer<Flag> Parent;
	typedef boost::shared_ptr< Self > Pointer;
	typedef boost::shared_ptr< Self const > ConstantPointer;
	typedef Buffer BufferType;
	typedef Type ValueType;
	typedef typename BufferType::ReferenceValueType ReferenceValueType;
	typedef typename BufferType::ConstantValueType ConstantValueType;
	typedef typename BufferType::IoValueType IoValueType;
	typedef typename Parent::OrientationMatrixType OrientationMatrixType;

	/******** New methods ********/
	static Pointer New(const Point3D<int>& size, const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
	{ 
		return New(Point3D<int>(0,0,0), size-1, spacing, origin); 
	}
	static Pointer New(const Point3D<int>& size, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
	{ 
		return New(Point3D<int>(0,0,0), size-1, spacing, origin, matrix); 
	}

	static Pointer New(const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
	{ 
		Pointer obj(new Self()); 
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
		obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
		obj->setBuffer(BufferType::New(obj->m_Size.x(), obj->m_Size.y(), obj->m_Size.z()));
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		return obj; 
	}
	
	static Pointer New(const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
	{ 
		Pointer obj(new Self()); 
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
		obj->setMinPoint(minp, Point3D<int>(0,0,0)); 
		obj->setBuffer(BufferType::New(obj->m_Size.x(), obj->m_Size.y(), obj->m_Size.z()));
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setOrientationMatrix(matrix);
		return obj; 
	} 

	static Pointer New(const typename BufferType::Pointer& ptr, const Point3D<int>& buffer_coord, const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
	{ 
		Pointer obj(new Self()); 
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
		obj->m_Size.min(ptr->getSize() - buffer_coord); 
		obj->setMinPoint(minp, buffer_coord); 
		obj->setBuffer(ptr); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		return obj; 
	}
	
	static Pointer New(const typename BufferType::Pointer& ptr, const Point3D<int>& buffer_coord, const Point3D<int>& minp, const Point3D<int>& maxp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
	{ 
		Pointer obj(new Self()); 
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1); 
		obj->m_Size.min(ptr->getSize() - buffer_coord); 
		obj->setMinPoint(minp, buffer_coord); 
		obj->setBuffer(ptr); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setOrientationMatrix(matrix);
		return obj; 
	}

	static Pointer New(ValueType* ptr, bool own, const Point3D<int>& size, const Point3D<int>& minp=Point3D<int>(0,0,0), const Point3D<double>& spacing=Point3D<double>(1,1,1), const Point3D<double>& origin=Point3D<double>(0,0,0)) 
	{
		typename BufferType::Pointer buffer = BufferType::New(ptr, size.x(), size.y(), size.z(), own);
		return New(buffer, pcl::Point3D<int>(0,0,0), minp, minp+size-1, spacing, origin);
	}
	
	static Pointer New(ValueType* ptr, bool own, const Point3D<int>& size, const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) 
	{
		typename BufferType::Pointer buffer = BufferType::New(ptr, size.x(), size.y(), size.z(), own);
		return New(buffer, pcl::Point3D<int>(0,0,0), minp, minp+size-1, spacing, origin, matrix);
	}

	template <class ImagePointer>
	static Pointer New(const ImagePointer& ptr) 
	{
		Pointer result;
		if (Self::UseOrientationMatrix) result = New(Point3D<int>(0,0,0), ptr->getBufferSize()-1, ptr->getSpacing(), ptr->getOrigin(), ptr->getOrientationMatrix());
		else result = New(Point3D<int>(0,0,0), ptr->getBufferSize()-1, ptr->getSpacing(), ptr->getOrigin());
		result->m_Size = ptr->getSize();
		result->setMinPoint(ptr->getMinPoint(), ptr->toBufferCoordinate(ptr->getMinPoint()));
		return result;
	}
	
	static Pointer NewAlias(const Pointer& ptr, bool force_aliasing=true)
	{
		if (!force_aliasing) return ptr;
		return ptr->getAlias(ptr->getMinPoint(), false);
	}
	static ConstantPointer NewAlias(const ConstantPointer& ptr, bool force_aliasing=true)
	{
		if (!force_aliasing) return ptr;
		return ptr->getAlias(ptr->getMinPoint(), false);
	}

	static Pointer NewAlias(const typename Image<Type,!Flag,Buffer>::Pointer& ptr, bool force_aliasing=true)
	{
		pcl_UnreferencedParameter(force_aliasing);
		Pointer result(new Self);
		result->copyInfo(ptr);
		result->m_Buffer = ptr->getBuffer();
		return result;
	}
	static ConstantPointer NewAlias(const typename Image<Type,!Flag,Buffer>::ConstantPointer& ptr, bool force_aliasing=true)
	{
		pcl_UnreferencedParameter(force_aliasing);
		Pointer result(new Self);
		result->copyInfo(ptr);
		result->m_Buffer = boost::const_pointer_cast<Image<Type,!Flag,Buffer>>(ptr)->getBuffer();
		return result;
	}

	/******** Aliasing and subimage methods ********/
	typename Self::Pointer getAlias(const Point3D<int>& minp, bool fix_physical)
	{
		if (fix_physical) {
			return getAlias(minp, this->getSpacing(), 
				this->toPhysicalCoordinate(this->getMinPoint()) - this->toPhysicalVector(minp)
				);
		} else return getAlias(minp, this->getSpacing(), this->getOrigin());
	}
	typename Self::Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing)
	{
		return getAlias(minp, spacing, this->getOrigin());
	}
	typename Self::Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin)
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getSize(); 
		obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}
	typename Self::Pointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix)
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getSize(); 
		obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setBuffer(this->m_Buffer);
		obj->setOrientationMatrix(matrix);
		return obj;
	}
	
	typename Self::ConstantPointer getAlias(const Point3D<int>& minp, bool fix_physical) const
	{
		if (fix_physical) {
			return getAlias(minp, this->getSpacing(), 
				this->toPhysicalCoordinate(this->getMinPoint()) - this->toPhysicalVector(minp)
				);
		} else return getAlias(minp, this->getSpacing(), this->getOrigin());
	}
	typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing) const
	{
		return getAlias(minp, spacing, this->getOrigin());
	}
	typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin) const
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getSize(); 
		obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}
	typename Self::ConstantPointer getAlias(const Point3D<int>& minp, const Point3D<double>& spacing, const Point3D<double>& origin, const OrientationMatrixType& matrix) const
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getSize(); 
		obj->setMinPoint(minp, this->toBufferCoordinate(this->getMinPoint())); 
		obj->setSpacing(spacing); 
		obj->setOrigin(origin); 
		obj->setBuffer(this->m_Buffer);
		obj->setOrientationMatrix(matrix);
		return obj;
	}

	typename Self::Pointer getSubImage(const Point3D<int>& input_minp, const Point3D<int>& input_maxp)
	{
		Pointer obj(new Self());
		Point3D<int> minp(input_minp), maxp(input_maxp);
		minp.min(this->getMaxPoint()).max(this->getMinPoint());
		maxp.min(this->getMaxPoint()).max(this->getMinPoint());
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1);
		obj->setMinPoint(minp, this->toBufferCoordinate(minp));
		obj->setSpacing(this->getSpacing()); 
		obj->setOrigin(this->getOrigin()); 
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}
	typename Self::ConstantPointer getSubImage(const Point3D<int>& input_minp, const Point3D<int>& input_maxp) const
	{
		Pointer obj(new Self()); 
		Point3D<int> minp(input_minp), maxp(input_maxp);
		minp.min(this->getMaxPoint()).max(this->getMinPoint());
		maxp.min(this->getMaxPoint()).max(this->getMinPoint());
		obj->m_Size.set(maxp.x()-minp.x()+1, maxp.y()-minp.y()+1, maxp.z()-minp.z()+1);
		obj->setMinPoint(minp, this->toBufferCoordinate(minp)); 
		obj->setSpacing(this->getSpacing()); 
		obj->setOrigin(this->getOrigin());  
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}
	
	typename Self::Pointer getWholeImage()
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getBufferSize();
		Point3D<int> minp = this->getMinPoint();
		minp -= this->toBufferCoordinate(minp);
		obj->setMinPoint(minp, Point3D<int>(0,0,0));
		obj->setSpacing(this->getSpacing()); 
		obj->setOrigin(this->getOrigin()); 
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}
	typename Self::ConstantPointer getWholeImage() const
	{
		Pointer obj(new Self()); 
		obj->m_Size = this->getBufferSize();
		Point3D<int> minp = this->getMinPoint();
		minp -= this->toBufferCoordinate(minp);
		obj->setMinPoint(minp, Point3D<int>(0,0,0));
		obj->setSpacing(this->getSpacing()); 
		obj->setOrigin(this->getOrigin()); 
		obj->setBuffer(this->m_Buffer);
		if (Self::UseOrientationMatrix) obj->setOrientationMatrix(this->getOrientationMatrix(), this->getInverseOrientationMatrix());
		return obj;
	}

	/******** Reference retrieval methods (only use when dealing with objects, not guaranteed to exist) ********/
	inline ReferenceValueType at(int x, int y, int z)
	{
		return m_Buffer->at(this->toBufferCoordinate(x,y,z));
	}
	inline ReferenceValueType at(const Point3D<int>& p)
	{
		return this->at(p.x(), p.y(), p.z());
	}
	inline ReferenceValueType at(long index)
	{
		return m_Buffer->at(index);
	}

	/******** Value get methods ********/
	inline ConstantValueType get(int x, int y, int z) const
	{
		return m_Buffer->at(this->toBufferCoordinate(x,y,z));
	}
	inline ConstantValueType get(const Point3D<int>& p) const
	{
		return this->get(p.x(), p.y(), p.z());
	}
	inline ConstantValueType get(long index) const
	{
		return m_Buffer->at(index);
	}

	/******** Virtual methods from parent ********/
	using Parent::toIndex;
	virtual long toIndex(int x, int y, int z) const
	{
		return this->localToIndex(x,y,z);
	}

	virtual Point3D<int> toPoint(long index) const
	{
		return this->localToPoint(index);
	}

	virtual const Point3D<int>& getBufferSize() const
	{
		return m_Buffer->getSize();
	}

	virtual bool isType(const std::type_info& t) const
	{
		return t==typeid(ValueType);
	}
	
	/******** Local methods ********/
	typename BufferType::Pointer getBuffer()
	{
		return m_Buffer;
	}

	long localToIndex(const Point3D<int>& p) const
	{
		return this->localToIndex(p.x(), p.y(), p.z());
	}
	long localToIndex(int x, int y, int z) const
	{
		const Point3D<int> &p = this->toBufferCoordinate(x,y,z);
		return m_Buffer->toIndex(p.x(), p.y(), p.z());
	}

	Point3D<int> localToPoint(long index) const
	{
		return m_Buffer->toPoint(index) -= this->m_Offset;
	}

protected:
	typename BufferType::Pointer m_Buffer;

	Image() {}
	
	void setBuffer(const typename BufferType::Pointer& buffer)
	{
		m_Buffer = buffer;
		this->m_OffsetTable = m_Buffer->getOffsetTable();
	}