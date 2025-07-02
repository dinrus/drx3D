// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __RenderMeshMerger_H
#define __RenderMeshMerger_H

#pragma once

//////////////////////////////////////////////////////////////////////////
// Input structure for RenderMesh merger
//////////////////////////////////////////////////////////////////////////
struct SRenderMeshInfoInput
{
	_smart_ptr<IRenderMesh> pMesh;
	_smart_ptr<IMaterial>   pMat;
	IRenderNode*            pSrcRndNode;
	Matrix34                mat;
	i32                     nSubObjectIndex;
	i32                     nChunkId;
	bool                    bIdentityMatrix;

	SRenderMeshInfoInput() : pMesh(0), pMat(0), nChunkId(-1), nSubObjectIndex(0), pSrcRndNode(0), bIdentityMatrix(false) { mat.SetIdentity(); }

	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SDecalClipInfo
{
	Vec3  vPos;
	float fRadius;
	Vec3  vProjDir;
};

struct SMergeInfo
{
	tukk     sMeshType;
	tukk     sMeshName;
	bool            bCompactVertBuffer;
	bool            bPrintDebugMessages;
	bool            bMakeNewMaterial;
	bool            bMergeToOneRenderMesh;
	bool            bPlaceInstancePositionIntoVertexNormal;
	IMaterial*      pUseMaterial; // Force to use this material

	SDecalClipInfo* pDecalClipInfo;
	AABB*           pClipCellBox;
	Vec3            vResultOffset; // this offset will be subtracted from output vertex positions

	SMergeInfo() :
		sMeshType(""),
		sMeshName(""),
		bCompactVertBuffer(false),
		bPrintDebugMessages(false),
		bMakeNewMaterial(true),
		bMergeToOneRenderMesh(false),
		bPlaceInstancePositionIntoVertexNormal(0),
		pUseMaterial(0),
		pDecalClipInfo(0),
		pClipCellBox(0),
		vResultOffset(0, 0, 0) {}
};

struct SMergedChunk
{
	CRenderChunk          rChunk;
	IMaterial*            pMaterial;
	SRenderMeshInfoInput* pFromMesh;

	void                  GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SMergeBuffersData
{
	i32      nPosStride;
	i32      nTexStride;
	i32      nColorStride;
	i32      nTangsStride;
	i32      nIndCount;
	u8*   pPos;
	u8*   pTex;
	u8*   pColor;
	u8*   pTangs;
	vtx_idx* pSrcInds;
#if ENABLE_NORMALSTREAM_SUPPORT
	i32      nNormStride;
	u8*   pNorm;
#endif
};

class CRenderMeshMerger : public DinrusX3dEngBase
{
public:
	CRenderMeshMerger(void);
	~CRenderMeshMerger(void);

	void                    Reset();

	_smart_ptr<IRenderMesh> MergeRenderMeshes(SRenderMeshInfoInput* pRMIArray, i32 nRMICount, PodArray<SRenderMeshInfoOutput>& outRenderMeshes, SMergeInfo& info);

	_smart_ptr<IRenderMesh> MergeRenderMeshes(SRenderMeshInfoInput* pRMIArray, i32 nRMICount, SMergeInfo& info);
public:

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));

		pSizer->AddObject(m_lstRMIChunks);
		pSizer->AddObject(m_lstVerts);
		pSizer->AddObject(m_lstTangBasises);
		pSizer->AddObject(m_lstIndices);

		pSizer->AddObject(m_lstChunks);
		pSizer->AddObject(m_lstChunksAll);

		pSizer->AddObject(m_lstNewVerts);
		pSizer->AddObject(m_lstNewTangBasises);
		pSizer->AddObject(m_lstNewIndices);
		pSizer->AddObject(m_lstNewChunks);

		pSizer->AddObject(m_lstChunksMergedTemp);

		pSizer->AddObject(m_tmpRenderChunkArray);
		pSizer->AddObject(m_tmpClipContext);
	}

	void MergeBuffersImpl(AABB* pBounds, SMergeBuffersData* arrMergeBuffersData);

protected:
	PodArray<SRenderMeshInfoInput> m_lstRMIChunks;
	PodArray<SVF_P3S_C4B_T2S>      m_lstVerts;
	PodArray<SPipTangents>         m_lstTangBasises;
	PodArray<u32>               m_lstIndices;

	PodArray<SMergedChunk>         m_lstChunks;
	PodArray<SMergedChunk>         m_lstChunksAll;

	PodArray<SVF_P3S_C4B_T2S>      m_lstNewVerts;
	PodArray<SPipTangents>         m_lstNewTangBasises;
	PodArray<vtx_idx>              m_lstNewIndices;
	PodArray<SMergedChunk>         m_lstNewChunks;

#if ENABLE_NORMALSTREAM_SUPPORT
	PodArray<SPipNormal> m_lstNormals;
	PodArray<SPipNormal> m_lstNewNormals;
#endif

	PodArray<SMergedChunk>      m_lstChunksMergedTemp;

	TRenderChunkArray           m_tmpRenderChunkArray;

	PodArray<SMergeBuffersData> m_lstMergeBuffersData;
	AABB                        m_tmpAABB;

	CPolygonClipContext         m_tmpClipContext;

	i32                         m_nTotalVertexCount;
	i32                         m_nTotalIndexCount;

private:
	bool       GenerateRenderChunks(SRenderMeshInfoInput* pRMIArray, i32 nRMICount);
	void       MergeRenderChunks();
	void       MergeBuffers(AABB& bounds);

	void       MakeRenderMeshInfoListOfAllChunks(SRenderMeshInfoInput* pRMIArray, i32 nRMICount, SMergeInfo& info);
	void       MakeListOfAllCRenderChunks(SMergeInfo& info);
	void       IsChunkValid(CRenderChunk& Ch, PodArray<SVF_P3S_C4B_T2S>& lstVerts, PodArray<u32>& lstIndices);

	void       ClipByAABB(SMergeInfo& info);
	void       ClipDecals(SMergeInfo& info);
	void       CompactVertices(SMergeInfo& info);
	void       TryMergingChunks(SMergeInfo& info);

	bool       ClipTriangle(i32 nStartIdxId, Plane* pPlanes, i32 nPlanesNum);

	static i32 Cmp_Materials(IMaterial* pMat1, IMaterial* pMat2);
	static i32 Cmp_RenderChunks_(ukk v1, ukk v2);
	static i32 Cmp_RenderChunksInfo(ukk v1, ukk v2);

};

#endif //__RenderMeshMerger_H
