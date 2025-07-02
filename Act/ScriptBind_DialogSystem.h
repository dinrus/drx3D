// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

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
#ifndef __SCRIPTBIND_DIALOGSYSTEM_H__
#define __SCRIPTBIND_DIALOGSYSTEM_H__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>

class CDialogSystem;

class CScriptBind_DialogSystem : public CScriptableBase
{
public:
	CScriptBind_DialogSystem(ISystem* pSystem, CDialogSystem* pDS);
	virtual ~CScriptBind_DialogSystem();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	void RegisterGlobals();
	void RegisterMethods();

	i32  CreateSession(IFunctionHandler* pH, tukk scriptID);
	i32  DeleteSession(IFunctionHandler* pH, i32 sessionID);
	i32  SetActor(IFunctionHandler* pH, i32 sessionID, i32 actorID, ScriptHandle entity);
	i32  Play(IFunctionHandler* pH, i32 sessionID);
	i32  Stop(IFunctionHandler* pH, i32 sessionID);
	i32  IsEntityInDialog(IFunctionHandler* pH, ScriptHandle entity);

private:
	ISystem*       m_pSystem;
	IEntitySystem* m_pEntitySystem;
	CDialogSystem* m_pDS;
};

#endif //__SCRIPTBIND_DIALOGSYSTEM_H__
