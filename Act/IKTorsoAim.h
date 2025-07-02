// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef IKTorsoAim_h
#define IKTorsoAim_h

#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/Act/IAnimatedCharacter.h>

#define TORSOAIM_MAX_JOINTS 5

class DRX_ALIGN(32) CIKTorsoAim:
	public IAnimationPoseModifierTorsoAim
{
public:
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(IAnimationPoseModifier)
		DRXINTERFACE_ADD(IAnimationPoseModifierTorsoAim)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CIKTorsoAim, "AnimationPoseModifier_IKTorsoAim", "2058e99d-d052-43e2-8898-5eff40b942e4"_drx_guid)

	CIKTorsoAim();
	virtual ~CIKTorsoAim() {}

public:
	void Enable(bool enable);
	static void InitCVars();

	// IAnimationIKTorsoAim
public:
	virtual void SetBlendWeight(float blend);
	virtual void SetBlendWeightPosition(float blend);
	virtual void SetTargetDirection(const Vec3 &direction);
	virtual void SetViewOffset(const QuatT &offset);
	virtual void SetAbsoluteTargetPosition(const Vec3 &targetPosition);
	virtual void SetFeatherWeights(u32 weights, const f32 * customBlends);
	virtual void SetJoints(u32 jntEffector, u32 jntAdjustment);
	virtual void SetBaseAnimTrackFactor(float factor);

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams &params) override;
	virtual bool Execute(const SAnimationPoseModifierParams &params) override;
	virtual void Synchronize() override;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	const QuatT& GetLastProcessedEffector() const
	{
		return m_lastProcessedEffector;
	}

private:

	struct SParams
	{
		SParams();

		QuatT viewOffset;
		Vec3  targetDirection;
		Vec3  absoluteTargetPosition;
		float baseAnimTrackFactor;
		float weights[TORSOAIM_MAX_JOINTS];
		float blend;
		float blendPosition;
		i32   numParents;
		i32   effectorJoint;
		i32   aimJoint;
	};

	struct STorsoAim_CVars
	{
		STorsoAim_CVars()
			: m_initialized(false)
			, STAP_DEBUG(0)
			, STAP_DISABLE(0)
			, STAP_TRANSLATION_FUDGE(0)
			, STAP_TRANSLATION_FEATHER(0)
			, STAP_LOCK_EFFECTOR(0)
			, STAP_OVERRIDE_TRACK_FACTOR(0.0f)
		{
		}

		~STorsoAim_CVars()
		{
			ReleaseCVars();
		}

		void InitCvars();

		i32   STAP_DEBUG;
		i32   STAP_DISABLE;
		i32   STAP_TRANSLATION_FUDGE;
		i32   STAP_TRANSLATION_FEATHER;
		i32   STAP_LOCK_EFFECTOR;
		float STAP_OVERRIDE_TRACK_FACTOR;

	private:

		void ReleaseCVars();

		bool m_initialized;
	};

	SParams m_params;
	SParams m_setParams;

	bool m_active;
	bool m_blending;
	float m_blendRate;

	Vec3 m_aimToEffector;
	Quat m_effectorDirQuat;
	QuatT m_lastProcessedEffector;

	static STorsoAim_CVars s_CVars;

	void Init();

};

#endif // IKTorsoAim_h
