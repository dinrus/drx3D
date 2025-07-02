// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages one-off reader/writer usages for Xml serialization

   -------------------------------------------------------------------------
   История:
   - 02:06:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __XMLREADERWRITER_H__
#define __XMLREADERWRITER_H__

#include <drx3D/Network/ISerializeHelper.h>

class CXmlSerializedObject : public ISerializedObject
{
public:
	CXmlSerializedObject(tukk szSection);
	virtual ~CXmlSerializedObject() {}

	enum { GUID = 0xD6BEE847 };
	virtual u32    GetGUID() const { return GUID; }
	virtual void      GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void      AddRef()  { ++m_nRefCount; }
	virtual void      Release() { if (--m_nRefCount <= 0) delete this; }

	virtual bool      IsEmpty() const;
	virtual void      Reset();
	virtual void      Serialize(TSerialize& serialize);

	void              CreateRootNode();

	XmlNodeRef&       GetXmlNode()       { return m_XmlNode; }
	const XmlNodeRef& GetXmlNode() const { return m_XmlNode; }

private:
	string     m_sTag;
	XmlNodeRef m_XmlNode;
	i32        m_nRefCount;
};

class CXmlSerializeHelper : public ISerializeHelper
{
public:
	CXmlSerializeHelper();
	virtual ~CXmlSerializeHelper();

	virtual void                          GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void                          AddRef()  { ++m_nRefCount; }
	virtual void                          Release() { if (--m_nRefCount <= 0) delete this; }

	virtual _smart_ptr<ISerializedObject> CreateSerializedObject(tukk szSection);
	virtual bool                          Write(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL);
	virtual bool                          Read(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL);

	// Local versions which work with XmlNodes directly
	ISerialize* GetWriter(XmlNodeRef& node);
	ISerialize* GetReader(XmlNodeRef& node);

private:
	static CXmlSerializedObject* GetXmlSerializedObject(ISerializedObject* pObject);

private:
	i32                        m_nRefCount;
	_smart_ptr<IXmlSerializer> m_pSerializer;
};

#endif //__XMLREADERWRITER_H__
