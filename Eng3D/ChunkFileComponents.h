// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __ChunkFileComponents_h__
#define __ChunkFileComponents_h__

#include <drx3D/Eng3D/DrxHeaders.h>

namespace ChunkFile
{

// All chunk files use *little-endian* format to store file header and chunk.
// Chunk data are stored in either little-endian or big-endian format,
// see kBigEndianVersionFlag.

struct FileHeader_0x744_0x745
{
	char   signature[7];
	char   _pad_[1];
	u32 fileType;
	u32 version;
	u32 chunkTableOffset;

	enum EFileType
	{
		eFileType_Geom = 0xFFFF0000U,
		eFileType_Anim,
	};

	static tukk GetExpectedSignature()
	{
		return "DrxTek";
	}

	bool HasValidSignature() const
	{
		return memcmp(GetExpectedSignature(), signature, sizeof(signature)) == 0;
	}

	void Set(u32 a_chunkTableOffset)
	{
		memcpy(signature, GetExpectedSignature(), sizeof(signature));
		_pad_[0] = 0;
		// We need to set eFileType_Geom or eFileType_Anim, but asking
		// the caller to provide us the type will complicate the code, so we
		// set eFileType_Geom only. It's ok because all our readers
		// don't differentiate between eFileType_Geom and eFileType_Anim.
		fileType = eFileType_Geom;
		version = 0x745;
		chunkTableOffset = a_chunkTableOffset;
	}

	void SwapEndianness()
	{
		SwapEndianBase(&fileType, 1);
		SwapEndianBase(&version, 1);
		SwapEndianBase(&chunkTableOffset, 1);
	}
};

struct FileHeader_0x746
{
	char               signature[4];
	u32             version;
	u32             chunkCount;
	u32             chunkTableOffset;

	static tukk GetExpectedSignature()
	{
		return "CrCh";
	}

	bool HasValidSignature() const
	{
		return memcmp(GetExpectedSignature(), signature, sizeof(signature)) == 0;
	}

	void Set(i32 a_chunkCount, u32 a_chunkTableOffset)
	{
		memcpy(signature, GetExpectedSignature(), sizeof(signature));
		version = 0x746;
		chunkCount = a_chunkCount;
		chunkTableOffset = a_chunkTableOffset;
	}

	void SwapEndianness()
	{
		SwapEndianBase(&version, 1);
		SwapEndianBase(&chunkCount, 1);
		SwapEndianBase(&chunkTableOffset, 1);
	}
};

struct ChunkHeader_0x744_0x745
{
	u32 type;
	u32 version;
	u32 offsetInFile;
	u32 id;

	enum { kBigEndianVersionFlag = 0x80000000U };

	void SwapEndianness()
	{
		SwapEndianBase(&type, 1);
		SwapEndianBase(&version, 1);
		SwapEndianBase(&offsetInFile, 1);
		SwapEndianBase(&id, 1);
	}
};

struct ChunkTableEntry_0x744
	: public ChunkHeader_0x744_0x745
{
	void SwapEndianness()
	{
		ChunkHeader_0x744_0x745::SwapEndianness();
	}
};

struct ChunkTableEntry_0x745
	: public ChunkHeader_0x744_0x745
{
	u32 size;

	void SwapEndianness()
	{
		ChunkHeader_0x744_0x745::SwapEndianness();
		SwapEndianBase(&size, 1);
	}
};

struct ChunkTableEntry_0x746
{
	u16 type;
	u16 version;
	u32 id;
	u32 size;
	u32 offsetInFile;

	enum { kBigEndianVersionFlag = 0x8000U };

	void SwapEndianness()
	{
		SwapEndianBase(&type, 1);
		SwapEndianBase(&version, 1);
		SwapEndianBase(&id, 1);
		SwapEndianBase(&size, 1);
		SwapEndianBase(&offsetInFile, 1);
	}
};

// We need this function to strip 0x744 & 0x745 chunk headers
// from chunk data properly: some chunks in 0x744 and 0x745 formats
// don't have chunk headers in their data.
// 'chunkType' is expected to be provided in the 0x746 format.
inline bool ChunkContainsHeader_0x744_0x745(u16k chunkType, u16k chunkVersion)
{
	switch (chunkType)
	{
	case ChunkType_SourceInfo:
		return false;
	case ChunkType_Controller:
		return (chunkVersion != CONTROLLER_CHUNK_DESC_0827::VERSION);
	case ChunkType_BoneNameList:
		return (chunkVersion != BONENAMELIST_CHUNK_DESC_0745::VERSION);
	case ChunkType_MeshMorphTarget:
		return (chunkVersion != MESHMORPHTARGET_CHUNK_DESC_0001::VERSION);
	case ChunkType_BoneInitialPos:
		return (chunkVersion != BONEINITIALPOS_CHUNK_DESC_0001::VERSION);
	default:
		return true;
	}
}

static inline u16 ConvertChunkTypeTo0x746(u32 type)
{
	if (type <= 0xFFFF)
	{
		// Input type seems to be already in 0x746 format (or it's 0)
		return type;
	}

	// Input type seems to be in 0x745 format

	if ((type & 0xFFFF) >= 0xF000)
	{
		// Cannot fit into resulting 0x746 type (u16)
		return 0;
	}
	if ((type & 0xFFFF0000U) == 0xCCCC0000U)
	{
		return 0x1000 + (type & 0x0FFF);
	}
	if ((type & 0xFFFF0000U) == 0xACDC0000U)
	{
		return 0x2000 + (type & 0x0FFF);
	}
	if ((type & 0xFFFF0000U) == 0xAAFC0000U)
	{
		return 0x3000 + (type & 0x0FFF);
	}

	// Unknown 0x745 chunk type
	return 0;
}

static inline u32 ConvertChunkTypeTo0x745(u32 type)
{
	if (type > 0xFFFF)
	{
		// Input type seems to be already in 0x745 format
		return type;
	}

	// Input type seems to be in 0x746 format

	if ((type & 0xF000) == 0x1000)
	{
		return 0xCCCC0000U + (type & 0x0FFF);
	}
	if ((type & 0xF000) == 0x2000)
	{
		return 0xACDC0000U + (type & 0x0FFF);
	}
	if ((type & 0xF000) == 0x3000)
	{
		return 0xAAFC0000U + (type & 0x0FFF);
	}

	// Unknown 0x746 chunk type (or it's 0)
	return 0;
}

} //endns ChunkFile

#endif
