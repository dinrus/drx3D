// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/Memory.h>
#include <drx3D/Animation/Pool.h>
#include <drx3D/Animation/Model.h>
#include <drx3D/Animation/Skeleton.h>

struct CCGAJoint;
class CSkeletonPose;
class CSkeletonAnim;
class CCharInstance;

class CSkeletonPhysicsNull
{
public:
	CSkeletonPhysicsNull()
	{
		m_pCharPhysics = NULL;
		m_ppBonePhysics = NULL;
		m_timeStandingUp = 0.0f;
		;

		m_bHasPhysics = false;
		m_bHasPhysicsProxies = false;
		m_bPhysicsAwake = false;
		m_bPhysicsWasAwake = false;
		;
		m_bPhysicsRelinquished = false;
	}

public:
	IPhysicalEntity*          GetPhysEntOnJoint(i32)                                                                                                                                     { return NULL; }
	const IPhysicalEntity*    GetPhysEntOnJoint(i32) const                                                                                                                               { return NULL; }
	void                      SetPhysEntOnJoint(i32, IPhysicalEntity*)                                                                                                                   {}
	i32                       GetPhysIdOnJoint(i32) const                                                                                                                                { return 0; }
	void                      BuildPhysicalEntity(IPhysicalEntity*, f32, i32, f32, i32, i32, const Matrix34&)                                                                              {}
	IPhysicalEntity*          CreateCharacterPhysics(IPhysicalEntity*, f32, i32, f32, i32, const Matrix34&)                                                                                { return NULL; }
	i32                       CreateAuxilaryPhysics(IPhysicalEntity*, const Matrix34&, i32 lod = 0)                                                                                        { return 0; }
	IPhysicalEntity*          GetCharacterPhysics(i32) const                                                                                                                               { return NULL; }
	IPhysicalEntity*          GetCharacterPhysics(tukk) const                                                                                                                       { return NULL; }
	IPhysicalEntity*          GetCharacterPhysics(void) const                                                                                                                              { return NULL; }
	void                      SetCharacterPhysics(IPhysicalEntity*)                                                                                                                        {}
	void                      SynchronizeWithPhysicalEntity(IPhysicalEntity*, const Vec3&, const Quat&)                                                                                    {}
	IPhysicalEntity*          RelinquishCharacterPhysics(const Matrix34&, f32, bool, const Vec3&)                                                                                          { return NULL; }
	void                      DestroyCharacterPhysics(i32)                                                                                                                                 {}
	bool                      AddImpact(i32, Vec3, Vec3)                                                                                                                                   { return false; }
	i32                       TranslatePartIdToDeadBody(i32)                                                                                                                               { return 0; }
	i32                       GetAuxPhysicsBoneId(i32, i32) const                                                                                                                          { return 0; }
	bool                      BlendFromRagdoll(QuatTS& location, IPhysicalEntity*& pPhysicalEntity)                                                                                        { return false; }
	i32                       GetFallingDir(void) const                                                                                                                                    { return 0; }
	i32                       getBonePhysParentOrSelfIndex(i32, i32) const                                                                                                                 { return 0; }
	i32                       GetBoneSurfaceTypeId(i32, i32) const                                                                                                                         { return 0; }

	bool                      Initialize(CSkeletonPose& skeletonPose)                                                                                                                      { return true; }
	i32                       getBonePhysParentIndex(i32 nBoneIndex, i32 nLod = 0)                                                                                                         { return 0; }
	CDefaultSkeleton::SJoint* GetModelJointPointer(i32 nBone)                                                                                                                              { return NULL; }
	Vec3                      GetOffset()                                                                                                                                                  { return Vec3(0.0f, 0.0f, 0.0f); }

