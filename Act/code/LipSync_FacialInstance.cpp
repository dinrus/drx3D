// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/LipSync_FacialInstance.h>

//=============================================================================
//
// CLipSyncProvider_FacialInstance
//
//=============================================================================

CLipSyncProvider_FacialInstance::CLipSyncProvider_FacialInstance(EntityId entityId)
	: m_entityId(entityId)
{
}

void CLipSyncProvider_FacialInstance::RequestLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	// actually facial sequence is triggered in OnSoundEvent SOUND_EVENT_ON_PLAYBACK_STARTED of the CSoundProxy
	// when playback is started, it will start facial sequence as well
}

void CLipSyncProvider_FacialInstance::StartLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId);
	}
}

void CLipSyncProvider_FacialInstance::PauseLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId, true);
	}
}

void CLipSyncProvider_FacialInstance::UnpauseLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId);
	}
}

void CLipSyncProvider_FacialInstance::StopLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
	if (lipSyncMethod != eLSM_None)
	{
		LipSyncWithSound(audioTriggerId, true);
	}
}

void CLipSyncProvider_FacialInstance::UpdateLipSync(IEntityAudioComponent* pProxy, const DrxAudio::ControlId audioTriggerId, const ELipSyncMethod lipSyncMethod)
{
}

void CLipSyncProvider_FacialInstance::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("LipSyncProvider_FacialInstance");
	ser.EndGroup();
}

void CLipSyncProvider_FacialInstance::LipSyncWithSound(const DrxAudio::ControlId audioTriggerId, bool bStop /*= false*/)
{
	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_entityId))
	{
		if (ICharacterInstance* pChar = pEntity->GetCharacter(0))
		{
			if (IFacialInstance* pFacial = pChar->GetFacialInstance())
			{
				pFacial->LipSyncWithSound(audioTriggerId, bStop);
			}
		}
	}
}

//=============================================================================
//
// CLipSync_FacialInstance
//
//=============================================================================

void CLipSync_FacialInstance::InjectLipSyncProvider()
{
	IEntity* pEntity = GetEntity();
	IEntityAudioComponent* pSoundProxy = pEntity->GetOrCreateComponent<IEntityAudioComponent>();
	DRX_ASSERT(pSoundProxy);
	m_pLipSyncProvider.reset(new CLipSyncProvider_FacialInstance(pEntity->GetId()));
	REINST(add SetLipSyncProvider to interface)
	//pSoundProxy->SetLipSyncProvider(m_pLipSyncProvider);
}

void CLipSync_FacialInstance::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	if (m_pLipSyncProvider)
	{
		pSizer->Add(*m_pLipSyncProvider);
	}
}

bool CLipSync_FacialInstance::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);
	return true;
}

void CLipSync_FacialInstance::PostInit(IGameObject* pGameObject)
{
	InjectLipSyncProvider();
}

void CLipSync_FacialInstance::InitClient(i32 channelId)
{
}

void CLipSync_FacialInstance::PostInitClient(i32 channelId)
{
}

bool CLipSync_FacialInstance::ReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params)
{
	ResetGameObject();
	return true;
}

void CLipSync_FacialInstance::PostReloadExtension(IGameObject* pGameObject, const SEntitySpawnParams& params)
{
	InjectLipSyncProvider();
}

void CLipSync_FacialInstance::FullSerialize(TSerialize ser)
{
	ser.BeginGroup("LipSync_FacialInstance");

	bool bLipSyncProviderIsInjected = (m_pLipSyncProvider != NULL);
	ser.Value("bLipSyncProviderIsInjected", bLipSyncProviderIsInjected);
	if (bLipSyncProviderIsInjected && !m_pLipSyncProvider)
	{
		DRX_ASSERT(ser.IsReading());
		InjectLipSyncProvider();
	}
	if (m_pLipSyncProvider)
	{
		m_pLipSyncProvider->FullSerialize(ser);
	}

	ser.EndGroup();
}

bool CLipSync_FacialInstance::NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 pflags)
{
	return true;
}

void CLipSync_FacialInstance::PostSerialize()
{
}

void CLipSync_FacialInstance::SerializeSpawnInfo(TSerialize ser)
{
}

ISerializableInfoPtr CLipSync_FacialInstance::GetSpawnInfo()
{
	return NULL;
}

void CLipSync_FacialInstance::Update(SEntityUpdateContext& ctx, i32 updateSlot)
{
}

void CLipSync_FacialInstance::HandleEvent(const SGameObjectEvent& event)
{
}

void CLipSync_FacialInstance::ProcessEvent(const SEntityEvent& event)
{
}

uint64 CLipSync_FacialInstance::GetEventMask() const
{
	return 0;
}

void CLipSync_FacialInstance::SetChannelId(u16 id)
{
}

void CLipSync_FacialInstance::PostUpdate(float frameTime)
{
}

void CLipSync_FacialInstance::PostRemoteSpawn()
{
}

void CLipSync_FacialInstance::OnShutDown()
{
	IEntity* pEntity = GetEntity();
	if (IEntityAudioComponent* pSoundProxy = pEntity->GetComponent<IEntityAudioComponent>())
	{
		REINST(add SetLipSyncProvider to interface)
		//pSoundProxy->SetLipSyncProvider(ILipSyncProviderPtr());
	}
}
