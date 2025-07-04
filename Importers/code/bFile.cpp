#include "../bFile.h"
#include "../bCommon.h"
#include "../bChunk.h"
#include "../bDNA.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "../bDefines.h"
#include <drx3D/Maths/Linear/Serializer.h>
#include <drx3D/Maths/Linear/AlignedAllocator.h>
#include <drx3D/Maths/Linear/MinMax.h>

#define SIZEOFBLENDERHEADER 12
#define MAX_ARRAY_LENGTH 512
using namespace bParse;
#define MAX_STRLEN 1024

tukk getCleanName(tukk memName, char *buffer)
{
	i32 slen = strlen(memName);
	assert(slen < MAX_STRLEN);
	slen = d3Min(slen, MAX_STRLEN);
	for (i32 i = 0; i < slen; i++)
	{
		if (memName[i] == ']' || memName[i] == '[')
		{
			buffer[i] = 0;  //'_';
		}
		else
		{
			buffer[i] = memName[i];
		}
	}
	buffer[slen] = 0;
	return buffer;
}

i32 numallocs = 0;

// ----------------------------------------------------- //
bFile::bFile(tukk filename, const char headerString[7])
	: mOwnsBuffer(true),
	  mFileBuffer(0),
	  mFileLen(0),
	  mVersion(0),
	  mDataStart(0),
	  mFileDNA(0),
	  mMemoryDNA(0),
	  mFlags(FD_INVALID)
{
	for (i32 i = 0; i < 7; i++)
	{
		m_headerString[i] = headerString[i];
	}

	FILE *fp = fopen(filename, "rb");
	if (fp)
	{
		fseek(fp, 0L, SEEK_END);
		mFileLen = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		mFileBuffer = (char *)malloc(mFileLen + 1);
                memset(mFileBuffer, 0, mFileLen+1);
		size_t bytesRead;
		bytesRead = fread(mFileBuffer, mFileLen, 1, fp);

		fclose(fp);

		//
		parseHeader();
	}
}

// ----------------------------------------------------- //
bFile::bFile(char *memoryBuffer, i32 len, const char headerString[7])
	: mOwnsBuffer(false),
	  mFileBuffer(0),
	  mFileLen(0),
	  mVersion(0),
	  mDataStart(0),
	  mFileDNA(0),
	  mMemoryDNA(0),
	  mFlags(FD_INVALID)
{
	for (i32 i = 0; i < 7; i++)
	{
		m_headerString[i] = headerString[i];
	}
	mFileBuffer = memoryBuffer;
	mFileLen = len;

	parseHeader();
}

// ----------------------------------------------------- //
bFile::~bFile()
{
	if (mOwnsBuffer && mFileBuffer)
	{
		free(mFileBuffer);
		mFileBuffer = 0;
	}

	delete mMemoryDNA;
	delete mFileDNA;
}

// ----------------------------------------------------- //
void bFile::parseHeader()
{
	if (!mFileLen || !mFileBuffer)
		return;

	char *blenderBuf = mFileBuffer;
	char header[SIZEOFBLENDERHEADER + 1];
	memcpy(header, blenderBuf, SIZEOFBLENDERHEADER);
	header[SIZEOFBLENDERHEADER] = '\0';

	if (strncmp(header, m_headerString, 6) != 0)
	{
		memcpy(header, m_headerString, SIZEOFBLENDERHEADER);
		return;
	}

	if (header[6] == 'd')
	{
		mFlags |= FD_DOUBLE_PRECISION;
	}

	char *ver = header + 9;
	mVersion = atoi(ver);
	if (mVersion <= 241)
	{
		//printf("Warning, %d not fully tested : <= 242\n", mVersion);
	}

	i32 littleEndian = 1;
	littleEndian = ((char *)&littleEndian)[0];

	// swap ptr sizes...
	if (header[7] == '-')
	{
		mFlags |= FD_FILE_64;
		if (!VOID_IS_8)
			mFlags |= FD_BITS_VARIES;
	}
	else if (VOID_IS_8)
		mFlags |= FD_BITS_VARIES;

	// swap endian...
	if (header[8] == 'V')
	{
		if (littleEndian == 1)
			mFlags |= FD_ENDIAN_SWAP;
	}
	else if (littleEndian == 0)
		mFlags |= FD_ENDIAN_SWAP;

	mFlags |= FD_OK;
}

// ----------------------------------------------------- //
bool bFile::ok()
{
	return (mFlags & FD_OK) != 0;
}

void bFile::setFileDNA(i32 verboseMode, char *dnaBuffer, i32 dnaLen)
{
	mFileDNA = new bDNA();

	///mFileDNA->init will convert part of DNA file endianness to current CPU endianness if necessary
	mFileDNA->init((char *)dnaBuffer, dnaLen, (mFlags & FD_ENDIAN_SWAP) != 0);

	if (verboseMode & FD_VERBOSE_DUMP_DNA_TYPE_DEFINITIONS)
		mFileDNA->dumpTypeDefinitions();
}

