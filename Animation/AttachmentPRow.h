// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/AttachmentBase.h>

class CAttachmentUpr;
class CCharInstance;

#define MAX_JOINTS_PER_ROW (100)

//dynamic members
struct Particle
{
	Vec2        m_vDistance;
	Vec3        m_vBobPosition; //this is a world-position
	Vec3        m_vBobVelocity;
	Vec3        m_vAttModelRelativePrev;
	QuatT       m_vLocationPrev;   //location from last frame
	JointIdType m_jointID;
	JointIdType m_childID;

	Particle()
	{
		m_jointID = JointIdType(-1);
		m_childID = JointIdType(-1);
		m_vDistance.zero();
		m_vBobPosition.zero();
		m_vBobVelocity.zero();
		m_vLocationPrev.SetIdentity();
		m_vAttModelRelativePrev = Vec3(ZERO);
	};
};

struct CPendulaRow : public RowSimulationParams
{
	CPendulaRow()
	{
		m_nParticlesPerRow = 0;
		m_nAnimOverrideJoint = -1;
		m_fMaxAngleCos = DEG2COS(m_fConeAngle);
		m_fMaxAngleHCos = DEG2HCOS(m_fConeAngle);
		m_vConeRotHCos.x = clamp_tpl(DEG2HCOS(m_vConeRotation.x), -1.0f, 1.0f);
		m_vConeRotHCos.y = clamp_tpl(DEG2HCOS(m_vConeRotation.y), -1.0f, 1.0f);
		m_vConeRotHCos.z = clamp_tpl(DEG2HCOS(m_vConeRotation.z), -1.0f, 1.0f);
		m_vConeRotHSin.x = clamp_tpl(DEG2HSIN(m_vConeRotation.x), -1.0f, 1.0f);
		m_vConeRotHSin.y = clamp_tpl(DEG2HSIN(m_vConeRotation.y), -1.0f, 1.0f);
		m_vConeRotHSin.z = clamp_tpl(DEG2HSIN(m_vConeRotation.z), -1.0f, 1.0f);
		m_pitchc = 1, m_pitchs = 0, m_rollc = 1, m_rolls = 0;
		m_fTimeAccumulator = 0;
		m_idxDirTransJoint = -1;
	};

	void PostUpdate(const CAttachmentUpr* pAttachmentUpr, tukk pJointName);
	void UpdatePendulumRow(const CAttachmentUpr* pAttachmentUpr, Skeleton::CPoseData& rPoseData, tukk pAttName);
	void Draw(const QuatTS& qt, const ColorB clr, u32k tesselation, const Vec3& vdir);

	//these members store the optimized parameters
	u16             m_nParticlesPerRow;
	i16              m_nAnimOverrideJoint;
	f32                m_fMaxAngleCos, m_fMaxAngleHCos; //cosine and half-cosine for the angle
	Vec3               m_vConeRotHCos;
	Vec3               m_vConeRotHSin;
	f32                m_pitchc, m_pitchs, m_rollc, m_rolls; //half-cosines of yaw & pitch
	f32                m_fTimeAccumulator;
	i32              m_idxDirTransJoint;

	DynArray<i16>    m_arrProxyIndex;     //the indices to the proxies (atm only lozenges)
	DynArray<Particle> m_arrParticles;
};

class CAttachmentPROW : public SAttachmentBase
{
public:

	CAttachmentPROW()
	{
		m_nRowJointID = -1;
	}

	virtual ~CAttachmentPROW() {}

	virtual void AddRef()
	{
		++m_nRefCounter;
	}

	virtual void Release()
	{
		if (--m_nRefCounter == 0)
			delete this;
	}

	virtual u32               GetType() const { return CA_PROW; }
	virtual u32               SetJointName(tukk szJointName);

	virtual tukk          GetName() const                                                                                    { return m_strSocketName; }
	virtual u32               GetNameCRC() const                                                                                 { return m_nSocketCRC32; }
	virtual u32               ReName(tukk strSocketName, u32 crc)                                                      { m_strSocketName.clear();  m_strSocketName = strSocketName; m_nSocketCRC32 = crc;  return 1; };

	virtual u32               GetFlags() const                                                                                   { return m_AttFlags; }
	virtual void                 SetFlags(u32 flags)                                                                             { m_AttFlags = flags; }

	virtual u32               Immediate_AddBinding(IAttachmentObject* pModel, ISkin* pISkinRender = 0, u32 nLoadingFlags = 0) { return 0; }
	virtual void                 Immediate_ClearBinding(u32 nLoadingFlags = 0)                                                   {};
	virtual u32               Immediate_SwapBinding(IAttachment* pNewAttachment)                                                 { return 0; }

	virtual void                 HideAttachment(u32 x)                                                                           {}
	virtual u32               IsAttachmentHidden() const                                                                         { return m_AttFlags & FLAGS_ATTACH_HIDE_MAIN_PASS; }
	virtual void                 HideInRecursion(u32 x);
	virtual u32               IsAttachmentHiddenInRecursion() const                                                              { return m_AttFlags & FLAGS_ATTACH_HIDE_RECURSION; }
	virtual void                 HideInShadow(u32 x);
	virtual u32               IsAttachmentHiddenInShadow() const                                                                 { return m_AttFlags & FLAGS_ATTACH_HIDE_SHADOW_PASS; }

	virtual void                 SetAttAbsoluteDefault(const QuatT& qt)                                                             {};
	virtual void                 SetAttRelativeDefault(const QuatT& qt)                                                             {};
	virtual const QuatT&         GetAttAbsoluteDefault() const                                                                      { return g_IdentityQuatT; };
	virtual const QuatT&         GetAttRelativeDefault() const                                                                      { return g_IdentityQuatT; };

	virtual const QuatT&         GetAttModelRelative() const                                                                        { return g_IdentityQuatT;  };//this is relative to the animated bone
	virtual const QuatTS         GetAttWorldAbsolute() const;
	virtual const QuatT&         GetAdditionalTransformation() const                                                                { return g_IdentityQuatT; }
	virtual void                 UpdateAttModelRelative()                                                                           {};

	virtual u32               GetJointID() const                                                                                 { return -1; };
	virtual void                 AlignJointAttachment()                                                                             {};

	virtual RowSimulationParams& GetRowParams()                                                                                     { return m_rowparams;   };
	virtual void                 PostUpdateSimulationParams(bool bAttachmentSortingRequired, tukk pJointName = 0);

	virtual void                 Serialize(TSerialize ser)                                                          {};
	virtual size_t               SizeOfThis() const                                                                 { return 0; };
	virtual void                 GetMemoryUsage(IDrxSizer* pSizer) const                                            {};
	virtual void                 TriggerMeshStreaming(u32 nDesiredRenderLOD, const SRenderingPassInfo& passInfo) {};

	virtual IAttachmentObject*   GetIAttachmentObject() const                                                       { return 0; }
	virtual IAttachmentSkin*     GetIAttachmentSkin()                                                               { return 0; }

public:
	void UpdateRow(Skeleton::CPoseData& rPoseData);
	string      m_strRowJointName;
	i32       m_nRowJointID;
	CPendulaRow m_rowparams;
};
