// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ReadOnlyChunkFile.h
//  Created:     2004/11/15 by Timur
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ReadOnlyChunkFile.h>
#include <drx3D/Eng3D/ChunkFileComponents.h>
#include <drx3D/Eng3D/ChunkFileReaders.h>

#define MAX_CHUNKS_NUM 10000000

#if !defined(FUNCTION_PROFILER_3DENGINE)
	#define FUNCTION_PROFILER_3DENGINE
#endif

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::CReadOnlyChunkFile(bool bCopyFileData, bool bNoWarningMode)
{
	m_pFileBuffer = 0;
	m_nBufferSize = 0;

	m_bNoWarningMode = bNoWarningMode;

	m_bOwnFileBuffer = false;
	m_bLoaded = false;
	m_bCopyFileData = bCopyFileData;

	m_hFile = 0;
}

CReadOnlyChunkFile::~CReadOnlyChunkFile()
{
	CloseFile();
	FreeBuffer();
}

//////////////////////////////////////////////////////////////////////////
void CReadOnlyChunkFile::CloseFile()
{
	if (m_hFile)
	{
		gEnv->pDrxPak->FClose(m_hFile);
		m_hFile = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CReadOnlyChunkFile::FreeBuffer()
{
	if (m_pFileBuffer && m_bOwnFileBuffer)
	{
		delete[] m_pFileBuffer;
	}
	m_pFileBuffer = 0;
	m_nBufferSize = 0;
	m_bOwnFileBuffer = false;
	m_bLoaded = false;
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::GetChunk(i32 nIndex)
{
	assert(size_t(nIndex) < m_chunks.size());
	return &m_chunks[nIndex];
}

//////////////////////////////////////////////////////////////////////////
const CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::GetChunk(i32 nIndex) const
{
	assert(size_t(nIndex) < m_chunks.size());
	return &m_chunks[nIndex];
}

// number of chunks
i32 CReadOnlyChunkFile::NumChunks() const
{
	return (i32)m_chunks.size();
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::FindChunkByType(ChunkTypes nChunkType)
{
	for (size_t i = 0, count = m_chunks.size(); i < count; ++i)
	{
		if (m_chunks[i].chunkType == nChunkType)
		{
			return &m_chunks[i];
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CReadOnlyChunkFile::ChunkDesc* CReadOnlyChunkFile::FindChunkById(i32 id)
{
	ChunkDesc chunkToFind;
	chunkToFind.chunkId = id;

	std::vector<ChunkDesc>::iterator it = std::lower_bound(m_chunks.begin(), m_chunks.end(), chunkToFind, IChunkFile::ChunkDesc::LessId);
	if (it != m_chunks.end() && id == (*it).chunkId)
	{
		return &(*it);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::ReadChunkTableFromBuffer()
{
	FUNCTION_PROFILER_3DENGINE;

	if (m_pFileBuffer == 0)
	{
		m_LastError.Format("Unexpected empty buffer");
		return false;
	}

	{
		ChunkFile::MemoryReader f;

		if (!f.Start(m_pFileBuffer, m_nBufferSize))
		{
			m_LastError.Format("Empty memory chunk file");
			return false;
		}

		bool bStripHeaders = false;
		tukk err = 0;

		err = ChunkFile::GetChunkTableEntries_0x746(&f, m_chunks);
		if (err)
		{
			err = ChunkFile::GetChunkTableEntries_0x744_0x745(&f, m_chunks);
			bStripHeaders = true;
		}

		if (!err)
		{
			for (size_t i = 0; i < m_chunks.size(); ++i)
			{
				ChunkDesc& cd = m_chunks[i];
				cd.data = m_pFileBuffer + cd.fileOffset;
			}
			if (bStripHeaders)
			{
				err = ChunkFile::StripChunkHeaders_0x744_0x745(&f, m_chunks);
			}
		}

		if (err)
		{
			m_LastError = err;
			return false;
		}
	}

	// Sort chunks by Id, for faster queries later (see FindChunkById()).
	std::sort(m_chunks.begin(), m_chunks.end(), IChunkFile::ChunkDesc::LessId);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::Read(tukk filename)
{
	FUNCTION_PROFILER_3DENGINE;

	CloseFile();
	FreeBuffer();

	m_hFile = gEnv->pDrxPak->FOpen(filename, "rb", (m_bNoWarningMode ? IDrxPak::FOPEN_HINT_QUIET : 0));
	if (!m_hFile)
	{
		m_LastError.Format("Failed to open file '%s'", filename);
		return false;
	}

	size_t nFileSize = 0;

	if (m_bCopyFileData)
	{
		nFileSize = gEnv->pDrxPak->FGetSize(m_hFile);
		m_pFileBuffer = new char[nFileSize];
		m_bOwnFileBuffer = true;
		if (gEnv->pDrxPak->FReadRawAll(m_pFileBuffer, nFileSize, m_hFile) != nFileSize)
		{
			m_LastError.Format("Failed to read %u bytes from file '%s'", (uint)nFileSize, filename);
			return false;
		}
	}
	else
	{
		m_pFileBuffer = (tuk)gEnv->pDrxPak->FGetCachedFileData(m_hFile, nFileSize);
		m_bOwnFileBuffer = false;
	}

	if (!m_pFileBuffer)
	{
		m_LastError.Format("Failed to get memory for file '%s'", filename);
		return false;
	}

	m_nBufferSize = nFileSize;

	if (!ReadChunkTableFromBuffer())
	{
		return false;
	}

	m_bLoaded = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CReadOnlyChunkFile::ReadFromMemory(ukk pData, i32 nDataSize)
{
	FUNCTION_PROFILER_3DENGINE;

	CloseFile();
	FreeBuffer();

	m_pFileBuffer = (tuk)pData;
	m_bOwnFileBuffer = false;
	m_nBufferSize = nDataSize;

	if (!ReadChunkTableFromBuffer())
	{
		return false;
	}
	m_bLoaded = true;
	return true;
}
