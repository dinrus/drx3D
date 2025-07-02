// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/D3D/Gpu/GpuComputeBackend.h>
#include <drx3D/Render/ComputeSkinningStorage.h>
#include <drx3D/Render/GraphicsPipelineStage.h>
#include <drx3D/Render/D3D/GraphicsPipeline/Common/ComputeRenderPass.h>

namespace compute_skinning
{

#pragma pack(push)
#pragma pack(1)

struct SComputeShaderSkinVertexOut
{
	Vec3  pos;
	float pad1;
	Quat  qtangent;
	Vec3  tangent;
	float pad2;
	Vec3  bitangent;
	float pad3;
	Vec2  uv;
	float pad4[2];
};

// check ORBIS
struct SIndices
{
	u32 ind;
};

struct SComputeShaderTriangleNT
{
	Vec3 normal;
	Vec3 tangent;
};

struct SMorphsDeltas
{
	Vec4 delta;
};

struct SMorphsBitField
{
	// packed as 32:32
	uint64 maskZoom;
};

#pragma pack(pop)

template<typename T> class CTypedReadResource
{
public:
	CTypedReadResource()
		: m_size(0)
	{}

	CGpuBuffer& GetBuffer() { return m_buffer; };

	void        UpdateBufferContent(const T* pData, size_t nSize) // called from multiple threads: always recreate immutable buffer
	{
		m_buffer.Create(nSize, sizeof(T), DXGI_FORMAT_UNKNOWN, CDeviceObjectFactory::USAGE_STRUCTURED | CDeviceObjectFactory::BIND_SHADER_RESOURCE, (ukk )pData);
		m_size = nSize;
	}

	i32 GetSize() { return m_size; }
private:
	i32        m_size;
	CGpuBuffer m_buffer;
};

struct SPerMeshResources : public compute_skinning::IPerMeshDataSupply
{
	CTypedReadResource<compute_skinning::SSkinning>     skinningVector;
	CTypedReadResource<compute_skinning::SSkinningMap>  skinningVectorMap;

	CTypedReadResource<compute_skinning::SSkinVertexIn> verticesIn;
	CTypedReadResource<vtx_idx>                         indicesIn;
	CTypedReadResource<u32>                          adjTriangles;

	// morph specific
	CTypedReadResource<Vec4>   morphsDeltas;
	CTypedReadResource<uint64> morphsBitField;

	size_t GetSizeBytes();

	enum SState
	{
		sState_NonInitialized     = 0,

		sState_PosesInitialized   = BIT(0),
		sState_WeightsInitialized = BIT(1),
		sState_MorphsInitialized  = BIT(2)
	};

	std::atomic<u32> uploadState;
	SPerMeshResources() : uploadState(sState_NonInitialized) {}
	bool IsInitialized(u32 wantedState) const { return (uploadState & wantedState) == wantedState; }

	// per mesh data supply implementation
	virtual void PushMorphs(i32k numMorps, i32k numMorphsBitField, const Vec4* morphsDeltas, const uint64* morphsBitField) override;
	virtual void PushBindPoseBuffers(i32k numVertices, i32k numIndices, i32k numAdjTriangles, const compute_skinning::SSkinVertexIn* vertices, const vtx_idx* indices, u32k* adjTriangles) override;
	virtual void PushWeights(i32k numWeights, i32k numWeightsMap, const compute_skinning::SSkinning* weights, const compute_skinning::SSkinningMap* weightsMap) override;
};

struct SPerInstanceResources
{
	SPerInstanceResources(i32k numVertices, i32k numTriangles);
	~SPerInstanceResources();

	CComputeRenderPass passDeform;
	CComputeRenderPass passDeformWithMorphs;
	CComputeRenderPass passTriangleTangents;
	CComputeRenderPass passVertexTangents;
	int64              lastFrameInUse;

	size_t             GetSizeBytes();

	// output data
	gpu::CStructuredResource<SComputeShaderSkinVertexOut, gpu::BufferFlagsReadWrite> verticesOut;
	gpu::CStructuredResource<SComputeShaderTriangleNT, gpu::BufferFlagsReadWrite>    tangentsOut;
};

class CStorage : public compute_skinning::IComputeSkinningStorage
{
public:
	std::shared_ptr<IPerMeshDataSupply>    GetOrCreateComputeSkinningPerMeshData(const CRenderMesh* pMesh) override;
	virtual CGpuBuffer*                    GetOutputVertices(ukk pCustomTag) override;

	void                                   RetirePerMeshResources();
	void                                   RetirePerInstanceResources(int64 frameId);
	std::shared_ptr<SPerMeshResources>     GetPerMeshResources(CRenderMesh* pMesh);
	std::shared_ptr<SPerInstanceResources> GetOrCreatePerInstanceResources(int64 frameId,ukk pCustomTag, i32k numVertices, i32k numTriangles);
	void                                   DebugDraw();

private:
	// needs a lock since this is updated through the streaming thread
	std::unordered_map<const CRenderMesh*, std::shared_ptr<SPerMeshResources>> m_perMeshResources;
	// this is addressed with the pCustomTag, identifying the Skin Attachment
	// this is only accessed through the render thread, so a lock shouldn't be necessary
	std::unordered_map<ukk , std::shared_ptr<SPerInstanceResources>> m_perInstanceResources;

	DrxCriticalSectionNonRecursive m_csMesh;
	DrxCriticalSectionNonRecursive m_csInstance;
};

}

class CComputeSkinningStage : public CGraphicsPipelineStage
{
public:
	CComputeSkinningStage();

	void Update() final;
	void Prepare();
	void Execute();
	void PreDraw();

	compute_skinning::IComputeSkinningStorage& GetStorage() { return m_storage; }

private:
	compute_skinning::CStorage m_storage;

#if !defined(_RELEASE) // !NDEBUG
	i32                      m_oldFrameIdExecute = -1;
#endif
};
