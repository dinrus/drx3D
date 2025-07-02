// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CDeviceCommandList;
class CWaterStage;
namespace water
{
struct SCompiledWaterOcean;
}

class CREWaterOcean : public CRenderElement
{
public:
	struct SWaterOceanParam
	{
		bool bWaterOceanFFT;

		SWaterOceanParam() : bWaterOceanFFT(false) {}
	};

public:
	CREWaterOcean();
	virtual ~CREWaterOcean();

	virtual void mfGetPlane(Plane& pl) override;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	virtual bool            Compile(CRenderObject* pObj, CRenderView *pRenderView, bool updateInstanceDataOnly) override;
	virtual void            DrawToCommandList(CRenderObject* pObj, const struct SGraphicsPipelinePassContext& ctx, CDeviceCommandList* commandList) override;

	virtual bool            RequestVerticesBuffer(SVF_P3F_C4B_T2F** pOutputVertices, u8** pOutputIndices, u32 nVerticesCount, u32 nIndicesCount, u32 nIndexSizeof);
	virtual bool            SubmitVerticesBuffer(u32 nVerticesCount, u32 nIndicesCount, u32 nIndexSizeof, SVF_P3F_C4B_T2F* pVertices, u8* pIndices);
	virtual Vec4*           GetDisplaceGrid() const;

	virtual SHRenderTarget* GetReflectionRenderTarget();

public:
	SWaterOceanParam m_oceanParam[RT_COMMAND_BUF_COUNT];

private:
	struct SUpdateRequest
	{
		u32           nVerticesCount = 0;
		SVF_P3F_C4B_T2F* pVertices = nullptr;
		u32           nIndicesCount = 0;
		u8*           pIndices = nullptr;
		u32           nIndexSizeof = 0;
	};

private:
	void Create(u32 nVerticesCount, SVF_P3F_C4B_T2F* pVertices, u32 nIndicesCount, ukk pIndices, u32 nIndexSizeof);
	void CreateVertexAndIndexBuffer(threadID threadId);
	void FrameUpdate();
	void ReleaseOcean();

	void PrepareForUse(water::SCompiledWaterOcean& compiledObj, bool bInstanceOnly, CDeviceCommandList& RESTRICT_REFERENCE commandList) const;

	void UpdatePerDrawRS(water::SCompiledWaterOcean& RESTRICT_REFERENCE compiledObj, const SWaterOceanParam& oceanParam, const CWaterStage& waterStage);
	void UpdatePerDrawCB(water::SCompiledWaterOcean& RESTRICT_REFERENCE compiledObj, const CRenderObject& renderObj) const;
	void UpdateVertex(water::SCompiledWaterOcean& compiledObj, i32 primType);

private:
	std::unique_ptr<water::SCompiledWaterOcean> m_pCompiledObject;

	std::unique_ptr<SHRenderTarget>             m_pRenderTarget;

	stream_handle_t                             m_vertexBufferHandle;
	stream_handle_t                             m_indexBufferHandle;

	u32                      m_nVerticesCount;
	u32                      m_nIndicesCount;
	u32                      m_nIndexSizeof;

	std::vector<SUpdateRequest> m_verticesUpdateRequests[RT_COMMAND_BUF_COUNT];

};
