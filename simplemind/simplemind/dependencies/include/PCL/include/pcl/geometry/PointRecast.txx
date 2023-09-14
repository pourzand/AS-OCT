/************************ Math related ************************/

inline Self& negate()
{
	Parent::negate();
	return *this;
}

template <class T>
inline Self& operator+=(const T* val) 
{
	Parent::operator+=(val);
	return *this;
}
template <class T>
inline Self& operator+=(const T& val) 
{
	Parent::operator+=(val);
	return *this;
}

template <class T>
inline Self& operator-=(const T* val) 
{
	Parent::operator-=(val);
	return *this;
}
template <class T>
inline Self& operator-=(const T& val) 
{
	Parent::operator-=(val);
	return *this;
}

template <class T>
inline Self& operator*=(const T* val) 
{
	Parent::operator*=(val);
	return *this;
}
template <class T>
inline Self& operator*=(const T& val) 
{
	Parent::operator*=(val);
	return *this;
}

template <class T>
inline Self& operator/=(const T* val) 
{
	Parent::operator/=(val);
	return *this;
}
template <class T>
inline Self& operator/=(const T& val) 
{
	Parent::operator/=(val);
	return *this;
}

template <class T>
inline Self operator+(const T* val) const 
{ 
	return Self(*this)+=val;
}
template <class T>
inline Self operator+(const T& val) const 
{ 
	return Self(*this)+=val; 
}

template <class T>
inline Self operator-(const T* val) const 
{ 
	return Self(*this)-=val;
}
template <class T>
inline Self operator-(const T& val) const 
{ 
	return Self(*this)-=val; 
}

template <class T>
inline Self operator*(const T* val) const 
{ 
	return Self(*this)*=val;
}
template <class T>
inline Self operator*(const T& val) const 
{ 
	return Self(*this)*=val; 
}

template <class T>
inline Self operator/(const T* val) const 
{ 
	return Self(*this)/=val; 
}
template <class T>
inline Self operator/(const T& val) const 
{ 
	return Self(*this)/=val; 
}

inline Self operator-() const 
{
	return Self(*this).negate(); 
}

inline Self& max(const Self& p) 
{
	Parent::max(p);
	return *this;
}
inline Self& min(const Self& p) 
{
	Parent::min(p);
	return *this;
}

inline Self& normalize() 
{
	Parent::normalize();
	return *this;
}
inline Self getNormalized() const
{
	return Self(*this).normalize();
}

/************************ Data assignment related ************************/

template <class PT>
inline Self& operator=(const PT& val)
{
	Parent::operator=(val);
	return *this;
}

template <class T>
inline Self& operator=(const T* arr) 
{
	Parent::operator=(arr);
	return *this;
}

template <class T>
inline Self& set(const T& val)
{
	Parent::set(val);
	return *this;
}

template <class PT>
inline Self& assign(const PT& p, ValueType trailing_val=0)
{
	Parent::assign(p,trailing_val);
	return *this;
}

template <class PT>
inline Self& assignRound(const PT& p, ValueType trailing_val=0)
{
	Parent::assignRound(p,trailing_val);
	return *this;
}
template <class PT, class F>
inline Self& assignCustom(const PT& p, F func, int num=Self::Dimension, ValueType trailing_val=0)
{
	Parent::assignCustom(p,func,num,trailing_val);
	return *this;
}

template <class T>
inline Self& assign(const T* p, int num=Self::Dimension, ValueType trailing_val=0)
{
	Parent::assign(p,num,trailing_val);
	return *this;
}
template <class T>
inline Self& assignRound(const T* p, int num=Self::Dimension, ValueType trailing_val=0)
{
	Parent::assignRound(p,num,trailing_val);
	return *this;
}
template <class T, class F>
inline Self& assignCustom(const T* p, F func, int num=Self::Dimension, ValueType trailing_val=0)
{
	Parent::assignCustom(p,func,num,trailing_val);
	return *this;
}
