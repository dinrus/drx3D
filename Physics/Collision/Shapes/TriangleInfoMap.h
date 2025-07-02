#ifndef _DRX3D_TRIANGLE_INFO_MAP_H
#define _DRX3D_TRIANGLE_INFO_MAP_H

#include <drx3D/Maths/Linear/HashMap.h>
#include <drx3D/Maths/Linear/Serializer.h>

///for TriangleInfo m_flags
#define TRI_INFO_V0V1_CONVEX 1
#define TRI_INFO_V1V2_CONVEX 2
#define TRI_INFO_V2V0_CONVEX 4

#define TRI_INFO_V0V1_SWAP_NORMALB 8
#define TRI_INFO_V1V2_SWAP_NORMALB 16
#define TRI_INFO_V2V0_SWAP_NORMALB 32

///The TriangleInfo structure stores information to adjust collision normals to avoid collisions against internal edges
///it can be generated using
struct TriangleInfo
{
	TriangleInfo()
	{
		m_edgeV0V1Angle = SIMD_2_PI;
		m_edgeV1V2Angle = SIMD_2_PI;
		m_edgeV2V0Angle = SIMD_2_PI;
		m_flags = 0;
	}

	i32 m_flags;

	Scalar m_edgeV0V1Angle;
	Scalar m_edgeV1V2Angle;
	Scalar m_edgeV2V0Angle;
};

typedef HashMap<HashInt, TriangleInfo> InternalTriangleInfoMap;

///The TriangleInfoMap stores edge angle information for some triangles. You can compute this information yourself or using GenerateInternalEdgeInfo.
struct TriangleInfoMap : public InternalTriangleInfoMap
{
	Scalar m_convexEpsilon;          ///used to determine if an edge or contact normal is convex, using the dot product
	Scalar m_planarEpsilon;          ///used to determine if a triangle edge is planar with zero angle
	Scalar m_equalVertexThreshold;   ///used to compute connectivity: if the distance between two vertices is smaller than m_equalVertexThreshold, they are considered to be 'shared'
	Scalar m_edgeDistanceThreshold;  ///used to determine edge contacts: if the closest distance between a contact point and an edge is smaller than this distance threshold it is considered to "hit the edge"
	Scalar m_maxEdgeAngleThreshold;  //ignore edges that connect triangles at an angle larger than this m_maxEdgeAngleThreshold
	Scalar m_zeroAreaThreshold;      ///used to determine if a triangle is degenerate (length squared of cross product of 2 triangle edges < threshold)

	TriangleInfoMap()
	{
		m_convexEpsilon = 0.00f;
		m_planarEpsilon = 0.0001f;
		m_equalVertexThreshold = Scalar(0.0001) * Scalar(0.0001);
		m_edgeDistanceThreshold = Scalar(0.1);
		m_zeroAreaThreshold = Scalar(0.0001) * Scalar(0.0001);
		m_maxEdgeAngleThreshold = SIMD_2_PI;
	}
	virtual ~TriangleInfoMap() {}

	virtual i32 calculateSerializeBufferSize() const;

	///fills the dataBuffer and returns the struct name (and 0 on failure)
	virtual tukk serialize(uk dataBuffer, Serializer* serializer) const;

	void deSerialize(struct TriangleInfoMapData& data);
};

// clang-format off

///those fields have to be float and not Scalar for the serialization to work properly
struct	TriangleInfoData
{
	i32			m_flags;
	float	m_edgeV0V1Angle;
	float	m_edgeV1V2Angle;
	float	m_edgeV2V0Angle;
};

struct	TriangleInfoMapData
{
	i32					*m_hashTablePtr;
	i32					*m_nextPtr;
	TriangleInfoData	*m_valueArrayPtr;
	i32					*m_keyArrayPtr;

	float	m_convexEpsilon;
	float	m_planarEpsilon;
	float	m_equalVertexThreshold; 
	float	m_edgeDistanceThreshold;
	float	m_zeroAreaThreshold;

	i32		m_nextSize;
	i32		m_hashTableSize;
	i32		m_numValues;
	i32		m_numKeys;
	char	m_padding[4];
};

// clang-format on

