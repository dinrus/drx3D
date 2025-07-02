// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 8:9:2004   10:33 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ActionFilter.h>
#include <drx3D/Act/ActionMapUpr.h>

//------------------------------------------------------------------------
CActionFilter::CActionFilter(CActionMapUpr* pActionMapUpr, IInput* pInput, tukk name, EActionFilterType type)
	: m_pActionMapUpr(pActionMapUpr),
	m_pInput(pInput),
	m_enabled(false),
	m_type(type),
	m_name(name)
{
}

//------------------------------------------------------------------------
CActionFilter::~CActionFilter()
{
}

//------------------------------------------------------------------------
void CActionFilter::Filter(const ActionId& action)
{
	m_filterActions.insert(action);
}

//------------------------------------------------------------------------
void CActionFilter::Enable(bool enable)
{
	m_enabled = enable;

	// auto-release all actions which
	if (m_enabled && m_type == eAFT_ActionFail)
	{
		TFilterActions::const_iterator it = m_filterActions.begin();
		TFilterActions::const_iterator end = m_filterActions.end();
		for (; it != end; ++it)
		{
			m_pActionMapUpr->ReleaseActionIfActive(*it);
		}
	}
	else if (!m_enabled)
	{
		// Reset analog key states
		if (!gEnv->IsDedicated())
			DRX_ASSERT(m_pInput); // don't assert on dedicated servers OR clients

		if (m_pInput)
		{
			m_pInput->ClearAnalogKeyState();
		}

		if (m_pActionMapUpr)
		{
			m_pActionMapUpr->RemoveAllRefireData();
		}
	}

	if (m_pActionMapUpr)
	{
		m_pActionMapUpr->BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_FilterStatusChanged, (UINT_PTR)enable, (UINT_PTR)GetName()));
	}
};
//------------------------------------------------------------------------
bool CActionFilter::ActionFiltered(const ActionId& action)
{
	// never filter anything out when disabled
	if (!m_enabled)
		return false;

	TFilterActions::const_iterator it = m_filterActions.find(action);

	if ((m_type == eAFT_ActionPass) == (it == m_filterActions.end()))
	{
		static ICVar* pDebugVar = gEnv->pConsole->GetCVar("i_debug");
		if (pDebugVar && pDebugVar->GetIVal() != 0)
		{
			DrxLog("Action %s is filtered by %s", action.c_str(), m_name.c_str());
		}
		return true;
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------
bool CActionFilter::SerializeXML(const XmlNodeRef& root, bool bLoading)
{
	if (bLoading)
	{
		// loading
		const XmlNodeRef& child = root;
		if (strcmp(child->getTag(), "actionfilter") != 0)
			return false;

		EActionFilterType actionFilterType = eAFT_ActionFail;
		if (!strcmp(child->getAttr("type"), "actionFail"))
			actionFilterType = eAFT_ActionFail;
		if (!strcmp(child->getAttr("type"), "actionPass"))
			actionFilterType = eAFT_ActionPass;

		m_type = actionFilterType;

		i32 nFilters = child->getChildCount();
		for (i32 f = 0; f < nFilters; ++f)
		{
			XmlNodeRef filter = child->getChild(f);
			Filter(CDrxName(filter->getAttr("name")));
		}
	}
	else
	{
		// saving
		XmlNodeRef filterRoot = root->newChild("actionfilter");
		filterRoot->setAttr("name", m_name);
		filterRoot->setAttr("type", m_type == eAFT_ActionPass ? "actionPass" : "actionFail");
		filterRoot->setAttr("version", m_pActionMapUpr->GetVersion());
		TFilterActions::const_iterator iter = m_filterActions.begin();
		while (iter != m_filterActions.end())
		{
			XmlNodeRef filterChild = filterRoot->newChild("filter");
			filterChild->setAttr("name", iter->c_str());
			++iter;
		}
	}
	return true;
}

void CActionFilter::GetMemoryUsage(IDrxSizer* s) const
{
	s->AddObject(m_filterActions);
}
