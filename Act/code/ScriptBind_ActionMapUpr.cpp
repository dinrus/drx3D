// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 8:11:2004   16:49 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ScriptBind_ActionMapUpr.h>
#include <drx3D/Act/ActionMapUpr.h>

//------------------------------------------------------------------------
CScriptBind_ActionMapUpr::CScriptBind_ActionMapUpr(ISystem* pSystem, CActionMapUpr* pActionMapUpr)
	: m_pSystem(pSystem),
	m_pUpr(pActionMapUpr)
{
	Init(gEnv->pScriptSystem, m_pSystem);
	SetGlobalName("ActionMapUpr");

	RegisterGlobals();
	RegisterMethods();
}

//------------------------------------------------------------------------
CScriptBind_ActionMapUpr::~CScriptBind_ActionMapUpr()
{
}

//------------------------------------------------------------------------
void CScriptBind_ActionMapUpr::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_ActionMapUpr::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_ActionMapUpr::

	SCRIPT_REG_TEMPLFUNC(EnableActionFilter, "name, enable");
	SCRIPT_REG_TEMPLFUNC(EnableActionMap, "name, enable");
	SCRIPT_REG_TEMPLFUNC(EnableActionMapUpr, "enable, resetStateOnDisable");
	SCRIPT_REG_TEMPLFUNC(LoadFromXML, "name");
	SCRIPT_REG_TEMPLFUNC(InitActionMaps, "path");
	SCRIPT_REG_TEMPLFUNC(SetDefaultActionEntity, "id, updateAll");
	SCRIPT_REG_TEMPLFUNC(GetDefaultActionEntity, "");
	SCRIPT_REG_TEMPLFUNC(LoadControllerLayoutFile, "layoutName");
	SCRIPT_REG_TEMPLFUNC(IsFilterEnabled, "filterName");
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::EnableActionFilter(IFunctionHandler* pH, tukk name, bool enable)
{
	m_pUpr->EnableFilter(name, enable);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::EnableActionMap(IFunctionHandler* pH, tukk name, bool enable)
{
	m_pUpr->EnableActionMap(name, enable);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::EnableActionMapUpr(IFunctionHandler* pH, bool enable, bool resetStateOnDisable)
{
	m_pUpr->Enable(enable, resetStateOnDisable);
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::LoadFromXML(IFunctionHandler* pH, tukk name)
{
	XmlNodeRef rootNode = m_pSystem->LoadXmlFromFile(name);
	m_pUpr->LoadFromXML(rootNode);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::InitActionMaps(IFunctionHandler* pH, tukk path)
{
	m_pUpr->InitActionMaps(path);
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::SetDefaultActionEntity(IFunctionHandler* pH, EntityId id, bool updateAll)
{
	m_pUpr->SetDefaultActionEntity(id, updateAll);
	return pH->EndFunction();
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::GetDefaultActionEntity(IFunctionHandler* pH)
{
	return pH->EndFunction(m_pUpr->GetDefaultActionEntity());
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::LoadControllerLayoutFile(IFunctionHandler* pH, tukk layoutName)
{
	return pH->EndFunction(m_pUpr->LoadControllerLayoutFile(layoutName));
}

//------------------------------------------------------------------------
i32 CScriptBind_ActionMapUpr::IsFilterEnabled(IFunctionHandler* pH, tukk filterName)
{
	return pH->EndFunction(m_pUpr->IsFilterEnabled(filterName));
}
