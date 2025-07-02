// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ReadOnlyChunkFile.h
//  Created:     2004/11/15 by Timur
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ReadOnlyChunkFile_h__
#define __ReadOnlyChunkFile_h__

#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/Eng3D/IChunkFile.h>

////////////////////////////////////////////////////////////////////////
// Chunk file reader.
// Accesses a chunked file structure through file mapping object.
// Opens a chunk file and checks for its validity.
// If it's invalid, closes it as if there was no open operation.
// Error handling is performed through the return value of Read: it must
// be true for successfully open files
////////////////////////////////////////////////////////////////////////

class CReadOnlyChunkFile : public IChunkFile
{
public:
	//////////////////////////////////////////////////////////////////////////
	CReadOnlyChunkFile(bool bCopyFileData, bool bNoWarningMode = false);
	virtual ~CReadOnlyChunkFile();

	// interface IChunkFile --------------------------------------------------

	virtual void             Release()          { delete this; }

	virtual bool             IsReadOnly() const { return true; }
	virtual bool             IsLoaded() const   { return m_bLoaded; }

	virtual bool             Read(tukk filename);
	virtual bool             ReadFromMemory(ukk pData, i32 nDataSize);

	virtual bool             Write(tukk filename)                                                                                     { return false; }
	virtual bool             WriteToMemoryBuffer(uk * pData, i32* nSize)                                                                   { return false; }
	virtual void             ReleaseMemoryBuffer()                                                                                           {}

	virtual i32              AddChunk(ChunkTypes chunkType, i32 chunkVersion, EEndianness eEndianness, ukk chunkData, i32 chunkSize) { return -1; }
	virtual void             DeleteChunkById(i32 nChunkId)                                                                                   {}
	virtual void             DeleteChunksByType(ChunkTypes nChunkType)                                                                       {}

	virtual ChunkDesc*       FindChunkByType(ChunkTypes nChunkType);
	virtual ChunkDesc*       FindChunkById(i32 nChunkId);

	virtual i32              NumChunks() const;

	virtual ChunkDesc*       GetChunk(i32 nIndex);
	virtual const ChunkDesc* GetChunk(i32 nIndex) const;

	virtual tukk      GetLastError() const { return m_LastError; }

	// -----------------------------------------------------------------------

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_LastError);
		pSizer->AddObject(m_chunks);
	}
private:
	bool ReadChunkTableFromBuffer();
	void FreeBuffer();
	void CloseFile();

private:
	// this variable contains the last error occurred in this class
	string                 m_LastError;

	std::vector<ChunkDesc> m_chunks;

	tuk                  m_pFileBuffer;
	i32                    m_nBufferSize;
	bool                   m_bOwnFileBuffer;
	bool                   m_bNoWarningMode;
	bool                   m_bLoaded;
	bool                   m_bCopyFileData;

	FILE*                  m_hFile;
};

TYPEDEF_AUTOPTR(CReadOnlyChunkFile);

#endif
