#ifndef DRX3D_SERIALIZER_H
#define DRX3D_SERIALIZER_H

#include <drx3D/Maths/Linear/Scalar.h>  // has definitions like SIMD_FORCE_INLINE
#include <drx3D/Maths/Linear/HashMap.h>

#if !defined(__CELLOS_LV2__) && !defined(__MWERKS__)
#include <memory.h>
#endif
#include <string.h>

extern char sBulletDNAstr[];
extern i32 sBulletDNAlen;
extern char sBulletDNAstr64[];
extern i32 sBulletDNAlen64;

SIMD_FORCE_INLINE i32 StrLen(tukk str)
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

class Chunk
{
public:
	i32 m_chunkCode;
	i32 m_length;
	uk m_oldPtr;
	i32 m_dna_nr;
	i32 m_number;
};

enum SerializationFlags
{
	DRX3D_SERIALIZE_NO_BVH = 1,
	DRX3D_SERIALIZE_NO_TRIANGLEINFOMAP = 2,
	DRX3D_SERIALIZE_NO_DUPLICATE_ASSERT = 4,
	DRX3D_SERIALIZE_CONTACT_MANIFOLDS = 8,
};

class Serializer
{
public:
	virtual ~Serializer() {}

	virtual u8k* getBufferPointer() const = 0;

	virtual i32 getCurrentBufferSize() const = 0;

	virtual Chunk* allocate(size_t size, i32 numElements) = 0;

	virtual void finalizeChunk(Chunk* chunk, tukk structType, i32 chunkCode, uk oldPtr) = 0;

	virtual uk findPointer(uk oldPtr) = 0;

	virtual uk getUniquePointer(uk oldPtr) = 0;

	virtual void startSerialization() = 0;

	virtual void finishSerialization() = 0;

	virtual tukk findNameForPointer(ukk ptr) const = 0;

	virtual void registerNameForPointer(ukk ptr, tukk name) = 0;

	virtual void serializeName(tukk ptr) = 0;

	virtual i32 getSerializationFlags() const = 0;

	virtual void setSerializationFlags(i32 flags) = 0;

	virtual i32 getNumChunks() const = 0;

	virtual const Chunk* getChunk(i32 chunkIndex) const = 0;
};

#define DRX3D_HEADER_LENGTH 12
#if defined(__sgi) || defined(__sparc) || defined(__sparc__) || defined(__PPC__) || defined(__ppc__) || defined(__BIG_ENDIAN__)
#define DRX3D_MAKE_ID(a, b, c, d) ((i32)(a) << 24 | (i32)(b) << 16 | (c) << 8 | (d))
#else
#define DRX3D_MAKE_ID(a, b, c, d) ((i32)(d) << 24 | (i32)(c) << 16 | (b) << 8 | (a))
#endif

#define DRX3D_MULTIBODY_CODE DRX3D_MAKE_ID('M', 'B', 'D', 'Y')
#define DRX3D_MB_LINKCOLLIDER_CODE DRX3D_MAKE_ID('M', 'B', 'L', 'C')
#define DRX3D_SOFTBODY_CODE DRX3D_MAKE_ID('S', 'B', 'D', 'Y')
#define DRX3D_COLLISIONOBJECT_CODE DRX3D_MAKE_ID('C', 'O', 'B', 'J')
#define DRX3D_RIGIDBODY_CODE DRX3D_MAKE_ID('R', 'B', 'D', 'Y')
#define DRX3D_CONSTRAINT_CODE DRX3D_MAKE_ID('C', 'O', 'N', 'S')
#define DRX3D_BOXSHAPE_CODE DRX3D_MAKE_ID('B', 'O', 'X', 'S')
#define DRX3D_QUANTIZED_BVH_CODE DRX3D_MAKE_ID('Q', 'B', 'V', 'H')
#define DRX3D_TRIANLGE_INFO_MAP DRX3D_MAKE_ID('T', 'M', 'A', 'P')
#define DRX3D_SHAPE_CODE DRX3D_MAKE_ID('S', 'H', 'A', 'P')
#define DRX3D_ARRAY_CODE DRX3D_MAKE_ID('A', 'R', 'A', 'Y')
#define DRX3D_SBMATERIAL_CODE DRX3D_MAKE_ID('S', 'B', 'M', 'T')
#define DRX3D_SBNODE_CODE DRX3D_MAKE_ID('S', 'B', 'N', 'D')
#define DRX3D_DYNAMICSWORLD_CODE DRX3D_MAKE_ID('D', 'W', 'L', 'D')
#define DRX3D_CONTACTMANIFOLD_CODE DRX3D_MAKE_ID('C', 'O', 'N', 'T')
#define DRX3D_DNA_CODE DRX3D_MAKE_ID('D', 'N', 'A', '1')

