// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

#ifdef INCLUDE_SCALEFORM_SDK

	#include "GFileDrxPak.h"
	#include <drx3D/Sys/SharedStates.h>

	#include <drx3D/Sys/ISystem.h>
	#include <drx3D/Sys/IDrxPak.h>
	#include <drx3D/Sys/IStreamEngine.h>

GFileDrxPak::GFileDrxPak(tukk pPath, u32 openFlags)
	: GFile()
	, m_pPak(0)
	, m_fileHandle(0)
	, m_relativeFilePath(pPath)
{
	m_pPak = gEnv->pDrxPak;
	assert(m_pPak);
	m_fileHandle = m_pPak->FOpen(pPath, "rb", openFlags);
}

GFileDrxPak::~GFileDrxPak()
{
	if (IsValidInternal())
		Close();
}

tukk GFileDrxPak::GetFilePath()
{
	return m_relativeFilePath.ToCStr();
}

bool GFileDrxPak::IsValid()
{
	return IsValidInternal();
}

bool GFileDrxPak::IsWritable()
{
	// writable files are not supported!
	return false;
}

inline long GFileDrxPak::TellInternal()
{
	if (IsValidInternal())
		return m_pPak->FTell(m_fileHandle);
	return 0;
}

SInt GFileDrxPak::Tell()
{
	return TellInternal();
}

SInt64 GFileDrxPak::LTell()
{
	static_assert(sizeof(SInt64) >= sizeof(long), "Invalid type size!");
	return TellInternal();
}

inline size_t GFileDrxPak::GetLengthInternal()
{
	if (IsValidInternal())
		return m_pPak->FGetSize(m_fileHandle);
	return 0;
}

SInt GFileDrxPak::GetLength()
{
	size_t len = GetLengthInternal();
	assert(len <= UInt(-1) >> 1);
	return (SInt) len;
}

SInt64 GFileDrxPak::LGetLength()
{
	size_t len = GetLengthInternal();
	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	assert(len <= ((UInt64) - 1) >> 1);
	#endif
	return (SInt64) len;
}

SInt GFileDrxPak::GetErrorCode()
{
	if (IsValidInternal())
	{
		if (!m_pPak->FError(m_fileHandle))
			return 0;
		return GFileConstants::Error_IOError;
	}
	return GFileConstants::Error_FileNotFound;
}

SInt GFileDrxPak::Write(const UByte* pBuf, SInt numBytes)
{
	// writable files are not supported!
	return -1;
}

SInt GFileDrxPak::Read(UByte* pBuf, SInt numBytes)
{
	if (IsValidInternal())
		return m_pPak->FReadRaw(pBuf, 1, numBytes, m_fileHandle);
	return -1;
}

SInt GFileDrxPak::SkipBytes(SInt numBytes)
{
	if (IsValidInternal())
	{
		long pos = m_pPak->FTell(m_fileHandle);
		if (pos == -1)
			return -1;

		long newPos = SeekInternal(numBytes, SEEK_CUR);
		if (newPos == -1)
			return -1;

		return newPos - pos;
	}
	return 0;
}

SInt GFileDrxPak::BytesAvailable()
{
	if (IsValidInternal())
	{
		long pos = m_pPak->FTell(m_fileHandle);
		if (pos == -1)
			return 0;

		size_t endPos = m_pPak->FGetSize(m_fileHandle);
		assert(endPos <= ((u64)-1) >> 1);

		return long(endPos) - pos;
	}
	return 0;
}

bool GFileDrxPak::Flush()
{
	// writable files are not supported!
	return false;
}

