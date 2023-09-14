#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTDLIB
#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#include "dcmtk/ofstd/ofstdinc.h"

#ifdef HAVE_GUSI_H
#include <GUSI.h>
#endif

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/dipijpeg.h"    /* for dcmimage JPEG plugin */
#include "dcmtk/dcmdata/dcrledrg.h"
#include "dcmtk/dcmdata/dcpxitem.h"
#include "dcmtk/dcmdata/dcuid.h"	   /* for generation of unique UID */ 	

#ifdef WITH_ZLIB
#include <zlib.h>      /* for zlibVersion() */
#endif

#include <pcl/exception.h>
#include <vector>

DcmTagKey getTagKey(const std::string& name)
{
	DcmTag tag;
	auto cond = DcmTag::findTagFromName(name.c_str(), tag);
	if (cond.bad()) {
		pcl_ThrowException(pcl::Exception(), "Cannot find tag \""+name+"\": "+cond.text());
	}
	return tag;
}

DcmTagKey getTagKey(int g, int e)
{
	return DcmTagKey(static_cast<Uint16>(g), static_cast<Uint16>(e));
}

DcmTag getTag(int g, int e)
{
	return DcmTag(DcmTagKey(static_cast<Uint16>(g), static_cast<Uint16>(e)));
}


class DicomItem
{
public:
	DicomItem(DcmItem* i)
	{
		m_Item = i;
		m_Own = false;
	}

	~DicomItem()
	{
		if (m_Own) delete m_Item;
	}

	DcmItem* getPointer()
	{
		return m_Item;
	}

	bool own() const
	{
		return m_Own;
	}

	void setOwn(bool o) 
	{
		m_Own = o;
	}

	unsigned long card() const
	{
		return m_Item->card();
	}

	void loadAllDataIntoMemory()
	{
		m_Item->loadAllDataIntoMemory();
	}

	DcmElement* get(const DcmTagKey& key)
	{
		DcmElement *elem;
		auto cond = m_Item->findAndGetElement(key, elem);
		if (cond==EC_Normal ) return elem;
		std::stringstream ss;
		ss << std::hex << key.getGroup() << "," << std::hex << key.getElement();
		pcl_ThrowException(pcl::Exception(), "Failed to retrieve tag ("+ss.str()+"): "+cond.text());
	}

	void set(const DcmTagKey& key, const std::string& val)
	{
		auto cond = m_Item->putAndInsertString(key, val.c_str());
		if (cond.bad()) {
			std::stringstream ss;
			ss << std::hex << key.getGroup() << "," << std::hex << key.getElement();
			pcl_ThrowException(pcl::Exception(), "Error occurred when putting string into ("+ss.str()+"): "+cond.text());
		}
	}

	void remove(const DcmTagKey& key)
	{
		auto cond = m_Item->findAndDeleteElement(key);
		if (cond.bad()) {
			std::stringstream ss;
			ss << std::hex << key.getGroup() << "," << std::hex << key.getElement();
			pcl_ThrowException(pcl::Exception(), "Error occurred when putting string into ("+ss.str()+"): "+cond.text());
		}
	}

	std::vector<DcmElement*>* getElements()
	{
		std::vector<DcmElement*> *ret_val = new std::vector<DcmElement*>();
		DcmStack stack;
		OFCondition status = m_Item->nextObject(stack, OFTrue);
		while(status.good()) {
			auto obj = stack.top();
			ret_val->push_back(static_cast<DcmElement*>(obj));
			status = m_Item->nextObject(stack, OFFalse);
		}
		return ret_val;
	}

protected:
	DcmItem *m_Item;
	bool m_Own;

	DicomItem() 
	{
		m_Own = false;
	}
};


class DicomPixelSequence
{
public:
	DicomPixelSequence(DcmPixelSequence *e)
	{
		m_Element = e;
	}

	DcmPixelSequence* getPointer()
	{
		return m_Element;
	}

	const DcmTag& getTag() const
	{
		return m_Element->getTag();
	}

	unsigned long card() const
	{
		return m_Element->card();
	}

	DcmElement* get(unsigned long num)
	{
		if (num>=this->card()) pcl_ThrowException(pcl::Exception(), "Invalid index provided!");
		DcmPixelItem *item;
		m_Element->getItem(item, num);
		return item;
	}
	
protected:
	DcmPixelSequence *m_Element;
};

class DicomSequence
{
public:
	DicomSequence(DcmSequenceOfItems* e)
	{
		m_Element = e;
	}

	DcmSequenceOfItems* getPointer()
	{
		return m_Element;
	}

	const DcmTag& getTag() const
	{
		return m_Element->getTag();
	}

	unsigned long card() const
	{
		return m_Element->card();
	}

	DcmItem* getItem(const unsigned long num)
	{
		if (num>=this->card()) pcl_ThrowException(pcl::Exception(), "Invalid index provided!");
		return m_Element->getItem(num);
	}

	void appendItem(DcmItem *item)
	{
		auto cond = m_Element->append(item);
		if (cond!=EC_Normal) {
			std::stringstream ss;
			ss << std::hex << getTag().getGTag() << "," << std::hex << getTag().getETag();
			pcl_ThrowException(pcl::Exception(), "Error appending item into ("+ss.str()+"):"+cond.text());
		}
	}

	bool removeItem(const unsigned long num)
	{
		DcmItem* item = m_Element->remove(num);
		if (item!=NULL) {
			delete item;
			return true;
		}
		return false;
	}

