// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/Skeleton.h>

class CAttachmentUpr;

struct CSimulation : public SimulationParams
{
	CSimulation()
	{
		m_crcProcFunction = 0;
		m_idxDirTransJoint = -1;
		m_nAnimOverrideJoint = -1;
		m_fHRotationHCosX = DEG2HCOS(m_vDiskRotation.x);
		m_fHRotationHCosZ = DEG2HCOS(m_vDiskRotation.y);
		m_fMaxAngleCos = DEG2COS(m_fMaxAngle);
		m_fMaxAngleHCos = DEG2HCOS(m_fMaxAngle);
		m_aa1c = 1, m_aa1s = 0, m_aa2c = 1, m_aa2s = 0;

		//dynamic members
		m_vBobPosition.zero();
		m_vBobVelocity.zero();
		m_vAttLocationPrev.SetIdentity();
		m_vAttModelRelativePrev = Vec3(ZERO);
		m_crcProcFunction = 0;

		m_vSpringBobVelocityMS.zero();
		m_vSpringBobPositionMS.zero();
		m_rPhysLocationWSPrev.SetIdentity();
		m_vAttLocationMSWithoutTranslateWSPrev.SetIdentity();
	};

	void PostUpdate(const CAttachmentUpr* pAttachmentUpr, tukk pJointName);
	void UpdateSimulation(const CAttachmentUpr* pAttachmentUpr, Skeleton::CPoseData& rPoseData, i32 nRedirectionID, const QuatT& rAttModelRelative, QuatT& raddTransformation, tukk pAttName);
	void UpdatePendulumSimulation(const CAttachmentUpr* pAttachmentUpr, Skeleton::CPoseData& rPoseData, i32 nRedirectionID, const QuatT& rAttModelRelative, QuatT& raddTransformation, tukk pAttName);
	void UpdateSpringSimulation(const CAttachmentUpr* pAttachmentUpr, Skeleton::CPoseData& rPoseData, i32 nRedirectionID, const QuatT& rAttModelRelative, Vec3& raddTranslation, tukk pAttName);
	void ProjectJointOutOfLozenge(const CAttachmentUpr* pAttachmentUpr, Skeleton::CPoseData& rPoseData, i32 nRedirectionID, const QuatT& rAttModelRelative, tukk pAttName);
	void Draw(const QuatTS& qt, const ColorB clr, u32k tesselation, const Vec3& vdir);

	//these members store the optimized parameters
	DynArray<i16>       m_arrProxyIndex;        // the indices to the proxies (atm only lozenges)
	DynArray<JointIdType> m_arrChildren;
	u32                m_crcProcFunction;
	i16                 m_nAnimOverrideJoint;

private:

	i16 m_idxDirTransJoint;
	f32   m_fMaxAngleCos, m_fMaxAngleHCos;        // cosine and half-cosine for the angle
	f32   m_fHRotationHCosX, m_fHRotationHCosZ;   // yaw and pitch  half-cosine

	f32   m_aa1c, m_aa1s, m_aa2c, m_aa2s;         // half-cosines of m_vStiffnessTarget

	//dynamic members
	Vec3  m_vBobPosition;                         // pendulum/spring end-position in world-space
	Vec3  m_vBobVelocity;
	Vec3  m_vAttModelRelativePrev;
	QuatT m_vAttLocationPrev;                     // attributes location from last frame

	Vec3  m_vSpringBobPositionMS;                 // spring end-position in model-space
	Vec3  m_vSpringBobVelocityMS;                 // spring velocity in model-space

	QuatT m_rPhysLocationWSPrev;                  // store actual nodes quaternion from last step (to determine time derivation)
	QuatT m_vAttLocationMSWithoutTranslateWSPrev; // store attributes model-space location from last step (to determine time derivation)

};
