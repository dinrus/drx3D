// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _ZIP_DIR_CACHE_HDR_
#define _ZIP_DIR_CACHE_HDR_

/////////////////////////////////////////////////////////////
// THe Zip Dir uses a special memory layout for keeping the structure of zip file.
// This layout is optimized for small memory footprint (for big zip files)
// and quick binary-search access to the individual files.
//
// The serialized layout consists of a number of directory records.
// Each directory record starts with the DirHeader structure, then
// it has an array of DirEntry structures (sorted by name),
// array of FileEntry structures (sorted by name) and then
// the pool of names, followed by pad bytes to align the whole directory
// record on 4-byte boundray.

struct FileExt;

namespace ZipDir
{

// this is the header of the instance data allocated dynamically
// it contains the FILE* : it owns it and closes upon destruction
struct Cache
{
	void AddRef()        { DrxInterlockedIncrement(m_pRefCount); }
	void Release()       { i32k nRefs = DrxInterlockedDecrement(m_pRefCount); assert(nRefs >= 0); if (nRefs == 0) Delete(); }
	i32  NumRefs() const { return *m_pRefCount; }

	// looks for the given file record in the Central Directory. If there's none, returns NULL.
	// if there is some, returns the pointer to it.
	// the Path must be the relative path to the file inside the Zip
	// if the file handle is passed, it will be used to find the file data offset, if one hasn't been initialized yet
	// if bFull is true, then the full information about the file is returned (the offset to the data may be unknown at this point)-
	// if needed, the file is accessed and the information is loaded
	FileEntry* FindFile(tukk szPath, bool bFullInfo = false);

	// loads the given file into the pCompressed buffer (the actual compressed data)
	// if the pUncompressed buffer is supplied, uncompresses the data there
	// buffers must have enough memory allocated, according to the info in the FileEntry
	// NOTE: there's no need to decompress if the method is 0 (store)
	// returns 0 if successful or error code if couldn't do something
	// when nDataReadSize and nDataOffset are 0, will assume whole file must be read.
	ErrorEnum ReadFile(FileEntry* pFileEntry, uk pCompressed, uk pUncompressed, const bool decompress = true, int64 nDataOffset = 0, int64 nDataReadSize = -1, const bool decrypt = true);
	ErrorEnum ReadFileStreaming(FileEntry* pFileEntry, uk pOut, int64 nDataOffset, int64 nDataReadSize);

	// decompress compressed file
	ErrorEnum DecompressFile(FileEntry* pFileEntry, uk pCompressed, uk pUncompressed, DrxCriticalSection& csDecmopressLock);

	// loads and unpacks the file into a newly created buffer (that must be subsequently freed with
	// Free()) Returns NULL if failed
	uk AllocAndReadFile(FileEntry* pFileEntry);

	// frees the memory block that was previously allocated by AllocAndReadFile
	void Free(uk );

	// refreshes information about the given file entry into this file entry
	ErrorEnum Refresh(FileEntry* pFileEntry);

	// Return FileEntity data offset inside zip file.
	u32      GetFileDataOffset(FileEntry* pFileEntry);

	tukk GetFileEntryName(FileEntry* pFileEntry);

	// returns the root directory record;
	// through this directory record, user can traverse the whole tree
	DirHeader* GetRoot() const        { return m_pRootData; }

	uk      GetDataPointer() const { return (DirHeader*)(this + 1); }

	// returns the size of memory occupied by the instance referred to by this cache
	// must be exact, because it's used by CacheRW to reallocate this cache
	size_t GetSize() const;

	// QUICK check to determine whether the file entry belongs to this object
	bool IsOwnerOf(const FileEntry* pFileEntry) const;

	// returns the string - path to the zip file from which this object was constructed.
	// this will be "" if the object was constructed with a factory that wasn't created with FLAGS_MEMORIZE_ZIP_PATH
	tukk GetFilePath() const
	{
		return ((tukk)(this + 1)) + m_nZipPathOffset;
	}

	ILINE FILE* GetFileHandle() { return m_zipFile.m_file; }

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
	ILINE i32 GetAssetOffset() { return m_zipFile.m_nAssetOffset; }
	ILINE i32 GetAssetLength() { return m_zipFile.m_nAssetLength; }
#endif

	size_t GetZipFileSize() const       { return m_nFileSize; };
	void SetZipFileSize(size_t nSize) { m_nFileSize = nSize; };

	bool ReOpen(tukk filePath);

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		// to account for the full memory, see ZipDir::CacheFactory::MakeCache for this cause of this calculation
		pSizer->AddObject(this, m_pCacheData->m_pHeap->PersistentAllocSize(m_nAllocatedSize));
		pSizer->AddObject(m_pCacheData);
	}