// ----------------------------------------------------- //
void bFile::parseInternal(i32 verboseMode, char *memDna, i32 memDnaLength)
{
	if ((mFlags & FD_OK) == 0)
		return;

	if (mFlags & FD_FILEDNA_IS_MEMDNA)
	{
		setFileDNA(verboseMode, memDna, memDnaLength);
	}

	if (mFileDNA == 0)
	{
		char *blenderData = mFileBuffer;
		bChunkInd dna;
		dna.oldPtr = 0;

		char *tempBuffer = blenderData;
		for (i32 i = 0; i < mFileLen; i++)
		{
			// looking for the data's starting position
			// and the start of SDNA decls

			if (!mDataStart && strncmp(tempBuffer, "REND", 4) == 0)
				mDataStart = i;

			if (strncmp(tempBuffer, "DNA1", 4) == 0)
			{
				// read the DNA1 block and extract SDNA
				if (getNextBlock(&dna, tempBuffer, mFlags) > 0)
				{
					if (strncmp((tempBuffer + ChunkUtils::getOffset(mFlags)), "SDNANAME", 8) == 0)
						dna.oldPtr = (tempBuffer + ChunkUtils::getOffset(mFlags));
					else
						dna.oldPtr = 0;
				}
				else
					dna.oldPtr = 0;
			}
			// Some drx3D files are missing the DNA1 block
			// In Blender it's DNA1 + ChunkUtils::getOffset() + SDNA + NAME
			// In drx3D tests its SDNA + NAME
			else if (strncmp(tempBuffer, "SDNANAME", 8) == 0)
			{
				dna.oldPtr = blenderData + i;
				dna.len = mFileLen - i;

				// Also no REND block, so exit now.
				if (mVersion == 276) break;
			}

			if (mDataStart && dna.oldPtr) break;
			tempBuffer++;
		}
		if (!dna.oldPtr || !dna.len)
		{
			//printf("Failed to find DNA1+SDNA pair\n");
			mFlags &= ~FD_OK;
			return;
		}

		mFileDNA = new bDNA();

		///mFileDNA->init will convert part of DNA file endianness to current CPU endianness if necessary
		mFileDNA->init((char *)dna.oldPtr, dna.len, (mFlags & FD_ENDIAN_SWAP) != 0);

		if (mVersion == 276)
		{
			i32 i;
			for (i = 0; i < mFileDNA->getNumNames(); i++)
			{
				if (strcmp(mFileDNA->getName(i), "i32") == 0)
				{
					mFlags |= FD_BROKEN_DNA;
				}
			}
			if ((mFlags & FD_BROKEN_DNA) != 0)
			{
				//printf("warning: fixing some broken DNA version\n");
			}
		}

		if (verboseMode & FD_VERBOSE_DUMP_DNA_TYPE_DEFINITIONS)
			mFileDNA->dumpTypeDefinitions();
	}
	mMemoryDNA = new bDNA();
	i32 littleEndian = 1;
	littleEndian = ((char *)&littleEndian)[0];

	mMemoryDNA->init(memDna, memDnaLength, littleEndian == 0);

	///@todo we need a better version check, add version/sub version info from FileGlobal into memory DNA/header files
	if (mMemoryDNA->getNumNames() != mFileDNA->getNumNames())
	{
		mFlags |= FD_VERSION_VARIES;
		//printf ("Warning, file DNA is different than built in, performance is reduced. Best to re-export file with a matching version/platform");
	}

	// as long as it kept up to date it will be ok!!
	if (mMemoryDNA->lessThan(mFileDNA))
	{
		//printf ("Warning, file DNA is newer than built in.");
	}

	mFileDNA->initCmpFlags(mMemoryDNA);

	parseData();

	resolvePointers(verboseMode);

	updateOldPointers();
}

// ----------------------------------------------------- //
void bFile::swap(char *head, bChunkInd &dataChunk, bool ignoreEndianFlag)
{
	char *data = head;
	short *strc = mFileDNA->getStruct(dataChunk.dna_nr);

	const char s[] = "SoftBodyMaterialData";
	i32 szs = sizeof(s);
	if (strncmp((char *)&dataChunk.code, "ARAY", 4) == 0)
	{
		short *oldStruct = mFileDNA->getStruct(dataChunk.dna_nr);
		char *oldType = mFileDNA->getType(oldStruct[0]);
		if (strncmp(oldType, s, szs) == 0)
		{
			return;
		}
	}

	i32 len = mFileDNA->getLength(strc[0]);

	for (i32 i = 0; i < dataChunk.nr; i++)
	{
		swapStruct(dataChunk.dna_nr, data, ignoreEndianFlag);
		data += len;
	}
}

void bFile::swapLen(char *dataPtr)
{
	const bool VOID_IS_8 = ((sizeof(uk ) == 8));
	if (VOID_IS_8)
	{
		if (mFlags & FD_BITS_VARIES)
		{
			bChunkPtr4 *c = (bChunkPtr4 *)dataPtr;
			if ((c->code & 0xFFFF) == 0)
				c->code >>= 16;
			SWITCH_INT(c->len);
			SWITCH_INT(c->dna_nr);
			SWITCH_INT(c->nr);
		}
		else
		{
			bChunkPtr8 *c = (bChunkPtr8 *)dataPtr;
			if ((c->code & 0xFFFF) == 0)
				c->code >>= 16;
			SWITCH_INT(c->len);
			SWITCH_INT(c->dna_nr);
			SWITCH_INT(c->nr);
		}
	}
	else
	{
		if (mFlags & FD_BITS_VARIES)
		{
			bChunkPtr8 *c = (bChunkPtr8 *)dataPtr;
			if ((c->code & 0xFFFF) == 0)
				c->code >>= 16;
			SWITCH_INT(c->len);
			SWITCH_INT(c->dna_nr);
			SWITCH_INT(c->nr);
		}
		else
		{
			bChunkPtr4 *c = (bChunkPtr4 *)dataPtr;
			if ((c->code & 0xFFFF) == 0)
				c->code >>= 16;
			SWITCH_INT(c->len);

			SWITCH_INT(c->dna_nr);
			SWITCH_INT(c->nr);
		}
	}
}

