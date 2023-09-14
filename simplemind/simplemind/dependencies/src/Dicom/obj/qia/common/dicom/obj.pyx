# distutils: language = c++

import os
from libcpp.string cimport string
from libcpp.vector cimport vector
from cython.operator cimport dereference as deref, preincrement 
from enum import Enum
import numpy as np
cimport numpy as np
 
np.import_array()
 
cdef extern from "pcl/exception/python.h":
    cdef void raisePyError()

cdef extern from "obj.h":
    cdef cppclass DcmTagKey:
        unsigned short getGroup() const
        unsigned short getElement() const
    cdef cppclass DcmTag:
        const char* getTagName()
        unsigned short getGTag() const
        unsigned short getETag() const
    cdef cppclass DcmElement:
        const DcmTag& getTag() const
        unsigned long getLength() const
    cdef cppclass DcmItem:
        pass
    cdef cppclass DcmSequenceOfItems:
        pass
    cdef cppclass DcmPixelSequence:
        pass
        
    DcmTagKey getTagKey(const string&) except +raisePyError
    DcmTagKey getTagKey(int,int)
    DcmTag getTag(int,int)
    
    DicomSequence* getDicomSequenceFromElement(DcmElement*)
    DicomPixelSequence* getDicomPixelSequenceFromElement(DcmElement*)
    string getStringFromElement(DcmElement*)
    char* getCharArrayFromElement(DcmElement*, const unsigned long)
    void setElementCharArray(DcmElement*, char*, const unsigned long)
    void setElementString(DcmElement*, const char*)
    string get_unique_UID(unsigned long)
    DicomItem* getDicomItem(DcmItem*)
    
    cdef cppclass DicomSequence:
        DcmSequenceOfItems* getPointer()
        unsigned long card() const
        const DcmTag& getTag() const
        DcmItem* getItem(const unsigned long)
        void appendItem(DcmItem *item)
        bint removeItem(const unsigned long)
        DcmItem* removeItem(DcmItem*)
    
    cdef cppclass DicomPixelSequence:
        DcmPixelSequence* getPointer()
        const DcmTag& getTag() const
        unsigned long card() const
        DcmElement* get(unsigned long)

    cdef cppclass DicomItem:
        DcmItem* getPointer()
        unsigned long card() const
        bint own() const
        void setOwn(bint) const
        void loadAllDataIntoMemory()
        DcmElement* get(DcmTagKey&) except +raisePyError
        void set(DcmTagKey&, string) except +raisePyError
        void remove(DcmTagKey&) except +raisePyError
        vector[DcmElement*]* getElements()
        
    cdef cppclass DicomObject:
        DicomObject()
        unsigned long card() const
        DicomObject(const string&) except +raisePyError
        int getTransferSyntax()
        string getMediaStorageClassUID() except +raisePyError 
        void write(const string&, int) except +raisePyError
        void loadAllDataIntoMemory()
        DcmElement* get(DcmTagKey&) except +raisePyError
        void set(DcmTagKey&, string) except +raisePyError
        void remove(DcmTagKey&) except +raisePyError
        vector[DcmElement*]* getElements()

class TransferSyntax(Enum):
    Unknown = -1
    LittleEndianImplicit = 0
    BigEndianImplicit = 1
    LittleEndianExplicit = 2
    BigEndianExplicit = 3
    JPEGProcess1TransferSyntax = 4
    JPEGProcess2_4TransferSyntax = 5
    JPEGProcess3_5TransferSyntax = 6
    JPEGProcess6_8TransferSyntax = 7
    JPEGProcess7_9TransferSyntax = 8
    JPEGProcess10_12TransferSyntax = 9
    JPEGProcess11_13TransferSyntax = 10
    JPEGProcess14TransferSyntax = 11
    JPEGProcess15TransferSyntax = 12
    JPEGProcess16_18TransferSyntax = 13
    JPEGProcess17_19TransferSyntax = 14
    JPEGProcess20_22TransferSyntax = 15
    JPEGProcess21_23TransferSyntax = 16
    JPEGProcess24_26TransferSyntax = 17
    JPEGProcess25_27TransferSyntax = 18
    JPEGProcess28TransferSyntax = 19
    JPEGProcess29TransferSyntax = 20
    JPEGProcess14SV1TransferSyntax = 21
    RLELossless = 22
    JPEGLSLossless = 23
    JPEGLSLossy = 24
    DeflatedLittleEndianExplicit = 25
    JPEG2000LosslessOnly = 26
    JPEG2000 = 27
    MPEG2MainProfileAtMainLevel = 28
    MPEG2MainProfileAtHighLevel = 29
    JPEG2000MulticomponentLosslessOnly = 30
    JPEG2000Multicomponent = 31
    JPIPReferenced = 32
    JPIPReferencedDeflate = 33
    
