#ifndef __Darray_h_
#define __Darray_h_

/*+
** ======================================================================
**     COPYRIGHT NOTICE
**     UCLA Department of Radiological Sciences (c) 1997
** ======================================================================
** This software comprises unpublished confidential information of the
** University of California and may not be used, copied or made
** available to anyone, except with written permission of the
** Department of Radiological Sciences and Regents of the University
** of California.  All rights reserved.
**
** This software program and documentation are copyrighted by The
** Regents of the University of California. The software program and
** documentation are supplied "as is", without any accompanying
** services from The Regents. The Regents does not warrant that the
** operation of the program will be uninterrupted or error-free. The
** end-user understands that the program was developed for research
** purposes and is advised not to rely exclusively on the program for
** any reason.
**
** IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY
** PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
** DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
** SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
** CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. THE
** UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
** PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
** CALIFORNIA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
** UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
**
** ======================================================================
**
** for more information, or for permission to use this software for
** commercial or non-commercial purposes, please contact:
**
** Matthew S. Brown, Ph.D.
** Assistant Professor
** Department of Radiological Sciences
** Mail Stop172115
** UCLA Medical Center
** Los Angeles  CA 90024-1721
** 310-267-1820
** mbrown@mednet.ucla.edu
** ======================================================================
-*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <vector>
//using std::ostream;
//using std::cerr;

#ifdef _N
#undef _N
#endif

//using std::cerr;

/**
Template class for dynamically allocated arrays.
This class was designed for use within the ROI methods but can be used more generally.
Elements should only be accessed using the [] operator (data is not stored contiguously).
Storage is allocated in blocks of elements. The number of elements in a block is specified as the modulation factor.
Some member functions assume an ordered Darray, they require a comparison function as an argument. The comparison function must accept two arguments.
It must return an integer less than, equal to, or greater than 0 accordingly, if the first argument is to be considered less than, equal to, or greater than the second.
The application programmer must ensure that the Darray is ordered if these member functions are called.
*/
template<class T> class Darray {

public:

	/// Default constructor
	Darray() {};

	/// Constructor with the modulation factor supplied
	Darray(const long mod_fact);

	/**
	Copy constructor.
	Calls the copy constructor of T.
	*/
	Darray(const Darray<T>&);

  	/// Destructor
 	~Darray();

	/// Inserts an element at the start of the Darray
	void push_first(const T&);

	/// Appends an element to the end of the Darray
	void push_last(const T&);

	/**
	Inserts an element into the Darray at position ind.
	Program exits with error if ind is invalid.
	*/
	void push_here(const T&, const long ind);

	/// Assumes Darray is ordered; the second argument is a comparison function
	void push_inorder(const T&, int (*)(const T&, const T&));

	/**
	Deletes item at specified index (i).
	Program exits with error if index is invalid.
	*/
	void delete_item(const long i);

	/// Returns number of entries
	//const long N() const { return _N; };
	const long N() const { return _data.size(); };

	/// Returns the modulation factor of the Darray
	//const long mod() const { return _mod; };

	/**
	UNSAFE dereferencing operator.
	Index into the array is supplied as an argument.
	There is no checking of the validity of the index (except by assert).
	*/
	//const T& operator[] (const long n) const { assert((n>=0)&&(n<_N)); return *_data[_spos+1+n]; };
	const T& operator[] (const long n) const { assert((n>=0)&&(n<_data.size())); return _data[n]; };

	/**
	VERY UNSAFE dereferencing operator.
	Index into the array is supplied as an argument.
	There is no checking of the validity of the index (except by assert).
	Returned reference may be modified.
	*/
	//T& operator() (const long n) const { assert((n>=0)&&(n<_N)); return *_data[_spos+1+n]; };
	T& operator() (const long n) { assert((n>=0)&&(n<_data.size())); return _data[n]; };

	/**
	Finds an item (fdata) in a Darray (ordered using compar) and returns its index.
	@param	fdata	item to be found
	@param	compar	comparison function by which Darray is ordered
	@return	index of item if found, -1 otherwise
	*/
	const long find_item (const T& fdata, int (*compar)(const T&, const T&)) const;

	/**
	Similar to find_item, except if the item (fdata) cannot be found it is inserted.
	@param	fdata	item to be found or inserted
	@param	compar	comparison function by which Darray is ordered
	@param	found	set to 1 if item is found, 0 if item was inserted
	@return	index of the data element
	*/
	const long find_or_add(const T& fdata, int (*compar)(const T&, const T&), int& found);

	/**
	Similar to find_item, except if the item (fdata) cannot be found it is inserted.
	@param	fdata	item to be found or inserted
	@param	compar	comparison function by which Darray is ordered
	@return	index of the data element
	*/
	const long find_or_add(const T& fdata, int (*compar)(const T&, const T&));

	/// Removes all items from the array
	void clear();

private:

	/// Number of entries
      	//long _N;

	/// modulation factor
    	//long _mod;

	/// start position-1
   	//long _spos;

	/// end position
	//long _epos;

	/// size of data item (pointer to T)
    	//long _elmsize;

	/// total size of space allocated
    	//long _tsize;

	/// Increment for increase or decrease of tsize
    	//long _inc;

	/// data storage starts here
   	//T **_data;
	std::vector<T> _data;
};


