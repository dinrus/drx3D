// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNCEDFILE_H__
#define __SYNCEDFILE_H__

#pragma once

#include <drx3D/Network/Config.h>

#if SERVER_FILE_SYNC_MODE

	#include <drx3D/Network/Whirlpool.h>

class CSyncedFile : public CMultiThreadRefCount
{
public:
	static u32k MAX_FILE_SIZE = 16384;
	static u32k HASH_BYTES = CWhirlpoolHash::DIGESTBYTES;

	CSyncedFile(const string& filename);
	~CSyncedFile();

	const CWhirlpoolHash* GetHash();
	u32                GetLength();
	bool                  FillData(u8* pBuffer, u32 bufLen);
	u8k*          GetData();

	bool                  LockData();
	void                  UnlockData();

	bool                  BeginUpdate();
	bool                  PutData(u8k* pData, u32 length);
	bool                  EndUpdate(bool success);

	const string&         GetFilename() const
	{
		return m_filename;
	}

private:
	enum EState
	{
		eS_Updating,
		eS_NotLoaded,
		eS_Loaded,
		eS_Hashed,
	};

	const string   m_filename;
	EState         m_state;
	CWhirlpoolHash m_hash;
	u32         m_length;
	u32         m_capacity;
	u8*         m_pData;
	i32            m_locks;

	bool EnterState(EState state);
	bool LoadData();
	bool HashData();
};

typedef _smart_ptr<CSyncedFile> CSyncedFilePtr;

class CSyncedFileDataLock
{
public:
	explicit CSyncedFileDataLock(CSyncedFilePtr pFile);
	CSyncedFileDataLock();
	CSyncedFileDataLock(const CSyncedFileDataLock& other);
	CSyncedFileDataLock& operator=(CSyncedFileDataLock other);
	void                 Swap(CSyncedFileDataLock& other);
	~CSyncedFileDataLock();

	bool Ok() const
	{
		return m_pData != 0;
	}

	u8k* GetData() const
	{
		return m_pData;
	}
	u32 GetLength() const
	{
		return m_length;
	}
	const string& GetFilename() const
	{
		return m_pFile->GetFilename();
	}

private:
	CSyncedFilePtr m_pFile;
	u8k*   m_pData;
	u32         m_length;
};

#endif // SERVER_FILE_SYNC_MODE

#endif