	void                      InitPhysicsSkeleton()                                                                                                                                        {}
	void                      InitializeAnimToPhysIndexArray()                                                                                                                             {}
	i32                       CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, f32 scale, Vec3 offset, i32 nLod)                                                         { return 0; }
	i32                       FillRopeLenArray(float* arr, i32 i0, i32 sz)                                                                                                                 { return 0; }
	void                      DestroyPhysics()                                                                                                                                             {}
	void                      SetAuxParams(pe_params* pf)                                                                                                                                  {}

	void                      SynchronizeWithPhysics(Skeleton::CPoseData& poseData)                                                                                                        {}
	void                      SynchronizeWithPhysicalEntity(Skeleton::CPoseData& poseData, IPhysicalEntity* pent, const Vec3& posMaster, const Quat& qMaster, QuatT offset, i32 iDir = -1) {}
	void                      SynchronizeWithPhysicsPost()                                                                                                                                 {}

public:
	IPhysicalEntity*  m_pCharPhysics;
	IPhysicalEntity** m_ppBonePhysics;
	float             m_timeStandingUp;

	bool              m_bHasPhysics          : 1;
	bool              m_bHasPhysicsProxies   : 1;
	bool              m_bPhysicsAwake        : 1;
	bool              m_bPhysicsWasAwake     : 1;
	bool              m_bPhysicsRelinquished : 1;
};

struct CPhysicsJoint
{
	CPhysicsJoint() :
		m_DefaultRelativeQuat(IDENTITY),
		m_qRelPhysParent(IDENTITY),
		m_qRelFallPlay(IDENTITY)
	{
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const {}

	QuatT m_DefaultRelativeQuat;  //default relative joint (can be different for every instance)
	Quat  m_qRelPhysParent;       // default orientation relative to the physicalized parent
	Quat  m_qRelFallPlay;
};

struct aux_bone_info
{
	quaternionf quat0;
	Vec3        dir0;
	i32         iBone;
	f32         rlen0;
};

struct aux_phys_data
{
	IPhysicalEntity* pPhysEnt;
	Vec3*            pVtx;
	Vec3*            pSubVtx;
	tukk      strName;
	aux_bone_info*   pauxBoneInfo;
	i32              nChars;
	i32              nBones;
	i32              iBoneTiedTo[2];
	i32              nSubVtxAlloc;
	bool             bPhysical;
	bool             bTied0, bTied1;
	i32              iBoneStiffnessController;

	void             GetMemoryUsage(IDrxSizer* pSizer) const {}
};

struct SBatchUpdateValidator : pe_action_batch_parts_update::Validator
{
	SBatchUpdateValidator()
	{
		bValid = 1;
		nRefCount = 1;
		lock = 0;
		WriteLock glock(g_lockList);
		next = prev = &g_firstValidator;
		next = g_firstValidator.next;
		g_firstValidator.next->prev = this;
		g_firstValidator.next = this;
	}
	~SBatchUpdateValidator() { WriteLock glock(g_lockList); prev->next = next; next->prev = prev; }
	i32                             bValid;
	i32                             nRefCount;
	 i32                    lock;
	 SBatchUpdateValidator* next, * prev;
	static SBatchUpdateValidator    g_firstValidator;
	static  i32             g_lockList;

	virtual bool Lock()
	{
		if (!bValid) { Release(); return false; }
		DrxReadLock(&lock);
		return true;
	}
	virtual void Unlock()  { DrxReleaseReadLock(&lock); Release(); }
	i32          AddRef()  { return DrxInterlockedIncrement(&nRefCount); }
	void         Release() { if (DrxInterlockedDecrement(&nRefCount) <= 0) delete this; }
};

class CSkeletonPhysics
{
public:
	CSkeletonPhysics();
	~CSkeletonPhysics();

public:
	bool Initialize(CSkeletonPose& skeletonPose);