class Type(Enum):
    Item = 1
    Element = 2
    SequenceOfItems = 3
    PixelSequence = 4

_INVERSE_LOOKUP = {i.value:i for i in TransferSyntax}

cdef _get_tag_name(int g, int e):
    cdef DcmTag tag = getTag(g, e)
    return tag.getTagName().decode("utf-8")

cdef _find_tag_from_name(name):
    cdef DcmTagKey tag = getTagKey(name.encode("utf-8"))
    return (tag.getGroup(), tag.getElement())

class Tag:
    def __init__(self, g, e=None):
        if isinstance(g, str):
            self._name = g
            self._tag = _find_tag_from_name(g)
        else:
            self._tag = (g,e)
            self._name = None
    
    def __getattr__(self, name):
        if name=="group":
            return self._tag[0]
        elif name=="element":
            return self._tag[1]
        elif name=="name":
            if self._name is None:
                self._name = _get_tag_name(self._tag[0], self._tag[1])
            return self._name
        raise AttributeError(name)
    
    def __getitem__(self, i):
        return self._tag[i]

    def __iter__(self):
        return self._tag

    def __len__(self):
        return len(self._tag)
    
    def __str__(self):
        return ",".join([hex(i).replace("0x","").rjust(4,"0") for i in self._tag])

cdef class _Elem:
    cdef DcmElement* ptr
    
    def type(self):
        return Type.Element
        
    def __len__(self):
        return self.ptr.getLength()
    
    def __getattr__(self, name):
        cdef np.npy_intp[1] dims
        cdef void* temp_char_array
        if name=="value":
            temp = getStringFromElement(self.ptr)
            return temp.c_str().decode("utf-8")
        elif name=="byte_array":
            temp_char_array = getCharArrayFromElement(self.ptr, self.ptr.getLength())
            dims[0] = <np.npy_intp>(<int>(self.ptr.getLength()))
            return np.PyArray_SimpleNewFromData(1, dims, np.NPY_INT8, <void*>temp_char_array)
        elif name=="tag":
            tag = self.ptr.getTag()
            return Tag(tag.getGTag(), tag.getETag())
        raise AttributeError(name)
    
    def __setattr__(self, name, val):
        cdef np.ndarray[np.int8_t, ndim=1, mode = 'c'] np_buff
        if name=="value":
            if not isinstance(val, str):
                try:
                    val = "\\".join([str(i) for i in val])
                except:
                    val = str(val)
            setElementString(self.ptr, val.encode("utf-8"))
        elif name=="byte_array":
            if isinstance(val, np.ndarray):
                np_buff = np.ascontiguousarray(val, dtype=np.int8)
                setElementCharArray(self.ptr, <char*> np_buff.data, len(val))
            else:
                raise RuntimeError("Value is not Numpy Array!")
        else:
            raise AttributeError(name)

cdef _get_object(DcmElement* elem):
    cdef DicomSequence* seq = getDicomSequenceFromElement(elem)
    cdef DicomPixelSequence* pseq = getDicomPixelSequenceFromElement(elem)
    if seq is not NULL:
        ret_seq = _Seq()
        ret_seq.ptr = seq
        return ret_seq
    elif pseq is not NULL:
        ret_pseq = _PixelSeq()
        ret_pseq.ptr = pseq
        return ret_pseq
    else:
        ret_elem = _Elem()
        ret_elem.ptr = elem
        return ret_elem

