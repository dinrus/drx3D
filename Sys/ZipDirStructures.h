// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ZIP_DIR_DEFINITIONS
#define _ZIP_DIR_DEFINITIONS

#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
	#include <ctype.h>
#endif

#if DRX_PLATFORM_ANDROID
	#include <android/asset_manager.h>
#endif

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/DrxEndian.h>
#include <drx3D/CoreX/Memory/IMemory.h>
#include <drx3D/CoreX/Project/ProjectDefines.h>

#include <drx3D/Sys/MTSafeAllocator.h>
#include <drx3D/Sys/ZipFileFormat.h>
#include <drx3D/CoreX/Platform/DrxWindows.h>

#if DRX_PLATFORM_WINDOWS
	#define SUPPORT_UNBUFFERED_IO
#endif

// this was enabled for last gen consoles, could be useful for durango & orbis
//#define OPTIMIZED_READONLY_ZIP_ENTRY

struct z_stream_s;

namespace ZipDir
{
struct FileEntry;
struct CZipFile
{
	FILE*  m_file;
#ifdef SUPPORT_UNBUFFERED_IO
	HANDLE m_unbufferedFile;
	size_t m_nSectorSize;
	uk  m_pReadTarget;
#endif
#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
	/// m_pPakFileRef is the pointer to the containing PAK file. It must
	/// be set to NULL if the PAK file is NOT contained in any opened
	/// PAK file. For example:
	///   * Any pak file in the file system, including the main and patch
	///     expansion files.
	///   * The uncompressed zip file (assets.ogg) file contained in APK
	///     package.
	FILE* m_pPakFileRef;

	/// m_pPakFileEntry is the pointer to the FileEntry structure of itself
	/// inside the containing APK file. For PAK file in file system, the
	/// main/patch expansion files and the asset file (assets.ogg),
	/// m_pPakFileEntry must be NULL.
	FileEntry* m_pPakFileEntry;

	/// For the asset file (assets.ogg) and any PAK file contained in it,
	/// m_nAssetOffset is the offset relative to the APK package.
	/// m_nAssetLength is the length of asset file. (asset.ogg).
	///
	/// For any PAK file (including the main and patch expansion files).
	i32                 m_nAssetOffset;
	i32                 m_nAssetLength;
#endif
	int64               m_nSize;
	int64               m_nCursor;
	tukk         m_szFilename;
	ICustomMemoryBlock* m_pInMemoryData;

	CZipFile()
		: m_file(0)
#ifdef SUPPORT_UNBUFFERED_IO
		, m_unbufferedFile(INVALID_HANDLE_VALUE)
		, m_nSectorSize(0)
		, m_pReadTarget(NULL)
#endif
#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
		, m_pPakFileRef(NULL)
		, m_pPakFileEntry(NULL)
		, m_nAssetOffset(0)
		, m_nAssetLength(0)
#endif
		, m_nSize(0)
		, m_nCursor(0)
		, m_szFilename(0)
		, m_pInMemoryData(0)
	{}

	void Swap(CZipFile& other)
	{
		using std::swap;
		swap(m_file, other.m_file);
#ifdef SUPPORT_UNBUFFERED_IO
		swap(m_unbufferedFile, other.m_unbufferedFile);
		swap(m_nSectorSize, other.m_nSectorSize);
		swap(m_pReadTarget, other.m_pReadTarget);
#endif
#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
		swap(m_pPakFileRef, other.m_pPakFileRef);
		swap(m_pPakFileEntry, other.m_pPakFileEntry);
		swap(m_nAssetOffset, other.m_nAssetOffset);
		swap(m_nAssetLength, other.m_nAssetLength);
#endif
		swap(m_nSize, other.m_nSize);
		swap(m_nCursor, other.m_nCursor);
		swap(m_szFilename, other.m_szFilename);
		swap(m_pInMemoryData, other.m_pInMemoryData);
	}

	bool IsInMemory() const { return m_pInMemoryData != 0; }
	void LoadToMemory(IMemoryBlock* pData = 0);
	void UnloadFromMemory();
	void Close(bool bUnloadFromMem = true);

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
	/// Return true if pak file is contained in another pak file, or
	/// contained in APK package.
	bool InPakFileOrAsset() const
	{
		return m_file == NULL && m_pPakFileRef != NULL &&
		       m_pPakFileEntry != NULL;
	}

