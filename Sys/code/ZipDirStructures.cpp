// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/MTSafeAllocator.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx/Core/lib/z/zlib.h>
#include <drx3D/Sys/ZipFileFormat.h>
#include <drx3D/Sys/ZipDirStructures.h>
#include <time.h>
#include <stdlib.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Thread/IJobUpr.h>
#include <drx3D/Sys/DrxPak.h>
#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>

#ifdef SUPPORT_UNBUFFERED_IO
	#include <shlwapi.h>
//LINK_SYSTEM_LIBRARY("libshlwapi.a")
#endif

using namespace ZipFile;

void        ZlibInflateElement_Impl(ukk pCompressed, uk pUncompressed, u64 compressedSize, u64 nUnCompressedSize, u64* pUncompressedSize, i32* pReturnCode, CMTSafeHeap* pHeap);

static void ZlibOverlapInflate(i32* pReturnCode, z_stream* pZStream, ZipDir::UncompressLookahead& lookahead, u8* pOutput, u64 nOutputLen, u8k* pInput, u64 nInputLen)
{
	pZStream->total_in = 0;
	pZStream->avail_in = 0;
	pZStream->avail_out = 0;

	Bytef* pIn = (Bytef*)pInput;
	uInt nIn = nInputLen;
	u8* pOutputEnd = pOutput + nOutputLen;

	uInt nInCursorZ = lookahead.cachedStartIdx;
	uInt nInCursorW = lookahead.cachedEndIdx;

	do
	{
		// Capture some more input

		{
			uInt nWrZ = nInCursorZ % sizeof(lookahead.buffer);
			uInt nWrW = nInCursorW % sizeof(lookahead.buffer);

			uInt nBytesToCache = nWrZ > nWrW
			                     ? nWrZ - nWrW
			                     : sizeof(lookahead.buffer) - nWrW;
			nBytesToCache = min(nBytesToCache, nIn);

			if (nBytesToCache)
			{
				memcpy(lookahead.buffer + nWrW, pIn, nBytesToCache);
				pIn += nBytesToCache;
				nIn -= nBytesToCache;
				nInCursorW += nBytesToCache;
			}
		}

		if (!pZStream->avail_in)
		{
			// Need more input storage - provide it from the local window
			uInt nBytesInWindow = nInCursorW - nInCursorZ;
			if (nBytesInWindow > 0)
			{
				uInt nWrZ = nInCursorZ % sizeof(lookahead.buffer);
				uInt nWrW = nInCursorW % sizeof(lookahead.buffer);
				pZStream->next_in = (Bytef*)lookahead.buffer + nWrZ;
				pZStream->avail_in = nWrW <= nWrZ
				                     ? sizeof(lookahead.buffer) - nWrZ
				                     : nWrW - nWrZ;
			}
			else if (!nIn)
			{
				break;
			}
			else
			{
				__debugbreak();
			}
		}

		pZStream->total_in = 0;

		// Limit so that output doesn't overflow into next read block
		pZStream->avail_out = (uInt)min((UINT_PTR)(pIn - pZStream->next_out), (UINT_PTR)(pOutputEnd - pZStream->next_out));

		i32 nAvailIn = pZStream->avail_in;
		i32 nAvailOut = pZStream->avail_out;

		*pReturnCode = inflate(pZStream, Z_SYNC_FLUSH);

		if (*pReturnCode == Z_BUF_ERROR)
		{
			// As long as we consumed something, keep going. Only fail permanently if we've stalled.
			if (nAvailIn != pZStream->avail_in || nAvailOut != pZStream->avail_out)
			{
				*pReturnCode = Z_OK;
			}
			else if (!nIn)
			{
				*pReturnCode = Z_OK;
				break;
			}
		}

		nInCursorZ += pZStream->total_in;
	}
	while (*pReturnCode == Z_OK);

	lookahead.cachedStartIdx = nInCursorZ;
	lookahead.cachedEndIdx = nInCursorW;
}

extern CMTSafeHeap* g_pPakHeap;

static i32 ZlibInflateIndependentWriteCombined(z_stream* pZS) PREFAST_SUPPRESS_WARNING(6262)
{
	Bytef outputLocal[16384];

	Bytef* pOutRemote = pZS->next_out;
	uInt nOutRemoteLen = pZS->avail_out;

	i32 err = Z_OK;

	while ((err == Z_OK) && (nOutRemoteLen > 0) && (pZS->avail_in > 0))
	{
		pZS->next_out = outputLocal;
		pZS->avail_out = min((uInt)sizeof(outputLocal), nOutRemoteLen);

		err = inflate(pZS, Z_SYNC_FLUSH);

		i32 nEmitted = (i32)(pZS->next_out - outputLocal);
		memcpy(pOutRemote, outputLocal, nEmitted);
		pOutRemote += nEmitted;
		nOutRemoteLen -= nEmitted;

		if ((err == Z_BUF_ERROR) && (nEmitted > 0))
			err = Z_OK;
	}

	pZS->next_out = pOutRemote;
	pZS->avail_out = nOutRemoteLen;

	return err;
}