/*
void my_memmove(void* dest, void* src, int len)
{
	char* tmp = new char [len];

	memcpy((char*)tmp, (char*)src, len);
	memcpy((char*)dest, (char*)tmp, len);

	delete [] tmp;
}
*/


template<class T> Darray<T>::Darray(const long mod_fact)
 	//: _N(0), _mod(mod_fact), _spos(-1), _epos(0), _tsize(0)
{
	//_elmsize = sizeof(T *);
	//_inc = _mod * _elmsize;
	//_data = (T **) malloc(_elmsize);
}



template<class T> Darray<T>::Darray(const Darray<T>& a)
{
	for (int i=0; i<a.N(); i++) 
        _data.push_back(a[i]); 

	/*
	int i;

	*this = a;
	_data = (T **) malloc(_tsize);
	//_data = (T **) calloc(1, _tsize);
	for(i=0; i<_N; i++) {
		T *tp = new T (a[i]);
		_data[i+_spos+1] = tp;
	}
	*/
}


template<class T> Darray<T>::~Darray()
{
	/*
	int i;

	for(i=_spos+1; i<_epos; i++) {
		delete _data[i];
		_data[i] = NULL;
	}

	free(_data);
	*/
}


template<class T> void Darray<T>::push_first(const T &ndata)
{
	_data.insert(_data.begin(), ndata);
	/*
	static int size;
	static T** ptr;

	if(_spos < 0) {
		_tsize += _inc;
		_data = (T **) realloc(_data, _tsize);

        	if(_data == NULL) {
			std::cerr << "ERROR: in darray: push_first\n";
 			exit(0);
        	}

        	size = (_N) * _elmsize;
        	ptr = _data;
		ptr += _mod;

       		if(size) {
			char* tmp = new char [size];
			memcpy((char*)tmp, (char*)_data, size);
			memcpy((char*)ptr, (char*)tmp, size);
			delete [] tmp;
		}
  		// memmove(ptr, _data, size) like memcpy, but handles overlaps

        	_epos += _mod;
        	_spos = _mod-1;
    	}
    	_data[_spos] = new T(ndata);
    	_spos--;
    	_N++;
		*/
}


template<class T> void Darray<T>::push_last(const T &ndata)
{
	_data.push_back(ndata);
	
	/*
	if (!(_epos % _mod)) {
        	_tsize += _inc;
        	_data = (T **) realloc(_data, _tsize);
        	if(_data == NULL) {
			std::cerr << "ERROR: in darray: push_last\n";
 			exit(0);
        	}
       	}

    	_data[_epos] = new T(ndata);
    	_N++;
    	_epos++;
		*/
}


template<class T> void Darray<T>::push_here(const T &ndata, const long ind)
{
	_data.insert(_data.begin()+ind, ndata);
	/*
	long size, rrow;
   	T **ptr;

   	if ((ind > _N) || (ind < 0)) {
		std::cerr << "ERROR: in darray: push_here\n";
 		exit(0);
    	}
    	rrow = ind + _spos + 1;

    	if (!(_epos % _mod)) {
        	_tsize += _inc;
        	_data = (T **) realloc(_data, _tsize);
        	if (_data==NULL){
			std::cerr << "ERROR: in darray: push_here\n";
 			exit(0);
        	}
    	}

    	size = (_epos - rrow) * _elmsize;
    	ptr = (T **)_data + rrow;

	char* tmp = new char [size];
	memcpy((char*)tmp, (char*)ptr, size);
	memcpy((char*)(ptr+1), (char*)tmp, size);
	delete [] tmp;
  	// memmove(ptr+1, ptr, size) like memcpy, but handles overlaps

    	*ptr = (T *)(new T(ndata));
    	_N++;
    	_epos++;
		*/
}


template<class T> void Darray<T>::push_inorder(const T &ndata, int (*compar)(const T &, const T &))
{
	static int low, high, mid, x;

	if(_data.empty()) {
    	_data.push_back(ndata);
        return;
    }

	low = 0; high = _data.size() - 1;
    while(low<=high) {
        mid = (low+high)>>1;
        x = (*compar)(ndata, _data[mid]);
        if(x < 0)
            high = mid - 1;
        else if(x > 0)
            low = mid + 1;
        else
            break;
    }
    if(low > mid)
        mid++;

    if(low == _data.size())
        _data.push_back(ndata);
	else
    	push_here(ndata, mid);


	/*
	static int low, high, mid, x;

	if(!_N) {
    		push_last(ndata);
        	return;
     	}

	low = _spos + 1; high = _epos - 1;
    	while(low<=high) {
        	mid = (low+high)>>1;
        	x = (*compar)(ndata, **(_data+mid));
        	if(x < 0)
            		high = mid - 1;
        	else if(x > 0)
            		low = mid + 1;
        	else
                	break;
    	}
    	if(low > mid)
        	mid++;

    	if(low == _epos)
        	push_last(ndata);
	else
    		push_here(ndata, mid - (_spos+1));
			*/
}

