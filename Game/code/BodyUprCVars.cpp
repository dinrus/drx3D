// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: 

-------------------------------------------------------------------------
История:
- 27:07:2010   Extracted from BodyDamage.h/.cpp by Benito Gangoso Rodriguez

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/BodyUprCVars.h>
#include <drx3D/Game/Actor.h>
#include <drx3D/Game/BodyUpr.h>

i32 CBodyUprCVars::g_bodyDamage_log = 0;
i32 CBodyUprCVars::g_bodyDestruction_debug = 0;
ICVar* CBodyUprCVars::g_bodyDestruction_debugFilter = NULL;

void CBodyUprCVars::RegisterCommands()
{
	REGISTER_COMMAND("g_bodyDamage_reload", ReloadBodyDamage, VF_CHEAT, "Reloads bodyDamage for the specified actor, or for everyone if not specified");
	REGISTER_COMMAND("g_bodyDestruction_reload", ReloadBodyDestruction, VF_CHEAT, "Reloads all body destruction data files");
}

void CBodyUprCVars::UnregisterCommands(IConsole* pConsole)
{
	if (pConsole)
	{
		pConsole->RemoveCommand("g_bodyDamage_reload");
		pConsole->RemoveCommand("g_bodyDestruction_reload");
	}
}

void CBodyUprCVars::RegisterVariables()
{
	REGISTER_CVAR(g_bodyDamage_log, 0, 0, "Enables/Disables BodyDamage logging");
	REGISTER_CVAR(g_bodyDestruction_debug, 0, 0, "Enables/Disables BodyDestruction Debug info");

	g_bodyDestruction_debugFilter = REGISTER_STRING("g_bodyDestruction_debugFilter","",VF_CHEAT,"");
	
	DRX_ASSERT(g_bodyDestruction_debugFilter);
}

void CBodyUprCVars::UnregisterVariables(IConsole* pConsole)
{
	if (pConsole)
	{
		pConsole->UnregisterVariable("g_bodyDamage_log", true);
		pConsole->UnregisterVariable("g_bodyDestruction_debug", true);
		pConsole->UnregisterVariable("g_bodyDestruction_debugFilter", true);
	}
	g_bodyDestruction_debugFilter = NULL;
}

void CBodyUprCVars::Reload(IActor* actor)
{
	CActor* pActor = static_cast<CActor*>(actor);
	if (pActor)
	{
		CBodyDamageUpr *pBodyDamageUpr = g_pGame->GetBodyDamageUpr();
		assert(pBodyDamageUpr);

		pBodyDamageUpr->ReloadBodyDamage(*pActor);
	}
}

void CBodyUprCVars::Reload(IEntity* pEntity)
{
	if (pEntity)
	{
		IActor* pActor = g_pGame->GetIGameFramework()->GetIActorSystem()->GetActor(pEntity->GetId());
		if (pActor)
		{
			Reload(pActor);
		}
		else
		{
			CBodyDamageUpr *pBodyDamageUpr = g_pGame->GetBodyDamageUpr();
			assert(pBodyDamageUpr);

			TBodyDamageProfileId profileId = pBodyDamageUpr->FindBodyDamageProfileIdBinding(pEntity->GetId());
			if (profileId != INVALID_BODYDAMAGEPROFILEID)
			{
				pBodyDamageUpr->ReloadBodyDamage(profileId, *pEntity);
			}
		}
	}
}

void CBodyUprCVars::ReloadBodyDamage(IConsoleCmdArgs* pArgs)
{
	if (pArgs->GetArgCount() > 1)
	{
		IEntity* pEntity = gEnv->pEntitySystem->FindEntityByName(pArgs->GetArg(1));
		Reload(pEntity);
		
	}
	else
	{
		CBodyDamageUpr *pBodyDamageUpr = g_pGame->GetBodyDamageUpr();
		assert(pBodyDamageUpr);

		pBodyDamageUpr->ReloadBodyDamage();

		IActorIteratorPtr pIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
		while (IActor* pActor = pIt->Next())
			Reload(pActor);
	}
}

void CBodyUprCVars::ReloadBodyDestruction(IConsoleCmdArgs* pArgs)
{

	CBodyDamageUpr *pBodyDamageUpr = g_pGame->GetBodyDamageUpr();
	assert(pBodyDamageUpr);

	pBodyDamageUpr->ReloadBodyDestruction();

	IActorIteratorPtr pIt = g_pGame->GetIGameFramework()->GetIActorSystem()->CreateActorIterator();
	while (CActor* pActor = static_cast<CActor*>(pIt->Next()))
	{
		pActor->ReloadBodyDestruction();
	}
}