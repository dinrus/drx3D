// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 21:9:2004   3:00 : Created by Mathieu Pinard

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ActorSystem.h>
#include <drx3D/CoreX/Game/IGameFramework.h>
#include <drx3D/Act/ScriptBind_ActorSystem.h>

//------------------------------------------------------------------------
CScriptBind_ActorSystem::CScriptBind_ActorSystem(ISystem* pSystem, IGameFramework* pGameFW)
{
	m_pSystem = pSystem;
	//m_pActorSystem = pActorSystem;
	m_pGameFW = pGameFW;

	Init(pSystem->GetIScriptSystem(), m_pSystem);
	SetGlobalName("Actor");

	RegisterGlobals();
	RegisterMethods();
}

//------------------------------------------------------------------------
CScriptBind_ActorSystem::~CScriptBind_ActorSystem()
{
}

//------------------------------------------------------------------------
void CScriptBind_ActorSystem::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_ActorSystem::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_ActorSystem::

	SCRIPT_REG_TEMPLFUNC(CreateActor, "channelId, actorParams");
}

//------------------------------------------------------------------------
// Example on how to use this function:
//
// local params =
// {
//   name     = "dude",
//   class    = "CSpectator",
//   position = {x=0, y=0, z=0},
//   rotation = {x=0, y=0, z=0},
//   scale    = {x=1, y=1, z=1}
// }
//
// Actor.CreateActor( channelId, params );
//
i32 CScriptBind_ActorSystem::CreateActor(IFunctionHandler* pH, i32 channelId, SmartScriptTable actorParams)
{
	DRX_ASSERT(m_pGameFW->GetIActorSystem() != NULL);
	tukk name;
	tukk className;
	Vec3 position;
	Vec3 scale;
	Vec3 rotation;

#define GET_VALUE_FROM_CHAIN(valName, val, chain)                                                                       \
  if (!chain.GetValue(valName, val))                                                                                    \
  {                                                                                                                     \
    DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, "CreateActor failed because <%s> field not specified", valName); \
    bFailed = true;                                                                                                     \
  }

	// The following code had to be enclosed in a bracket because
	// CScriptSetGetChain needs to be declared statically and also needs to
	// be destructed before EndFunction
	bool bFailed = false;
	do
	{
		CScriptSetGetChain actorChain(actorParams);
		GET_VALUE_FROM_CHAIN("name", name, actorChain);
		GET_VALUE_FROM_CHAIN("class", className, actorChain);
		GET_VALUE_FROM_CHAIN("position", position, actorChain);
		GET_VALUE_FROM_CHAIN("rotation", rotation, actorChain);
		GET_VALUE_FROM_CHAIN("scale", scale, actorChain);
	}
	while (false);

	if (bFailed)
		return pH->EndFunction(false);

	Quat q;
	q.SetRotationXYZ(Ang3(rotation));
	IActor* pActor = m_pGameFW->GetIActorSystem()->CreateActor(channelId, name, className, position, q, scale);

	if (pActor == NULL)
		return pH->EndFunction();
	else
		return pH->EndFunction(pActor->GetEntity()->GetScriptTable());
}