	// Helper
public:
	IPhysicalEntity* GetCharacterPhysics() const
	{
		return m_pCharPhysics;
	}
	IPhysicalEntity* GetCharacterPhysics(tukk pRootBoneName) const;
	IPhysicalEntity* GetCharacterPhysics(i32 iAuxPhys) const;
	void             SetCharacterPhysics(IPhysicalEntity* pent)
	{
		m_pCharPhysics = pent;
		m_timeStandingUp = -1.0f;
	}
	i32                             getBonePhysParentIndex(i32 nBoneIndex, i32 nLod = 0);

	CDefaultSkeleton::SJoint*       GetModelJointPointer(i32 nBone);
	const CDefaultSkeleton::SJoint* GetModelJointPointer(i32 nBone) const;

	Vec3                            GetOffset()                        { return m_vOffset; }

	IPhysicalEntity*                GetPhysEntOnJoint(i32 nId)       { return m_ppBonePhysics ? m_ppBonePhysics[nId] : 0; }
	const IPhysicalEntity*          GetPhysEntOnJoint(i32 nId) const { return const_cast<CSkeletonPhysics*>(this)->GetPhysEntOnJoint(nId); }
	void                            SetPhysEntOnJoint(i32 nId, IPhysicalEntity* pPhysEnt);
	i32                             GetPhysIdOnJoint(i32 nId) const;
	i32                             GetAuxPhysicsBoneId(i32 iAuxPhys, i32 iBone = 0) const
	{
		PREFAST_SUPPRESS_WARNING(6385)
		return (iAuxPhys < m_nAuxPhys && iBone < m_auxPhys[iAuxPhys].nBones) ? m_auxPhys[iAuxPhys].pauxBoneInfo[iBone].iBone : -1;
	}
	i32 TranslatePartIdToDeadBody(i32 partid);
	i32 getBonePhysParentOrSelfIndex(i32 nBoneIndex, i32 nLod = 0) const;
	i32 GetBoneSurfaceTypeId(i32 nBoneIndex, i32 nLod) const;

private:
	i32    getBonePhysChildIndex(i32 nBoneIndex, i32 nLod = 0) const;
	u32 getBoneParentIndex(u32 nBoneIndex) const;
	i32    GetPhysRoot() const;

	i32    GetModelJointChildIndex(i32 nBone, i32 i) const
	{
		i32 numChildren = i32(GetModelJointPointer(nBone)->m_numChildren);
		assert(i >= 0 && i < numChildren);
		return nBone + GetModelJointPointer(nBone)->m_nOffsetChildren + i;
	}
	i32  GetPhysicsLod() const { return m_bHasPhysicsProxies ? 1 : 0; }

	void ResetNonphysicalBoneRotations(Skeleton::CPoseData& poseData, i32 nLod, float fBlend);
	void UnconvertBoneGlobalFromRelativeForm(Skeleton::CPoseData& poseData, bool bNonphysicalOnly, i32 nLod = 0, bool bRopeTipsOnly = false);

	void FindSpineBones() const;

	void ForceReskin();

	void SetOffset(Vec3 offset) { m_vOffset = offset; }

	// Initialization/creation
public:
	void                     InitPhysicsSkeleton();
	void                     InitializeAnimToPhysIndexArray();
	i32                      CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, i32 nLod = 0);
	i32                      CreateAuxilaryPhysics(IPhysicalEntity* pHost, const Matrix34& mtx, f32 scale, Vec3 offset, i32 nLod);
	i32                      FillRopeLenArray(float* arr, i32 i0, i32 sz);
	void                     DestroyPhysics();
	void                     SetAuxParams(pe_params* pf);
	void                     BuildPhysicalEntity(IPhysicalEntity* pent, f32 mass, i32 surface_idx, f32 stiffness_scale, i32 nLod = 0, i32 partid0 = -1, const Matrix34& mtxloc = Matrix34(QuatT(IDENTITY)));
	IPhysicalEntity*         CreateCharacterPhysics(IPhysicalEntity* pHost, f32 mass, i32 surface_idx, f32 stiffness_scale, i32 nLod = 0, const Matrix34& mtxloc = Matrix34(QuatT(IDENTITY)));
	IPhysicalEntity*         RelinquishCharacterPhysics(const Matrix34& mtx, float stiffness, bool bCopyJointVelocities, const Vec3& velHost);
	void                     DestroyCharacterPhysics(i32 iMode = 0);
	bool                     AddImpact(i32 partid, Vec3 point, Vec3 impact);
	void                     RequestForcedPostSynchronization();

