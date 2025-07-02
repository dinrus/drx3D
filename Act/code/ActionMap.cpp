// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 7:9:2004   17:48 : Created by Márcio Martins
   - 15:9:2010  12:30 : Revised by Dean Claassen

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ActionMap.h>
#include <drx3D/Act/ActionMapUpr.h>
#include <drx3D/Sys/IConsole.h>
#include <drx3D/Act/GameObject.h>
#include <drx3D/CoreX/DrxCrc32.h>

#define ACTIONINPUT_NAME_STR                               "name"

#define ACTIONINPUT_ONPRESS_STR                            "onPress"
#define ACTIONINPUT_ONRELEASE_STR                          "onRelease"
#define ACTIONINPUT_ONHOLD_STR                             "onHold"
#define ACTIONINPUT_ALWAYS_STR                             "always"

#define ACTIONINPUT_CONSOLECMD_STR                         "consoleCmd"
#define ACTIONINPUT_NOMODIFIERS_STR                        "noModifiers"
#define ACTIONINPUT_RETRIGGERABLE_STR                      "retriggerable"

#define ACTIONINPUT_PRESSTRIGGERDELAY_STR                  "pressTriggerDelay"
#define ACTIONINPUT_PRESSTRIGGERDELAY_REPEATOVERRIDE_STR   "pressTriggerDelayRepeatOverride"
#define ACTIONINPUT_PRESSDELAYPRIORITY_STR                 "pressDelayPriority"

#define ACTIONINPUT_HOLDTRIGGERDELAY_STR                   "holdTriggerDelay"
#define ACTIONINPUT_HOLDTRIGGERDELAY_REPEATOVERRIDE_STR    "holdTriggerDelayRepeatOverride"
#define ACTIONINPUT_HOLDREPEATDELAY_STR                    "holdRepeatDelay"
#define ACTIONINPUT_RELEASETRIGGERTHRESHOLD_STR            "releaseTriggerThreshold"

#define ACTIONINPUT_INPUTSTOBLOCK_STR                      "inputsToBlock"
#define ACTIONINPUT_INPUTBLOCKTIME_STR                     "inputBlockTime"
#define ACTIONINPUT_INPUTBLOCKACTIVATION_STR               "inputBlockActivation"
#define ACTIONINPUT_INPUTBLOCKDEVICEINDEX_STR              "inputBlockDeviceIndex"

#define ACTIONINPUT_INPUTSTOBLOCK_SPECIALTYPE_CLEARALL_STR "CLEARALL"

#define ACTIONINPUT_USEANALOGCOMPARE_STR                   "useAnalogCompare"
#define ACTIONINPUT_ANALOGCOMPAREVAL_STR                   "analogCompareVal"
#define ACTIONINPUT_ANALOGCOMPAREOP_STR                    "analogCompareOp"

static tukk s_analogCompareOpEqualsStr = "EQUALS";
static tukk s_analogCompareOpNotEqualsStr = "NOTEQUALS";
static tukk s_analogCompareOpGreaterThanStr = "GREATERTHAN";
static tukk s_analogCompareOpLessThanStr = "LESSTHAN";

//------------------------------------------------------------------------
struct DrxNameSorter
{
	//------------------------------------------------------------------------
	bool operator()(const CDrxName& lhs, const CDrxName& rhs) const
	{
		assert(lhs.c_str() != 0);
		assert(rhs.c_str() != 0);
		return strcmp(lhs.c_str(), rhs.c_str()) < 0;
	}
};

//------------------------------------------------------------------------
CActionMapAction::CActionMapAction()
	: m_triggeredInput("")
	, m_actionId("")
	, m_iNumRebindedInputs(0)
	, m_pParentActionMap(NULL)
{
}

//------------------------------------------------------------------------
CActionMapAction::~CActionMapAction()
{
	RemoveAllActionInputs();
}

//------------------------------------------------------------------------
void CActionMapAction::GetMemoryUsage(IDrxSizer* pSizer) const
{
	TActionInputs::const_iterator iter = m_actionInputs.begin();
	TActionInputs::const_iterator iterEnd = m_actionInputs.end();

	for (; iter != iterEnd; ++iter)
	{
		const SActionInput* pActionInput = *iter;
		pSizer->AddObject(pActionInput->input);
		pSizer->AddObject(pActionInput->defaultInput);
	}
}

//------------------------------------------------------------------------
bool CActionMapAction::AddActionInput(const SActionInput& actionInput, i32k iByDeviceIndex)
{
	// Double check no invalid inputs
	if (strcmp(actionInput.input, "") == 0)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddActionInput: Can't add empty input");
		return false;
	}

	// Don't require to have a crc when passing in, but if has one, check it
	u32 inputCRC = CCrc32::ComputeLowercase(actionInput.input);
	if (actionInput.inputCRC != 0)
	{
		if (inputCRC != actionInput.inputCRC)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddActionInput: Can't add, string crc specified doesn't match actual crc");
			return false;
		}
	}

	SActionInput* pActionInput = FindActionInput(inputCRC);
	if (pActionInput != NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAction::AddActionInput::Unable to add input with input: %s, either input already exists, or possibly 2 different inputs sharing same crc value", actionInput.input.c_str());
		return false;
	}

	SActionInput* pNewActionInput = new SActionInput();
	*pNewActionInput = actionInput;
	pNewActionInput->input.MakeLower();
	pNewActionInput->defaultInput.MakeLower();
	pNewActionInput->inputCRC = inputCRC;

	if (iByDeviceIndex == -1)
	{
		m_actionInputs.push_back(pNewActionInput);
		return true;
	}
	else
	{
		i32 iCurrentByDeviceIndex = 0;

		// Find actual index
		TActionInputs::iterator iter = m_actionInputs.begin();
		TActionInputs::iterator iterEnd = m_actionInputs.end();
		for (; iter != iterEnd; ++iter)
		{
			SActionInput* pContainedActionInput = *iter;
			DRX_ASSERT(pContainedActionInput != NULL);

			if (pContainedActionInput->inputDevice == actionInput.inputDevice && iCurrentByDeviceIndex == iByDeviceIndex)
			{
				m_actionInputs.insert(iter, pNewActionInput);
				return true;
			}
			else
			{
				++iCurrentByDeviceIndex;
			}
		}

		// Reached here so iByDeviceIndex must be incorrect
		SAFE_DELETE(pNewActionInput);
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAction::AddActionInput: Failed adding input with input: %s, iByDeviceIndex: %d, is incorrect", actionInput.input.c_str(), iByDeviceIndex);
		return false;
	}
}

//------------------------------------------------------------------------
bool CActionMapAction::RemoveActionInput(u32 inputCRC)
{
	TActionInputs::iterator iter = m_actionInputs.begin();
	TActionInputs::iterator iterEnd = m_actionInputs.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInput* pActionInput = *iter;
		DRX_ASSERT(pActionInput != NULL);
		if (pActionInput->inputCRC == inputCRC)
		{
			SAFE_DELETE(pActionInput);
			m_actionInputs.erase(iter);
			return true;
		}
	}

	return false;
}

//------------------------------------------------------------------------
void CActionMapAction::RemoveAllActionInputs()
{
	TActionInputs::iterator iter = m_actionInputs.begin();
	TActionInputs::iterator iterEnd = m_actionInputs.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInput* pActionInput = *iter;
		DRX_ASSERT(pActionInput != NULL);
		SAFE_DELETE(pActionInput);
	}
	m_actionInputs.clear();
}

//------------------------------------------------------------------------
SActionInput* CActionMapAction::FindActionInput(u32 inputCRC)
{
	SActionInput* pFoundActionInput = NULL;

	TActionInputs::iterator iter = m_actionInputs.begin();
	TActionInputs::iterator iterEnd = m_actionInputs.end();
	for (; iter != iterEnd; ++iter)
	{
		SActionInput* pActionInput = *iter;
		DRX_ASSERT(pActionInput != NULL);
		if (pActionInput->inputCRC == inputCRC)
		{
			pFoundActionInput = pActionInput;
			break;
		}
	}

	return pFoundActionInput;
}

//------------------------------------------------------------------------
const SActionInput* CActionMapAction::FindActionInput(tukk szInput) const
{
	u32 inputCRC = CCrc32::ComputeLowercase(szInput);

	const SActionInput* pFoundActionInput = NULL;

	TActionInputs::const_iterator iter = m_actionInputs.begin();
	TActionInputs::const_iterator iterEnd = m_actionInputs.end();
	for (; iter != iterEnd; ++iter)
	{
		const SActionInput* pActionInput = *iter;
		DRX_ASSERT(pActionInput != NULL);
		if (pActionInput->inputCRC == inputCRC)
		{
			pFoundActionInput = pActionInput;
			break;
		}
	}

	return pFoundActionInput;
}

//------------------------------------------------------------------------
SActionInput* CActionMapAction::GetActionInput(i32k iIndex)
{
	DRX_ASSERT(iIndex >= 0 && iIndex < (i32)m_actionInputs.size());
	if (iIndex >= 0 && iIndex < (i32)m_actionInputs.size())
	{
		return m_actionInputs[iIndex];
	}

	return NULL;
}

