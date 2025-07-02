// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 7:9:2004   17:42 : Created by Márcio Martins
   - 15:9:2010  12:30 : Revised by Dean Claassen

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ActionMapUpr.h>
#include <drx3D/Act/ActionMap.h>
#include <drx3D/Act/ActionFilter.h>
#include <drx3D/CoreX/DrxCrc32.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/CoreX/Platform/IPlatformOS.h>

CActionMapUpr* CActionMapUpr::s_pThis = nullptr;

//------------------------------------------------------------------------
CActionMapUpr::CActionMapUpr(IInput* pInput)
	: m_loadedXMLPath(""),
	m_pInput(pInput),
	m_enabled(true),
	m_bRefiringInputs(false),
	m_bDelayedRemoveAllRefiringData(false),
	m_bIncomingInputRepeated(false),
	m_bRepeatedInputHoldTriggerFired(false),
	m_currentInputKeyID(eKI_Unknown),
	m_defaultEntityId(INVALID_ENTITYID),
	m_version(-1)    // the first actionmap/filter sets the version
{
	DRX_ASSERT(!s_pThis);
	s_pThis = this;

	if (m_pInput)
	{
		m_pInput->AddEventListener(this);
	}

#ifndef _RELEASE
	i_listActionMaps = 0;
	REGISTER_CVAR(i_listActionMaps, i_listActionMaps, 0, "List action maps/filters on screen (1 = list all, 2 = list blocked inputs only)");
	REGISTER_COMMAND("i_reloadActionMaps", ReloadActionMaps, VF_CHEAT, "Reloads action maps");
#endif
}

//------------------------------------------------------------------------
CActionMapUpr::~CActionMapUpr()
{
	if (m_pInput)
	{
		m_pInput->RemoveEventListener(this);
	}

	Clear();
	RemoveAllAlwaysActionListeners();
}

//------------------------------------------------------------------------
bool CActionMapUpr::OnInputEvent(const SInputEvent& event)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	if (!m_enabled)
		return false;

	// no actions while console is open
	if (gEnv->pConsole->IsOpened())
		return false;

	if (gEnv->IsEditor() && gEnv->pGameFramework->IsEditing())
		return false;

	if (event.keyName.c_str() && event.keyName.c_str()[0] == 0)
		return false;

	// Filter alt+enter which toggles between fullscreen and windowed mode and shouldn't be processed here
	if ((event.modifiers & eMM_Alt) && event.keyId == eKI_Enter)
		return false;

	// Check if input is repeated and record (Used for bUseHoldTriggerDelayRepeatOverride)
	if (event.keyId < eKI_SYS_Commit) // Ignore special types and unknown
	{
		if (m_currentInputKeyID != event.keyId)
		{
			m_currentInputKeyID = event.keyId;
			m_bIncomingInputRepeated = false;
			SetRepeatedInputHoldTriggerFired(false);
		}
		else
		{
			m_bIncomingInputRepeated = true;
		}
	}

	// Process priority list based on event
	TBindPriorityList priorityList;
	bool bHasItems = CreateEventPriorityList(event, priorityList);
	if (bHasItems)
	{
		return HandleAcceptedEvents(event, priorityList);
	}

	return false; // Not handled
}

//------------------------------------------------------------------------
void CActionMapUpr::Update()
{
	if (!m_enabled)
	{
		return;
	}

	// Process refired events
	UpdateRefiringInputs();

	for (TActionMapMap::iterator it = m_actionMaps.begin(); it != m_actionMaps.end(); ++it)
	{
		CActionMap* pAm = it->second;
		if (pAm->Enabled())
		{
			pAm->InputProcessed();
		}
	}

#ifndef _RELEASE
	if (i_listActionMaps)
	{
		const float color[4] = { 1, 1, 1, 1 };
		const float textSize = 1.5f;
		const bool renderAll = (i_listActionMaps != 2);
		const float xMargin = 40.f;

		IFFont* defaultFont = gEnv->pDrxFont->GetFont("default");
		float yPos = gEnv->pRenderer->GetOverlayHeight() - 100.f;
		float secondColumnOffset = 0.f;

		STextDrawContext ctx;
		ctx.SetSizeIn800x600(false);
		ctx.SetSize(Vec2(UIDRAW_TEXTSIZEFACTOR * textSize, UIDRAW_TEXTSIZEFACTOR * textSize));

		if (defaultFont)
		{
			TActionFilterMap::iterator it = m_actionFilters.begin();
			TActionFilterMap::iterator itEnd = m_actionFilters.end();
			while (it != itEnd)
			{
				const bool isEnabled = it->second->Enabled();

				if (renderAll || isEnabled)
				{
					string message;
					message.Format("%sFilter '%s' %s", isEnabled ? "$7" : "$5", it->second->GetName(), isEnabled ? "blocking inputs" : "allowing inputs");

					const float width = defaultFont->GetTextSize(message, true, ctx).x + 5.f;
					secondColumnOffset = max(width, secondColumnOffset);

					IRenderAuxText::Draw2dLabel(xMargin, yPos, 1.5f, color, false, "%s", message.c_str());
					yPos -= 15.f;
				}

				++it;
			}
		}

		yPos = gEnv->pRenderer->GetOverlayHeight() - 100.f;

		for (TActionMapMap::iterator it = m_actionMaps.begin(); it != m_actionMaps.end(); ++it)
		{
			CActionMap* pAm = it->second;
			const bool isEnabled = pAm->Enabled();
			if (renderAll || !isEnabled)
			{
				IRenderAuxText::Draw2dLabel(xMargin + secondColumnOffset, yPos, 1.5f, color, false, "%sAction map '%s' %s", isEnabled ? "$3" : "$4", pAm->GetName(), isEnabled ? "enabled" : "disabled");
				yPos -= 15.f;
			}
		}
	}
#endif
}

//------------------------------------------------------------------------
void CActionMapUpr::Reset()
{
	for (TActionMapMap::iterator it = m_actionMaps.begin(); it != m_actionMaps.end(); ++it)
	{
		it->second->Reset();
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::Clear()
{
	RemoveAllActionMaps();
	RemoveAllFilters();

	m_preloadedControllerLayouts.clear();
}

//------------------------------------------------------------------------
bool CActionMapUpr::InitActionMaps(tukk filename)
{
	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(filename);
	if (rootNode)
	{
		Clear();
		ClearInputDevicesMappings();
		ClearStoredCurrentInputData();

		XmlNodeRef platformsNode = rootNode->findChild("platforms");
		if (platformsNode)
		{
			XmlNodeRef platform = platformsNode->findChild(GetISystem()->GetPlatformOS()->GetPlatformName());
			if (platform)
			{
				BYTE devices(eAID_KeyboardMouse | eAID_XboxPad | eAID_PS4Pad | eAID_OculusTouch);

				if (!strcmp(platform->getAttr("keyboard"), "0"))      devices &= ~eAID_KeyboardMouse;
				if (!strcmp(platform->getAttr("xboxpad"), "0"))       devices &= ~eAID_XboxPad;
				if (!strcmp(platform->getAttr("ps4pad"), "0"))        devices &= ~eAID_PS4Pad;
				if (!strcmp(platform->getAttr("oculustouch"), "0"))   devices &= ~eAID_OculusTouch;

				if (devices & eAID_KeyboardMouse) AddInputDeviceMapping(eAID_KeyboardMouse, "keyboard");
				if (devices & eAID_XboxPad)       AddInputDeviceMapping(eAID_XboxPad, "xboxpad");
				if (devices & eAID_PS4Pad)        AddInputDeviceMapping(eAID_PS4Pad, "ps4pad");
				if (devices & eAID_OculusTouch)   AddInputDeviceMapping(eAID_OculusTouch, "oculustouch");

				SetLoadFromXMLPath(filename);
				if (LoadFromXML(rootNode))
				{
					BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_ActionMapsInitialized, (UINT_PTR)0, (UINT_PTR)0));
					return true;
				}
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::InitActionMaps: Failed to find platform defintions for '%s', action mappings loading will fail", GetISystem()->GetPlatformOS()->GetPlatformName());
			}
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::InitActionMaps: Failed to find XML node 'platforms', action mappings loading will fail");
		}
	}
	else
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::InitActionMaps: Failed to open %s, action mappings loading will fail", filename);
	}
	return false;
};