//Works on an initialized z_stream, is responsible for calling ...init and ...end
void ZlibInflateElementPartial_Impl(
  i32* pReturnCode, z_stream* pZStream, ZipDir::UncompressLookahead* pLookahead,
  u8* pOutput, u64 nOutputLen, bool bOutputWriteOnly,
  u8k* pInput, u64 nInputLen, u64* pTotalOut) PREFAST_SUPPRESS_WARNING(6262)
{
	z_stream localStream;
	bool bUsingLocal = false;

	if (!pZStream)
	{
		memset(&localStream, 0, sizeof(localStream));
		pZStream = &localStream;
		bUsingLocal = true;
	}

	if (pZStream->total_out == 0)
	{
		*pReturnCode = inflateInit2(pZStream, -MAX_WBITS);

		if (*pReturnCode != Z_OK)
		{
			return;
		}
	}

	pZStream->next_out = pOutput;
	pZStream->avail_out = nOutputLen;

	// If src/dst overlap (in place decompress), then inflate in chunks, copying src locally to ensure
	// pointers don't foul each other.
	bool bIndependantBlocks = ((pInput + nInputLen) <= pOutput) || (pInput >= (pOutput + nOutputLen));
	if (bIndependantBlocks)
	{
		pZStream->next_in = (Bytef*)pInput;
		pZStream->avail_in = nInputLen;

		if (!bOutputWriteOnly)
			*pReturnCode = inflate(pZStream, Z_SYNC_FLUSH);
		else
			*pReturnCode = ZlibInflateIndependentWriteCombined(pZStream);
	}
	else if (pLookahead)
	{
		ZlibOverlapInflate(pReturnCode, pZStream, *pLookahead, pOutput, nOutputLen, pInput, nInputLen);
	}
	else
	{
		ZipDir::UncompressLookahead lookahead;
		ZlibOverlapInflate(pReturnCode, pZStream, lookahead, pOutput, nOutputLen, pInput, nInputLen);
	}

	if (pTotalOut)
	{
		*pTotalOut = pZStream->total_out;
	}

	//error during inflate
	if (*pReturnCode != Z_STREAM_END && *pReturnCode != Z_OK)
	{
#ifndef _RELEASE
		__debugbreak();
#endif

		inflateEnd(pZStream);
		return;
	}

	//check if we have finished the read
	if (*pReturnCode == Z_STREAM_END)
	{
		inflateEnd(pZStream);
	}
	else if (bUsingLocal)
	{
#ifndef _RELEASE
		__debugbreak();
#endif
		*pReturnCode = Z_VERSION_ERROR;
	}
}

// function to do the zlib decompression, on SPU, we use ZlibInflateElement, which is defined in zlib_spu (which in case only forward to edge zlib decompression)
void ZlibInflateElement_Impl(ukk pCompressed, uk pUncompressed, u64 compressedSize, u64 nUnCompressedSize, u64* pUncompressedSize, i32* pReturnCode, CMTSafeHeap* pHeap) PREFAST_SUPPRESS_WARNING(6262)
{
	DRX_PROFILE_FUNCTION(PROFILE_SYSTEM);
	i32 err;
	z_stream stream;

	Bytef* pIn = (Bytef*)pCompressed;
	uInt nIn = (uInt)compressedSize;

	stream.next_out = (Bytef*)pUncompressed;
	stream.avail_out = (uInt)nUnCompressedSize;

	stream.zalloc = CMTSafeHeap::StaticAlloc;
	stream.zfree = CMTSafeHeap::StaticFree;
	stream.opaque = pHeap;

	err = inflateInit2(&stream, -MAX_WBITS);
	if (err != Z_OK)
	{
		*pReturnCode = err;
		return;
	}

	// If src/dst overlap (in place decompress), then inflate in chunks, copying src locally to ensure
	// pointers don't foul each other.
	bool bIndependantBlocks = ((pIn + nIn) <= stream.next_out) || (pIn >= (stream.next_out + stream.avail_out));
	if (bIndependantBlocks)
	{
		stream.next_in = pIn;
		stream.avail_in = nIn;

		// for some strange reason, passing Z_FINISH doesn't work -
		// it seems the stream isn't finished for some files and
		// inflate returns an error due to stream-end-not-reached (though expected) problem
		err = inflate(&stream, Z_SYNC_FLUSH);
	}
	else
	{
		ZipDir::UncompressLookahead lookahead;
		ZlibOverlapInflate(&err, &stream, lookahead, (u8*)pUncompressed, nUnCompressedSize, (u8*)pCompressed, compressedSize);
	}

	if (err != Z_STREAM_END && err != Z_OK)
	{
		inflateEnd(&stream);
		*pReturnCode = err == Z_OK ? Z_BUF_ERROR : err;
		return;
	}

	*pUncompressedSize = stream.total_out;

	err = inflateEnd(&stream);
	*pReturnCode = err;
}

