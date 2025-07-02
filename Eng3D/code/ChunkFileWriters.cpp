// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла: ChunkFileWriters.cpp
//  Created:   2013/10/21 by Sergey Sokov
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>
#include <drx3D/Eng3D/ChunkFileWriters.h>

static inline size_t ComputeSizeOfAlignment(size_t pos, size_t alignment)
{
	if (alignment <= 1 || (alignment & (alignment - 1)))
	{
		return 0;
	}
	const size_t mask = alignment - 1;
	return (alignment - (pos & mask)) & mask;
}

namespace ChunkFile
{

//////////////////////////////////////////////////////////////////////////

bool IWriter::WriteZeros(size_t size)
{
	if (size <= 0)
	{
		return true;
	}

	char bf[1024];
	memset(bf, 0, (sizeof(bf) < size ? sizeof(bf) : size));

	while (size > 0)
	{
		u32k sz = (sizeof(bf) < size ? sizeof(bf) : size);
		size -= sz;
		if (!Write(bf, sz))
		{
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

OsFileWriter::OsFileWriter()
	: m_f(0)
{
}

OsFileWriter::~OsFileWriter()
{
	Erase();
}

bool OsFileWriter::Create(tukk filename)
{
	Erase();

	if (filename == 0 || filename[0] == 0)
	{
		// RCLogError("Filename is empty");
		return false;
	}

	m_filename = filename;

	m_f = fopen(filename, "wb");
	if (!m_f)
	{
		// RCLogError("Failed to create file %s.", m_filename.c_str());
		return false;
	}

	m_offset = 0;

	return true;
}

void OsFileWriter::Erase()
{
	if (m_f)
	{
		Close();
		::remove(m_filename.c_str());
	}
}

void OsFileWriter::Close()
{
	if (m_f)
	{
		fclose(m_f);
		m_f = 0;
	}
}

i32 OsFileWriter::GetPos() const
{
	return m_offset;
}

bool OsFileWriter::Write(ukk buffer, size_t size)
{
	if (!m_f)
	{
		return false;
	}

	if (size <= 0)
	{
		return true;
	}

	if (fwrite(buffer, size, 1, m_f) != 1)
	{
		// RCLogError("Failed to write %u byte(s) to file %s.", (uint)size, m_filename.c_str());
		Erase();
		return false;
	}

	m_offset += size;

	return true;
}

//////////////////////////////////////////////////////////////////////////
#if !defined(RESOURCE_COMPILER)

DrxPakFileWriter::DrxPakFileWriter()
	: m_pPak(0)
	, m_f(0)
{
}

DrxPakFileWriter::~DrxPakFileWriter()
{
	Erase();
}

bool DrxPakFileWriter::Create(IDrxPak* pPak, tukk filename)
{
	Erase();

	if (pPak == 0 || filename == 0 || filename[0] == 0)
	{
		return false;
	}

	m_pPak = pPak;
	m_filename = filename;

	m_f = m_pPak->FOpen(m_filename.c_str(), "w+b");

	if (!m_f)
	{
		return false;
	}

	m_offset = 0;

	return true;
}

void DrxPakFileWriter::Erase()
{
	if (m_f)
	{
		Close();
		m_pPak->RemoveFile(m_filename.c_str());
	}
}

void DrxPakFileWriter::Close()
{
	if (m_f)
	{
		m_pPak->FClose(m_f);
		m_f = 0;
	}
}

i32 DrxPakFileWriter::GetPos() const
{
	return m_offset;
}

bool DrxPakFileWriter::Write(ukk buffer, size_t size)
{
	if (!m_f)
	{
		return false;
	}

	if (size <= 0)
	{
		return true;
	}

	if (m_pPak->FWrite(buffer, 1, size, m_f) != size)
	{
		Erase();
		return false;
	}

	m_offset += size;

	return true;
}

#endif
//////////////////////////////////////////////////////////////////////////

MemoryWriter::MemoryWriter()
	: m_ptr(0)
	, m_size(0)
{
}

MemoryWriter::~MemoryWriter()
{
}

bool MemoryWriter::Start(uk ptr, i32 size)
{
	Erase();

	if (ptr == 0 || size <= 0)
	{
		return false;
	}

	m_ptr = (tuk)ptr;
	m_size = size;
	m_offset = 0;

	return true;
}

void MemoryWriter::Erase()
{
	m_ptr = 0;
	m_size = 0;
}

void MemoryWriter::Close()
{
	m_ptr = 0;
	m_size = 0;
}

i32 MemoryWriter::GetPos() const
{
	return m_offset;
}

bool MemoryWriter::Write(ukk buffer, size_t size)
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
		Erase();
		return false;
	}

	memcpy(&m_ptr[m_offset], buffer, size);
	m_offset += size;

	return true;
}

//////////////////////////////////////////////////////////////////////////

MemorylessChunkFileWriter::MemorylessChunkFileWriter(
  EChunkFileFormat eFormat,
  IWriter* pWriter)
	: m_eChunkFileFormat(eFormat)
	, m_pWriter(pWriter)
	, m_alignment(4)
	, m_chunkCount(0)
	, m_eState(eState_Init)
{
	if (!m_pWriter)
	{
		m_eState = eState_Fail;
	}
}

MemorylessChunkFileWriter::~MemorylessChunkFileWriter()
{
	if (m_eState != eState_Success)
	{
		Fail();
	}
}

void MemorylessChunkFileWriter::SetAlignment(size_t alignment)
{
	m_alignment = (alignment < 1) ? 1 : alignment;
}

bool MemorylessChunkFileWriter::StartPass()
{
	switch (m_eState)
	{
	case eState_Init:
		m_eState = eState_CountingChunks;
		m_chunkIndex = -1;
		break;
	case eState_CountingChunks:
		WriteFileHeader(m_chunkIndex + 1, GetSizeOfHeader());
		m_eState = eState_WritingChunkTable;
		WriteChunkTableHeader(m_chunkIndex + 1);
		m_dataOffsetInFile = GetSizeOfHeader() + GetSizeOfChunkTable(m_chunkIndex + 1);
		m_chunkIndex = -1;
		break;
	case eState_WritingChunkTable:
		if (m_chunkIndex >= 0)
		{
			WriteChunkEntry();
		}
		m_eState = eState_WritingData;
		m_dataOffsetInFile = GetSizeOfHeader() + GetSizeOfChunkTable(m_chunkIndex + 1);
		m_chunkIndex = -1;
		break;
	case eState_WritingData:
		m_eState = eState_Success;
		m_pWriter->Close();
		return false;
	case eState_Fail:
		return false;
	default:
		assert(0);
		Fail();
		return false;
	}
	return true;
}

void MemorylessChunkFileWriter::StartChunk(EEndianness eEndianness, u32 type, u32 version, u32 id)
{
	if (type != 0)
	{
		type = ConvertChunkTypeTo0x746(type);
		if (type == 0)
		{
			Fail();
			return;
		}
	}

	if (version >= ChunkFile::ChunkTableEntry_0x746::kBigEndianVersionFlag)
	{
		Fail();
		return;
	}

	switch (m_eState)
	{
	case eState_CountingChunks:
		++m_chunkIndex;
		break;
	case eState_WritingChunkTable:
		if (m_chunkIndex >= 0)
		{
			WriteChunkEntry();
		}
	/* fall through */
	case eState_WritingData:
		{
			size_t size = ComputeSizeOfAlignment(m_dataOffsetInFile, m_alignment);

			// Make sure that zero-length chunks have distinct positions in file
			if (size == 0 && m_chunkIndex > 0 && m_chunkSize == 0)
			{
				size = m_alignment;
			}

			m_dataOffsetInFile += size;
			m_chunkOffsetInFile = m_dataOffsetInFile;
			if (m_eState == eState_WritingData && !m_pWriter->WriteZeros(size))
			{
				Fail();
				return;
			}
		}

		++m_chunkIndex;
		m_chunkEndianness = eEndianness;
		m_chunkType = type;
		m_chunkVersion = version;
		m_chunkId = id;
		m_chunkSize = 0;

		if (m_eChunkFileFormat == eChunkFileFormat_0x745 &&
		    ChunkContainsHeader_0x744_0x745(m_chunkType, m_chunkVersion))
		{
			ChunkHeader_0x744_0x745 c;

			c.type = ConvertChunkTypeTo0x745(m_chunkType);
			c.version = m_chunkVersion | (m_chunkEndianness == eEndianness_Big ? ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag : 0);
			c.id = m_chunkId;
			c.offsetInFile = m_chunkOffsetInFile;

			if (SYSTEM_IS_BIG_ENDIAN)
			{
				c.SwapEndianness();
			}

			AddChunkData(&c, sizeof(c));
		}
		break;
	default:
		Fail();
		break;
	}
}

void MemorylessChunkFileWriter::AddChunkData(uk ptr, size_t size)
{
	if (m_chunkIndex < 0)
	{
		Fail();
		return;
	}

	switch (m_eState)
	{
	case eState_CountingChunks:
		break;
	case eState_WritingChunkTable:
	case eState_WritingData:
		m_chunkSize += size;
		m_dataOffsetInFile += size;
		if (m_eState == eState_WritingData && !m_pWriter->Write(ptr, size))
		{
			Fail();
		}
		break;
	default:
		Fail();
		break;
	}
}

void MemorylessChunkFileWriter::AddChunkDataZeros(size_t size)
{
	if (m_chunkIndex < 0)
	{
		Fail();
		return;
	}

	switch (m_eState)
	{
	case eState_CountingChunks:
		break;
	case eState_WritingChunkTable:
	case eState_WritingData:
		m_chunkSize += size;
		m_dataOffsetInFile += size;
		if (m_eState == eState_WritingData && !m_pWriter->WriteZeros(size))
		{
			Fail();
		}
		break;
	default:
		Fail();
		break;
	}
}

void MemorylessChunkFileWriter::AddChunkDataAlignment(size_t alignment)
{
	const size_t size = ComputeSizeOfAlignment(m_chunkSize, alignment);
	return AddChunkDataZeros(size);
}

bool MemorylessChunkFileWriter::HasWrittenSuccessfully() const
{
	return m_eState == eState_Success;
}

IWriter* MemorylessChunkFileWriter::GetWriter() const
{
	return m_pWriter;
}

//////////////////////////////////////////////////////////////////////////

void MemorylessChunkFileWriter::Fail()
{
	m_eState = eState_Fail;
	if (m_pWriter)
	{
		m_pWriter->Erase();
	}
}

size_t MemorylessChunkFileWriter::GetSizeOfHeader() const
{
	return (m_eChunkFileFormat == eChunkFileFormat_0x745)
	       ? sizeof(FileHeader_0x744_0x745)
	       : sizeof(FileHeader_0x746);
}

void MemorylessChunkFileWriter::WriteFileHeader(i32 chunkCount, u32 chunkTableOffsetInFile)
{
	if (m_eChunkFileFormat == eChunkFileFormat_0x745)
	{
		FileHeader_0x744_0x745 h;
		h.Set(chunkTableOffsetInFile);

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			h.SwapEndianness();
		}

		if (!m_pWriter->Write(&h, sizeof(h)))
		{
			Fail();
		}
	}
	else
	{
		FileHeader_0x746 h;
		h.Set(chunkCount, chunkTableOffsetInFile);

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			h.SwapEndianness();
		}

		if (!m_pWriter->Write(&h, sizeof(h)))
		{
			Fail();
		}
	}
}

