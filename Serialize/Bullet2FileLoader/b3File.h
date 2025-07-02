#ifndef __BFILE_H__
#define __BFILE_H__

#include <drx3D/Serialize/Bullet2FileLoader/b3Common.h>
#include <drx3D/Serialize/Bullet2FileLoader/b3Chunk.h>
#include <stdio.h>

namespace bParse
{
// ----------------------------------------------------- //
enum bFileFlags
{
	FD_INVALID = 0,
	FD_OK = 1,
	FD_VOID_IS_8 = 2,
	FD_ENDIAN_SWAP = 4,
	FD_FILE_64 = 8,
	FD_BITS_VARIES = 16,
	FD_VERSION_VARIES = 32,
	FD_DOUBLE_PRECISION = 64,
	FD_BROKEN_DNA = 128
};

enum bFileVerboseMode
{
	FD_VERBOSE_EXPORT_XML = 1,
	FD_VERBOSE_DUMP_DNA_TYPE_DEFINITIONS = 2,
	FD_VERBOSE_DUMP_CHUNKS = 4,
	FD_VERBOSE_DUMP_FILE_INFO = 8,
};
// ----------------------------------------------------- //
class bFile
{
protected:
	char m_headerString[7];

	bool mOwnsBuffer;
	char *mFileBuffer;
	i32 mFileLen;
	i32 mVersion;

	bPtrMap mLibPointers;

	i32 mDataStart;
	bDNA *mFileDNA;
	bDNA *mMemoryDNA;

	b3AlignedObjectArray<char *> m_pointerFixupArray;
	b3AlignedObjectArray<char *> m_pointerPtrFixupArray;

	b3AlignedObjectArray<bChunkInd> m_chunks;
	b3HashMap<b3HashPtr, bChunkInd> m_chunkPtrPtrMap;

	//

	bPtrMap mDataPointers;

	i32 mFlags;

	// ////////////////////////////////////////////////////////////////////////////

	// buffer offset util
	i32 getNextBlock(bChunkInd *dataChunk, tukk dataPtr, i32k flags);
	void safeSwapPtr(char *dst, tukk src);

	virtual void parseHeader();

	virtual void parseData() = 0;

	void resolvePointersMismatch();
	void resolvePointersChunk(const bChunkInd &dataChunk, i32 verboseMode);

	i32 resolvePointersStructRecursive(char *strcPtr, i32 old_dna, i32 verboseMode, i32 recursion);
	//void swapPtr(char *dst, char *src);

	void parseStruct(char *strcPtr, char *dtPtr, i32 old_dna, i32 new_dna, bool fixupPointers);
	void getMatchingFileDNA(short *old, tukk lookupName, tukk lookupType, char *strcData, char *data, bool fixupPointers);
	char *getFileElement(short *firstStruct, char *lookupName, char *lookupType, char *data, short **foundPos);

	void swap(char *head, class bChunkInd &ch, bool ignoreEndianFlag);
	void swapData(char *data, short type, i32 arraySize, bool ignoreEndianFlag);
	void swapStruct(i32 dna_nr, char *data, bool ignoreEndianFlag);
	void swapLen(char *dataPtr);
	void swapDNA(char *ptr);

	char *readStruct(char *head, class bChunkInd &chunk);
	char *getAsString(i32 code);

	void parseInternal(i32 verboseMode, char *memDna, i32 memDnaLength);

public:
	bFile(tukk filename, const char headerString[7]);

	//todo: make memoryBuffer const char
	//bFile( tukk memoryBuffer, i32 len);
	bFile(char *memoryBuffer, i32 len, const char headerString[7]);
	virtual ~bFile();

	bDNA *getFileDNA()
	{
		return mFileDNA;
	}

	virtual void addDataBlock(char *dataBlock) = 0;

	i32 getFlags() const
	{
		return mFlags;
	}

	bPtrMap &getLibPointers()
	{
		return mLibPointers;
	}

	uk findLibPointer(uk ptr);

	bool ok();

	virtual void parse(i32 verboseMode) = 0;

	virtual i32 write(tukk fileName, bool fixupPointers = false) = 0;

	virtual void writeChunks(FILE *fp, bool fixupPointers);

	virtual void writeDNA(FILE *fp) = 0;

	void updateOldPointers();
	void resolvePointers(i32 verboseMode);

	void dumpChunks(bDNA *dna);

	i32 getVersion() const
	{
		return mVersion;
	}
	//pre-swap the endianness, so that data loaded on a target with different endianness doesn't need to be swapped
	void preSwap();
	void writeFile(tukk fileName);
};
}  // namespace bParse

#endif  //__BFILE_H__