//////////////////////////////////////////////////////////////////////////
void ZipDir::CZipFile::LoadToMemory(IMemoryBlock* pData)
{
	LOADING_TIME_PROFILE_SECTION;

	if (!m_pInMemoryData)
	{
		tukk szUsage = "In Memory Zip File";
		m_nCursor = 0;

		if (pData)
		{
			m_pInMemoryData = (ICustomMemoryBlock*) pData;
			m_pInMemoryData->AddRef();

			m_nSize = pData->GetSize();
		}
		else
		{
			FILE* realfile = m_file;
			size_t nFileSize = ~0;
			int64 offset = 0;

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
			if (InPakFileOrAsset())
			{
				realfile = m_pPakFileRef;
				nFileSize = m_pPakFileEntry->desc.lSizeCompressed;
			}
			else
			{
				if (!IsAssetFile())
				{
#endif
			if (CIOWrapper::Fseek(realfile, 0, SEEK_END) != 0)
				goto error;

			nFileSize = (size_t)CIOWrapper::FTell(realfile);
			if (nFileSize == -1L)
				goto error;
#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
		}
		else
		{
			nFileSize = m_nAssetLength;
		}
	}
#endif

			m_pInMemoryData = ((CDrxPak*)(gEnv->pDrxPak))->GetInMemoryPakHeap()->AllocateBlock(nFileSize, szUsage);
			m_pInMemoryData->AddRef();

			m_nSize = nFileSize;

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
			if (InPakFileOrAsset())
			{
				offset = GetFileDataOffset();
			}
#endif
			if (CIOWrapper::Fseek(realfile, offset, SEEK_SET) != 0)
				goto error;
			if (CIOWrapper::Fread(m_pInMemoryData->GetData(), nFileSize, 1, realfile) != 1)
				goto error;

			return;
error:
			m_nSize = 0;
			if (m_pInMemoryData)
				m_pInMemoryData->Release();
			m_pInMemoryData = NULL;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ZipDir::CZipFile::UnloadFromMemory()
{
	SAFE_RELEASE(m_pInMemoryData);
}

//////////////////////////////////////////////////////////////////////////
void ZipDir::CZipFile::Close(bool bUnloadFromMem)
{
	if (m_file)
	{
		CIOWrapper::Fclose(m_file);
		m_file = NULL;
	}

#ifdef SUPPORT_UNBUFFERED_IO
	if (m_unbufferedFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_unbufferedFile);
		m_unbufferedFile = INVALID_HANDLE_VALUE;
	}

	if (m_pReadTarget)
	{
		_aligned_free(m_pReadTarget);
		m_pReadTarget = NULL;
	}
#endif

	if (bUnloadFromMem)
	{
		UnloadFromMemory();
	}

	if (!m_pInMemoryData)
	{
		m_nCursor = 0;
		m_nSize = 0;
	}
}

#ifdef SUPPORT_UNBUFFERED_IO
bool ZipDir::CZipFile::OpenUnbuffered(tukk filename)
{
	if (!EvaluateSectorSize(filename))
		return false;

	HANDLE unbufferedFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);

	if (unbufferedFile != INVALID_HANDLE_VALUE)
	{
		m_unbufferedFile = unbufferedFile;

		return true;
	}

	return false;
}

bool ZipDir::CZipFile::EvaluateSectorSize(tukk filename)
{
	char volume[_MAX_PATH];

	if (PathIsRelativeA(filename))
	{
		_fullpath(volume, filename, sizeof(volume));
	}
	else
	{
		drx_strcpy(volume, filename);
	}

	if (!PathStripToRoot(volume))
	{
		return false;
	}

	DWORD bytesPerSector;
	if (GetDiskFreeSpaceA(volume, NULL, &bytesPerSector, NULL, NULL))
	{
		m_nSectorSize = bytesPerSector;
		if (m_pReadTarget)
			_aligned_free(m_pReadTarget);
		m_pReadTarget = _aligned_malloc(128 * 1024, m_nSectorSize);
		return true;
	}

	return false;
}
#endif

#if DRX_PLATFORM_ANDROID && defined(ANDROID_OBB)
u32 ZipDir::CZipFile::GetFileDataOffset() const
{
	if (InPakFileOrAsset() || IsAssetFile())
	{
		if (InPakFileOrAsset())
		{
			assert(m_pPakFileEntry->nFileDataOffset != ZipDir::FileEntry::INVALID_DATA_OFFSET);
			return m_pPakFileEntry->nFileDataOffset + m_nAssetOffset;
		}
		return m_nAssetOffset;
	}
	else
	{
		return 0;
	}
}
#endif

#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
//////////////////////////////////////////////////////////////////////////
ZipDir::FileEntry::FileEntry(const CDRFileHeader& header, const SExtraZipFileData& extra)
{
	this->desc = header.desc;
	this->nFileHeaderOffset = header.lLocalHeaderOffset;
	//this->nFileDataOffset   = INVALID_DATA_OFFSET; // we don't know yet
	this->nMethod = header.nMethod;
	this->nNameOffset = 0;       // we don't know yet
	this->nLastModTime = header.nLastModTime;
	this->nLastModDate = header.nLastModDate;
	this->nNTFS_LastModifyTime_Lo = extra.nLastModifyTime_Lo;
	this->nNTFS_LastModifyTime_Hi = extra.nLastModifyTime_Hi;

	// make an estimation (at least this offset should be there), but we don't actually know yet
	this->nFileDataOffset = header.lLocalHeaderOffset + sizeof(ZipFile::LocalFileHeader) + header.nFileNameLength;
	this->nEOFOffset = header.lLocalHeaderOffset + sizeof(ZipFile::LocalFileHeader) + header.nFileNameLength + header.desc.lSizeCompressed;
}
#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

// Uncompresses raw (without wrapping) data that is compressed with method 8 (deflated) in the Zip file
// returns one of the Z_* errors (Z_OK upon success)
// This function just mimics the standard uncompress (with modification taken from unzReadCurrentFile)
// with 2 differences: there are no 16-bit checks, and
// it initializes the inflation to start without waiting for compression method byte, as this is the
// way it's stored into zip file
i32 ZipDir::ZipRawUncompress(CMTSafeHeap* pHeap, uk pUncompressed, u64* pDestSize, ukk pCompressed, u64 nSrcSize)
{
	LOADING_TIME_PROFILE_SECTION(gEnv->pSystem);
	i32 nReturnCode;

	ZlibInflateElement_Impl(pCompressed, pUncompressed, nSrcSize, *pDestSize, pDestSize, &nReturnCode, pHeap);

	return nReturnCode;
}

// compresses the raw data into raw data. The buffer for compressed data itself with the heap passed. Uses method 8 (deflate)
// returns one of the Z_* errors (Z_OK upon success)
i32 ZipDir::ZipRawCompress(CMTSafeHeap* pHeap, ukk pUncompressed, u64* pDestSize, uk pCompressed, u64 nSrcSize, i32 nLevel)
{
	z_stream stream;
	i32 err;

	stream.next_in = (Bytef*)pUncompressed;
	stream.avail_in = (uInt)nSrcSize;

	stream.next_out = (Bytef*)pCompressed;
	stream.avail_out = (uInt) * pDestSize;

	stream.zalloc = CMTSafeHeap::StaticAlloc;
	stream.zfree = CMTSafeHeap::StaticFree;
	stream.opaque = pHeap;

	err = deflateInit2(&stream, nLevel, Z_DEFLATED, -MAX_WBITS, 9, Z_DEFAULT_STRATEGY);
	if (err != Z_OK) return err;

	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*pDestSize = stream.total_out;

	err = deflateEnd(&stream);
	return err;
}

// finds the subdirectory entry by the name, using the names from the name pool
// assumes: all directories are sorted in alphabetical order.
// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
ZipDir::DirEntry* ZipDir::DirHeader::FindSubdirEntry(tukk szName)
{
	if (this->numDirs)
	{
		tukk pNamePool = GetNamePool();
		DirEntrySortPred pred(pNamePool);
		DirEntry* pBegin = GetSubdirEntry(0);
		DirEntry* pEnd = pBegin + this->numDirs;
		DirEntry* pEntry = std::lower_bound(pBegin, pEnd, szName, pred);
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
		if (pEntry != pEnd && !strcasecmp(szName, pEntry->GetName(pNamePool)))
#else
		if (pEntry != pEnd && !strcmp(szName, pEntry->GetName(pNamePool)))
#endif
			return pEntry;
	}
	return NULL;
}

// finds the file entry by the name, using the names from the name pool
// assumes: all directories are sorted in alphabetical order.
// case-sensitive (must be lower-case if case-insensitive search in Win32 is performed)
ZipDir::FileEntry* ZipDir::DirHeader::FindFileEntry(tukk szName)
{
	if (this->numFiles)
	{
		tukk pNamePool = GetNamePool();
		DirEntrySortPred pred(pNamePool);
		FileEntry* pBegin = GetFileEntry(0);
		FileEntry* pEnd = pBegin + this->numFiles;
		FileEntry* pEntry = std::lower_bound(pBegin, pEnd, szName, pred);
#if DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
		if (pEntry != pEnd && !strcasecmp(szName, pEntry->GetName(pNamePool)))
#else
		if (pEntry != pEnd && !strcmp(szName, pEntry->GetName(pNamePool)))
#endif
			return pEntry;
	}
	return NULL;
}

// tries to refresh the file entry from the given file (reads fromthere if needed)
// returns the error code if the operation was impossible to complete
ZipDir::ErrorEnum ZipDir::Refresh(CZipFile* f, FileEntry* pFileEntry)
{
#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

	if (pFileEntry->nFileDataOffset != pFileEntry->INVALID_DATA_OFFSET)
		return ZD_ERROR_SUCCESS;

	if (ZipDir::FSeek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		//#if DRX_PLATFORM_WINDOWS
		//  if (_fseeki64(f, (__int64)pFileEntry->nFileHeaderOffset, SEEK_SET))
		//#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID
		//	if (fseeko(f, (off_t)(pFileEntry->nFileHeaderOffset), SEEK_SET))
		//#else
		//  if (fseek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
		//#endif
		return ZD_ERROR_IO_FAILED;

	// No validation in release or profile!

	// read the local file header and the name (for validation) into the buffer
	LocalFileHeader fileHeader;
	//if (1 != fread (&fileHeader, sizeof(fileHeader), 1, f))
	if (1 != ZipDir::FRead(f, &fileHeader, sizeof(fileHeader), 1))
		return ZD_ERROR_IO_FAILED;
	SwapEndian(fileHeader);

	if (fileHeader.desc != pFileEntry->desc
	    || fileHeader.nMethod != pFileEntry->nMethod)
	{
		char szErrDesc[1024];
		drx_sprintf(szErrDesc, "ERROR: File header doesn't match previously cached file entry record (%s) \n fileheader desc=(%u,%u,%u), method=%u, \n fileentry desc=(%u,%u,%u),method=%u",
		            "Unknown" /*f->_tmpfname*/,
		            fileHeader.desc.lCRC32, fileHeader.desc.lSizeCompressed, fileHeader.desc.lSizeUncompressed,
		            fileHeader.nMethod,
		            pFileEntry->desc.lCRC32, pFileEntry->desc.lSizeCompressed, pFileEntry->desc.lSizeUncompressed,
		            pFileEntry->nMethod);
		DrxFatalError("%s", szErrDesc);

		return ZD_ERROR_IO_FAILED;
		//DrxFatalError(szErrDesc);
		//THROW_ZIPDIR_ERROR(ZD_ERROR_VALIDATION_FAILED,szErrDesc);
	}
	pFileEntry->nFileDataOffset = pFileEntry->nFileHeaderOffset + sizeof(LocalFileHeader) + fileHeader.nFileNameLength + fileHeader.nExtraFieldLength;
	pFileEntry->nEOFOffset = pFileEntry->nFileDataOffset + pFileEntry->desc.lSizeCompressed;

#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

	return ZD_ERROR_SUCCESS;
}

// writes into the file local header (NOT including the name, only the header structure)
// the file must be opened both for reading and writing
ZipDir::ErrorEnum ZipDir::UpdateLocalHeader(FILE* f, FileEntry* pFileEntry)
{
#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

	#if DRX_PLATFORM_WINDOWS
	if (_fseeki64(f, (__int64)pFileEntry->nFileHeaderOffset, SEEK_SET))
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	if (fseeko(f, (off_t)(pFileEntry->nFileHeaderOffset), SEEK_SET))
	#else
	if (fseek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
	#endif
		return ZD_ERROR_IO_FAILED;

	LocalFileHeader h;
	if (1 != fread(&h, sizeof(h), 1, f))
		return ZD_ERROR_IO_FAILED;
	SwapEndian(h);

	assert(h.lSignature == h.SIGNATURE);

	h.desc.lCRC32 = pFileEntry->desc.lCRC32;
	h.desc.lSizeCompressed = pFileEntry->desc.lSizeCompressed;
	h.desc.lSizeUncompressed = pFileEntry->desc.lSizeUncompressed;
	h.nMethod = pFileEntry->nMethod;
	h.nFlags &= ~GPF_ENCRYPTED; // we don't support encrypted files

	#if DRX_PLATFORM_WINDOWS
	if (_fseeki64(f, (__int64)pFileEntry->nFileHeaderOffset, SEEK_SET))
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	if (fseeko(f, (off_t)(pFileEntry->nFileHeaderOffset), SEEK_SET))
	#else
	if (fseek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
	#endif
		return ZD_ERROR_IO_FAILED;

	if (1 != fwrite(&h, sizeof(h), 1, f))
		return ZD_ERROR_IO_FAILED;

#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
	return ZD_ERROR_SUCCESS;
}

// writes into the file local header - without Extra data
// puts the new offset to the file data to the file entry
// in case of error can put INVALID_DATA_OFFSET into the data offset field of file entry
ZipDir::ErrorEnum ZipDir::WriteLocalHeader(FILE* f, FileEntry* pFileEntry, tukk szRelativePath, bool encrypt)
{
#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
	size_t nFileNameLength = strlen(szRelativePath);
	size_t nHeaderSize = sizeof(LocalFileHeader) + nFileNameLength;

	pFileEntry->nFileDataOffset = pFileEntry->nFileHeaderOffset + nHeaderSize;
	pFileEntry->nEOFOffset = pFileEntry->nFileDataOffset + pFileEntry->desc.lSizeCompressed;

	#if DRX_PLATFORM_WINDOWS
	if (_fseeki64(f, (__int64)pFileEntry->nFileHeaderOffset, SEEK_SET))
	#elif DRX_PLATFORM_LINUX || DRX_PLATFORM_ANDROID || DRX_PLATFORM_APPLE
	if (fseeko(f, (off_t)(pFileEntry->nFileHeaderOffset), SEEK_SET))
	#else
	if (fseek(f, pFileEntry->nFileHeaderOffset, SEEK_SET))
	#endif
		return ZD_ERROR_IO_FAILED;

	if (encrypt)
	{
		std::vector<u8> garbage;
		garbage.resize(nHeaderSize);
		for (size_t i = 0; i < nHeaderSize; ++i)
		{
			garbage[i] = drx_random_uint32() & 0xff;
		}

		if (fwrite(&garbage[0], nHeaderSize, 1, f) != 1)
		{
			return ZD_ERROR_IO_FAILED;
		}
	}
	else
	{
		LocalFileHeader h;
		memset(&h, 0, sizeof(h));

		h.lSignature = LocalFileHeader::SIGNATURE;
		h.nVersionNeeded = 10;
		h.nFlags = 0;
		h.nMethod = pFileEntry->nMethod;
		h.nLastModDate = pFileEntry->nLastModDate;
		h.nLastModTime = pFileEntry->nLastModTime;
		h.desc = pFileEntry->desc;
		h.nFileNameLength = (unsigned short)strlen(szRelativePath);
		h.nExtraFieldLength = 0;

		pFileEntry->nFileDataOffset = pFileEntry->nFileHeaderOffset + sizeof(h) + h.nFileNameLength;
		pFileEntry->nEOFOffset = pFileEntry->nFileDataOffset + pFileEntry->desc.lSizeCompressed;

		if (1 != fwrite(&h, sizeof(h), 1, f))
			return ZD_ERROR_IO_FAILED;

		if (nFileNameLength > 0)
		{
			if (1 != fwrite(szRelativePath, nFileNameLength, 1, f))
				return ZD_ERROR_IO_FAILED;
		}
	}

#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

	return ZD_ERROR_SUCCESS;
}

// conversion routines for the date/time fields used in Zip
u16 ZipDir::DOSDate(tm* t)
{
	return
	  ((t->tm_year - 80) << 9)
	  | (t->tm_mon << 5)
	  | t->tm_mday;
}

u16 ZipDir::DOSTime(tm* t)
{
	return
	  ((t->tm_hour) << 11)
	  | ((t->tm_min) << 5)
	  | ((t->tm_sec) >> 1);
}

#ifndef OPTIMIZED_READONLY_ZIP_ENTRY
// sets the current time to modification time
// calculates CRC32 for the new data
void ZipDir::FileEntry::OnNewFileData(uk pUncompressed, unsigned nSize, unsigned nCompressedSize, unsigned nCompressionMethod, bool bContinuous)
{
	time_t nTime;
	time(&nTime);
	tm* t = localtime(&nTime);

	// While local time converts the month to a 0 to 11 interval...
	// ...the pack file expects months from 1 to 12...
	// Therefore, for correct date, we have to do t->tm_mon+=1;
	t->tm_mon += 1;

	this->nLastModTime = DOSTime(t);
	this->nLastModDate = DOSDate(t);

	#if DRX_PLATFORM_WINDOWS
	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);
	LARGE_INTEGER lt;
	lt.HighPart = ft.dwHighDateTime;
	lt.LowPart = ft.dwLowDateTime;
	this->nNTFS_LastModifyTime_Lo = lt.LowPart;
	this->nNTFS_LastModifyTime_Hi = lt.HighPart;
	#endif

	if (!bContinuous)
	{
		this->desc.lCRC32 = crc32(0L, Z_NULL, 0);
		this->desc.lSizeCompressed = nCompressedSize;
		this->desc.lSizeUncompressed = nSize;
	}

	// we'll need CRC32 of the file to pack it
	this->desc.lCRC32 = crc32(this->desc.lCRC32, (Bytef*)pUncompressed, nSize);

	this->nMethod = nCompressionMethod;
}

uint64 ZipDir::FileEntry::GetModificationTime()
{
	if (nNTFS_LastModifyTime_Lo != 0 && nNTFS_LastModifyTime_Hi != 0)
		return static_cast<uint64>(nNTFS_LastModifyTime_Lo) + (static_cast<uint64>(nNTFS_LastModifyTime_Hi) << 32);

	// TODO/TIME: check and test
	SYSTEMTIME st;
	st.wYear = (nLastModDate >> 9) + 1980;
	st.wMonth = ((nLastModDate >> 5) & 0xF);
	st.wDay = (nLastModDate & 0x1F);
	st.wHour = (nLastModTime >> 11);
	st.wMinute = (nLastModTime >> 5) & 0x3F;
	st.wSecond = (nLastModTime << 1) & 0x3F;
	st.wMilliseconds = 0;
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	LARGE_INTEGER lt;
	lt.HighPart = ft.dwHighDateTime;
	lt.LowPart = ft.dwLowDateTime;
	return lt.QuadPart;
}
#endif //#ifndef OPTIMIZED_READONLY_ZIP_ENTRY

tukk ZipDir::DOSTimeCStr(u16 nTime)
{
	static char szBuf[16];
	drx_sprintf(szBuf, "%02d:%02d.%02d", (nTime >> 11), ((nTime & ((1 << 11) - 1)) >> 5), ((nTime & ((1 << 5) - 1)) << 1));
	return szBuf;
}

tukk ZipDir::DOSDateCStr(u16 nTime)
{
	static char szBuf[32];
	drx_sprintf(szBuf, "%02d.%02d.%04d", (nTime & 0x1F), (nTime >> 5) & 0xF, (nTime >> 9) + 1980);
	return szBuf;
}

tukk ZipDir::Error::getError()
{
	switch (this->nError)
	{
#define DECLARE_ERROR(x) case ZD_ERROR_ ## x: \
  return # x;
		DECLARE_ERROR(SUCCESS);
		DECLARE_ERROR(IO_FAILED);
		DECLARE_ERROR(UNEXPECTED);
		DECLARE_ERROR(UNSUPPORTED);
		DECLARE_ERROR(INVALID_SIGNATURE);
		DECLARE_ERROR(ZIP_FILE_IS_CORRUPT);
		DECLARE_ERROR(DATA_IS_CORRUPT);
		DECLARE_ERROR(NO_CDR);
		DECLARE_ERROR(CDR_IS_CORRUPT);
		DECLARE_ERROR(NO_MEMORY);
		DECLARE_ERROR(VALIDATION_FAILED);
		DECLARE_ERROR(CRC32_CHECK);
		DECLARE_ERROR(ZLIB_FAILED);
		DECLARE_ERROR(ZLIB_CORRUPTED_DATA);
		DECLARE_ERROR(ZLIB_NO_MEMORY);
		DECLARE_ERROR(CORRUPTED_DATA);
		DECLARE_ERROR(INVALID_CALL);
		DECLARE_ERROR(NOT_IMPLEMENTED);
		DECLARE_ERROR(FILE_NOT_FOUND);
		DECLARE_ERROR(DIR_NOT_FOUND);
		DECLARE_ERROR(NAME_TOO_LONG);
		DECLARE_ERROR(INVALID_PATH);
		DECLARE_ERROR(FILE_ALREADY_EXISTS);
#undef DECLARE_ERROR
	default:
		return "Unknown ZD_ERROR code";
	}
}

//////////////////////////////////////////////////////////////////////////
u32 ZipDir::FileNameHash(tukk filename)
{
	stack_string str = filename;
	str.replace('/', '\\');

	uLong uCRC32 = crc32(0L, Z_NULL, 0);
	uCRC32 = crc32(uCRC32, (Bytef*)str.c_str(), str.length());
	return uCRC32;
}

//////////////////////////////////////////////////////////////////////////
inline void btea(u32* v, i32 n, u32 const k[4])
{
#define TEA_DELTA 0x9e3779b9
#define TEA_MX    (((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4)) ^ ((sum ^ y) + (k[(p & 3) ^ e] ^ z)))
	u32 y, z, sum;
	unsigned p, rounds, e;
	if (n > 1)            /* Coding Part */
	{
		rounds = 6 + 52 / n;
		sum = 0;
		z = v[n - 1];
		do
		{
			sum += TEA_DELTA;
			e = (sum >> 2) & 3;
			for (p = 0; p < u32(n - 1); p++)
			{
				y = v[p + 1];
				z = v[p] += TEA_MX;
			}
			y = v[0];
			z = v[n - 1] += TEA_MX;
		}
		while (--rounds);
	}
	else if (n < -1)      /* Decoding Part */
	{
		n = -n;
		rounds = 6 + 52 / n;
		sum = rounds * TEA_DELTA;
		y = v[0];
		do
		{
			e = (sum >> 2) & 3;
			for (p = u32(n - 1); p > 0; p--)
			{
				z = v[p - 1];
				y = v[p] -= TEA_MX;
			}
			z = v[n - 1];
			y = v[0] -= TEA_MX;
		}
		while ((sum -= TEA_DELTA) != 0);
	}
#undef TEA_DELTA
#undef TEA_MX
}

//////////////////////////////////////////////////////////////////////////
inline void SwapByteOrder(u32* values, size_t count)
{
	for (u32* w = values, * e = values + count; w != e; ++w)
	{
		*w = (*w >> 24) + ((*w >> 8) & 0xff00) + ((*w & 0xff00) << 8) + (*w << 24);
	}
}

//////////////////////////////////////////////////////////////////////////
#if !defined(OPTIMIZED_READONLY_ZIP_ENTRY) && defined(SUPPORT_XTEA_PAK_ENCRYPTION)
void ZipDir::Encrypt(tuk buffer, size_t size)
{
	u32k preciousData[4] = { 0xc968fb67, 0x8f9b4267, 0x85399e84, 0xf9b99dc4 };

	u32* intBuffer = (u32*)buffer;
	i32k encryptedLen = size >> 2;

	SwapByteOrder(intBuffer, encryptedLen);

	btea(intBuffer, encryptedLen, preciousData);

	SwapByteOrder(intBuffer, encryptedLen);
}
#endif

//////////////////////////////////////////////////////////////////////////
#if defined(SUPPORT_XTEA_PAK_ENCRYPTION)
void ZipDir::Decrypt(tuk buffer, size_t size)
{
	u32k preciousData[4] = { 0xc968fb67, 0x8f9b4267, 0x85399e84, 0xf9b99dc4 };

	u32* intBuffer = (u32*)buffer;
	i32k encryptedLen = size >> 2;

	SwapByteOrder(intBuffer, encryptedLen);

	btea(intBuffer, -encryptedLen, preciousData);

	SwapByteOrder(intBuffer, encryptedLen);
}
#endif //SUPPORT_XTEA_PAK_ENCRYPTION

////////////////////////////////////////////////////////////////////////// STREAM CIPHER COPY
// stream cipher is in drxnetwork, but we need to use it before drxnetwork is initialized
// copy and paste code here for now, not worth refactoring stuff
class CStreamCipherCopy
{
public:
	void Init(u8k* key, i32 keyLen);
	void Encrypt(u8k* input, i32 inputLen, u8* output)       { ProcessBuffer(input, inputLen, output, true); }
	void Decrypt(u8k* input, i32 inputLen, u8* output)       { ProcessBuffer(input, inputLen, output, true); }
	void EncryptStream(u8k* input, i32 inputLen, u8* output) { ProcessBuffer(input, inputLen, output, false); }
	void DecryptStream(u8k* input, i32 inputLen, u8* output) { ProcessBuffer(input, inputLen, output, false); }

private:
	u8 GetNext();
	void  ProcessBuffer(u8k* input, i32 inputLen, u8* output, bool resetKey);

	u8 m_StartS[256];
	u8 m_S[256];
	i32 m_StartI;
	i32 m_I;
	i32 m_StartJ;
	i32 m_J;
};

void CStreamCipherCopy::Init(u8k* key, i32 keyLen)
{
	i32 i, j;

	for (i = 0; i < 256; i++)
	{
		m_S[i] = i;
	}

	if (key)
	{
		for (i = j = 0; i < 256; i++)
		{
			u8 temp;

			j = (j + key[i % keyLen] + m_S[i]) & 255;
			temp = m_S[i];
			m_S[i] = m_S[j];
			m_S[j] = temp;
		}
	}

	m_I = m_J = 0;

	for (i = 0; i < 1024; i++)
	{
		GetNext();
	}

	memcpy(m_StartS, m_S, sizeof(m_StartS));

	m_StartI = m_I;
	m_StartJ = m_J;
}

u8 CStreamCipherCopy::GetNext()
{
	u8 tmp;

	m_I = (m_I + 1) & 0xff;
	m_J = (m_J + m_S[m_I]) & 0xff;

	tmp = m_S[m_J];
	m_S[m_J] = m_S[m_I];
	m_S[m_I] = tmp;

	return m_S[(m_S[m_I] + m_S[m_J]) & 0xff];
}

void CStreamCipherCopy::ProcessBuffer(u8k* input, i32 inputLen, u8* output, bool resetKey)
{
	if (resetKey)
	{
		memcpy(m_S, m_StartS, sizeof(m_S));
		m_I = m_StartI;
		m_J = m_StartJ;
	}

	for (i32 i = 0; i < inputLen; i++)
	{
		output[i] = input[i] ^ GetNext();
	}
}

#if !defined(OPTIMIZED_READONLY_ZIP_ENTRY) && defined(SUPPORT_STREAMCIPHER_PAK_ENCRYPTION)
////////////////////////////////////////////////////////////////////////// STREAM CIPHER COPY

//////////////////////////////////////////////////////////////////////////
void ZipDir::StreamCipher(tuk buffer, size_t size, u32 inKey)
{
	CStreamCipherCopy cipher;
	cipher.Init((u8k*)&inKey, sizeof(inKey));
	cipher.EncryptStream((u8*)buffer, size, (u8*)buffer);
}

//////////////////////////////////////////////////////////////////////////
void ZipDir::StreamCipher(tuk buffer, const FileEntry* inEntry)
{
	StreamCipher(buffer, inEntry->desc.lSizeCompressed, GetStreamCipherKey(inEntry));
}
#endif  //!OPTIMIZED_READONLY_ZIP_ENTRY && SUPPORT_STREAMCIPHER_PAK_ENCRYPTION

// TypeInfo implementations for DinrusSystem
#include <drx3D/CoreX/TypeInfo_impl.h>
#include <drx3D/Sys/ZipFileFormat_info.h>
