// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Act/IDrxMannequin.h>
#include <drx3D/Act/Serialization.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourcesAudio.h>

/*
   TEMPORARY_SOUND_FLAGS wraps soundFlags which are used to specify either
   an audio should be triggered on the local player or not.
   This game specific data should be moved to the game once a Wwise switch
   is setup, so that only one audio trigger is needed and Wwise sort it
   internally out which sound to play.
 */
#define TEMPORARY_SOUND_FLAGS

SERIALIZATION_ENUM_BEGIN_NESTED(DrxAudio, EOcclusionType, "OcclusionType")
SERIALIZATION_ENUM(DrxAudio::EOcclusionType::Ignore, "ignore_state_name", "Ignore");
SERIALIZATION_ENUM(DrxAudio::EOcclusionType::Adaptive, "adaptive_state_name", "Adaptive");
SERIALIZATION_ENUM(DrxAudio::EOcclusionType::Low, "low_state_name", "Low");
SERIALIZATION_ENUM(DrxAudio::EOcclusionType::Medium, "medium_state_name", "Medium");
SERIALIZATION_ENUM(DrxAudio::EOcclusionType::High, "high_state_name", "High");
SERIALIZATION_ENUM_END();

class CAudioContext : public IProceduralContext
{
private:
	CAudioContext();

	using BaseClass = IProceduralContext;

public:

	PROCEDURAL_CONTEXT(CAudioContext, "AudioContext", "c6c087f6-4ce1-4854-adca-544d252834bd"_drx_guid);

	virtual void Initialise(IEntity& entity, IActionController& actionController) override
	{
		BaseClass::Initialise(entity, actionController);

		m_pIEntityAudioComponent = entity.GetOrCreateComponent<IEntityAudioComponent>();
	}

	virtual void Update(float timePassed) override
	{
	}

	void ExecuteAudioTrigger(DrxAudio::ControlId const audioTriggerId, DrxAudio::EOcclusionType const occlusionType, bool playFacial)
	{
		if (m_pIEntityAudioComponent != nullptr)
		{
			m_pIEntityAudioComponent->SetObstructionCalcType(occlusionType);
			REINST("support facial animations for CAudioContext (if needed)");
			m_pIEntityAudioComponent->ExecuteTrigger(audioTriggerId);
		}
	}

	void StopAudioTrigger(DrxAudio::ControlId const audioTriggerId)
	{
		if (m_pIEntityAudioComponent != nullptr)
		{
			m_pIEntityAudioComponent->StopTrigger(audioTriggerId);
		}
	}

	void SetAudioObjectPos(QuatT const& offset)
	{
		if (m_pIEntityAudioComponent != nullptr)
		{
			m_pIEntityAudioComponent->SetAudioAuxObjectOffset(Matrix34(IDENTITY, offset.t));
		}
	}

	void SetAudioParameter(DrxAudio::ControlId const audioParameterId, float const value)
	{
		if (m_pIEntityAudioComponent != nullptr)
		{
			m_pIEntityAudioComponent->SetParameter(audioParameterId, value);
		}
	}

private:

	IEntityAudioComponent* m_pIEntityAudioComponent;
};

CAudioContext::CAudioContext() : m_pIEntityAudioComponent()
{
}

DRXREGISTER_CLASS(CAudioContext);

#if defined(TEMPORARY_SOUND_FLAGS)
typedef enum
{
	ePSF_LocalPlayerOnly    = BIT(0),   //only play this sound if we are the local player entity
	ePSF_ExcludeLocalPlayer = BIT(1),   //don't play this sound if we are the local player
} ePlaySoundFlags;
#endif

struct SAudioParams : public IProceduralParams
{
	SAudioParams()
		: audioOcclusionType(DrxAudio::EOcclusionType::Ignore)
		, radius(0.0f)
		, audioParameterValue(0.0f)
		, synchStop(false)
		, forceStopOnExit(false)
		, isVoice(false)
		, playFacial(false)
#if defined(TEMPORARY_SOUND_FLAGS)
		, soundFlags(0)
#endif
	{
	}