//------------------------------------------------------------------------
SActionInput* CActionMapAction::GetActionInput(const EActionInputDevice device, i32k iIndexByDevice)
{
	i32 iCurrentIndexByDevice = 0; // Array may not be sorted, but the index specified by iIndexByDevice means the order in which its found
	SActionInput* pFoundActionInput = NULL;
	for (i32 i = 0; i < m_actionInputs.size(); i++)
	{
		SActionInput* pActionInput = m_actionInputs[i];
		DRX_ASSERT(pActionInput != NULL);

		if (pActionInput->inputDevice == device)
		{
			if (iCurrentIndexByDevice == iIndexByDevice)
			{
				pFoundActionInput = pActionInput;
				break;
			}
			else // Keep looking for next one
			{
				iCurrentIndexByDevice++;
			}
		}
	}

	return pFoundActionInput;
}

//------------------------------------------------------------------------
const SActionInput* CActionMapAction::GetActionInput(i32k iIndex) const
{
	DRX_ASSERT(iIndex >= 0 && iIndex < (i32)m_actionInputs.size());
	if (iIndex >= 0 && iIndex < (i32)m_actionInputs.size())
	{
		return m_actionInputs[iIndex];
	}

	return NULL;
}

//------------------------------------------------------------------------
const SActionInput* CActionMapAction::GetActionInput(const EActionInputDevice device, i32k iIndexByDevice) const
{
	i32 iCurrentIndexByDevice = 0; // Array may not be sorted, but the index specified by iIndexByDevice means the order in which its found
	const SActionInput* pFoundActionInput = NULL;
	for (i32 i = 0; i < m_actionInputs.size(); i++)
	{
		const SActionInput* pActionInput = m_actionInputs[i];
		DRX_ASSERT(pActionInput != NULL);

		if (pActionInput->inputDevice == device)
		{
			if (iCurrentIndexByDevice == iIndexByDevice)
			{
				pFoundActionInput = pActionInput;
				break;
			}
			else // Keep looking for next one
			{
				iCurrentIndexByDevice++;
			}
		}
	}

	return pFoundActionInput;
}

//------------------------------------------------------------------------
SActionInput* CActionMapAction::AddAndGetActionInput(const SActionInput& actionInput)
{
	// Don't require to have a crc when passing in, but if has one, check it
	u32 inputCRC = CCrc32::ComputeLowercase(actionInput.input);
	if (actionInput.inputCRC != 0)
	{
		if (inputCRC != actionInput.inputCRC)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddAndGetActionInput: Can't add, string crc specified doesn't match actual crc");
			return NULL;
		}
	}

	SActionInput* pActionInput = FindActionInput(inputCRC);
	if (pActionInput != NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CAction::AddAndGetActionInput::Unable to add input with input: %s, already exists", actionInput.input.c_str());
		return NULL;
	}
	SActionInput* pNewActionInput = new SActionInput();
	*pNewActionInput = actionInput;
	pNewActionInput->input.MakeLower();
	pNewActionInput->defaultInput.MakeLower();
	pNewActionInput->inputCRC = inputCRC;
	m_actionInputs.push_back(pNewActionInput);

	return pNewActionInput;
}

//------------------------------------------------------------------------
CActionMap::CActionMap(CActionMapUpr* pActionMapUpr, tukk name)
	: m_pActionMapUpr(pActionMapUpr),
	m_enabled(!stricmp(name, "default")),
	m_listenerId(0),
	m_actionMapListeners(2),
	m_flownodesListeners(2),
	m_name(name),
	m_iNumRebindedInputs(0)
{
}

//------------------------------------------------------------------------
CActionMap::~CActionMap()
{
	Clear();
}

//------------------------------------------------------------------------
void CActionMap::Release()
{
	m_pActionMapUpr->RemoveActionMap(m_name);
};

//------------------------------------------------------------------------
void CActionMap::Clear()
{
	m_pActionMapUpr->RemoveBind(this);
	m_actions.clear();
	SetNumRebindedInputs(0);
}

//------------------------------------------------------------------------
bool CActionMap::CreateAction(const ActionId& actionId)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter != m_actions.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::CreateAction: Unable to create action, %s already exists", actionId.c_str());
		return false;
	}

	CActionMapAction action;
	action.SetActionId(actionId);
	action.SetParentActionMap(this);

	m_actions[actionId] = action;

	return true;
}

//------------------------------------------------------------------------
bool CActionMap::RemoveAction(const ActionId& actionId)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter == m_actions.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::RemoveAction: Failed to find action: %s", actionId.c_str());
		return false;
	}

	CActionMapAction& action = iter->second;
	m_pActionMapUpr->RemoveBind(&action);

	// Update new rebind count
	i32 newRebindCount = GetNumRebindedInputs();

	i32 numActionInputs = action.GetNumActionInputs();
	for (i32 i = 0; i < numActionInputs; ++i)
	{
		SActionInput* pActionInput = action.GetActionInput(i);
		DRX_ASSERT(pActionInput != NULL);

		if (pActionInput->input != pActionInput->defaultInput)
		{
			--newRebindCount;
		}
	}

	if (newRebindCount != GetNumRebindedInputs())
	{
		SetNumRebindedInputs(newRebindCount);
	}

	m_actions.erase(iter);

	return true;
}

//------------------------------------------------------------------------
bool CActionMap::AddActionInput(const ActionId& actionId, const SActionInput& actionInput, i32k iByDeviceIndex)
{
	CActionMapAction* pAction = NULL;

	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter == m_actions.end()) // Don't have an action, so create one to hold the input
	{
		pAction = CreateAndGetAction(actionId);
		DRX_ASSERT(pAction != NULL);
		if (!pAction)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddActionInput: Failed to create action: %s", actionId.c_str());
			return false;
		}
	}
	else
	{
		pAction = &iter->second;
		DRX_ASSERT(pAction != NULL);
	}

	return pAction->AddActionInput(actionInput, iByDeviceIndex);
}

//------------------------------------------------------------------------
bool CActionMap::AddAndBindActionInput(const ActionId& actionId, const SActionInput& actionInput)
{
	CActionMapAction* pAction = NULL;
	TActionMap::iterator iAction = m_actions.find(actionId);
	if (iAction == m_actions.end())
	{
		pAction = CreateAndGetAction(actionId);
		DRX_ASSERT(pAction != NULL);
		if (pAction == NULL)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddAndBindActionInput: Failed to create action: %s", actionId.c_str());
			return false;
		}
	}
	else
	{
		pAction = &iAction->second;
		DRX_ASSERT(pAction != NULL);
	}
	SActionInput* pNewActionInput = pAction->AddAndGetActionInput(actionInput);
	if (pNewActionInput != NULL)
	{
		return m_pActionMapUpr->AddBind(this, pAction, pNewActionInput);
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------
bool CActionMap::RemoveActionInput(const ActionId& actionId, tukk szInput)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter == m_actions.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::RemoveActionInput: Failed to find action: %s", actionId.c_str());
		return false;
	}

	CActionMapAction& action = iter->second;

	u32 inputCRC = CCrc32::ComputeLowercase(szInput);

	SActionInput* pActionInput = action.FindActionInput(inputCRC);
	if (pActionInput == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::RemoveActionInput: Failed to find action input for input: %s", szInput);
		return false;
	}

	bool bResult = m_pActionMapUpr->RemoveBind(this, &action, pActionInput);
	if (!bResult)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::RemoveActionInput: Failed to remove binding for action input with input: %s", szInput);
		return false;
	}

	return action.RemoveActionInput(inputCRC);
}

//------------------------------------------------------------------------
bool CActionMap::ReBindActionInput(const ActionId& actionId, tukk szCurrentInput, tukk szNewInput)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter != m_actions.end())
	{
		CActionMapAction& action = iter->second;
		return ReBindActionInput(&action, szCurrentInput, szNewInput);
	}

	DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Unable to rebind action, %s, doesn't exist", actionId.c_str());
	return false;
}

//------------------------------------------------------------------------
bool CActionMap::ReBindActionInput(const ActionId& actionId,
                                   tukk szNewInput,
                                   const EActionInputDevice device,
                                   i32k iByDeviceIndex)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter == m_actions.end())
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Can't find action: %s", actionId.c_str());
		return false;
	}

	CActionMapAction& action = iter->second;
	return ReBindActionInput(&action, szNewInput, device, iByDeviceIndex);
}

//------------------------------------------------------------------------
CActionMapAction* CActionMap::CreateAndGetAction(const ActionId& actionId)
{
	TActionMap::iterator iter = m_actions.find(actionId);
	if (iter != m_actions.end())
	{
		return NULL;
	}

	CActionMapAction action;
	action.SetActionId(actionId);
	action.SetParentActionMap(this);

	m_actions[actionId] = action;

	return &m_actions[actionId];
}