inline long GFileDrxPak::SeekInternal(long offset, i32 origin)
{
	if (IsValidInternal())
	{
		i32 newOrigin = 0;
		switch (origin)
		{
		case Seek_Set:
			newOrigin = SEEK_SET;
			break;
		case Seek_Cur:
			newOrigin = SEEK_CUR;
			break;
		case Seek_End:
			newOrigin = SEEK_END;
			break;
		}

		if (newOrigin == SEEK_SET)
		{
			long curPos = m_pPak->FTell(m_fileHandle);
			if (offset == curPos)
				return curPos;
		}

		if (!m_pPak->FSeek(m_fileHandle, offset, newOrigin))
			return m_pPak->FTell(m_fileHandle);
	}
	return -1;
}

SInt GFileDrxPak::Seek(SInt offset, SInt origin)
{
	return SeekInternal(offset, origin);
}

SInt64 GFileDrxPak::LSeek(SInt64 offset, SInt origin)
{
	assert(offset <= (long)(((u64)-1) >> 1));

	static_assert(sizeof(SInt64) >= sizeof(long), "Invalid type size!");
	return SeekInternal((long) offset, origin);
}

bool GFileDrxPak::ChangeSize(SInt newSize)
{
	// not supported!
	return false;
}

SInt GFileDrxPak::CopyFromStream(GFile* pStream, SInt byteSize)
{
	// not supported!
	return -1;
}

