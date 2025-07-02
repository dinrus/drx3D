// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ZIP_DIR_TREE_HDR_
#define _ZIP_DIR_TREE_HDR_

namespace ZipDir
{

class FileEntryTree
{
public:
	~FileEntryTree() { Clear(); }

	// adds a file to this directory
	// Function can modify szPath input
	ErrorEnum Add(tuk szPath, const FileEntry& file);

	// Adds or finds the file. Returns non-initialized structure if it was added,
	// or an IsInitialized() structure if it was found
	// Function can modify szPath input
	FileEntry* Add(tuk szPath);

	// returns the number of files in this tree, including this and sublevels
	u32 NumFilesTotal() const;

	// Total number of directories
	u32 NumDirsTotal() const;

	// returns the size required to serialize the tree
	size_t GetSizeSerialized() const;

	// serializes into the memory
	size_t Serialize(DirHeader* pDir) const;

	void   Clear();

	void   Swap(FileEntryTree& rThat)
	{
		m_mapDirs.swap(rThat.m_mapDirs);
		m_mapFiles.swap(rThat.m_mapFiles);
	}

	size_t GetSize() const;

	bool   IsOwnerOf(const FileEntry* pFileEntry) const;

	// subdirectories
	typedef std::map<tukk , FileEntryTree*, stl::less_strcmp<tukk >> SubdirMap;
	// file entries
	typedef std::map<tukk , FileEntry, stl::less_strcmp<tukk >>      FileMap;

	FileEntryTree*      FindDir(tukk szDirName);
	ErrorEnum           RemoveDir(tukk szDirName);
	ErrorEnum           RemoveAll() { Clear(); return ZD_ERROR_SUCCESS; }
	FileEntry*          FindFileEntry(tukk szFileName);
	FileMap::iterator   FindFile(tukk szFileName);
	ErrorEnum           RemoveFile(tukk szFileName);
	FileEntryTree*      GetDirectory()                     { return this; } // the FileENtryTree is simultaneously an entry in the dir list AND the directory header

	FileMap::iterator   GetFileBegin()                     { return m_mapFiles.begin(); }
	FileMap::iterator   GetFileEnd()                       { return m_mapFiles.end(); }
	unsigned            NumFiles() const                   { return (unsigned)m_mapFiles.size(); }
	SubdirMap::iterator GetDirBegin()                      { return m_mapDirs.begin(); }
	SubdirMap::iterator GetDirEnd()                        { return m_mapDirs.end(); }
	unsigned            NumDirs() const                    { return (unsigned)m_mapDirs.size(); }
	tukk         GetFileName(FileMap::iterator it)  { return it->first; }
	tukk         GetDirName(SubdirMap::iterator it) { return it->first; }

	FileEntry*          GetFileEntry(FileMap::iterator it);
	FileEntryTree*      GetDirEntry(SubdirMap::iterator it);

protected:
	SubdirMap m_mapDirs;
	FileMap   m_mapFiles;
};

}
#endif