	/// Return true if the file is the uncompressed zip file contained in
	/// apk package.
	bool IsAssetFile() const
	{
		return m_nAssetLength != 0 && m_nAssetOffset != 0;
	}

	/// Return offset within containing OBB file or asset.
	u32 GetFileDataOffset() const;
#endif

#ifdef SUPPORT_UNBUFFERED_IO
	bool OpenUnbuffered(tukk filename);
	bool EvaluateSectorSize(tukk filename);
#endif

private:
	explicit CZipFile(const CZipFile& file) {};     // Protect copy ctor.
	CZipFile& operator=(const CZipFile& from);
};

// possible errors occuring during the method execution
// to avoid clashing with the global Windows defines, we prefix these with ZD_
enum ErrorEnum
{
	ZD_ERROR_SUCCESS = 0,
	ZD_ERROR_IO_FAILED,
	ZD_ERROR_UNEXPECTED,
	ZD_ERROR_UNSUPPORTED,
	ZD_ERROR_INVALID_SIGNATURE,
	ZD_ERROR_ZIP_FILE_IS_CORRUPT,
	ZD_ERROR_DATA_IS_CORRUPT,
	ZD_ERROR_NO_CDR,
	ZD_ERROR_CDR_IS_CORRUPT,
	ZD_ERROR_NO_MEMORY,
	ZD_ERROR_VALIDATION_FAILED,
	ZD_ERROR_CRC32_CHECK,
	ZD_ERROR_ZLIB_FAILED,
	ZD_ERROR_ZLIB_CORRUPTED_DATA,
	ZD_ERROR_ZLIB_NO_MEMORY,
	ZD_ERROR_CORRUPTED_DATA,
	ZD_ERROR_INVALID_CALL,
	ZD_ERROR_NOT_IMPLEMENTED,
	ZD_ERROR_FILE_NOT_FOUND,
	ZD_ERROR_DIR_NOT_FOUND,
	ZD_ERROR_NAME_TOO_LONG,
	ZD_ERROR_INVALID_PATH,
	ZD_ERROR_FILE_ALREADY_EXISTS,
	ZD_ERROR_ARCHIVE_TOO_LARGE,
};

// the error describes the reason of the error, as well as the error code, line of code where it happened etc.
struct Error
{
	Error(ErrorEnum _nError, tukk _szDescription, tukk _szFunction, tukk _szFile, unsigned _nLine) :
		nError(_nError),
		m_szDescription(_szDescription),
		szFunction(_szFunction),
		szFile(_szFile),
		nLine(_nLine)
	{
	}

	ErrorEnum   nError;
	tukk getError();

	tukk getDescription() { return m_szDescription; }
	tukk szFunction, * szFile;
	unsigned    nLine;
protected:
	// the description of the error; if needed, will be made as a dynamic string
	tukk m_szDescription;
};

//#define THROW_ZIPDIR_ERROR(ZD_ERR,DESC) throw Error (ZD_ERR, DESC, __FUNCTION__, __FILE__, __LINE__)
#define THROW_ZIPDIR_ERROR(ZD_ERR, DESC)       DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, DESC)
#define THROW_ZIPDIR_FATAL_ERROR(ZD_ERR, DESC) { DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_ERROR, DESC); DrxFatalError(DESC); }

// possible initialization methods
enum InitMethodEnum
{
	// initialize as fast as possible, with minimal validation
	ZD_INIT_FAST,
	// after initialization, scan through all file headers, precache the actual file data offset values and validate the headers
	ZD_INIT_FULL,
	// scan all file headers and try to decompress the data, searching for corrupted files
	ZD_INIT_VALIDATE_IN_MEMORY,
	// store archive in memory
	ZD_INIT_VALIDATE,
	// maximum level of validation, checks for integrity of the archive
	ZD_INIT_VALIDATE_MAX = ZD_INIT_VALIDATE
};

typedef uk (* FnAlloc) (uk pUserData, unsigned nItems, unsigned nSize);
typedef void (*  FnFree)  (uk pUserData, uk pAddress);

// instance of this class just releases the memory when it's destructed
template<class _Heap>
struct TSmartHeapPtr
{
	TSmartHeapPtr(_Heap* pHeap) :
		m_pHeap(pHeap),
		m_pAddress(NULL)
	{
	}
	~TSmartHeapPtr()
	{
		Release();
	}

	void Attach(uk p)
	{
		Release();
		m_pAddress = p;
	}

