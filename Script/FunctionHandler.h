// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#if !defined(AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_)
#define AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include "ScriptSystem.h"

/*! IFunctionHandler implementation
   @see IFunctionHandler
 */
class CFunctionHandler : public IFunctionHandler
{
public:
	CFunctionHandler(CScriptSystem* pSS, lua_State* lState, tukk sFuncName, i32 paramIdOffset)
	{
		m_pSS = pSS;
		L = lState;
		m_sFuncName = sFuncName;
		m_paramIdOffset = paramIdOffset;
	}
	~CFunctionHandler() {}

public:
	i32                 GetParamCount();
	ScriptVarType       GetParamType(i32 nIdx);
	IScriptSystem*      GetIScriptSystem();

	virtual uk       GetThis();
	virtual bool        GetSelfAny(ScriptAnyValue& any);

	virtual tukk GetFuncName();

	//////////////////////////////////////////////////////////////////////////
	virtual bool GetParamAny(i32 nIdx, ScriptAnyValue& any);
	virtual i32  EndFunctionAny(const ScriptAnyValue& any);
	virtual i32  EndFunctionAny(const ScriptAnyValue& any1, const ScriptAnyValue& any2);
	virtual i32  EndFunctionAny(const ScriptAnyValue& any1, const ScriptAnyValue& any2, const ScriptAnyValue& any3);
	virtual i32  EndFunction();
	//////////////////////////////////////////////////////////////////////////

private:
	lua_State*     L;
	CScriptSystem* m_pSS;
	tukk    m_sFuncName;
	i32            m_paramIdOffset;
};

#endif // !defined(AFX_FUNCTIONHANDLER_H__CB02D9A1_DFAA_4DA3_8DF7_E2E8769F4ECE__INCLUDED_)