void bFile::swapDNA(char *ptr)
{
	bool swap = ((mFlags & FD_ENDIAN_SWAP) != 0);

	i32 offset = (mFlags & FD_FILE_64) ? 24 : 20;
	char *data = &ptr[offset];

	//	void bDNA::init(char *data, i32 len, bool swap)
	i32 *intPtr = 0;
	short *shtPtr = 0;
	char *cp = 0;
	i32 dataLen = 0;
	intPtr = (i32 *)data;

	/*
		SDNA (4 bytes) (magic number)
		NAME (4 bytes)
		<nr> (4 bytes) amount of names (i32)
		<string>
		<string>
	*/

	if (strncmp(data, "SDNA", 4) == 0)
	{
		// skip ++ NAME
		intPtr++;
		intPtr++;
	}
	else
	{
		if (strncmp(data + 4, "SDNA", 4) == 0)
		{
			// skip ++ NAME
			intPtr++;
			intPtr++;
			intPtr++;
		}
	}

	// Parse names
	if (swap)
		dataLen = ChunkUtils::swapInt(*intPtr);
	else
		dataLen = *intPtr;

	*intPtr = ChunkUtils::swapInt(*intPtr);
	intPtr++;

	cp = (char *)intPtr;
	i32 i;
	for (i = 0; i < dataLen; i++)
	{
		while (*cp) cp++;
		cp++;
	}

	cp = AlignPointer(cp, 4);

	/*
		TYPE (4 bytes)
		<nr> amount of types (i32)
		<string>
		<string>
	*/

	intPtr = (i32 *)cp;
	assert(strncmp(cp, "TYPE", 4) == 0);
	intPtr++;

	if (swap)
		dataLen = ChunkUtils::swapInt(*intPtr);
	else
		dataLen = *intPtr;

	*intPtr = ChunkUtils::swapInt(*intPtr);

	intPtr++;

	cp = (char *)intPtr;
	for (i = 0; i < dataLen; i++)
	{
		while (*cp) cp++;
		cp++;
	}

	cp = AlignPointer(cp, 4);

	/*
		TLEN (4 bytes)
		<len> (short) the lengths of types
		<len>
	*/

	// Parse type lens
	intPtr = (i32 *)cp;
	assert(strncmp(cp, "TLEN", 4) == 0);
	intPtr++;

	shtPtr = (short *)intPtr;
	for (i = 0; i < dataLen; i++, shtPtr++)
	{
		//??????if (swap)
		shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
	}

	if (dataLen & 1)
		shtPtr++;

	/*
		STRC (4 bytes)
		<nr> amount of structs (i32)
		<typenr>
		<nr_of_elems>
		<typenr>
		<namenr>
		<typenr>
		<namenr>
	*/

	intPtr = (i32 *)shtPtr;
	cp = (char *)intPtr;
	assert(strncmp(cp, "STRC", 4) == 0);
	intPtr++;

	if (swap)
		dataLen = ChunkUtils::swapInt(*intPtr);
	else
		dataLen = *intPtr;

	*intPtr = ChunkUtils::swapInt(*intPtr);

	intPtr++;

	shtPtr = (short *)intPtr;
	for (i = 0; i < dataLen; i++)
	{
		//if (swap)
		{
			i32 len = shtPtr[1];

			shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
			shtPtr[1] = ChunkUtils::swapShort(shtPtr[1]);

			shtPtr += 2;

			for (i32 a = 0; a < len; a++, shtPtr += 2)
			{
				shtPtr[0] = ChunkUtils::swapShort(shtPtr[0]);
				shtPtr[1] = ChunkUtils::swapShort(shtPtr[1]);
			}
		}
		//		else
		//			shtPtr+= (2*shtPtr[1])+2;
	}
}

void bFile::writeFile(tukk fileName)
{
	FILE *f = fopen(fileName, "wb");
	fwrite(mFileBuffer, 1, mFileLen, f);
	fclose(f);
}

void bFile::preSwap()
{
	//const bool brokenDNA = (mFlags&FD_BROKEN_DNA)!=0;
	//FD_ENDIAN_SWAP
	//byte 8 determines the endianness of the file, little (v) versus big (V)
	i32 littleEndian = 1;
	littleEndian = ((char *)&littleEndian)[0];

	if (mFileBuffer[8] == 'V')
	{
		mFileBuffer[8] = 'v';
	}
	else
	{
		mFileBuffer[8] = 'V';
	}

	mDataStart = 12;

	char *dataPtr = mFileBuffer + mDataStart;

	bChunkInd dataChunk;
	dataChunk.code = 0;
	bool ignoreEndianFlag = true;

	//we always want to swap here

	i32 seek = getNextBlock(&dataChunk, dataPtr, mFlags);
	//dataPtr += ChunkUtils::getOffset(mFlags);
	char *dataPtrHead = 0;

	while (1)
	{
		// one behind
		if (dataChunk.code == SDNA || dataChunk.code == DNA1 || dataChunk.code == TYPE || dataChunk.code == TLEN || dataChunk.code == STRC)
		{
			swapDNA(dataPtr);
			break;
		}
		else
		{
			//if (dataChunk.code == DNA1) break;
			dataPtrHead = dataPtr + ChunkUtils::getOffset(mFlags);

			swapLen(dataPtr);
			if (dataChunk.dna_nr >= 0)
			{
				swap(dataPtrHead, dataChunk, ignoreEndianFlag);
			}
			else
			{
				//printf("unknown chunk\n");
			}
		}

		// next please!
		dataPtr += seek;

		seek = getNextBlock(&dataChunk, dataPtr, mFlags);
		if (seek < 0)
			break;
	}

	if (mFlags & FD_ENDIAN_SWAP)
	{
		mFlags &= ~FD_ENDIAN_SWAP;
	}
	else
	{
		mFlags |= FD_ENDIAN_SWAP;
	}
}

// ----------------------------------------------------- //
char *bFile::readStruct(char *head, bChunkInd &dataChunk)
{
	bool ignoreEndianFlag = false;

	if (mFlags & FD_ENDIAN_SWAP)
		swap(head, dataChunk, ignoreEndianFlag);

	if (!mFileDNA->flagEqual(dataChunk.dna_nr))
	{
		// Ouch! need to rebuild the struct
		short *oldStruct, *curStruct;
		char *oldType, *newType;
		i32 oldLen, curLen, reverseOld;

		oldStruct = mFileDNA->getStruct(dataChunk.dna_nr);
		oldType = mFileDNA->getType(oldStruct[0]);

		oldLen = mFileDNA->getLength(oldStruct[0]);

		if ((mFlags & FD_BROKEN_DNA) != 0)
		{
			if ((strcmp(oldType, "QuantizedBvhNodeData") == 0) && oldLen == 20)
			{
				return 0;
			}
			if ((strcmp(oldType, "ShortIntIndexData") == 0))
			{
				i32 allocLen = 2;
				char *dataAlloc = new char[(dataChunk.nr * allocLen) + sizeof(uk )];
				memset(dataAlloc, 0, (dataChunk.nr * allocLen) + sizeof(uk ));
				short *dest = (short *)dataAlloc;
				const short *src = (short *)head;
				for (i32 i = 0; i < dataChunk.nr; i++)
				{
					dest[i] = src[i];
					if (mFlags & FD_ENDIAN_SWAP)
					{
						SWITCH_SHORT(dest[i]);
					}
				}
				addDataBlock(dataAlloc);
				return dataAlloc;
			}
		}

		///don't try to convert Link block data, just memcpy it. Other data can be converted.
		if (strcmp("Link", oldType) != 0)
		{
			reverseOld = mMemoryDNA->getReverseType(oldType);

			if ((reverseOld != -1))
			{
				// make sure it's here
				//assert(reverseOld!= -1 && "getReverseType() returned -1, struct required!");

				//
				curStruct = mMemoryDNA->getStruct(reverseOld);
				newType = mMemoryDNA->getType(curStruct[0]);
				curLen = mMemoryDNA->getLength(curStruct[0]);

				// make sure it's the same
				assert((strcmp(oldType, newType) == 0) && "internal error, struct mismatch!");

				numallocs++;
				// numBlocks * length

				i32 allocLen = (curLen);
				char *dataAlloc = new char[(dataChunk.nr * allocLen) + sizeof(uk )];
				memset(dataAlloc, 0, (dataChunk.nr * allocLen) + sizeof(uk ));

				// track allocated
				addDataBlock(dataAlloc);

				char *cur = dataAlloc;
				char *old = head;
				for (i32 block = 0; block < dataChunk.nr; block++)
				{
					bool fixupPointers = true;
					parseStruct(cur, old, dataChunk.dna_nr, reverseOld, fixupPointers);
					mLibPointers.insert(old, (bStructHandle *)cur);

					cur += curLen;
					old += oldLen;
				}
				return dataAlloc;
			}
		}
		else
		{
			//printf("Link found\n");
		}
	}
	else
	{
//#define DEBUG_EQUAL_STRUCTS
#ifdef DEBUG_EQUAL_STRUCTS
		short *oldStruct;
		char *oldType;
		oldStruct = mFileDNA->getStruct(dataChunk.dna_nr);
		oldType = mFileDNA->getType(oldStruct[0]);
		printf("%s equal structure, just memcpy\n", oldType);
#endif  //
	}

	char *dataAlloc = new char[(dataChunk.len) + sizeof(uk )];
	memset(dataAlloc, 0, dataChunk.len + sizeof(uk ));

	// track allocated
	addDataBlock(dataAlloc);

	memcpy(dataAlloc, head, dataChunk.len);
	return dataAlloc;
}

