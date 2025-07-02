// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/ScriptVarNode.h>
#include <drx3D/Movie/AnimTrack.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Script/IScriptSystem.h>

CAnimScriptVarNode::CAnimScriptVarNode(i32k id) : CAnimNode(id)
{
	SetFlags(GetFlags() | eAnimNodeFlags_CanChangeName);
	m_value = -1e-20f;
}

void CAnimScriptVarNode::OnReset()
{
	m_value = -1e-20f;
}

void CAnimScriptVarNode::OnResume()
{
	OnReset();
}

void CAnimScriptVarNode::CreateDefaultTracks()
{
	CreateTrack(eAnimParamType_Float);
};

u32 CAnimScriptVarNode::GetParamCount() const
{
	return 1;
}

CAnimParamType CAnimScriptVarNode::GetParamType(u32 nIndex) const
{
	if (nIndex == 0)
	{
		return eAnimParamType_Float;
	}

	return eAnimParamType_Invalid;
}

bool CAnimScriptVarNode::GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const
{
	if (paramId.GetType() == eAnimParamType_Float)
	{
		info.flags = IAnimNode::ESupportedParamFlags(0);
		info.name = "Value";
		info.paramType = eAnimParamType_Float;
		info.valueType = eAnimValue_Float;
		return true;
	}

	return false;
}

void CAnimScriptVarNode::Animate(SAnimContext& animContext)
{
	float floatValue = m_value;

	IAnimTrack* pValueTrack = GetTrackForParameter(eAnimParamType_Float);

	if (pValueTrack)
	{
		if (pValueTrack->GetFlags() & IAnimTrack::eAnimTrackFlags_Disabled)
		{
			return;
		}

		TMovieSystemValue value = pValueTrack->GetValue(animContext.time);
		floatValue = stl::get<float>(value);
	}

	if (floatValue != m_value)
	{
		m_value = floatValue;
		// Change console var value.
		SetScriptValue();
	}
}

void CAnimScriptVarNode::SetScriptValue()
{
	IScriptSystem* pScriptSystem = gEnv->pMovieSystem->GetSystem()->GetIScriptSystem();

	if (!pScriptSystem)
	{
		return;
	}

	tukk sVarName = GetName();
	tukk sPnt = strchr(sVarName, '.');

	if (sPnt == 0)
	{
		// Global variable.
		pScriptSystem->SetGlobalValue(sVarName, m_value);
	}
	else
	{
		char sTable[256];
		char sName[256];
		drx_strcpy(sTable, sVarName);
		sTable[sPnt - sVarName] = 0;
		drx_strcpy(sName, sPnt + 1);

		// In Table value.
		SmartScriptTable pTable;

		if (pScriptSystem->GetGlobalValue(sTable, pTable))
		{
			// Set float value inside table.
			pTable->SetValue(sName, m_value);
		}
	}
}
