// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   SaveReaderWriter_DrxPak.cpp
//  Created:     15/02/2010 by Alex McCarthy.
//  Описание: Implementation of the ISaveReader and ISaveWriter
//               interfaces using DrxPak
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Sys/StdAfx.h>

#include <drx3D/Sys/SaveReaderWriter_DrxPak.h>
#include <drx3D/Sys/SaveReaderWriter_Memory.h>
#include <drx3D/Network/INetwork.h>

static i32k INVALID_SEEK = -1;

i32 TranslateSeekMode(IPlatformOS::ISaveReader::ESeekMode mode)
{
	static_assert(INVALID_SEEK != SEEK_SET, "Invalid SEEK_SET value!");
	static_assert(INVALID_SEEK != SEEK_CUR, "Invalid SEEK_CUR value!");
	static_assert(INVALID_SEEK != SEEK_END, "INVALID SEEK_END value!");

	switch (mode)
	{
	case IPlatformOS::ISaveReader::ESM_BEGIN:
		return SEEK_SET;
	case IPlatformOS::ISaveReader::ESM_CURRENT:
		return SEEK_CUR;
	case IPlatformOS::ISaveReader::ESM_END:
		return SEEK_END;
	default:
		{
			DRX_ASSERT_TRACE(false, ("Unrecognized seek mode %i", static_cast<i32>(mode)));
			return INVALID_SEEK;
		}
	}
}

#if !DRX_PLATFORM_ORBIS
NO_INLINE_WEAK FILE* CDrxPakFile::FOpen(tukk pName, tukk mode, unsigned nFlags)
{
	return gEnv->pDrxPak->FOpen(pName, mode, nFlags);
}

NO_INLINE_WEAK i32 CDrxPakFile::FClose(FILE* fp)
{
	return gEnv->pDrxPak->FClose(fp);
}

NO_INLINE_WEAK size_t CDrxPakFile::FGetSize(FILE* fp)
{
	return gEnv->pDrxPak->FGetSize(fp);
}

NO_INLINE_WEAK size_t CDrxPakFile::FReadRaw(uk data, size_t length, size_t elems, FILE* fp)
{
	return gEnv->pDrxPak->FReadRaw(data, length, elems, fp);
}

NO_INLINE_WEAK size_t CDrxPakFile::FWrite(ukk data, size_t length, size_t elems, FILE* fp)
{
	return gEnv->pDrxPak->FWrite(data, length, elems, fp);
}

NO_INLINE_WEAK size_t CDrxPakFile::FSeek(FILE* fp, long offset, i32 mode)
{
	return gEnv->pDrxPak->FSeek(fp, offset, mode);
}
#endif // !DRX_PLATFORM_ORBIS
////////////////////////////////////////////////////////////////////////////

CDrxPakFile::CDrxPakFile(tukk fileName, tukk szMode)
	: m_fileName(fileName)
	, m_filePos(0)
	, m_eLastError(IPlatformOS::eFOC_Success)
	, m_bWriting(!strchr(szMode, 'r'))
{
	m_pFile = FOpen(fileName, szMode, IDrxPak::FOPEN_ONDISK);

	if (m_pFile == nullptr)
	{
		m_pFile = FOpen(fileName, szMode);
	}

	if (m_pFile != nullptr && !m_bWriting)
	{
		size_t fileSize = FGetSize(m_pFile);

		if (fileSize > 0)
		{
			m_data.resize(fileSize);

			const std::vector<char>* pMagic;
			const std::vector<u8>* pKey;
			GetISystem()->GetPlatformOS()->GetEncryptionKey(&pMagic, &pKey);

			bool isEncrypted = false;

			if (pMagic->size() > 0 && fileSize >= pMagic->size())
			{
				std::vector<u8> magic;
				magic.resize(pMagic->size());
				FReadRaw(&magic[0], 1, magic.size(), m_pFile);
				fileSize -= magic.size();
				isEncrypted = 0 == memcmp(&magic[0], &pMagic->front(), magic.size());

				if (isEncrypted)
				{
					m_data.resize(fileSize);
				}
				else
				{
					memcpy(&m_data[0], &magic[0], magic.size());
					m_filePos += magic.size();
				}
			}

			if (fileSize > 0)
			{
				FReadRaw(&m_data[m_filePos], 1, fileSize, m_pFile);

				if (isEncrypted)
				{
					gEnv->pNetwork->DecryptBuffer(&m_data[0], &m_data[0], fileSize, &pKey->front(), pKey->size());
				}
			}

			m_filePos = 0;
		}
	}
}

CDrxPakFile::~CDrxPakFile()
{
	CloseImpl();
}

