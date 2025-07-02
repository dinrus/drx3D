// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ZIP_DIR_SEARCH_HDR_
#define _ZIP_DIR_SEARCH_HDR_

namespace ZipDir
{

// create this structure and loop:
//  FindData fd (pZip);
//  for (fd.FindFirst("*.cgf"); fd.GetFileEntry(); fd.FindNext())
//  {} // inside the loop, use GetFileEntry() and GetFileName() to get the file entry and name records
class FindData
{
public:

	FindData(DirHeader* pRoot) :
		m_pRoot(pRoot),
		m_pDirHeader(NULL)
	{
		m_szWildcard[0] = 0;
	}

protected:
	// initializes everything until the point where the file must be searched for
	// after this call returns successfully (with true returned), the m_szWildcard
	// contains the file name/wildcard and m_pDirHeader contains the directory where
	// the file (s) are to be found
	bool PreFind(tukk szWildcard);

	// matches the file wilcard in the m_szWildcard to the given file/dir name
	// this takes into account the fact that xxx. is the alias name for xxx
	bool MatchWildcard(tukk szName);

	DirHeader* m_pRoot;      // the zip file inwhich the search is performed
	DirHeader* m_pDirHeader; // the header of the directory in which the files reside
	//unsigned m_nDirEntry; // the current directory entry inside the parent directory

	// the actual wildcard being used in the current scan - the file name wildcard only!
	char m_szWildcard[_MAX_PATH];
};

class FindFile : public FindData
{
public:
	FindFile(Cache* pCache) :
		FindData(pCache->GetRoot()),
		m_nFileEntry(0)
	{
	}
	FindFile(DirHeader* pRoot) :
		FindData(pRoot),
		m_nFileEntry(0)
	{
	}
	// if bExactFile is passed, only the file is searched, and besides with the exact name as passed (no wildcards)
	bool       FindFirst(tukk szWildcard);

	FileEntry* FindExact(tukk szPath);

	// goes on to the next file entry
	bool        FindNext();

	FileEntry*  GetFileEntry();
	tukk GetFileName();

protected:
	bool SkipNonMatchingFiles();
	unsigned m_nFileEntry; // the current file index inside the parent directory
};

class FindDir : public FindData
{
public:
	FindDir(Cache* pCache) :
		FindData(pCache->GetRoot()),
		m_nDirEntry(0)
	{
	}
	FindDir(DirHeader* pRoot) :
		FindData(pRoot),
		m_nDirEntry(0)
	{
	}
	// if bExactFile is passed, only the file is searched, and besides with the exact name as passed (no wildcards)
	bool FindFirst(tukk szWildcard);

	// goes on to the next file entry
	bool        FindNext();

	DirEntry*   GetDirEntry();
	tukk GetDirName();

protected:
	bool SkipNonMatchingDirs();
	unsigned m_nDirEntry; // the current dir index inside the parent directory
};
}

#endif