bool GFileDrxPak::Close()
{
	if (IsValidInternal())
	{
		bool res = !m_pPak->FClose(m_fileHandle);
		m_fileHandle = 0;
		return res;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

GFileDrxStream::GFileDrxStream(tukk pPath, EStreamTaskType streamType)
	: GFile()
	, m_pStreamEngine(0)
	, m_relativeFilePath(pPath)
	, m_fileSize(0)
	, m_curPos(0)
	, m_isValid(false)
	, m_readError(false)
	, m_streamType(streamType)
{
	ISystem* pSystem = gEnv->pSystem;
	assert(pSystem);

	m_pStreamEngine = pSystem->GetStreamEngine();
	assert(m_pStreamEngine);

	IDrxPak* pPak = gEnv->pDrxPak;
	assert(pPak);

	if (pPath && *pPath)
	{
		m_fileSize = pPak->FGetSize(pPath); // assume zero size files are invalid
		m_isValid = m_fileSize != 0;
		if (m_isValid && pPak->IsFileCompressed(pPath))
		{
			DrxGFxLog::GetAccess().LogError("\"%s\" is compressed inside pak. File access not granted! Streamed "
			                                "random file access would be highly inefficient. Make sure file is stored not compressed.", pPath);
			m_isValid = false;
		}
	}
}

GFileDrxStream::~GFileDrxStream()
{
}

tukk GFileDrxStream::GetFilePath()
{
	return m_relativeFilePath.ToCStr();
}

bool GFileDrxStream::IsValid()
{
	return IsValidInternal();
}

bool GFileDrxStream::IsWritable()
{
	// writable files are not supported!
	return false;
}

inline long GFileDrxStream::TellInternal()
{
	if (IsValidInternal())
		return m_curPos;
	return 0;
}

SInt GFileDrxStream::Tell()
{
	return TellInternal();
}

SInt64 GFileDrxStream::LTell()
{
	static_assert(sizeof(SInt64) >= sizeof(long), "Invalid type size!");
	return TellInternal();
}

inline size_t GFileDrxStream::GetLengthInternal()
{
	if (IsValidInternal())
		return m_fileSize;
	return 0;
}

SInt GFileDrxStream::GetLength()
{
	size_t len = GetLengthInternal();
	assert(len <= UInt(-1) >> 1);
	return (SInt) len;
}

SInt64 GFileDrxStream::LGetLength()
{
	size_t len = GetLengthInternal();
	#if DRX_PLATFORM_WINDOWS && DRX_PLATFORM_64BIT
	assert(len <= ((UInt64) - 1) >> 1);
	#endif
	return (SInt64) len;
}

SInt GFileDrxStream::GetErrorCode()
{
	if (IsValidInternal())
	{
		if (!m_readError)
			return 0;
		return GFileConstants::Error_IOError;
	}
	return GFileConstants::Error_FileNotFound;
}

SInt GFileDrxStream::Write(const UByte* pBuf, SInt numBytes)
{
	// writable files are not supported!
	return -1;
}

SInt GFileDrxStream::Read(UByte* pBuf, SInt numBytes)
{
	m_readError = true;

	if (IsValidInternal())
	{
		if (!InRange(m_curPos) || numBytes < 0)
			return -1;

		if ((size_t) (m_curPos + numBytes) > m_fileSize)
			numBytes = m_fileSize - m_curPos;

		StreamReadParams params(0, estpUrgent, 0, 0, (unsigned) m_curPos, (unsigned) numBytes, pBuf, IStreamEngine::FLAGS_NO_SYNC_CALLBACK);
		IReadStreamPtr p = m_pStreamEngine->StartRead(m_streamType, m_relativeFilePath.ToCStr(), 0, &params);

		if (p)
		{
			p->Wait();
			m_readError = p->IsError();
			if (!m_readError)
			{
				u32 bytesRead = p->GetBytesRead();
				m_curPos += (SInt) bytesRead;
				return (SInt) bytesRead;
			}
		}
	}
	return -1;
}

SInt GFileDrxStream::SkipBytes(SInt numBytes)
{
	if (IsValidInternal())
	{
		long pos = m_curPos;
		if (!InRange(pos))
			return -1;

		long newPos = m_curPos + numBytes;
		if (!InRange(newPos))
			return -1;

		return newPos - pos;
	}
	return 0;
}

SInt GFileDrxStream::BytesAvailable()
{
	if (IsValidInternal())
	{
		long pos = m_curPos;
		if (!InRange(pos))
			return 0;

		size_t endPos = m_fileSize;
		assert(endPos <= ((u64)-1) >> 1);

		return long(endPos) - pos;
	}
	return 0;
}

bool GFileDrxStream::Flush()
{
	// writable files are not supported!
	return false;
}

inline long GFileDrxStream::SeekInternal(long offset, i32 origin)
{
	if (IsValidInternal())
	{
		switch (origin)
		{
		case Seek_Set:
			m_curPos = offset;
			break;
		case Seek_Cur:
			m_curPos += offset;
			break;
		case Seek_End:
			m_curPos = m_fileSize - offset;
			break;
		}

		if (InRange(m_curPos))
			return m_curPos;
	}
	return -1;
}

SInt GFileDrxStream::Seek(SInt offset, SInt origin)
{
	return SeekInternal(offset, origin);
}

SInt64 GFileDrxStream::LSeek(SInt64 offset, SInt origin)
{
	assert(offset <= (long)(((u64)-1) >> 1));

	static_assert(sizeof(SInt64) >= sizeof(long), "Invalid type size!");
	return SeekInternal((long) offset, origin);
}

bool GFileDrxStream::ChangeSize(SInt newSize)
{
	// not supported!
	return false;
}

SInt GFileDrxStream::CopyFromStream(GFile* pStream, SInt byteSize)
{
	// not supported!
	return -1;
}

bool GFileDrxStream::Close()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

GFileInMemoryDrxStream::GFileInMemoryDrxStream(tukk pPath, EStreamTaskType streamType)
	: GFile()
	, m_pMemFile(0)
	, m_pData(0)
	, m_dataSize(0)
	, m_errCode(EC_Undefined)
{
	ISystem* pSystem = gEnv->pSystem;
	assert(pSystem);

	IStreamEngine* pStreamEngine = pSystem->GetStreamEngine();
	assert(pStreamEngine);

	IDrxPak* pPak = gEnv->pDrxPak;
	assert(pPak);

	if (pPath && *pPath)
	{
		pPak->CheckFileAccessDisabled(pPath, "rb");

		m_dataSize = pPak->FGetSize(pPath); // assume zero size files are invalid
		m_pData = m_dataSize ? (u8*) GALLOC(m_dataSize, GStat_Default_Mem) : 0;

		if (m_dataSize && m_pData)
		{
			StreamReadParams params(0, estpUrgent, 0, 0, 0, (unsigned) m_dataSize, m_pData, IStreamEngine::FLAGS_NO_SYNC_CALLBACK);
			IReadStreamPtr pReadStream = pStreamEngine->StartRead(streamType, pPath, 0, &params);

			if (pReadStream)
			{
				pReadStream->Wait();
				if (!pReadStream->IsError())
				{
					m_pMemFile = *new GMemoryFile(pPath, (const UByte*) m_pData, (SInt) m_dataSize);
					m_errCode = EC_Ok;
				}
				else
				{
					m_errCode = pReadStream->GetError() == ERROR_CANT_OPEN_FILE ? EC_FileNotFound : EC_IOError;
				}
			}
		}
	}

	if (!m_pMemFile)
		m_pMemFile = *new GMemoryFile(0, 0, 0);
}

GFileInMemoryDrxStream::~GFileInMemoryDrxStream()
{
	m_pMemFile = 0;
	if (m_pData)
	{
		GFREE(m_pData);
		m_pData = 0;
	}
}

tukk GFileInMemoryDrxStream::GetFilePath()
{
	return m_pMemFile->GetFilePath();
}

bool GFileInMemoryDrxStream::IsValid()
{
	return m_pMemFile->IsValid();
}

bool GFileInMemoryDrxStream::IsWritable()
{
	// writable files are not supported!
	return m_pMemFile->IsWritable();
}

SInt GFileInMemoryDrxStream::Tell()
{
	return m_pMemFile->Tell();
}

SInt64 GFileInMemoryDrxStream::LTell()
{
	return m_pMemFile->LTell();
}

SInt GFileInMemoryDrxStream::GetLength()
{
	return m_pMemFile->GetLength();
}

SInt64 GFileInMemoryDrxStream::LGetLength()
{
	return m_pMemFile->LGetLength();
}

SInt GFileInMemoryDrxStream::GetErrorCode()
{
	switch (m_errCode)
	{
	case EC_Ok:
		return 0;
	case EC_FileNotFound:
		return GFileConstants::Error_FileNotFound;
	case EC_IOError:
		return GFileConstants::Error_IOError;
	case EC_Undefined:
	default:
		return GFileConstants::Error_IOError;
	}
}

SInt GFileInMemoryDrxStream::Write(const UByte* pBuf, SInt numBytes)
{
	// writable files are not supported!
	return -1;
}

SInt GFileInMemoryDrxStream::Read(UByte* pBuf, SInt numBytes)
{
	return m_pMemFile->Read(pBuf, numBytes);
}

SInt GFileInMemoryDrxStream::SkipBytes(SInt numBytes)
{
	return m_pMemFile->SkipBytes(numBytes);
}

SInt GFileInMemoryDrxStream::BytesAvailable()
{
	return m_pMemFile->BytesAvailable();
}

bool GFileInMemoryDrxStream::Flush()
{
	// writable files are not supported!
	return false;
}

SInt GFileInMemoryDrxStream::Seek(SInt offset, SInt origin)
{
	return m_pMemFile->Seek(offset, origin);
}

SInt64 GFileInMemoryDrxStream::LSeek(SInt64 offset, SInt origin)
{
	return m_pMemFile->LSeek(offset, origin);
}

bool GFileInMemoryDrxStream::ChangeSize(SInt newSize)
{
	// not supported!
	return false;
}

SInt GFileInMemoryDrxStream::CopyFromStream(GFile* pStream, SInt byteSize)
{
	// not supported!
	return -1;
}

bool GFileInMemoryDrxStream::Close()
{
	return m_pMemFile->Close();
}

#endif // #ifdef INCLUDE_SCALEFORM_SDK
