// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Script/ScriptHelpers.h>
#include <drx3D/CoreX/Game/IGameTokens.h>
#include "GameTokenSystem.h"

/*! <description>This class implements script functions for game tokens.</description> */
class CScriptBind_GameToken :
	public CScriptableBase
{
public:
	CScriptBind_GameToken(CGameTokenSystem* pTokenSystem)
	{
		m_pTokenSystem = pTokenSystem;

		Init(gEnv->pScriptSystem, GetISystem());
		SetGlobalName("GameToken");

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_GameToken::

		SCRIPT_REG_FUNC(SetToken);
		SCRIPT_REG_TEMPLFUNC(GetToken, "sTokenName");
		SCRIPT_REG_FUNC(DumpAllTokens);
	}

	virtual ~CScriptBind_GameToken(void)
	{
	}

	void Release();

	//! <code>GameToken.SetToken( TokenName, TokenValue )</code>
	//! <description>Set the value of a game token.</description>
	i32 SetToken(IFunctionHandler* pH);

	//! <code>GameToken.GetToken( TokenName )</code>
	//! <description>Get the value of a game token.</description>
	i32 GetToken(IFunctionHandler* pH, tukk sTokenName);

	//! <code>GameToken.DumpAllTokens()</code>
	//! <description>Dump all game tokens with their values to the log.</description>
	i32          DumpAllTokens(IFunctionHandler* pH);

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	CGameTokenSystem* m_pTokenSystem;
};
