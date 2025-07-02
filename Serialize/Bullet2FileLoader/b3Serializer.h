#ifndef D3_SERIALIZER_H
#define D3_SERIALIZER_H

#include <drx3D/Common/b3Scalar.h>  // has definitions like D3_FORCE_INLINE
#include <drx3D/Common/b3StackAlloc.h>
#include <drx3D/Common/b3HashMap.h>

#if !defined(__CELLOS_LV2__) && !defined(__MWERKS__)
#include <memory.h>
#endif
#include <string.h>

extern char b3s_bulletDNAstr[];
extern i32 b3s_bulletDNAlen;
extern char b3s_bulletDNAstr64[];
extern i32 b3s_bulletDNAlen64;

D3_FORCE_INLINE i32 b3StrLen(tukk str)
{
	if (!str)
		return (0);
	i32 len = 0;

	while (*str != 0)
	{
		str++;
		len++;
	}

	return len;
}

class b3Chunk
{
public:
	i32 m_chunkCode;
	i32 m_length;
	uk m_oldPtr;
	i32 m_dna_nr;
	i32 m_number;
};

enum b3SerializationFlags
{
	D3_SERIALIZE_NO_BVH = 1,
	D3_SERIALIZE_NO_TRIANGLEINFOMAP = 2,
	D3_SERIALIZE_NO_DUPLICATE_ASSERT = 4
};

class b3Serializer
{
public:
	virtual ~b3Serializer() {}

	virtual u8k* getBufferPointer() const = 0;

	virtual i32 getCurrentBufferSize() const = 0;

	virtual b3Chunk* allocate(size_t size, i32 numElements) = 0;

	virtual void finalizeChunk(b3Chunk* chunk, tukk structType, i32 chunkCode, uk oldPtr) = 0;

	virtual uk findPointer(uk oldPtr) = 0;

	virtual uk getUniquePointer(uk oldPtr) = 0;

	virtual void startSerialization() = 0;

	virtual void finishSerialization() = 0;

	virtual tukk findNameForPointer(ukk ptr) const = 0;

	virtual void registerNameForPointer(ukk ptr, tukk name) = 0;

	virtual void serializeName(tukk ptr) = 0;

	virtual i32 getSerializationFlags() const = 0;

	virtual void setSerializationFlags(i32 flags) = 0;
};

#define D3_HEADER_LENGTH 12
#if defined(__sgi) || defined(__sparc) || defined(__sparc__) || defined(__PPC__) || defined(__ppc__) || defined(__BIG_ENDIAN__)
#define D3_MAKE_ID(a, b, c, d) ((i32)(a) << 24 | (i32)(b) << 16 | (c) << 8 | (d))
#else
#define D3_MAKE_ID(a, b, c, d) ((i32)(d) << 24 | (i32)(c) << 16 | (b) << 8 | (a))
#endif

#define D3_SOFTBODY_CODE D3_MAKE_ID('S', 'B', 'D', 'Y')
#define D3_COLLISIONOBJECT_CODE D3_MAKE_ID('C', 'O', 'B', 'J')
#define D3_RIGIDBODY_CODE D3_MAKE_ID('R', 'B', 'D', 'Y')
#define D3_CONSTRAINT_CODE D3_MAKE_ID('C', 'O', 'N', 'S')
#define D3_BOXSHAPE_CODE D3_MAKE_ID('B', 'O', 'X', 'S')
#define D3_QUANTIZED_BVH_CODE D3_MAKE_ID('Q', 'B', 'V', 'H')
#define D3_TRIANLGE_INFO_MAP D3_MAKE_ID('T', 'M', 'A', 'P')
#define D3_SHAPE_CODE D3_MAKE_ID('S', 'H', 'A', 'P')
#define D3_ARRAY_CODE D3_MAKE_ID('A', 'R', 'A', 'Y')
#define D3_SBMATERIAL_CODE D3_MAKE_ID('S', 'B', 'M', 'T')
#define D3_SBNODE_CODE D3_MAKE_ID('S', 'B', 'N', 'D')
#define D3_DYNAMICSWORLD_CODE D3_MAKE_ID('D', 'W', 'L', 'D')
#define D3_DNA_CODE D3_MAKE_ID('D', 'N', 'A', '1')

struct b3PointerUid
{
	union {
		uk m_ptr;
		i32 m_uniqueIds[2];
	};
};

