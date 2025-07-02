// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CGpuBuffer;

namespace compute_skinning
{

#pragma pack(push)
#pragma pack(1)

struct SSkinning
{
	// packed as 8:24
	uint weightIndex;
};

struct SSkinningMap
{
	uint offset;
	uint count;
};

#pragma pack(pop)

struct IPerMeshDataSupply
{
	virtual void PushMorphs(i32k numMorphs, i32k numMorphsBitField, const Vec4* morphsDeltas, const uint64* morphsBitField) = 0;
	virtual void PushBindPoseBuffers(i32k numVertices, i32k numIndices, i32k numAdjTriangles, const compute_skinning::SSkinVertexIn* vertices, const vtx_idx* indices, u32k* adjTriangles) = 0;
	virtual void PushWeights(i32k numWeights, i32k numWeightsMap, const compute_skinning::SSkinning* weights, const compute_skinning::SSkinningMap* weightsMap) = 0;
};

struct IComputeSkinningStorage
{
	virtual std::shared_ptr<compute_skinning::IPerMeshDataSupply> GetOrCreateComputeSkinningPerMeshData(const CRenderMesh* pMesh) = 0;
	virtual CGpuBuffer*                                           GetOutputVertices(ukk pCustomTag) = 0;
};

}
