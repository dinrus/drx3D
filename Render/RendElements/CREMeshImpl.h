// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CREMeshImpl final : public CREMesh
{
public:

	uint       m_nPatchIDOffset;

	CREMeshImpl()
		: m_nPatchIDOffset(0)
	{}

	virtual ~CREMeshImpl()
	{}

public:
	struct CRenderChunk* mfGetMatInfo();
	TRenderChunkArray*   mfGetMatInfoList();
	i32                  mfGetMatId();

	bool                 mfIsHWSkinned()
	{
		return (m_Flags & FCEF_SKINNED) != 0;
	}
	void  mfGetPlane(Plane& pl);

	void  mfCenter(Vec3& Pos, CRenderObject* pObj, const SRenderingPassInfo& passInfo);

	uk mfGetPointer(ESrcPointer ePT, i32* Stride, EParamType Type, ESrcPointer Dst, i32 Flags);
	bool  mfUpdate(InputLayoutHandle eVertFormat, i32 Flags, bool bTessellation = false);
	void  mfGetBBox(Vec3& vMins, Vec3& vMaxs) const;

	i32   Size()
	{
		i32 nSize = sizeof(*this);
		return nSize;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	bool          BindRemappedSkinningData(u32 guid);

	bool          GetGeometryInfo(SGeometryInfo& geomInfo, bool bSupportTessellation = false);
	InputLayoutHandle GetVertexFormat() const;
	bool          Compile(CRenderObject* pObj,CRenderView *pRenderView, bool updateInstanceDataOnly);
	void          DrawToCommandList(CRenderObject* pObj, const SGraphicsPipelinePassContext& ctx, CDeviceCommandList* commandList);

	//protected:
	//	CREMeshImpl(CREMeshImpl&);
	//	CREMeshImpl& operator=(CREMeshImpl&);
};