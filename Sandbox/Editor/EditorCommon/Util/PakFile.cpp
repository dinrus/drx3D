// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include "Util/PakFile.h"
#include "Util/DrxMemFile.h"
#include "Util/MemoryBlock.h"

//////////////////////////////////////////////////////////////////////////
CPakFile::CPakFile()
	: m_pArchive(NULL),
	m_pDrxPak(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
CPakFile::CPakFile(IDrxPak* pDrxPak)
	: m_pArchive(NULL),
	m_pDrxPak(pDrxPak)
{
}

//////////////////////////////////////////////////////////////////////////
CPakFile::~CPakFile()
{
	Close();
}

//////////////////////////////////////////////////////////////////////////
CPakFile::CPakFile(tukk filename)
{
	m_pArchive = NULL;
	Open(filename);
}

//////////////////////////////////////////////////////////////////////////
void CPakFile::Close()
{
	m_pArchive = NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::Open(tukk filename, bool bAbsolutePath)
{
	if (m_pArchive)
		Close();

	IDrxPak* pDrxPak = m_pDrxPak ? m_pDrxPak : GetIEditor()->GetSystem()->GetIPak();
	if (pDrxPak == NULL)
		return false;

	if (bAbsolutePath)
		m_pArchive = pDrxPak->OpenArchive(filename, IDrxArchive::FLAGS_ABSOLUTE_PATHS);
	else
		m_pArchive = pDrxPak->OpenArchive(filename);
	if (m_pArchive)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::OpenForRead(tukk filename)
{
	if (m_pArchive)
		Close();
	IDrxPak* pDrxPak = m_pDrxPak ? m_pDrxPak : GetIEditor()->GetSystem()->GetIPak();
	if (pDrxPak == NULL)
		return false;
	m_pArchive = pDrxPak->OpenArchive(filename, IDrxArchive::FLAGS_OPTIMIZED_READ_ONLY | IDrxArchive::FLAGS_ABSOLUTE_PATHS);
	if (m_pArchive)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::UpdateFile(tukk filename, CDrxMemFile& file, bool bCompress)
{
	if (m_pArchive)
	{
		i32 nSize = file.GetLength();

		UpdateFile(filename, file.GetMemPtr(), nSize, bCompress);
		file.Close();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::UpdateFile(tukk filename, CMemoryBlock& mem, bool bCompress, i32 nCompressLevel)
{
	if (m_pArchive)
	{
		return UpdateFile(filename, mem.GetBuffer(), mem.GetSize(), bCompress, nCompressLevel);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////
bool CPakFile::UpdateFile(tukk filename, uk pBuffer, i32 nSize, bool bCompress, i32 nCompressLevel)
{
	if (m_pArchive)
	{
		if (bCompress)
			return 0 == m_pArchive->UpdateFile(filename, pBuffer, nSize, IDrxArchive::METHOD_DEFLATE, nCompressLevel);
		else
			return 0 == m_pArchive->UpdateFile(filename, pBuffer, nSize);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::RemoveFile(tukk filename)
{
	if (m_pArchive)
	{
		return m_pArchive->RemoveFile(filename);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CPakFile::RemoveDir(tukk directory)
{
	if (m_pArchive)
	{
		return m_pArchive->RemoveDir(directory);
	}
	return false;
}