	virtual void Serialize(Serialization::IArchive& ar) override
	{
		ar(Serialization::AudioTrigger<TProcClipString>(startTrigger), "StartTrigger", "Start Trigger");
		ar(Serialization::AudioTrigger<TProcClipString>(stopTrigger), "StopTrigger", "Stop Trigger");
		ar(Serialization::AudioRTPC<TProcClipString>(audioParameter), "AudioParameter", "Audio Parameter");
		ar(audioParameterValue, "AudioParameterValue", "Audio Parameter Value");
		ar(audioOcclusionType, "OcclusionType", "Occlusion Type");
		ar(Serialization::Decorators::JointName<SProcDataCRC>(attachmentJoint), "AttachmentJoint", "Joint Name");
		if (!ar.isEdit())
		{
			// Disabled the following deprecated options in the UI, but still read/write old data in case we need them back
			ar(radius, "Radius", "AI Radius");
			ar(isVoice, "IsVoice", "Is Voice");
		}
		ar(playFacial, "PlayFacial", "Request Anim Matching Start Trigger");
#if defined(TEMPORARY_SOUND_FLAGS)
		ar(soundFlags, "SoundFlags", "Sound Flags (deprecated)");
#endif
	}

	virtual void GetExtraDebugInfo(StringWrapper& extraInfoOut) const override
	{
		extraInfoOut = startTrigger.c_str();
	}

	TProcClipString          startTrigger;
	TProcClipString          stopTrigger;
	TProcClipString          audioParameter;
	DrxAudio::EOcclusionType audioOcclusionType;
	SProcDataCRC             attachmentJoint;
	float                    radius;
	float                    audioParameterValue;
	bool                     synchStop;
	bool                     forceStopOnExit;
	bool                     isVoice;
	bool                     playFacial;

#if defined(TEMPORARY_SOUND_FLAGS)
	u32 soundFlags;
#endif
};

class CProceduralClipAudio : public TProceduralContextualClip<CAudioContext, SAudioParams>
{
public:
	CProceduralClipAudio()
		: m_referenceJointID(0)
		, m_audioTriggerStartId(DrxAudio::InvalidControlId)
		, m_audioTriggerStopId(DrxAudio::InvalidControlId)
		, m_audioParameterId(DrxAudio::InvalidControlId)
		, m_audioOcclusionType(DrxAudio::EOcclusionType::None)
		, m_audioParameterValue(0.0f)
	{
	}

private:
	struct SAudioParamInfo
	{
		SAudioParamInfo() : paramIndex(-1), paramCRC(0), paramValue(-1.f) {}
		SAudioParamInfo(i32 _index, u32 _crc, float _value) : paramIndex(_index), paramCRC(_crc), paramValue(_value) {}

		i32    paramIndex;
		u32 paramCRC;
		float  paramValue;
	};

