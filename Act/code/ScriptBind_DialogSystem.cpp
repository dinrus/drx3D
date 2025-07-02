// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_DialogSystem.cpp
//  Version:     v1.00
//  Created:     02/08/2006 by AlexL
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Dialog System ScriptBinding
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ScriptBind_DialogSystem.h>
#include <drx3D/Act/DialogSystem.h>
#include <drx3D/Act/DialogSession.h>

//------------------------------------------------------------------------
CScriptBind_DialogSystem::CScriptBind_DialogSystem(ISystem* pSystem, CDialogSystem* pDS)
{
	m_pSystem = pSystem;
	m_pEntitySystem = gEnv->pEntitySystem;
	m_pDS = pDS;
	assert(m_pDS != 0);

	Init(gEnv->pScriptSystem, m_pSystem);
	SetGlobalName("DialogSystem");

	RegisterGlobals();
	RegisterMethods();
}

//------------------------------------------------------------------------
CScriptBind_DialogSystem::~CScriptBind_DialogSystem()
{
}

//------------------------------------------------------------------------
void CScriptBind_DialogSystem::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_DialogSystem::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_DialogSystem::

	SCRIPT_REG_TEMPLFUNC(CreateSession, "scriptID");
	SCRIPT_REG_TEMPLFUNC(DeleteSession, "sessionID");
	SCRIPT_REG_TEMPLFUNC(SetActor, "sessionID, actorID, entityID");
	SCRIPT_REG_TEMPLFUNC(Play, "sessionID");
	SCRIPT_REG_TEMPLFUNC(Stop, "sessionID");
	SCRIPT_REG_TEMPLFUNC(IsEntityInDialog, "entityID");
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::CreateSession(IFunctionHandler* pH, tukk scriptID)
{
	CDialogSystem::SessionID sessionID = m_pDS->CreateSession(scriptID);
	return pH->EndFunction(sessionID);
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::DeleteSession(IFunctionHandler* pH, i32 sessionID)
{
	const bool ok = m_pDS->DeleteSession(sessionID);
	return pH->EndFunction(ok);
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::SetActor(IFunctionHandler* pH, i32 sessionID, i32 actorID, ScriptHandle entity)
{
	bool ok = false;
	CDialogSession* pSession = m_pDS->GetSession(sessionID);
	if (pSession)
	{
		ok = pSession->SetActor(actorID, (EntityId) entity.n);
	}
	else
	{
		GameWarning("[DiaLOG] CScriptBind_DialogSystem::SetActor: Cannot find session %d", sessionID);
	}
	return pH->EndFunction(ok);
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::Play(IFunctionHandler* pH, i32 sessionID)
{
	bool ok = false;
	CDialogSession* pSession = m_pDS->GetSession(sessionID);
	if (pSession)
	{
		pSession->SetAutoDelete(true);
		ok = pSession->Play();
	}
	else
	{
		GameWarning("[DiaLOG] CScriptBind_DialogSystem::Play: Cannot find session %d", sessionID);
	}
	return pH->EndFunction(ok);
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::Stop(IFunctionHandler* pH, i32 sessionID)
{
	bool ok = false;
	CDialogSession* pSession = m_pDS->GetSession(sessionID);
	if (pSession)
	{
		pSession->Stop();
		m_pDS->DeleteSession(sessionID);
	}
	else
	{
		GameWarning("[DiaLOG] CScriptBind_DialogSystem::Play: Cannot find session %d", sessionID);
	}
	return pH->EndFunction(ok);
}

//------------------------------------------------------------------------
i32 CScriptBind_DialogSystem::IsEntityInDialog(IFunctionHandler* pH, ScriptHandle entity)
{
	CDialogScript::TActorID actorID;
	CDialogSystem::SessionID sessionID;
	const bool inDialog = m_pDS->FindSessionAndActorForEntity((EntityId)entity.n, sessionID, actorID);
	return pH->EndFunction(inDialog);
}
