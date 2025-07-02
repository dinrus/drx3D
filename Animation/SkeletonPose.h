// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/Skeleton.h>
#include <drx3D/Animation/ModelAnimationSet.h>
#include <drx3D/CoreX/Math/GeomQuery.h>
#include <drx3D/Animation/Pool.h>
#include <drx3D/Animation/SkeletonPhysics.h>

class CDefaultSkeleton;
class CCharInstance;
class CSkeletonAnim;
class CSkeletonPose;
struct Locator;

struct CCGAJoint
{
	CCGAJoint() : m_CGAObjectInstance(0), m_qqqhasPhysics(~0), m_pMaterial(0) {}
	~CCGAJoint()
	{
	};
	void GetMemoryUsage(IDrxSizer* pSizer) const {}

	_smart_ptr<IStatObj>  m_CGAObjectInstance; //Static object controlled by this joint (this can be different for every instance).
	i32                   m_qqqhasPhysics;     //>=0 if have physics (don't make it i16!!)
	_smart_ptr<IMaterial> m_pMaterial;         // custom material override
};

struct SPostProcess
{
	QuatT  m_RelativePose;
	u32 m_JointIdx;

	SPostProcess(){};

	SPostProcess(i32 idx, const QuatT& qp)
	{
		m_JointIdx = idx;
		m_RelativePose = qp;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const {}
};

#include <drx3D/Animation/DirectionalBlender.h>
#include <drx3D/Animation/PoseBlenderAim.h>
#include <drx3D/Animation/PoseBlenderLook.h>
#include <drx3D/Animation/FeetLock.h>

class DRX_ALIGN(128) CSkeletonPose: public ISkeletonPose
{

public:
	CSkeletonPose();
	~CSkeletonPose();

	size_t SizeOfThis();
	void GetMemoryUsage(IDrxSizer * pSizer) const;

	//-----------------------------------------------------------------------------------
	//---- interface functions to access bones ----
	//-----------------------------------------------------------------------------------
	virtual const QuatT& GetAbsJointByID(i32 index) const override
	{
		const Skeleton::CPoseData& poseData = GetPoseData();
		if (index >= 0 && (u32)index < poseData.GetJointCount())
		{
			return poseData.GetJointAbsolute(index);
		}
		else
		{
			assert(false);
			return g_IdentityQuatT;
		}
	}

	virtual const QuatT& GetRelJointByID(i32 index) const override
	{
		const Skeleton::CPoseData& poseData = GetPoseData();
		if (index >= 0 && (u32)index < poseData.GetJointCount())
		{
			return poseData.GetJointRelative(index);
		}
		else
		{
			assert(false);
			return g_IdentityQuatT;
		}
	}

	virtual Diag33 GetAbsJointScalingByID(i32 index) const override
	{
		const Skeleton::CPoseData& poseData = GetPoseData();
		if (index >= 0 && (u32)index < poseData.GetJointCount())
		{
			return poseData.GetJointAbsoluteS(index);
		}
		else
		{
			assert(false);
			return Diag33(1.0);
		}
	}

	virtual Diag33 GetRelJointScalingByID(i32 index) const override
	{
		const Skeleton::CPoseData& poseData = GetPoseData();
		if (index >= 0 && (u32)index < poseData.GetJointCount())
		{
			return poseData.GetJointRelativeS(index);
		}
		else
		{
			assert(false);
			return Diag33(1.0);
		}
	}

	void InitSkeletonPose(CCharInstance * pInstance, CSkeletonAnim * pSkeletonAnim);
	void InitCGASkeleton();

	void SetDefaultPosePerInstance(bool bDataPoseForceWriteable);
	void SetDefaultPoseExecute(bool bDataPoseForceWriteable);
	void SetDefaultPose() override;

	//------------------------------------------------------------------------------------
	void SkeletonPostProcess(Skeleton::CPoseData & poseData);

	i32 m_nForceSkeletonUpdate;
	void SetForceSkeletonUpdate(i32 i) override { m_nForceSkeletonUpdate = i; };

	void UpdateBBox(u32 update = 0);

public:

	// -------------------------------------------------------------------------
	// Implicit Pose Modifiers
	// -------------------------------------------------------------------------
	// LimbIk
	u32 SetHumanLimbIK(const Vec3 &vWorldPos, tukk strLimb) override;
	IAnimationPoseModifierPtr m_limbIk;

	// Recoil
	void ApplyRecoilAnimation(f32 fDuration, f32 fImpact, f32 fKickIn, u32 arms = 3) override;
	IAnimationPoseModifierPtr m_recoil;

	// FeetLock
	CFeetLock m_feetLock;