struct PointerUid
{
	union {
		uk m_ptr;
		i32 m_uniqueIds[2];
	};
};

struct BulletSerializedArrays
{
	BulletSerializedArrays()
	{
	}
	AlignedObjectArray<struct QuantizedBvhDoubleData*> m_bvhsDouble;
	AlignedObjectArray<struct QuantizedBvhFloatData*> m_bvhsFloat;
	AlignedObjectArray<struct CollisionShapeData*> m_colShapeData;
	AlignedObjectArray<struct DynamicsWorldDoubleData*> m_dynamicWorldInfoDataDouble;
	AlignedObjectArray<struct DynamicsWorldFloatData*> m_dynamicWorldInfoDataFloat;
	AlignedObjectArray<struct RigidBodyDoubleData*> m_rigidBodyDataDouble;
	AlignedObjectArray<struct RigidBodyFloatData*> m_rigidBodyDataFloat;
	AlignedObjectArray<struct CollisionObject2DoubleData*> m_collisionObjectDataDouble;
	AlignedObjectArray<struct CollisionObject2FloatData*> m_collisionObjectDataFloat;
	AlignedObjectArray<struct TypedConstraintFloatData*> m_constraintDataFloat;
	AlignedObjectArray<struct TypedConstraintDoubleData*> m_constraintDataDouble;
	AlignedObjectArray<struct TypedConstraintData*> m_constraintData;  //for backwards compatibility
	AlignedObjectArray<struct SoftBodyFloatData*> m_softBodyFloatData;
	AlignedObjectArray<struct SoftBodyDoubleData*> m_softBodyDoubleData;
};

///The DefaultSerializer is the main drx3D serialization class.
///The constructor takes an optional argument for backwards compatibility, it is recommended to leave this empty/zero.
class DefaultSerializer : public Serializer
{
protected:
	AlignedObjectArray<tuk> mTypes;
	AlignedObjectArray<short*> mStructs;
	AlignedObjectArray<short> mTlens;
	HashMap<HashInt, i32> mStructReverse;
	HashMap<HashString, i32> mTypeLookup;

	HashMap<HashPtr, uk> m_chunkP;

	HashMap<HashPtr, tukk > m_nameMap;

	HashMap<HashPtr, PointerUid> m_uniquePointers;
	i32 m_uniqueIdGenerator;

	i32 m_totalSize;
	u8* m_buffer;
	bool m_ownsBuffer;
	i32 m_currentSize;
	uk m_dna;
	i32 m_dnaLength;

	i32 m_serializationFlags;

	AlignedObjectArray<Chunk*> m_chunkPtrs;

protected:
	virtual uk findPointer(uk oldPtr)
	{
		uk * ptr = m_chunkP.find(oldPtr);
		if (ptr && *ptr)
			return *ptr;
		return 0;
	}

	virtual void writeDNA()
	{
		Chunk* dnaChunk = allocate(m_dnaLength, 1);
		memcpy(dnaChunk->m_oldPtr, m_dna, m_dnaLength);
		finalizeChunk(dnaChunk, "DNA1", DRX3D_DNA_CODE, m_dna);
	}