// ----------------------------------------------------- //
void bFile::parseStruct(char *strcPtr, char *dtPtr, i32 old_dna, i32 new_dna, bool fixupPointers)
{
	if (old_dna == -1) return;
	if (new_dna == -1) return;

	//disable this, because we need to fixup pointers/ListBase
	if (/* DISABLES CODE */ (0))  //mFileDNA->flagEqual(old_dna))
	{
		short *strc = mFileDNA->getStruct(old_dna);
		i32 len = mFileDNA->getLength(strc[0]);

		memcpy(strcPtr, dtPtr, len);
		return;
	}

	// Ok, now build the struct
	char *memType, *memName, *cpc, *cpo;
	short *fileStruct, *filePtrOld, *memoryStruct, *firstStruct;
	i32 elementLength, size, revType, old_nr, new_nr, fpLen;
	short firstStructType;

	// File to memory lookup
	memoryStruct = mMemoryDNA->getStruct(new_dna);
	fileStruct = mFileDNA->getStruct(old_dna);
	firstStruct = fileStruct;

	filePtrOld = fileStruct;
	firstStructType = mMemoryDNA->getStruct(0)[0];

	// Get number of elements
	elementLength = memoryStruct[1];
	memoryStruct += 2;

	cpc = strcPtr;
	cpo = 0;
	for (i32 ele = 0; ele < elementLength; ele++, memoryStruct += 2)
	{
		memType = mMemoryDNA->getType(memoryStruct[0]);
		memName = mMemoryDNA->getName(memoryStruct[1]);

		size = mMemoryDNA->getElementSize(memoryStruct[0], memoryStruct[1]);
		revType = mMemoryDNA->getReverseType(memoryStruct[0]);

		if (revType != -1 && memoryStruct[0] >= firstStructType && memName[0] != '*')
		{
			cpo = getFileElement(firstStruct, memName, memType, dtPtr, &filePtrOld);
			if (cpo)
			{
				i32 arrayLen = mFileDNA->getArraySizeNew(filePtrOld[1]);
				old_nr = mFileDNA->getReverseType(memType);
				new_nr = revType;
				fpLen = mFileDNA->getElementSize(filePtrOld[0], filePtrOld[1]);
				if (arrayLen == 1)
				{
					parseStruct(cpc, cpo, old_nr, new_nr, fixupPointers);
				}
				else
				{
					char *tmpCpc = cpc;
					char *tmpCpo = cpo;

					for (i32 i = 0; i < arrayLen; i++)
					{
						parseStruct(tmpCpc, tmpCpo, old_nr, new_nr, fixupPointers);
						tmpCpc += size / arrayLen;
						tmpCpo += fpLen / arrayLen;
					}
				}
				cpc += size;
				cpo += fpLen;
			}
			else
				cpc += size;
		}
		else
		{
			getMatchingFileDNA(fileStruct, memName, memType, cpc, dtPtr, fixupPointers);
			cpc += size;
		}
	}
}

// ----------------------------------------------------- //
static void getElement(i32 arrayLen, tukk cur, tukk old, char *oldPtr, char *curData)
{
#define getEle(value, current, type, cast, size, ptr) \
	if (strcmp(current, type) == 0)                   \
	{                                                 \
		value = (*(cast *)ptr);                       \
		ptr += size;                                  \
	}

#define setEle(value, current, type, cast, size, ptr) \
	if (strcmp(current, type) == 0)                   \
	{                                                 \
		(*(cast *)ptr) = (cast)value;                 \
		ptr += size;                                  \
	}
	double value = 0.0;

	for (i32 i = 0; i < arrayLen; i++)
	{
		getEle(value, old, "char", char, sizeof(char), oldPtr);
		setEle(value, cur, "char", char, sizeof(char), curData);
		getEle(value, old, "short", short, sizeof(short), oldPtr);
		setEle(value, cur, "short", short, sizeof(short), curData);
		getEle(value, old, "ushort", unsigned short, sizeof(unsigned short), oldPtr);
		setEle(value, cur, "ushort", unsigned short, sizeof(unsigned short), curData);
		getEle(value, old, "i32", i32, sizeof(i32), oldPtr);
		setEle(value, cur, "i32", i32, sizeof(i32), curData);
		getEle(value, old, "long", i32, sizeof(i32), oldPtr);
		setEle(value, cur, "long", i32, sizeof(i32), curData);
		getEle(value, old, "float", float, sizeof(float), oldPtr);
		setEle(value, cur, "float", float, sizeof(float), curData);
		getEle(value, old, "double", double, sizeof(double), oldPtr);
		setEle(value, cur, "double", double, sizeof(double), curData);
	}
}