size_t MemorylessChunkFileWriter::GetSizeOfChunkTable(i32 chunkCount) const
{
	if (m_eChunkFileFormat == eChunkFileFormat_0x745)
	{
		return sizeof(u32) + chunkCount * sizeof(ChunkTableEntry_0x745);
	}
	else
	{
		return chunkCount * sizeof(ChunkTableEntry_0x746);
	}
}

void MemorylessChunkFileWriter::WriteChunkTableHeader(i32 chunkCount)
{
	if (m_eChunkFileFormat == eChunkFileFormat_0x745)
	{
		if (SYSTEM_IS_BIG_ENDIAN)
		{
			SwapEndianBase(&chunkCount, 1);
		}

		if (!m_pWriter->Write(&chunkCount, sizeof(chunkCount)))
		{
			Fail();
		}
	}
}

void MemorylessChunkFileWriter::WriteChunkEntry()
{
	if (m_chunkIndex < 0)
	{
		assert(0);
		Fail();
		return;
	}

	if (m_eChunkFileFormat == eChunkFileFormat_0x745)
	{
		ChunkTableEntry_0x745 c;

		c.type = ConvertChunkTypeTo0x745(m_chunkType);
		c.version = m_chunkVersion | (m_chunkEndianness == eEndianness_Big ? ChunkFile::ChunkHeader_0x744_0x745::kBigEndianVersionFlag : 0);
		c.id = m_chunkId;
		c.size = m_chunkSize;
		c.offsetInFile = m_chunkOffsetInFile;

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			c.SwapEndianness();
		}

		if (!m_pWriter->Write(&c, sizeof(c)))
		{
			Fail();
		}
	}
	else
	{
		ChunkTableEntry_0x746 c;

		c.type = m_chunkType;
		c.version = m_chunkVersion | (m_chunkEndianness == eEndianness_Big ? ChunkFile::ChunkTableEntry_0x746::kBigEndianVersionFlag : 0);
		c.id = m_chunkId;
		c.size = m_chunkSize;
		c.offsetInFile = m_chunkOffsetInFile;

		if (SYSTEM_IS_BIG_ENDIAN)
		{
			c.SwapEndianness();
		}

		if (!m_pWriter->Write(&c, sizeof(c)))
		{
			Fail();
		}
	}
}

//////////////////////////////////////////////////////////////////////////

}  // namespace ChunkFile