	// Look and Aim
	IAnimationPoseBlenderDirPtr m_PoseBlenderLook;
	IAnimationPoseBlenderDir*       GetIPoseBlenderLook() override       { return m_PoseBlenderLook.get(); }
	const IAnimationPoseBlenderDir* GetIPoseBlenderLook() const override { return m_PoseBlenderLook.get(); }

	IAnimationPoseBlenderDirPtr m_PoseBlenderAim;
	IAnimationPoseBlenderDir*       GetIPoseBlenderAim() override       { return m_PoseBlenderAim.get(); }
	const IAnimationPoseBlenderDir* GetIPoseBlenderAim() const override { return m_PoseBlenderAim.get(); }

public:
	i32 (* m_pPostProcessCallback)(ICharacterInstance*, uk );
	uk m_pPostProcessCallbackData;
	void SetPostProcessCallback(CallBackFuncType func, uk pdata) override
	{
		m_pPostProcessCallback = func;
		m_pPostProcessCallbackData = pdata;
	}

	IStatObj*       GetStatObjOnJoint(i32 nId) override;
	const IStatObj* GetStatObjOnJoint(i32 nId) const override;

	void SetStatObjOnJoint(i32 nId, IStatObj * pStatObj) override;
	void SetMaterialOnJoint(i32 nId, IMaterial* pMaterial)  override
	{
		i32 numJoints = i32(m_arrCGAJoints.size());
		if (nId >= 0 && nId < numJoints)
			m_arrCGAJoints[nId].m_pMaterial = pMaterial;
	}
	const IMaterial* GetMaterialOnJoint(i32 nId) const override
	{
		i32 numJoints = i32(m_arrCGAJoints.size());
		if (nId >= 0 && nId < numJoints)
			return m_arrCGAJoints[nId].m_pMaterial;
		return 0;
	}
	IMaterial* GetMaterialOnJoint(i32 nId) override
	{
		const CSkeletonPose* pConstThis = this;
		return const_cast<IMaterial*>(pConstThis->GetMaterialOnJoint(nId));
	}

	i32 GetInstanceVisible() const { return m_bFullSkeletonUpdate; };

	//just for debugging
	void DrawPose(const Skeleton::CPoseData & pose, const Matrix34 &rRenderMat34);
	void DrawBBox(const Matrix34 &rRenderMat34);
	void DrawSkeleton(const Matrix34 &rRenderMat34, u32 shift = 0) override;
	f32 SecurityCheck();
	u32 IsSkeletonValid();
	const AABB& GetAABB() const { return m_AABB; }
	void ExportSkeleton();

