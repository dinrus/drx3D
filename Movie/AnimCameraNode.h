// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __animcameranode_h__
#define __animcameranode_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Movie/EntityNode.h>
#include <drx3D/CoreX/Math/PNoise3.h>

class CAnimSplineTrack;

/** Camera node controls camera entity.
 */
class CAnimCameraNode : public CAnimEntityNode, public IAnimCameraNode
{
public:
	CAnimCameraNode(i32k id);
	virtual ~CAnimCameraNode();

	virtual IAnimCameraNode* QueryCameraNodeInterface() override { return this; }

	static void              Initialize();
	virtual EAnimNodeType    GetType() const override { return eAnimNodeType_Camera; }
	virtual void             Animate(SAnimContext& animContext) override;

	virtual void             CreateDefaultTracks() override;
	virtual void             OnReset() override;
	float                    GetFOV() { return m_fFOV; }

	virtual void             Activate(bool bActivate) override;

	virtual void             OnStop() override;

	virtual u32     GetParamCount() const override;
	virtual CAnimParamType   GetParamType(u32 nIndex) const override;

	virtual void             InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override;
	virtual void             Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual void             SetParameter(SAnimTime time, const EAnimParamType& paramType, const TMovieSystemValue& value) override;
	TMovieSystemValue        GetParameter(SAnimTime time, const EAnimParamType& paramType) const override;

	virtual bool             GetShakeRotation(SAnimTime time, Quat& rot) override;
	virtual void             SetCameraShakeSeed(const uint cameraShakeSeedValue) override { m_cameraShakeSeedValue = cameraShakeSeedValue; }

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	// Field of view in DEGREES! To Display it nicely for user.
	float                   m_fFOV;
	Vec3                    m_fDOF;
	float                   m_fNearZ;
	bool                    m_bJustActivated;
	uint                    m_cameraShakeSeedValue;
	CPNoise3                m_pNoiseGen;
	static CAnimCameraNode* m_pLastFrameActiveCameraNode;

	static i32k        SHAKE_COUNT = 2;
	struct ShakeParam
	{
		Vec3  amplitude;
		float amplitudeMult;
		Vec3  frequency;
		float frequencyMult;
		float noiseAmpMult;
		float noiseFreqMult;
		float timeOffset;

		Vec3  phase;
		Vec3  phaseNoise;

		ShakeParam()
		{
			amplitude = Vec3(1.0f, 1.0f, 1.0f);
			amplitudeMult = 0.0f;
			frequency = Vec3(1.0f, 1.0f, 1.0f);
			frequencyMult = 0.0f;
			noiseAmpMult = 0.0f;
			noiseFreqMult = 0.0f;
			timeOffset = 0.0f;
		}

		Ang3 ApplyCameraShake(CPNoise3& noiseGen, SAnimTime time, Ang3 angles, CAnimSplineTrack* pFreqTrack, EntityId camEntityId, i32 shakeIndex, const uint shakeSeed);
	};

	ShakeParam m_shakeParam[SHAKE_COUNT];

	ICVar*     m_cv_r_PostProcessEffects;
};

#endif // __animcameranode_h__
