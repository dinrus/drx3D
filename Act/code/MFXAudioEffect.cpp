// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/MFXAudioEffect.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/CoreX/Audio/IObject.h>

namespace MaterialEffectsUtils
{
	static constexpr DrxAudio::ControlId switchId = DrxAudio::StringToId("1stOr3rdP");
	static constexpr DrxAudio::SwitchStateId fpStateId = DrxAudio::StringToId("1stP");
	static constexpr DrxAudio::SwitchStateId tpStateId = DrxAudio::StringToId("3rdP");

template<typename AudioObjectType>
void PrepareForAudioTriggerExecution(AudioObjectType* pIAudioObject, const SMFXAudioEffectParams& audioParams, const SMFXRunTimeEffectParams& runtimeParams)
{
	pIAudioObject->SetSwitchState(switchId,	runtimeParams.playSoundFP ? fpStateId : tpStateId);

	for (auto const& switchWrapper : audioParams.triggerSwitches)
	{
		pIAudioObject->SetSwitchState(switchWrapper.GetSwitchId(), switchWrapper.GetSwitchStateId());
	}

	for (u32 i = 0; i < runtimeParams.numAudioRtpcs; ++i)
	{
		char const* const szParameterName = runtimeParams.audioRtpcs[i].rtpcName;

		if (szParameterName != nullptr && szParameterName[0] != '\0')
		{
			DrxAudio::ControlId const parameterId = DrxAudio::StringToId(szParameterName);
			pIAudioObject->SetParameter(parameterId, runtimeParams.audioRtpcs[i].rtpcValue);
		}
	}
}
} //endns MaterialEffectsUtils

//////////////////////////////////////////////////////////////////////////

void SAudioTriggerWrapper::Init(tukk triggerName)
{
	DRX_ASSERT(triggerName != nullptr);
	m_triggerID = DrxAudio::StringToId(triggerName);

#if defined(MATERIAL_EFFECTS_DEBUG)
	m_triggerName = triggerName;
#endif
}

void SAudioSwitchWrapper::Init(tukk switchName, tukk switchStateName)
{
	DRX_ASSERT(switchName != nullptr);
	DRX_ASSERT(switchStateName != nullptr);
	m_switchID = DrxAudio::StringToId(switchName);
	m_switchStateID = DrxAudio::StringToId(switchStateName);

#if defined(MATERIAL_EFFECTS_DEBUG)
	m_switchName = switchName;
	m_switchStateName = switchStateName;
#endif
}

//////////////////////////////////////////////////////////////////////////

CMFXAudioEffect::CMFXAudioEffect()
	: CMFXEffectBase(eMFXPF_Audio)
{

}

void CMFXAudioEffect::Execute(const SMFXRunTimeEffectParams& params)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	IF_UNLIKELY (!m_audioParams.trigger.IsValid())
		return;

	IEntity* pOwnerEntity = (params.audioProxyEntityId != 0) ? gEnv->pEntitySystem->GetEntity(params.audioProxyEntityId) : nullptr;
	if (pOwnerEntity)
	{
		IEntityAudioComponent* pIEntityAudioComponent = pOwnerEntity->GetOrCreateComponent<IEntityAudioComponent>();
		DRX_ASSERT(pIEntityAudioComponent);

		MaterialEffectsUtils::PrepareForAudioTriggerExecution<IEntityAudioComponent>(pIEntityAudioComponent, m_audioParams, params);

		pIEntityAudioComponent->ExecuteTrigger(m_audioParams.trigger.GetTriggerId(), params.audioProxyId);
	}
	else
	{
		DrxAudio::SCreateObjectData const objectData("MFXAudioEffect", DrxAudio::EOcclusionType::Low, params.pos, INVALID_ENTITYID, true);
		DrxAudio::IObject* const pIObject = gEnv->pAudioSystem->CreateObject(objectData);

		MaterialEffectsUtils::PrepareForAudioTriggerExecution<DrxAudio::IObject>(pIObject, m_audioParams, params);

		pIObject->ExecuteTrigger(m_audioParams.trigger.GetTriggerId());
		gEnv->pAudioSystem->ReleaseObject(pIObject);
	}
}

void CMFXAudioEffect::GetResources(SMFXResourceList& resourceList) const
{
	SMFXAudioListNode* pListNode = SMFXAudioListNode::Create();

	pListNode->m_audioParams.triggerName = m_audioParams.trigger.GetTriggerName();

	const size_t switchesCount = std::min<size_t>(m_audioParams.triggerSwitches.size(), pListNode->m_audioParams.triggerSwitches.max_size());

	for (size_t i = 0; i < switchesCount; ++i)
	{
		IMFXAudioParams::SSwitchData switchData;
		switchData.switchName = m_audioParams.triggerSwitches[i].GetSwitchName();
		switchData.switchStateName = m_audioParams.triggerSwitches[i].GetSwitchStateName();

		pListNode->m_audioParams.triggerSwitches.push_back(switchData);
	}

	SMFXAudioListNode* pNextNode = resourceList.m_audioList;

	if (pNextNode == nullptr)
	{
		resourceList.m_audioList = pListNode;
	}
	else
	{
		while (pNextNode->pNext)
			pNextNode = pNextNode->pNext;

		pNextNode->pNext = pListNode;
	}
}

void CMFXAudioEffect::LoadParamsFromXml(const XmlNodeRef& paramsNode)
{
	// Xml data format
	/*
	   <Audio trigger="footstep">
	   <Switch name="Switch1" state="switch1_state" />
	   <Switch name="Switch2" state="swtich2_state" />
	   <Switch ... />
	   </Audio>
	 */

	m_audioParams.trigger.Init(paramsNode->getAttr("trigger"));

	i32k childCount = paramsNode->getChildCount();
	m_audioParams.triggerSwitches.reserve(childCount);

	for (i32 i = 0; i < childCount; ++i)
	{
		const XmlNodeRef& childNode = paramsNode->getChild(i);

		if (strcmp(childNode->getTag(), "Switch") == 0)
		{
			SAudioSwitchWrapper switchWrapper;
			switchWrapper.Init(childNode->getAttr("name"), childNode->getAttr("state"));
			if (switchWrapper.IsValid())
			{
				m_audioParams.triggerSwitches.push_back(switchWrapper);
			}
#if defined(MATERIAL_EFFECTS_DEBUG)
			else
			{
				GameWarning("[MFX] AudioEffect (at line %d) : Switch '%s' or SwitchState '%s' not valid", paramsNode->getLine(), switchWrapper.GetSwitchName(), switchWrapper.GetSwitchStateName());
			}
#endif
		}
	}

#if defined(MATERIAL_EFFECTS_DEBUG)
	if (!m_audioParams.trigger.IsValid())
	{
		GameWarning("[MFX] AudioEffect (at line %d) : Trigger '%s'not valid", paramsNode->getLine(), m_audioParams.trigger.GetTriggerName());
	}
#endif
}

void CMFXAudioEffect::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(&m_audioParams);
}
