// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptBind_Sound.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>

CScriptBind_Sound::CScriptBind_Sound(IScriptSystem* pScriptSystem, ISystem* pSystem)
{
	CScriptableBase::Init(pScriptSystem, pSystem);
	SetGlobalName("Sound");

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Sound::

	// AudioSystem
	SCRIPT_REG_TEMPLFUNC(GetAudioTriggerID, "sTriggerName");
	SCRIPT_REG_TEMPLFUNC(GetAudioSwitchID, "sSwitchName");
	SCRIPT_REG_TEMPLFUNC(GetAudioSwitchStateID, "hSwitchID, sStateName");
	SCRIPT_REG_TEMPLFUNC(GetAudioRtpcID, "sRtpcName");
	SCRIPT_REG_TEMPLFUNC(GetAudioEnvironmentID, "sEnvironmentName");
	SCRIPT_REG_TEMPLFUNC(SetAudioRtpcValue, "hRtpcID, fValue");
	SCRIPT_REG_TEMPLFUNC(GetAudioTriggerRadius, "triggerId");
}

///////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioTriggerID(IFunctionHandler* pH, char const* const szName)
{
	if ((szName != nullptr) && (szName[0] != '\0'))
	{
		DrxAudio::ControlId const triggerId = DrxAudio::StringToId(szName);
		return pH->EndFunction(IntToHandle(triggerId));
	}

	return pH->EndFunction();
}

///////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioSwitchID(IFunctionHandler* pH, char const* const szName)
{
	if ((szName != nullptr) && (szName[0] != '\0'))
	{
		DrxAudio::ControlId const switchId = DrxAudio::StringToId(szName);
		return pH->EndFunction(IntToHandle(switchId));
	}

	return pH->EndFunction();
}

///////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioSwitchStateID(IFunctionHandler* pH, ScriptHandle const hSwitchID, char const* const szName)
{
	if ((szName != nullptr) && (szName[0] != '\0'))
	{
		DrxAudio::SwitchStateId const switchStateId = DrxAudio::StringToId(szName);
		return pH->EndFunction(IntToHandle(switchStateId));
	}

	return pH->EndFunction();
}

///////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioRtpcID(IFunctionHandler* pH, char const* const szName)
{
	if ((szName != nullptr) && (szName[0] != '\0'))
	{
		DrxAudio::ControlId const parameterId = DrxAudio::StringToId(szName);
		return pH->EndFunction(IntToHandle(parameterId));
	}

	return pH->EndFunction();
}

///////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioEnvironmentID(IFunctionHandler* pH, char const* const szName)
{
	if ((szName != nullptr) && (szName[0] != '\0'))
	{
		DrxAudio::EnvironmentId const environmentId = DrxAudio::StringToId(szName);
		return pH->EndFunction(IntToHandle(environmentId));
	}

	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::SetAudioRtpcValue(IFunctionHandler* pH, ScriptHandle const hParameterId, float const value)
{
	gEnv->pAudioSystem->SetParameter(HandleToInt<DrxAudio::ControlId>(hParameterId), value);
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Sound::GetAudioTriggerRadius(IFunctionHandler* pH, ScriptHandle const hTriggerID)
{
	DrxAudio::STriggerData data;
	gEnv->pAudioSystem->GetTriggerData(HandleToInt<DrxAudio::ControlId>(hTriggerID), data);
	return pH->EndFunction(data.radius);
}
