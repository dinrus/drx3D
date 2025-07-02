// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/MTSafeAllocator.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/Sys/ZipFileFormat.h>
#include <drx3D/Sys/ZipDirStructures.h>
#include <drx3D/Sys/ZipDirCache.h>
#include <drx3D/Sys/ZipDirFind.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/String/Path.h>

bool ZipDir::FindFile::FindFirst(tukk szWildcard)
{
	if (!PreFind(szWildcard))
		return false;

	// finally, this is the name of the file
	m_nFileEntry = 0;
	return SkipNonMatchingFiles();
}

bool ZipDir::FindDir::FindFirst(tukk szWildcard)
{
	if (!PreFind(szWildcard))
		return false;

	// finally, this is the name of the file
	m_nDirEntry = 0;
	return SkipNonMatchingDirs();
}

// matches the file wilcard in the m_szWildcard to the given file/dir name
// this takes into account the fact that xxx. is the alias name for xxx
bool ZipDir::FindData::MatchWildcard(tukk szName)
{
	if (PathUtil::MatchWildcard(szName, m_szWildcard))
		return true;

	// check if the file object name contains extension sign (.)
	tukk p;
	for (p = szName; *p && *p != '.'; ++p)
		continue;

	if (*p)
	{
		// there's an extension sign in the object, but it wasn't matched..
		assert(*p == '.');
		return false;
	}

	// no extension sign - add it
	char szAlias[_MAX_PATH + 2];
	unsigned nLength = (unsigned)(p - szName);
	if (nLength > _MAX_PATH)
		nLength = _MAX_PATH;
	memcpy(szAlias, szName, nLength);
	szAlias[nLength] = '.';      // add the alias
	szAlias[nLength + 1] = '\0'; // terminate the string
	return PathUtil::MatchWildcard(szAlias, m_szWildcard);
}

ZipDir::FileEntry* ZipDir::FindFile::FindExact(tukk szPath)
{
	if (!PreFind(szPath))
		return NULL;

	FileEntry* pFileEntry = m_pDirHeader->FindFileEntry(m_szWildcard);
	if (pFileEntry)
		m_nFileEntry = (u32)(pFileEntry - m_pDirHeader->GetFileEntry(0));
	else
		m_pDirHeader = NULL; // we didn't find it, fail the search

	return pFileEntry;
}

//////////////////////////////////////////////////////////////////////////
// after this call returns successfully (with true returned), the m_szWildcard
// contains the file name/wildcard and m_pDirHeader contains the directory where
// the file (s) are to be found
bool ZipDir::FindData::PreFind(tukk szWildcard)
{
	if (!m_pRoot)
		return false;

	// start the search from the root
	m_pDirHeader = m_pRoot;

	// for each path dir name, copy it into the buffer and try to find the subdirectory
	tukk pPath = szWildcard;
	for (;; )
	{
		tuk pName = m_szWildcard;

		// at first we'll use the wildcard memory to save the directory names
		for (; *pPath && *pPath != '/' && *pPath != '\\' && pName < m_szWildcard + sizeof(m_szWildcard) - 1; ++pPath, ++pName)
			*pName = ::tolower(*pPath);
		*pName = '\0';

		if (*pPath)
		{
			if (*pPath != '/' && *pPath != '\\')
				return false;//ZD_ERROR_NAME_TOO_LONG;

			// this is the name of the directory
			DirEntry* pDirEntry = m_pDirHeader->FindSubdirEntry(m_szWildcard);
			if (!pDirEntry)
			{
				m_pDirHeader = NULL; // finish the search
				return false;
			}
			m_pDirHeader = pDirEntry->GetDirectory();
			++pPath;
			assert(m_pDirHeader);
		}
		else
		{
			// finally, this is the name of the file (or directory)
			return true;
		}
	}
}

// goes on to the next entry
bool ZipDir::FindFile::FindNext()
{
	if (m_pDirHeader && m_nFileEntry < m_pDirHeader->numFiles)
	{
		++m_nFileEntry;
		return SkipNonMatchingFiles();
	}
	else
		return false;
}

// goes on to the next entry
bool ZipDir::FindDir::FindNext()
{
	if (m_pDirHeader && m_nDirEntry < m_pDirHeader->numDirs)
	{
		++m_nDirEntry;
		return SkipNonMatchingDirs();
	}
	else
		return false;
}

bool ZipDir::FindFile::SkipNonMatchingFiles()
{
	assert(m_pDirHeader && m_nFileEntry <= m_pDirHeader->numFiles);

	for (; m_nFileEntry < m_pDirHeader->numFiles; ++m_nFileEntry)
	{
		if (MatchWildcard(GetFileName()))
			return true;
	}
	// we didn't find anything other file else
	return false;
}

bool ZipDir::FindDir::SkipNonMatchingDirs()
{
	assert(m_pDirHeader && m_nDirEntry <= m_pDirHeader->numDirs);

	for (; m_nDirEntry < m_pDirHeader->numDirs; ++m_nDirEntry)
	{
		if (MatchWildcard(GetDirName()))
			return true;
	}
	// we didn't find anything other file else
	return false;
}

ZipDir::FileEntry* ZipDir::FindFile::GetFileEntry()
{
	return m_pDirHeader && m_nFileEntry < m_pDirHeader->numFiles ? m_pDirHeader->GetFileEntry(m_nFileEntry) : NULL;
}
ZipDir::DirEntry* ZipDir::FindDir::GetDirEntry()
{
	return m_pDirHeader && m_nDirEntry < m_pDirHeader->numDirs ? m_pDirHeader->GetSubdirEntry(m_nDirEntry) : NULL;
}

tukk ZipDir::FindFile::GetFileName()
{
	if (m_pDirHeader && m_nFileEntry < m_pDirHeader->numFiles)
	{
		tukk pNamePool = m_pDirHeader->GetNamePool();
		return m_pDirHeader->GetFileEntry(m_nFileEntry)->GetName(pNamePool);
	}
	else
		return ""; // default name
}

tukk ZipDir::FindDir::GetDirName()
{
	if (m_pDirHeader && m_nDirEntry < m_pDirHeader->numDirs)
	{
		tukk pNamePool = m_pDirHeader->GetNamePool();
		return m_pDirHeader->GetSubdirEntry(m_nDirEntry)->GetName(pNamePool);
	}
	else
		return ""; // default name
}