//------------------------------------------------------------------------
bool CActionMapUpr::LoadFromXML(const XmlNodeRef& node)
{
	i32 version = -1;
	if (!node->getAttr("version", version))
	{
		GameWarning("Obsolete action map format - version info is missing");
		return false;
	}
	m_version = version;

	//	load action maps
	i32 nChildren = node->getChildCount();
	for (i32 i = 0; i < nChildren; ++i)
	{
		XmlNodeRef child = node->getChild(i);
		if (!strcmp(child->getTag(), "actionmap"))
		{
			tukk actionMapName = child->getAttr("name");
			IActionMap* pActionMap = CreateActionMap(actionMapName);
			if (!pActionMap)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadFromXML: Unable to create actionmap: %s", actionMapName);
				continue;
			}

			bool bResult = pActionMap->LoadFromXML(child);
			if (!bResult)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadFromXML: Failed to serialize actionmap: %s", actionMapName);
				continue;
			}
		}

		if (!strcmp(child->getTag(), "actionfilter"))
		{
			tukk actionFilterName = child->getAttr("name");
			tukk actionFilterType = child->getAttr("type");
			IActionFilter* pActionFilter = CreateActionFilter(actionFilterName, !stricmp(actionFilterType, "actionPass") ? eAFT_ActionPass : eAFT_ActionFail);
			if (!pActionFilter)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadFromXML: Unable to create actionfilter: %s", actionFilterName);
				continue;
			}

			bool bResult = pActionFilter->SerializeXML(child, true);
			if (!bResult)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::SerializeXML: Failed to serialize actionfilter: %s", actionFilterName);
				continue;
			}
		}

		if (!strcmp(child->getTag(), "controllerlayouts"))
		{
			i32 nLayoutChildren = child->getChildCount();
			for (i32 j = 0; j < nLayoutChildren; ++j)
			{
				XmlNodeRef layoutChild = child->getChild(j);
				if (!PreloadControllerLayout(layoutChild))
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::SerializeXML: Failed to serialize controllerlayout in line %i", j);
				}
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::PreloadControllerLayout(const XmlNodeRef& controllerLayout)
{
	tukk szLayoutName = controllerLayout->getAttr("name");
	tukk szLayoutFile = controllerLayout->getAttr("file");

	if (!strcmp(szLayoutName, "") || !strcmp(szLayoutFile, ""))
		return false;

	string layoutPath;
	layoutPath.Format("libs/config/controller/%s", szLayoutFile);

	XmlNodeRef rootNode = GetISystem()->LoadXmlFromFile(layoutPath);
	if (!rootNode)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::PreloadControllerLayout: Failed to load xmlfile: %s", layoutPath.c_str());
		return false;
	}

	m_preloadedControllerLayouts[szLayoutName] = rootNode;

	return true;
}

