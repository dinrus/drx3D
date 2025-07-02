// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//
//	File:ChunkFile.h
//  Declaration of class CChunkFile
//
//	История:
//
//////////////////////////////////////////////////////////////////////
#ifndef _CHUNK_FILE_READER_HDR_
#define _CHUNK_FILE_READER_HDR_

#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/Eng3D/IChunkFile.h>

////////////////////////////////////////////////////////////////////////
// Chunk file reader.
// Accesses a chunked file structure through file mapping object.
// Opens a chunk file and checks for its validity.
// If it's invalid, closes it as if there was no open operation.
// Error handling is performed through the return value of Read():
// it must be true for successfully open files
////////////////////////////////////////////////////////////////////////

class CChunkFile : public IChunkFile
{
public:
	//////////////////////////////////////////////////////////////////////////
	CChunkFile();
	virtual ~CChunkFile();

	// interface IChunkFile --------------------------------------------------

	virtual void             Release() { delete this; }

	void                     Clear();

	virtual bool             IsReadOnly() const { return false; }
	virtual bool             IsLoaded() const   { return m_bLoaded; }

	virtual bool             Read(tukk filename);
	virtual bool             ReadFromMemory(ukk pData, i32 nDataSize) { return false; }

	virtual bool             Write(tukk filename);
	virtual bool             WriteToMemoryBuffer(uk * pData, i32* nSize);
	virtual void             ReleaseMemoryBuffer();

	virtual i32              AddChunk(ChunkTypes chunkType, i32 chunkVersion, EEndianness eEndianness, ukk chunkData, i32 chunkSize);
	virtual void             DeleteChunkById(i32 nChunkId);
	virtual void             DeleteChunksByType(ChunkTypes nChunkType);

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
		pSizer->AddObject(m_chunkIdMap);
	}

private:
	void ReleaseChunks();

private:
	// this variable contains the last error occurred in this class
	string                  m_LastError;

	i32                     m_nLastChunkId;
	std::vector<ChunkDesc*> m_chunks;
	typedef std::map<i32, ChunkDesc*> ChunkIdMap;
	ChunkIdMap              m_chunkIdMap;
	tuk                   m_pInternalData;
	bool                    m_bLoaded;
};

TYPEDEF_AUTOPTR(CChunkFile);

#endif
