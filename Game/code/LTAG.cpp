// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------

Описание: LTAG Implementation

-------------------------------------------------------------------------
История:
- 16:09:09	: Created by Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/LTAG.h>

#include <drx3D/Game/Game.h>
#include <drx3D/Game/GameActions.h>
#include <drx3D/Game/GameInputActionHandlers.h>
#include <drx3D/Game/LTagSingle.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/UI/HUD/HUDEventDispatcher.h>
#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>


CLTag::CLTag()
{
	CGameInputActionHandlers::TLTagActionHandler& ltagActionHandler = g_pGame->GetGameInputActionHandlers().GetCLtagActionHandler();
	
	if(ltagActionHandler.GetNumHandlers() == 0)
	{
		const CGameActions& actions = g_pGame->Actions();
		ltagActionHandler.AddHandler(actions.weapon_change_firemode, &CLTag::OnActionSwitchFireMode);
	}
}


CLTag::~CLTag()
{

}

void CLTag::StartFire(const SProjectileLaunchParams& launchParams)
{
	if (m_fm)
	{
		m_fm->SetProjectileLaunchParams(launchParams);

		CWeapon::StartFire();
	}
}

void CLTag::OnAction(EntityId actorId, const ActionId& actionId, i32 activationMode, float value)
{
	CGameInputActionHandlers::TLTagActionHandler& ltagActionHandler = g_pGame->GetGameInputActionHandlers().GetCLtagActionHandler();

	if(!ltagActionHandler.Dispatch(this,actorId,actionId,activationMode,value))
	{
		CWeapon::OnAction(actorId, actionId, activationMode, value);
	}
}

bool CLTag::OnActionSwitchFireMode(EntityId actorId, const ActionId& actionId, i32 activationMode, float value)
{
	if (activationMode != eAAM_OnPress)
		return true;
	StartChangeFireMode();
	return true;
}

void CLTag::ProcessEvent(SEntityEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	if (event.event == ENTITY_EVENT_ANIM_EVENT)
	{
		const AnimEventInstance* pAnimEvent = reinterpret_cast<const AnimEventInstance*>(event.nParam[0]);
		ICharacterInstance* pCharacter = reinterpret_cast<ICharacterInstance*>(event.nParam[1]);
		if (pAnimEvent && pCharacter)
		{
			AnimationEvent(pCharacter, *pAnimEvent);
		}
	}
	else
	{
		inherited::ProcessEvent(event);
	}
}

void CLTag::StartChangeFireMode()
{
	if (m_fm == NULL)
		return;

	assert(crygti_isof<CLTagSingle>(m_fm));

	CLTagSingle* pLTagFireMode = static_cast<CLTagSingle*>(m_fm);
	bool changed = pLTagFireMode->NextGrenadeType();

	if(changed)
	{
		CWeapon::OnFireModeChanged(m_firemode);

		SHUDEvent event(eHUDEvent_OnWeaponFireModeChanged);
		event.AddData(SHUDEventData(m_firemode));
		CHUDEventDispatcher::CallEvent(event);
	}
}

void CLTag::AnimationEvent( ICharacterInstance *pCharacter, const AnimEventInstance &event )
{
	if(s_animationEventsTable.m_ltagUpdateGrenades == event.m_EventNameLowercaseCRC32)
	{
		UpdateGrenades();
	}
}

void CLTag::UpdateGrenades()
{
	if (!m_fm)
		return;

	ICharacterInstance* pWeaponCharacter = GetEntity()->GetCharacter(eIGS_FirstPerson);
	if (!pWeaponCharacter)
		return;

	tukk newChell = "newShell";
	tukk currentShell = "currentShell";
	i32k ammoCount = GetAmmoCount(m_fm->GetAmmoType());

	HideGrenadeAttachment(pWeaponCharacter, currentShell, ammoCount <= 2);
	HideGrenadeAttachment(pWeaponCharacter, newChell, ammoCount <= 1);
}

void CLTag::HideGrenadeAttachment( ICharacterInstance* pWeaponCharacter, tukk attachmentName, bool hide )
{
	DRX_ASSERT(pWeaponCharacter);

	CCCPOINT_IF(string("newShell") == attachmentName && hide, ltag_hide_newShell);
	CCCPOINT_IF(string("newShell") == attachmentName && !hide, ltag_show_newShell);
	CCCPOINT_IF(string("currentShell") == attachmentName && hide, ltag_hide_currentShell);
	CCCPOINT_IF(string("currentShell") == attachmentName && !hide, ltag_show_currentShell);

	IAttachment* pAttachment = pWeaponCharacter->GetIAttachmentUpr()->GetInterfaceByName(attachmentName);
	if (pAttachment)
	{
		pAttachment->HideAttachment(hide ? 1 : 0);
	}
}

void CLTag::Reset()
{
	inherited::Reset();

	UpdateGrenades();
}

void CLTag::FullSerialize( TSerialize ser )
{
	inherited::FullSerialize(ser);

	if (ser.IsReading())
	{
		UpdateGrenades();
	}

}

void CLTag::OnSelected( bool selected )
{
	inherited::OnSelected(selected);
	UpdateGrenades();
}
//CA: MP design have chosen to have only one grenade type so this is no longer necessary, and there were problems with delegate authority
//Leaving here for now in case the fickle designers change their mind again
/*
void CLTag::Select(bool select)
{
	inherited::Select(select);

	CActor* pOwner = GetOwnerActor();

	if (select && gEnv->bServer && pOwner && pOwner->IsPlayer())
	{
		INetContext *pNetContext = g_pGame->GetIGameFramework()->GetNetContext();	

		if (pNetContext)        
			pNetContext->DelegateAuthority(GetEntityId(), pOwner->GetGameObject()->GetNetChannel());     
	}   
}

bool CLTag::NetSerialize( TSerialize ser, EEntityAspects aspect, u8 profile, i32 flags )
{
	//TODO: FIX ME! Yuck, optional group is bad, but firemode can be null. Need a proper solution to client firemode serialisation
	bool optional = (m_fm && (strcmp(m_fm->GetType(), CLTagSingle::GetWeaponComponentType() == 0));
	
	if(ser.BeginOptionalGroup("firemode", optional))
	{
		CLTagSingle* pLTagFireMode = static_cast<CLTagSingle*>(m_fm);
		pLTagFireMode->NetSerialize(ser, aspect, profile, flags);
	}
	
	return inherited::NetSerialize(ser, aspect, profile, flags);
}
*/