//------------------------------------------------------------------------
bool CActionMap::AddAndBindActionInput(CActionMapAction* pAction, const SActionInput& actionInput)
{
	DRX_ASSERT(pAction != NULL);
	if (!pAction)
		return false;

	SActionInput* pAddedActionInput = pAction->AddAndGetActionInput(actionInput);
	if (!pAddedActionInput)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddAndBindActionInput: Failed adding actioninput for input: %s", actionInput.input.c_str());
		return false;
	}

	bool bResult = m_pActionMapUpr->AddBind(this, pAction, pAddedActionInput);
	if (!bResult)
	{
		pAction->RemoveActionInput(pAddedActionInput->inputCRC);
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::AddAndBindActionInput: Failed binding actioninput");
		return false;
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMap::ReBindActionInput(CActionMapAction* pAction, tukk szCurrentInput, tukk szNewInput)
{
	u32 inputCRC = CCrc32::ComputeLowercase(szCurrentInput);

	SActionInput* pActionInput = pAction->FindActionInput(inputCRC);
	if (!pActionInput)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Can't find input: %s for action: %s", szCurrentInput, pAction->GetActionId().c_str());
		return false;
	}

	return ReBindActionInput(pAction, *pActionInput, szNewInput);
}

//------------------------------------------------------------------------
bool CActionMap::ReBindActionInput(CActionMapAction* pAction, SActionInput& actionInput, tukk szNewInput)
{
	DRX_ASSERT(pAction != NULL);

	if (actionInput.input == szNewInput) // Rebinding to same input
		return false;

	if (m_pActionMapUpr->HasBind(this, pAction, &actionInput)) // Might not be binded if cleared
	{
		bool bResult = m_pActionMapUpr->RemoveBind(this, pAction, &actionInput);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Failed removing bind for input: %s for action: %s", actionInput.input.c_str(), pAction->GetActionId().c_str());
			return false;
		}
	}

	actionInput.input = szNewInput;

	// Possible to assign empty to clear the binding
	if (szNewInput != NULL && strcmp(szNewInput, "") != 0)
	{
		actionInput.inputCRC = CCrc32::ComputeLowercase(szNewInput);

		bool bResult = m_pActionMapUpr->AddBind(this, pAction, &actionInput);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Failed removing bind for input: %s for action: %s", actionInput.input.c_str(), pAction->GetActionId().c_str());
			return false;
		}
	}

	// If input now equals the default, nolonger needs the mark from the previous rebind
	if (actionInput.input == actionInput.defaultInput)
	{
		SetNumRebindedInputs(GetNumRebindedInputs() - 1);
		pAction->SetNumRebindedInputs(pAction->GetNumRebindedInputs() - 1);
	}
	else // Remember this rebind
	{
		SetNumRebindedInputs(GetNumRebindedInputs() + 1);
		pAction->SetNumRebindedInputs(pAction->GetNumRebindedInputs() + 1);
	}

	return true;
}

//------------------------------------------------------------------------
bool CActionMap::ReBindActionInput(CActionMapAction* pAction,
                                   tukk szNewInput,
                                   const EActionInputDevice device,
                                   i32k iByDeviceIndex)
{

	SActionInput* pActionInput = pAction->GetActionInput(device, iByDeviceIndex);
	if (!pActionInput)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Can't get input by index: %d for action: %s", iByDeviceIndex, pAction->GetActionId().c_str());
		return false;
	}

	if (pActionInput->input == szNewInput) // Rebinding to same input
		return false;

	if (m_pActionMapUpr->HasBind(this, pAction, pActionInput)) // Might not be binded if cleared
	{
		bool bResult = m_pActionMapUpr->RemoveBind(this, pAction, pActionInput);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Failed removing bind for input: %s for action: %s", pActionInput->input.c_str(), pAction->GetActionId().c_str());
			return false;
		}
	}

	pActionInput->input = szNewInput;

	// Possible to assign empty to clear the binding
	if (szNewInput != NULL && strcmp(szNewInput, "") != 0)
	{
		pActionInput->inputCRC = CCrc32::ComputeLowercase(szNewInput);

		bool bResult = m_pActionMapUpr->AddBind(this, pAction, pActionInput);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::ReBindActionInput: Failed removing bind for input: %s for action: %s", pActionInput->input.c_str(), pAction->GetActionId().c_str());
			return false;
		}
	}
	else
	{
		pActionInput->inputCRC = 0;
	}

	// If input now equals the default, nolonger needs the mark from the previous rebind
	if (pActionInput->input == pActionInput->defaultInput)
	{
		SetNumRebindedInputs(GetNumRebindedInputs() - 1);
		pAction->SetNumRebindedInputs(pAction->GetNumRebindedInputs() - 1);
	}
	else // Remember this rebind
	{
		SetNumRebindedInputs(GetNumRebindedInputs() + 1);
		pAction->SetNumRebindedInputs(pAction->GetNumRebindedInputs() + 1);
	}

	return true;

}

//------------------------------------------------------------------------
void CActionMap::SetActionListener(EntityId id)
{
	m_listenerId = id;
}

//------------------------------------------------------------------------
EntityId CActionMap::GetActionListener() const
{
	return m_listenerId;
}

//------------------------------------------------------------------------
bool CActionMap::Reset()
{
	m_enabled = !stricmp(GetName(), "default");

	for (TActionMap::iterator it = m_actions.begin(); it != m_actions.end(); ++it)
	{
		CActionMapAction& action = it->second;

		i32k numInputs = action.GetNumActionInputs();
		for (i32 i = 0; i < numInputs; i++)
		{
			SActionInput* pActionInput = action.GetActionInput(i);
			DRX_ASSERT(pActionInput != NULL);

			if (pActionInput->input == pActionInput->defaultInput) // Input is already default
				continue;

			if (!ReBindActionInput(&action, *pActionInput, pActionInput->defaultInput))
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::Reset: Failed resetting binding to default input in action: %s", action.GetActionId().c_str());
				return false;
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
const IActionMapAction* CActionMap::GetAction(const ActionId& actionId) const
{
	TActionMap::const_iterator actionIter = m_actions.find(actionId);
	if (actionIter == m_actions.end())
		return NULL;

	const CActionMapAction& action = actionIter->second;

	return &action;
}

//------------------------------------------------------------------------
IActionMapAction* CActionMap::GetAction(const ActionId& actionId)
{
	TActionMap::iterator actionIter = m_actions.find(actionId);
	if (actionIter == m_actions.end())
		return NULL;

	IActionMapAction& action = actionIter->second;

	return &action;
}

//------------------------------------------------------------------------
bool CActionMap::CanProcessInput(const SInputEvent& inputEvent, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput)
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION);
	bool bRes = false;

	float fCurrTime = gEnv->pTimer->GetCurrTime();
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	if (pGameFramework && pGameFramework->IsGamePaused())
	{
		fCurrTime = gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI);
	}

	if (m_enabled)
	{
		// Normal buttons + keys send a press, down, release events
		// Analog events only send changed
		// So deal with these totally differently
		if (pActionInput->activationMode & eAAM_AnalogCompare)
		{
			switch (pActionInput->analogCompareOp) // Check if fulfills analog condition
			{
			case eAACO_Equals:
				{
					if (fabs(inputEvent.value - pActionInput->fAnalogCompareVal) < FLT_EPSILON)
					{
						bRes = true;
					}
				}
				break;
			case eAACO_NotEquals:
				{
					if (fabs(inputEvent.value - pActionInput->fAnalogCompareVal) >= FLT_EPSILON)
					{
						bRes = true;
					}
				}
				break;
			case eAACO_GreaterThan:
				{
					if (inputEvent.value > pActionInput->fAnalogCompareVal)
					{
						bRes = true;
					}
				}
				break;
			case eAACO_LessThan:
				{
					if (inputEvent.value < pActionInput->fAnalogCompareVal)
					{
						bRes = true;
					}
				}
				break;
			}

			if (bRes)
			{
				// Since analog inputs dont have the concept of being down, create a refire timer to simulate hold
				// and remove this when analog condition nolonger fulfilled
				if (!m_pActionMapUpr->IsCurrentlyRefiringInput()) // Input event is from refire, dont try to refire a refired event
				{
					m_pActionMapUpr->UpdateRefireData(inputEvent, pActionMap, pAction, pActionInput);
				}

				if (pActionInput->bAnalogConditionFulfilled != true)
				{
					pActionInput->fPressedTime = fCurrTime;
					pActionInput->fLastRepeatTime = fCurrTime;
					pActionInput->bHoldTriggerFired = false;
					pActionInput->fCurrentHoldValue = 0.f;
					pActionInput->bAnalogConditionFulfilled = true;
					pActionInput->currentState = eIS_Pressed;
				}
				else if (pActionInput->bHoldTriggerFired)
				{
					pActionInput->currentState = eIS_Changed;
				}

				// Although condition is true, result value can be set false again if hold timer conditions aren't true
				// Only check if action should be processed here
				bRes = IsActionInputTriggered(inputEvent, pActionMap, pAction, pActionInput);
			}
			else // Didn't fulfill analog condition
			{
				if (!m_pActionMapUpr->IsCurrentlyRefiringInput() && pActionInput->bAnalogConditionFulfilled != false)
				{
					pActionInput->bAnalogConditionFulfilled = false;
					m_pActionMapUpr->RemoveRefireData(pActionMap, pAction, pActionInput);
				}
				pActionInput->fCurrentHoldValue = 0.f;
				pActionInput->currentState = eIS_Released;
			}
		}
		else if (inputEvent.state == eIS_Pressed)
		{
			pActionInput->fPressedTime = fCurrTime;

			if (pActionInput->activationMode & eAAM_OnHold) // Needed for hold
			{
				pActionInput->fLastRepeatTime = fCurrTime;
				if (pActionInput->fHoldTriggerDelay > 0.f)
				{
					pActionInput->bHoldTriggerFired = false;
				}
				else // Hold trigger delay is immediate, same with press event, so dont fire twice
				{
					pActionInput->bHoldTriggerFired = true;
				}

				pActionInput->fCurrentHoldValue = 0.f;
			}

			// Check if the action should be processed
			bRes = IsActionInputTriggered(inputEvent, pActionMap, pAction, pActionInput);
		}
		else // All other cases
		{
			// Check if the action should be processed
			bRes = IsActionInputTriggered(inputEvent, pActionMap, pAction, pActionInput);
		}

		// Remember the triggered input

		// Change action's current state
		if (!bRes)
		{
			if (inputEvent.state == eIS_Down && pActionInput->currentState == eIS_Pressed && (pActionInput->activationMode & eAAM_OnRelease))
				pActionInput->currentState = eIS_Pressed;
			else
				pActionInput->currentState = eIS_Unknown;
		}
		else if (inputEvent.state == eIS_Changed && !(pActionInput->activationMode & eAAM_AnalogCompare))
		{
			pActionInput->currentState = eIS_Changed;
		}
		else if (inputEvent.state == eIS_Down)
		{
			pActionInput->currentState = eIS_Down;
		}
		else if (pActionInput->activationMode & (eAAM_OnPress | eAAM_OnRelease))
		{
			if (inputEvent.state == eIS_Released || inputEvent.state == eIS_Pressed)
				pActionInput->currentState = inputEvent.state;
		}
	}

	return bRes;
}

