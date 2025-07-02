// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   DrxArchive.h
//  Created:     18/08/2010 by Timur.
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRX_ARCHIVE_HDR__
#define __DRX_ARCHIVE_HDR__

#include <drx3D/Sys/DrxPak.h>

struct DrxArchiveSortByName
{
	bool operator()(const IDrxArchive* left, const IDrxArchive* right) const
	{
		return stricmp(left->GetFullPath(), right->GetFullPath()) < 0;
	}
	bool operator()(tukk left, const IDrxArchive* right) const
	{
		return stricmp(left, right->GetFullPath()) < 0;
	}
	bool operator()(const IDrxArchive* left, tukk right) const
	{
		return stricmp(left->GetFullPath(), right) < 0;
	}
};

template<class Cache>
class TDrxArchive : public IDrxArchive
{
public:
	TDrxArchive(CDrxPak* pPak, const string& strBindRoot, Cache* pCache, unsigned nFlags = 0) :
		m_pCache(pCache),
		m_strBindRoot(strBindRoot),
		m_pPak(pPak),
		m_nFlags(nFlags)
	{
		pPak->Register(this);
	}

	~TDrxArchive()
	{
		m_pPak->Unregister(this);
	}

	// finds the file; you don't have to close the returned handle
	Handle FindFile(tukk szRelativePath)
	{
		char szFullPath[CDrxPak::g_nMaxPath];
		tukk pPath = AdjustPath(szRelativePath, szFullPath);
		if (!pPath)
			return NULL;
		return m_pCache->FindFile(pPath);
	}

	// returns the size of the file (unpacked) by the handle
	unsigned GetFileSize(Handle h)
	{
		assert(m_pCache->IsOwnerOf((ZipDir::FileEntry*)h));
		return ((ZipDir::FileEntry*)h)->desc.lSizeUncompressed;
	}

	// reads the file into the preallocated buffer (must be at least the size of GetFileSize())
	i32 ReadFile(Handle h, uk pBuffer)
	{
		assert(m_pCache->IsOwnerOf((ZipDir::FileEntry*)h));
		return m_pCache->ReadFile((ZipDir::FileEntry*)h, NULL, pBuffer);
	}

	// returns the full path to the archive file
	tukk GetFullPath() const
	{
		return m_pCache->GetFilePath();
	}

	unsigned GetFlags() const { return m_nFlags; }
	bool     SetFlags(unsigned nFlagsToSet)
	{
		if (nFlagsToSet & FLAGS_RELATIVE_PATHS_ONLY)
			m_nFlags |= FLAGS_RELATIVE_PATHS_ONLY;

		if (nFlagsToSet & FLAGS_ON_HDD)
			m_nFlags |= FLAGS_ON_HDD;

		if (nFlagsToSet & FLAGS_RELATIVE_PATHS_ONLY ||
		    nFlagsToSet & FLAGS_ON_HDD)
		{
			// we don't support changing of any other flags
			return true;
		}
		return false;
	}

	bool ResetFlags(unsigned nFlagsToReset)
	{
		if (nFlagsToReset & FLAGS_RELATIVE_PATHS_ONLY)
			m_nFlags &= ~FLAGS_RELATIVE_PATHS_ONLY;

		if (nFlagsToReset & ~(FLAGS_RELATIVE_PATHS_ONLY))
		{
			// we don't support changing of any other flags
			return false;
		}
		return true;
	}

	bool SetPackAccessible(bool bAccessible)
	{
		if (bAccessible)
		{
			bool bResult = (m_nFlags& IDrxArchive::FLAGS_DISABLE_PAK) != 0;
			m_nFlags &= ~IDrxArchive::FLAGS_DISABLE_PAK;
			return bResult;
		}
		else
		{
			bool bResult = (m_nFlags& IDrxArchive::FLAGS_DISABLE_PAK) == 0;
			m_nFlags |= IDrxArchive::FLAGS_DISABLE_PAK;
			return bResult;
		}
	}

	Cache* GetCache() { return m_pCache; }
protected:
	// returns the pointer to the relative file path to be passed
	// to the underlying Cache pointer. Uses the given buffer to construct the path.
	// returns NULL if the file path is invalid
	tukk AdjustPath(tukk szRelativePath, char szFullPathBuf[CDrxPak::g_nMaxPath])
	{
		if (!szRelativePath[0])
			return NULL;

		if (m_nFlags & FLAGS_RELATIVE_PATHS_ONLY)
			return szRelativePath;

		if (szRelativePath[1] == ':' || (m_nFlags & FLAGS_ABSOLUTE_PATHS))
		{
			// make the normalized full path and try to match it against the binding root of this object
			tukk szFullPath = m_pPak->AdjustFileName(szRelativePath, szFullPathBuf, IDrxPak::FLAGS_PATH_REAL);
			size_t nPathLen = strlen(szFullPath);
			if (nPathLen <= m_strBindRoot.length())
				return NULL;

			// you should access exactly the file under the directly in which the zip is situated
			if (szFullPath[m_strBindRoot.length()] != '/' && szFullPath[m_strBindRoot.length()] != '\\')
				return NULL;
			if (memicmp(szFullPath, m_strBindRoot.c_str(), m_strBindRoot.length()))
				return NULL; // the roots don't match

			return szFullPath + m_strBindRoot.length() + 1;
		}

		return szRelativePath;
	}
protected:
	_smart_ptr<Cache> m_pCache;
	// the binding root may be empty string - in this case, the absolute path binding won't work
	string            m_strBindRoot;
	CDrxPak*          m_pPak;
	unsigned          m_nFlags;
};

