// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/ScriptBind_Turret.h>

#include <drx3D/Game/Turret.h>
#include <drx3D/Game/TurretBehaviorEvents.h>

#include <drx3D/Game/GameRules.h>
#include <drx3D/AI/IAIObject.h>


#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Turret::


namespace
{
	CTurret* GetTurret( IFunctionHandler* pH )
	{
		return static_cast< CTurret* >( pH->GetThis() );
	}
}


CScriptBind_Turret::CScriptBind_Turret( ISystem* pSystem )
{
	assert( pSystem != NULL );

	IScriptSystem* pScriptSystem = pSystem->GetIScriptSystem();
	Init( pScriptSystem, pSystem, 1 );

	SCRIPT_REG_TEMPLFUNC( Enable, "" );
	SCRIPT_REG_TEMPLFUNC( Disable, "" );

	SCRIPT_REG_TEMPLFUNC( OnPropertyChange, "" );
	SCRIPT_REG_TEMPLFUNC( OnHit, "hitScriptTable" );
	SCRIPT_REG_TEMPLFUNC( SetStateById, "stateId" );
	SCRIPT_REG_TEMPLFUNC( SetFactionToPlayerFaction, "" );

	SCRIPT_REG_GLOBAL( eTurretBehaviorState_Undeployed );
	SCRIPT_REG_GLOBAL( eTurretBehaviorState_PartiallyDeployed );
	SCRIPT_REG_GLOBAL( eTurretBehaviorState_Deployed );
}


CScriptBind_Turret::~CScriptBind_Turret()
{

}


void CScriptBind_Turret::AttachTo( CTurret* pTurret )
{
	IEntity* const pEntity = pTurret->GetEntity();
	IScriptTable* const pScriptTable = pEntity->GetScriptTable();
	IF_UNLIKELY ( pScriptTable == NULL )
	{
		return;
	}

	SmartScriptTable thisTable( m_pSS );

	thisTable->SetValue( "__this", ScriptHandle( pTurret ) );
	
	IScriptTable* const pMethodsTable = GetMethodsTable();
	thisTable->Delegate( pMethodsTable );

	pScriptTable->SetValue( "turret", thisTable );
}


void CScriptBind_Turret::DettachFrom( CTurret* pTurret )
{
	IEntity* const pEntity = pTurret->GetEntity();
	IScriptTable* const pScriptTable = pEntity->GetScriptTable();
	IF_UNLIKELY ( pScriptTable == NULL )
	{
		return;
	}
	
	pScriptTable->SetToNull( "turret" );
}


i32 CScriptBind_Turret::Enable( IFunctionHandler* pH )
{
	CTurret* const pTurret = GetTurret( pH );
	IF_UNLIKELY ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	pTurret->Enable();
	return pH->EndFunction();
}


i32 CScriptBind_Turret::Disable( IFunctionHandler* pH )
{
	CTurret* const pTurret = GetTurret( pH );
	IF_UNLIKELY ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	pTurret->Disable();
	return pH->EndFunction();
}


i32 CScriptBind_Turret::OnPropertyChange( IFunctionHandler* pH )
{
	CTurret* const pTurret = GetTurret( pH );
	IF_UNLIKELY ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	pTurret->OnPropertyChange();
	return pH->EndFunction();
}


i32 CScriptBind_Turret::OnHit( IFunctionHandler* pH, SmartScriptTable scriptHitInfo )
{
	CTurret* const pTurret = GetTurret( pH );
	if ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	HitInfo hitInfo;
	CGameRules::CreateHitInfoFromScript( scriptHitInfo, hitInfo );

	pTurret->OnHit( hitInfo );
	return pH->EndFunction();
}


i32 CScriptBind_Turret::SetStateById( IFunctionHandler* pH, i32 stateId )
{
	CTurret* const pTurret = GetTurret( pH );
	IF_UNLIKELY ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	IF_UNLIKELY ( stateId < 0 )
	{
		return pH->EndFunction();;
	}

	IF_UNLIKELY ( eTurretBehaviorState_Count <= stateId )
	{
		return pH->EndFunction();;
	}

	pTurret->SetStateById( static_cast< ETurretBehaviorState >( stateId ) );
	return pH->EndFunction();
}


i32 CScriptBind_Turret::SetFactionToPlayerFaction( IFunctionHandler* pH )
{
	CTurret* const pTurret = GetTurret( pH );
	IF_UNLIKELY ( pTurret == NULL )
	{
		return pH->EndFunction();
	}

	const EntityId localPlayerEntityId = g_pGame->GetIGameFramework()->GetClientActorId();
	IEntity* const pLocalPlayerEntity = gEnv->pEntitySystem->GetEntity( localPlayerEntityId );
	IF_UNLIKELY ( pLocalPlayerEntity == NULL )
	{
		return  pH->EndFunction();
	}

	IAIObject* const pPlayerAiObject = pLocalPlayerEntity->GetAI();
	IF_UNLIKELY ( pPlayerAiObject == NULL )
	{
		return  pH->EndFunction();
	}

	u8k playerFactionId = pPlayerAiObject->GetFactionID();
	pTurret->SetFactionId( playerFactionId );
	return pH->EndFunction();
}