	uk Detach()
	{
		uk p = m_pAddress;
		m_pAddress = NULL;
		return p;
	}

	void Release()
	{
		if (m_pAddress)
		{
			m_pHeap->FreeTemporary(m_pAddress);
			m_pAddress = NULL;
		}
	}
protected:
	// the pointer to free
	uk  m_pAddress;
	_Heap* m_pHeap;
};

typedef TSmartHeapPtr<CMTSafeHeap> SmartPtr;

// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
// returns one of the Z_* errors (Z_OK upon success)
extern i32 ZipRawUncompress(CMTSafeHeap* pHeap, uk pUncompressed, u64* pDestSize, ukk pCompressed, u64 nSrcSize);

// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
// returns one of the Z_* errors (Z_OK upon success), and the size in *pDestSize. the pCompressed buffer must be at least nSrcSize*1.001+12 size
extern i32 ZipRawCompress(CMTSafeHeap* pHeap, ukk pUncompressed, u64* pDestSize, uk pCompressed, u64 nSrcSize, i32 nLevel);

// fseek wrapper with memory in file support.
extern int64 FSeek(CZipFile* zipFile, int64 origin, i32 command);

// fread wrapper with file in memory  support
extern int64 FRead(CZipFile* zipFile, uk data, size_t nElemSize, size_t nCount);

// ftell wrapper with file in memory support
extern int64  FTell(CZipFile* zipFile);

extern i32    FError(CZipFile* zipFile);
extern i32    FEof(CZipFile* zipFile);

extern u32 FileNameHash(tukk filename);

//////////////////////////////////////////////////////////////////////////
struct SExtraZipFileData
{
	SExtraZipFileData() : nLastModifyTime_Lo(0), nLastModifyTime_Hi(0) {}

	u32 nLastModifyTime_Lo;
	u32 nLastModifyTime_Hi;
};

#ifdef OPTIMIZED_READONLY_ZIP_ENTRY
// this is the record about the file in the Zip file.
struct FileEntry
{
	enum {INVALID_DATA_OFFSET = 0xFFFFFFFF};

	struct SOptimizedDataDescriptor
	{
		u32 lSizeCompressed;    // compressed size                 4 bytes
		u32 lSizeUncompressed;  // uncompressed size               4 bytes
		u32 lCRC32;             // crc-32													4 bytes
	};
	SOptimizedDataDescriptor desc;
	u32                   nFileDataOffset; // offset of the packed info inside the file; NOTE: this can be INVALID_DATA_OFFSET, if not calculated yet!
	u32                   nNameOffset;     // offset of the file name in the name pool for the directory

	u16                   nMethod; // the method of compression (0 if no compression/store)

	FileEntry() {}
	FileEntry(const ZipFile::CDRFileHeader& header, const SExtraZipFileData& extra)
	{
		desc.lSizeCompressed = header.desc.lSizeCompressed;
		desc.lSizeUncompressed = header.desc.lSizeUncompressed;
		desc.lCRC32 = header.desc.lCRC32;
		nFileDataOffset = header.lLocalHeaderOffset + sizeof(ZipFile::LocalFileHeader) + header.nFileNameLength;
		nMethod = header.nMethod;
		nNameOffset = 0;       // we don't know yet
	}

	bool IsInitialized() { return false; }

	// returns the name of this file, given the pointer to the name pool
	tukk GetName(tukk pNamePool) const
	{
		return pNamePool + nNameOffset;
	}
	// Not used in Read-Only version.
	void   OnNewFileData(uk pUncompressed, unsigned nSize, unsigned nCompressedSize, unsigned nCompressionMethod, bool bContinuous) {};
	uint64 GetModificationTime()                                                                                                       { return 0; };

	bool   IsEncrypted() const
	{
		return (
		  nMethod == ZipFile::METHOD_STORE_AND_STREAMCIPHER_KEYTABLE ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_ENCRYPT ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_STREAMCIPHER ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_STREAMCIPHER_KEYTABLE
		  );
	}

	bool IsCompressed() const
	{
		return (
		  nMethod != ZipFile::METHOD_STORE_AND_STREAMCIPHER_KEYTABLE &&
		  nMethod != ZipFile::METHOD_STORE
		  );
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const { /* nothing */ }
};
#else //OPTIMIZED_READONLY_ZIP_ENTRY
// this is the record about the file in the Zip file.
struct FileEntry
{
	enum {INVALID_DATA_OFFSET = 0xFFFFFFFF};