// ----------------------------------------------------- //
void bFile::swapData(char *data, short type, i32 arraySize, bool ignoreEndianFlag)
{
	if (ignoreEndianFlag || (mFlags & FD_ENDIAN_SWAP))
	{
		if (type == 2 || type == 3)
		{
			short *sp = (short *)data;
			for (i32 i = 0; i < arraySize; i++)
			{
				sp[0] = ChunkUtils::swapShort(sp[0]);
				sp++;
			}
		}
		if (type > 3 && type < 8)
		{
			char c;
			char *cp = data;
			for (i32 i = 0; i < arraySize; i++)
			{
				c = cp[0];
				cp[0] = cp[3];
				cp[3] = c;
				c = cp[1];
				cp[1] = cp[2];
				cp[2] = c;
				cp += 4;
			}
		}
	}
}

void bFile::safeSwapPtr(char *dst, tukk src)
{
	i32 ptrFile = mFileDNA->getPointerSize();
	i32 ptrMem = mMemoryDNA->getPointerSize();

	if (!src && !dst)
		return;

	if (ptrFile == ptrMem)
	{
		memcpy(dst, src, ptrMem);
	}
	else if (ptrMem == 4 && ptrFile == 8)
	{
		PointerUid *oldPtr = (PointerUid *)src;
		PointerUid *newPtr = (PointerUid *)dst;

		if (oldPtr->m_uniqueIds[0] == oldPtr->m_uniqueIds[1])
		{
			//drx3D stores the 32bit unique ID in both upper and lower part of 64bit pointers
			//so it can be used to distinguish between .blend and .bullet
			newPtr->m_uniqueIds[0] = oldPtr->m_uniqueIds[0];
		}
		else
		{
			//deal with pointers the Blender .blend style way, see
			//readfile.c in the Blender source tree
			long64 longValue = *((long64 *)src);
			//endian swap for 64bit pointer otherwise truncation will fail due to trailing zeros
			if (mFlags & FD_ENDIAN_SWAP)
				SWITCH_LONGINT(longValue);
			*((i32 *)dst) = (i32)(longValue >> 3);
		}
	}
	else if (ptrMem == 8 && ptrFile == 4)
	{
		PointerUid *oldPtr = (PointerUid *)src;
		PointerUid *newPtr = (PointerUid *)dst;
		if (oldPtr->m_uniqueIds[0] == oldPtr->m_uniqueIds[1])
		{
			newPtr->m_uniqueIds[0] = oldPtr->m_uniqueIds[0];
			newPtr->m_uniqueIds[1] = 0;
		}
		else
		{
			*((long64 *)dst) = *((i32 *)src);
		}
	}
	else
	{
		printf("%d %d\n", ptrFile, ptrMem);
		assert(0 && "Invalid pointer len");
	}
}

// ----------------------------------------------------- //
void bFile::getMatchingFileDNA(short *dna_addr, tukk lookupName, tukk lookupType, char *strcData, char *data, bool fixupPointers)
{
	// find the matching memory dna data
	// to the file being loaded. Fill the
	// memory with the file data...

	i32 len = dna_addr[1];
	dna_addr += 2;

	for (i32 i = 0; i < len; i++, dna_addr += 2)
	{
		tukk type = mFileDNA->getType(dna_addr[0]);
		tukk name = mFileDNA->getName(dna_addr[1]);

		i32 eleLen = mFileDNA->getElementSize(dna_addr[0], dna_addr[1]);

		if ((mFlags & FD_BROKEN_DNA) != 0)
		{
			if ((strcmp(type, "short") == 0) && (strcmp(name, "i32") == 0))
			{
				eleLen = 0;
			}
		}

		if (strcmp(lookupName, name) == 0)
		{
			//i32 arrayLenold = mFileDNA->getArraySize((tuk)name.c_str());
			i32 arrayLen = mFileDNA->getArraySizeNew(dna_addr[1]);
			//assert(arrayLenold == arrayLen);

			if (name[0] == '*')
			{
				// cast pointers
				i32 ptrFile = mFileDNA->getPointerSize();
				i32 ptrMem = mMemoryDNA->getPointerSize();
				safeSwapPtr(strcData, data);

				if (fixupPointers)
				{
					if (arrayLen > 1)
					{
						//uk *sarray = (uk *)strcData;
						//uk *darray = (uk *)data;

						char *cpc, *cpo;
						cpc = (char *)strcData;
						cpo = (char *)data;

						for (i32 a = 0; a < arrayLen; a++)
						{
							safeSwapPtr(cpc, cpo);
							m_pointerFixupArray.push_back(cpc);
							cpc += ptrMem;
							cpo += ptrFile;
						}
					}
					else
					{
						if (name[1] == '*')
							m_pointerPtrFixupArray.push_back(strcData);
						else
							m_pointerFixupArray.push_back(strcData);
					}
				}
				else
				{
					//					printf("skipped %s %s : %x\n",type.c_str(),name.c_str(),strcData);
				}
			}

			else if (strcmp(type, lookupType) == 0)
				memcpy(strcData, data, eleLen);
			else
				getElement(arrayLen, lookupType, type, data, strcData);

			// --
			return;
		}
		data += eleLen;
	}
}

// ----------------------------------------------------- //
char *bFile::getFileElement(short *firstStruct, char *lookupName, char *lookupType, char *data, short **foundPos)
{
	short *old = firstStruct;  //mFileDNA->getStruct(old_nr);
	i32 elementLength = old[1];
	old += 2;

	for (i32 i = 0; i < elementLength; i++, old += 2)
	{
		char *type = mFileDNA->getType(old[0]);
		char *name = mFileDNA->getName(old[1]);
		i32 len = mFileDNA->getElementSize(old[0], old[1]);

		if (strcmp(lookupName, name) == 0)
		{
			if (strcmp(type, lookupType) == 0)
			{
				if (foundPos)
					*foundPos = old;
				return data;
			}
			return 0;
		}
		data += len;
	}
	return 0;
}