///The b3DefaultSerializer is the main drx3D serialization class.
///The constructor takes an optional argument for backwards compatibility, it is recommended to leave this empty/zero.
class b3DefaultSerializer : public b3Serializer
{
	b3AlignedObjectArray<tuk> mTypes;
	b3AlignedObjectArray<short*> mStructs;
	b3AlignedObjectArray<short> mTlens;
	b3HashMap<b3HashInt, i32> mStructReverse;
	b3HashMap<b3HashString, i32> mTypeLookup;

	b3HashMap<b3HashPtr, uk> m_chunkP;

	b3HashMap<b3HashPtr, tukk > m_nameMap;

	b3HashMap<b3HashPtr, b3PointerUid> m_uniquePointers;
	i32 m_uniqueIdGenerator;

	i32 m_totalSize;
	u8* m_buffer;
	i32 m_currentSize;
	uk m_dna;
	i32 m_dnaLength;

	i32 m_serializationFlags;

	b3AlignedObjectArray<b3Chunk*> m_chunkPtrs;

protected:
	virtual uk findPointer(uk oldPtr)
	{
		uk * ptr = m_chunkP.find(oldPtr);
		if (ptr && *ptr)
			return *ptr;
		return 0;
	}

	void writeDNA()
	{
		b3Chunk* dnaChunk = allocate(m_dnaLength, 1);
		memcpy(dnaChunk->m_oldPtr, m_dna, m_dnaLength);
		finalizeChunk(dnaChunk, "DNA1", D3_DNA_CODE, m_dna);
	}

	i32 getReverseType(tukk type) const
	{
		b3HashString key(type);
		i32k* valuePtr = mTypeLookup.find(key);
		if (valuePtr)
			return *valuePtr;

		return -1;
	}

