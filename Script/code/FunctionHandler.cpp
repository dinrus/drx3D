// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/FunctionHandler.h>
#include <drx3D/Script/ScriptSystem.h>
#include <drx3D/Script/ScriptTable.h>

//////////////////////////////////////////////////////////////////////////
IScriptSystem* CFunctionHandler::GetIScriptSystem()
{
	return m_pSS;
};

//////////////////////////////////////////////////////////////////////////
uk CFunctionHandler::GetThis()
{
	uk ptr = NULL;
	// Get implicit self table.
	if (m_paramIdOffset > 0 && lua_type(L, 1) == LUA_TTABLE)
	{
		// index "__this" member.
		lua_pushstring(L, "__this");
		lua_rawget(L, 1);
		if (lua_type(L, -1) == LUA_TLIGHTUSERDATA)
			ptr = const_cast<uk>(lua_topointer(L, -1));
		lua_pop(L, 1); // pop result.
	}
	return ptr;
}

//////////////////////////////////////////////////////////////////////////
bool CFunctionHandler::GetSelfAny(ScriptAnyValue& any)
{
	bool bRes = false;
	if (m_paramIdOffset > 0)
	{
		bRes = m_pSS->ToAny(any, 1);
	}
	return bRes;
}

//////////////////////////////////////////////////////////////////////////
tukk CFunctionHandler::GetFuncName()
{
	return m_sFuncName;
}

//////////////////////////////////////////////////////////////////////////
i32 CFunctionHandler::GetParamCount()
{
	return max(lua_gettop(L) - m_paramIdOffset, 0);
}

//////////////////////////////////////////////////////////////////////////
ScriptVarType CFunctionHandler::GetParamType(i32 nIdx)
{
	i32 nRealIdx = nIdx + m_paramIdOffset;
	ScriptVarType type = svtNull;
	i32 luatype = lua_type(L, nRealIdx);
	switch (luatype)
	{
	case LUA_TNIL:
		type = svtNull;
		break;
	case LUA_TBOOLEAN:
		type = svtBool;
		break;
	case LUA_TNUMBER:
		type = svtNumber;
		break;
	case LUA_TSTRING:
		type = svtString;
		break;
	case LUA_TFUNCTION:
		type = svtFunction;
		break;
	case LUA_TLIGHTUSERDATA:
		type = svtPointer;
		break;
	case LUA_TTABLE:
		type = svtObject;
		break;
	}
	return type;
}

//////////////////////////////////////////////////////////////////////////
bool CFunctionHandler::GetParamAny(i32 nIdx, ScriptAnyValue& any)
{
	i32 nRealIdx = nIdx + m_paramIdOffset;
	bool bRes = m_pSS->ToAny(any, nRealIdx);
	if (!bRes)
	{
		ScriptVarType paramType = GetParamType(nIdx);
		tukk sParamType = ScriptVarTypeAsCStr(paramType);
		tukk sType = ScriptAnyTypeToString(any.GetType());
		// Report wrong param.
		ScriptWarning("[Script Error] Wrong parameter type. Function %s expect parameter %d of type %s (Provided type %s)", m_sFuncName, nIdx, sType, sParamType);
		m_pSS->LogStackTrace();
	}
	return bRes;
}

//////////////////////////////////////////////////////////////////////////
i32 CFunctionHandler::EndFunctionAny(const ScriptAnyValue& any)
{
	m_pSS->PushAny(any);
	return (any.GetType() == EScriptAnyType::Nil || any.GetType() == EScriptAnyType::Any) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////
i32 CFunctionHandler::EndFunctionAny(const ScriptAnyValue& any1, const ScriptAnyValue& any2)
{
	m_pSS->PushAny(any1);
	m_pSS->PushAny(any2);
	return 2;
}

//////////////////////////////////////////////////////////////////////////
i32 CFunctionHandler::EndFunctionAny(const ScriptAnyValue& any1, const ScriptAnyValue& any2, const ScriptAnyValue& any3)
{
	m_pSS->PushAny(any1);
	m_pSS->PushAny(any2);
	m_pSS->PushAny(any3);
	return 3;
}

//////////////////////////////////////////////////////////////////////////
i32 CFunctionHandler::EndFunction()
{
	return 0;
}
