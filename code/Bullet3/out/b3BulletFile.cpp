#include <drx3D/Serialize/Bullet2FileLoader/b3BulletFile.h>
#include <drx3D/Serialize/Bullet2FileLoader/b3Defines.h>
#include <drx3D/Serialize/Bullet2FileLoader/b3DNA.h>

#if !defined(__CELLOS_LV2__) && !defined(__MWERKS__)
#include <memory.h>
#endif
#include <string.h>

// 32 && 64 bit versions
#ifdef D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
#ifdef _WIN64
extern char b3s_bulletDNAstr64[];
extern i32 b3s_bulletDNAlen64;
#else
extern char b3s_bulletDNAstr[];
extern i32 b3s_bulletDNAlen;
#endif  //_WIN64
#else   //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES

extern char b3s_bulletDNAstr64[];
extern i32 b3s_bulletDNAlen64;
extern char b3s_bulletDNAstr[];
extern i32 b3s_bulletDNAlen;

#endif  //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES

using namespace bParse;

b3BulletFile::b3BulletFile()
	: bFile("", "BULLET ")
{
	mMemoryDNA = new bDNA();  //this memory gets released in the bFile::~bFile destructor,@todo not consistent with the rule 'who allocates it, has to deallocate it"

	m_DnaCopy = 0;

#ifdef D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
#ifdef _WIN64
	m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen64, 16);
	memcpy(m_DnaCopy, b3s_bulletDNAstr64, b3s_bulletDNAlen64);
	mMemoryDNA->init(m_DnaCopy, b3s_bulletDNAlen64);
#else   //_WIN64
	m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen, 16);
	memcpy(m_DnaCopy, b3s_bulletDNAstr, b3s_bulletDNAlen);
	mMemoryDNA->init(m_DnaCopy, b3s_bulletDNAlen);
#endif  //_WIN64
#else   //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	if (VOID_IS_8)
	{
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen64, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr64, b3s_bulletDNAlen64);
		mMemoryDNA->init(m_DnaCopy, b3s_bulletDNAlen64);
	}
	else
	{
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr, b3s_bulletDNAlen);
		mMemoryDNA->init(m_DnaCopy, b3s_bulletDNAlen);
	}
#endif  //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
}

b3BulletFile::b3BulletFile(tukk fileName)
	: bFile(fileName, "BULLET ")
{
	m_DnaCopy = 0;
}

b3BulletFile::b3BulletFile(tuk memoryBuffer, i32 len)
	: bFile(memoryBuffer, len, "BULLET ")
{
	m_DnaCopy = 0;
}

b3BulletFile::~b3BulletFile()
{
	if (m_DnaCopy)
		b3AlignedFree(m_DnaCopy);

	while (m_dataBlocks.size())
	{
		tuk dataBlock = m_dataBlocks[m_dataBlocks.size() - 1];
		delete[] dataBlock;
		m_dataBlocks.pop_back();
	}
}