cdef class _ElementListIterator:
    cdef vector[DcmElement*] *ptr
    cdef vector[DcmElement*].iterator end
    cdef vector[DcmElement*].iterator iter
    
    def __dealloc__(self):
        del self.ptr
        
    def _begin(self):
        self.end = deref(self.ptr).end()
        self.iter = deref(self.ptr).begin()
        
    def __iter__(self):
        return self
        
    def __next__(self):
        if self.iter!=self.end:
            ret_obj = _get_object(deref(self.iter))
            preincrement(self.iter)
            return ret_obj
        else:
            raise StopIteration()


cdef class _Item:
    cdef DicomItem* ptr
    
    def __dealloc__(self):
        del self.ptr
        
    def __len__(self):
        return self.ptr.card()
    
    def type(self):
        return Type.Item

    def load_all_data_into_memory(self):
        self.ptr.loadAllDataIntoMemory()
        
    def __getitem__(self, tag):
        if isinstance(tag, str):
            return _get_object(self.ptr.get(getTagKey(tag.encode("utf-8"))))
        else:
            return _get_object(self.ptr.get(getTagKey(tag[0], tag[1])))
        
    def get(self, tag):
        try:
            return self[tag]
        except:
            return None

    def __setitem__(self, tag, val):
        if not isinstance(val, str):
            try:
                val = "\\".join([str(i) for i in val])
            except:
                val = str(val)
        if isinstance(tag, str):
            self.ptr.set(getTagKey(tag.encode("utf-8")), val.encode("utf-8"))
        else:
            self.ptr.set(getTagKey(tag[0], tag[1]), val.encode("utf-8"))
            
    def set(self, tag, val):
        try:
            self[tag] = val
            return True
        except:
            return False

    def __delitem__(self, tag):
        if isinstance(tag, str):
            self.ptr.remove(getTagKey(tag.encode("utf-8")))
        else:
            self.ptr.remove(getTagKey(tag[0], tag[1]))
            
    def remove(self, tag):
        try:
            del self[tag]
            return True
        except:
            return False
            
    def tags(self):
        cdef vector[DcmElement*] *cres = self.ptr.getElements()
        cdef vector[DcmElement*].iterator end = deref(cres).end()
        cdef vector[DcmElement*].iterator iter = deref(cres).begin()
        result = []
        while iter!=end:
            result.append(Tag(deref(iter).getTag().getGTag(), deref(iter).getTag().getETag()))
            preincrement(iter)
        del cres
        return result
        
    def __iter__(self):
        iter = _ElementListIterator()
        iter.ptr = self.ptr.getElements()
        iter._begin()
        return iter

class _SequenceIter:
    def __init__(self, source):
        self._source = source
        self._index = 0
        self._end = len(source)
        
    def __iter__(self):
        return self
        
    def __next__(self):
        if self._index>=self._end:
            raise StopIteration
        ret_obj = self._source[self._index]
        self._index += 1
        return ret_obj

        
cdef class _PixelSeq:
    cdef DicomPixelSequence* ptr
    
    def __dealloc__(self):
        del self.ptr
        
    def __len__(self):
        return self.ptr.card()
        
    def type(self):
        return Type.PixelSequence
        
    def __getattr__(self, name):
        if name=="tag":
            tag = self.ptr.getTag()
            return Tag(tag.getGTag(), tag.getETag())
        raise AttributeError(name)
        
    def __getitem__(self, unsigned long index):
        cdef DcmElement* item = self.ptr.get(index)
        if item is NULL:
            raise RuntimeError("NULL pointer returned!")
        else:
            ret_obj = _Elem()
            ret_obj.ptr = item
            return ret_obj
            
    def get(self, unsigned long index):
        try:
            return self[index]
        except:
            return None
            
    def __iter__(self):
        return _SequenceIter(self)


