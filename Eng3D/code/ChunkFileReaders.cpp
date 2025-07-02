// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла: ChunkFileParsers.h
//  Created:   2013/11/18 by Sergey Sokov
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/DrxHeaders.h>
#include <drx3D/Eng3D/ChunkFileReaders.h>

namespace ChunkFile
{

//////////////////////////////////////////////////////////////////////////

DrxFileReader::DrxFileReader()
{
}

DrxFileReader::~DrxFileReader()
{
	Close();
}

bool DrxFileReader::Open(tukk filename)
{
	Close();

	if (filename == 0 || filename[0] == 0)
	{
		return false;
	}

	if (!m_f.Open(filename, "rb"))
	{
		return false;
	}

	m_offset = 0;

	return true;
}

void DrxFileReader::Close()
{
	m_f.Close();
}

i32 DrxFileReader::GetSize()
{
	return m_f.GetLength();
}

bool DrxFileReader::SetPos(i32 pos)
{
	if (pos < 0)
	{
		return false;
	}
	m_offset = pos;
	return m_f.Seek(m_offset, SEEK_SET) == 0;
}

bool DrxFileReader::Read(uk buffer, size_t size)
{
	return m_f.ReadRaw(buffer, size) == size;
}

//////////////////////////////////////////////////////////////////////////

MemoryReader::MemoryReader()
	: m_ptr(0)
	, m_size(0)
{
}

MemoryReader::~MemoryReader()
{
}

bool MemoryReader::Start(uk ptr, i32 size)
{
	if (ptr == 0 || size <= 0)
	{
		return false;
	}

	m_ptr = (tuk)ptr;
	m_size = size;
	m_offset = 0;

	return true;
}

void MemoryReader::Close()
{
}

i32 MemoryReader::GetSize()
{
	return m_size;
}

bool MemoryReader::SetPos(i32 pos)
{
	if (pos < 0 || pos > m_size)
	{
		return false;
	}
	m_offset = pos;
	return true;
}

bool MemoryReader::Read(uk buffer, size_t size)
{
	if (!m_ptr)
	{
		return false;
	}

	if (size <= 0)
	{
		return true;
	}

	if ((size_t)m_offset + size > (size_t)m_size)
	{
		return false;
	}

	memcpy(buffer, &m_ptr[m_offset], size);
	m_offset += size;

	return true;
}

//////////////////////////////////////////////////////////////////////////

namespace
{
class ChunkListRef
{
	std::vector<IChunkFile::ChunkDesc>& m_chunks;

public:
	ChunkListRef(std::vector<IChunkFile::ChunkDesc>& chunks)
		: m_chunks(chunks)
	{
	}

	void Clear()
	{
		for (size_t i = 0; i < m_chunks.size(); ++i)
		{
			if (m_chunks[i].data)
			{
				delete[] (tuk)m_chunks[i].data;
				m_chunks[i].data = 0;
			}
		}
		m_chunks.clear();
	}

	void Create(size_t count)
	{
		Clear();
		m_chunks.resize(count);
		for (size_t i = 0; i < count; ++i)
		{
			m_chunks[i].data = 0;
			m_chunks[i].size = 0;
		}
	}

	void Sort()
	{
		std::sort(m_chunks.begin(), m_chunks.end(), IChunkFile::ChunkDesc::LessOffset);
	}

	size_t GetCount() const
	{
		return m_chunks.size();
	}

	IChunkFile::ChunkDesc& Get(size_t index)
	{
		return m_chunks[index];
	}
};

class ChunkPtrListRef
{
	std::vector<IChunkFile::ChunkDesc*>& m_chunks;

public:
	ChunkPtrListRef(std::vector<IChunkFile::ChunkDesc*>& chunks)
		: m_chunks(chunks)
	{
	}

	void Clear()
	{
		for (size_t i = 0; i < m_chunks.size(); ++i)
		{
			if (m_chunks[i])
			{
				if (m_chunks[i]->data)
				{
					delete[] (tuk)(m_chunks[i]->data);
					m_chunks[i]->data = 0;
				}
				delete m_chunks[i];
			}
		}
		m_chunks.clear();
	}

	void Create(size_t count)
	{
		Clear();
		m_chunks.resize(count, 0);
		for (size_t i = 0; i < count; ++i)
		{
			m_chunks[i] = new IChunkFile::ChunkDesc;
			m_chunks[i]->data = 0;
			m_chunks[i]->size = 0;
		}
	}

	void Sort()
	{
		std::sort(m_chunks.begin(), m_chunks.end(), IChunkFile::ChunkDesc::LessOffsetByPtr);
	}

	size_t GetCount() const
	{
		return m_chunks.size();
	}

