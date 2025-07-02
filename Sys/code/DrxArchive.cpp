// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/DrxPak.h>
#include <drx3D/Sys/DrxArchive.h>

//////////////////////////////////////////////////////////////////////////
#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

i32 DrxArchiveRW::EnumEntries(Handle hFolder, IEnumerateArchiveEntries* pEnum)
{
	ZipDir::FileEntryTree* pRoot = (ZipDir::FileEntryTree*)hFolder;
	i32 nEntries = 0;
	bool bContinue = true;

	if (pEnum)
	{
		ZipDir::FileEntryTree::SubdirMap::iterator iter = pRoot->GetDirBegin();

		while (iter != pRoot->GetDirEnd() && bContinue)
		{
			bContinue = pEnum->OnEnumArchiveEntry(iter->first, iter->second, true, 0, 0);
			++iter;
			++nEntries;
		}

		ZipDir::FileEntryTree::FileMap::iterator iterFile = pRoot->GetFileBegin();

		while (iterFile != pRoot->GetFileEnd() && bContinue)
		{
			bContinue = pEnum->OnEnumArchiveEntry(iterFile->first, &iterFile->second, false, iterFile->second.desc.lSizeUncompressed, iterFile->second.GetModificationTime());
			++iterFile;
			++nEntries;
		}
	}

	return nEntries;
}

DrxArchiveRW::Handle DrxArchiveRW::GetRootFolderHandle()
{
	return m_pCache->GetRoot();
}

i32 DrxArchiveRW::UpdateFileCRC(tukk szRelativePath, u32k dwCRC)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;

	m_pCache->UpdateFileCRC(pPath, dwCRC);

	return ZipDir::ZD_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// Удаляет файл из архива
i32 DrxArchiveRW::RemoveFile(tukk szRelativePath)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;
	return m_pCache->RemoveFile(pPath);
}

//////////////////////////////////////////////////////////////////////////
// deletes the directory, with all its descendants (files and subdirs)
i32 DrxArchiveRW::RemoveDir(tukk szRelativePath)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;
	return m_pCache->RemoveDir(pPath);
}

i32 DrxArchiveRW::RemoveAll()
{
	return m_pCache->RemoveAll();

}

//////////////////////////////////////////////////////////////////////////
// Adds a new file to the zip or update an existing one
// adds a directory (creates several nested directories if needed)
// compression methods supported are 0 (store) and 8 (deflate) , compression level is 0..9 or -1 for default (like in zlib)
i32 DrxArchiveRW::UpdateFile(tukk szRelativePath, uk pUncompressed, unsigned nSize, unsigned nCompressionMethod, i32 nCompressionLevel)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;
	return m_pCache->UpdateFile(pPath, pUncompressed, nSize, nCompressionMethod, nCompressionLevel);
}

//////////////////////////////////////////////////////////////////////////
//   Adds a new file to the zip or update an existing one if it is not compressed - just stored  - start a big file
i32 DrxArchiveRW::StartContinuousFileUpdate(tukk szRelativePath, unsigned nSize)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;
	return m_pCache->StartContinuousFileUpdate(pPath, nSize);
}

//////////////////////////////////////////////////////////////////////////
// Adds a new file to the zip or update an existing's segment if it is not compressed - just stored
// adds a directory (creates several nested directories if needed)
i32 DrxArchiveRW::UpdateFileContinuousSegment(tukk szRelativePath, unsigned nSize, uk pUncompressed, unsigned nSegmentSize, unsigned nOverwriteSeekPos)
{
	if (m_nFlags & FLAGS_READ_ONLY)
		return ZipDir::ZD_ERROR_INVALID_CALL;

	char szFullPath[CDrxPak::g_nMaxPath];
	tukk pPath = AdjustPath(szRelativePath, szFullPath);
	if (!pPath)
		return ZipDir::ZD_ERROR_INVALID_PATH;
	return m_pCache->UpdateFileContinuousSegment(pPath, nSize, pUncompressed, nSegmentSize, nOverwriteSeekPos);
}

#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