//------------------------------------------------------------------------
bool CActionMap::IsActionInputTriggered(const SInputEvent& inputEvent, CActionMap* pActionMap, CActionMapAction* pAction, SActionInput* pActionInput) const
{
	bool bRes = false;

	if ((inputEvent.modifiers & eMM_Modifiers) &&
	    (pActionInput->activationMode & eAAM_NoModifiers))
	{
		return bRes;
	}

	float fCurrTime = gEnv->pTimer->GetCurrTime();
	IGameFramework* pGameFramework = gEnv->pGameFramework;
	if (pGameFramework && pGameFramework->IsGamePaused())
	{
		fCurrTime = gEnv->pTimer->GetCurrTime(ITimer::ETIMER_UI);
	}

	// currentMode -> bit flag based on key state needed for trigger, and actual key state
	// Special handling for certain types of logic
	i32 currentMode = pActionInput->activationMode & inputEvent.state;
	const float fTimePressed = fCurrTime - pActionInput->fPressedTime;
	const bool bJustPressed = fabs(fTimePressed) < FLT_EPSILON;

	if ((inputEvent.state == eIS_Down) && (pActionInput->activationMode & eAAM_OnHold))
	{
		if (bJustPressed)
		{
			return bRes; // When holding, a press event is handled and immediately on same time frame a hold event is passed in, ignore it
		}
		else
		{
			currentMode = eAAM_Always; // Held events aren't checked in the above line, always allow them
		}
	}
	else if (inputEvent.state == eIS_Changed) // Analog events aren't checked in the above line, always allow them
	{
		if (bJustPressed && ((pActionInput->activationMode & eAAM_OnPress) == 0)) // This is a press from analog event, but input doesn't handle press, ignore
		{
			return bRes;
		}

		currentMode = eAAM_Always;
	}
	else if (inputEvent.state == eIS_Released)
	{
		pActionInput->fCurrentHoldValue = 0.f;

		//ignore if we have a release threshold and it's been exceeded
		if (pActionInput->fReleaseTriggerThreshold >= 0.0f && fTimePressed >= pActionInput->fReleaseTriggerThreshold)
		{
			return bRes;
		}
	}

	// Get blocking type based on how the action was triggered
	if (currentMode)
	{
		// Support initial hold trigger delay and repeating
		if ((pActionInput->activationMode & eAAM_OnHold) &&
		    (inputEvent.state == eIS_Down || (inputEvent.state == eIS_Changed && bJustPressed == false)))   // Down is for digital controls, changed is for analog (For analog, won't reach here if analog condition is false, only for analog hold, press will go below like normal press)
		{
			if (pActionInput->bHoldTriggerFired) // Initial hold trigger already fired
			{
				if (fCurrTime < pActionInput->fLastRepeatTime)
				{
					pActionInput->fLastRepeatTime = fCurrTime;
				}

				if (pActionInput->fHoldRepeatDelay != -1.0f && fCurrTime - pActionInput->fLastRepeatTime >= pActionInput->fHoldRepeatDelay) // fHoldRepeatDelay of -1.0f means no repeat
				{
					pActionInput->fLastRepeatTime = fCurrTime;
					bRes = true;
				}
			}
			else
			{
				const bool bIsCurrentKey = (m_pActionMapUpr->GetIncomingInputKeyID() == inputEvent.keyId);
				// Support overriding hold trigger timer if repeating same input
				if ((m_pActionMapUpr->IsRepeatedInputHoldTriggerFired()) &&     // Could have been interrupted by another held key
				    (bIsCurrentKey) &&                                              // Need to check since could have multiple held keys
				    (pActionInput->fHoldTriggerDelayRepeatOverride >= FLT_EPSILON)) // Default is -1.0f which won't activate this
				{
					if (fTimePressed >= pActionInput->fHoldTriggerDelayRepeatOverride)
					{
						pActionInput->bHoldTriggerFired = true;
						pActionInput->fCurrentHoldValue = 1.f;
						pActionInput->fLastRepeatTime = fCurrTime;
						bRes = true;
					}
				}
				else if (fTimePressed >= pActionInput->fHoldTriggerDelay)
				{
					pActionInput->bHoldTriggerFired = true;
					pActionInput->fCurrentHoldValue = 1.f;
					pActionInput->fLastRepeatTime = fCurrTime;
					if (m_pActionMapUpr->IsIncomingInputRepeated() && bIsCurrentKey) // Need to check since could have multiple held keys
					{
						m_pActionMapUpr->SetRepeatedInputHoldTriggerFired(true);
					}
					bRes = true;
				}
				else
				{
					if (pActionInput->fHoldTriggerDelay > 0.f)
					{
						pActionInput->fCurrentHoldValue = (fTimePressed / pActionInput->fHoldTriggerDelay);
					}
					else
					{
						pActionInput->fCurrentHoldValue = 0.f;
					}
				}
			}
		}
		else if ((inputEvent.state == eIS_Pressed) &&
		         (pActionInput->fPressTriggerDelay >= FLT_EPSILON) &&  // Handle delayed press situation
		         (!m_pActionMapUpr->IsCurrentlyRefiringInput()))   // Input event is from refire, dont try to refire a refired event
		{
			if ((m_pActionMapUpr->IsIncomingInputRepeated()) &&
			    (m_pActionMapUpr->GetIncomingInputKeyID() == inputEvent.keyId) && // Repeated same input, allow override if is one
			    (pActionInput->fPressTriggerDelayRepeatOverride >= FLT_EPSILON))      // Default is -1.0f which won't activate this
			{
				// Remove delayed press fire
				m_pActionMapUpr->RemoveRefireData(pActionMap, pAction, pActionInput);

				bRes = true;
			}
			else // Not repeated key, must wait for delay before firing
			{
				// Check press delay priority, if greater or equal to current, override
				i32 currentHighestDelayPriority = m_pActionMapUpr->GetHighestPressDelayPriority();
				if (pActionInput->iPressDelayPriority >= currentHighestDelayPriority)
				{
					m_pActionMapUpr->RemoveAllDelayedPressRefireData();
					m_pActionMapUpr->UpdateRefireData(inputEvent, pActionMap, pAction, pActionInput); // Will fire again after delay
				}
				bRes = false;
			}
		}
		else if ((inputEvent.state == eIS_Released) &&
		         (pActionInput->fPressTriggerDelay >= FLT_EPSILON) &&  // Handle delayed press situation
		         (!m_pActionMapUpr->IsCurrentlyRefiringInput()))   // Input event is from refire, dont try to refire a refired event
		{
			if (fTimePressed - pActionInput->fPressTriggerDelay < FLT_EPSILON) // Press delay hasn't fired yet, release should be fired right after delayed press
			{
				m_pActionMapUpr->SetRefireDataDelayedPressNeedsRelease(inputEvent, pActionMap, pAction, pActionInput, true);
				bRes = false;
			}
			else // Ok to fire release since delayed press already fired
			{
				bRes = true;
			}
		}
		else // Not held
		{
			bRes = true;
		}
	}

	return bRes;
}

