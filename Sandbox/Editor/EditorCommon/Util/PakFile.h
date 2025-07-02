// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/File/IDrxPak.h>

// forward references.
struct IDrxArchive;
class CDrxMemFile;
class CMemoryBlock;
TYPEDEF_AUTOPTR(IDrxArchive);

/*!	CPakFile Wraps game implementation of IDrxArchive.
    Used for storing multiple files into zip archive file.
 */
class EDITOR_COMMON_API CPakFile
{
public:
	CPakFile();
	CPakFile(IDrxPak* pDrxPak);
	~CPakFile();
	//! Opens archive for writing.
	explicit CPakFile(tukk filename);
	//! Opens archive for writing.
	bool Open(tukk filename, bool bAbsolutePath = true);
	//! Opens archive for reading only.
	bool OpenForRead(tukk filename);

	void Close();
	//! Adds or update file in archive.
	bool UpdateFile(tukk filename, CDrxMemFile& file, bool bCompress = true);
	//! Adds or update file in archive.
	bool UpdateFile(tukk filename, CMemoryBlock& mem, bool bCompress = true, i32 nCompressLevel = IDrxArchive::LEVEL_BETTER);
	//! Adds or update file in archive.
	bool UpdateFile(tukk filename, uk pBuffer, i32 nSize, bool bCompress = true, i32 nCompressLevel = IDrxArchive::LEVEL_BETTER);
	//! Remove file from archive.
	bool RemoveFile(tukk filename);
	//! Remove dir from archive.
	bool RemoveDir(tukk directory);

	//! Return archive of this pak file wrapper.
	IDrxArchive* GetArchive() { return m_pArchive; };
private:
	IDrxArchive_AutoPtr m_pArchive;
	IDrxPak*            m_pDrxPak;
};