#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
class DrxArchiveRW : public TDrxArchive<ZipDir::CacheRW>
{
public:

	DrxArchiveRW(CDrxPak* pPak, const string& strBindRoot, ZipDir::CacheRW* pCache, unsigned nFlags = 0) :
		TDrxArchive<ZipDir::CacheRW>(pPak, strBindRoot, pCache, nFlags)
	{
	}

	~DrxArchiveRW()
	{
	}

	i32    EnumEntries(Handle hFolder, IEnumerateArchiveEntries* pEnum);
	Handle GetRootFolderHandle();

	// Adds a new file to the zip or update an existing one
	// adds a directory (creates several nested directories if needed)
	// compression methods supported are 0 (store) and 8 (deflate) , compression level is 0..9 or -1 for default (like in zlib)
	i32 UpdateFile(tukk szRelativePath, uk pUncompressed, unsigned nSize, unsigned nCompressionMethod = 0, i32 nCompressionLevel = -1);

	//   Adds a new file to the zip or update an existing one if it is not compressed - just stored  - start a big file
	i32 StartContinuousFileUpdate(tukk szRelativePath, unsigned nSize);

	// Adds a new file to the zip or update an existing's segment if it is not compressed - just stored
	// adds a directory (creates several nested directories if needed)
	// Arguments:
	//   nOverwriteSeekPos - 0xffffffff means the seek pos should not be overwritten
	i32         UpdateFileContinuousSegment(tukk szRelativePath, unsigned nSize, uk pUncompressed, unsigned nSegmentSize, unsigned nOverwriteSeekPos);

	virtual i32 UpdateFileCRC(tukk szRelativePath, u32k dwCRC);

	// deletes the file from the archive
	i32 RemoveFile(tukk szRelativePath);

	// deletes the directory, with all its descendants (files and subdirs)
	i32 RemoveDir(tukk szRelativePath);

	i32 RemoveAll();

	enum {gClassId = 1};
	unsigned GetClassId() const { return gClassId; }

	void     GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};
#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

class DrxArchive : public TDrxArchive<ZipDir::Cache>
{
public:
	DrxArchive(CDrxPak* pPak, const string& strBindRoot, ZipDir::Cache* pCache, unsigned nFlags) :
		TDrxArchive<ZipDir::Cache>(pPak, strBindRoot, pCache, nFlags)
	{}

	~DrxArchive(){}

	i32    EnumEntries(Handle hFolder, IEnumerateArchiveEntries* pEnum) { return 0; };
	Handle GetRootFolderHandle()                                        { return NULL; };

	// Adds a new file to the zip or update an existing one
	// adds a directory (creates several nested directories if needed)
	// compression methods supported are METHOD_STORE == 0 (store) and
	// METHOD_DEFLATE == METHOD_COMPRESS == 8 (deflate) , compression
	// level is LEVEL_FASTEST == 0 till LEVEL_BEST == 9 or LEVEL_DEFAULT == -1
	// for default (like in zlib)
	i32 UpdateFile(tukk szRelativePath, uk pUncompressed, unsigned nSize, unsigned nCompressionMethod = 0, i32 nCompressionLevel = -1) { return ZipDir::ZD_ERROR_INVALID_CALL; }

	//   Adds a new file to the zip or update an existing one if it is not compressed - just stored  - start a big file
	i32 StartContinuousFileUpdate(tukk szRelativePath, unsigned nSize) { return ZipDir::ZD_ERROR_INVALID_CALL; }

	// Adds a new file to the zip or update an existing's segment if it is not compressed - just stored
	// adds a directory (creates several nested directories if needed)
	i32         UpdateFileContinuousSegment(tukk szRelativePath, unsigned nSize, uk pUncompressed, unsigned nSegmentSize, unsigned nOverwriteSeekPos) { return ZipDir::ZD_ERROR_INVALID_CALL; }

	virtual i32 UpdateFileCRC(tukk szRelativePath, u32k dwCRC)                                                                                   { return ZipDir::ZD_ERROR_INVALID_CALL; }

	// deletes the file from the archive
	i32 RemoveFile(tukk szRelativePath) { return ZipDir::ZD_ERROR_INVALID_CALL; }
	i32 RemoveAll()                            { return ZipDir::ZD_ERROR_INVALID_CALL; }

	// deletes the directory, with all its descendants (files and subdirs)
	i32      RemoveDir(tukk szRelativePath) { return ZipDir::ZD_ERROR_INVALID_CALL; }
	enum {gClassId = 2};
	unsigned GetClassId() const                    { return gClassId; }

	void     GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};
#endif //__DRX_ARCHIVE_HDR__