// ----------------------------------------------------- //
void b3BulletFile::parseData()
{
	//	printf ("Building datablocks");
	//	printf ("Chunk size = %d",CHUNK_HEADER_LEN);
	//	printf ("File chunk size = %d",ChunkUtils::getOffset(mFlags));

	const bool brokenDNA = (mFlags & FD_BROKEN_DNA) != 0;

	//const bool swap = (mFlags&FD_ENDIAN_SWAP)!=0;

	mDataStart = 12;

	tuk dataPtr = mFileBuffer + mDataStart;

	bChunkInd dataChunk;
	dataChunk.code = 0;

	//dataPtr += ChunkUtils::getNextBlock(&dataChunk, dataPtr, mFlags);
	i32 seek = getNextBlock(&dataChunk, dataPtr, mFlags);

	if (mFlags & FD_ENDIAN_SWAP)
		swapLen(dataPtr);

	//dataPtr += ChunkUtils::getOffset(mFlags);
	tuk dataPtrHead = 0;

	while (dataChunk.code != D3_DNA1)
	{
		if (!brokenDNA || (dataChunk.code != D3_QUANTIZED_BVH_CODE))
		{
			// one behind
			if (dataChunk.code == D3_SDNA) break;
			//if (dataChunk.code == DNA1) break;

			// same as (BHEAD+DATA dependency)
			dataPtrHead = dataPtr + ChunkUtils::getOffset(mFlags);
			if (dataChunk.dna_nr >= 0)
			{
				tuk id = readStruct(dataPtrHead, dataChunk);

				// lookup maps
				if (id)
				{
					m_chunkPtrPtrMap.insert(dataChunk.oldPtr, dataChunk);
					mLibPointers.insert(dataChunk.oldPtr, (bStructHandle*)id);

					m_chunks.push_back(dataChunk);
					// block it
					//bListBasePtr *listID = mMain->getListBasePtr(dataChunk.code);
					//if (listID)
					//	listID->push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_SOFTBODY_CODE)
				{
					m_softBodies.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_RIGIDBODY_CODE)
				{
					m_rigidBodies.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_DYNAMICSWORLD_CODE)
				{
					m_dynamicsWorldInfo.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_CONSTRAINT_CODE)
				{
					m_constraints.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_QUANTIZED_BVH_CODE)
				{
					m_bvhs.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_TRIANLGE_INFO_MAP)
				{
					m_triangleInfoMaps.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_COLLISIONOBJECT_CODE)
				{
					m_collisionObjects.push_back((bStructHandle*)id);
				}

				if (dataChunk.code == D3_SHAPE_CODE)
				{
					m_collisionShapes.push_back((bStructHandle*)id);
				}

				//		if (dataChunk.code == GLOB)
				//		{
				//			m_glob = (bStructHandle*) id;
				//		}
			}
			else
			{
				//printf("unknown chunk\n");

				mLibPointers.insert(dataChunk.oldPtr, (bStructHandle*)dataPtrHead);
			}
		}
		else
		{
			printf("skipping D3_QUANTIZED_BVH_CODE due to broken DNA\n");
		}

		dataPtr += seek;

		seek = getNextBlock(&dataChunk, dataPtr, mFlags);
		if (mFlags & FD_ENDIAN_SWAP)
			swapLen(dataPtr);

		if (seek < 0)
			break;
	}
}

void b3BulletFile::addDataBlock(tuk dataBlock)
{
	m_dataBlocks.push_back(dataBlock);
}

void b3BulletFile::writeDNA(FILE* fp)
{
	bChunkInd dataChunk;
	dataChunk.code = D3_DNA1;
	dataChunk.dna_nr = 0;
	dataChunk.nr = 1;
#ifdef D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	if (VOID_IS_8)
	{
#ifdef _WIN64
		dataChunk.len = b3s_bulletDNAlen64;
		dataChunk.oldPtr = b3s_bulletDNAstr64;
		fwrite(&dataChunk, sizeof(bChunkInd), 1, fp);
		fwrite(b3s_bulletDNAstr64, b3s_bulletDNAlen64, 1, fp);
#else
		drx3DAssert(0);
#endif
	}
	else
	{
#ifndef _WIN64
		dataChunk.len = b3s_bulletDNAlen;
		dataChunk.oldPtr = b3s_bulletDNAstr;
		fwrite(&dataChunk, sizeof(bChunkInd), 1, fp);
		fwrite(b3s_bulletDNAstr, b3s_bulletDNAlen, 1, fp);
#else   //_WIN64
		drx3DAssert(0);
#endif  //_WIN64
	}
#else   //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	if (VOID_IS_8)
	{
		dataChunk.len = b3s_bulletDNAlen64;
		dataChunk.oldPtr = b3s_bulletDNAstr64;
		fwrite(&dataChunk, sizeof(bChunkInd), 1, fp);
		fwrite(b3s_bulletDNAstr64, b3s_bulletDNAlen64, 1, fp);
	}
	else
	{
		dataChunk.len = b3s_bulletDNAlen;
		dataChunk.oldPtr = b3s_bulletDNAstr;
		fwrite(&dataChunk, sizeof(bChunkInd), 1, fp);
		fwrite(b3s_bulletDNAstr, b3s_bulletDNAlen, 1, fp);
	}
#endif  //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
}

void b3BulletFile::parse(i32 verboseMode)
{
#ifdef D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	if (VOID_IS_8)
	{
#ifdef _WIN64

		if (m_DnaCopy)
			delete m_DnaCopy;
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen64, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr64, b3s_bulletDNAlen64);
		parseInternal(verboseMode, (tuk)b3s_bulletDNAstr64, b3s_bulletDNAlen64);
#else
		drx3DAssert(0);
#endif
	}
	else
	{
#ifndef _WIN64

		if (m_DnaCopy)
			delete m_DnaCopy;
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr, b3s_bulletDNAlen);
		parseInternal(verboseMode, m_DnaCopy, b3s_bulletDNAlen);
#else
		drx3DAssert(0);
#endif
	}
