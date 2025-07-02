// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages one-off reader/writer usages for binary serialization

   -------------------------------------------------------------------------
   История:
   - 02:06:2010: Created by Kevin Kirst

*************************************************************************/

#ifndef __BINARYREADERWRITER_H__
#define __BINARYREADERWRITER_H__

#include <drx3D/Network/ISerializeHelper.h>

#include <drx3D/Act/SerializeWriterXMLCPBin.h>
#include <drx3D/Act/SerializeReaderXMLCPBin.h>
#include <drx3D/Act/XMLCPB_WriterInterface.h>
#include <drx3D/Act/XMLCPB_ReaderInterface.h>

class CBinarySerializedObject : public ISerializedObject
{
public:
	CBinarySerializedObject(tukk szSection);
	virtual ~CBinarySerializedObject();

	enum { GUID = 0xBDE84A9A };
	virtual u32 GetGUID() const { return GUID; }
	virtual void   GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void   AddRef()  { ++m_nRefCount; }
	virtual void   Release() { if (--m_nRefCount <= 0) delete this; }

	virtual bool   IsEmpty() const;
	virtual void   Reset();
	virtual void   Serialize(TSerialize& serialize);

	bool           FinishWriting(XMLCPB::CWriterInterface& Writer);
	bool           PrepareReading(XMLCPB::CReaderInterface& Reader);
	tukk    GetSectionName() const { return m_sSection.c_str(); }

private:
	void FreeData();

	string m_sSection;

	i32    m_nRefCount;
	u32 m_uSerializedDataSize;
	u8* m_pSerializedData;
};

class CBinarySerializeHelper : public ISerializeHelper
{
public:
	CBinarySerializeHelper();
	virtual ~CBinarySerializeHelper();

	virtual void                          GetMemoryUsage(IDrxSizer* pSizer) const;

	virtual void                          AddRef()  { ++m_nRefCount; }
	virtual void                          Release() { if (--m_nRefCount <= 0) delete this; }

	virtual _smart_ptr<ISerializedObject> CreateSerializedObject(tukk szSection);
	virtual bool                          Write(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL);
	virtual bool                          Read(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument = NULL);

private:
	static CBinarySerializedObject* GetBinarySerializedObject(ISerializedObject* pObject);

private:
	i32 m_nRefCount;
};

#endif //__BINARYREADERWRITER_H__