	DcmItem* removeItem(DcmItem* item)
	{
		return m_Element->remove(item);
	}

protected:
	DcmSequenceOfItems *m_Element;
};


class DicomMetaInfo : public DicomItem
{
	DicomMetaInfo()
	{
		this->m_Item = m_FileFormat.getMetaInfo();
	}

	DicomMetaInfo(const std::string& file)
	{
		auto error = m_FileFormat.loadFile(file.c_str());
		if (error.bad()) {
			pcl_ThrowException(pcl::Exception(), "Error occurred when opening \"" + file + "\": " + error.text());
		}
		this->m_Item = m_FileFormat.getMetaInfo();
	}

protected:
	DcmFileFormat m_FileFormat;
	void setOwn() {}
};

class DicomObject: public DicomItem
{
public:
	DicomObject()
	{
		this->m_Item = m_FileFormat.getDataset();
	}

	DicomObject(const std::string& file)
	{
		auto error = m_FileFormat.loadFile(file.c_str());
		if (error.bad()) {
			pcl_ThrowException(pcl::Exception(), "Error occurred when opening \""+file+"\": "+error.text());
		}
		this->m_Item = m_FileFormat.getDataset();
	}

	int getTransferSyntax()
	{
		return m_FileFormat.getDataset()->getOriginalXfer();
	}

	std::string getMediaStorageClassUID()
	{
		DcmItem* meta_info = m_FileFormat.getMetaInfo();

		DcmElement *elem;
		auto cond = meta_info->findAndGetElement(DcmTagKey(0x0002, 0x0002), elem);
		if (cond == EC_Normal) {
			OFString buffer;
			elem->getOFStringArray(buffer, 0);
			return std::string(buffer.c_str());
		}
		else 
			pcl_ThrowException(pcl::Exception(), "Failed to retrieve Media Storage SOP Class UID");
	}

	void write(const std::string& file, int ts)
	{
		E_TransferSyntax transfer_syntax = static_cast<E_TransferSyntax>(ts);
		DcmDataset *dataset = m_FileFormat.getDataset();

		if (transfer_syntax!=E_TransferSyntax::EXS_Unknown) {
			DcmXfer original_xfer(dataset->getOriginalXfer());
			DcmXfer target_xfer(transfer_syntax);
			bool error_occured = false;
			OFCondition error = EC_Normal;
			error = dataset->chooseRepresentation(transfer_syntax, NULL);
			if (error.bad()) {
				if (error == EJ_UnsupportedColorConversion) pcl_ThrowException(pcl::Exception(), "Unsupported color space conversion!");
				else pcl_ThrowException(pcl::Exception(), std::string("Input transfer syntax ") + original_xfer.getXferName() + " not supported!");
			} else {
				if (!dataset->canWriteXfer(transfer_syntax)) pcl_ThrowException(pcl::Exception(), std::string("Error: conversion to transfer syntax ") + target_xfer.getXferName() + " not possible!");
			}
		}

		OFCondition error = EC_Normal;
		error = m_FileFormat.saveFile(file.c_str(), transfer_syntax);
		if (error != EC_Normal) {
			pcl_ThrowException(pcl::Exception(), "Error occurred when writing file using given transfer syntax!");
		}
	}

protected:
	DcmFileFormat m_FileFormat;

	void setOwn() {}
};

char* getCharArrayFromElement(DcmElement* elem, const unsigned long num_chars)
{
	Uint8 *buffer;
	elem->getUint8Array(buffer);
	char* char_buffer = reinterpret_cast<char*>(buffer);

	char* element_data = new char[num_chars]();
	for (int ctr = 0; ctr < num_chars; ctr++) {
		element_data[ctr] = char_buffer[ctr];
	}
	return element_data;
}

void setElementCharArray(DcmElement* elem, char* val, const unsigned long num_chars)
{
	Uint8* buffer = reinterpret_cast<Uint8*>(val);
	auto cond = elem->putUint8Array (buffer, num_chars);
	if (cond!=EC_Normal) {
		pcl_ThrowException(pcl::Exception(), std::string("Error occurred when setting value: ")+cond.text());
	}
}

std::string getStringFromElement(DcmElement* elem)
{
	OFString buffer;
	elem->getOFStringArray(buffer, 0);
	return std::string(buffer.c_str());
}

void setElementString(DcmElement* elem, const char* val)
{
	auto cond = elem->putString(val);
	if (cond!=EC_Normal) {
		pcl_ThrowException(pcl::Exception(), std::string("Error occurred when setting value: ")+cond.text());
	}
}

DicomSequence* getDicomSequenceFromElement(DcmElement* elem)
{
	if (elem->ident()!=EVR_SQ) return NULL;
	return new DicomSequence(static_cast<DcmSequenceOfItems*>(elem));
}

DicomPixelSequence* getDicomPixelSequenceFromElement(DcmElement* elem)
{
	if (elem->ident()!=EVR_pixelSQ) return NULL;
	return new DicomPixelSequence(static_cast<DcmPixelSequence*>(elem));
}

DicomItem* getDicomItem(DcmItem* item)
{
	return new DicomItem(item);
}

std::string get_unique_UID(unsigned long level)
{
	// 0 is	image, 1 is series and 2 is study
	char* uid = new char[100]();
	if (level == 0)
		return std::string(dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT));
	if (level == 1)
		return std::string(dcmGenerateUniqueIdentifier(uid, SITE_SERIES_UID_ROOT));
	return std::string(dcmGenerateUniqueIdentifier(uid, SITE_STUDY_UID_ROOT));
}
