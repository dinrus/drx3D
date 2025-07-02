// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Animation/AttachmentUpr.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/SkeletonEffectUpr.h>
#include <drx3D/Animation/SkeletonAnim.h>
#include <drx3D/Animation/SkeletonPose.h>

struct AnimData;
class CCamera;
class CFacialInstance;
class CModelMesh;
class CSkeletonAnim;
class CSkeletonPose;
class DrxCharUpr;
struct DrxParticleSpawnInfo;
namespace CharacterInstanceProcessing {
struct SStartAnimationProcessing;
}

extern f32 g_YLine;

class DRX_ALIGN(128) CCharInstance final : public ICharacterInstance
{
public:
	friend CharacterInstanceProcessing::SStartAnimationProcessing;

	~CCharInstance();

	CCharInstance(const string &strFileName, CDefaultSkeleton * pDefaultSkeleton);

	//////////////////////////////////////////////////////////
	// ICharacterInstance implementation
	//////////////////////////////////////////////////////////
	virtual i32 AddRef() override;
	virtual i32 Release() override;
	virtual i32 GetRefCount() const override;
	virtual ISkeletonAnim*            GetISkeletonAnim() override                 { return &m_SkeletonAnim; }
	virtual const ISkeletonAnim*      GetISkeletonAnim() const override           { return &m_SkeletonAnim; }
	virtual ISkeletonPose*            GetISkeletonPose() override                 { return &m_SkeletonPose; }
	virtual const ISkeletonPose*      GetISkeletonPose() const override           { return &m_SkeletonPose; }
	virtual IAttachmentUpr*       GetIAttachmentUpr() override            { return &m_AttachmentUpr; }
	virtual const IAttachmentUpr* GetIAttachmentUpr() const override      { return &m_AttachmentUpr; }
	virtual IDefaultSkeleton&       GetIDefaultSkeleton() override              { return *m_pDefaultSkeleton; }
	virtual const IDefaultSkeleton& GetIDefaultSkeleton() const override        { return *m_pDefaultSkeleton; }
	virtual IAnimationSet*          GetIAnimationSet() override                 { return m_pDefaultSkeleton->GetIAnimationSet(); }
	virtual const IAnimationSet*    GetIAnimationSet() const override           { return m_pDefaultSkeleton->GetIAnimationSet(); }
	virtual tukk             GetModelAnimEventDatabase() const override  { return m_pDefaultSkeleton->GetModelAnimEventDatabaseCStr(); }
	virtual void                    EnableStartAnimation(bool bEnable) override { m_bEnableStartAnimation = bEnable; }
	virtual void StartAnimationProcessing(const SAnimationProcessParams &params) override;
	virtual AABB                    GetAABB() const override                    { return m_SkeletonPose.GetAABB(); }
	virtual float GetExtent(EGeomForm eForm) override;
	virtual void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const override;
	virtual CLodValue ComputeLod(i32 wantedLod, const SRenderingPassInfo &passInfo) override;
	virtual void Render(const SRendParams &rParams, const SRenderingPassInfo &passInfo) override;
	virtual void                   SetFlags(i32 nFlags) override                        { m_rpFlags = nFlags; }
	virtual i32                    GetFlags() const override                            { return m_rpFlags; }
	virtual i32                    GetObjectType() const override                       { return m_pDefaultSkeleton->m_ObjectType; }
	virtual tukk            GetFilePath() const override                         { return m_strFilePath.c_str(); }
	virtual SMeshLodInfo           ComputeGeometricMean() const override;
	virtual bool                   HasVertexAnimation() const override                  { return m_bHasVertexAnimation; }
	virtual IMaterial*             GetMaterial() const override                         { return GetIMaterial(); }
	virtual IMaterial*             GetIMaterial() const override                        { return m_pInstanceMaterial ? m_pInstanceMaterial.get() : m_pDefaultSkeleton->GetIMaterial(); }
	virtual void                   SetIMaterial_Instance(IMaterial* pMaterial) override { m_pInstanceMaterial = pMaterial; }
	virtual IMaterial*             GetIMaterial_Instance() const override               { return m_pInstanceMaterial; }
	virtual IRenderMesh*           GetRenderMesh() const override                       { return m_pDefaultSkeleton ? m_pDefaultSkeleton->GetIRenderMesh() : nullptr; }
	virtual phys_geometry*         GetPhysGeom(i32 nType) const override;
	virtual IPhysicalEntity*       GetPhysEntity() const override;
	virtual IFacialInstance*       GetFacialInstance() override;
	virtual const IFacialInstance* GetFacialInstance() const override;
	virtual void EnableFacialAnimation(bool bEnable) override;
	virtual void EnableProceduralFacialAnimation(bool bEnable) override;
	virtual void   SetPlaybackScale(f32 speed) override { m_fPlaybackScale = max(0.0f, speed); }
	virtual f32    GetPlaybackScale() const override    { return m_fPlaybackScale; }
	virtual u32 IsCharacterVisible() const override  { return m_SkeletonPose.m_bInstanceVisible; };
	virtual void SpawnSkeletonEffect(const AnimEventInstance& animEvent, const QuatTS &entityLoc) override;
	virtual void KillAllSkeletonEffects() override;
	virtual void  SetViewdir(const Vec3& rViewdir) override                                     { m_Viewdir = rViewdir; }
	virtual float GetUniformScale() const override                                              { return m_location.s; }
	virtual void CopyPoseFrom(const ICharacterInstance &instance) override;
	virtual void  FinishAnimationComputations() override                                        { m_SkeletonAnim.FinishAnimationComputations(); }
	virtual void  SetAttachmentLocation_DEPRECATED(const QuatTS& newCharacterLocation) override { m_location = newCharacterLocation; } // TODO: Resolve this issue (has been described as "This is a hack to keep entity attachments in sync.").
	virtual void OnDetach() override;
	virtual void  HideMaster(u32 h) override                                                 { m_bHideMaster = (h > 0); };
	virtual void GetMemoryUsage(IDrxSizer * pSizer) const override;
	virtual void Serialize(TSerialize ser) override;
#ifdef EDITOR_PCDEBUGCODE
	virtual u32 GetResetMode() const override        { return m_ResetMode; }
	virtual void   SetResetMode(u32 rm) override     { m_ResetMode = rm > 0; }
	virtual f32    GetAverageFrameTime() const override { return g_AverageFrameTime; }
	virtual void   SetCharEditMode(u32 m) override   { m_CharEditMode = m; }
	virtual u32 GetCharEditMode() const override     { return m_CharEditMode; }
	virtual void DrawWireframeStatic(const Matrix34 &m34, i32 nLOD, u32 color) override;
	virtual void ReloadCHRPARAMS() override;
#endif
	//////////////////////////////////////////////////////////

