// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/GameContext.h>
#include <drx3D/Act/VoiceListener.h>
#include <drx3D/Act/DinrusAction.h>
#include <drx3D/Act/NetworkCVars.h>

#ifndef OLD_VOICE_SYSTEM_DEPRECATED

	#define DUMP_RECEIVED_VOICE 0

CVoiceListener::CVoiceListener()
	: m_pSound(0)
	, m_min2dDistance(15.0f)
	, m_max3dDistance(18.0f)
	, m_3dPan(1.0f)
{
}

CVoiceListener::~CVoiceListener()
{
	if (m_pSound)
	{
		m_pSound->RemoveEventListener(this);
		m_pSound->Stop();
	}
}

bool CVoiceListener::Init(IGameObject* pGameObject)
{
	SetGameObject(pGameObject);

	StartPlaying(true);

	return true;
}

void CVoiceListener::PostInit(IGameObject* pGameObject)
{
	pGameObject->EnableUpdateSlot(this, 0);
	pGameObject->SetUpdateSlotEnableCondition(this, 0, eUEC_InRange);
}

void CVoiceListener::InitClient(i32 channelId)
{
}

void CVoiceListener::PostInitClient(i32 channelId)
{
}

void CVoiceListener::Release()
{
	delete this;
}

void CVoiceListener::Update(SEntityUpdateContext& ctx, i32 updateSlot)
{
	// this will happen whenever entity is within range (150m).
	UpdateSound3dPan();
}

void CVoiceListener::HandleEvent(const SGameObjectEvent& event)
{
}

void CVoiceListener::ProcessEvent(const SEntityEvent& evt)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (evt.event != ENTITY_EVENT_START_GAME || m_pSound)
		return;

	StartPlaying(false);
}

void CVoiceListener::StartPlaying(bool checkStarted)
{
	m_pVoiceContext = CDrxAction::GetDrxAction()->GetGameContext()->GetNetContext()->GetVoiceContext();

	if (!m_pVoiceContext)
		return;

	if (!m_pVoiceContext->IsEnabled())
		return;

	if (checkStarted)
		if (!CDrxAction::GetDrxAction()->IsGameStarted())
			return;

	// TODO: figure out the correct parameters with the network system...

	m_pSound = gEnv->pAudioSystem->CreateNetworkSound(this, 16, 8000, 160 * 10, GetEntityId());
	if (!m_pSound)
	{
		//DrxLog("Listener failed to create sound");
		return;
	}

	m_pSound->AddEventListener(this, "VoiceListener");

	IEntityAudioComponent* pIEntityAudioComponent = GetGameObject()->GetEntity()->GetOrCreateComponent<IEntityAudioComponent>();

	if (pIEntityAudioComponent)
		pIEntityAudioComponent->PlaySound(m_pSound);
	//pIEntityAudioComponent->PlaySound(m_pSound,Vec3(0.0f,0.0f,1.0f),FORWARD_DIRECTION,1.0f);

	IActor* act = CDrxAction::GetDrxAction()->GetClientActor();
	if (act && (act->GetEntityId() != GetGameObject()->GetEntityId()))
	{
		m_pSound->GetInterfaceExtended()->SetFlags(m_pSound->GetFlags() | FLAG_SOUND_3D);// | FLAG_SOUND_RADIUS);
		//		m_pSound->SetMinMaxDistance(1.5f,30.0f);
		//		m_pSound->Set3DPan(1.0f);
	}

	UpdateSound3dPan();
}

void CVoiceListener::PostUpdate(float frameTime)
{
	DRX_ASSERT(false);
}

bool CVoiceListener::FillDataBuffer(u32 nBitsPerSample, u32 nSamplesPerSecond, u32 nNumSamples, uk pData)
{
	if (!m_pVoiceContext)
		return false;

	//request decoder for sound data
	if (m_pVoiceContext->GetDataFor(GetEntityId(), nNumSamples, (i16*)pData))
	{
		// keep sound volume correct
		if (m_pSound)
			m_pSound->GetInterfaceExtended()->SetVolume(CNetworkCVars::Get().VoiceVolume);
	}
	else
		memset(pData, 0, nNumSamples * sizeof(i16));

	return true;
}

void CVoiceListener::OnActivate(bool active)
{
	if (!m_pVoiceContext)
		return;
	m_pVoiceContext->PauseDecodingFor(GetEntityId(), !active);
}

void CVoiceListener::SetSoundPlaybackDistances(float max3dDistance, float min2dDistance)
{
	DRX_ASSERT(max3dDistance >= min2dDistance);

	m_max3dDistance = max3dDistance;
	m_min2dDistance = min2dDistance;
}

void CVoiceListener::UpdateSound3dPan()
{
	// if player is still talking, don't change their sound
	INetChannel* pChannel = CDrxAction::GetDrxAction()->GetClientChannel();
	if (pChannel)
	{
		CTimeValue tm = pChannel->TimeSinceVoiceReceipt(GetEntity()->GetId());
		if (tm.GetSeconds() < 0.5f)
		{
			return;
		}
	}

	// find the distance of the source entity from the local player, and compare to
	// max3d and min2d distances
	if (!m_pSound)
		return;
	float new3dpan = m_pSound->GetInterfaceExtended()->Get3DPan();

	// TEMP TEMP TEMP until 3dpan is working correctly
	//	assume players are far apart always

	Vec3 sourcePos = GetEntity()->GetWorldPos();
	IActor* playerActor = gEnv->pGameFramework->GetClientActor();
	if (playerActor)
	{
		//    Vec3 playerPos = playerActor->GetEntity()->GetWorldPos();

		//    Vec3 dir = playerPos - sourcePos;
		//    float dist = dir.GetLength();

		//    if(dist < m_min2dDistance)
		//      new3dpan = 1.0f;
		//    else if(dist > m_max3dDistance)
		//      new3dpan = 0.0f;

		new3dpan = 0.0f;

		//    if(new3dpan != m_3dPan)
		{
			m_3dPan = new3dpan;
			//      m_pSound->Set3DPan(new3dpan);

			u32 flags = m_pSound->GetFlags();
			if (new3dpan > 0.5f)
			{
				flags &= ~FLAG_SOUND_RELATIVE;
			}
			else
			{
				flags |= FLAG_SOUND_RELATIVE;

				// disable reverb for that sound
				float fReverb = 0.0f;
				ptParamF32 fParam(fReverb);
				m_pSound->SetParam(spREVERBWET, &fParam);
			}
			m_pSound->GetInterfaceExtended()->SetFlags(flags);
		}
	}
}

void CVoiceListener::OnSoundEvent(ESoundCallbackEvent event, ISound* pSound)
{
	DRX_ASSERT(pSound == m_pSound);

	switch (event)
	{
	case SOUND_EVENT_ON_PLAYBACK_STARTED:
		//		DrxLog("Activating listener");
		OnActivate(true);
		break;

	case SOUND_EVENT_ON_PLAYBACK_STOPPED:
		//		DrxLog("Deactivating listener");
		OnActivate(false);
		break;
	}
}
#endif
