// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
Network-syncs the actor entity ID being targeted by a player
**************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PlayerPlugin_CurrentlyTargetting.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/GameCVars.h>
#include <drx3D/Game/Utility/DrxWatch.h>
#include <drx3D/Game/IWorldQuery.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/Battlechatter.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/UI/HUD/HUDEventWrapper.h>
#include <drx3D/Game/PlayerPluginEventDistributor.h>


CPlayerPlugin_CurrentlyTargetting::CPlayerPlugin_CurrentlyTargetting()
{
	m_currentTarget = 0;
	m_currentTargetTime = 0.0f;
	m_bTargetingLocalPlayer = false;
	m_targetedSignal.SetSignal("SniperCountermeasuresTargetted");
}

void CPlayerPlugin_CurrentlyTargetting::Enter( CPlayerPluginEventDistributor* pEventDist )
{
	CPlayerPlugin::Enter(pEventDist);

	pEventDist->RegisterEvent(this, EPE_Reset);
}

void CPlayerPlugin_CurrentlyTargetting::Leave()
{
	m_currentTarget = 0;
	m_currentTargetTime = 0.0f;

	if(m_bTargetingLocalPlayer)
	{
		SHUDEvent event (eHUDEvent_LocalPlayerTargeted);
		event.AddData(false);
		CHUDEventDispatcher::CallEvent(event);

		REINST("needs verification!");
		/*EntityId clientId = g_pGame->GetClientActorId();
		if(clientId && m_targetedSignal.IsPlaying(clientId))
		{
			m_targetedSignal.Stop(clientId);
		}*/
	}

	CPlayerPlugin::Leave();
}

void CPlayerPlugin_CurrentlyTargetting::Update(float dt)
{
	m_currentTargetTime += dt;	//updated locally for all players (so doesn't have to be synced)

	assert (IsEntered());
	if (m_ownerPlayer->IsClient())
	{
		EntityId newTargetId = !m_ownerPlayer->IsDead() ? m_ownerPlayer->GetGameObject()->GetWorldQuery()->GetLookAtEntityId() : 0;

		if (newTargetId)
		{
			IActor * targettedActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(newTargetId);
			if (targettedActor == NULL || targettedActor->IsDead())
			{
				newTargetId = 0;
			}
		}

		if (m_currentTarget != newTargetId)
		{
			m_currentTarget = newTargetId;

#if 0 //PLAYER_PLUGIN_DEBUGGING
			// Quite spammy, feel free to re-enable locally
			if (gEnv->bMultiplayer)
			{
				IEntity * entity = gEnv->pEntitySystem->GetEntity(m_currentTarget);
				DrxLog("[TARGETING] Local %s %s is now targeting e%05d (%s %s)", m_ownerPlayer->GetEntity()->GetClass()->GetName(), m_ownerPlayer->GetEntity()->GetName(), m_currentTarget, entity ? entity->GetName() : "NULL", entity ? entity->GetClass()->GetName() : "entity");
			}
#endif

			CCCPOINT_IF(m_currentTarget, PlayerState_LocalPlayerNowTargettingSomebody);
			CCCPOINT_IF(!m_currentTarget, PlayerState_LocalPlayerNowTargettingNobody);

			m_currentTargetTime = 0.0f;
			CHANGED_NETWORK_STATE(m_ownerPlayer, CPlayer::ASPECT_CURRENTLYTARGETTING_CLIENT);
		}
	}

#if PLAYER_PLUGIN_DEBUGGING
	IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_currentTarget);
	PlayerPluginWatch ("Target e%05d (%s %s) - %.2f", m_currentTarget, pEntity ? pEntity->GetName() : "NULL", pEntity ? pEntity->GetClass()->GetName() : "entity", m_currentTargetTime);
#endif
}

void CPlayerPlugin_CurrentlyTargetting::NetSerialize(TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags)
{
	if(aspect == CPlayer::ASPECT_CURRENTLYTARGETTING_CLIENT)
	{
		NET_PROFILE_SCOPE("CurrentlyTargeting", ser.IsReading());

		EntityId previousTarget = m_currentTarget;
		ser.Value("curTargetId", m_currentTarget, 'eid');

		if(ser.IsReading())
		{
			if(m_currentTarget != previousTarget)
			{
#if 0 // PLAYER_PLUGIN_DEBUGGING
			// Quite spammy, feel free to re-enable locally
				if (gEnv->bMultiplayer)
				{
					IEntity * entity = gEnv->pEntitySystem->GetEntity(m_currentTarget);
					DrxLog("[TARGETING] Remote %s %s is now targeting e%05d (%s %s)", m_ownerPlayer->GetEntity()->GetClass()->GetName(), m_ownerPlayer->GetEntity()->GetName(), m_currentTarget, entity ? entity->GetName() : "NULL", entity ? entity->GetClass()->GetName() : "entity");
				}
#endif

				CCCPOINT_IF(m_currentTarget, PlayerState_RemotePlayerNowTargettingSomebody);
				CCCPOINT_IF(!m_currentTarget, PlayerState_RemotePlayerNowTargettingNobody);

				m_currentTargetTime = 0.0f;

				CGameRules *pGameRules = g_pGame->GetGameRules();

				const EntityId clientActorId = g_pGame->GetClientActorId();
				if (m_bTargetingLocalPlayer)
				{
					SHUDEvent event (eHUDEvent_LocalPlayerTargeted);
					event.AddData(false);
					CHUDEventDispatcher::CallEvent(event);

					m_bTargetingLocalPlayer = false;
					//m_targetedSignal.Stop(clientActorId);
				}
			}
		}
	}
}

void CPlayerPlugin_CurrentlyTargetting::HandleEvent(EPlayerPlugInEvent theEvent, uk  data)
{
	CPlayerPlugin::HandleEvent(theEvent, data);

	switch (theEvent)
	{
		case EPE_Reset:
		if (m_ownerPlayer->IsClient())
		{
			assert (m_currentTarget == 0);
		}
		break;
	}
}