	// -------------------------------------------------------------------------
	// Physics
	// -------------------------------------------------------------------------
	IPhysicalEntity*         GetPhysEntOnJoint(i32 nId) override                                                                                                                                                  { return m_physics.GetPhysEntOnJoint(nId); }
	const IPhysicalEntity*   GetPhysEntOnJoint(i32 nId) const override                                                                                                                                            { return m_physics.GetPhysEntOnJoint(nId); }
	void                     SetPhysEntOnJoint(i32 nId, IPhysicalEntity* pPhysEnt) override                                                                                                                       { m_physics.SetPhysEntOnJoint(nId, pPhysEnt); }
	i32                      GetPhysIdOnJoint(i32 nId) const override                                                                                                                                             { return m_physics.GetPhysIdOnJoint(nId); }
	void                     BuildPhysicalEntity(IPhysicalEntity* pent, f32 mass, i32 surface_idx, f32 stiffness_scale = 1.0f, i32 nLod = 0, i32 partid0 = 0, const Matrix34& mtxloc = Matrix34(IDENTITY)) override { m_physics.BuildPhysicalEntity(pent, mass, surface_idx, stiffness_scale, nLod, partid0, mtxloc); }
	IPhysicalEntity*         CreateCharacterPhysics(IPhysicalEntity* pHost, f32 mass, i32 surface_idx, f32 stiffness_scale, i32 nLod = 0, const Matrix34& mtxloc = Matrix34(IDENTITY)) override                     { return m_physics.CreateCharacterPhysics(pHost, mass, surface_idx, stiffness_scale, nLod, mtxloc); }
	i32                      CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, i32 nLod = 0) override                                                                                              { return m_physics.CreateAuxilaryPhysics(pHost, mtx, nLod); }
	IPhysicalEntity*         GetCharacterPhysics() const override                                                                                                                                                   { return m_physics.GetCharacterPhysics(); }
	IPhysicalEntity*         GetCharacterPhysics(tukk pRootBoneName) const override                                                                                                                          { return m_physics.GetCharacterPhysics(pRootBoneName); }
	IPhysicalEntity*         GetCharacterPhysics(i32 iAuxPhys) const override                                                                                                                                       { return m_physics.GetCharacterPhysics(iAuxPhys); }
	void                     SetCharacterPhysics(IPhysicalEntity* pent) override                                                                                                                                    { m_physics.SetCharacterPhysics(pent); }
	void                     SynchronizeWithPhysicalEntity(IPhysicalEntity* pent, const Vec3& posMaster = Vec3(ZERO), const Quat& qMaster = Quat(1, 0, 0, 0)) override                                              { m_physics.SynchronizeWithPhysicalEntity(pent, posMaster, qMaster); }
	IPhysicalEntity*         RelinquishCharacterPhysics(const Matrix34& mtx, f32 stiffness = 0.0f, bool bCopyJointVelocities = false, const Vec3& velHost = Vec3(ZERO)) override                                    { return m_physics.RelinquishCharacterPhysics(mtx, stiffness, bCopyJointVelocities, velHost); }
	void                     DestroyCharacterPhysics(i32 iMode = 0) override                                                                                                                                        { m_physics.DestroyCharacterPhysics(iMode); }
	bool                     AddImpact(i32 partid, Vec3 point, Vec3 impact) override                                                                                                                                { return m_physics.AddImpact(partid, point, impact); }
	i32                      TranslatePartIdToDeadBody(i32 partid) override                                                                                                                                         { return m_physics.TranslatePartIdToDeadBody(partid); }
	i32                      GetAuxPhysicsBoneId(i32 iAuxPhys, i32 iBone = 0) const override                                                                                                                        { return m_physics.GetAuxPhysicsBoneId(iAuxPhys, iBone); }
	bool                     BlendFromRagdoll(QuatTS& location, IPhysicalEntity*& pPhysicalEntity, bool b3dof) override                                                                                             { return m_physics.BlendFromRagdoll(location, pPhysicalEntity); }
	i32                      GetFallingDir() const override                                                                                                                                                         { return m_physics.GetFallingDir(); }
	i32                      getBonePhysParentOrSelfIndex(i32 nBoneIndex, i32 nLod = 0) const override                                                                                                              { return m_physics.getBonePhysParentOrSelfIndex(nBoneIndex, nLod = 0); }
	i32                      GetBoneSurfaceTypeId(i32 nBoneIndex, i32 nLod = 0) const override                                                                                                                      { return m_physics.GetBoneSurfaceTypeId(nBoneIndex, nLod); }
	DynArray<SJointProperty> GetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod) const override                                                                                                                { return m_physics.GetJointPhysProperties_ROPE(jointIndex, nLod); }
	bool                     SetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod, const DynArray<SJointProperty>& props) override                                                                               { return m_physics.SetJointPhysProperties_ROPE(jointIndex, nLod, props); }

	float GetExtent(EGeomForm eForm);
	void GetRandomPoints(Array<PosNorm> points, CRndGen& seed, EGeomForm eForm) const;

	CSkeletonPhysics m_physics;

public:
	CCharInstance* m_pInstance;

public:
	ILINE const Skeleton::CPoseData& GetPoseData() const            { return m_poseData; }
	ILINE Skeleton::CPoseData&       GetPoseDataExplicitWriteable() { return m_poseData; }
	Skeleton::CPoseData&             GetPoseDataForceWriteable();
	Skeleton::CPoseData*             GetPoseDataWriteable();
	ILINE const Skeleton::CPoseData& GetPoseDataDefault() const { return *m_pPoseDataDefault; }

	bool PreparePoseDataAndLocatorWriteables(Memory::CPool & memoryPool);
	void SynchronizePoseDataAndLocatorWriteables();

	//private:
	f32 m_fDisplaceRadiant;
	Skeleton::CPoseData m_poseData;
	Skeleton::CPoseData m_poseDataWriteable;
	Skeleton::CPoseData* m_pPoseDataWriteable;
	Skeleton::CPoseData* m_pPoseDataDefault;

public:
	DynArray<Vec3> m_FaceAnimPosSmooth;
	DynArray<Vec3> m_FaceAnimPosSmoothRate;
	DynArray<CCGAJoint> m_arrCGAJoints;
	CGeomExtents m_Extents;
	AABB m_AABB;

	bool m_bInstanceVisible : 1;
	bool m_bFullSkeletonUpdate : 1;
	u32 m_bAllNodesValid : 1; //True if this animation was already played once.
	bool m_bSetDefaultPoseExecute : 1;

	CSkeletonAnim* m_pSkeletonAnim;
};
