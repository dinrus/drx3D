// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/AttachmentBase.h>
#include <drx3D/Animation/VertexData.h>
#include <drx3D/Animation/VertexAnimation.h>
#include <drx3D/Animation/ModelSkin.h>

class CModelMesh;
struct SVertexAnimationJob;

//////////////////////////////////////////////////////////////////////
// Особое значение для указания на то, что работа по Skinning Transformation
// (трансформации скининга) всё ещё выполняется.
#define SKINNING_TRANSFORMATION_RUNNING_INDICATOR (reinterpret_cast<SSkinningData*>(static_cast<intptr_t>(-1)))

class CAttachmentSKIN : public IAttachmentSkin, public SAttachmentBase
{
public:

	CAttachmentSKIN()
	{
		for (u32 j = 0; j < 2; ++j) m_pRenderMeshsSW[j] = NULL;
		memset(m_arrSkinningRendererData, 0, sizeof(m_arrSkinningRendererData));
		m_pModelSkin = 0;
	};

	~CAttachmentSKIN();

	virtual void AddRef() override
	{
		++m_nRefCounter;
	}

	virtual void Release() override
	{
		if (--m_nRefCounter == 0)
			delete this;
	}

	virtual u32             GetType() const override                               { return CA_SKIN; }
	virtual u32             SetJointName(tukk szJointName) override         { return 0; }

	virtual tukk        GetName() const override                               { return m_strSocketName; };
	virtual u32             GetNameCRC() const override                            { return m_nSocketCRC32; }
	virtual u32             ReName(tukk strSocketName, u32 crc) override { m_strSocketName.clear();  m_strSocketName = strSocketName; m_nSocketCRC32 = crc;  return 1; };

	virtual u32             GetFlags() const override                              { return m_AttFlags; }
	virtual void               SetFlags(u32 flags) override                        { m_AttFlags = flags; }

	void                       ReleaseRemapTablePair();
	void                       ReleaseSoftwareRenderMeshes();

	virtual u32             Immediate_AddBinding(IAttachmentObject* pModel, ISkin* pISkin = 0, u32 nLoadingFlags = 0) override;
	virtual void               Immediate_ClearBinding(u32 nLoadingFlags = 0) override;
	virtual u32             Immediate_SwapBinding(IAttachment* pNewAttachment) override;

	virtual IAttachmentObject* GetIAttachmentObject() const override { return m_pIAttachmentObject; }
	virtual IAttachmentSkin*   GetIAttachmentSkin() override         { return this; }

	virtual void               HideAttachment(u32 x) override;
	virtual u32             IsAttachmentHidden() const override             { return m_AttFlags & FLAGS_ATTACH_HIDE_MAIN_PASS; }
	virtual void               HideInRecursion(u32 x) override;
	virtual u32             IsAttachmentHiddenInRecursion() const override  { return m_AttFlags & FLAGS_ATTACH_HIDE_RECURSION; }
	virtual void               HideInShadow(u32 x) override;
	virtual u32             IsAttachmentHiddenInShadow() const override     { return m_AttFlags & FLAGS_ATTACH_HIDE_SHADOW_PASS; }

	virtual void               SetAttAbsoluteDefault(const QuatT& qt) override {};
	virtual void               SetAttRelativeDefault(const QuatT& qt) override {};
	virtual const QuatT& GetAttAbsoluteDefault() const override          { return g_IdentityQuatT; };
	virtual const QuatT& GetAttRelativeDefault() const override          { return g_IdentityQuatT; };

	virtual const QuatT& GetAttModelRelative() const override            { return g_IdentityQuatT;  };//this is relative to the animated bone
	virtual const QuatTS GetAttWorldAbsolute() const override;
	virtual const QuatT& GetAdditionalTransformation() const override    { return g_IdentityQuatT; }
	virtual void         UpdateAttModelRelative() override;

	virtual u32       GetJointID() const override     { return -1; };
	virtual void         AlignJointAttachment() override {};

	virtual void         Serialize(TSerialize ser) override;
	virtual size_t       SizeOfThis() const override;
	virtual void         GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual void         TriggerMeshStreaming(u32 nDesiredRenderLOD, const SRenderingPassInfo& passInfo);// override;

	void                 DrawAttachment(SRendParams& rParams, const SRenderingPassInfo& passInfo, const Matrix34& rWorldMat34, f32 fZoomFactor = 1);
	void                 RecreateDefaultSkeleton(CCharInstance* pInstanceSkel, u32 nLoadingFlags);
	void                 UpdateRemapTable();

	// Трансформация Вершин (Vertex Transformation)
public:
	SSkinningData*          GetVertexTransformationData(bool useSwSkinningCpu, u8 nRenderLOD, const SRenderingPassInfo& passInfo);
	bool                    ShouldSwSkin() const     { return (m_AttFlags & FLAGS_ATTACH_SW_SKINNING) != 0; }
	bool                    ShouldSkinLinear() const { return (m_AttFlags & FLAGS_ATTACH_LINEAR_SKINNING) != 0; }
	_smart_ptr<IRenderMesh> CreateVertexAnimationRenderMesh(uint lod, uint id);
	void                    CullVertexFrames(const SRenderingPassInfo& passInfo, float fDistance);

#ifdef EDITOR_PCDEBUGCODE
	void DrawWireframeStatic(const Matrix34& m34, i32 nLOD, u32 color) override;
	void SoftwareSkinningDQ_VS_Emulator(CModelMesh* pModelMesh, Matrix34 rRenderMat34, u8 tang, u8 binorm, u8 norm, u8 wire, const DualQuat* const pSkinningTransformations);
#endif

	//////////////////////////////////////////////////////////////////////////
	//Реализация IAttachmentSkin
	//////////////////////////////////////////////////////////////////////////
	virtual IVertexAnimation* GetIVertexAnimation() override { return &m_vertexAnimation; }
	virtual ISkin*            GetISkin() override            { return m_pModelSkin; };
	virtual float             GetExtent(EGeomForm eForm) override;
	virtual void              GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const override;
	virtual SMeshLodInfo      ComputeGeometricMean() const override;

	i32                       GetGuid() const;

	//---------------------------------------------------------

	//just for skin-attachments
	DynArray<JointIdType>   m_arrRemapTable; // maps skin's bone indices to skeleton's bone indices
	_smart_ptr<CSkin>       m_pModelSkin;
	_smart_ptr<IRenderMesh> m_pRenderMeshsSW[2];
	string                  m_sSoftwareMeshName;
	CVertexData             m_vertexData;
	CVertexAnimation        m_vertexAnimation;
	// history for skinning data, needed for motion blur
	struct { SSkinningData* pSkinningData; i32 nFrameID; } m_arrSkinningRendererData[3]; // triple buffered for motion blur

private:
	// function to keep in sync ref counts on skins and cleanup of remap tables
	void ReleaseModelSkin();

};