	IChunkFile::ChunkDesc& Get(size_t index)
	{
		return *m_chunks[index];
	}
};
} //endns

//////////////////////////////////////////////////////////////////////////

template<class TListRef>
static tukk GetChunkTableEntries_0x744_0x745_Tpl(IReader* pReader, TListRef& chunks)
{
	chunks.Clear();

	ChunkFile::FileHeader_0x744_0x745 header;

	if (!pReader->SetPos(0) ||
	    !pReader->Read(&header, sizeof(header)))
	{
		return "Cannot read header of chunk file";
	}

	if (!header.HasValidSignature())
	{
		return "Unknown signature in chunk file";
	}

	if (SYSTEM_IS_BIG_ENDIAN)
	{
		header.SwapEndianness();
	}

	if (header.version != 0x744 && header.version != 0x745)
	{
		return "Version of chunk file is neither 0x744 nor 0x745";
	}

	if (header.fileType != header.eFileType_Geom && header.fileType != header.eFileType_Anim)
	{
		return "Type of chunk file is neither FileType_Geom nor FileType_Anim";
	}

	u32 chunkCount = 0;
	{
		if (!pReader->SetPos(header.chunkTableOffset) ||
		    !pReader->Read(&chunkCount, sizeof(chunkCount)))
		{
			return "Failed to read # of chunks";
		}

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			SwapEndianBase(&chunkCount, 1);
		}

		if (chunkCount > 1000000)
		{
			return "Invalid # of chunks in file";
		}
	}

	if (chunkCount <= 0)
	{
		return 0;
	}

	chunks.Create(chunkCount);

	if (header.version == 0x744)
	{
		std::vector<ChunkFile::ChunkTableEntry_0x744> srcChunks;
		srcChunks.resize(chunkCount);

		if (!pReader->Read(&srcChunks[0], sizeof(srcChunks[0]) * srcChunks.size()))
		{
			return "Failed to read chunk entries from file";
		}

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			for (u32 i = 0; i < chunkCount; ++i)
			{
				srcChunks[i].SwapEndianness();
			}
		}

		for (u32 i = 0; i < chunkCount; ++i)
		{
			IChunkFile::ChunkDesc& cd = chunks.Get(i);

			cd.chunkType = (ChunkTypes)ConvertChunkTypeTo0x746(srcChunks[i].type);
			cd.chunkVersion = srcChunks[i].version & ~ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag;
			cd.chunkId = srcChunks[i].id;
			cd.fileOffset = srcChunks[i].offsetInFile;
			cd.bSwapEndian = (srcChunks[i].version & ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag) ? SYSTEM_IS_LITTLE_ENDIAN : SYSTEM_IS_BIG_ENDIAN;
		}

		chunks.Sort();

		u32k endOfChunkData = (header.chunkTableOffset < chunks.Get(0).fileOffset)
		                              ? (u32)pReader->GetSize()
		                              : header.chunkTableOffset;

		for (u32 i = 0; i < chunkCount; ++i)
		{
			// calculate chunk size based on the next (by offset in file) chunk or
			// on the end of the chunk data portion of the file

			const size_t nextOffsetInFile = (i + 1 < chunkCount)
			                                ? chunks.Get(i + 1).fileOffset
			                                : endOfChunkData;

			chunks.Get(i).size = nextOffsetInFile - chunks.Get(i).fileOffset;
		}
	}
	else // header.version == 0x745
	{
		std::vector<ChunkFile::ChunkTableEntry_0x745> srcChunks;
		srcChunks.resize(chunkCount);

		if (!pReader->Read(&srcChunks[0], sizeof(srcChunks[0]) * srcChunks.size()))
		{
			return "Failed to read chunk entries from file.";
		}

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			for (u32 i = 0; i < chunkCount; ++i)
			{
				srcChunks[i].SwapEndianness();
			}
		}

		for (u32 i = 0; i < chunkCount; ++i)
		{
			IChunkFile::ChunkDesc& cd = chunks.Get(i);

			cd.chunkType = (ChunkTypes)ConvertChunkTypeTo0x746(srcChunks[i].type);
			cd.chunkVersion = srcChunks[i].version & ~ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag;
			cd.chunkId = srcChunks[i].id;
			cd.fileOffset = srcChunks[i].offsetInFile;
			cd.size = srcChunks[i].size;
			cd.bSwapEndian = (srcChunks[i].version & ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag) ? SYSTEM_IS_LITTLE_ENDIAN : SYSTEM_IS_BIG_ENDIAN;
		}
	}

	u32k fileSize = (u32)pReader->GetSize();

	for (u32 i = 0; i < chunkCount; ++i)
	{
		const IChunkFile::ChunkDesc& cd = chunks.Get(i);

		if (cd.size + cd.fileOffset > fileSize)
		{
			return "Data in chunk file are corrupted";
		}
	}

	return 0;
}