	ZipFile::DataDescriptor desc;
	u32                  nFileDataOffset;   // offset of the packed info inside the file; NOTE: this can be INVALID_DATA_OFFSET, if not calculated yet!
	u32                  nFileHeaderOffset; // offset of the local file header
	u32                  nNameOffset;       // offset of the file name in the name pool for the directory

	u16                  nMethod;    // the method of compression (0 if no compression/store)
	u16                  nReserved0; // Reserved

	// the file modification times
	u16 nLastModTime;
	u16 nLastModDate;
	u32 nNTFS_LastModifyTime_Lo; // Note: Struct is stored in file with 4 byte alignment, cannot use 64-bit type.
	u32 nNTFS_LastModifyTime_Hi;

	// the offset to the start of the next file's header - this
	// can be used to calculate the available space in zip file
	u32 nEOFOffset;

	FileEntry() : nFileHeaderOffset(INVALID_DATA_OFFSET){}
	FileEntry(const ZipFile::CDRFileHeader& header, const SExtraZipFileData& extra);

	bool IsInitialized()
	{
		// structure marked as non-initialized should have nFileHeaderOffset == INVALID_DATA_OFFSET
		return nFileHeaderOffset != INVALID_DATA_OFFSET;
	}
	// returns the name of this file, given the pointer to the name pool
	tukk GetName(tukk pNamePool) const
	{
		return pNamePool + nNameOffset;
	}

	// sets the current time to modification time
	// calculates CRC32 for the new data
	void   OnNewFileData(uk pUncompressed, unsigned nSize, unsigned nCompressedSize, unsigned nCompressionMethod, bool bContinuous);

	uint64 GetModificationTime();

	bool   IsEncrypted() const
	{
		return (
		  nMethod == ZipFile::METHOD_STORE_AND_STREAMCIPHER_KEYTABLE ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_ENCRYPT ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_STREAMCIPHER ||
		  nMethod == ZipFile::METHOD_DEFLATE_AND_STREAMCIPHER_KEYTABLE
		  );
	}

	bool IsCompressed() const
	{
		return (
		  nMethod != ZipFile::METHOD_STORE_AND_STREAMCIPHER_KEYTABLE &&
		  nMethod != ZipFile::METHOD_STORE
		  );
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const { /* nothing */ }
};
#endif //OPTIMIZED_READONLY_ZIP_ENTRY

// tries to refresh the file entry from the given file (reads fromthere if needed)
// returns the error code if the operation was impossible to complete
extern ErrorEnum Refresh(CZipFile* f, FileEntry* pFileEntry);

// writes into the file local header (NOT including the name, only the header structure)
// the file must be opened both for reading and writing
extern ErrorEnum UpdateLocalHeader(FILE* f, FileEntry* pFileEntry);

// writes into the file local header - without Extra data
// puts the new offset to the file data to the file entry
// in case of error can put INVALID_DATA_OFFSET into the data offset field of file entry
extern ErrorEnum WriteLocalHeader(FILE* f, FileEntry* pFileEntry, tukk szRelativePath, bool encrypt);

// conversion routines for the date/time fields used in Zip
extern u16      DOSDate(struct tm*);
extern u16      DOSTime(struct tm*);

extern tukk DOSTimeCStr(u16 nTime);
extern tukk DOSDateCStr(u16 nTime);

struct DirHeader;
// this structure represents a subdirectory descriptor in the directory record.
// it points to the actual directory info (list of its subdirs and files), as well
// as on its name
struct DirEntry
{
	u32 nDirHeaderOffset; // offset, in bytes, relative to this object, of the actual directory record header
	u32 nNameOffset;      // offset of the dir name in the name pool of the parent directory

	// returns the name of this directory, given the pointer to the name pool of hte parent directory
	tukk GetName(tukk pNamePool) const
	{
		return pNamePool + nNameOffset;
	}

	// returns the pointer to the actual directory record.
	// call this function only for the actual structure instance contained in a directory record and
	// followed by the other directory records
	const DirHeader* GetDirectory()  const
	{
		return (const DirHeader*)(((tukk)this) + nDirHeaderOffset);
	}
	DirHeader* GetDirectory()
	{
		return (DirHeader*)(((tuk)this) + nDirHeaderOffset);
	}
};

// this is the head of the directory record
// the name pool follows straight the directory and file entries.
struct DirHeader
{
	u16 numDirs;  // number of directory entries - DirEntry structures
	u16 numFiles; // number of file entries - FileEntry structures