	void initDNA(tukk bdnaOrg, i32 dnalen)
	{
		///was already initialized
		if (m_dna)
			return;

		i32 littleEndian = 1;
		littleEndian = ((tuk)&littleEndian)[0];

		m_dna = b3AlignedAlloc(dnalen, 16);
		memcpy(m_dna, bdnaOrg, dnalen);
		m_dnaLength = dnalen;

		i32* intPtr = 0;
		short* shtPtr = 0;
		tuk cp = 0;
		i32 dataLen = 0;
		intPtr = (i32*)m_dna;

		/*
				SDNA (4 bytes) (magic number)
				NAME (4 bytes)
				<nr> (4 bytes) amount of names (i32)
				<string>
				<string>
			*/

		if (strncmp((tukk)m_dna, "SDNA", 4) == 0)
		{
			// skip ++ NAME
			intPtr++;
			intPtr++;
		}

		// Parse names
		if (!littleEndian)
			*intPtr = b3SwapEndian(*intPtr);

		dataLen = *intPtr;

		intPtr++;

		cp = (tuk)intPtr;
		i32 i;
		for (i = 0; i < dataLen; i++)
		{
			while (*cp) cp++;
			cp++;
		}
		cp = b3AlignPointer(cp, 4);

		/*
				TYPE (4 bytes)
				<nr> amount of types (i32)
				<string>
				<string>
			*/

		intPtr = (i32*)cp;
		drx3DAssert(strncmp(cp, "TYPE", 4) == 0);
		intPtr++;

		if (!littleEndian)
			*intPtr = b3SwapEndian(*intPtr);

		dataLen = *intPtr;
		intPtr++;

		cp = (tuk)intPtr;
		for (i = 0; i < dataLen; i++)
		{
			mTypes.push_back(cp);
			while (*cp) cp++;
			cp++;
		}

		cp = b3AlignPointer(cp, 4);

		/*
				TLEN (4 bytes)
				<len> (short) the lengths of types
				<len>
			*/

		// Parse type lens
		intPtr = (i32*)cp;
		drx3DAssert(strncmp(cp, "TLEN", 4) == 0);
		intPtr++;

		dataLen = (i32)mTypes.size();

		shtPtr = (short*)intPtr;
		for (i = 0; i < dataLen; i++, shtPtr++)
		{
			if (!littleEndian)
				shtPtr[0] = b3SwapEndian(shtPtr[0]);
			mTlens.push_back(shtPtr[0]);
		}

		if (dataLen & 1) shtPtr++;

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

		intPtr = (i32*)shtPtr;
		cp = (tuk)intPtr;
		drx3DAssert(strncmp(cp, "STRC", 4) == 0);
		intPtr++;

		if (!littleEndian)
			*intPtr = b3SwapEndian(*intPtr);
		dataLen = *intPtr;
		intPtr++;

		shtPtr = (short*)intPtr;
		for (i = 0; i < dataLen; i++)
		{
			mStructs.push_back(shtPtr);

			if (!littleEndian)
			{
				shtPtr[0] = b3SwapEndian(shtPtr[0]);
				shtPtr[1] = b3SwapEndian(shtPtr[1]);

				i32 len = shtPtr[1];
				shtPtr += 2;

				for (i32 a = 0; a < len; a++, shtPtr += 2)
				{
					shtPtr[0] = b3SwapEndian(shtPtr[0]);
					shtPtr[1] = b3SwapEndian(shtPtr[1]);
				}
			}
			else
			{
				shtPtr += (2 * shtPtr[1]) + 2;
			}
		}

		// build reverse lookups
		for (i = 0; i < (i32)mStructs.size(); i++)
		{
			short* strc = mStructs.at(i);
			mStructReverse.insert(strc[0], i);
			mTypeLookup.insert(b3HashString(mTypes[strc[0]]), i);
		}
	}

public:
	b3DefaultSerializer(i32 totalSize = 0)
		: m_totalSize(totalSize),
		  m_currentSize(0),
		  m_dna(0),
		  m_dnaLength(0),
		  m_serializationFlags(0)
	{
		m_buffer = m_totalSize ? (u8*)b3AlignedAlloc(totalSize, 16) : 0;

		const bool VOID_IS_8 = ((sizeof(uk ) == 8));

#ifdef D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
		if (VOID_IS_8)
		{
#if _WIN64
			initDNA((tukk)b3s_bulletDNAstr64, b3s_bulletDNAlen64);
#else
			drx3DAssert(0);
#endif
		}
		else
		{
#ifndef _WIN64
			initDNA((tukk)b3s_bulletDNAstr, b3s_bulletDNAlen);
#else
			drx3DAssert(0);
#endif
		}

#else   //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
		if (VOID_IS_8)
		{
			initDNA((tukk)b3s_bulletDNAstr64, b3s_bulletDNAlen64);
		}
		else
		{
			initDNA((tukk)b3s_bulletDNAstr, b3s_bulletDNAlen);
		}
#endif  //D3_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	}

	virtual ~b3DefaultSerializer()
	{
		if (m_buffer)
			b3AlignedFree(m_buffer);
		if (m_dna)
			b3AlignedFree(m_dna);
	}

	void writeHeader(u8* buffer) const
	{
#ifdef D3_USE_DOUBLE_PRECISION
		memcpy(buffer, "BULLETd", 7);
#else
		memcpy(buffer, "BULLETf", 7);
#endif  //D3_USE_DOUBLE_PRECISION

		i32 littleEndian = 1;
		littleEndian = ((tuk)&littleEndian)[0];

		if (sizeof(uk ) == 8)
		{
			buffer[7] = '-';
		}
		else
		{
			buffer[7] = '_';
		}

		if (littleEndian)
		{
			buffer[8] = 'v';
		}
		else
		{
			buffer[8] = 'V';
		}

		buffer[9] = '2';
		buffer[10] = '8';
		buffer[11] = '1';
	}

	virtual void startSerialization()
	{
		m_uniqueIdGenerator = 1;
		if (m_totalSize)
		{
			u8* buffer = internalAlloc(D3_HEADER_LENGTH);
			writeHeader(buffer);
		}
	}

	virtual void finishSerialization()
	{
		writeDNA();

		//if we didn't pre-allocate a buffer, we need to create a contiguous buffer now
		i32 mysize = 0;
		if (!m_totalSize)
		{
			if (m_buffer)
				b3AlignedFree(m_buffer);

			m_currentSize += D3_HEADER_LENGTH;
			m_buffer = (u8*)b3AlignedAlloc(m_currentSize, 16);

			u8* currentPtr = m_buffer;
			writeHeader(m_buffer);
			currentPtr += D3_HEADER_LENGTH;
			mysize += D3_HEADER_LENGTH;
			for (i32 i = 0; i < m_chunkPtrs.size(); i++)
			{
				i32 curLength = sizeof(b3Chunk) + m_chunkPtrs[i]->m_length;
				memcpy(currentPtr, m_chunkPtrs[i], curLength);
				b3AlignedFree(m_chunkPtrs[i]);
				currentPtr += curLength;
				mysize += curLength;
			}
		}

		mTypes.clear();
		mStructs.clear();
		mTlens.clear();
		mStructReverse.clear();
		mTypeLookup.clear();
		m_chunkP.clear();
		m_nameMap.clear();
		m_uniquePointers.clear();
		m_chunkPtrs.clear();
	}

	virtual uk getUniquePointer(uk oldPtr)
	{
		if (!oldPtr)
			return 0;

		b3PointerUid* uptr = (b3PointerUid*)m_uniquePointers.find(oldPtr);
		if (uptr)
		{
			return uptr->m_ptr;
		}
		m_uniqueIdGenerator++;

		b3PointerUid uid;
		uid.m_uniqueIds[0] = m_uniqueIdGenerator;
		uid.m_uniqueIds[1] = m_uniqueIdGenerator;
		m_uniquePointers.insert(oldPtr, uid);
		return uid.m_ptr;
	}

	virtual u8k* getBufferPointer() const
	{
		return m_buffer;
	}

	virtual i32 getCurrentBufferSize() const
	{
		return m_currentSize;
	}

	virtual void finalizeChunk(b3Chunk* chunk, tukk structType, i32 chunkCode, uk oldPtr)
	{
		if (!(m_serializationFlags & D3_SERIALIZE_NO_DUPLICATE_ASSERT))
		{
			drx3DAssert(!findPointer(oldPtr));
		}

		chunk->m_dna_nr = getReverseType(structType);

		chunk->m_chunkCode = chunkCode;

		uk uniquePtr = getUniquePointer(oldPtr);

		m_chunkP.insert(oldPtr, uniquePtr);  //chunk->m_oldPtr);
		chunk->m_oldPtr = uniquePtr;         //oldPtr;
	}

	virtual u8* internalAlloc(size_t size)
	{
		u8* ptr = 0;

		if (m_totalSize)
		{
			ptr = m_buffer + m_currentSize;
			m_currentSize += i32(size);
			drx3DAssert(m_currentSize < m_totalSize);
		}
		else
		{
			ptr = (u8*)b3AlignedAlloc(size, 16);
			m_currentSize += i32(size);
		}
		return ptr;
	}

	virtual b3Chunk* allocate(size_t size, i32 numElements)
	{
		u8* ptr = internalAlloc(i32(size) * numElements + sizeof(b3Chunk));

		u8* data = ptr + sizeof(b3Chunk);

		b3Chunk* chunk = (b3Chunk*)ptr;
		chunk->m_chunkCode = 0;
		chunk->m_oldPtr = data;
		chunk->m_length = i32(size) * numElements;
		chunk->m_number = numElements;

		m_chunkPtrs.push_back(chunk);

		return chunk;
	}

	virtual tukk findNameForPointer(ukk ptr) const
	{
		tukk const* namePtr = m_nameMap.find(ptr);
		if (namePtr && *namePtr)
			return *namePtr;
		return 0;
	}

	virtual void registerNameForPointer(ukk ptr, tukk name)
	{
		m_nameMap.insert(ptr, name);
	}

	virtual void serializeName(tukk name)
	{
		if (name)
		{
			//don't serialize name twice
			if (findPointer((uk )name))
				return;

			i32 len = b3StrLen(name);
			if (len)
			{
				i32 newLen = len + 1;
				i32 padding = ((newLen + 3) & ~3) - newLen;
				newLen += padding;

				//serialize name string now
				b3Chunk* chunk = allocate(sizeof(char), newLen);
				tuk destinationName = (tuk)chunk->m_oldPtr;
				for (i32 i = 0; i < len; i++)
				{
					destinationName[i] = name[i];
				}
				destinationName[len] = 0;
				finalizeChunk(chunk, "char", D3_ARRAY_CODE, (uk )name);
			}
		}
	}

	virtual i32 getSerializationFlags() const
	{
		return m_serializationFlags;
	}

	virtual void setSerializationFlags(i32 flags)
	{
		m_serializationFlags = flags;
	}
};

#endif  //D3_SERIALIZER_H
