// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла: ChunkFileReaders.h
//  Created:   2013/11/18 by Sergey Sokov
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ChunkFileReaders_h__
#define __ChunkFileReaders_h__

#include <drx3D/Eng3D/ChunkFileComponents.h>
#include <drx3D/Eng3D/IChunkFile.h>

namespace ChunkFile
{

struct IReader
{
	virtual ~IReader()
	{
	}

	virtual void  Close() = 0;
	virtual i32 GetSize() = 0;
	virtual bool  SetPos(i32 pos) = 0;
	virtual bool  Read(uk buffer, size_t size) = 0;
};

class DrxFileReader
	: public IReader
{
public:
	DrxFileReader();
	virtual ~DrxFileReader();

	bool Open(tukk filename);

	//-------------------------------------------------------
	// IReader interface
	virtual void  Close();
	virtual i32 GetSize();
	virtual bool  SetPos(i32 pos);
	virtual bool  Read(uk buffer, size_t size);
	//-------------------------------------------------------

private:
	CDrxFile m_f;
	i32    m_offset;
};

class MemoryReader
	: public IReader
{
public:
	MemoryReader();
	virtual ~MemoryReader();

	bool Start(uk ptr, i32 size);

	//-------------------------------------------------------
	// IReader interface
	virtual void  Close();
	virtual i32 GetSize();
	virtual bool  SetPos(i32 pos);
	virtual bool  Read(uk buffer, size_t size);
	//-------------------------------------------------------

private:
	tuk m_ptr;
	i32 m_size;
	i32 m_offset;
};

tukk GetChunkTableEntries_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks);
tukk GetChunkTableEntries_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks);

tukk GetChunkTableEntries_0x746(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks);
tukk GetChunkTableEntries_0x746(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks);

tukk StripChunkHeaders_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks);
tukk StripChunkHeaders_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks);

}  // namespace ChunkFile

#endif