	// returns the pointer to the name pool that follows this object
	// you can only call this method for the structure instance actually followed by the dir record
	tukk GetNamePool() const
	{
		return ((tuk)(this + 1)) + (size_t)this->numDirs * sizeof(DirEntry) + (size_t)this->numFiles * sizeof(FileEntry);
	}
	tuk GetNamePool()
	{
		return ((tuk)(this + 1)) + (size_t)this->numDirs * sizeof(DirEntry) + (size_t)this->numFiles * sizeof(FileEntry);
	}

	// returns the pointer to the i-th directory
	// call this only for the actual instance of the structure at the head of dir record
	const DirEntry* GetSubdirEntry(unsigned i) const
	{
		assert(i < numDirs);
		return ((const DirEntry*)(this + 1)) + i;
	}
	DirEntry* GetSubdirEntry(unsigned i)
	{
		assert(i < numDirs);
		return ((DirEntry*)(this + 1)) + i;
	}

	// returns the pointer to the i-th file
	// call this only for the actual instance of the structure at the head of dir record
	const FileEntry* GetFileEntry(unsigned i) const
	{
		assert(i < numFiles);
		return (const FileEntry*)(((const DirEntry*)(this + 1)) + numDirs) + i;
	}
	FileEntry* GetFileEntry(unsigned i)
	{
		assert(i < numFiles);
		return (FileEntry*)(((DirEntry*)(this + 1)) + numDirs) + i;
	}

	// finds the subdirectory entry by the name, using the names from the name pool
	// assumes: all directories are sorted in alphabetical order.
	// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
	DirEntry* FindSubdirEntry(tukk szName);

	// finds the file entry by the name, using the names from the name pool
	// assumes: all directories are sorted in alphabetical order.
	// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
	FileEntry* FindFileEntry(tukk szName);

};

// this is the sorting predicate for directory entries
struct DirEntrySortPred
{
	DirEntrySortPred(tukk pNamePool) :
		m_pNamePool(pNamePool)
	{
	}

	bool operator()(const FileEntry& left, const FileEntry& right) const
	{
		return strcmp(left.GetName(m_pNamePool), right.GetName(m_pNamePool)) < 0;
	}

	bool operator()(const FileEntry& left, tukk szRight) const
	{
		return strcmp(left.GetName(m_pNamePool), szRight) < 0;
	}

	bool operator()(tukk szLeft, const FileEntry& right) const
	{
		return strcmp(szLeft, right.GetName(m_pNamePool)) < 0;
	}

	bool operator()(const DirEntry& left, const DirEntry& right) const
	{
		return strcmp(left.GetName(m_pNamePool), right.GetName(m_pNamePool)) < 0;
	}

	bool operator()(const DirEntry& left, tukk szName) const
	{
		return strcmp(left.GetName(m_pNamePool), szName) < 0;
	}

	bool operator()(tukk szLeft, const DirEntry& right) const
	{
		return strcmp(szLeft, right.GetName(m_pNamePool)) < 0;
	}

	tukk m_pNamePool;
};

struct UncompressLookahead
{
	enum
	{
		Capacity = 16384
	};

	// Suppress uninitMemberVar for buffer - considered too expensive to initialize
	// cppcheck-suppress uninitMemberVar
	UncompressLookahead()
		: cachedStartIdx(0)
		, cachedEndIdx(0)
	{
	}

	char   buffer[Capacity];
	u32 cachedStartIdx;
	u32 cachedEndIdx;
};

#if !defined(OPTIMIZED_READONLY_ZIP_ENTRY) && defined(SUPPORT_STREAMCIPHER_PAK_ENCRYPTION)
extern void          Encrypt(tuk buffer, size_t size);
extern void          StreamCipher(tuk buffer, size_t size, u32 inKey);
extern void          StreamCipher(tuk buffer, const FileEntry* inEntry);
extern u64 GetStreamCipherKey(const FileEntry* inEntry);
#endif

#if defined(SUPPORT_XTEA_PAK_ENCRYPTION)
extern void Decrypt(tuk buffer, size_t size);
#endif //SUPPORT_XTEA_PAK_ENCRYPTION

}

#endif