//------------------------------------------------------------------------
void CActionMap::InputProcessed()
{
	IActionListener* pEntityListener = NULL;

	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_listenerId))
		if (CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER))
			pEntityListener = pGameObject;

	if (pEntityListener)
		pEntityListener->AfterAction();

	for (TActionMapListeners::Notifier notify(m_actionMapListeners); notify.IsValid(); notify.Next())
	{
		notify->AfterAction();
	}
}

//------------------------------------------------------------------------
void CActionMap::ReleaseActionsIfActive()
{
	TActionMap::iterator it = m_actions.begin();
	TActionMap::iterator end = m_actions.end();
	for (; it != end; ++it)
	{
		CActionMapAction& action = it->second;
		ReleaseActionIfActiveInternal(action);
	}
}

//------------------------------------------------------------------------
void CActionMap::ReleaseFilteredActions()
{
	IActionListener* pEntityListener = NULL;

	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_listenerId))
		if (CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER))
			pEntityListener = pGameObject;

	IActionListener* pListener = pEntityListener;
	if (!pListener)
		return;

	for (TActionMap::iterator it = m_actions.begin(), eit = m_actions.end(); it != eit; ++it)
	{
		CActionMapAction& action = it->second;

		i32 iNumActionInputs = action.GetNumActionInputs();
		for (i32 i = 0; i < iNumActionInputs; ++i)
		{
			SActionInput* pActionInput = action.GetActionInput(i);
			DRX_ASSERT(pActionInput != NULL);

			bool isPressedOrDown = (pActionInput->currentState == eIS_Pressed) || (pActionInput->currentState == eIS_Down);
			bool isChanged = pActionInput->currentState == eIS_Changed;

			if ((!isPressedOrDown && !isChanged) || !m_pActionMapUpr->ActionFiltered(it->first))
				continue;

			pActionInput->fCurrentHoldValue = 0.f;

			i32 currentMode = pActionInput->activationMode & eIS_Released;
			if (currentMode || isChanged)
			{
				pListener->OnAction(it->first, currentMode, 0);

				const TActionListeners& extraActionListeners = m_pActionMapUpr->GetExtraActionListeners();
				for (TActionListeners::const_iterator extraListener = extraActionListeners.begin(); extraListener != extraActionListeners.end(); ++extraListener)
					(*extraListener)->OnAction(it->first, currentMode, 0);

				NotifyExtraActionListeners(it->first, currentMode, 0);
			}
		}
	}
}

//------------------------------------------------------------------------
void CActionMap::AddExtraActionListener(IActionListener* pExtraActionListener)
{
	m_actionMapListeners.Add(pExtraActionListener);
}

//------------------------------------------------------------------------
void CActionMap::RemoveExtraActionListener(IActionListener* pExtraActionListener)
{
	m_actionMapListeners.Remove(pExtraActionListener);
}

//------------------------------------------------------------------------
void CActionMap::NotifyFlowNodeActionListeners(const ActionId& action, i32 currentState, float value)
{
	for (TActionMapListeners::Notifier notify(m_flownodesListeners); notify.IsValid(); notify.Next())
	{
		notify->OnAction(action, currentState, value);
	}
}

//------------------------------------------------------------------------
void CActionMap::AddFlowNodeActionListener(IActionListener* pFlowgraphNodeActionListener)
{
	m_flownodesListeners.Add(pFlowgraphNodeActionListener);
}

//------------------------------------------------------------------------
void CActionMap::RemoveFlowNodeActionListener(IActionListener* pFlowgraphNodeActionListener)
{
	m_flownodesListeners.Remove(pFlowgraphNodeActionListener);
}

//------------------------------------------------------------------------
void CActionMap::NotifyExtraActionListeners(const ActionId& action, i32 currentState, float value)
{
	for (TActionMapListeners::Notifier notify(m_actionMapListeners); notify.IsValid(); notify.Next())
	{
		notify->OnAction(action, currentState, value);
	}
}

//------------------------------------------------------------------------
bool CActionMap::LoadFromXML(const XmlNodeRef& actionMapNode)
{
	i32k iNumDeviceData = m_pActionMapUpr->GetNumInputDeviceData();
	bool bResult;

	i32 iNumActions = actionMapNode->getChildCount();
	for (i32 i = 0; i < iNumActions; ++i)
	{
		XmlNodeRef actionNode = actionMapNode->getChild(i);
		if (strcmp(actionNode->getTag(), "action") != 0)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Found non action child, actionmaps should only have action children");
			continue;
		}

		tukk szActionName = actionNode->getAttr("name");
		if (!szActionName || strcmp(szActionName, "") == 0)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Action missing name, ignoring action");
			continue;
		}

		ActionId actionName = ActionId(szActionName);
		CActionMapAction* pAction = (CActionMapAction*)GetAction(actionName);
		if (pAction != NULL)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Can't add action: %s, already exists", szActionName);
			continue;
		}

		pAction = CreateAndGetAction(actionName);
		if (!pAction)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed to create action: %s", szActionName);
			continue;
		}

		// Actioninput used originally contains default values from constructor, then each LoadActionInputFromXML is cumulative
		// i.e. Only the values in xml will be set to allow attributes specified at topmost level in xml to apply to all children
		SActionInput actionInput;

		if (actionNode->getNumAttributes() > 1) // Contains attributes to apply to all inputs
		{
			bResult = LoadActionInputAttributesFromXML(actionNode, actionInput);
			if (!bResult)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed loading action input attributes for action: %s", szActionName);
				continue;
			}

			// Support specifying all devices in 1 line like before

			// All inputs are under a devicetype for organization
			// Devicemapping data should be called from game level using
			// IActionMapUpr::AddInputDeviceMapping
			for (i32 j = 0; j < iNumDeviceData; ++j)
			{
				const SActionInputDeviceData* pInputDeviceData = m_pActionMapUpr->GetInputDeviceDataByIndex(j);

				tukk szInput = actionNode->getAttr(pInputDeviceData->m_typeStr);
				if (szInput == NULL || strcmp(szInput, "") == 0) // Not in xml which is fine
					continue;

				actionInput.input = szInput;
				actionInput.defaultInput = szInput;
				actionInput.inputDevice = pInputDeviceData->m_type;

				bResult = AddAndBindActionInput(pAction, actionInput);
				if (!bResult)
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed binding action input for action: %s, with input: %s", szActionName, szInput);
					continue;
				}
			}
		}

		i32 iNumDeviceChildren = actionNode->getChildCount();

		// Check now for action inputs specified per platform
		if (iNumDeviceChildren > 0)
		{
			// Support specifying separate device with attributes in their own child
			for (i32 j = 0; j < iNumDeviceChildren; ++j)
			{
				XmlNodeRef inputDeviceNode = actionNode->getChild(j);

				EActionInputDevice inputDeviceNodeDeviceType = eAID_Unknown;

				const SActionInputDeviceData* pInputDeviceData = m_pActionMapUpr->GetInputDeviceDataByType(inputDeviceNode->getTag());
				if (pInputDeviceData != NULL)
				{
					inputDeviceNodeDeviceType = pInputDeviceData->m_type;
				}
				else
				{
					continue;
				}

				SActionInput platformInput = actionInput; // Need to copy parent settings since don't want to overwrite for the other nodes

				bResult = LoadActionInputAttributesFromXML(inputDeviceNode, platformInput);
				if (!bResult)
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed loading action input attributes for action: %s", szActionName);
					continue;
				}

				tukk szPlatformInput = inputDeviceNode->getAttr("input");
				if (szPlatformInput != NULL && strcmp(szPlatformInput, "") != 0) // Device specific key is here, dont need to look for children
				{
					platformInput.input = szPlatformInput;
					platformInput.defaultInput = szPlatformInput;
					platformInput.inputDevice = inputDeviceNodeDeviceType;

					bResult = AddAndBindActionInput(pAction, platformInput);
					if (!bResult)
					{
						DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed binding action input for action: %s, with input: %s", szActionName, szPlatformInput);
						continue;
					}
				}
				else
				{
					i32 iNumInputData = inputDeviceNode->getChildCount();
					for (i32 k = 0; k < iNumInputData; k++)
					{
						// Go through each of the separate inputs
						SActionInput platformChildInput = platformInput;

						XmlNodeRef platformChildNode = inputDeviceNode->getChild(k);
						if (strcmp(platformChildNode->getTag(), "inputdata") != 0)
						{
							DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Children inside platform tags must be inputdata");
							continue;
						}

						tukk szPlatformChildInput = platformChildNode->getAttr("input");
						if (szPlatformChildInput == NULL || strcmp(szPlatformChildInput, "") == 0)
						{
							DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: inputdata tag must contain input");
							continue;
						}

						bResult = LoadActionInputAttributesFromXML(platformChildNode, platformChildInput);
						if (!bResult)
						{
							DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed loading action input attributes on platform child input node for action: %s", szActionName);
							continue;
						}

						platformChildInput.input = szPlatformChildInput;
						platformChildInput.defaultInput = szPlatformChildInput;
						platformChildInput.inputDevice = pInputDeviceData->m_type;

						bResult = AddAndBindActionInput(pAction, platformChildInput);
						if (!bResult)
						{
							DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed binding action input for action: %s, with input: %s", szActionName, szPlatformChildInput);
							continue;
						}
					}
				}
			}
		}
	}

	return true;
}

