// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SYNCEDFILEPAK_H__
#define __SYNCEDFILEPAK_H__

#include <drx3D/Network/Config.h>

#pragma once

#if SERVER_FILE_SYNC_MODE

class CSyncedFileSet;

// wrapper around IDrxPak
// supports just enough to make CDrxFile work!
class CSyncedFilePak : public IDrxPak
{
public:
	explicit CSyncedFilePak(CSyncedFileSet& fileset) : m_fileset(fileset) {}

	virtual tukk                  AdjustFileName(tukk src, char dst[g_nMaxPath], unsigned nFlags, bool* bFoundInPak);
	virtual bool                         Init(tukk szBasePath);
	virtual void                         Release();
	virtual bool                         OpenPack(tukk pName, unsigned nFlags);
	virtual bool                         OpenPack(tukk pBindingRoot, tukk pName, unsigned nFlags);
	virtual bool                         ClosePack(tukk pName, unsigned nFlags);
	virtual bool                         OpenPacks(tukk pWildcard, unsigned nFlags);
	virtual bool                         OpenPacks(tukk pBindingRoot, tukk pWildcard, unsigned nFlags);
	virtual bool                         ClosePacks(tukk pWildcard, unsigned nFlags);
	virtual void                         AddMod(tukk szMod);
	virtual void                         RemoveMod(tukk szMod);
	virtual void                         ParseAliases(tukk szCommandLine);
	virtual void                         SetAlias(tukk szName, tukk szAlias, bool bAdd);
	virtual tukk                  GetAlias(tukk szName, bool bReturnSame);
	virtual void                         Lock();
	virtual void                         Unlock();
	virtual void                         SetGameFolder(tukk szFolder);
	virtual tukk                  GetGameFolder() const;
	virtual IDrxPak::PakInfo*            GetPakInfo();
	virtual void                         FreePakInfo(PakInfo*);
	virtual FILE*                        FOpen(tukk pName, tukk mode, unsigned nFlags);
	virtual FILE*                        FOpen(tukk pName, tukk mode, tuk szFileGamePath, i32 nLen);
	virtual size_t                       FReadRaw(uk data, size_t length, size_t elems, FILE* handle);
	virtual size_t                       FReadRawAll(uk data, size_t nFileSize, FILE* handle);
	virtual uk                        FGetCachedFileData(FILE* handle, size_t& nFileSize);
	virtual size_t                       FWrite(ukk data, size_t length, size_t elems, FILE* handle);
	virtual i32                          FPrintf(FILE* handle, tukk format, ...) PRINTF_PARAMS(3, 4);
	virtual tuk                        FGets(tuk, i32, FILE*);
	virtual i32                          Getc(FILE*);
	virtual size_t                       FGetSize(FILE* f);
	virtual size_t                       FGetSize(tukk pName);
	virtual i32                          Ungetc(i32 c, FILE*);
	virtual bool                         IsInPak(FILE* handle);
	virtual bool                         RemoveFile(tukk pName);               // remove file from FS (if supported)
	virtual bool                         RemoveDir(tukk pName, bool bRecurse); // remove directory from FS (if supported)
	virtual bool                         IsAbsPath(tukk pPath);                // determines if pPath is an absolute or relative path
	virtual bool                         CopyFileOnDisk(tukk source, tukk dest, bool bFailIfExist);
	virtual size_t                       FSeek(FILE* handle, long seek, i32 mode);
	virtual long                         FTell(FILE* handle);
	virtual i32                          FClose(FILE* handle);
	virtual i32                          FEof(FILE* handle);
	virtual i32                          FError(FILE* handle);
	virtual i32                          FGetErrno();
	virtual i32                          FFlush(FILE* handle);
	virtual uk                        PoolMalloc(size_t size);
	virtual void                         PoolFree(uk p);
	virtual intptr_t                     FindFirst(tukk pDir, _finddata_t* fd, u32 nFlags);
	virtual i32                          FindNext(intptr_t handle, _finddata_t* fd);
	virtual i32                          FindClose(intptr_t handle);
	virtual IDrxPak::FileTime            GetModificationTime(FILE* f);
	virtual bool                         IsFileExist(tukk sFilename);
	virtual bool                         MakeDir(tukk szPath, bool bGamePathMapping);
	virtual IDrxArchive*                 OpenArchive(tukk szPath, u32 nFlags);
	virtual tukk                  GetFileArchivePath(FILE* f);
	virtual i32                          RawCompress(ukk pUncompressed, u64* pDestSize, uk pCompressed, u64 nSrcSize, i32 nLevel);
	virtual i32                          RawUncompress(uk pUncompressed, u64* pDestSize, ukk pCompressed, u64 nSrcSize);
	virtual void                         RecordFileOpen(const ERecordFileOpenList eList);
	virtual void                         RecordFile(FILE* in, tukk szFilename);
	virtual IResourceList*               GetResourceList(const ERecordFileOpenList eList);
	virtual IDrxPak::ERecordFileOpenList GetRecordFileOpenList();
	virtual void                         Notify(ENotifyEvent event);
	virtual u32                       ComputeCRC(tukk szPath);
	virtual bool                         ComputeMD5(tukk szPath, u8* md5);
	virtual void                         RegisterFileAccessSink(IDrxPakFileAcesssSink* pSink);
	virtual void                         UnregisterFileAccessSink(IDrxPakFileAcesssSink* pSink);
	virtual bool                         GetLvlResStatus() const;

private:
	struct IFileManip
	{
		inline virtual ~IFileManip() {}
		virtual size_t ReadRaw(uk data, size_t length, size_t elems) = 0;
		virtual size_t Seek(long seek, i32 mode) = 0;
		virtual size_t Tell() = 0;
		virtual i32    Eof() = 0;
	};

	class CNonSyncedManip;
	class CSyncedManip;

	CSyncedFileSet& m_fileset;
};

#endif

#endif