IPlatformOS::EFileOperationCode CDrxPakFile::CloseImpl()
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	if (m_eLastError == IPlatformOS::eFOC_Success)
	{
		if (m_pFile)
		{
			if (m_bWriting)
			{
				if (!m_data.empty())
				{
#ifndef _RELEASE
					// Default true if CVar doesn't exist
					ICVar* pUseEncryption = gEnv->pConsole ? gEnv->pConsole->GetCVar("sys_usePlatformSavingAPIEncryption") : NULL;
					bool bUseEncryption = pUseEncryption && pUseEncryption->GetIVal() != 0;
					if (bUseEncryption)
#endif
					{
						// Write the magic tag
						const std::vector<char>* pMagic;
						const std::vector<u8>* pKey;
						GetISystem()->GetPlatformOS()->GetEncryptionKey(&pMagic, &pKey);
						if (pMagic->size() != FWrite(&pMagic->front(), 1, pMagic->size(), m_pFile))
						{
							m_eLastError = IPlatformOS::eFOC_ErrorWrite;
						}
						else
						{
							gEnv->pNetwork->EncryptBuffer(&m_data[0], &m_data[0], m_data.size(), &pKey->front(), pKey->size());
						}
					}

					if (m_data.size() != FWrite(&m_data[0], 1, m_data.size(), m_pFile))
					{
						m_eLastError = IPlatformOS::eFOC_ErrorWrite;
					}
				}
			}

			if (FClose(m_pFile) != 0)
			{
				m_eLastError = IPlatformOS::eFOC_Failure;
			}
		}
	}
	m_pFile = NULL;
	m_data.clear();

	return m_eLastError;
}

////////////////////////////////////////////////////////////////////////////

// Update file stats to make sure SaveGame system reads from new info
void UpdateFileStats(tukk fileName)
{
#if DRX_PLATFORM_ORBIS
	gEnv->pDrxPak->GetFileSizeOnDisk(fileName);
#endif
}

////////////////////////////////////////////////////////////////////////////

CSaveReader_DrxPak::CSaveReader_DrxPak(tukk fileName)
	: CDrxPakFile(fileName, "rbx") // x=don't cache full file
{
	m_eLastError = m_pFile == NULL ? IPlatformOS::eFOC_ErrorOpenRead : IPlatformOS::eFOC_Success;
}

IPlatformOS::EFileOperationCode CSaveReader_DrxPak::Seek(long seek, ESeekMode mode)
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	long pos = (long)m_filePos;

	switch (mode)
	{
	case ESM_BEGIN:
		pos = seek;
		break;
	case ESM_CURRENT:
		pos += seek;
		break;
	case ESM_END:
		pos = m_data.size() + seek;
		break;
	}

	if (pos < 0 || pos > (long)m_data.size())
		return IPlatformOS::eFOC_Failure;

	m_filePos = (size_t)pos;

	return IPlatformOS::eFOC_Success;
}

IPlatformOS::EFileOperationCode CSaveReader_DrxPak::GetFileCursor(long& fileCursor)
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	fileCursor = (long)m_filePos;

	return IPlatformOS::eFOC_Success;
}

IPlatformOS::EFileOperationCode CSaveReader_DrxPak::ReadBytes(uk data, size_t numBytes)
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	size_t readBytes = numBytes;
	if (m_filePos + readBytes > m_data.size())
		readBytes = m_data.size() - m_filePos;

	memcpy(data, &m_data[m_filePos], readBytes);
	m_filePos += readBytes;

	if (numBytes != readBytes)
	{
		m_eLastError = IPlatformOS::eFOC_ErrorRead;
		return m_eLastError;
	}
	return IPlatformOS::eFOC_Success;
}

IPlatformOS::EFileOperationCode CSaveReader_DrxPak::GetNumBytes(size_t& numBytes)
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	numBytes = m_data.size();

	return IPlatformOS::eFOC_Success;
}

void CSaveReader_DrxPak::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	pSizer->AddContainer(m_data);
}

void CSaveReader_DrxPak::TouchFile()
{
	// Touch the file so it's used as the most previous checkpoint
	CloseImpl();
	m_pFile = FOpen(m_fileName, "r+b", IDrxPak::FOPEN_ONDISK);
	if (m_pFile)
	{
		char c;
		if (1 == FReadRaw(&c, 1, sizeof(c), m_pFile))
		{
			FSeek(m_pFile, 0, SEEK_SET);
			FWrite(&c, 1, sizeof(c), m_pFile);
		}
		FClose(m_pFile);
		m_pFile = NULL;
	}

	UpdateFileStats(m_fileName.c_str());
}

////////////////////////////////////////////////////////////////////////////

CSaveWriter_DrxPak::CSaveWriter_DrxPak(tukk fileName)
	: CDrxPakFile(fileName, "wb")
{
	UpdateFileStats(fileName);

	m_eLastError = m_pFile == NULL ? IPlatformOS::eFOC_ErrorOpenWrite : IPlatformOS::eFOC_Success;
}

IPlatformOS::EFileOperationCode CSaveWriter_DrxPak::AppendBytes(ukk data, size_t length)
{
	DRX_ASSERT(gEnv);
	DRX_ASSERT(gEnv->pDrxPak);

	if (m_pFile && length != 0)
	{
		size_t newLength = m_data.size() + length;
		if (newLength > m_data.capacity())
		{
			size_t numBlocks = newLength / s_dataBlockSize;
			size_t newCapaticy = (numBlocks + 1) * s_dataBlockSize;
			m_data.reserve(newCapaticy);
		}
		m_data.resize(newLength);
		memcpy(&m_data[m_filePos], data, length);
		m_filePos += length;
	}
	return IPlatformOS::eFOC_Success;
}

void CSaveWriter_DrxPak::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	pSizer->AddContainer(m_data);
}