	i32 getReverseType(tukk type) const
	{
		HashString key(type);
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

		m_dna = AlignedAlloc(dnalen, 16);
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
			*intPtr = SwapEndian(*intPtr);

		dataLen = *intPtr;

		intPtr++;

		cp = (tuk)intPtr;
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

		intPtr = (i32*)cp;
		Assert(strncmp(cp, "TYPE", 4) == 0);
		intPtr++;

		if (!littleEndian)
			*intPtr = SwapEndian(*intPtr);

		dataLen = *intPtr;
		intPtr++;

		cp = (tuk)intPtr;
		for (i = 0; i < dataLen; i++)
		{
			mTypes.push_back(cp);
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
		intPtr = (i32*)cp;
		Assert(strncmp(cp, "TLEN", 4) == 0);
		intPtr++;

		dataLen = (i32)mTypes.size();

		shtPtr = (short*)intPtr;
		for (i = 0; i < dataLen; i++, shtPtr++)
		{
			if (!littleEndian)
				shtPtr[0] = SwapEndian(shtPtr[0]);
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
		Assert(strncmp(cp, "STRC", 4) == 0);
		intPtr++;

		if (!littleEndian)
			*intPtr = SwapEndian(*intPtr);
		dataLen = *intPtr;
		intPtr++;

		shtPtr = (short*)intPtr;
		for (i = 0; i < dataLen; i++)
		{
			mStructs.push_back(shtPtr);

			if (!littleEndian)
			{
				shtPtr[0] = SwapEndian(shtPtr[0]);
				shtPtr[1] = SwapEndian(shtPtr[1]);

				i32 len = shtPtr[1];
				shtPtr += 2;

				for (i32 a = 0; a < len; a++, shtPtr += 2)
				{
					shtPtr[0] = SwapEndian(shtPtr[0]);
					shtPtr[1] = SwapEndian(shtPtr[1]);
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
			mTypeLookup.insert(HashString(mTypes[strc[0]]), i);
		}
	}

public:
	HashMap<HashPtr, uk> m_skipPointers;

	DefaultSerializer(i32 totalSize = 0, u8* buffer = 0)
		: m_uniqueIdGenerator(0),
		  m_totalSize(totalSize),
		  m_currentSize(0),
		  m_dna(0),
		  m_dnaLength(0),
		  m_serializationFlags(0)
	{
		if (buffer == 0)
		{
			m_buffer = m_totalSize ? (u8*)AlignedAlloc(totalSize, 16) : 0;
			m_ownsBuffer = true;
		}
		else
		{
			m_buffer = buffer;
			m_ownsBuffer = false;
		}

		const bool VOID_IS_8 = ((sizeof(uk ) == 8));

#ifdef DRX3D_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
		if (VOID_IS_8)
		{
#if _WIN64
			initDNA((tukk)sBulletDNAstr64, sBulletDNAlen64);
#else
			Assert(0);
#endif
		}
		else
		{
#ifndef _WIN64
			initDNA((tukk)sBulletDNAstr, sBulletDNAlen);
#else
			Assert(0);
#endif
		}

#else   //DRX3D_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
		if (VOID_IS_8)
		{
			initDNA((tukk)sBulletDNAstr64, sBulletDNAlen64);
		}
		else
		{
			initDNA((tukk)sBulletDNAstr, sBulletDNAlen);
		}
#endif  //DRX3D_INTERNAL_UPDATE_SERIALIZATION_STRUCTURES
	}

	virtual ~DefaultSerializer()
	{
		if (m_buffer && m_ownsBuffer)
			AlignedFree(m_buffer);
		if (m_dna)
			AlignedFree(m_dna);
	}

	static i32 getMemoryDnaSizeInBytes()
	{
		const bool VOID_IS_8 = ((sizeof(uk ) == 8));

		if (VOID_IS_8)
		{
			return sBulletDNAlen64;
		}
		return sBulletDNAlen;
	}
	static tukk getMemoryDna()
	{
		const bool VOID_IS_8 = ((sizeof(uk ) == 8));
		if (VOID_IS_8)
		{
			return (tukk)sBulletDNAstr64;
		}
		return (tukk)sBulletDNAstr;
	}

	void insertHeader()
	{
		writeHeader(m_buffer);
		m_currentSize += DRX3D_HEADER_LENGTH;
	}

	void writeHeader(u8* buffer) const
	{
#ifdef DRX3D_USE_DOUBLE_PRECISION
		memcpy(buffer, "BULLETd", 7);
#else
		memcpy(buffer, "BULLETf", 7);
#endif  //DRX3D_USE_DOUBLE_PRECISION

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

		buffer[9] = '3';
		buffer[10] = '2';
		buffer[11] = '5';
	}

	virtual void startSerialization()
	{
		m_uniqueIdGenerator = 1;
		if (m_totalSize)
		{
			u8* buffer = internalAlloc(DRX3D_HEADER_LENGTH);
			writeHeader(buffer);
		}
	}

	virtual void finishSerialization()
	{
		writeDNA();

		//if we didn't pre-allocate a buffer, we need to create a contiguous buffer now
		if (!m_totalSize)
		{
			if (m_buffer)
				AlignedFree(m_buffer);

			m_currentSize += DRX3D_HEADER_LENGTH;
			m_buffer = (u8*)AlignedAlloc(m_currentSize, 16);

			u8* currentPtr = m_buffer;
			writeHeader(m_buffer);
			currentPtr += DRX3D_HEADER_LENGTH;
			for (i32 i = 0; i < m_chunkPtrs.size(); i++)
			{
				i32 curLength = (i32)sizeof(Chunk) + m_chunkPtrs[i]->m_length;
				memcpy(currentPtr, m_chunkPtrs[i], curLength);
				AlignedFree(m_chunkPtrs[i]);
				currentPtr += curLength;
			}
		}

		mTypes.clear();
		mStructs.clear();
		mTlens.clear();
		mStructReverse.clear();
		mTypeLookup.clear();
		m_skipPointers.clear();
		m_chunkP.clear();
		m_nameMap.clear();
		m_uniquePointers.clear();
		m_chunkPtrs.clear();
	}

	virtual uk getUniquePointer(uk oldPtr)
	{
		Assert(m_uniqueIdGenerator >= 0);
		if (!oldPtr)
			return 0;

		PointerUid* uptr = (PointerUid*)m_uniquePointers.find(oldPtr);
		if (uptr)
		{
			return uptr->m_ptr;
		}

		uk * ptr2 = m_skipPointers[oldPtr];
		if (ptr2)
		{
			return 0;
		}

		m_uniqueIdGenerator++;

		PointerUid uid;
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

	virtual void finalizeChunk(Chunk* chunk, tukk structType, i32 chunkCode, uk oldPtr)
	{
		if (!(m_serializationFlags & DRX3D_SERIALIZE_NO_DUPLICATE_ASSERT))
		{
			Assert(!findPointer(oldPtr));
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
			Assert(m_currentSize < m_totalSize);
		}
		else
		{
			ptr = (u8*)AlignedAlloc(size, 16);
			m_currentSize += i32(size);
		}
		return ptr;
	}

	virtual Chunk* allocate(size_t size, i32 numElements)
	{
		u8* ptr = internalAlloc(i32(size) * numElements + sizeof(Chunk));

		u8* data = ptr + sizeof(Chunk);

		Chunk* chunk = (Chunk*)ptr;
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

			i32 len = StrLen(name);
			if (len)
			{
				i32 newLen = len + 1;
				i32 padding = ((newLen + 3) & ~3) - newLen;
				newLen += padding;

				//serialize name string now
				Chunk* chunk = allocate(sizeof(char), newLen);
				tuk destinationName = (tuk)chunk->m_oldPtr;
				for (i32 i = 0; i < len; i++)
				{
					destinationName[i] = name[i];
				}
				destinationName[len] = 0;
				finalizeChunk(chunk, "char", DRX3D_ARRAY_CODE, (uk )name);
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
	i32 getNumChunks() const
	{
		return m_chunkPtrs.size();
	}

	const Chunk* getChunk(i32 chunkIndex) const
	{
		return m_chunkPtrs[chunkIndex];
	}
};

///In general it is best to use DefaultSerializer,
///in particular when writing the data to disk or sending it over the network.
///The InMemorySerializer is experimental and only suitable in a few cases.
///The InMemorySerializer takes a shortcut and can be useful to create a deep-copy
///of objects. There will be a demo on how to use the InMemorySerializer.
#ifdef ENABLE_INMEMORY_SERIALIZER

struct InMemorySerializer : public DefaultSerializer
{
	HashMap<HashPtr, Chunk*> m_uid2ChunkPtr;
	HashMap<HashPtr, uk> m_orgPtr2UniqueDataPtr;
	HashMap<HashString, ukk> m_names2Ptr;

	BulletSerializedArrays m_arrays;

	InMemorySerializer(i32 totalSize = 0, u8* buffer = 0)
		: DefaultSerializer(totalSize, buffer)
	{
	}

	virtual void startSerialization()
	{
		m_uid2ChunkPtr.clear();
		//todo: m_arrays.clear();
		DefaultSerializer::startSerialization();
	}

	Chunk* findChunkFromUniquePointer(uk uniquePointer)
	{
		Chunk** chkPtr = m_uid2ChunkPtr[uniquePointer];
		if (chkPtr)
		{
			return *chkPtr;
		}
		return 0;
	}

	virtual void registerNameForPointer(ukk ptr, tukk name)
	{
		DefaultSerializer::registerNameForPointer(ptr, name);
		m_names2Ptr.insert(name, ptr);
	}

	virtual void finishSerialization()
	{
	}

	virtual uk getUniquePointer(uk oldPtr)
	{
		if (oldPtr == 0)
			return 0;

		// uk uniquePtr = getUniquePointer(oldPtr);
		Chunk* chunk = findChunkFromUniquePointer(oldPtr);
		if (chunk)
		{
			return chunk->m_oldPtr;
		}
		else
		{
			tukk n = (tukk)oldPtr;
			ukk* ptr = m_names2Ptr[n];
			if (ptr)
			{
				return oldPtr;
			}
			else
			{
				uk * ptr2 = m_skipPointers[oldPtr];
				if (ptr2)
				{
					return 0;
				}
				else
				{
					//If this assert hit, serialization happened in the wrong order
					// 'getUniquePointer'
					Assert(0);
				}
			}
			return 0;
		}
		return oldPtr;
	}

	virtual void finalizeChunk(Chunk* chunk, tukk structType, i32 chunkCode, uk oldPtr)
	{
		if (!(m_serializationFlags & DRX3D_SERIALIZE_NO_DUPLICATE_ASSERT))
		{
			Assert(!findPointer(oldPtr));
		}

		chunk->m_dna_nr = getReverseType(structType);
		chunk->m_chunkCode = chunkCode;
		//uk uniquePtr = getUniquePointer(oldPtr);
		m_chunkP.insert(oldPtr, oldPtr);  //chunk->m_oldPtr);
		// chunk->m_oldPtr = uniquePtr;//oldPtr;

		uk uid = findPointer(oldPtr);
		m_uid2ChunkPtr.insert(uid, chunk);

		switch (chunk->m_chunkCode)
		{
			case DRX3D_SOFTBODY_CODE:
			{
#ifdef DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_softBodyDoubleData.push_back((SoftBodyDoubleData*)chunk->m_oldPtr);
#else
				m_arrays.m_softBodyFloatData.push_back((SoftBodyFloatData*)chunk->m_oldPtr);
#endif
				break;
			}
			case DRX3D_COLLISIONOBJECT_CODE:
			{
#ifdef DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_collisionObjectDataDouble.push_back((CollisionObject2DoubleData*)chunk->m_oldPtr);
#else   //DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_collisionObjectDataFloat.push_back((CollisionObject2FloatData*)chunk->m_oldPtr);
#endif  //DRX3D_USE_DOUBLE_PRECISION
				break;
			}
			case DRX3D_RIGIDBODY_CODE:
			{
#ifdef DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_rigidBodyDataDouble.push_back((RigidBodyDoubleData*)chunk->m_oldPtr);
#else
				m_arrays.m_rigidBodyDataFloat.push_back((RigidBodyFloatData*)chunk->m_oldPtr);
#endif  //DRX3D_USE_DOUBLE_PRECISION
				break;
			};
			case DRX3D_CONSTRAINT_CODE:
			{
#ifdef DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_constraintDataDouble.push_back((TypedConstraintDoubleData*)chunk->m_oldPtr);
#else
				m_arrays.m_constraintDataFloat.push_back((TypedConstraintFloatData*)chunk->m_oldPtr);
#endif
				break;
			}
			case DRX3D_QUANTIZED_BVH_CODE:
			{
#ifdef DRX3D_USE_DOUBLE_PRECISION
				m_arrays.m_bvhsDouble.push_back((QuantizedBvhDoubleData*)chunk->m_oldPtr);
#else
				m_arrays.m_bvhsFloat.push_back((QuantizedBvhFloatData*)chunk->m_oldPtr);
#endif
				break;
			}

			case DRX3D_SHAPE_CODE:
			{
				CollisionShapeData* shapeData = (CollisionShapeData*)chunk->m_oldPtr;
				m_arrays.m_colShapeData.push_back(shapeData);
				break;
			}
			case DRX3D_TRIANLGE_INFO_MAP:
			case DRX3D_ARRAY_CODE:
			case DRX3D_SBMATERIAL_CODE:
			case DRX3D_SBNODE_CODE:
			case DRX3D_DYNAMICSWORLD_CODE:
			case DRX3D_DNA_CODE:
			{
				break;
			}
			default:
			{
			}
		};
	}

	i32 getNumChunks() const
	{
		return m_uid2ChunkPtr.size();
	}

	const Chunk* getChunk(i32 chunkIndex) const
	{
		return *m_uid2ChunkPtr.getAtIndex(chunkIndex);
	}
};
#endif  //ENABLE_INMEMORY_SERIALIZER

#endif  //DRX3D_SERIALIZER_H
