// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Implements the Player ledge state

-------------------------------------------------------------------------
История:
- 14.9.10: Created by Stephen M. North

*************************************************************************/
#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PlayerStateDead.h>
#include <drx3D/Game/Player.h>
#include <drx3D/Game/IPlayerEventListener.h>

#include <drx3D/Game/HitDeathReactions.h>
#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/Audio/Announcer.h>
#include <drx3D/Game/ActorImpulseHandler.h>
#include <drx3D/Game/PickAndThrowProxy.h>

#include <drx3D/Game/AI/AICorpse.h>

CPlayerStateDead::CPlayerStateDead()
	: m_swapToCorpseTimeout(10.0f)
	, m_corpseUpdateStatus(eCorpseStatus_WaitingForSwap)
{
}

CPlayerStateDead::~CPlayerStateDead()
{
}

void CPlayerStateDead::OnEnter( CPlayer& player )
{
	player.StopLoopingSounds();

	player.SetDeathTimer();

	if( player.m_pPickAndThrowProxy )
	{
		player.m_pPickAndThrowProxy->Unphysicalize();
	}

	const bool bIsClient = player.IsClient();

	if(gEnv->bMultiplayer)
	{
		if(bIsClient)
			CAudioSignalPlayer::JustPlay("PlayerDeath_MP_Marine", player.GetEntityId());
	}

	CGameRules* pGameRules = g_pGame->GetGameRules();

	if(bIsClient)
	{
		player.SetClientSoundmood(CPlayer::ESoundmood_Dead);

		const bool dropHeavyWeapon = true;
		player.SetBackToNormalWeapon(dropHeavyWeapon);

		bool  itemIsUsed = false;
		bool  itemIsDroppable = false;
		if(!gEnv->bMultiplayer || (pGameRules->GetCurrentItemForActorWithStatus(&player, &itemIsUsed, &itemIsDroppable) && !itemIsDroppable))
		{
			player.HolsterItem(true);
		}

		CCCPOINT(PlayerState_LocalPlayerDied);
	}
	else
	{
		CCCPOINT(PlayerState_NonLocalPlayerDied);
	}

	// report normal death
	if (player.m_playerEventListeners.empty() == false)
	{
		CPlayer::TPlayerEventListeners::const_iterator iter = player.m_playerEventListeners.begin();
		CPlayer::TPlayerEventListeners::const_iterator cur;
		while (iter != player.m_playerEventListeners.end())
		{
			cur = iter;
			++iter;
			(*cur)->OnDeath(&player, false);
		}
	}

	pGameRules->OnActorDeath( &player );
	if (gEnv->bMultiplayer == false && player.IsClient())
	{
		g_pGame->GetIGameFramework()->GetIGameplayRecorder()->Event(player.GetEntity(), eGE_Death);
	}

	m_swapToCorpseTimeout = g_pGameCVars->g_aiCorpses_DelayTimeToSwap;
	m_corpseUpdateStatus = eCorpseStatus_WaitingForSwap;
}

void CPlayerStateDead::OnLeave( CPlayer& player )
{
	player.GetEntity()->KillTimer( RECYCLE_AI_ACTOR_TIMER_ID );
}

void CPlayerStateDead::OnPrePhysicsUpdate( CPlayer& player, const SActorFrameMovementParams& movement, float frameTime ) 
{
	// noone has any idea why this is needed but without the player animation doesnt execute correctly.
	if ( player.IsClient() )
	{
		//--- STAP must be updated here to ensure that the most recent values are there for the 
		//--- animation processing pass
		const Vec3 cameraPositionForTorso = player.GetFPCameraPosition(false);
		player.UpdateFPIKTorso(frameTime, player.GetCurrentItem(), cameraPositionForTorso);
	}
}

void CPlayerStateDead::OnUpdate( CPlayer& player, const CPlayerStateDead::UpdateCtx& updateCtx )
{
	if (player.m_pHitDeathReactions)
	{
		player.m_pHitDeathReactions->Update(updateCtx.frameTime);
	}

	if (player.IsClient())
	{
		// Need to create this even when player is dead, otherwise spectators don't see tank turrets rotate etc until they spawn in.
		player.UpdateClient( updateCtx.frameTime );
	}	
	else if (gEnv->bMultiplayer == false)
	{
		UpdateAICorpseStatus( player, updateCtx );
	}

	if (player.GetImpulseHander())
	{
		player.GetImpulseHander()->UpdateDeath(updateCtx.frameTime);
	}

	player.UpdateBodyDestruction(updateCtx.frameTime);
	player.GetDamageEffectController().UpdateEffects(updateCtx.frameTime);
}

void CPlayerStateDead::Serialize( TSerialize& serializer )
{
	i32 corpseUpdateStatus = m_corpseUpdateStatus;

	serializer.Value( "AICorpseUpdateStatus", corpseUpdateStatus );
	serializer.Value( "AICorpseSwapTimeOut", m_swapToCorpseTimeout );

	if (serializer.IsReading())
	{
		m_corpseUpdateStatus = (EAICorpseUpdateStatus)corpseUpdateStatus;
	}
}

void CPlayerStateDead::UpdateAICorpseStatus( CPlayer& player, const CPlayerStateDead::UpdateCtx& updateCtx )
{
	const EAICorpseUpdateStatus updateStatus = m_corpseUpdateStatus;

	if (updateStatus == eCorpseStatus_WaitingForSwap)
	{
		const float newForcedRemovalTimeOut = m_swapToCorpseTimeout - updateCtx.frameTime;
		m_swapToCorpseTimeout = newForcedRemovalTimeOut;

		const	bool removeCorpse = (newForcedRemovalTimeOut < 0.0f);
		if (removeCorpse)
		{
			if(player.GetEntity()->IsHidden() == false)
			{
				CAICorpseUpr* pAICorpseUpr = CAICorpseUpr::GetInstance();
				if(pAICorpseUpr != NULL)
				{
					CAICorpseUpr::SCorpseParameters corpseParams;
					corpseParams.priority = CAICorpseUpr::GetPriorityForClass( player.GetEntity()->GetClass() );
					pAICorpseUpr->SpawnAICorpseFromEntity( *player.GetEntity(), corpseParams );
				}

				// Hide after spawning the corpse to make sure it is properly recreated
				player.GetEntity()->Hide( true );

				IAnimatedCharacter *pAnimatedCharacter = player.GetAnimatedCharacter();
				if (pAnimatedCharacter)
				{
					pAnimatedCharacter->UpdateCharacterPtrs();
				}
			}

			// Schedule recycling (doing during update might end in a crash)
			player.GetEntity()->SetTimer( RECYCLE_AI_ACTOR_TIMER_ID, 1000 );

			m_corpseUpdateStatus = eCorpseStatus_SwapDone;
		}
	}
}


