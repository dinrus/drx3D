// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <utility>

#include <drx3D/Animation/AttachmentBase.h>
#include <drx3D/Animation/ModelSkin.h>

class CAttachmentMerged;
struct ISkin;

class CAttachmentMerger : public IAttachmentMerger
{
	static const EDefaultInputLayouts::PreDefs TargetVertexFormat = EDefaultInputLayouts::P3F;

public:

	struct ShadowChunkRange
	{
		struct IsShadowChunk
		{
			IsShadowChunk(IMaterial* pMat)
				: pMaterial(pMat) {}

			bool operator()(const CRenderChunk& chunk) const
			{
				IMaterial* pSubMtl = pMaterial->GetSafeSubMtl(chunk.m_nMatID);
				return (pSubMtl->GetFlags() & (MTL_FLAG_NODRAW | MTL_FLAG_NOSHADOW)) == 0;
			}

			IMaterial* pMaterial;
		};

		struct ShadowChunkIterator
		{
			ShadowChunkIterator(TRenderChunkArray::const_iterator curPos, TRenderChunkArray::const_iterator rangeEnd, IMaterial* pMtl);

			ShadowChunkIterator& operator++();
			const CRenderChunk& operator*()  const                                 { return  (*itCurPos); }
			const CRenderChunk* operator->() const                                 { return &(*itCurPos); }
			bool                operator!=(const ShadowChunkIterator& other) const { return itCurPos != other.itCurPos; }

			TRenderChunkArray::const_iterator itCurPos;
			TRenderChunkArray::const_iterator itEnd;

			IsShadowChunk                     isShadowChunk;
		};

		ShadowChunkRange(IRenderMesh* pRM, IMaterial* pMtl)
			: pRenderMesh(pRM), pMaterial(pMtl) {}

		ShadowChunkIterator begin();
		ShadowChunkIterator end();

		IRenderMesh*        pRenderMesh;
		IMaterial*          pMaterial;
	};

	struct AttachmentLodData
	{
		IRenderMesh*     pMesh;
		IMaterial*       pMaterial;
		Matrix34         mTransform;

		ShadowChunkRange GetShadowChunks() const { return ShadowChunkRange(pMesh, pMaterial); }
	};

	struct AttachmentRenderData
	{
		IAttachment* pAttachment;
		uint         nLodCount;
		uint         nBoneCount;
		StaticArray<AttachmentLodData, MAX_STATOBJ_LODS_NUM> arrLods;

		AttachmentRenderData(IAttachment* pAtt);
		const AttachmentLodData& GetLod(i32 lod) const { return arrLods[lod]; }
	};

	struct MeshStreams
	{
		uk        pPositions;
		uk        pSkinningInfos;
		vtx_idx*     pIndices;

		i32          nPositionStride;
		i32          nSkinningInfoStride;

		IRenderMesh* pMesh;

		MeshStreams(IRenderMesh* pRenderMesh, u32 lockFlags = FSL_READ, bool bGetSkinningInfos = true);
		~MeshStreams();

		template<typename T>
		T& GetVertex(i32 nVtxIndex)
		{
			DRX_ASSERT(nVtxIndex < pMesh->GetVerticesCount());
			return static_cast<T*>(pPositions)[nVtxIndex];
		}

		SVF_W4B_I4S&  GetSkinningInfo(i32 nVtxIndex);
		vtx_idx&      GetVertexIndex(i32 i);
		InputLayoutHandle GetVertexFormat();
	};

	struct MergeContext
	{
		enum TrinaryState { eStateOn = 0, eStateOff = 1, eStateMixed = 2 };

		uint         nAccumulatedBoneCount;
		uint         nAccumulatedVertexCount[MAX_STATOBJ_LODS_NUM];
		uint         nAccumulatedIndexCount[MAX_STATOBJ_LODS_NUM];
		uint         nMaxLodCount;

		TrinaryState nTwoSided;
		bool         bAlphaTested;
		bool         bEightWeightSkinning;

		MergeContext(IAttachment* pAttachment);
		MergeContext(const AttachmentRenderData& renderData);
		void Update(const AttachmentRenderData& renderData);
		uint EstimateRequiredMemory() const;
	};

public:
	CAttachmentMerger()
		: m_nBytesAllocated(0)
		, m_nMergedSkinCount(0)
		, m_bOutOfMemory(false)
	{}

	static CAttachmentMerger& Instance()                                 { static CAttachmentMerger instance; return instance; }

	virtual uint              GetAllocatedBytes() const override         { return m_nBytesAllocated; }
	virtual bool              IsOutOfMemory() const override             { return m_bOutOfMemory; }
	virtual uint              GetMergedAttachmentsCount() const override { return m_nMergedSkinCount; }

	void                      MergeAttachments(DynArray<_smart_ptr<SAttachmentBase>>& arrAttachments, DynArray<_smart_ptr<CAttachmentMerged>>& arrMergedAttachments, CAttachmentUpr* pAttachmentUpr);
	void                      UpdateIndices(CAttachmentMerged* pAttachment, i32 lod, const DynArray<std::pair<u32, u32>>& srcRanges);
	void                      OnDeleteMergedAttachment(CAttachmentMerged* pAttachment);

	static bool               CanMerge(IAttachment* pAttachment1, IAttachment* pAttachment2);

private:
	void              Merge(CAttachmentMerged* pDstAttachment, const DynArray<AttachmentRenderData>& attachmentData, CAttachmentUpr* pAttachmentUpr);

	uint              CopyVertices(MeshStreams& dstStreams, uint dstVtxOffset, MeshStreams& srcStreams, uint numVertices);
	uint              CopyVertices(MeshStreams& dstStreams, uint dstVtxOffset, MeshStreams& srcStreams, uint numVertices, const Matrix34& transform);
	uint              CopyIndices(MeshStreams& dstStreams, uint dstVtxOffset, uint dstIdxOffset, MeshStreams& srcStreams, const CRenderChunk& chunk);
	uint              CopySkinning(MeshStreams& dstStreams, uint dstVtxOffset, MeshStreams& srcStreams, uint numVertices, const DynArray<JointIdType>& boneIDs);
	void              SkinToBone(MeshStreams& dstStreams, uint dstVtxOffset, MeshStreams& srcStreams, uint numVertices, JointIdType jointID);

	static bool       CanMerge(const MergeContext& context1, const MergeContext& context2);
	static IMaterial* GetAttachmentMaterial(const IAttachment* pAttachment, i32 lod);

	uint m_nMergedSkinCount;
	uint m_nBytesAllocated;
	bool m_bOutOfMemory;
};