template<class TListRef>
static tukk GetChunkTableEntries_0x746_Tpl(IReader* pReader, TListRef& chunks)
{
	chunks.Clear();

	ChunkFile::FileHeader_0x746 header;

	if (!pReader->SetPos(0) ||
	    !pReader->Read(&header, sizeof(header)))
	{
		return "Cannot read header from file.";
	}

	if (!header.HasValidSignature())
	{
		return "Unknown signature in chunk file";
	}

	if (SYSTEM_IS_BIG_ENDIAN)
	{
		header.SwapEndianness();
	}

	if (header.version != 0x746)
	{
		return "Version of chunk file is not 0x746";
	}

	if (header.chunkCount > 10000000)
	{
		return "Invalid # of chunks in file.";
	}

	if (header.chunkCount <= 0)
	{
		return 0;
	}

	chunks.Create(header.chunkCount);

	std::vector<ChunkFile::ChunkTableEntry_0x746> srcChunks;
	srcChunks.resize(header.chunkCount);

	if (!pReader->SetPos(header.chunkTableOffset) ||
	    !pReader->Read(&srcChunks[0], sizeof(srcChunks[0]) * srcChunks.size()))
	{
		return "Failed to read chunk entries from file";
	}

	if (SYSTEM_IS_BIG_ENDIAN)
	{
		for (u32 i = 0; i < header.chunkCount; ++i)
		{
			srcChunks[i].SwapEndianness();
		}
	}

	for (size_t i = 0, n = chunks.GetCount(); i < n; ++i)
	{
		IChunkFile::ChunkDesc& cd = chunks.Get(i);

		cd.chunkType = (ChunkTypes)srcChunks[i].type;
		cd.chunkVersion = srcChunks[i].version & ~ChunkFile::ChunkTableEntry_0x746::kBigEndianVersionFlag;
		cd.chunkId = srcChunks[i].id;
		cd.size = srcChunks[i].size;
		cd.fileOffset = srcChunks[i].offsetInFile;
		cd.bSwapEndian = (srcChunks[i].version & ChunkFile::ChunkTableEntry_0x746::kBigEndianVersionFlag) ? SYSTEM_IS_LITTLE_ENDIAN : SYSTEM_IS_BIG_ENDIAN;
	}

	return 0;
}

template<class TListRef>
static tukk StripChunkHeaders_0x744_0x745_Tpl(IReader* pReader, TListRef& chunks)
{
	for (size_t i = 0, n = chunks.GetCount(); i < n; ++i)
	{
		IChunkFile::ChunkDesc& ct = chunks.Get(i);

		if (ChunkFile::ChunkContainsHeader_0x744_0x745(ct.chunkType, ct.chunkVersion))
		{
			ChunkFile::ChunkHeader_0x744_0x745 ch;
			if (ct.size < sizeof(ch))
			{
				return "Damaged data: reported size of chunk data is less that size of the chunk header";
			}

			// Validation
			{
				if (!pReader->SetPos(ct.fileOffset) ||
				    !pReader->Read(&ch, sizeof(ch)))
				{
					return "Failed to read chunk header from file";
				}

				if (SYSTEM_IS_BIG_ENDIAN)
				{
					ch.SwapEndianness();
				}

				ch.version &= ~ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag;

				if (ConvertChunkTypeTo0x746(ch.type) != ct.chunkType ||
				    ch.version != ct.chunkVersion ||
				    ch.id != ct.chunkId)
				{
					return "Data in a chunk header don't match data in the chunk table";
				}

				// The following check is commented out because we have (on 2013/11/25)
				// big number of .cgf files in Drxsis 3 that fail to pass the check.
				//if (ch.offsetInFile != ct.fileOffset)
				//{
				//	return "File offset data in a chunk header don't match data in the chunk table";
				//}
			}

			ct.fileOffset += sizeof(ch);
			ct.size -= sizeof(ch);

			if (ct.data)
			{
				ct.data = ((tuk)ct.data) + sizeof(ch);
			}
		}

		if (ct.size < 0)
		{
			return "A negative-length chunk found in file";
		}
	}

	return 0;
}

tukk GetChunkTableEntries_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks)
{
	ChunkListRef c(chunks);
	return GetChunkTableEntries_0x744_0x745_Tpl(pReader, c);
}

tukk GetChunkTableEntries_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks)
{
	ChunkPtrListRef c(chunks);
	return GetChunkTableEntries_0x744_0x745_Tpl(pReader, c);
}

tukk GetChunkTableEntries_0x746(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks)
{
	ChunkListRef c(chunks);
	return GetChunkTableEntries_0x746_Tpl(pReader, c);
}

tukk GetChunkTableEntries_0x746(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks)
{
	ChunkPtrListRef c(chunks);
	return GetChunkTableEntries_0x746_Tpl(pReader, c);
}

tukk StripChunkHeaders_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc>& chunks)
{
	ChunkListRef c(chunks);
	return StripChunkHeaders_0x744_0x745_Tpl(pReader, c);
}

tukk StripChunkHeaders_0x744_0x745(IReader* pReader, std::vector<IChunkFile::ChunkDesc*>& chunks)
{
	ChunkPtrListRef c(chunks);
	return StripChunkHeaders_0x744_0x745_Tpl(pReader, c);
}

}  // namespace ChunkFile