cdef class _Seq:
    cdef DicomSequence* ptr
    
    def __dealloc__(self):
        del self.ptr
        
    def __len__(self):
        return self.ptr.card()
        
    def type(self):
        return Type.SequenceOfItems
        
    def __getattr__(self, name):
        if name=="tag":
            tag = self.ptr.getTag()
            return Tag(tag.getGTag(), tag.getETag())
        elif name=="value":
            return self
        raise AttributeError(name)
        
    def __getitem__(self, unsigned long index):
        cdef DcmItem* item = self.ptr.getItem(index)
        if item is NULL:
            raise RuntimeError("NULL pointer returned!")
        else:
            ret_obj = _Item()
            ret_obj.ptr = getDicomItem(item)
            return ret_obj
            
    def get(self, unsigned long index):
        try:
            return self[index]
        except:
            return None
            
    def __delitem__(self, item):
        error = False
        if type(item) is _Item:
            temp = self.ptr.removeItem((<_Item>item).ptr.getPointer())
            if temp is not NULL:
                (<_Item>item).ptr.setOwn(True)
            else:
                error = True
        else:
            if not self.ptr.removeItem((<unsigned long>item)):
                error = True
        if error:
            raise SystemError("Unable to delete using "+repr(item))
                
    def remove(self, item):
        try:
            del self[item]
            return True
        except:
            return False
            
    def append(self, _Item item):
        self.ptr.appendItem(item.ptr.getPointer())
        item.ptr.setOwn(False)
        
    def __iter__(self):
        return _SequenceIter(self)


cdef class _Obj:
    cdef DicomObject *ptr
    cdef public object _source_file

    def __dealloc__(self):
        del self.ptr
        
    def __len__(self):
        return self.ptr.card()
        
    def source_file(self):
        return self._source_file
        
    def load_all_data_into_memory(self):
        self.ptr.loadAllDataIntoMemory()

    def get_transfer_syntax(self):
        return _INVERSE_LOOKUP[self.ptr.getTransferSyntax()]

    def get_media_storage_sop_class_uid(self):
        return self.ptr.getMediaStorageClassUID().decode("utf-8")

    def __getitem__(self, tag):
        if isinstance(tag, str):
            return _get_object(self.ptr.get(getTagKey(tag.encode("utf-8"))))
        else:
            return _get_object(self.ptr.get(getTagKey(tag[0], tag[1])))
        
    def get(self, tag):
        try:
            return self[tag]
        except:
            return None

    def __setitem__(self, tag, val):
        if not isinstance(val, str):
            try:
                val = "\\".join([str(i) for i in val])
            except:
                val = str(val)
        if isinstance(tag, str):
            self.ptr.set(getTagKey(tag.encode("utf-8")), val.encode("utf-8"))
        else:
            self.ptr.set(getTagKey(tag[0], tag[1]), val.encode("utf-8"))
            
    def set(self, tag, val):
        try:
            self[tag] = val
            return True
        except:
            return False

    def __delitem__(self, tag):
        if isinstance(tag, str):
            self.ptr.remove(getTagKey(tag.encode("utf-8")))
        else:
            self.ptr.remove(getTagKey(tag[0], tag[1]))
            
    def remove(self, tag):
        try:
            del self[tag]
            return True
        except:
            return False
            
    def write(self, file, ts=TransferSyntax.Unknown):
        if self._source_file==os.path.abspath(file):
            self.load_all_data_into_memory()
        self.ptr.write(file.encode("utf-8"), ts.value)
    
    def tags(self):
        cdef vector[DcmElement*] *cres = self.ptr.getElements()
        cdef vector[DcmElement*].iterator end = deref(cres).end()
        cdef vector[DcmElement*].iterator iter = deref(cres).begin()
        result = []
        while iter!=end:
            result.append(Tag(deref(iter).getTag().getGTag(), deref(iter).getTag().getETag()))
            preincrement(iter)
        del cres
        return result
        
    def __iter__(self):
        iter = _ElementListIterator()
        iter.ptr = self.ptr.getElements()
        iter._begin()
        return iter

def new():
    ret_obj = _Obj()
    ret_obj.ptr = new DicomObject()
    return ret_obj

def read(file):
    ret_obj = _Obj()
    ret_obj.ptr = new DicomObject(file.encode("utf-8"))
    ret_obj._source_file = os.path.abspath(file)
    return ret_obj
    
def new_item():
    ret_obj = _Item()
    ret_obj.ptr = getDicomItem(new DcmItem())
    ret_obj.ptr.setOwn(True)
    return ret_obj

def get_unique_uid(unsigned long level):
    # 0 is	image, 1 is series and 2 is study
    return get_unique_UID(level).c_str().decode("utf-8")
