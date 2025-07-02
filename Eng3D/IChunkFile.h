// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include "DrxHeaders.h"

//! Chunked File (.cgf, .chr etc.) interface.
struct IChunkFile : _reference_target_t
{
	//! Chunk Description.
	struct ChunkDesc
	{
		ChunkTypes chunkType;
		i32        chunkVersion;
		i32        chunkId;
		u32     fileOffset;
		uk      data;
		u32     size;
		bool       bSwapEndian;

		ChunkDesc()
			: chunkType(ChunkType_ANY)
			, chunkVersion(0)
			, chunkId(0)
			, fileOffset(0)
			, data(0)
			, size(0)
			, bSwapEndian(false)
		{
		}

		void               GetMemoryUsage(IDrxSizer* pSizer) const                   { /*nothing*/ }

		static inline bool LessOffset(const ChunkDesc& d1, const ChunkDesc& d2)      { return d1.fileOffset < d2.fileOffset; }
		static inline bool LessOffsetByPtr(const ChunkDesc* d1, const ChunkDesc* d2) { return d1->fileOffset < d2->fileOffset; }
		static inline bool LessId(const ChunkDesc& d1, const ChunkDesc& d2)          { return d1.chunkId < d2.chunkId; }
	};

	// <interfuscator:shuffle>

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;

	//! Releases chunk file interface.
	virtual void Release() = 0;

	virtual bool IsReadOnly() const = 0;
	virtual bool IsLoaded() const = 0;

	virtual bool Read(tukk filename) = 0;
	virtual bool ReadFromMemory(ukk pData, i32 nDataSize) = 0;

	//! Writes chunks to file.
	virtual bool Write(tukk filename) = 0;

	//! Writes chunks to a memory buffer (allocated inside) and returns a pointer to the allocated memory (pData) and its size (nSize).
	//! The memory will be released on destruction of the ChunkFile object, or on
	//! the next WriteToMemoryBuffer() call, or on ReleaseMemoryBuffer() call.
	virtual bool WriteToMemoryBuffer(uk * pData, i32* nSize) = 0;

	//! Releases memory that was allocated in WriteToMemoryBuffer().
	virtual void ReleaseMemoryBuffer() = 0;

	//! Adds chunk to file, returns ChunkID of the added chunk.
	virtual i32        AddChunk(ChunkTypes chunkType, i32 chunkVersion, EEndianness eEndianness, ukk chunkData, i32 chunkSize) = 0;
	virtual void       DeleteChunkById(i32 nChunkId) = 0;
	virtual void       DeleteChunksByType(ChunkTypes nChunkType) = 0;

	virtual ChunkDesc* FindChunkByType(ChunkTypes nChunkType) = 0;
	virtual ChunkDesc* FindChunkById(i32 nChunkId) = 0;

	//! Gets the number of chunks.
	virtual i32 NumChunks() const = 0;

	//! Gets chunk description at i-th index.
	virtual ChunkDesc*       GetChunk(i32 nIndex) = 0;
	virtual const ChunkDesc* GetChunk(i32 nIndex) const = 0;

	virtual tukk      GetLastError() const = 0;

	// </interfuscator:shuffle>
};

//! \endcond