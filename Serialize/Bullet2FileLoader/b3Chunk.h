#ifndef __BCHUNK_H__
#define __BCHUNK_H__

#if defined(_WIN32) && !defined(__MINGW32__)
#define b3Long64 __int64
#elif defined(__MINGW32__)
#include <stdint.h>
#define b3Long64 int64_t
#else
#define b3Long64 long long
#endif

namespace bParse
{
// ----------------------------------------------------- //
class bChunkPtr4
{
public:
	bChunkPtr4() {}
	i32 code;
	i32 len;
	union {
		i32 m_uniqueInt;
	};
	i32 dna_nr;
	i32 nr;
};

// ----------------------------------------------------- //
class bChunkPtr8
{
public:
	bChunkPtr8() {}
	i32 code, len;
	union {
		b3Long64 oldPrev;
		i32 m_uniqueInts[2];
	};
	i32 dna_nr, nr;
};

// ----------------------------------------------------- //
class bChunkInd
{
public:
	bChunkInd() {}
	i32 code, len;
	uk oldPtr;
	i32 dna_nr, nr;
};

// ----------------------------------------------------- //
class ChunkUtils
{
public:
	// file chunk offset
	static i32 getOffset(i32 flags);

	// endian utils
	static short swapShort(short sht);
	static i32 swapInt(i32 inte);
	static b3Long64 swapLong64(b3Long64 lng);
};

i32k CHUNK_HEADER_LEN = ((sizeof(bChunkInd)));
const bool VOID_IS_8 = ((sizeof(uk ) == 8));
}  // namespace bParse

#endif  //__BCHUNK_H__