	using TAudioParamVec = std::vector<SAudioParamInfo>;

public:
	virtual void OnEnter(float blendTime, float duration, const SAudioParams& params) override
	{
#if defined(TEMPORARY_SOUND_FLAGS)
		if (params.soundFlags)
		{
			bool bIsLocalPlayer = false;

			if (IGameFramework* gameFramework = gEnv->pGameFramework)
			{
				if (gameFramework->GetClientActorId() == m_entity->GetId())
				{
					bIsLocalPlayer = true;
				}
			}

			if ((params.soundFlags & ePSF_LocalPlayerOnly) && !bIsLocalPlayer)
			{
				return;
			}

			if ((params.soundFlags & ePSF_ExcludeLocalPlayer) && bIsLocalPlayer)
			{
				return;
			}
		}
#endif

		m_referenceJointID = -1;
		bool const playFacial = params.playFacial;
		bool const isVoice = playFacial || params.isVoice;

		bool bIsSilentPlaybackMode = gEnv->IsEditor() && gEnv->pGameFramework->GetMannequinInterface().IsSilentPlaybackMode();

		ICharacterInstance const* const pCharacterInstance = m_scope->GetCharInst();

		if (pCharacterInstance != nullptr)
		{
			m_referenceJointID = pCharacterInstance->GetIDefaultSkeleton().GetJointIDByCRC32(params.attachmentJoint.ToUInt32());
		}

		QuatT const soundOffset = GetBoneAbsLocationByID(m_referenceJointID);

		if (!bIsSilentPlaybackMode)
		{
			if (!params.audioParameter.empty())
			{
				m_audioParameterId = DrxAudio::StringToId(params.audioParameter.c_str());
				m_audioParameterValue = params.audioParameterValue;
				m_context->SetAudioParameter(m_audioParameterId, m_audioParameterValue);
			}

			if (!params.startTrigger.empty())
			{
				m_audioTriggerStartId = DrxAudio::StringToId(params.startTrigger.c_str());
				m_audioOcclusionType = params.audioOcclusionType;
				m_context->ExecuteAudioTrigger(m_audioTriggerStartId, m_audioOcclusionType, playFacial);
			}

			if (!params.stopTrigger.empty())
			{
				m_audioTriggerStopId = DrxAudio::StringToId(params.stopTrigger.c_str());
			}
		}
	}

	virtual void OnExit(float blendTime) override
	{
		if (m_audioTriggerStopId != DrxAudio::InvalidControlId)
		{
			m_context->ExecuteAudioTrigger(m_audioTriggerStopId, m_audioOcclusionType, false);
		}
		else if (m_audioTriggerStartId != DrxAudio::InvalidControlId)
		{
			m_context->StopAudioTrigger(m_audioTriggerStartId);
		}

		m_audioTriggerStartId = DrxAudio::InvalidControlId;
		m_audioTriggerStopId = DrxAudio::InvalidControlId;
		m_audioParameterId = DrxAudio::InvalidControlId;
	}

	virtual void Update(float timePassed) override
	{
		UpdateSoundParams();
		UpdateSoundPosition();
	}

private:

	void UpdateSoundParams()
	{
		if (!m_audioParams.empty())
		{
			const size_t numParams = m_audioParams.size();
			for (size_t i = 0; i < numParams; i++)
			{
				float paramValue = 0.f;
				if (GetParam(m_audioParams[i].paramCRC, paramValue) && (m_audioParams[i].paramValue != paramValue))
				{
					REINST("update RTPCs")
					//pSound->SetParam(m_soundParams[i].paramIndex, paramValue);
					m_audioParams[i].paramValue = paramValue;
				}
			}
		}
	}

	void UpdateSoundPosition()
	{
		if (m_referenceJointID < 0)
			return;

		m_context->SetAudioObjectPos(GetBoneAbsLocationByID(m_referenceJointID));
	}

	QuatT GetBoneAbsLocationByID(i32k jointID)
	{
		ICharacterInstance* pCharacterInstance = m_scope->GetCharInst();
		if ((pCharacterInstance != nullptr) && (jointID >= 0))
		{
			return pCharacterInstance->GetISkeletonPose()->GetAbsJointByID(jointID);
		}

		return QuatT(ZERO, IDENTITY);
	}

	TAudioParamVec           m_audioParams;

	i32                      m_referenceJointID;

	DrxAudio::ControlId      m_audioTriggerStartId;
	DrxAudio::ControlId      m_audioTriggerStopId;
	DrxAudio::ControlId      m_audioParameterId;
	DrxAudio::EOcclusionType m_audioOcclusionType;
	float                    m_audioParameterValue;
};

using CProceduralClipPlaySound = CProceduralClipAudio;
REGISTER_PROCEDURAL_CLIP(CProceduralClipPlaySound, "PlaySound");
REGISTER_PROCEDURAL_CLIP(CProceduralClipAudio, "Audio");
