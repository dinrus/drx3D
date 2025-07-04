// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/FaceEffector.h>
#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/Animation/Skeleton.h>

class CDefaultSkeleton;
class CFacialEffectorsLibrary;
class CCharInstance;
class CFaceState;
class DrxModEffMorph;
class CSkeletonPose;

class CFacialDisplaceInfo
{
public:
	CFacialDisplaceInfo(u32k count = 0)
		: m_displaceInfo(count, IDENTITY)
		, m_used(count, 0)
		, m_hasUsed(false)
	{
	}

	void Initialize(const size_t count)
	{
		m_displaceInfo.clear();
		m_displaceInfo.resize(count, IDENTITY);

		m_used.clear();
		m_used.resize(count, 0);

		m_hasUsed = false;
	}

	void CopyUsed(const CFacialDisplaceInfo& other)
	{
		m_used = other.m_used;
		m_hasUsed = other.m_hasUsed;
	}

	size_t GetCount() const { return m_displaceInfo.size(); }

	bool   IsUsed(const size_t i) const
	{
		DrxPrefetch((uk )&m_displaceInfo[i]);
		return (m_used[i] == 1);
	}

	bool         HasUsed() const { return m_hasUsed; }

	const QuatT& GetDisplaceInfo(const size_t i) const
	{
		DrxPrefetch((uk )&(m_used[i]));
		return m_displaceInfo[i];
	}

	void SetDisplaceInfo(const size_t i, const QuatT& q)
	{
		m_displaceInfo[i] = q;
		m_used[i] = 1;
		m_hasUsed = true;
	}

	void Clear()
	{
		const size_t count = m_displaceInfo.size();
		Initialize(count);
	}

	void ClearFast()
	{
		m_hasUsed = false;
	}

private:
	std::vector<QuatT>  m_displaceInfo;
	std::vector<u32> m_used;
	bool                m_hasUsed;
};

//////////////////////////////////////////////////////////////////////////
class CFacialModel : public IFacialModel, public _i_reference_target_t
{
public:
	struct SMorphTargetInfo
	{
		CFaceIdentifierStorage name;
		i32                    nMorphTargetId;
		bool operator<(const SMorphTargetInfo& m) const  { return name.GetCRC32() < m.name.GetCRC32(); }
		bool operator>(const SMorphTargetInfo& m) const  { return name.GetCRC32() > m.name.GetCRC32(); }
		bool operator==(const SMorphTargetInfo& m) const { return name.GetCRC32() == m.name.GetCRC32(); }
		bool operator!=(const SMorphTargetInfo& m) const { return name.GetCRC32() != m.name.GetCRC32(); }
		void GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
		}
	};

	typedef std::vector<SMorphTargetInfo> MorphTargetsInfo;

	struct SEffectorInfo
	{
		_smart_ptr<CFacialEffector> pEffector;
		string                      name;           // morph target name.
		i32                         nMorphTargetId; // positive if effector is a morph target.
		void                        GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(name);
		}
	};

	CFacialModel(CDefaultSkeleton* pDefaultSkeleton);

	size_t SizeOfThis();
	void   GetMemoryUsage(IDrxSizer* pSizer) const;

	//////////////////////////////////////////////////////////////////////////
	// IFacialModel interface
	//////////////////////////////////////////////////////////////////////////
	i32                              GetEffectorCount() const      { return m_effectors.size(); }
	CFacialEffector*                 GetEffector(i32 nIndex) const { return m_effectors[nIndex].pEffector; }
	virtual void                     AssignLibrary(IFacialEffectorsLibrary* pLibrary);
	virtual IFacialEffectorsLibrary* GetLibrary();
	virtual i32                      GetMorphTargetCount() const;
	virtual tukk              GetMorphTargetName(i32 morphTargetIndex) const;
	//////////////////////////////////////////////////////////////////////////

	// Creates a new FaceState.
	CFaceState*              CreateState();

	SEffectorInfo*           GetEffectorsInfo()          { return &m_effectors[0]; }
	CFacialEffectorsLibrary* GetFacialEffectorsLibrary() { return m_pEffectorsLibrary; }

	//////////////////////////////////////////////////////////////////////////
	// Apply state to the specified character instance.
	void        FillInfoMapFromFaceState(CFacialDisplaceInfo& info, CFaceState* const pFaceState, CCharInstance* const pInstance, i32 numForcedRotations, const CFacialAnimForcedRotationEntry* const forcedRotations, float blendBoneRotations);

	static void ApplyDisplaceInfoToJoints(const CFacialDisplaceInfo& info, const CDefaultSkeleton* const pSkeleton, Skeleton::CPoseData* const pPoseData, const QuatT* const pJointsRelativeDefault);

	static void ClearResources();
private:
	void        InitMorphTargets();

	void        TransformTranslations(CFacialDisplaceInfo& info, const CCharInstance* const pInstance);
	i32       ApplyBoneEffector(CCharInstance* pInstance, CFacialEffector* pEffector, float fWeight, QuatT& quatT);
	//	void HandleLookIK(FacialDisplaceInfoMap& Info, CLookAt& lookIK, bool physicsRelinquished, i32 head, i32 leye, i32 reye);
	void        PushMorphEffectors(i32 i, const MorphTargetsInfo& morphTargets, f32 fWeight, f32 fBalance, DynArray<DrxModEffMorph>& arrMorphEffectors);

private:
	// Array of facial effectors that apply to this model.
	std::vector<SEffectorInfo>          m_effectors;

	_smart_ptr<CFacialEffectorsLibrary> m_pEffectorsLibrary;

	// Maps indices from the face state into the morph target id.
	std::vector<i32> m_faceStateIndexToMorphTargetId;
	MorphTargetsInfo m_morphTargets;

	// Character model who owns this facial model.
	CDefaultSkeleton*          m_pDefaultSkeleton;

	bool                       m_bAttachmentChanged;

	static CFacialDisplaceInfo s_boneInfoBlending;
};