bool CActionMap::LoadRebindingDataFromXML(const XmlNodeRef& actionMapNode)
{
	i32 iNumActions = actionMapNode->getChildCount();
	for (i32 i = 0; i < iNumActions; ++i)
	{
		XmlNodeRef actionNode = actionMapNode->getChild(i);
		if (strcmp(actionNode->getTag(), "action") != 0)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadRebindingDataFromXML: Found non action child, actionmaps should only have action children");
			continue;
		}

		tukk szActionName = actionNode->getAttr("name");
		if (szActionName == NULL || strcmp(szActionName, "") == 0)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadRebindingDataFromXML: Action missing name, ignoring action");
			continue;
		}

		ActionId actionName = ActionId(szActionName);
		CActionMapAction* pAction = (CActionMapAction*)GetAction(actionName);
		if (pAction == NULL)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadRebindingDataFromXML: Action: %s, doesn't exist, can't rebind", szActionName);
			continue;
		}

		// Check now for action inputs specified per platform
		i32 iNumRebindInputs = actionNode->getChildCount();
		if (iNumRebindInputs == 0)
			continue; // Shouldn't have empty action tags

		for (i32 j = 0; j < iNumRebindInputs; ++j)
		{
			XmlNodeRef inputNode = actionNode->getChild(j);
			tukk szInput = inputNode->getAttr("input");
			if (szInput == NULL)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadRebindingDataFromXML: Action: %s, has an input tag missing input attribute", szActionName);
				continue;
			}

			tukk szDevice = inputNode->getAttr("device");
			if (szDevice == NULL || strcmp(szDevice, "") == 0)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadRebindingDataFromXML: Action: %s, has an input tag missing device attribute", szActionName);
				continue;
			}

			const SActionInputDeviceData* pInputDeviceData = m_pActionMapUpr->GetInputDeviceDataByType(szDevice);
			if (pInputDeviceData == NULL)
			{
				continue;
			}

			// Want to allow rebinding without defaultInput to avoid data dependencies
			tukk szDefaultInput = inputNode->getAttr("defaultInput");
			if (szDefaultInput == NULL || strcmp(szDefaultInput, "") == 0) // Assumes rebinding the first input for that device
			{
				if (stricmp(szInput, "DEFAULT") == 0) // Reset to default
				{
					SActionInput* pActionInput = pAction->GetActionInput(pInputDeviceData->m_type, 0);
					if (pActionInput == NULL)
					{
						DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadFromXML: Failed trying to find actioninput for input device type: %s", szDevice);
						continue;
					}

					ReBindActionInput(pAction, pActionInput->defaultInput, pInputDeviceData->m_type, 0);
				}
				else
				{
					ReBindActionInput(pAction, szInput, pInputDeviceData->m_type, 0);
				}
			}
			else
			{
				if (stricmp(szInput, "DEFAULT") == 0) // Reset to default
				{
					ReBindActionInput(pAction, szDefaultInput, szDefaultInput);
				}
				else
				{
					ReBindActionInput(pAction, szDefaultInput, szInput);
				}
			}
		}
	}

	return true;
}

bool CActionMap::SaveRebindingDataToXML(XmlNodeRef& actionMapNode) const
{
	i32k iNumDeviceData = m_pActionMapUpr->GetNumInputDeviceData();

#if 0
	// for debug reasons, we sort the ActionMap alphabetically
	// DrxName normally sorts by pointer address
	std::map<ActionId, SAction, DrxNameSorter> sortedActions(m_actions.begin(), m_actions.end());
	std::map<ActionId, SAction, DrxNameSorter>::const_iterator iter = sortedActions.begin();
	std::map<ActionId, SAction, DrxNameSorter>::const_iterator iterEnd = sortedActions.end();
#else
	TActionMap::const_iterator iter = m_actions.begin();
	TActionMap::const_iterator iterEnd = m_actions.end();
#endif

	actionMapNode->setAttr("name", m_name);
	for (; iter != iterEnd; ++iter)
	{
		ActionId actionId = iter->first;
		const CActionMapAction& action = iter->second;
		if (action.GetNumRebindedInputs() == 0)
			continue;

		XmlNodeRef actionNode = actionMapNode->newChild("action");
		actionNode->setAttr("name", actionId.c_str());

		i32 numActionInputs = action.GetNumActionInputs();
		for (i32 i = 0; i < numActionInputs; ++i)
		{
			const SActionInput* pActionInput = action.GetActionInput(i);
			DRX_ASSERT(pActionInput != NULL);

			if (pActionInput->input == pActionInput->defaultInput)
			{
				continue;
			}

			const SActionInputDeviceData* pInputDeviceData = m_pActionMapUpr->GetInputDeviceDataByType(pActionInput->inputDevice);
			if (pInputDeviceData == NULL)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::SaveRebindingDataToXML: Failed to find device string");
				continue;
			}

			XmlNodeRef inputDeviceNode = actionNode->newChild("rebind");
			inputDeviceNode->setAttr("device", pInputDeviceData->m_typeStr.c_str());
			inputDeviceNode->setAttr("input", pActionInput->input.c_str());
			inputDeviceNode->setAttr("defaultInput", pActionInput->defaultInput.c_str());
		}
	}

	return true;
}

