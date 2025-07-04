// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#ifndef RESOURCE_COMPILER
#include <drx3D/Animation/GlobalAnimationHeader.h>
#include <drx3D/Animation/Controller.h>
#include <drx3D/Animation/ControllerPQLog.h>
#include <drx3D/Animation/ControllerTCB.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/CoreX/String/NameCRCHelper.h>

class CDefaultSkeleton;

#define AIM_POSES (1)
#define PRECISION (0.001)

struct AimIKPose
{
	DynArray<Quat> m_arrRotation;
	DynArray<Vec3> m_arrPosition;

	void           GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_arrRotation);
		pSizer->AddObject(m_arrPosition);
	}
};

struct QuadIndices
{
	u8  i0, i1, i2, i3;
	Vec4   w0, w1, w2, w3;
	ColorB col;
	Vec3   height;
};

//////////////////////////////////////////////////////////////////////////
// this is the animation information on the module level (not on the per-model level)
// it doesn't know the name of the animation (which is model-specific), but does know the file name
// Implements some services for bone binding and ref counting
struct GlobalAnimationHeaderAIM : public GlobalAnimationHeader
{
	friend class CAnimationUpr;
	friend class CAnimationSet;

	GlobalAnimationHeaderAIM()
		: m_nRef_by_Model(0)
		, m_nTouchedCounter(0)
		, m_nControllers(0)
		, m_fStartSec(-1.0f)
		, m_fEndSec(-1.0f)
		, m_fTotalDuration(-1.0f)
		, m_arrController()
		, m_AnimTokenCRC32(0)
		, m_arrAimIKPosesAIM()
		, m_nExist(0)
		, m_MiddleAimPoseRot(IDENTITY)
		, m_MiddleAimPose(IDENTITY)
		//, m_PolarGrid() skipped
		, VE2()
	{
	}

	~GlobalAnimationHeaderAIM()
	{
		ClearControllers();
	}

	void AddRef()
	{
		++m_nRef_by_Model;
	}

	void Release()
	{
		if (!--m_nRef_by_Model)
		{
			m_nControllers = 0;
			m_arrController.clear();
		}
	}

	ILINE u32 IsAssetLoaded() const
	{
		return m_nControllers;
	}

	ILINE u32 IsAssetOnDemand() const
	{
		return 0;
	}

	IController* GetControllerByJointCRC32(u32 nControllerID) const
	{
		for (u32 i = 0; i < m_nControllers; ++i)
		{
			u32 nCID = m_arrController[i]->GetID();
			if (nControllerID == nCID)
			{
				return m_arrController[i];
			}
		}
		return nullptr;
	}

	f32 NTime2KTime(f32 ntime) const
	{
		ntime = min(ntime, 1.0f);
		assert(ntime >= 0 && ntime <= 1);
		f32 duration = m_fEndSec - m_fStartSec;
		f32 start = m_fStartSec;
		f32 key = (ntime * ANIMATION_30Hz * duration + start * ANIMATION_30Hz); ///40.0f;
		return key;
	}

	size_t SizeOfAIM() const
	{
		size_t nSize = 0;//sizeof(*this);
		size_t nTemp00 = m_FilePath.capacity();
		nSize += nTemp00;
		size_t nTemp08 = m_arrAimIKPosesAIM.get_alloc_size();
		nSize += nTemp08;
		u32 numAimPoses = m_arrAimIKPosesAIM.size();
		for (size_t i = 0; i < numAimPoses; ++i)
		{
			nSize += m_arrAimIKPosesAIM[i].m_arrRotation.get_alloc_size();
			nSize += m_arrAimIKPosesAIM[i].m_arrPosition.get_alloc_size();
		}

		size_t nTemp09 = m_arrController.get_alloc_size();
		nSize += nTemp09;
		for (u16 i = 0; i < m_nControllers; ++i)
			nSize += m_arrController[i]->SizeOfController();
		return nSize;
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_FilePath);
		pSizer->AddObject(m_arrAimIKPosesAIM);
	}

	size_t GetControllersCount() const
	{
		return m_nControllers;
	}

	void ClearControllers()
	{
		ClearAssetRequested();
		m_nControllers = 0;
		m_arrController.clear();
	}

	u32 LoadAIM();

	void   ProcessDirectionalPoses(const CDefaultSkeleton* pDefaultSkeleton, const DynArray<DirectionalBlends>& rDirBlends, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos);
	void   ProcessPoses(const IDefaultSkeleton* pDefaultSkeleton, tukk szAnimationName);

	Vec3   Debug_PolarCoordinate(const Quat& q) const;
	u32 Debug_AnnotateExamples2(u32 numPoses, QuadIndices* arrQuat) const;
	void   Debug_NLerp2AimPose(const CDefaultSkeleton* pDefaultSkeleton, const SJointsAimIK_Rot* rRot, u32 numRotJoints, const SJointsAimIK_Pos* rPos, u32 numPosJoints, const QuatT* arrRelPose0, const QuatT* arrRelPose1, f32 t, QuatT* arrAbsPose) const;
	void   Debug_Blend4AimPose(const CDefaultSkeleton* pDefaultSkeleton, const SJointsAimIK_Rot* rRot, u32 numRotJoints, const SJointsAimIK_Pos* rPos, u32 numPosJoints, int8 i0, int8 i1, int8 i2, int8 i3, const Vec4& w, QuatT* arrRelPose, QuatT* arrAbsPose) const;

