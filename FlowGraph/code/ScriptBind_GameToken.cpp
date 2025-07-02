// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/ScriptBind_GameToken.h>
#include <drx3D/CoreX/Game/IGameTokens.h>

namespace
{
// Returns literal representation of the type value
tukk ScriptAnyTypeToString(EScriptAnyType type)
{
	switch (type)
	{
	case EScriptAnyType::Any:
		return "Any";
	case EScriptAnyType::Nil:
		return "Null";
	case EScriptAnyType::Boolean:
		return "Boolean";
	case EScriptAnyType::String:
		return "String";
	case EScriptAnyType::Number:
		return "Number";
	case EScriptAnyType::Function:
		return "Function";
	case EScriptAnyType::Table:
		return "Table";
	case EScriptAnyType::UserData:
		return "UserData";
	case EScriptAnyType::Handle:
		return "Pointer";
	case EScriptAnyType::Vector:
		return "Vec3";
	default:
		return "Unknown";
	}
}
};

void CScriptBind_GameToken::Release()
{
	delete this;
};

i32 CScriptBind_GameToken::SetToken(IFunctionHandler* pH)
{
	SCRIPT_CHECK_PARAMETERS(2);
	tukk szTokenName = nullptr;
	if (pH->GetParams(szTokenName) == false)
	{
		GameWarning("[GameToken.SetToken] Usage: GameToken.SetToken TokenName TokenValue]");
		return pH->EndFunction();
	}

	ScriptAnyValue val;
	if (pH->GetParamAny(2, val) == false)
	{
		GameWarning("[GameToken.SetToken(%s)] Usage: GameToken.SetToken TokenName TokenValue]", szTokenName);
		return pH->EndFunction();
	}

	TFlowInputData data;
	switch (val.GetType())
	{
	case EScriptAnyType::Boolean:
		{
			bool v;
			val.CopyTo(v);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_Bool, true);
			data.SetValueWithConversion(v);
		}
		break;

	case EScriptAnyType::Number:
		{
			float v;
			val.CopyTo(v);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_Float, true);
			data.SetValueWithConversion(v);
		}
		break;

	case EScriptAnyType::String:
		{
			string v;
			val.CopyTo(v);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_String, true);
			data.SetValueWithConversion(v);
		}
		break;

	case EScriptAnyType::Vector:
		{
			Vec3 v;
			val.CopyTo(v);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_Vec3, true);
			data.SetValueWithConversion(v);
		}

	case EScriptAnyType::Table:
		{
			Vec3 v;
			val.CopyFromTableTo(v);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_Vec3, true);
			data.SetValueWithConversion(v);
		}
		break;

	case EScriptAnyType::Handle:
		{
			ScriptHandle handle;
			val.CopyTo(handle);
			const EntityId v = static_cast<EntityId>(handle.n);

			data = TFlowInputData::CreateDefaultInitializedForTag(eFDT_Int, true);
			data.SetValueWithConversion(v);
		}
		break;

	default:
		GameWarning("[GameToken.SetToken(%s)] Unsupported type '%s']", szTokenName, ScriptAnyTypeToString(val.GetType()));
		return pH->EndFunction();
	}

	m_pTokenSystem->SetOrCreateToken(szTokenName, data);

	return pH->EndFunction();
}

i32 CScriptBind_GameToken::GetToken(IFunctionHandler* pH, tukk sTokenName)
{
	IGameToken* pToken = m_pTokenSystem->FindToken(sTokenName);
	if (!pToken)
	{
		GameWarning("[GameToken.GetToken] Cannot find token '%s'", sTokenName);
		return pH->EndFunction();
	}

	return pH->EndFunction(pToken->GetValueAsString());
}

i32 CScriptBind_GameToken::DumpAllTokens(IFunctionHandler* pH)
{
	m_pTokenSystem->DumpAllTokens();
	return pH->EndFunction();
}

void CScriptBind_GameToken::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
}
