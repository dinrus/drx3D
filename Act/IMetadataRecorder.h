// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IMETADATARECORDER_H__
#define __IMETADATARECORDER_H__

#pragma once

struct IMetadata
{
	static IMetadata*        CreateInstance();
	void                     Delete();

	virtual void             SetTag(u32 tag) = 0;
	virtual bool             SetValue(u32 type, u8k* data, u8 size) = 0;
	virtual bool             AddField(const IMetadata* metadata) = 0;
	virtual bool             AddField(u32 tag, u32 type, u8k* data, u8 size) = 0;

	virtual u32           GetTag() const = 0;
	virtual size_t           GetNumFields() const = 0; // 0 means this is a basic typed value
	virtual const IMetadata* GetFieldByIndex(size_t i) const = 0;
	virtual u32           GetValueType() const = 0;
	virtual u8            GetValueSize() const = 0;
	virtual bool             GetValue(u8* data /*[out]*/, u8* size /*[in|out]*/) const = 0;

	virtual IMetadata*       Clone() const = 0;

	virtual void             Reset() = 0;

protected:
	virtual ~IMetadata() {}
};

// This interface should be implemented by user of IMetadataRecorder.
struct IMetadataListener
{
	virtual ~IMetadataListener(){}
	virtual void OnData(const IMetadata* metadata) = 0;
};

// Records toplevel metadata - everything being recorded is added to the toplevel in a sequential manner.
struct IMetadataRecorder
{
	static IMetadataRecorder* CreateInstance();
	void                      Delete();

	virtual bool              InitSave(tukk filename) = 0;
	virtual bool              InitLoad(tukk filename) = 0;

	virtual void              RecordIt(const IMetadata* metadata) = 0;
	virtual void              Flush() = 0;

	virtual bool              Playback(IMetadataListener* pListener) = 0;

	virtual void              Reset() = 0;

protected:
	virtual ~IMetadataRecorder() {}
};

template<typename I>
class CSimpleAutoPtr
{
public:
	CSimpleAutoPtr() { m_pI = I::CreateInstance(); }
	~CSimpleAutoPtr() { m_pI->Delete(); }
	I*       operator->() const { return m_pI; }
	const I* get() const        { return m_pI; }
private:
	CSimpleAutoPtr(const CSimpleAutoPtr& rhs);
	CSimpleAutoPtr& operator=(const CSimpleAutoPtr& rhs);
	I* m_pI;
};

typedef CSimpleAutoPtr<IMetadata>         IMetadataPtr;
typedef CSimpleAutoPtr<IMetadataRecorder> IMetadataRecorderPtr;

#endif