	void                     SetJointPhysInfo(u32 iJoint, const DrxBonePhysics& pi, i32 nLod);
	const DrxBonePhysics&    GetJointPhysInfo(u32 iJoint, i32 nLod) const;
	DynArray<SJointProperty> GetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod) const;
	bool                     SetJointPhysProperties_ROPE(u32 jointIndex, i32 nLod, const DynArray<SJointProperty>& props);

	void                     SetLocation(const QuatTS& location) { m_location = location; }

private:
	void CreateRagdollDefaultPose(Skeleton::CPoseData& poseData);

public:
	bool BlendFromRagdoll(QuatTS& location, IPhysicalEntity*& pPhysicalEntity);
	i32  GetFallingDir() const;

	// Execution
public:
	void Job_SynchronizeWithPhysicsPrepare(Memory::CPool& memoryPool);
	void Job_Physics_SynchronizeFrom(Skeleton::CPoseData& poseData, float timeDelta);

	void SynchronizeWithPhysics(Skeleton::CPoseData& poseData);
	void SynchronizeWithPhysicalEntity(IPhysicalEntity* pent, const Vec3& posMaster, const Quat& qMaster);
	void SynchronizeWithPhysicalEntity(Skeleton::CPoseData& poseData, IPhysicalEntity* pPhysicalEntity, const Vec3& posMaster, const Quat& qMaster, QuatT offset, i32 iDir = -1);
	void SynchronizeWithPhysicsPost();

private:
	void ProcessPhysics(Skeleton::CPoseData& poseData, float timeDelta);

	void Physics_SynchronizeToAux(const Skeleton::CPoseData& poseData);
	void Physics_SynchronizeToEntity(IPhysicalEntity& physicalEntity, QuatT offset);
	void Physics_SynchronizeToEntityArticulated(float fDeltaTimePhys);
	void Physics_SynchronizeToImpact(float timeDelta);

	void Job_Physics_SynchronizeFromEntityPrepare(Memory::CPool& memoryPool, IPhysicalEntity* pPhysicalEntity);
	void Job_Physics_SynchronizeFromEntity(Skeleton::CPoseData& poseData, IPhysicalEntity* pPhysicalEntity, QuatT offset);
	void Job_Physics_SynchronizeFromEntityArticulated(Skeleton::CPoseData& poseData, float timeDelta);
	void Job_Physics_SynchronizeFromAuxPrepare(Memory::CPool& memoryPool);
	void Job_Physics_SynchronizeFromAux(Skeleton::CPoseData& poseData);
	void Job_Physics_SynchronizeFromImpactPrepare(Memory::CPool& memoryPool);
	void Job_Physics_SynchronizeFromImpact(Skeleton::CPoseData& poseData, float timeDelta);

	void Job_Physics_BlendFromRagdoll(Skeleton::CPoseData& poseData, float timeDelta);
	void SetPrevHost(IPhysicalEntity* pNewHost = 0)
	{
		if (m_pPrevCharHost == pNewHost)
			return;
		if (m_pPrevCharHost)
		{
			m_pPrevCharHost->Release();
			if (m_pPrevCharHost->GetForeignData(PHYS_FOREIGN_ID_RAGDOLL) == (uk )(ICharacterInstance*)m_pInstance)
				gEnv->pPhysicalWorld->DestroyPhysicalEntity(m_pPrevCharHost);
		}
		if (m_pPrevCharHost = pNewHost)
			pNewHost->AddRef();
	}