// ----------------------------------------------------- //
void bFile::swapStruct(i32 dna_nr, char *data, bool ignoreEndianFlag)
{
	if (dna_nr == -1) return;

	short *strc = mFileDNA->getStruct(dna_nr);
	//short *firstStrc = strc;

	i32 elementLen = strc[1];
	strc += 2;

	short first = mFileDNA->getStruct(0)[0];

	char *buf = data;
	for (i32 i = 0; i < elementLen; i++, strc += 2)
	{
		char *type = mFileDNA->getType(strc[0]);
		char *name = mFileDNA->getName(strc[1]);

		i32 size = mFileDNA->getElementSize(strc[0], strc[1]);
		if (strc[0] >= first && name[0] != '*')
		{
			i32 old_nr = mFileDNA->getReverseType(type);
			i32 arrayLen = mFileDNA->getArraySizeNew(strc[1]);
			if (arrayLen == 1)
			{
				swapStruct(old_nr, buf, ignoreEndianFlag);
			}
			else
			{
				char *tmpBuf = buf;
				for (i32 i = 0; i < arrayLen; i++)
				{
					swapStruct(old_nr, tmpBuf, ignoreEndianFlag);
					tmpBuf += size / arrayLen;
				}
			}
		}
		else
		{
			//i32 arrayLenOld = mFileDNA->getArraySize(name);
			i32 arrayLen = mFileDNA->getArraySizeNew(strc[1]);
			//assert(arrayLenOld == arrayLen);
			swapData(buf, strc[0], arrayLen, ignoreEndianFlag);
		}
		buf += size;
	}
}

void bFile::resolvePointersMismatch()
{
	//	printf("resolvePointersStructMismatch\n");

	i32 i;

	for (i = 0; i < m_pointerFixupArray.size(); i++)
	{
		char *cur = m_pointerFixupArray.at(i);
		uk *ptrptr = (uk *)cur;
		uk ptr = *ptrptr;
		ptr = findLibPointer(ptr);
		if (ptr)
		{
			//printf("Fixup pointer!\n");
			*(ptrptr) = ptr;
		}
		else
		{
			//			printf("pointer not found: %x\n",cur);
		}
	}

	for (i = 0; i < m_pointerPtrFixupArray.size(); i++)
	{
		char *cur = m_pointerPtrFixupArray.at(i);
		uk *ptrptr = (uk *)cur;

		bChunkInd *block = m_chunkPtrPtrMap.find(*ptrptr);
		if (block)
		{
			i32 ptrMem = mMemoryDNA->getPointerSize();
			i32 ptrFile = mFileDNA->getPointerSize();

			i32 blockLen = block->len / ptrFile;

			uk onptr = findLibPointer(*ptrptr);
			if (onptr)
			{
				char *newPtr = new char[blockLen * ptrMem];
				addDataBlock(newPtr);
				memset(newPtr, 0, blockLen * ptrMem);

				uk *onarray = (uk *)onptr;
				char *oldPtr = (char *)onarray;

				i32 p = 0;
				while (blockLen-- > 0)
				{
					PointerUid dp = {{0}};
					safeSwapPtr((char *)dp.m_uniqueIds, oldPtr);

					uk *tptr = (uk *)(newPtr + p * ptrMem);
					*tptr = findLibPointer(dp.m_ptr);

					oldPtr += ptrFile;
					++p;
				}

				*ptrptr = newPtr;
			}
		}
	}
}

///this loop only works fine if the Blender DNA structure of the file matches the headerfiles
void bFile::resolvePointersChunk(const bChunkInd &dataChunk, i32 verboseMode)
{
	bParse::bDNA *fileDna = mFileDNA ? mFileDNA : mMemoryDNA;

	i16 *oldStruct = fileDna->getStruct(dataChunk.dna_nr);
	short oldLen = fileDna->getLength(oldStruct[0]);
	//tuk structType = fileDna->getType(oldStruct[0]);

	char *cur = (char *)findLibPointer(dataChunk.oldPtr);
	for (i32 block = 0; block < dataChunk.nr; block++)
	{
		resolvePointersStructRecursive(cur, dataChunk.dna_nr, verboseMode, 1);
		cur += oldLen;
	}
}

