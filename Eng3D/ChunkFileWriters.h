// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла: ChunkFileWriters.h
//  Created:   2013/10/21 by Sergey Sokov
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ChunkFileWriters_h__
#define __ChunkFileWriters_h__

#include <drx3D/Eng3D/ChunkFileComponents.h>

namespace ChunkFile
{

struct IWriter
{
	virtual ~IWriter()
	{
	}

	virtual void  Erase() = 0;
	virtual void  Close() = 0; // if Close() is not called then the file should be deleted in destructor

	virtual i32 GetPos() const = 0;

	virtual bool  Write(ukk buffer, size_t size) = 0;

	// non-virtual helper function
	bool WriteZeros(size_t size);
};

class OsFileWriter
	: public IWriter
{
public:
	OsFileWriter();
	virtual ~OsFileWriter();

	bool Create(tukk filename);

	//-------------------------------------------------------
	// IWriter interface
	virtual void  Erase();
	virtual void  Close();

	virtual i32 GetPos() const;

	virtual bool  Write(ukk buffer, size_t size);
	//-------------------------------------------------------

private:
	string m_filename;
	FILE*  m_f;
	i32  m_offset;
};

#if !defined(RESOURCE_COMPILER)
class DrxPakFileWriter
	: public IWriter
{
public:
	DrxPakFileWriter();
	virtual ~DrxPakFileWriter();

	bool Create(IDrxPak* pPak, tukk filename);

	//-------------------------------------------------------
	// IWriter interface
	virtual void  Erase();
	virtual void  Close();

	virtual i32 GetPos() const;

	virtual bool  Write(ukk buffer, size_t size);
	//-------------------------------------------------------

private:
	string   m_filename;
	IDrxPak* m_pPak;
	FILE*    m_f;
	i32    m_offset;
};
#endif

// Doesn't write any data, just computes the size
class SizeWriter
	: public IWriter
{
public:
	SizeWriter()
	{
	}
	virtual ~SizeWriter()
	{
	}

	void Start()
	{
		m_offset = 0;
	}

	//-------------------------------------------------------
	// IWriter interface
	virtual void Erase()
	{
	}

	virtual void Close()
	{
	}

	virtual i32 GetPos() const
	{
		return m_offset;
	}

	virtual bool Write(ukk buffer, size_t size)
	{
		m_offset += size;
		return true;
	}
	//-------------------------------------------------------

private:
	i32 m_offset;
};

class MemoryWriter
	: public IWriter
{
public:
	MemoryWriter();
	virtual ~MemoryWriter();

	bool Start(uk ptr, i32 size);

	//-------------------------------------------------------
	// IWriter interface
	virtual void  Erase();
	virtual void  Close();

	virtual i32 GetPos() const;

	virtual bool  Write(ukk buffer, size_t size);
	//-------------------------------------------------------

private:
	tuk m_ptr;
	i32 m_size;
	i32 m_offset;
};

struct IChunkFileWriter
{
	virtual ~IChunkFileWriter()
	{
	}

	// Sets alignment for *beginning* of chunk data.
	// Allowed to be called at any time, influences all future
	// StartChunk() calls (until a new SetAlignment() call).
	virtual void SetAlignment(size_t alignment) = 0;

	// Returns false when there is no more passes left.
	virtual bool StartPass() = 0;

	// eEndianness specifies endianness of the data user is
	// going to provide via AddChunkData*(). The data will be
	// sent to the low-level writer as is, without any re-coding.
	virtual void     StartChunk(EEndianness eEndianness, u32 type, u32 version, u32 id) = 0;
	virtual void     AddChunkData(uk ptr, size_t size) = 0;
	virtual void     AddChunkDataZeros(size_t size) = 0;
	virtual void     AddChunkDataAlignment(size_t alignment) = 0;

	virtual bool     HasWrittenSuccessfully() const = 0;

	virtual IWriter* GetWriter() const = 0;
};

// Memoryless chunk file writer
//
// Usage example:
//
//	OsFileWriter writer;
//	if (!writer.Create(filename))
//	{
//		showAnErrorMessage();
//	}
//	else
//	{
//		MemorylessChunkFileWriter wr(eChunFileFormat, writer);
//		while (wr.StartPass())
//		{
//			// default alignment of chunk data in file is 4, but you may change it by calling wr.SetAlignment(xxx)
//
//			wr.StartChunk(eEndianness_Native, chunkA_type, chunkA_version, chunkA_id);
//			wr.AddChunkData(data_ptr0, data_size0);  // make sure that data have endianness specified by bLittleEdndian
//			wr.AddChunkData(data_ptr1, data_size1);
//			...
//			wr.StartChunk(eEndianness_Native, chunkB_type, chunkB_version, chunkB_id);
//			wr.AddChunkData(data_ptrN, data_sizeN);
//			...
//		}
//		if (!wr.HasWrittenSuccessfully())
//		{
//			showAnErrorMessage();
//		}
//	}

class MemorylessChunkFileWriter
	: public IChunkFileWriter
{
public:
	enum EChunkFileFormat
	{
		eChunkFileFormat_0x745,
		eChunkFileFormat_0x746,
	};

	MemorylessChunkFileWriter(EChunkFileFormat eFormat, IWriter* pWriter);

	//-------------------------------------------------------
	// IChunkFileWriter interface
	virtual ~MemorylessChunkFileWriter();

	virtual void     SetAlignment(size_t alignment);

	virtual bool     StartPass();

	virtual void     StartChunk(EEndianness eEndianness, u32 type, u32 version, u32 id);
	virtual void     AddChunkData(uk ptr, size_t size);
	virtual void     AddChunkDataZeros(size_t size);
	virtual void     AddChunkDataAlignment(size_t alignment);

	virtual bool     HasWrittenSuccessfully() const;

	virtual IWriter* GetWriter() const;
	//-------------------------------------------------------

private:
	void          Fail();

	static size_t ComputeSizeOfAlignmentArea(size_t pos, size_t alignment);

	size_t        GetSizeOfHeader() const;
	void          WriteFileHeader(i32 chunkCount, u32 chunkTableOffsetInFile);

	size_t        GetSizeOfChunkTable(i32 chunkCount) const;
	void          WriteChunkTableHeader(i32 chunkCount);
	void          WriteChunkEntry();

private:
	IWriter* const   m_pWriter;

	EChunkFileFormat m_eChunkFileFormat;

	size_t           m_alignment;

	i32            m_chunkCount;

	i32            m_chunkIndex;
	u16           m_chunkType;
	u16           m_chunkVersion;
	u32           m_chunkId;
	u32           m_chunkSize;
	u32           m_chunkOffsetInFile;
	EEndianness      m_chunkEndianness;

	u32           m_dataOffsetInFile;

	enum EState
	{
		eState_Init,

		eState_CountingChunks,
		eState_WritingChunkTable,
		eState_WritingData,

		eState_Success,
		eState_Fail,
	};
	EState m_eState;
};

}  // namespace ChunkFile

#endif