public:
	IPhysicalEntity*  m_pCharPhysics;
	IPhysicalEntity** m_ppBonePhysics;
	float             m_timeStandingUp;

	bool              m_bBlendFromRagdollFlip : 1;
	bool              m_bHasPhysics           : 1;
	bool              m_bHasPhysicsProxies    : 1;
	bool              m_bPhysicsAwake         : 1;
	bool              m_bPhysicsWasAwake      : 1;
	bool              m_bPhysicsRelinquished  : 1;

private:
	QuatTS                  m_location;

	IPhysicalEntity*        m_pPrevCharHost;
	DynArray<CPhysicsJoint> m_arrPhysicsJoints;
	Vec3                    m_physicsJointRootPosition;

	struct PhysData
	{
		QuatT location;
		Quat  rotation;
		Ang3  angles;

		bool  bSet  : 1;
		bool  bSet2 : 1;
	}*   m_pPhysBuffer;
	bool m_bPhysicsSynchronize;
	bool m_bPhysicsSynchronizeFromEntity;
	bool m_bPhysBufferFilled;
	i32  m_idLastSyncFrame = 0;

	struct PhysAuxData
	{
		QuatT matrix;
		Vec3* pPoints;

		bool  bSet : 1;
	}*   m_pPhysAuxBuffer;
	bool m_bPhysicsSynchronizeAux;

	struct PhysImpactData
	{
		Ang3 angles; // in-out
		Vec3 pivot;  // out
		Quat q0;     // out

		bool bSet : 1;
	}*                      m_pPhysImpactBuffer;

	Vec3                    m_vOffset;
	mutable i32             m_iSpineBone[3];
	mutable u32          m_nSpineBones;
	i32                     m_nAuxPhys;
	i32                     m_iSurfaceIdx;
	DynArray<aux_phys_data> m_auxPhys;
	f32                     m_fPhysBlendTime;
	f32                     m_fPhysBlendMaxTime;
	f32                     m_frPhysBlendMaxTime;
	float                   m_stiffnessScale;
	f32                     m_fScale;
	f32                     m_fMass;
	Vec3                    m_prevPosPivot;
	Vec3                    m_velPivot;
	DynArray<QuatT>         m_physJoints;
	DynArray<i32>           m_physJointsIdx;
	i32                     m_nPhysJoints;
	SBatchUpdateValidator*  m_pPhysUpdateValidator;
	struct SExtraPhysInfo
	{
		u32         iJoint;
		i32            nLod;
		DrxBonePhysics info;
	};
	DynArray<SExtraPhysInfo> m_extraPhysInfo;

	// From SkeletonPose
private:
	CCharInstance*       m_pInstance;
	CSkeletonPose*       m_pSkeletonPose;
	CSkeletonAnim*       m_pSkeletonAnim;

	bool                 m_bLimpRagdoll              : 1;
	bool                 m_bSetDefaultPoseExecute    : 1;

	bool                 m_bFullSkeletonUpdate       : 1;
	bool                 m_bForcePostSynchronization : 1;

	DynArray<CCGAJoint>* m_arrCGAJoints;

	const Skeleton::CPoseData& GetPoseData() const;
	Skeleton::CPoseData&       GetPoseDataExplicitWriteable();
	Skeleton::CPoseData&       GetPoseDataForceWriteable();
	Skeleton::CPoseData*       GetPoseDataWriteable();
	const Skeleton::CPoseData& GetPoseDataDefault() const;

	i16                      GetJointIDByName(tukk szJointName) const;

public:
	size_t SizeOfThis()
	{
		size_t TotalSize = 0;
		TotalSize += m_arrPhysicsJoints.get_alloc_size();
		if (m_ppBonePhysics)
			TotalSize += GetPoseData().GetJointCount() * sizeof(IPhysicalEntity*);
		return TotalSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddContainer(m_arrPhysicsJoints);
		pSizer->AddContainer(m_auxPhys);
	}
};
