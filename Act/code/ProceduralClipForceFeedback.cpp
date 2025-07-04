// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Act/IForceFeedbackSystem.h>

#include <drx3D/Act/Serialization.h>

struct SForceFeedbackClipParams : public IProceduralParams
{
	SForceFeedbackClipParams()
		: scale(1.f)
		, delay(0.f)
		, onlyLocal(true)
	{
	}

	virtual void Serialize(Serialization::IArchive& ar) override
	{
		ar(Serialization::Decorators::ForceFeedbackIdName<TProcClipString>(forceFeedbackId), "ForceFeedbackID", "ForceFeedbackID");
		ar(scale, "Scale", "Scale");
		ar(delay, "Delay", "Delay");
		ar(onlyLocal, "OnlyLocal", "OnlyLocal");
	}

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = forceFeedbackId.c_str();
	}

	TProcClipString forceFeedbackId;
	float           scale;
	float           delay;
	bool            onlyLocal;
};

class CProceduralClipForceFeedback : public TProceduralClip<SForceFeedbackClipParams>
{
public:
	CProceduralClipForceFeedback()
	{
	}

	virtual void OnEnter(float blendTime, float duration, const SForceFeedbackClipParams& params)
	{
		IGameFramework* pGameFrameWork = gEnv->pGameFramework;

		if ((gEnv->IsEditor() && pGameFrameWork->GetMannequinInterface().IsSilentPlaybackMode())
		    || (params.onlyLocal && pGameFrameWork->GetClientActorId() != m_entity->GetId()))
		{
			return;
		}

		IForceFeedbackSystem* pForceFeedback = CDrxAction::GetDrxAction()->GetIForceFeedbackSystem();
		DRX_ASSERT(pForceFeedback);
		ForceFeedbackFxId fxId = pForceFeedback->GetEffectIdByName(params.forceFeedbackId.c_str());

		if (fxId != InvalidForceFeedbackFxId)
		{
			float actionScale = 0.f;
			float ffScale = params.scale;

			if (GetParam("ffScale", actionScale))
			{
				ffScale *= actionScale;
			}

			SForceFeedbackRuntimeParams runtimeParams(ffScale, params.delay);
			pForceFeedback->PlayForceFeedbackEffect(fxId, runtimeParams);
		}
	}

	virtual void OnExit(float blendTime)  {}

	virtual void Update(float timePassed) {}

};

REGISTER_PROCEDURAL_CLIP(CProceduralClipForceFeedback, "ForceFeedback");
