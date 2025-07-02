// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/SyncedFilePak.h>

#if SERVER_FILE_SYNC_MODE

	#include  <drx3D/Network/SyncedFileSet.h>

class CSyncedFilePak::CNonSyncedManip : public CSyncedFilePak::IFileManip
{
public:
	explicit CNonSyncedManip(FILE* f) : m_file(f)
	{
		NET_ASSERT(f);
	}
	~CNonSyncedManip()
	{
		gEnv->pDrxPak->FClose(m_file);
	}

	size_t ReadRaw(uk data, size_t length, size_t elems)
	{
		return gEnv->pDrxPak->FReadRaw(data, length, elems, m_file);
	}

	size_t Seek(long seek, i32 mode)
	{
		return gEnv->pDrxPak->FSeek(m_file, seek, mode);
	}

	size_t Tell()
	{
		return gEnv->pDrxPak->FTell(m_file);
	}

	i32 Eof()
	{
		return gEnv->pDrxPak->FEof(m_file);
	}

private:
	FILE* m_file;
};

class CSyncedFilePak::CSyncedManip : public CSyncedFilePak::IFileManip
{
public:
	explicit CSyncedManip(const CSyncedFileDataLock& lk) : m_lk(lk), m_cursor(0) {}

	size_t ReadRaw(uk data, size_t length, size_t elems)
	{
		u32 elemsleft = (m_lk.GetLength() - m_cursor) / elems;
		u32 elemsread = std::min(length, (size_t)elemsleft);
		u32 bytesread = elemsread * elems;
		memcpy(data, m_lk.GetData() + m_cursor, bytesread);
		m_cursor += bytesread;
		return elemsread;
	}

	size_t Seek(long seek, i32 mode)
	{
		u32 setpos = -1;
		switch (mode)
		{
		case SEEK_SET:
			setpos = seek;
			break;
		case SEEK_CUR:
			setpos = m_cursor + seek;
			break;
		case SEEK_END:
			setpos = m_lk.GetLength() - seek;
			break;
		}
		if (setpos < m_lk.GetLength())
		{
			m_cursor = setpos;
			return 0;
		}
		else
		{
			return 1;
		}
	}

	size_t Tell()
	{
		return m_cursor;
	}

	i32 Eof()
	{
		return (m_cursor >= m_lk.GetLength()) ? 1 : 0;
	}

private:
	CSyncedFileDataLock m_lk;
	u32              m_cursor;
};

tukk CSyncedFilePak::AdjustFileName(tukk src, char dst[g_nMaxPath], unsigned nFlags, bool* bFoundInPak /*=NULL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

bool CSyncedFilePak::Init(tukk szBasePath)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

void CSyncedFilePak::Release()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

bool CSyncedFilePak::OpenPack(tukk pName, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::OpenPack(tukk pBindingRoot, tukk pName, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::ClosePack(tukk pName, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::OpenPacks(tukk pWildcard, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::OpenPacks(tukk pBindingRoot, tukk pWildcard, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::ClosePacks(tukk pWildcard, unsigned nFlags /*= FLAGS_PATH_REAL*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

void CSyncedFilePak::AddMod(tukk szMod)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::RemoveMod(tukk szMod)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::ParseAliases(tukk szCommandLine)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::SetAlias(tukk szName, tukk szAlias, bool bAdd)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

tukk CSyncedFilePak::GetAlias(tukk szName, bool bReturnSame /*=true*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

void CSyncedFilePak::Lock()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::Unlock()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::SetGameFolder(tukk szFolder)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

tukk CSyncedFilePak::GetGameFolder() const
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

IDrxPak::PakInfo* CSyncedFilePak::GetPakInfo()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

void CSyncedFilePak::FreePakInfo(PakInfo*)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

FILE* CSyncedFilePak::FOpen(tukk pName, tukk mode, unsigned nFlags /*= 0*/)
{
	if (0 == strcmp("r", mode) || 0 == strcmp("rb", mode))
	{
		IFileManip* file = 0;
		if (CSyncedFilePtr pSyncedFile = m_fileset.GetSyncedFile(pName))
		{
			CSyncedFileDataLock flk(pSyncedFile);
			if (flk.Ok())
				file = new CSyncedManip(flk);
		}
		else
		{
			FILE* oldfile = gEnv->pDrxPak->FOpen(pName, mode, nFlags);
			if (oldfile)
				file = new CNonSyncedManip(oldfile);
		}
		return reinterpret_cast<FILE*>(file);
	}
	else
	{
		DrxFatalError("%s not implemented for mode '%s'", __FUNCTION__, mode);
		return NULL;
	}
}