bool CActionMap::LoadActionInputAttributesFromXML(const XmlNodeRef& actionInputNode, SActionInput& actionInput)
{
	i32& activationFlags = actionInput.activationMode;

	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_ONPRESS_STR, eAAM_OnPress);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_ONRELEASE_STR, eAAM_OnRelease);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_ONHOLD_STR, eAAM_OnHold);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_ALWAYS_STR, eAAM_Always);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_CONSOLECMD_STR, eAAM_ConsoleCmd);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_NOMODIFIERS_STR, eAAM_NoModifiers);
	LoadActivationModeBitAttributeFromXML(actionInputNode, activationFlags, ACTIONINPUT_RETRIGGERABLE_STR, eAAM_Retriggerable);

	// If not specified, value is taken from constructor (0.0f)
	actionInputNode->getAttr(ACTIONINPUT_PRESSTRIGGERDELAY_STR, actionInput.fPressTriggerDelay);
	// If not specified, value is taken from constructor (-1.0f) (Disabled)
	actionInputNode->getAttr(ACTIONINPUT_PRESSTRIGGERDELAY_REPEATOVERRIDE_STR, actionInput.fPressTriggerDelayRepeatOverride);
	// If not specified, value is taken from constructor (0) (Higher number will override when both delayed (i.e. pressing top right on dpad)
	actionInputNode->getAttr(ACTIONINPUT_PRESSDELAYPRIORITY_STR, actionInput.iPressDelayPriority);

	// If not specified, value is taken from constructor (0.0f)
	actionInputNode->getAttr(ACTIONINPUT_HOLDTRIGGERDELAY_STR, actionInput.fHoldTriggerDelay);
	// If not specified, value is taken from constructor (0.0f) (Always repeats)
	actionInputNode->getAttr(ACTIONINPUT_HOLDREPEATDELAY_STR, actionInput.fHoldRepeatDelay);
	// If not specified, value is taken from constructor (-1.0f) (Disabled)
	actionInputNode->getAttr(ACTIONINPUT_HOLDTRIGGERDELAY_REPEATOVERRIDE_STR, actionInput.fHoldTriggerDelayRepeatOverride);
	// If not specified, value is taken from constructor (-1.0f) (Disabled)
	actionInputNode->getAttr(ACTIONINPUT_RELEASETRIGGERTHRESHOLD_STR, actionInput.fReleaseTriggerThreshold);

	if (!strcmp(actionInputNode->getAttr(ACTIONINPUT_USEANALOGCOMPARE_STR), "1"))
	{
		activationFlags |= eAAM_AnalogCompare;

		tukk analogCompareOp = actionInputNode->getAttr(ACTIONINPUT_ANALOGCOMPAREOP_STR);
		if (analogCompareOp == NULL || strcmp(analogCompareOp, "") == 0)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to find analogCompareOp");
			return false;
		}

		actionInput.analogCompareOp = GetAnalogCompareOpTypeFromStr(analogCompareOp);
		if (actionInput.analogCompareOp == eAACO_None)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to find analogCompareOp");
			return false;
		}

		bool bResult = actionInputNode->getAttr(ACTIONINPUT_ANALOGCOMPAREVAL_STR, actionInput.fAnalogCompareVal);
		if (!bResult)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to find analogCompareVal");
			return false;
		}
	}

	// Input blocking
	tukk inputsToBlock = actionInputNode->getAttr(ACTIONINPUT_INPUTSTOBLOCK_STR);
	if (inputsToBlock != NULL && strcmp(inputsToBlock, "") != 0)
	{
		SActionInputBlockData& inputBlockData = actionInput.inputBlockData;

		if (strcmp(inputsToBlock, ACTIONINPUT_INPUTSTOBLOCK_SPECIALTYPE_CLEARALL_STR) == 0)
		{
			inputBlockData.blockType = eAIBT_Clear;
		}
		else // Must contain list of inputs separated by |
		{
			// Must contain a valid block time
			actionInputNode->getAttr(ACTIONINPUT_INPUTBLOCKTIME_STR, inputBlockData.fBlockDuration);
			if (inputBlockData.fBlockDuration < FLT_EPSILON)
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Must have a valid blockTime, value is: %.2f", inputBlockData.fBlockDuration);
				return false;
			}

			// Read in the blocking activation (Default is always, matches all specified action activations)
			tukk inputBlockActivation = actionInputNode->getAttr(ACTIONINPUT_INPUTBLOCKACTIVATION_STR);
			bool useAlwaysActivation;
			if (inputBlockActivation != NULL && strcmp(inputBlockActivation, "") != 0)
			{
				useAlwaysActivation = false;
				string inputsBlockActivationStr(inputBlockActivation);
				i32 curPos = 0;
				string singleBlockActivation = inputsBlockActivationStr.Tokenize("|", curPos);
				bool bEmpty = singleBlockActivation.empty();
				if (bEmpty) // Only 1 input
				{
					singleBlockActivation = inputsBlockActivationStr;
					curPos = 0;
				}

				do
				{
					if (strcmp(singleBlockActivation, ACTIONINPUT_ONPRESS_STR) == 0)
					{
						inputBlockData.activationMode |= eIS_Pressed;
					}
					else if (strcmp(singleBlockActivation, ACTIONINPUT_ONHOLD_STR) == 0)
					{
						inputBlockData.activationMode |= eIS_Down;
					}
					else if (strcmp(singleBlockActivation, ACTIONINPUT_ONRELEASE_STR) == 0)
					{
						inputBlockData.activationMode |= eIS_Released;
					}
					else if (strcmp(inputBlockActivation, ACTIONINPUT_ALWAYS_STR) == 0)
					{
						useAlwaysActivation = true;
						break;
					}
					else
					{
						DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Invalid block activation: %s", singleBlockActivation.c_str());
						return false;
					}

					if (bEmpty)
					{
						break;
					}

					singleBlockActivation = inputsBlockActivationStr.Tokenize("|", curPos);
					bEmpty = singleBlockActivation.empty();
				}
				while (!bEmpty);
			}
			else
			{
				useAlwaysActivation = true;
			}

			if (useAlwaysActivation)
			{
				inputBlockData.activationMode |= eIS_Pressed;
				inputBlockData.activationMode |= eIS_Down;
				inputBlockData.activationMode |= eIS_Released;
			}

			// Now read in all the inputs to block, separated by |
			string inputsToBlockStr(inputsToBlock);
			std::vector<string> inputsToBlockVec;
			i32 curPos = 0;
			string singleInput = inputsToBlockStr.Tokenize("|", curPos);
			if (singleInput.empty()) // Only 1 input
			{
				inputsToBlockVec.push_back(inputsToBlockStr);
			}
			else
			{
				do
				{
					inputsToBlockVec.push_back(singleInput);
					singleInput = inputsToBlockStr.Tokenize("|", curPos);
				}
				while (!singleInput.empty());
			}

			// Read in the device index (Default is all device indices)
			i32 iDeviceIndex = -1;
			actionInputNode->getAttr(ACTIONINPUT_INPUTBLOCKDEVICEINDEX_STR, iDeviceIndex);

			if (iDeviceIndex != -1)
			{
				inputBlockData.bAllDeviceIndices = false;
				inputBlockData.deviceIndex = (u8)iDeviceIndex;
			}

			for (size_t i = 0; i < inputsToBlockVec.size(); i++)
			{
				const SInputSymbol* pInputSymbol = gEnv->pInput ? gEnv->pInput->GetSymbolByName(inputsToBlockVec[i]) : NULL;
				if (!pInputSymbol)
				{
					// Data is currently read all in on pc, dont show warnings until thats changed
					// DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to find input symbol for input name: %s", inputsToBlockVec[i].c_str());
					continue;
				}

				SActionInputBlocker inputBlockerToSend(pInputSymbol->keyId);
				inputBlockData.inputs.push_back(inputBlockerToSend);
			}
			if (!inputBlockData.inputs.empty())
			{
				inputBlockData.blockType = eAIBT_BlockInputs; // Only having this makes enables blocking
			}

			/* Data is currently read all in on pc, dont show warnings until thats changed
			   else
			   {
			   DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to find any inputs to block for inputs: %s", inputsToBlock);
			   return false;
			   }
			 */
		}
	}

	return true;
}

bool CActionMap::SaveActionInputAttributesToXML(XmlNodeRef& actionInputNode, const SActionInput& actionInput) const
{
	i32k activationFlags = actionInput.activationMode;

	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_ONPRESS_STR, eAAM_OnPress);
	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_ONRELEASE_STR, eAAM_OnRelease);
	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_ALWAYS_STR, eAAM_Always);
	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_CONSOLECMD_STR, eAAM_ConsoleCmd);
	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_NOMODIFIERS_STR, eAAM_NoModifiers);
	SaveActivationModeBitAttributeToXML(actionInputNode, activationFlags, ACTIONINPUT_RETRIGGERABLE_STR, eAAM_Retriggerable);

	if (actionInput.activationMode & eAAM_OnHold)
	{
		actionInputNode->setAttr(ACTIONINPUT_ONHOLD_STR, "1");
		actionInputNode->setAttr(ACTIONINPUT_HOLDTRIGGERDELAY_STR, actionInput.fHoldTriggerDelay);
		actionInputNode->setAttr(ACTIONINPUT_HOLDREPEATDELAY_STR, actionInput.fHoldRepeatDelay);
		actionInputNode->setAttr(ACTIONINPUT_HOLDTRIGGERDELAY_REPEATOVERRIDE_STR, actionInput.fHoldTriggerDelayRepeatOverride);
	}
	else
	{
		actionInputNode->setAttr(ACTIONINPUT_ONHOLD_STR, "0");
	}

	if (actionInput.activationMode & eAAM_AnalogCompare)
	{
		actionInputNode->setAttr(ACTIONINPUT_USEANALOGCOMPARE_STR, "1");
		actionInputNode->setAttr(ACTIONINPUT_ANALOGCOMPAREOP_STR, GetAnalogCompareOpStr(actionInput.analogCompareOp));
		actionInputNode->setAttr(ACTIONINPUT_ANALOGCOMPAREVAL_STR, actionInput.fAnalogCompareVal);
	}
	else
	{
		actionInputNode->setAttr(ACTIONINPUT_USEANALOGCOMPARE_STR, "0");
	}

	// Input blocking
	const SActionInputBlockData& inputBlockData = actionInput.inputBlockData;
	if (inputBlockData.blockType != eAIBT_None)
	{
		if (inputBlockData.blockType == eAIBT_Clear)
		{
			actionInputNode->setAttr(ACTIONINPUT_INPUTSTOBLOCK_STR, ACTIONINPUT_INPUTSTOBLOCK_SPECIALTYPE_CLEARALL_STR);
		}
		else // Normal blocking inputs
		{
			if (inputBlockData.inputs.empty())
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::SaveActionInputAttributesToXML: Failed, using normal blocking inputs type but dont have any inputs");
				return false;
			}

			if (inputBlockData.fBlockDuration <= (0.0f - FLT_EPSILON))
			{
				DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::SaveActionInputAttributesToXML: Failed, using normal blocking inputs type but don't have valid block duration");
				return false;
			}
			actionInputNode->setAttr(ACTIONINPUT_INPUTBLOCKTIME_STR, inputBlockData.fBlockDuration);

			// Save input blocking activation mode
			if ((inputBlockData.activationMode & eIS_Pressed) &&
			    (inputBlockData.activationMode & eIS_Down) &&
			    (inputBlockData.activationMode & eIS_Released))
			{
				actionInputNode->setAttr(ACTIONINPUT_INPUTBLOCKACTIVATION_STR, ACTIONINPUT_ALWAYS_STR);
			}
			else
			{
				DrxFixedStringT<32> inputBlockActivationStr("");
				bool bAddedActivationMode = false;
				if (inputBlockData.activationMode & eIS_Pressed)
				{
					inputBlockActivationStr = ACTIONINPUT_ONPRESS_STR;
					bAddedActivationMode = true;
				}
				if (inputBlockData.activationMode & eIS_Down)
				{
					if (bAddedActivationMode)
					{
						inputBlockActivationStr += "|";
					}
					inputBlockActivationStr += ACTIONINPUT_ONHOLD_STR;
					bAddedActivationMode = true;
				}
				if (inputBlockData.activationMode & eIS_Released)
				{
					if (bAddedActivationMode)
					{
						inputBlockActivationStr += "|";
					}
					inputBlockActivationStr += ACTIONINPUT_ONRELEASE_STR;
				}

				actionInputNode->setAttr(ACTIONINPUT_INPUTBLOCKACTIVATION_STR, inputBlockActivationStr.c_str());
			}

			// Now save the blocked inputs
			string blockedInputsStr("");
			bool bBlockAllDeviceIndices = true;
			for (size_t i = 0; i < inputBlockData.inputs.size(); i++)
			{
				const SActionInputBlocker& inputBlocker = inputBlockData.inputs[i];

				if (i != 0)
				{
					blockedInputsStr += "|";
				}

				tukk szKeyName = gEnv->pInput->GetKeyName(inputBlocker.keyId);
				if (!szKeyName)
				{
					DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CActionMap::LoadActionInputAttributesFromXML: Failed to translate keyname for key id: %d", (i32)inputBlocker.keyId);
					return false;
				}

				blockedInputsStr += szKeyName;
			}

			// Now save device index data
			if (!inputBlockData.bAllDeviceIndices) // Default is true so dont need to save to xml
			{
				actionInputNode->setAttr(ACTIONINPUT_INPUTBLOCKDEVICEINDEX_STR, inputBlockData.deviceIndex);
			}
		}
	}

	return true;
}