i32 bFile::resolvePointersStructRecursive(char *strcPtr, i32 dna_nr, i32 verboseMode, i32 recursion)
{
	bParse::bDNA *fileDna = mFileDNA ? mFileDNA : mMemoryDNA;

	char *memType;
	char *memName;
	short firstStructType = fileDna->getStruct(0)[0];

	char *elemPtr = strcPtr;

	i16 *oldStruct = fileDna->getStruct(dna_nr);

	i32 elementLength = oldStruct[1];
	oldStruct += 2;

	i32 totalSize = 0;

	for (i32 ele = 0; ele < elementLength; ele++, oldStruct += 2)
	{
		memType = fileDna->getType(oldStruct[0]);
		memName = fileDna->getName(oldStruct[1]);

		i32 arrayLen = fileDna->getArraySizeNew(oldStruct[1]);
		if (memName[0] == '*')
		{
			if (arrayLen > 1)
			{
				uk *array = (uk *)elemPtr;
				for (i32 a = 0; a < arrayLen; a++)
				{
					if (verboseMode & FD_VERBOSE_EXPORT_XML)
					{
						for (i32 i = 0; i < recursion; i++)
						{
							printf("  ");
						}
						//skip the *
						printf("<%s type=\"pointer\"> ", &memName[1]);
						printf("%p ", array[a]);
						printf("</%s>\n", &memName[1]);
					}

					array[a] = findLibPointer(array[a]);
				}
			}
			else
			{
				uk *ptrptr = (uk *)elemPtr;
				uk ptr = *ptrptr;
				if (verboseMode & FD_VERBOSE_EXPORT_XML)
				{
					for (i32 i = 0; i < recursion; i++)
					{
						printf("  ");
					}
					printf("<%s type=\"pointer\"> ", &memName[1]);
					printf("%p ", ptr);
					printf("</%s>\n", &memName[1]);
				}
				ptr = findLibPointer(ptr);

				if (ptr)
				{
					//				printf("Fixup pointer at 0x%x from 0x%x to 0x%x!\n",ptrptr,*ptrptr,ptr);
					*(ptrptr) = ptr;
					if (memName[1] == '*' && ptrptr && *ptrptr)
					{
						// This	will only work if the given	**array	is continuous
						uk *array = (uk *)*(ptrptr);
						uk np = array[0];
						i32 n = 0;
						while (np)
						{
							np = findLibPointer(array[n]);
							if (np) array[n] = np;
							n++;
						}
					}
				}
				else
				{
					//				printf("Cannot fixup pointer at 0x%x from 0x%x to 0x%x!\n",ptrptr,*ptrptr,ptr);
				}
			}
		}
		else
		{
			i32 revType = fileDna->getReverseType(oldStruct[0]);
			if (oldStruct[0] >= firstStructType)  //revType != -1 &&
			{
				char cleanName[MAX_STRLEN];
				getCleanName(memName, cleanName);

				i32 arrayLen = fileDna->getArraySizeNew(oldStruct[1]);
				i32 byteOffset = 0;

				if (verboseMode & FD_VERBOSE_EXPORT_XML)
				{
					for (i32 i = 0; i < recursion; i++)
					{
						printf("  ");
					}

					if (arrayLen > 1)
					{
						printf("<%s type=\"%s\" count=%d>\n", cleanName, memType, arrayLen);
					}
					else
					{
						printf("<%s type=\"%s\">\n", cleanName, memType);
					}
				}

				for (i32 i = 0; i < arrayLen; i++)
				{
					byteOffset += resolvePointersStructRecursive(elemPtr + byteOffset, revType, verboseMode, recursion + 1);
				}
				if (verboseMode & FD_VERBOSE_EXPORT_XML)
				{
					for (i32 i = 0; i < recursion; i++)
					{
						printf("  ");
					}
					printf("</%s>\n", cleanName);
				}
			}
			else
			{
				//export a simple type
				if (verboseMode & FD_VERBOSE_EXPORT_XML)
				{
					if (arrayLen > MAX_ARRAY_LENGTH)
					{
						printf("too long\n");
					}
					else
					{
						//printf("%s %s\n",memType,memName);

						bool isIntegerType = (strcmp(memType, "char") == 0) || (strcmp(memType, "i32") == 0) || (strcmp(memType, "short") == 0);

						if (isIntegerType)
						{
							tukk newtype = "i32";
							i32 dbarray[MAX_ARRAY_LENGTH];
							i32 *dbPtr = 0;
							char *tmp = elemPtr;
							dbPtr = &dbarray[0];
							if (dbPtr)
							{
								char cleanName[MAX_STRLEN];
								getCleanName(memName, cleanName);

								i32 i;
								getElement(arrayLen, newtype, memType, tmp, (char *)dbPtr);
								for (i = 0; i < recursion; i++)
									printf("  ");
								if (arrayLen == 1)
									printf("<%s type=\"%s\">", cleanName, memType);
								else
									printf("<%s type=\"%s\" count=%d>", cleanName, memType, arrayLen);
								for (i = 0; i < arrayLen; i++)
									printf(" %d ", dbPtr[i]);
								printf("</%s>\n", cleanName);
							}
						}
						else
						{
							tukk newtype = "double";
							double dbarray[MAX_ARRAY_LENGTH];
							double *dbPtr = 0;
							char *tmp = elemPtr;
							dbPtr = &dbarray[0];
							if (dbPtr)
							{
								i32 i;
								getElement(arrayLen, newtype, memType, tmp, (char *)dbPtr);
								for (i = 0; i < recursion; i++)
									printf("  ");
								char cleanName[MAX_STRLEN];
								getCleanName(memName, cleanName);

								if (arrayLen == 1)
								{
									printf("<%s type=\"%s\">", memName, memType);
								}
								else
								{
									printf("<%s type=\"%s\" count=%d>", cleanName, memType, arrayLen);
								}
								for (i = 0; i < arrayLen; i++)
									printf(" %f ", dbPtr[i]);
								printf("</%s>\n", cleanName);
							}
						}
					}
				}
			}
		}

		i32 size = fileDna->getElementSize(oldStruct[0], oldStruct[1]);
		totalSize += size;
		elemPtr += size;
	}

	return totalSize;
}