#else   //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	if (VOID_IS_8)
	{
		if (m_DnaCopy)
			delete m_DnaCopy;
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen64, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr64, b3s_bulletDNAlen64);
		parseInternal(verboseMode, m_DnaCopy, b3s_bulletDNAlen64);
	}
	else
	{
		if (m_DnaCopy)
			delete m_DnaCopy;
		m_DnaCopy = (tuk)b3AlignedAlloc(b3s_bulletDNAlen, 16);
		memcpy(m_DnaCopy, b3s_bulletDNAstr, b3s_bulletDNAlen);
		parseInternal(verboseMode, m_DnaCopy, b3s_bulletDNAlen);
	}
#endif  //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES

	//the parsing will convert to cpu endian
	mFlags &= ~FD_ENDIAN_SWAP;

	i32 littleEndian = 1;
	littleEndian = ((tuk)&littleEndian)[0];

	mFileBuffer[8] = littleEndian ? 'v' : 'V';
}

// experimental
i32 b3BulletFile::write(tukk fileName, bool fixupPointers)
{
	FILE* fp = fopen(fileName, "wb");
	if (fp)
	{
		char header[D3_SIZEOFBLENDERHEADER];
		memcpy(header, m_headerString, 7);
		i32 endian = 1;
		endian = ((tuk)&endian)[0];

		if (endian)
		{
			header[7] = '_';
		}
		else
		{
			header[7] = '-';
		}
		if (VOID_IS_8)
		{
			header[8] = 'V';
		}
		else
		{
			header[8] = 'v';
		}

		header[9] = '2';
		header[10] = '7';
		header[11] = '5';

		fwrite(header, D3_SIZEOFBLENDERHEADER, 1, fp);

		writeChunks(fp, fixupPointers);

		writeDNA(fp);

		fclose(fp);
	}
	else
	{
		printf("Ошибка: cannot open file %s for writing\n", fileName);
		return 0;
	}
	return 1;
}

void b3BulletFile::addStruct(tukk structType, uk data, i32 len, uk oldPtr, i32 code)
{
	bParse::bChunkInd dataChunk;
	dataChunk.code = code;
	dataChunk.nr = 1;
	dataChunk.len = len;
	dataChunk.dna_nr = mMemoryDNA->getReverseType(structType);
	dataChunk.oldPtr = oldPtr;

	///Perform structure size validation
	short* structInfo = mMemoryDNA->getStruct(dataChunk.dna_nr);
	i32 elemBytes;
	elemBytes = mMemoryDNA->getLength(structInfo[0]);
	//	i32 elemBytes = mMemoryDNA->getElementSize(structInfo[0],structInfo[1]);
	assert(len == elemBytes);

	mLibPointers.insert(dataChunk.oldPtr, (bStructHandle*)data);
	m_chunks.push_back(dataChunk);
}