	CharacterInstanceProcessing::SContext* GetProcessingContext();
	void                                   ClearProcessingContext() { m_processingContext = -1; };

	void WaitForSkinningJob() const;

	void RuntimeInit(CDefaultSkeleton * pExtDefaultSkeleton);

	SSkinningData* GetSkinningData(const SRenderingPassInfo& passInfo);

	void           SetFilePath(const string& filePath) { m_strFilePath = filePath; }

	void           SetHasVertexAnimation(bool state)   { m_bHasVertexAnimation = state; }

	void SkinningTransformationsComputation(SSkinningData * pSkinningData, CDefaultSkeleton * pDefaultSkeleton, i32 nRenderLod, const Skeleton::CPoseData * pPoseData, f32 * pSkinningTransformationsMovement);

	void  SetWasVisible(bool wasVisible) { m_bWasVisible = wasVisible; }

	bool  GetWasVisible() const          { return m_bWasVisible; };

	i32 GetAnimationLOD() const        { return m_nAnimationLOD; }

	bool  FacialAnimationEnabled() const { return m_bFacialAnimationEnabled; }

	void SkeletonPostProcess();

	void UpdatePhysicsCGA(Skeleton::CPoseData & poseData, f32 fScale, const QuatTS &rAnimLocationNext);

	void ApplyJointVelocitiesToPhysics(IPhysicalEntity * pent, const Quat& qrot = Quat(IDENTITY), const Vec3& velHost = Vec3(ZERO));