FILE* CSyncedFilePak::FOpen(tukk pName, tukk mode, tuk szFileGamePath, i32 nLen)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return NULL;
}

size_t CSyncedFilePak::FReadRaw(uk data, size_t length, size_t elems, FILE* handle)
{
	return reinterpret_cast<IFileManip*>(handle)->ReadRaw(data, length, elems);
}

size_t CSyncedFilePak::FReadRawAll(uk data, size_t nFileSize, FILE* handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

uk CSyncedFilePak::FGetCachedFileData(FILE* handle, size_t& nFileSize)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

size_t CSyncedFilePak::FWrite(ukk data, size_t length, size_t elems, FILE* handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::FPrintf(FILE* handle, tukk format, ...)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

tuk CSyncedFilePak::FGets(tuk, i32, FILE*)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::Getc(FILE*)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

size_t CSyncedFilePak::FGetSize(FILE* f)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

size_t CSyncedFilePak::FGetSize(tukk pName)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::Ungetc(i32 c, FILE*)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

bool CSyncedFilePak::IsInPak(FILE* handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::RemoveFile(tukk pName)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::RemoveDir(tukk pName, bool bRecurse)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::IsAbsPath(tukk pPath)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::CopyFileOnDisk(tukk source, tukk dest, bool bFailIfExist)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

size_t CSyncedFilePak::FSeek(FILE* handle, long seek, i32 mode)
{
	return reinterpret_cast<IFileManip*>(handle)->Seek(seek, mode);
}

long CSyncedFilePak::FTell(FILE* handle)
{
	return reinterpret_cast<IFileManip*>(handle)->Tell();
}

i32 CSyncedFilePak::FClose(FILE* handle)
{
	delete reinterpret_cast<IFileManip*>(handle);
	return 0;
}

i32 CSyncedFilePak::FEof(FILE* handle)
{
	return reinterpret_cast<IFileManip*>(handle)->Eof();
}

i32 CSyncedFilePak::FError(FILE* handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::FGetErrno()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::FFlush(FILE* handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

uk CSyncedFilePak::PoolMalloc(size_t size)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

void CSyncedFilePak::PoolFree(uk p)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

intptr_t CSyncedFilePak::FindFirst(tukk pDir, _finddata_t* fd, u32 nFlags /*=0 */)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::FindNext(intptr_t handle, _finddata_t* fd)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::FindClose(intptr_t handle)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

IDrxPak::FileTime CSyncedFilePak::GetModificationTime(FILE* f)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

bool CSyncedFilePak::IsFileExist(tukk sFilename)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

bool CSyncedFilePak::MakeDir(tukk szPath, bool bGamePathMapping /*=false */)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

IDrxArchive* CSyncedFilePak::OpenArchive(tukk szPath, u32 nFlags /*=0*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

tukk CSyncedFilePak::GetFileArchivePath(FILE* f)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::RawCompress(ukk pUncompressed, u64* pDestSize, uk pCompressed, u64 nSrcSize, i32 nLevel /*= -1*/)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

i32 CSyncedFilePak::RawUncompress(uk pUncompressed, u64* pDestSize, ukk pCompressed, u64 nSrcSize)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

void CSyncedFilePak::RecordFileOpen(const ERecordFileOpenList eList)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::RecordFile(FILE* in, tukk szFilename)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

IResourceList* CSyncedFilePak::GetResourceList(const ERecordFileOpenList eList)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

IDrxPak::ERecordFileOpenList CSyncedFilePak::GetRecordFileOpenList()
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return RFOM_Disabled;
}

void CSyncedFilePak::Notify(ENotifyEvent event)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

u32 CSyncedFilePak::ComputeCRC(tukk szPath)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return 0;
}

bool CSyncedFilePak::ComputeMD5(tukk szPath, u8* md5)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

void CSyncedFilePak::RegisterFileAccessSink(IDrxPakFileAcesssSink* pSink)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

void CSyncedFilePak::UnregisterFileAccessSink(IDrxPakFileAcesssSink* pSink)
{
	DrxFatalError("%s not implemented", __FUNCTION__);
}

bool CSyncedFilePak::GetLvlResStatus() const
{
	DrxFatalError("%s not implemented", __FUNCTION__);
	return false;
}

#endif
