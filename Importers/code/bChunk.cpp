#include "../bChunk.h"
#include "../bDefines.h"
#include "../bFile.h"

#if !defined(__CELLOS_LV2__) && !defined(__MWERKS__)
#include <memory.h>
#endif
#include <string.h>

using namespace bParse;

// ----------------------------------------------------- //
short ChunkUtils::swapShort(short sht)
{
	SWITCH_SHORT(sht);
	return sht;
}

// ----------------------------------------------------- //
i32 ChunkUtils::swapInt(i32 inte)
{
	SWITCH_INT(inte);
	return inte;
}

// ----------------------------------------------------- //
long64 ChunkUtils::swapLong64(long64 lng)
{
	SWITCH_LONGINT(lng);
	return lng;
}

// ----------------------------------------------------- //
i32 ChunkUtils::getOffset(i32 flags)
{
	// if the file is saved in a
	// different format, get the
	// file's chunk size
	i32 res = CHUNK_HEADER_LEN;

	if (VOID_IS_8)
	{
		if (flags & FD_BITS_VARIES)
			res = sizeof(bChunkPtr4);
	}
	else
	{
		if (flags & FD_BITS_VARIES)
			res = sizeof(bChunkPtr8);
	}
	return res;
}

//eof