	size_t SizeOfCharInstance() const;

	void RenderCGA(const struct SRendParams& rParams, const Matrix34 &RotTransMatrix34, const SRenderingPassInfo &passInfo);

	void RenderCHR(const struct SRendParams& rParams, const Matrix34 &RotTransMatrix34, const SRenderingPassInfo &passInfo);

	bool MotionBlurMotionCheck(uint64 nObjFlags) const;

	u32 GetSkinningTransformationCount() const { return m_skinningTransformationsCount; }

	void BeginSkinningTransformationsComputation(SSkinningData * pSkinningData);

	void PerFrameUpdate();

private:
	// Functions that are called from Character Instance Processing
	void SetupThroughParent(const CCharInstance * pParent);
	void SetupThroughParams(const SAnimationProcessParams * pParams);

	CCharInstance(const CCharInstance &) /*= delete*/;
	void operator=(const CCharInstance&) /*= delete*/;

public:

	CAttachmentUpr m_AttachmentUpr;

	CSkeletonEffectUpr m_skeletonEffectUpr;

	CSkeletonAnim m_SkeletonAnim;

	CSkeletonPose m_SkeletonPose;

	QuatTS m_location;
	Vec3 m_Viewdir;

	string m_strFilePath;

	struct
	{
		SSkinningData* pSkinningData;
		i32            nFrameID;
	} arrSkinningRendererData[3];                                                      // Triple buffered for motion blur. (These resources are owned by the renderer)

	_smart_ptr<CDefaultSkeleton> m_pDefaultSkeleton;

	_smart_ptr<CFacialInstance> m_pFacialInstance;

	u32 m_CharEditMode;

	u32 m_rpFlags;

	u32 m_LastUpdateFrameID_Pre;
	u32 m_LastUpdateFrameID_Post;

	float m_fPostProcessZoomAdjustedDistanceFromCamera;

	f32 m_fDeltaTime;
	f32 m_fOriginalDeltaTime;

	// This is the scale factor that affects the animation speed of the character.
	// All the animations are played with the constant real-time speed multiplied by this factor.
	// So, 0 means still animations (stuck at some frame), 1 - normal, 2 - twice as fast, 0.5 - twice slower than normal.
	f32 m_fPlaybackScale;

	bool m_bEnableStartAnimation;

private:

	_smart_ptr<IMaterial> m_pInstanceMaterial;

	CGeomExtents m_Extents;

	i32 m_nRefCounter;

	u32 m_LastRenderedFrameID;     // Can be accessed from main thread only!

	i32 m_nAnimationLOD;

	u32 m_ResetMode;

	u32 m_skinningTransformationsCount;

	f32 m_skinningTransformationsMovement;

	f32 m_fZoomDistanceSq;

	bool m_bHasVertexAnimation;

	bool m_bWasVisible;

	bool m_bPlayedPhysAnim;

	bool m_bHideMaster;

	bool m_bFacialAnimationEnabled;

	i32 m_processingContext;

};

inline i32 CCharInstance::AddRef()
{
	return ++m_nRefCounter;
}

inline i32 CCharInstance::GetRefCount() const
{
	return m_nRefCounter;
}

inline i32 CCharInstance::Release()
{
	if (--m_nRefCounter == 0)
	{
		m_AttachmentUpr.RemoveAllAttachments();
		delete this;
		return 0;
	}
	else if (m_nRefCounter < 0)
	{
		// Should never happens, someone tries to release CharacterInstance pointer twice.
		assert(0 && "CSkelInstance::Release");
		DrxFatalError("CSkelInstance::Release");
	}
	return m_nRefCounter;
}

inline void CCharInstance::SpawnSkeletonEffect(const AnimEventInstance& animEvent, const QuatTS &entityLoc)
{
	m_skeletonEffectUpr.SpawnEffect(this, animEvent, entityLoc);
}

inline void CCharInstance::KillAllSkeletonEffects()
{
	m_skeletonEffectUpr.KillAllEffects();
}