bool CActionMapUpr::LoadControllerLayoutFile(tukk szLayoutKeyName)
{
	DRX_ASSERT(szLayoutKeyName != NULL);

	TControllerLayouts::const_iterator iter = m_preloadedControllerLayouts.find(szLayoutKeyName);
	if (iter == m_preloadedControllerLayouts.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadControllerLayoutFile: Failed to find preloaded controller layout for key: %s", szLayoutKeyName);
		return false;
	}

	XmlNodeRef rootNode = iter->second;
	DRX_ASSERT(rootNode != NULL);

	if (!LoadRebindDataFromXML(rootNode))
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadControllerLayoutFile: Failed to load controller layout: %s", szLayoutKeyName);
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::LoadRebindDataFromXML(const XmlNodeRef& node)
{
	i32 ignoreVersion = 0;
	node->getAttr("ignoreVersion", ignoreVersion);

	if (ignoreVersion == 0)
	{
		i32 version = -1;
		if (node->getAttr("version", version))
		{
			if (version != m_version)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadRebindDataFromXML: Failed, version found %d -> required %d", version, m_version);
				return false;
			}
		}
		else
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadRebindDataFromXML: Failed, obsolete action map format - version info is missing");
			return false;
		}
	}

	//	load action maps
	i32 nChildren = node->getChildCount();
	for (i32 i = 0; i < nChildren; ++i)
	{
		XmlNodeRef child = node->getChild(i);
		if (!strcmp(child->getTag(), "actionmap"))
		{
			tukk actionMapName = child->getAttr("name");
			IActionMap* pActionMap = GetActionMap(actionMapName);
			if (!pActionMap)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadRebindDataFromXML: Failed to find actionmap: %s", actionMapName);
				continue;
			}

			bool bResult = pActionMap->LoadRebindingDataFromXML(child);
			if (!bResult)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::LoadRebindDataFromXML: Failed to load rebind data for actionmap: %s", actionMapName);
				continue;
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::SaveRebindDataToXML(XmlNodeRef& node)
{
	node->setAttr("version", m_version);
	TActionMapMap::iterator iter = m_actionMaps.begin();
	TActionMapMap::iterator iterEnd = m_actionMaps.end();
	for (; iter != iterEnd; ++iter)
	{
		CActionMap* pActionMap = iter->second;
		DRX_ASSERT(pActionMap != NULL);

		if (pActionMap->GetNumRebindedInputs() == 0)
			continue;

		XmlNodeRef child = node->newChild("actionmap");
		bool bResult = pActionMap->SaveRebindingDataToXML(child);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::SaveRebindDataToXML: Failed to save rebind data for actionmap: %s", pActionMap->GetName());
		}
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::AddExtraActionListener(IActionListener* pExtraActionListener, tukk actionMap)
{
	if ((actionMap != NULL) && (actionMap[0] != '\0'))
	{
		TActionMapMap::iterator iActionMap = m_actionMaps.find(CONST_TEMP_STRING(actionMap));
		if (iActionMap != m_actionMaps.end())
		{
			iActionMap->second->AddExtraActionListener(pExtraActionListener);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		stl::push_back_unique(m_ExtraActionListeners, pExtraActionListener);
		return true;
	}
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveExtraActionListener(IActionListener* pExtraActionListener, tukk actionMap)
{
	if ((actionMap != NULL) && (actionMap[0] != '\0'))
	{
		TActionMapMap::iterator iActionMap = m_actionMaps.find(CONST_TEMP_STRING(actionMap));
		if (iActionMap != m_actionMaps.end())
		{
			iActionMap->second->RemoveExtraActionListener(pExtraActionListener);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		stl::find_and_erase(m_ExtraActionListeners, pExtraActionListener);
		return true;
	}
}

//------------------------------------------------------------------------
const TActionListeners& CActionMapUpr::GetExtraActionListeners() const
{
	return m_ExtraActionListeners;
}

//------------------------------------------------------------------------
bool CActionMapUpr::AddFlowgraphNodeActionListener(IActionListener* pFlowgraphNodeActionListener, tukk actionMap)
{
	if ((actionMap != NULL) && (actionMap[0] != '\0'))
	{
		TActionMapMap::iterator	iActionMap = m_actionMaps.find(CONST_TEMP_STRING(actionMap));
		if(iActionMap != m_actionMaps.end())
		{
			iActionMap->second->AddFlowNodeActionListener(pFlowgraphNodeActionListener);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		stl::push_back_unique(m_ExtraActionListeners, pFlowgraphNodeActionListener);
		return true;
	}
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveFlowgraphNodeActionListener(IActionListener* pFlowgraphNodeActionListener, tukk actionMap)
{
	if ((actionMap != NULL) && (actionMap[0] != '\0'))
	{
		TActionMapMap::iterator	iActionMap = m_actionMaps.find(CONST_TEMP_STRING(actionMap));
		if(iActionMap != m_actionMaps.end())
		{
			iActionMap->second->RemoveFlowNodeActionListener(pFlowgraphNodeActionListener);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		stl::find_and_erase(m_ExtraActionListeners, pFlowgraphNodeActionListener);
		return true;
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::AddAlwaysActionListener(TBlockingActionListener pActionListener)
{
	// Don't add if already exists
	TBlockingActionListeners::const_iterator iter = std::find(m_alwaysActionListeners.begin(), m_alwaysActionListeners.end(), pActionListener);
	if (iter != m_alwaysActionListeners.end())
		return;

	m_alwaysActionListeners.push_back(TBlockingActionListener(pActionListener));
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAlwaysActionListener(TBlockingActionListener pActionListener)
{
	TBlockingActionListeners::iterator iter = std::find(m_alwaysActionListeners.begin(), m_alwaysActionListeners.end(), pActionListener);
	if (iter == m_alwaysActionListeners.end())
		return;

	m_alwaysActionListeners.erase(iter);
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAllAlwaysActionListeners()
{
	m_alwaysActionListeners.clear();
}

//------------------------------------------------------------------------
IActionMap* CActionMapUpr::CreateActionMap(tukk name)
{
	if (GetActionMap(name) != NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::CreateActionMap: Failed to create actionmap: %s, already exists", name);
		return NULL;
	}

	CActionMap* pActionMap = new CActionMap(this, name);
	if (pActionMap && (m_defaultEntityId != INVALID_ENTITYID))
	{
		pActionMap->SetActionListener(m_defaultEntityId);
	}

	m_actionMaps.insert(TActionMapMap::value_type(name, pActionMap));
	return pActionMap;
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveActionMap(tukk name)
{
	TActionMapMap::iterator it = m_actionMaps.find(CONST_TEMP_STRING(name));
	if (it == m_actionMaps.end())
		return false;

	CActionMap* pActionMap = it->second;
	DRX_ASSERT(pActionMap != NULL);

	RemoveBind(pActionMap);

	SAFE_DELETE(pActionMap);
	m_actionMaps.erase(it);

	return true;
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAllActionMaps()
{
	TActionMapMap::iterator actionMapit = m_actionMaps.begin();
	TActionMapMap::iterator actionMapitEnd = m_actionMaps.end();
	for (; actionMapit != actionMapitEnd; ++actionMapit)
	{
		CActionMap* pActionMap = actionMapit->second;
		DRX_ASSERT(pActionMap != NULL);
		SAFE_DELETE(pActionMap);
	}
	m_actionMaps.clear();
	m_inputCRCToBind.clear();

	RemoveAllRefireData();
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAllFilters()
{
	TActionFilterMap::iterator filtersIt = m_actionFilters.begin();
	TActionFilterMap::iterator filtersItEnd = m_actionFilters.end();
	for (; filtersIt != filtersItEnd; ++filtersIt)
	{
		CActionFilter* pFilter = filtersIt->second;
		DRX_ASSERT(pFilter != NULL);
		SAFE_DELETE(pFilter);
	}
	m_actionFilters.clear();
}

//------------------------------------------------------------------------
IActionMap* CActionMapUpr::GetActionMap(tukk name)
{
	TActionMapMap::iterator it = m_actionMaps.find(CONST_TEMP_STRING(name));
	if (it == m_actionMaps.end())
	{
		return NULL;
	}
	else
	{
		return it->second;
	}
}

//------------------------------------------------------------------------
const IActionMap* CActionMapUpr::GetActionMap(tukk name) const
{
	TActionMapMap::const_iterator it = m_actionMaps.find(CONST_TEMP_STRING(name));
	if (it == m_actionMaps.end())
		return NULL;
	else
		return it->second;
}

//------------------------------------------------------------------------
IActionFilter* CActionMapUpr::CreateActionFilter(tukk name, EActionFilterType type)
{
	IActionFilter* pActionFilter = GetActionFilter(name);
	if (pActionFilter)
		return NULL;

	CActionFilter* pFilter = new CActionFilter(this, m_pInput, name, type);
	m_actionFilters.insert(TActionFilterMap::value_type(name, pFilter));

	return pFilter;
}

//------------------------------------------------------------------------
IActionFilter* CActionMapUpr::GetActionFilter(tukk name)
{
	TActionFilterMap::iterator it = m_actionFilters.find(CONST_TEMP_STRING(name));
	if (it != m_actionFilters.end())
	{
		return it->second;
	}

	return NULL;
}

//------------------------------------------------------------------------
void CActionMapUpr::Enable(const bool enable, const bool resetStateOnDisable)
{
	m_enabled = enable;

	if (!enable && resetStateOnDisable)
	{
		TActionMapMap::iterator it = m_actionMaps.begin();
		TActionMapMap::iterator itEnd = m_actionMaps.end();
		for (; it != itEnd; ++it)
		{
			CActionMap* pActionMap = it->second;
			DRX_ASSERT(pActionMap != NULL);
			pActionMap->ReleaseActionsIfActive(); // Releases pressed held state

			// Also need to clear repeat data
			RemoveAllRefireData();
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::EnableActionMap(tukk name, bool enable)
{
	if (name == 0 || *name == 0)
	{
		TActionMapMap::iterator it = m_actionMaps.begin();
		TActionMapMap::iterator itEnd = m_actionMaps.end();
		while (it != itEnd)
		{
			it->second->Enable(enable);
			++it;
		}
	}
	else
	{
		TActionMapMap::iterator it = m_actionMaps.find(CONST_TEMP_STRING(name));

		if (it != m_actionMaps.end())
		{
			if (it->second->Enabled() != enable)
			{
				it->second->Enable(enable);
				ReleaseFilteredActions();
				BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_ActionMapStatusChanged, (UINT_PTR)it->second, (UINT_PTR)enable));
			}
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::EnableFilter(tukk name, bool enable)
{
	if (name == 0 || *name == 0)
	{
		TActionFilterMap::iterator it = m_actionFilters.begin();
		TActionFilterMap::iterator itEnd = m_actionFilters.end();
		while (it != itEnd)
		{
			it->second->Enable(enable);
			++it;
		}
	}
	else
	{
		TActionFilterMap::iterator it = m_actionFilters.find(CONST_TEMP_STRING(name));

		if (it != m_actionFilters.end())
		{
			if (it->second->Enabled() != enable)
			{
				it->second->Enable(enable);
				ReleaseFilteredActions();
				BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_FilterStatusChanged, (UINT_PTR)name, (UINT_PTR)enable));
			}
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::ReleaseFilteredActions()
{
	for (TActionMapMap::iterator it = m_actionMaps.begin(), eit = m_actionMaps.end(); it != eit; ++it)
	{
		if (it->second->Enabled())
		{
			it->second->ReleaseFilteredActions();
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::ClearStoredCurrentInputData()
{
	m_currentInputKeyID = eKI_Unknown;
	m_bIncomingInputRepeated = false;
}

//------------------------------------------------------------------------
bool CActionMapUpr::ReBindActionInput(tukk actionMapName, const ActionId& actionId, tukk szCurrentInput, tukk szNewInput)
{
	IActionMap* pActionMap = GetActionMap(actionMapName);
	if (pActionMap)
	{
		return pActionMap->ReBindActionInput(actionId, szCurrentInput, szNewInput);
	}

	GameWarning("CActionMapUpr::ReBindActionInput: Failed to find actionmap: %s", actionMapName);
	return false;
}

//------------------------------------------------------------------------
const SActionInput* CActionMapUpr::GetActionInput(tukk actionMapName, const ActionId& actionId, const EActionInputDevice device, i32k iByDeviceIndex) const
{
	const IActionMap* pActionMap = GetActionMap(actionMapName);
	if (!pActionMap)
	{
		GameWarning("CActionMapUpr::GetActionInput: Failed to find actionmap: %s", actionMapName);
		return NULL;
	}

	const IActionMapAction* pActionMapAction = pActionMap->GetAction(actionId);
	if (!pActionMapAction)
	{
		GameWarning("CActionMapUpr::GetActionInput: Failed to find action:%s in actionmap: %s", actionId.c_str(), actionMapName);
		return NULL;
	}

	const SActionInput* pActionInput = pActionMapAction->GetActionInput(device, iByDeviceIndex);
	if (!pActionInput)
	{
		GameWarning("CActionMapUpr::GetActionInput: Failed to find action input with deviceid: %d and index: %d, in action:%s in actionmap: %s", (i32)device, iByDeviceIndex, actionId.c_str(), actionMapName);
		return NULL;
	}

	return pActionInput;
}

//------------------------------------------------------------------------
bool CActionMapUpr::IsFilterEnabled(tukk name)
{
	TActionFilterMap::iterator it = m_actionFilters.find(CONST_TEMP_STRING(name));

	if (it != m_actionFilters.end())
	{
		return it->second->Enabled();
	}

	return false;
}

//------------------------------------------------------------------------
bool CActionMapUpr::ActionFiltered(const ActionId& action)
{
	for (TActionFilterMap::iterator it = m_actionFilters.begin(); it != m_actionFilters.end(); ++it)
	{
		if (it->second->ActionFiltered(action))
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
bool CActionMapUpr::CreateEventPriorityList(const SInputEvent& inputEvent, TBindPriorityList& priorityList)
{
	bool bRes = false;

	// Look up the actions which are associated with this
	u32 inputCRC = CCrc32::ComputeLowercase(inputEvent.keyName.c_str());
	TInputCRCToBind::iterator itCRCBound = m_inputCRCToBind.lower_bound(inputCRC);
	TInputCRCToBind::const_iterator itCRCBoundEnd = m_inputCRCToBind.end();

	// Create priority listing for which processes should be fired
	float fLongestHeldTime = 0.0f;
	for (; itCRCBound != itCRCBoundEnd && itCRCBound->first == inputCRC; ++itCRCBound)
	{
		SBindData& bindData = itCRCBound->second;
		SActionInput* pActionInput = bindData.m_pActionInput;
		DRX_ASSERT(pActionInput != NULL);
		CActionMapAction* pAction = bindData.m_pAction;
		DRX_ASSERT(pAction != NULL);
		CActionMap* pActionMap = bindData.m_pActionMap;
		DRX_ASSERT(pActionMap != NULL);

#ifdef _DEBUG
		DRX_ASSERT((pActionInput->inputCRC == inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) == 0));

		if ((pActionInput->inputCRC == inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) != 0))
		{
			GameWarning("Hash values are identical, but values are different! %s - %s", pActionInput->input.c_str(), inputEvent.keyName.c_str());
		}
		if ((pActionInput->inputCRC != inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) == 0))
		{
			GameWarning("Hash values are different, but values are identical! %s - %s", pActionInput->input.c_str(), inputEvent.keyName.c_str());
		}

#endif //_DEBUG

		bool bAdd = (!ActionFiltered(pAction->GetActionId()) && pActionMap->CanProcessInput(inputEvent, pActionMap, pAction, pActionInput));

		if (bAdd)
		{
			priorityList.push_back(&bindData);
			bRes = true;
		}
	}

	return bRes;
}

//------------------------------------------------------------------------
bool CActionMapUpr::ProcessAlwaysListeners(const ActionId& action, i32 activationMode, float value, const SInputEvent& inputEvent)
{
	TBlockingActionListeners::iterator iter = m_alwaysActionListeners.begin();
	TBlockingActionListeners::iterator iterEnd = m_alwaysActionListeners.end();

	bool bHandled = false;
	for (; iter != iterEnd; ++iter)
	{
		TBlockingActionListener& listener = *iter;
		bHandled = listener->OnAction(action, activationMode, value, inputEvent);
		if (bHandled)
		{
			break;
		}
	}

	return bHandled;
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveBind(CActionMap* pActionMap)
{
	for (TInputCRCToBind::iterator iter = m_inputCRCToBind.begin(); iter != m_inputCRCToBind.end(); )
	{
		SBindData* pBindData = &((*iter).second);
		if (pBindData->m_pActionMap == pActionMap)
		{
			TInputCRCToBind::iterator iterDelete = iter++;
			m_inputCRCToBind.erase(iterDelete);
		}
		else
		{
			++iter;
		}
	}

	RemoveRefireData(pActionMap);
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveBind(CActionMapAction* pAction)
{
	for (TInputCRCToBind::iterator iter = m_inputCRCToBind.begin(); iter != m_inputCRCToBind.end(); )
	{
		SBindData* pBindData = &((*iter).second);
		if (pBindData->m_pAction == pAction)
		{
			TInputCRCToBind::iterator iterDelete = iter++;
			m_inputCRCToBind.erase(iterDelete);
		}
		else
		{
			++iter;
		}
	}

	RemoveRefireData(pAction);
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveActionFilter(CActionFilter* pActionFilter)
{
	for (TActionFilterMap::iterator it = m_actionFilters.begin(); it != m_actionFilters.end(); ++it)
	{
		if (it->second == pActionFilter)
		{
			m_actionFilters.erase(it);

			return;
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::ReleaseActionIfActive(const ActionId& actionId)
{
	TActionMapMap::iterator it = m_actionMaps.begin();
	TActionMapMap::const_iterator end = m_actionMaps.end();
	for (; it != end; ++it)
	{
		it->second->ReleaseActionIfActive(actionId);
	}
}
//------------------------------------------------------------------------
bool CActionMapUpr::AddBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	if (pActionInput->inputCRC == 0)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::AddBind: Failed to add, missing crc value");
		return false;
	}

	SBindData* pBindData = GetBindData(pActionMap, pAction, pActionInput);
	if (pBindData != NULL)
	{
		pActionInput->inputCRC = 0;
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::AddBind: Failed to add, already exists");
		return false;
	}

	SBindData bindData(pActionMap, pAction, pActionInput);

	m_inputCRCToBind.insert(std::pair<u32, SBindData>(pActionInput->inputCRC, bindData));

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	u32 inputCRC = pActionInput->inputCRC;

	TInputCRCToBind::iterator iter = m_inputCRCToBind.lower_bound(inputCRC);
	TInputCRCToBind::const_iterator end = m_inputCRCToBind.end();

	for (; iter != end && iter->first == inputCRC; ++iter)
	{
		SBindData& bindData = iter->second;

		if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput->input == pActionInput->input)
		{
			m_inputCRCToBind.erase(iter);
			RemoveRefireData(pActionMap, pAction, pActionInput);
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
bool CActionMapUpr::HasBind(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput) const
{
	u32 inputCRC = pActionInput->inputCRC;

	TInputCRCToBind::const_iterator iter = m_inputCRCToBind.lower_bound(inputCRC);
	TInputCRCToBind::const_iterator end = m_inputCRCToBind.end();

	for (; iter != end && iter->first == inputCRC; ++iter)
	{
		const SBindData& bindData = iter->second;

		if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput->input == pActionInput->input)
		{
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
bool CActionMapUpr::UpdateRefireData(const SInputEvent& event, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	SRefireData* pRefireData = NULL;
	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.find(pActionInput->inputCRC);
	if (iter != m_inputCRCToRefireData.end())
	{
		pRefireData = iter->second;
		DRX_ASSERT(pRefireData != NULL);

		// Search if match exists
		SRefireBindData* pFoundBindData = NULL;
		TRefireBindData& refireBindData = pRefireData->m_refireBindData;
		TRefireBindData::iterator refireBindDataIt = refireBindData.begin();
		TRefireBindData::const_iterator refireBindDataItEnd = refireBindData.end();
		for (; refireBindDataIt != refireBindDataItEnd; ++refireBindDataIt)
		{
			SRefireBindData& singleRefireBindData = *refireBindDataIt;
			SBindData& bindData = singleRefireBindData.m_bindData;
			if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput == pActionInput)
			{
				pFoundBindData = &singleRefireBindData;
			}
		}

		if (pFoundBindData) // Just update input since analog value could have changed or delayed press needing a release
		{
			pRefireData->m_inputEvent = event;
			pFoundBindData->m_bIgnoreNextUpdate = true;
		}
		else
		{
			refireBindData.push_back(SRefireBindData(pActionMap, pAction, pActionInput));
			refireBindData[refireBindData.size() - 1].m_bIgnoreNextUpdate = true;
		}
	}
	else
	{
		pRefireData = new SRefireData(event, pActionMap, pAction, pActionInput);
		DRX_ASSERT(pRefireData->m_refireBindData.empty() == false);
		pRefireData->m_refireBindData[0].m_bIgnoreNextUpdate = true;
		m_inputCRCToRefireData.insert(std::pair<u32, SRefireData*>(pActionInput->inputCRC, pRefireData));
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveRefireData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	DRX_ASSERT(m_bRefiringInputs == false); // This should never happen

	u32 inputCRC = pActionInput->inputCRC;
	if (inputCRC == 0)
		return false;

	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.find(inputCRC);
	if (iter == m_inputCRCToRefireData.end())
		return false;

	SRefireData* pRefireData = iter->second;
	DRX_ASSERT(pRefireData != NULL);

	// Try to find the actual match now, just finding the crc, doesn't mean this action input is here

	TRefireBindData& refireBindData = pRefireData->m_refireBindData;
	TRefireBindData::iterator refireBindDataIt = refireBindData.begin();
	TRefireBindData::const_iterator refireBindDataItEnd = refireBindData.end();
	for (; refireBindDataIt != refireBindDataItEnd; ++refireBindDataIt)
	{
		SRefireBindData& singleRefireBindData = *refireBindDataIt;
		SBindData& bindData = singleRefireBindData.m_bindData;
		if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput == pActionInput)
		{
			if (refireBindData.size() > 1) // Don't remove whole mapping, just this specific one
			{
				refireBindData.erase(refireBindDataIt);
			}
			else // No other inputs mapped, remove whole mapping
			{
				SAFE_DELETE(pRefireData);
				m_inputCRCToRefireData.erase(iter);
			}

			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveRefireData(CActionMap* pActionMap)
{
	DRX_ASSERT(m_bRefiringInputs == false); // This should never happen

	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.begin();
	TInputCRCToRefireData::iterator iterEnd = m_inputCRCToRefireData.end();
	while (iter != iterEnd)
	{
		SRefireData* pRefireData = iter->second;
		DRX_ASSERT(pRefireData != NULL);

		TRefireBindData& refireBindData = pRefireData->m_refireBindData;
		TRefireBindData::iterator refireBindDataIt = refireBindData.begin();
		TRefireBindData::const_iterator refireBindDataItEnd = refireBindData.end();
		while (refireBindDataIt != refireBindDataItEnd)
		{
			SRefireBindData& singleRefireBindData = *refireBindDataIt;
			SBindData& bindData = singleRefireBindData.m_bindData;
			if (bindData.m_pActionMap == pActionMap)
			{
				TRefireBindData::iterator iterDelete = refireBindDataIt++;
				refireBindData.erase(iterDelete);
			}
			else
			{
				++refireBindDataIt;
			}
		}

		if (refireBindData.empty()) // Remove whole mapping if no more items exist for the input
		{
			SAFE_DELETE(pRefireData);
			TInputCRCToRefireData::iterator iterDelete = iter++;
			m_inputCRCToRefireData.erase(iterDelete);
		}
		else
		{
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveRefireData(CActionMapAction* pAction)
{
	DRX_ASSERT(m_bRefiringInputs == false); // This should never happen

	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.begin();
	TInputCRCToRefireData::iterator iterEnd = m_inputCRCToRefireData.end();
	while (iter != iterEnd)
	{
		SRefireData* pRefireData = iter->second;
		DRX_ASSERT(pRefireData != NULL);

		TRefireBindData& refireBindData = pRefireData->m_refireBindData;
		TRefireBindData::iterator refireBindDataIt = refireBindData.begin();
		TRefireBindData::const_iterator refireBindDataItEnd = refireBindData.end();
		while (refireBindDataIt != refireBindDataItEnd)
		{
			SRefireBindData& singleRefireBindData = *refireBindDataIt;
			SBindData& bindData = singleRefireBindData.m_bindData;
			if (bindData.m_pAction == pAction)
			{
				TRefireBindData::iterator iterDelete = refireBindDataIt++;
				refireBindData.erase(iterDelete);
			}
			else
			{
				++refireBindDataIt;
			}
		}

		if (refireBindData.empty()) // Remove whole mapping if no more items exist for the input
		{
			SAFE_DELETE(pRefireData);
			TInputCRCToRefireData::iterator iterDelete = iter++;
			m_inputCRCToRefireData.erase(iterDelete);
		}
		else
		{
			++iter;
		}
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAllRefireData()
{
	if (!m_bRefiringInputs)
	{
		TInputCRCToRefireData::iterator it = m_inputCRCToRefireData.begin();
		TInputCRCToRefireData::iterator itEnd = m_inputCRCToRefireData.end();
		for (; it != itEnd; ++it)
		{
			SRefireData* pRefireData = it->second;
			DRX_ASSERT(pRefireData != NULL);
			SAFE_DELETE(pRefireData);
		}

		m_inputCRCToRefireData.clear();
	}
	else // This can happen if disabling an action filter from refired input
	{
		m_bDelayedRemoveAllRefiringData = true;
	}
}

//------------------------------------------------------------------------
void CActionMapUpr::SetDefaultActionEntity(EntityId id, bool bUpdateAll)
{
	if (bUpdateAll)
	{
		IActionMapIteratorPtr pActionMapIter = CreateActionMapIterator();
		while (IActionMap* pActionMap = pActionMapIter->Next())
		{
			pActionMap->SetActionListener(id);
		}
	}

	m_defaultEntityId = id;

	BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_DefaultActionEntityChanged, (UINT_PTR)id));
}

//------------------------------------------------------------------------
void CActionMapUpr::RegisterActionMapEventListener(IActionMapEventListener* pActionMapEventListener)
{
	stl::push_back_unique(m_actionMapEventListeners, pActionMapEventListener);
}

//------------------------------------------------------------------------
void CActionMapUpr::UnregisterActionMapEventListener(IActionMapEventListener* pActionMapEventListener)
{
	stl::find_and_erase(m_actionMapEventListeners, pActionMapEventListener);
}

//------------------------------------------------------------------------
void CActionMapUpr::BroadcastActionMapEvent(const SActionMapEvent& event)
{
	if (!m_actionMapEventListeners.empty())
	{
		CActionMapUpr::TActionMapEventListeners::const_iterator iter = m_actionMapEventListeners.begin();
		CActionMapUpr::TActionMapEventListeners::const_iterator cur;
		while (iter != m_actionMapEventListeners.end())
		{
			cur = iter++;
			(*cur)->OnActionMapEvent(event);
		}
	}
}

//------------------------------------------------------------------------
bool CActionMapUpr::SetRefireDataDelayedPressNeedsRelease(const SInputEvent& event, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput, const bool bDelayedPressNeedsRelease)
{
	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.find(pActionInput->inputCRC);
	if (iter == m_inputCRCToRefireData.end())
		return false;

	SRefireData* pRefireData = iter->second;
	DRX_ASSERT(pRefireData != NULL);

	// Search if match exists
	TRefireBindData& refireBindData = pRefireData->m_refireBindData;
	TRefireBindData::iterator refireBindDataIt = refireBindData.begin();
	TRefireBindData::const_iterator refireBindDataItEnd = refireBindData.end();
	for (; refireBindDataIt != refireBindDataItEnd; ++refireBindDataIt)
	{
		SRefireBindData& singleRefireBindData = *refireBindDataIt;
		SBindData& bindData = singleRefireBindData.m_bindData;
		if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput == pActionInput)
		{
			singleRefireBindData.m_bDelayedPressNeedsRelease = bDelayedPressNeedsRelease;
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
void CActionMapUpr::RemoveAllDelayedPressRefireData()
{
	DRX_ASSERT(m_bRefiringInputs == false); // This should never happen

	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.begin();
	TInputCRCToRefireData::iterator iterEnd = m_inputCRCToRefireData.end();

	while (iter != iterEnd)
	{
		SRefireData* pRefireData = iter->second;
		DRX_ASSERT(pRefireData != NULL);
		PREFAST_ASSUME(pRefireData != NULL);
		if (pRefireData->m_inputEvent.state == eIS_Pressed)
		{
			TInputCRCToRefireData::iterator deleteIter = iter;
			++iter;
			SAFE_DELETE(pRefireData);
			m_inputCRCToRefireData.erase(deleteIter);
		}
		else
		{
			++iter;
		}
	}
}

//------------------------------------------------------------------------
i32 CActionMapUpr::GetHighestPressDelayPriority() const
{
	i32 iHighestPressDelayPriority = -1;
	TInputCRCToRefireData::const_iterator iter = m_inputCRCToRefireData.begin();
	TInputCRCToRefireData::const_iterator iterEnd = m_inputCRCToRefireData.end();

	TRefireBindData::const_iterator refireBindIter;
	TRefireBindData::const_iterator refireBindIterEnd;

	for (; iter != iterEnd; ++iter)
	{
		const SRefireData* pRefireData = iter->second;
		DRX_ASSERT(pRefireData != NULL);

		refireBindIter = pRefireData->m_refireBindData.begin();
		refireBindIterEnd = pRefireData->m_refireBindData.end();
		for (; refireBindIter != refireBindIterEnd; ++refireBindIter)
		{
			const SRefireBindData& refireBindData = *refireBindIter;
			const SBindData& bindData = refireBindData.m_bindData;
			const SActionInput* pActionInput = bindData.m_pActionInput;
			DRX_ASSERT(pActionInput != NULL);
			if (pActionInput->iPressDelayPriority > iHighestPressDelayPriority)
			{
				iHighestPressDelayPriority = pActionInput->iPressDelayPriority;
			}
		}
	}

	return iHighestPressDelayPriority;
}

//------------------------------------------------------------------------
void CActionMapUpr::ReloadActionMaps(IConsoleCmdArgs* pArgs)
{
	s_pThis->InitActionMaps(s_pThis->GetLoadFromXMLPath());
}

//------------------------------------------------------------------------
CActionMapUpr::SBindData* CActionMapUpr::GetBindData(CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	SBindData* pFoundBindData = NULL;
	u32 inputCRC = pActionInput->inputCRC;
	TInputCRCToBind::iterator iter = m_inputCRCToBind.lower_bound(inputCRC);
	TInputCRCToBind::const_iterator end = m_inputCRCToBind.end();

	for (; iter != end && iter->first == inputCRC; ++iter)
	{
		SBindData& bindData = iter->second;

		if (bindData.m_pActionMap == pActionMap && bindData.m_pAction == pAction && bindData.m_pActionInput->input == pActionInput->input)
		{
			pFoundBindData = &bindData;
			break;
		}
	}

	return pFoundBindData;
}

//------------------------------------------------------------------------

namespace
{
template<typename T, class Derived>
struct CGenericPtrMapIterator : public Derived
{
	typedef typename T::mapped_type MappedType;
	typedef typename T::iterator    IterType;
	CGenericPtrMapIterator(T& theMap)
		: m_nRefs(0)
	{
		m_cur = theMap.begin();
		m_end = theMap.end();
	}
	virtual MappedType Next()
	{
		if (m_cur == m_end)
			return 0;
		MappedType& dt = m_cur->second;
		++m_cur;
		return dt;
	}
	virtual void AddRef()
	{
		++m_nRefs;
	}
	virtual void Release()
	{
		if (--m_nRefs <= 0)
			delete this;
	}
	IterType m_cur;
	IterType m_end;
	i32      m_nRefs;
};
};

//------------------------------------------------------------------------
IActionMapIteratorPtr CActionMapUpr::CreateActionMapIterator()
{
	return new CGenericPtrMapIterator<TActionMapMap, IActionMapIterator>(m_actionMaps);
}

//------------------------------------------------------------------------
IActionFilterIteratorPtr CActionMapUpr::CreateActionFilterIterator()
{
	return new CGenericPtrMapIterator<TActionFilterMap, IActionFilterIterator>(m_actionFilters);
}

//------------------------------------------------------------------------
void CActionMapUpr::EnumerateActions(IActionMapPopulateCallBack* pCallBack) const
{
	for (TActionMapMap::const_iterator actionMapIt = m_actionMaps.begin(); actionMapIt != m_actionMaps.end(); ++actionMapIt)
	{
		actionMapIt->second->EnumerateActions(pCallBack);
	}
}

//------------------------------------------------------------------------
i32 CActionMapUpr::GetActionsCount() const
{
	i32 actionCount = 0;
	for (TActionMapMap::const_iterator actionMapIt = m_actionMaps.begin(); actionMapIt != m_actionMaps.end(); ++actionMapIt)
	{
		actionCount += actionMapIt->second->GetActionsCount();
	}

	return actionCount;
}

//------------------------------------------------------------------------
bool CActionMapUpr::AddInputDeviceMapping(const EActionInputDevice deviceType, tukk szDeviceTypeStr)
{
	TInputDeviceData::iterator iter = m_inputDeviceData.begin();
	TInputDeviceData::iterator iterEnd = m_inputDeviceData.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInputDeviceData& inputDeviceData = *iter;
		if (inputDeviceData.m_type == deviceType)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::AddInputDeviceMapping: Device type already exists, failed to add device: %s", szDeviceTypeStr);
			return false;
		}
	}

	SActionInputDeviceData inputDeviceData(deviceType, szDeviceTypeStr);
	m_inputDeviceData.push_back(inputDeviceData);

	return true;
}

//------------------------------------------------------------------------
bool CActionMapUpr::RemoveInputDeviceMapping(const EActionInputDevice deviceType)
{
	TInputDeviceData::iterator iter = m_inputDeviceData.begin();
	TInputDeviceData::iterator iterEnd = m_inputDeviceData.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInputDeviceData& inputDeviceData = *iter;
		if (inputDeviceData.m_type == deviceType)
		{
			m_inputDeviceData.erase(iter);
			return true;
		}
	}

	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMapUpr::RemoveInputDeviceMapping: Failed to find device");
	return false;
}

//------------------------------------------------------------------------
void CActionMapUpr::ClearInputDevicesMappings()
{
	m_inputDeviceData.clear();
}

//------------------------------------------------------------------------
i32 CActionMapUpr::GetNumInputDeviceData() const
{
	return m_inputDeviceData.size();
}

//------------------------------------------------------------------------
const SActionInputDeviceData* CActionMapUpr::GetInputDeviceDataByIndex(i32k iIndex)
{
	DRX_ASSERT(iIndex >= 0 && iIndex < m_inputDeviceData.size());
	if (iIndex < 0 || iIndex >= m_inputDeviceData.size())
		return NULL;

	return &m_inputDeviceData[iIndex];
}

//------------------------------------------------------------------------
const SActionInputDeviceData* CActionMapUpr::GetInputDeviceDataByType(const EActionInputDevice deviceType)
{
	SActionInputDeviceData* pFoundInputDeviceData = NULL;

	TInputDeviceData::iterator iter = m_inputDeviceData.begin();
	TInputDeviceData::iterator iterEnd = m_inputDeviceData.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInputDeviceData& inputDeviceData = *iter;
		if (inputDeviceData.m_type == deviceType)
		{
			pFoundInputDeviceData = &inputDeviceData;
			break;
		}
	}

	return pFoundInputDeviceData;
}

//------------------------------------------------------------------------
const SActionInputDeviceData* CActionMapUpr::GetInputDeviceDataByType(tukk szDeviceType)
{
	SActionInputDeviceData* pFoundInputDeviceData = NULL;

	TInputDeviceData::iterator iter = m_inputDeviceData.begin();
	TInputDeviceData::iterator iterEnd = m_inputDeviceData.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInputDeviceData& inputDeviceData = *iter;
		if (strcmp(inputDeviceData.m_typeStr, szDeviceType) == 0)
		{
			pFoundInputDeviceData = &inputDeviceData;
			break;
		}
	}

	return pFoundInputDeviceData;
}

//------------------------------------------------------------------------
void CActionMapUpr::GetMemoryStatistics(IDrxSizer* pSizer)
{
	SIZER_SUBCOMPONENT_NAME(pSizer, "ActionMapUpr");
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_actionMaps);
	pSizer->AddObject(m_actionFilters);
}

//------------------------------------------------------------------------
bool CActionMapUpr::HandleAcceptedEvents(const SInputEvent& event, TBindPriorityList& priorityList)
{
	float fCurrTime = gEnv->pTimer->GetCurrTime();
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	if (pGameFramework && pGameFramework->IsGamePaused())
	{
		fCurrTime = gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI);
	}

	TBindPriorityList::iterator itBind = priorityList.begin();
	TBindPriorityList::const_iterator itBindEnd = priorityList.end();
	for (; itBind != itBindEnd; ++itBind)
	{
		const SBindData* pBindData = *itBind;
		DRX_ASSERT(pBindData);

		CActionMap* pActionMap = pBindData->m_pActionMap;
		DRX_ASSERT(pActionMap != NULL);

		if (!pActionMap->Enabled())
			continue;

		SActionInput* pActionInput = pBindData->m_pActionInput;
		DRX_ASSERT(pActionInput != NULL);

		const CActionMapAction* pAction = pBindData->m_pAction;
		DRX_ASSERT(pAction != NULL);

		const ActionId& actionID = pAction->GetActionId();

		if (gEnv->pInput->Retriggering() && (!(pActionInput->activationMode & eAAM_Retriggerable)))
			continue;

		// Current action will fire below, check if actioninput blocks input and handle
		HandleInputBlocking(event, pActionInput, fCurrTime);

		EInputState currentState = pActionInput->currentState;

		// Process console
		if (pActionInput->activationMode & eAAM_ConsoleCmd)
		{
			CConsoleActionListener consoleActionListener;
			consoleActionListener.OnAction(actionID, currentState, event.value);

			const TActionListeners& extraActionListeners = GetExtraActionListeners();
			for (TActionListeners::const_iterator extraListener = extraActionListeners.begin(); extraListener != extraActionListeners.end(); ++extraListener)
				(*extraListener)->OnAction(actionID, currentState, event.value);

			return true;
		}

		pActionMap->NotifyFlowNodeActionListeners(actionID, currentState, event.value);

		// TODO: Remove always action listeners and integrate into 1 prioritized type
		// Process the always events first, then process normal if not handled
		bool bHandled = ProcessAlwaysListeners(actionID, currentState, event.value, event);
		if (bHandled)
			continue;

		const TActionListeners& extraActionListeners = GetExtraActionListeners();
		for (TActionListeners::const_iterator extraListener = extraActionListeners.begin(); extraListener != extraActionListeners.end(); ++extraListener)
			(*extraListener)->OnAction(actionID, currentState, event.value);

		pActionMap->NotifyExtraActionListeners(actionID, currentState, event.value);

		// Process normal events
		IEntity* pEntity = gEnv->pEntitySystem->GetEntity(pActionMap->GetActionListener());
		if (!pEntity)
			continue;

		CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER);
		if (!pGameObject)
			continue;

		pGameObject->OnAction(actionID, currentState, event.value);
	}

	return false; // Not handled
}

//------------------------------------------------------------------------
void CActionMapUpr::HandleInputBlocking(const SInputEvent& event, const SActionInput* pActionInput, const float fCurrTime)
{
	// Refired events will make it here even when input blocked, handle these here
	if (IsCurrentlyRefiringInput())
	{
		if (gEnv->pInput->ShouldBlockInputEventPosting(event.keyId, event.deviceType, event.deviceIndex))
		{
			return;
		}
	}

	const SActionInputBlockData& inputBlockData = pActionInput->inputBlockData;
	EActionInputBlockType blockType = inputBlockData.blockType;
	if (blockType == eAIBT_None)
		return;

	// Check if block condition fulfilled

	// This can be optimized once EInputState + EActionActivationMode are combined like it should be
	EInputState compareState;

	// Need to translate analog to press, down, release state for easy comparison
	if (event.state == eIS_Changed)
	{
		if (pActionInput->bAnalogConditionFulfilled)
		{
			if (fCurrTime - pActionInput->fPressedTime >= FLT_EPSILON) // Can't be pressed since time has elapsed, must be hold
			{
				compareState = eIS_Down;
			}
			else
			{
				compareState = eIS_Pressed;
			}
		}
		else // Must be release
		{
			compareState = eIS_Released;
		}
	}
	else
	{
		compareState = event.state;
	}

	if (compareState & inputBlockData.activationMode)
	{
		switch (inputBlockData.blockType)
		{
		case eAIBT_BlockInputs:
			{
				const TActionInputBlockers& inputs = inputBlockData.inputs;
				TActionInputBlockers::const_iterator iter = inputs.begin();
				TActionInputBlockers::const_iterator iterEnd = inputs.end();
				for (; iter != iterEnd; ++iter)
				{
					const SActionInputBlocker& inputToBlock = *iter;
					SInputBlockData inputDataToSend(inputToBlock.keyId, inputBlockData.fBlockDuration, inputBlockData.bAllDeviceIndices, inputBlockData.deviceIndex);
					m_pInput->SetBlockingInput(inputDataToSend);
				}
			}
			break;
		case eAIBT_Clear:
			{
				m_pInput->ClearBlockingInputs();
			}
			break;
		}
	}
}

//------------------------------------------------------------------------
bool CActionMapUpr::CreateRefiredEventPriorityList(SRefireData* pRefireData,
                                                       TBindPriorityList& priorityList,
                                                       TBindPriorityList& removeList,
                                                       TBindPriorityList& delayPressNeedsReleaseList)
{
	bool bRes = false;

	u32 inputCRC = pRefireData->m_inputCRC;
	SInputEvent& inputEvent = pRefireData->m_inputEvent;
	TRefireBindData& refireBindData = pRefireData->m_refireBindData;

	// Look up the actions which are associated with this
	TRefireBindData::iterator it = refireBindData.begin();
	TRefireBindData::const_iterator itEnd = refireBindData.end();

	float fCurrTime = gEnv->pTimer->GetCurrTime();
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	if (pGameFramework && pGameFramework->IsGamePaused())
	{
		fCurrTime = gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI);
	}

	// Create priority listing for which processes should be fired
	float fLongestHeldTime = 0.0f;
	for (; it != itEnd; ++it)
	{
		SRefireBindData& singleRefireBindData = *it;
		SBindData& bindData = singleRefireBindData.m_bindData;

		if (singleRefireBindData.m_bIgnoreNextUpdate) // Ignoring is set true when an actual input event has just added or updated a refiring event
		{
			singleRefireBindData.m_bIgnoreNextUpdate = false;
			continue;
		}

		SActionInput* pActionInput = bindData.m_pActionInput;
		DRX_ASSERT(pActionInput != NULL);
		CActionMapAction* pAction = bindData.m_pAction;
		DRX_ASSERT(pAction != NULL);
		CActionMap* pActionMap = bindData.m_pActionMap;
		DRX_ASSERT(pActionMap != NULL);

		// Refire data could be either from analog repeating or delayed press
		// If its a delayed press event, only fire once when reach delay then remove
		if (inputEvent.state == eIS_Pressed)
		{
			const float fPressDelay = pActionInput->fPressTriggerDelay;
			if (fPressDelay >= FLT_EPSILON) // Must be delayed press event
			{
				if (fCurrTime - pActionInput->fPressedTime >= fPressDelay) // Meets delay so fire and mark for removal
				{
					removeList.push_back(&bindData);

					if (singleRefireBindData.m_bDelayedPressNeedsRelease) // Release must be fired after too
					{
						delayPressNeedsReleaseList.push_back(&bindData);
					}
				}
				else // Don't fire yet
				{
					continue;
				}
			}
		}

#ifdef _DEBUG

		DRX_ASSERT((pActionInput->inputCRC == inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) == 0)); \

		if ((pActionInput->inputCRC == inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) != 0))
		{
			GameWarning("Hash values are identical, but values are different! %s - %s", pActionInput->input.c_str(), inputEvent.keyName.c_str());
		}
		if ((pActionInput->inputCRC != inputCRC) && (strcmp(pActionInput->input.c_str(), inputEvent.keyName.c_str()) == 0))
		{
			GameWarning("Hash values are different, but values are identical! %s - %s", pActionInput->input.c_str(), inputEvent.keyName.c_str());
		}

#endif //_DEBUG
		bool bAdd = (!ActionFiltered(pAction->GetActionId()) && pActionMap->CanProcessInput(inputEvent, pActionMap, pAction, pActionInput));

		if (bAdd)
		{
			priorityList.push_back(&bindData);
			bRes = true;
		}
	}

	return bRes;
}

//------------------------------------------------------------------------
void CActionMapUpr::UpdateRefiringInputs()
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);

	// no actions while console is open
	if (gEnv->pConsole->IsOpened())
		return;

	if (gEnv->IsEditor() && gEnv->pGameFramework->IsEditing())
		return;

	SetCurrentlyRefiringInput(true);

	TBindPriorityList removeList;

	std::vector<SRefireReleaseListData> releaseListRefireData;

	TInputCRCToRefireData::iterator iter = m_inputCRCToRefireData.begin();
	TInputCRCToRefireData::iterator iterEnd = m_inputCRCToRefireData.end();
	for (; iter != iterEnd; ++iter)
	{
		SRefireData* pRefireData = iter->second;
		const SInputEvent& event = pRefireData->m_inputEvent;

		TBindPriorityList priorityList;
		TBindPriorityList delayPressNeedsReleaseList;

		bool bHasItems = CreateRefiredEventPriorityList(pRefireData, priorityList, removeList, delayPressNeedsReleaseList);
		if (bHasItems)
		{
			HandleAcceptedEvents(event, priorityList);

			if (m_bDelayedRemoveAllRefiringData) // Probably an action filter just got disabled which wants to remove all refire data, do it now
			{
				m_bDelayedRemoveAllRefiringData = false;
				SetCurrentlyRefiringInput(false);
				RemoveAllRefireData();
				return;
			}
		}

		if (delayPressNeedsReleaseList.empty() == false)
		{
			SRefireReleaseListData singleReleaseListRefireData;
			singleReleaseListRefireData.m_inputEvent = event;
			// Since not going through normal path for release event (Doesn't need to be checked, already approved for fire, just delayed)
			// Need to manually change the current states to release before firing to ensure action is fired with a release
			singleReleaseListRefireData.m_inputEvent.state = eIS_Released;
			TBindPriorityList::iterator iterDelay = delayPressNeedsReleaseList.begin();
			TBindPriorityList::const_iterator iterDelayEnd = delayPressNeedsReleaseList.end();
			for (; iterDelay != iterDelayEnd; ++iterDelay)
			{
				const SBindData* pReleaseBindData = *iterDelay;
				DRX_ASSERT(pReleaseBindData != NULL);
				SActionInput* pReleaseActionInput = pReleaseBindData->m_pActionInput;
				DRX_ASSERT(pReleaseActionInput != NULL);
				pReleaseActionInput->currentState = eIS_Released;
			}

			singleReleaseListRefireData.m_inputsList = delayPressNeedsReleaseList;
			releaseListRefireData.push_back(singleReleaseListRefireData);
		}
	}

	// Now process delayed release events which must happen after the delayed press
	const size_t eventCount = releaseListRefireData.size();
	for (size_t i = 0; i < eventCount; i++)
	{
		SRefireReleaseListData& singleReleaseListRefireData = releaseListRefireData[i];
		HandleAcceptedEvents(singleReleaseListRefireData.m_inputEvent, singleReleaseListRefireData.m_inputsList);
	}
	releaseListRefireData.clear();
	SetCurrentlyRefiringInput(false);

	// Safe to remove now
	TBindPriorityList::iterator removeIt = removeList.begin();
	TBindPriorityList::const_iterator removeItEnd = removeList.end();
	for (; removeIt != removeItEnd; ++removeIt)
	{
		const SBindData* pBindData = *removeIt;
		DRX_ASSERT(pBindData);

		RemoveRefireData(pBindData->m_pActionMap, pBindData->m_pAction, pBindData->m_pActionInput);
	}
	removeList.clear();
}