	bool IsInMemory() const
	{
		return m_zipFile.IsInMemory();
	}

	//explicitly sets the priority
	uint64 SetPakFileOffsetOnMedia(uint64 off) { m_nPakFileOffsetOnMedia = off; return m_nPakFileOffsetOnMedia; }
	uint64 GetPakFileOffsetOnMedia()           { return m_nPakFileOffsetOnMedia; }

	//offsets for cdr data in a .pak file for anti-cheat purposes
	void SetCDROffsetSize(u32 offset, u32 size)   { m_cdrOffset = offset; m_cdrSize = size; }
	void GetCDROffsetSize(u32& offset, u32& size) { offset = m_cdrOffset; size = m_cdrSize; }

	friend class CacheFactory; // the factory class creates instances of this class
	friend class CacheRW;      // the Read-Write 2-way cache can modify this cache directly during write operations

public:
#ifndef _RELEASE
	// Validate that other zip cache do not have duplicate file hashes
	static void ValidateFilenameHash(u32 nHash, tukk filename, tukk pakFile);
#endif

	ZipFile::DrxCustomEncryptionHeader& GetEncryptionHeader()                   { return m_headerEncryption; }
	ZipFile::DrxSignedCDRHeader&      GetSignedHeader()                       { return m_headerSignature; }
	ZipFile::DrxCustomExtendedHeader& GetExtendedHeader()                     { return m_headerExtended; }
#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
	u8*                    GetBlockCipherKeyTable(i32k index) { return m_block_cipher_keys_table[index]; }
#endif

protected:
	 i32* m_pRefCount; // the reference count
	CZipFile             m_zipFile;   // the opened file
	uint64               m_nPakFileOffsetOnMedia;

	size_t               m_nFileSize;

	// Flags from Cache Factory
	u32 m_nCacheFactoryFlags;

	// the size of the serialized data following this instance (not including the extra fields after the serialized tree data)
	size_t m_nDataSize;

	// actual size of allocated memory(for correct tracking)
	size_t m_nAllocatedSize;

	// the offset to the path/name of the zip file relative to (tuk)(this+1) pointer in bytes
	size_t m_nZipPathOffset;

	u32 m_cdrOffset;
	u32 m_cdrSize;

	// Zip Headers
	ZipFile::DrxCustomEncryptionHeader m_headerEncryption;
	ZipFile::DrxSignedCDRHeader        m_headerSignature;
	ZipFile::DrxCustomExtendedHeader   m_headerExtended;

#ifdef SUPPORT_RSA_AND_STREAMCIPHER_PAK_ENCRYPTION
	u8 m_block_cipher_keys_table[ZipFile::BLOCK_CIPHER_NUM_KEYS][ZipFile::BLOCK_CIPHER_KEY_LENGTH];
#endif

	DirHeader* m_pRootData;

	// cache internal data
	// need to assemble into one struct to have pointer on it
	struct CacheData
	{
		// compatible with zlib memory-management functions used both
		// to allocate/free this instance and for decompression operations
		CMTSafeHeap* m_pHeap;

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
	#define UNIQUE_LOCK static
#else
	#define UNIQUE_LOCK
#endif
		// file I/O guard: guarantees thread-safe read/seek/write operations and safe header reading.
		// The lock should be globally unique when all pak files are archived in a single obb file.
		UNIQUE_LOCK DrxCriticalSection m_csCacheIOLock;
#undef UNIQUE_LOCK

		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));
		}
	};

	// need to have pointer on the structure
	// because of fixed size of the class
	CacheData* m_pCacheData;
	ZipFile::EHeaderEncryptionType m_encryptedHeaders;
	ZipFile::EHeaderSignatureType m_signedHeaders;

public:
	// initializes the instance structure
	void Construct(CZipFile& fNew, CMTSafeHeap* pHeap, size_t nDataSize, u32 nFactoryFlags, size_t nAllocatedSize);
	void Delete();
	void PreloadToMemory(IMemoryBlock* pMemoryBlock = NULL);
	void UnloadFromMemory();
private:
	// the constructor/destructor cannot be called at all - everything will go through the factory class
	Cache() { m_nCacheFactoryFlags = 0; m_nPakFileOffsetOnMedia = 0; }
	~Cache(){}
};

TYPEDEF_AUTOPTR(Cache);

typedef Cache_AutoPtr CachePtr;

// initializes this object from the given Zip file: caches the central directory
// returns 0 if successfully parsed, error code if an error has occured
CachePtr NewCache(tukk szFilePath, CMTSafeHeap* pHeap, InitMethodEnum nInitMethod = ZD_INIT_FAST);

}

#endif