///Resolve pointers replaces the original pointers in structures, and linked lists by the new in-memory structures
void bFile::resolvePointers(i32 verboseMode)
{
	bParse::bDNA *fileDna = mFileDNA ? mFileDNA : mMemoryDNA;

	//char *dataPtr = mFileBuffer+mDataStart;

	if (1)  //mFlags & (FD_BITS_VARIES | FD_VERSION_VARIES))
	{
		resolvePointersMismatch();
	}

	{
		if (verboseMode & FD_VERBOSE_EXPORT_XML)
		{
			printf("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
			i32 numitems = m_chunks.size();
			printf("<bullet_physics version=%d itemcount = %d>\n", GetVersion(), numitems);
		}
		for (i32 i = 0; i < m_chunks.size(); i++)
		{
			const bChunkInd &dataChunk = m_chunks.at(i);

			if (!mFileDNA || fileDna->flagEqual(dataChunk.dna_nr))
			{
				//dataChunk.len
				i16 *oldStruct = fileDna->getStruct(dataChunk.dna_nr);
				char *oldType = fileDna->getType(oldStruct[0]);

				if (verboseMode & FD_VERBOSE_EXPORT_XML)
					printf(" <%s pointer=%p>\n", oldType, dataChunk.oldPtr);

				resolvePointersChunk(dataChunk, verboseMode);

				if (verboseMode & FD_VERBOSE_EXPORT_XML)
					printf(" </%s>\n", oldType);
			}
			else
			{
				//printf("skipping mStruct\n");
			}
		}
		if (verboseMode & FD_VERBOSE_EXPORT_XML)
		{
			printf("</bullet_physics>\n");
		}
	}
}

// ----------------------------------------------------- //
uk bFile::findLibPointer(uk ptr)
{
	bStructHandle **ptrptr = getLibPointers().find(ptr);
	if (ptrptr)
		return *ptrptr;
	return 0;
}

void bFile::updateOldPointers()
{
	i32 i;

	for (i = 0; i < m_chunks.size(); i++)
	{
		bChunkInd &dataChunk = m_chunks[i];
		dataChunk.oldPtr = findLibPointer(dataChunk.oldPtr);
	}
}
void bFile::dumpChunks(bParse::bDNA *dna)
{
	i32 i;

	for (i = 0; i < m_chunks.size(); i++)
	{
		bChunkInd &dataChunk = m_chunks[i];
		char *codeptr = (char *)&dataChunk.code;
		char codestr[5] = {codeptr[0], codeptr[1], codeptr[2], codeptr[3], 0};

		short *newStruct = dna->getStruct(dataChunk.dna_nr);
		char *typeName = dna->getType(newStruct[0]);
		printf("%3d: %s  ", i, typeName);

		printf("code=%s  ", codestr);

		printf("ptr=%p  ", dataChunk.oldPtr);
		printf("len=%d  ", dataChunk.len);
		printf("nr=%d  ", dataChunk.nr);
		if (dataChunk.nr != 1)
		{
			printf("not 1\n");
		}
		printf("\n");
	}

#if 0
	IDFinderData ifd;
	ifd.success = 0;
	ifd.IDname = NULL;
	ifd.just_print_it = 1;
	for (i=0; i<bf->m_blocks.size(); ++i) 
	{
		BlendBlock* bb = bf->m_blocks[i];
		printf("tag='%s'\tptr=%p\ttype=%s\t[%4d]",		bb->tag, bb,bf->types[bb->type_index].name,bb->m_array_entries_.size());
		block_ID_finder(bb, bf, &ifd);
		printf("\n");
	}
#endif
}

void bFile::writeChunks(FILE *fp, bool fixupPointers)
{
	bParse::bDNA *fileDna = mFileDNA ? mFileDNA : mMemoryDNA;

	for (i32 i = 0; i < m_chunks.size(); i++)
	{
		bChunkInd &dataChunk = m_chunks.at(i);

		// Ouch! need to rebuild the struct
		short *oldStruct, *curStruct;
		char *oldType, *newType;
		i32 curLen, reverseOld;

		oldStruct = fileDna->getStruct(dataChunk.dna_nr);
		oldType = fileDna->getType(oldStruct[0]);
		//i32 oldLen = fileDna->getLength(oldStruct[0]);
		///don't try to convert Link block data, just memcpy it. Other data can be converted.
		reverseOld = mMemoryDNA->getReverseType(oldType);

		if ((reverseOld != -1))
		{
			// make sure it's here
			//assert(reverseOld!= -1 && "getReverseType() returned -1, struct required!");
			//
			curStruct = mMemoryDNA->getStruct(reverseOld);
			newType = mMemoryDNA->getType(curStruct[0]);
			// make sure it's the same
			assert((strcmp(oldType, newType) == 0) && "internal error, struct mismatch!");

			curLen = mMemoryDNA->getLength(curStruct[0]);
			dataChunk.dna_nr = reverseOld;
			if (strcmp("Link", oldType) != 0)
			{
				dataChunk.len = curLen * dataChunk.nr;
			}
			else
			{
				//				printf("keep length of link = %d\n",dataChunk.len);
			}

			//write the structure header
			fwrite(&dataChunk, sizeof(bChunkInd), 1, fp);

			i16 *curStruct1;
			curStruct1 = mMemoryDNA->getStruct(dataChunk.dna_nr);
			assert(curStruct1 == curStruct);

			char *cur = fixupPointers ? (char *)findLibPointer(dataChunk.oldPtr) : (char *)dataChunk.oldPtr;

			//write the actual contents of the structure(s)
			fwrite(cur, dataChunk.len, 1, fp);
		}
		else
		{
			printf("serious error, struct mismatch: don't write\n");
		}
	}
}

// ----------------------------------------------------- //
i32 bFile::getNextBlock(bChunkInd *dataChunk, tukk dataPtr, i32k flags)
{
	bool swap = false;
	bool varies = false;

	if (flags & FD_ENDIAN_SWAP)
		swap = true;
	if (flags & FD_BITS_VARIES)
		varies = true;

	if (VOID_IS_8)
	{
		if (varies)
		{
			bChunkPtr4 head;
			memcpy(&head, dataPtr, sizeof(bChunkPtr4));

			bChunkPtr8 chunk;

			chunk.code = head.code;
			chunk.len = head.len;
			chunk.m_uniqueInts[0] = head.m_uniqueInt;
			chunk.m_uniqueInts[1] = 0;
			chunk.dna_nr = head.dna_nr;
			chunk.nr = head.nr;

			if (swap)
			{
				if ((chunk.code & 0xFFFF) == 0)
					chunk.code >>= 16;

				SWITCH_INT(chunk.len);
				SWITCH_INT(chunk.dna_nr);
				SWITCH_INT(chunk.nr);
			}

			memcpy(dataChunk, &chunk, sizeof(bChunkInd));
		}
		else
		{
			bChunkPtr8 c;
			memcpy(&c, dataPtr, sizeof(bChunkPtr8));

			if (swap)
			{
				if ((c.code & 0xFFFF) == 0)
					c.code >>= 16;

				SWITCH_INT(c.len);
				SWITCH_INT(c.dna_nr);
				SWITCH_INT(c.nr);
			}

			memcpy(dataChunk, &c, sizeof(bChunkInd));
		}
	}
	else
	{
		if (varies)
		{
			bChunkPtr8 head;
			memcpy(&head, dataPtr, sizeof(bChunkPtr8));

			bChunkPtr4 chunk;
			chunk.code = head.code;
			chunk.len = head.len;

			if (head.m_uniqueInts[0] == head.m_uniqueInts[1])
			{
				chunk.m_uniqueInt = head.m_uniqueInts[0];
			}
			else
			{
				long64 oldPtr = 0;
				memcpy(&oldPtr, &head.m_uniqueInts[0], 8);
				if (swap)
					SWITCH_LONGINT(oldPtr);
				chunk.m_uniqueInt = (i32)(oldPtr >> 3);
			}

			chunk.dna_nr = head.dna_nr;
			chunk.nr = head.nr;

			if (swap)
			{
				if ((chunk.code & 0xFFFF) == 0)
					chunk.code >>= 16;

				SWITCH_INT(chunk.len);
				SWITCH_INT(chunk.dna_nr);
				SWITCH_INT(chunk.nr);
			}

			memcpy(dataChunk, &chunk, sizeof(bChunkInd));
		}
		else
		{
			bChunkPtr4 c;
			memcpy(&c, dataPtr, sizeof(bChunkPtr4));

			if (swap)
			{
				if ((c.code & 0xFFFF) == 0)
					c.code >>= 16;

				SWITCH_INT(c.len);
				SWITCH_INT(c.dna_nr);
				SWITCH_INT(c.nr);
			}
			memcpy(dataChunk, &c, sizeof(bChunkInd));
		}
	}

	if (dataChunk->len < 0)
		return -1;

#if 0
	print ("----------");
	print (dataChunk->code);
	print (dataChunk->len);
	print (dataChunk->old);
	print (dataChunk->dna_nr);
	print (dataChunk->nr);
#endif
	return (dataChunk->len + ChunkUtils::getOffset(flags));
}

//eof