//------------------------------------------------------------------------
void CActionMap::LoadActivationModeBitAttributeFromXML(const XmlNodeRef& attributeNode, i32& activationFlags, tukk szActivationModeName, EActionActivationMode activationMode)
{
	i32 readFlag;

	if (attributeNode->getAttr(szActivationModeName, readFlag))
	{
		if (readFlag == 1)
		{
			activationFlags |= activationMode;
		}
		else
		{
			activationFlags &= ~activationMode;
		}
	}
}

//------------------------------------------------------------------------
void CActionMap::SaveActivationModeBitAttributeToXML(XmlNodeRef& attributeNode, i32k activationFlags, tukk szActivationModeName, EActionActivationMode activationMode) const
{
	if (activationFlags & activationMode)
	{
		attributeNode->setAttr(szActivationModeName, "1");
	}
	else
	{
		attributeNode->setAttr(szActivationModeName, "0");
	}
}

#define USE_SORTED_ACTIONS_ITERATOR
#undef  USE_SORTED_ACTIONS_ITERATOR

//------------------------------------------------------------------------
IActionMapActionIteratorPtr CActionMap::CreateActionIterator()
{
	class CActionInfo : public IActionMapActionIterator
	{
#ifndef USE_SORTED_ACTIONS_ITERATOR
		typedef TActionMap::iterator                                TIterator;
#else
		typedef std::map<ActionId, CActionMapAction, DrxNameSorter> TSortedActionMap;
		typedef TSortedActionMap::iterator                          TIterator;
#endif

	public:
		CActionInfo(TActionMap::iterator& itBegin, TActionMap::iterator& itEnd)
			: m_nRefs(0)
		{
#ifdef USE_SORTED_ACTIONS_ITERATOR
			m_sortedActions.insert(itBegin, itEnd);
			m_cur = m_sortedActions.begin();
			m_end = m_sortedActions.end();
#else
			m_cur = itBegin;
			m_end = itEnd;
#endif
		}

		virtual ~CActionInfo()
		{
		}

		const IActionMapAction* Next()
		{
			if (m_cur == m_end)
				return NULL;
			const ActionId& actionId = m_cur->first;
			const CActionMapAction& action = m_cur->second;

			++m_cur;
			return &action;
		}

		void AddRef()
		{
			++m_nRefs;
		}

		void Release()
		{
			if (--m_nRefs <= 0)
				delete this;
		}
		i32              m_nRefs;
		TIterator        m_cur;
		TIterator        m_end;
#ifdef USE_SORTED_ACTIONS_ITERATOR
		TSortedActionMap m_sortedActions;
#endif
	};
	TActionMap::iterator actionsBegin(m_actions.begin());
	TActionMap::iterator actionsEnd(m_actions.end());
	return new CActionInfo(actionsBegin, actionsEnd);
}

void CActionMap::Enable(bool enable)
{
	// detect nop
	if (enable == m_enabled)
		return;

	// now things get a bit interesting, when we get disabled we "release" all our
	// active actions
	if (!enable)
	{
		TActionMap::iterator it = m_actions.begin();
		TActionMap::iterator end = m_actions.end();
		for (; it != end; ++it)
		{
			ReleaseActionIfActiveInternal(it->second);
		}
	}

	// finally set the flag
	m_enabled = enable;

	if (m_pActionMapUpr)
	{
		m_pActionMapUpr->BroadcastActionMapEvent(SActionMapEvent(SActionMapEvent::eActionMapUprEvent_ActionMapStatusChanged, (UINT_PTR)this, (UINT_PTR)m_enabled));
	}
}

void CActionMap::ReleaseActionIfActive(const ActionId& actionId)
{
	TActionMap::iterator it = m_actions.find(actionId);
	if (it != m_actions.end())
	{
		ReleaseActionIfActiveInternal(it->second);
	}
}

void CActionMap::ReleaseActionIfActiveInternal(CActionMapAction& action)
{
	bool bFireOnActionRelease = false;
	bool bFireOnActionAlways = false;

	i32 numActionInputs = action.GetNumActionInputs();
	for (i32 i = 0; i < numActionInputs; ++i)
	{
		SActionInput* pActionInput = action.GetActionInput(i);
		DRX_ASSERT(pActionInput != NULL);

		bool hasReleaseMode = (pActionInput->activationMode & eAAM_OnRelease) != 0;
		bool isPressedOrDown = (pActionInput->currentState == eIS_Pressed) || (pActionInput->currentState == eIS_Down);
		bool isChanged = pActionInput->currentState == eIS_Changed;

		if (hasReleaseMode && isPressedOrDown)
		{
			bFireOnActionRelease = true;
		}

		if (isChanged)
		{
			bFireOnActionAlways = true;
		}

		pActionInput->fCurrentHoldValue = 0.f;
	}

	if (bFireOnActionRelease)
	{
		IActionListener* pEntityListener = NULL;

		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_listenerId))
		{
			if (CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER))
			{
				pEntityListener = pGameObject;
			}
		}
		IActionListener* pListener = pEntityListener;
		if (pListener)
		{
			pListener->OnAction(action.GetActionId(), eAAM_OnRelease, 0.0f);
		}

		NotifyExtraActionListeners(action.GetActionId(), eAAM_OnRelease, 0.0f);
	}

	if (bFireOnActionAlways)
	{
		IActionListener* pEntityListener = NULL;

		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(m_listenerId))
		{
			if (CGameObject* pGameObject = (CGameObject*) pEntity->GetProxy(ENTITY_PROXY_USER))
			{
				pEntityListener = pGameObject;
			}
		}
		IActionListener* pListener = pEntityListener;
		if (pListener)
		{
			pListener->OnAction(action.GetActionId(), eAAM_Always, 0.0f);
		}

		NotifyExtraActionListeners(action.GetActionId(), eAAM_Always, 0.0f);
	}
}

EActionAnalogCompareOperation CActionMap::GetAnalogCompareOpTypeFromStr(tukk szTypeStr)
{
	EActionAnalogCompareOperation compareOpType = eAACO_None;
	if (!strcmp(szTypeStr, s_analogCompareOpEqualsStr))
	{
		compareOpType = eAACO_Equals;
	}
	else if (!strcmp(szTypeStr, s_analogCompareOpNotEqualsStr))
	{
		compareOpType = eAACO_NotEquals;
	}
	else if (!strcmp(szTypeStr, s_analogCompareOpGreaterThanStr))
	{
		compareOpType = eAACO_GreaterThan;
	}
	else if (!strcmp(szTypeStr, s_analogCompareOpLessThanStr))
	{
		compareOpType = eAACO_LessThan;
	}

	return compareOpType;
}

tukk CActionMap::GetAnalogCompareOpStr(EActionAnalogCompareOperation compareOpType) const
{
	if (compareOpType == eAACO_Equals)
	{
		return s_analogCompareOpEqualsStr;
	}
	else if (compareOpType == eAACO_NotEquals)
	{
		return s_analogCompareOpNotEqualsStr;
	}
	else if (compareOpType == eAACO_GreaterThan)
	{
		return s_analogCompareOpGreaterThanStr;
	}
	else if (compareOpType == eAACO_LessThan)
	{
		return s_analogCompareOpLessThanStr;
	}
	else
	{
		return NULL;
	}
}

void CActionMap::EnumerateActions(IActionMapPopulateCallBack* pCallBack) const
{
	for (TActionMap::const_iterator actionCit = m_actions.begin(); actionCit != m_actions.end(); ++actionCit)
	{
		pCallBack->AddActionName(actionCit->first.c_str());
	}
}

void CActionMap::GetMemoryUsage(IDrxSizer* s) const
{
	s->AddObject(m_actions);
	s->AddObject(m_name);
}