/*
template<class T> void Darray<T>::push_inorder_temp(const T &ndata, int (*compar)(const T &, const T &))
{
	static int low, high, mid, x;

	if(_data.empty()) {
    	_data.push_back(ndata);
        return;
    }

	low = 0; high = _data.size() - 1;
    while(low<=high) {
        mid = (low+high)>>1;
        x = (*compar)(ndata, _data[mid]);
        if(x < 0)
            high = mid - 1;
        else if(x > 0)
            low = mid + 1;
        else
            break;
    }
    if(low > mid)
        mid++;


std::cout << "push_inorder_temp: " << low << ", " << mid << ", " << high << std::endl;

    if(low == _data.size())
        _data.push_back(ndata);
	else
    	push_here(ndata, mid);
}
*/

template<class T> void Darray<T>::delete_item(const long i)
{
	_data.erase(_data.begin()+i);
	/*
    	static unsigned size;
    	static T ** ptr;

    	if((i >= _N) || (i < 0)) {
			std::cerr << "ERROR: in darray: delete_item: bounds\n";
			exit(0);
    	}
    	size = (_N - i - 1) * _elmsize;
    	ptr = (_data + i + _spos + 1);

		delete *ptr;

    	memcpy(ptr, ptr+1,size);
    	_N--;
    	_epos--;

    	if(!(_epos % _mod)){
        	_tsize -= _inc;

        	if (_tsize==0) {
				free(_data);
        		//_data = (T **) calloc(1, _elmsize);
				_data = (T **) malloc(_elmsize);
			}
			else
				_data = (T **) realloc(_data, _tsize);

        	if (!_data) {
				std::cerr << "ERROR: in darray: delete_item: realloc\n";
				exit(1);
        	}
    	}
		*/
}

template<class T> const long Darray<T>::find_item(const T& fdata, int (*compar)(const T&, const T&)) const
{
	static int low, high, mid, x;

    if(_data.empty())
        return -1;

    low = 0; high = _data.size() - 1;
    while(low<=high) {
        mid = (low+high)>>1;
        x = (*compar)(fdata, _data[mid]);
        if(x < 0)
            	high = mid - 1;
        else if(x > 0)
            	low = mid + 1;
        else
           	return mid;
    }
    return -1;

	/*
    	static int low, high, mid, x;

    	if(!_N)
        	return -1;

    	low = _spos + 1; high = _epos - 1;
    	while(low<=high) {
        	mid = (low+high)>>1;
        	x = (*compar)(fdata, **(_data+mid));
        	if(x < 0)
            		high = mid - 1;
        	else if(x > 0)
            		low = mid + 1;
        	else
           		return (mid - _spos - 1);
    	}
    	return -1;
		*/
}

template<class T> const long Darray<T>::find_or_add(const T& fdata, int (*compar)(const T&, const T&), int& found)
{
	static int low, high, mid, x;

    found = 0;

	high = _data.size() - 1;
    if(_data.empty()||((*compar)(fdata, _data[high])>0)) {
		push_last(fdata);
        return _data.size()-1;
	}

    low = 0; x = 1;
    while(low<high) {
        mid = (low+high)>>1;
        x = (*compar)(fdata, _data[mid]);
        if(x < 0)
            high = mid;
        else if(x > 0)
            low = mid + 1;
        else {
        	found = 1;
           	return mid;
        }
    }
	if ((*compar)(fdata, _data[high]) != 0)
		push_here(fdata, high);
	else {
	    found = 1;
	}

    return high;


	/*
    	static int low, high, mid, x;

    	found = 0;

	high = _epos - 1;
    	if(!_N||((*compar)(fdata, **(_data+high))>0)) {
		push_last(fdata);
        	return _N-1;
	}

    	low = _spos + 1; x = 1;
    	while(low<high) {
        	mid = (low+high)>>1;
        	x = (*compar)(fdata, **(_data+mid));
        	if(x < 0)
            		high = mid;
        	else if(x > 0)
            		low = mid + 1;
        	else {
        		found = 1;
           		return (mid - _spos - 1);
           	}
    	}
	if ((*compar)(fdata, **(_data+high)) != 0)
		push_here(fdata, high);
	else {
	        found = 1;
	}

    	return (high - _spos - 1);
		*/
}


template<class T> const long Darray<T>::find_or_add(const T & fdata, int (*compar)(const T &, const T &))
{
	int f;

	return find_or_add(fdata, compar, f);
}


template<class T> void Darray<T>::clear()
{
	_data.clear();
	/*
	for(int i=_spos+1; i<_epos; i++)
		delete _data[i];

	free(_data);

	_N = 0;
	_spos = -1;
	_epos = 0;
	_tsize = 0;

	_data = (T **) malloc(_elmsize);
	*/
}


#endif // !__Darray_h_