private:

	bool   LoadChunks(IChunkFile* pChunkFile);
	bool   ReadMotionParameters(IChunkFile::ChunkDesc* pChunkDesc);
	bool   ReadController(IChunkFile::ChunkDesc* pChunkDesc, u32 bLoadUncompressedChunks);
	bool   ReadTiming(IChunkFile::ChunkDesc* pChunkDesc);

	u32 FlagsSanityFilter(u32 flags);

public:

	u16                                           m_nRef_by_Model;   //< Counts how many models are referencing this asset.
	u16                                           m_nTouchedCounter; //< Keeps track of asset usage.
	u16                                           m_nControllers;

	f32                                              m_fStartSec;      //< Start time in seconds.
	f32                                              m_fEndSec;        // <End time in seconds.
	f32                                              m_fTotalDuration; //< Duration in seconds.

	DynArray<IController_AutoPtr>                    m_arrController;
	u32                                           m_AnimTokenCRC32;
	DynArray<AimIKPose>                              m_arrAimIKPosesAIM;

	uint64                                           m_nExist;
	Quat                                             m_MiddleAimPoseRot;
	Quat                                             m_MiddleAimPose;
	CHUNK_GAHAIM_INFO::VirtualExample                m_PolarGrid[CHUNK_GAHAIM_INFO::XGRID * CHUNK_GAHAIM_INFO::YGRID];
	DynArray<CHUNK_GAHAIM_INFO::VirtualExampleInit2> VE2;
};

struct VExampleInit
{
	struct VirtualExampleInit1
	{
		f32   m_fSmalest;
		u8 i0, i1, i2, i3;
		f64   w0, w1, w2, w3;
	};

	f64                 m_fSmallest;
	u32              m_nIterations;
	Vec4d               m_Weight;

	i32               m_arrParentIdx[MAX_JOINT_AMOUNT];
	QuatT               m_arrDefaultRelPose[MAX_JOINT_AMOUNT];
	u32              m_arrJointCRC32[MAX_JOINT_AMOUNT];

	QuatTd              m_arrRelPose[MAX_JOINT_AMOUNT];
	QuatTd              m_arrAbsPose[MAX_JOINT_AMOUNT];
	QuatTd              m_arrRelPose0[MAX_JOINT_AMOUNT];
	QuatTd              m_arrAbsPose0[MAX_JOINT_AMOUNT];
	QuatTd              m_arrRelPose1[MAX_JOINT_AMOUNT];
	QuatTd              m_arrAbsPose1[MAX_JOINT_AMOUNT];
	QuatTd              m_arrRelPose2[MAX_JOINT_AMOUNT];
	QuatTd              m_arrAbsPose2[MAX_JOINT_AMOUNT];
	QuatTd              m_arrRelPose3[MAX_JOINT_AMOUNT];
	QuatTd              m_arrAbsPose3[MAX_JOINT_AMOUNT];
	VirtualExampleInit1 PolarGrid[CHUNK_GAHAIM_INFO::XGRID * CHUNK_GAHAIM_INFO::YGRID];

	void   Init(const CDefaultSkeleton* pDefaultSkeleton, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, GlobalAnimationHeaderAIM& rAIM, i32 nWeaponBoneIdx, u32 numAimPoses);
	void   CopyPoses2(const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, GlobalAnimationHeaderAIM& rAIM, u32 numPoses, u32 skey);
	void   RecursiveTest(const Vec2d& ControlPoint, GlobalAnimationHeaderAIM& rAIM, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, i32 nWBone, i32 i0, i32 i1, i32 i2, i32 i3, const Vec4d& w0, const Vec4d& w1, const Vec4d& w2, const Vec4d& w3);
	u32 PointInQuat(const Vec2d& ControlPoint, GlobalAnimationHeaderAIM& rAIM, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, i32 nWBone, i32 i0, i32 i1, i32 i2, i32 i3, const Vec4d& w0, const Vec4d& w1, const Vec4d& w2, const Vec4d& w3);

	u32 LinesegOverlap2D(const Planed& plane0, const Vec2d& ls0, const Vec2d& le0, const Vec2d& tp0, const Vec2d& tp1);

	void   ComputeAimPose(GlobalAnimationHeaderAIM& rAIM, const CDefaultSkeleton* pDefaultSkeleton, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, QuatTd* arrAbsPose, u32 nAimPose);
	void   Blend4AimPose(GlobalAnimationHeaderAIM& rAIM, const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, int8 i0, int8 i1, int8 i2, int8 i3, const Vec4d& w, QuatTd* arrRelPose, QuatTd* arrAbsPose);
	void   NLerp2AimPose(const DynArray<SJointsAimIK_Rot>& rRot, const DynArray<SJointsAimIK_Pos>& rPos, QuatTd* arrRelPose0, QuatTd* arrRelPose1, f64 t, QuatTd* arrAbsPose);
	u32 AnnotateExamples(u32 numPoses, QuadIndices* arrQuat);
	Vec3d  PolarCoordinate(const Quatd& q);
};
#endif