SIMD_FORCE_INLINE i32 TriangleInfoMap::calculateSerializeBufferSize() const
{
	return sizeof(TriangleInfoMapData);
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE tukk TriangleInfoMap::serialize(uk dataBuffer, Serializer* serializer) const
{
	TriangleInfoMapData* tmapData = (TriangleInfoMapData*)dataBuffer;
	tmapData->m_convexEpsilon = (float)m_convexEpsilon;
	tmapData->m_planarEpsilon = (float)m_planarEpsilon;
	tmapData->m_equalVertexThreshold = (float)m_equalVertexThreshold;
	tmapData->m_edgeDistanceThreshold = (float)m_edgeDistanceThreshold;
	tmapData->m_zeroAreaThreshold = (float)m_zeroAreaThreshold;

	tmapData->m_hashTableSize = m_hashTable.size();

	tmapData->m_hashTablePtr = tmapData->m_hashTableSize ? (i32*)serializer->getUniquePointer((uk )&m_hashTable[0]) : 0;
	if (tmapData->m_hashTablePtr)
	{
		//serialize an i32 buffer
		i32 sz = sizeof(i32);
		i32 numElem = tmapData->m_hashTableSize;
		Chunk* chunk = serializer->allocate(sz, numElem);
		i32* memPtr = (i32*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			*memPtr = m_hashTable[i];
		}
		serializer->finalizeChunk(chunk, "i32", DRX3D_ARRAY_CODE, (uk )&m_hashTable[0]);
	}

	tmapData->m_nextSize = m_next.size();
	tmapData->m_nextPtr = tmapData->m_nextSize ? (i32*)serializer->getUniquePointer((uk )&m_next[0]) : 0;
	if (tmapData->m_nextPtr)
	{
		i32 sz = sizeof(i32);
		i32 numElem = tmapData->m_nextSize;
		Chunk* chunk = serializer->allocate(sz, numElem);
		i32* memPtr = (i32*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			*memPtr = m_next[i];
		}
		serializer->finalizeChunk(chunk, "i32", DRX3D_ARRAY_CODE, (uk )&m_next[0]);
	}

	tmapData->m_numValues = m_valueArray.size();
	tmapData->m_valueArrayPtr = tmapData->m_numValues ? (TriangleInfoData*)serializer->getUniquePointer((uk )&m_valueArray[0]) : 0;
	if (tmapData->m_valueArrayPtr)
	{
		i32 sz = sizeof(TriangleInfoData);
		i32 numElem = tmapData->m_numValues;
		Chunk* chunk = serializer->allocate(sz, numElem);
		TriangleInfoData* memPtr = (TriangleInfoData*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			memPtr->m_edgeV0V1Angle = (float)m_valueArray[i].m_edgeV0V1Angle;
			memPtr->m_edgeV1V2Angle = (float)m_valueArray[i].m_edgeV1V2Angle;
			memPtr->m_edgeV2V0Angle = (float)m_valueArray[i].m_edgeV2V0Angle;
			memPtr->m_flags = m_valueArray[i].m_flags;
		}
		serializer->finalizeChunk(chunk, "TriangleInfoData", DRX3D_ARRAY_CODE, (uk )&m_valueArray[0]);
	}

	tmapData->m_numKeys = m_keyArray.size();
	tmapData->m_keyArrayPtr = tmapData->m_numKeys ? (i32*)serializer->getUniquePointer((uk )&m_keyArray[0]) : 0;
	if (tmapData->m_keyArrayPtr)
	{
		i32 sz = sizeof(i32);
		i32 numElem = tmapData->m_numValues;
		Chunk* chunk = serializer->allocate(sz, numElem);
		i32* memPtr = (i32*)chunk->m_oldPtr;
		for (i32 i = 0; i < numElem; i++, memPtr++)
		{
			*memPtr = m_keyArray[i].getUid1();
		}
		serializer->finalizeChunk(chunk, "i32", DRX3D_ARRAY_CODE, (uk )&m_keyArray[0]);
	}

	// Fill padding with zeros to appease msan.
	tmapData->m_padding[0] = 0;
	tmapData->m_padding[1] = 0;
	tmapData->m_padding[2] = 0;
	tmapData->m_padding[3] = 0;

	return "TriangleInfoMapData";
}

///fills the dataBuffer and returns the struct name (and 0 on failure)
SIMD_FORCE_INLINE void TriangleInfoMap::deSerialize(TriangleInfoMapData& tmapData)
{
	m_convexEpsilon = tmapData.m_convexEpsilon;
	m_planarEpsilon = tmapData.m_planarEpsilon;
	m_equalVertexThreshold = tmapData.m_equalVertexThreshold;
	m_edgeDistanceThreshold = tmapData.m_edgeDistanceThreshold;
	m_zeroAreaThreshold = tmapData.m_zeroAreaThreshold;
	m_hashTable.resize(tmapData.m_hashTableSize);
	i32 i = 0;
	for (i = 0; i < tmapData.m_hashTableSize; i++)
	{
		m_hashTable[i] = tmapData.m_hashTablePtr[i];
	}
	m_next.resize(tmapData.m_nextSize);
	for (i = 0; i < tmapData.m_nextSize; i++)
	{
		m_next[i] = tmapData.m_nextPtr[i];
	}
	m_valueArray.resize(tmapData.m_numValues);
	for (i = 0; i < tmapData.m_numValues; i++)
	{
		m_valueArray[i].m_edgeV0V1Angle = tmapData.m_valueArrayPtr[i].m_edgeV0V1Angle;
		m_valueArray[i].m_edgeV1V2Angle = tmapData.m_valueArrayPtr[i].m_edgeV1V2Angle;
		m_valueArray[i].m_edgeV2V0Angle = tmapData.m_valueArrayPtr[i].m_edgeV2V0Angle;
		m_valueArray[i].m_flags = tmapData.m_valueArrayPtr[i].m_flags;
	}

	m_keyArray.resize(tmapData.m_numKeys, HashInt(0));
	for (i = 0; i < tmapData.m_numKeys; i++)
	{
		m_keyArray[i].setUid1(tmapData.m_keyArrayPtr[i]);
	}
}

#endif  //_DRX3D_TRIANGLE_INFO_MAP_H
