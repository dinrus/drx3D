// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Entity/IEntity.h>
#include <drx3D/Entity/IEntitySystem.h>

#include <drx3D/Act/CustomActionUpr.h>

///////////////////////////////////////////////////
// CCustomActionUpr keeps track of all CustomActions
///////////////////////////////////////////////////

CCustomActionUpr::CCustomActionUpr()
{
}

CCustomActionUpr::~CCustomActionUpr()
{
	ClearActiveActions();
	ClearLibraryActions();
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomActionUpr::StartAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction != NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::StartAction: Action already running on entity, can't manually start");
		return false;
	}

	pActiveCustomAction = AddActiveCustomAction(pObject, szCustomActionGraphName, pListener);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::StartAction: Failed to add action");
		return false;
	}

	pActiveCustomAction->StartAction();

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomActionUpr::SucceedAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction != NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::SucceedAction: Action already running on entity, can't manually succeed");
		return false;
	}

	pActiveCustomAction = AddActiveCustomAction(pObject, szCustomActionGraphName, pListener);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::SucceedAction: Failed to add action");
		return false;
	}

	pActiveCustomAction->SucceedAction();

	return true;
}

bool CCustomActionUpr::SucceedWaitAction(IEntity* pObject)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::SucceedWaitAction: Failed find active action");
		return false;
	}

	return pActiveCustomAction->SucceedWaitAction();
}

bool CCustomActionUpr::SucceedWaitCompleteAction(IEntity* pObject)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::SucceedWaitCompleteAction: Failed find active action");
		return false;
	}

	return pActiveCustomAction->SucceedWaitCompleteAction();
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomActionUpr::AbortAction(IEntity* pObject)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::AbortAction: Failed find active action");
		return false;
	}

	pActiveCustomAction->AbortAction();

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomActionUpr::EndAction(IEntity* pObject, bool bSuccess)
{
	DRX_ASSERT(pObject);
	if (!pObject)
		return false;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	if (pActiveCustomAction == NULL)
	{
		DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::EndAction: Failed find active action");
		return false;
	}

	if (bSuccess)
	{
		pActiveCustomAction->EndActionSuccess();
	}
	else
	{
		pActiveCustomAction->EndActionFailure();
	}

	return true;
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::LoadLibraryActions(tukk sPath)
{
	m_activeActions.clear();

	// don't delete all actions - only those which are added or modified will be reloaded
	//m_actionsLib.clear();

	string path(sPath);
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	_finddata_t fd;
	string filename;

	path.TrimRight("/\\");
	string search = path + "/*.xml";
	intptr_t handle = pDrxPak->FindFirst(search.c_str(), &fd);
	if (handle != -1)
	{
		do
		{
			filename = path;
			filename += "/";
			filename += fd.name;

			XmlNodeRef root = GetISystem()->LoadXmlFromFile(filename);
			if (root)
			{
				if (gEnv->pFlowSystem)
				{
					filename = PathUtil::GetFileName(filename);
					CCustomAction& action = m_actionsLib[filename];   // this creates a new entry in m_actionsLib
					if (!action.m_pFlowGraph)
					{
						action.m_customActionGraphName = filename;
						action.m_pFlowGraph = gEnv->pFlowSystem->CreateFlowGraph();
						action.m_pFlowGraph->SetSuspended(true);
						action.m_pFlowGraph->SetCustomAction(&action);
						action.m_pFlowGraph->SerializeXML(root, true);
					}
				}
			}
		}
		while (pDrxPak->FindNext(handle, &fd) >= 0);

		pDrxPak->FindClose(handle);
	}
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::ClearActiveActions()
{
	TActiveActions::iterator it, itEnd = m_activeActions.end();
	for (it = m_activeActions.begin(); it != itEnd; ++it)
	{
		CCustomAction& action = *it;
		action.TerminateAction();
	}
	m_activeActions.clear();
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::ClearLibraryActions()
{
	m_actionsLib.clear();
}

//------------------------------------------------------------------------------------------------------------------------
ICustomAction* CCustomActionUpr::GetCustomActionFromLibrary(tukk szCustomActionGraphName)
{
	if (!szCustomActionGraphName || !*szCustomActionGraphName)
		return NULL;

	TCustomActionsLib::iterator it = m_actionsLib.find(CONST_TEMP_STRING(szCustomActionGraphName));
	if (it != m_actionsLib.end())
		return &it->second;
	else
		return NULL;
}

//------------------------------------------------------------------------------------------------------------------------
ICustomAction* CCustomActionUpr::GetCustomActionFromLibrary(const size_t index)
{
	if (index >= m_actionsLib.size())
		return NULL;

	TCustomActionsLib::iterator it = m_actionsLib.begin();
	for (size_t i = 0; i < index; i++)
	{
		++it;
	}

	return &it->second;
}

//------------------------------------------------------------------------------------------------------------------------
size_t CCustomActionUpr::GetNumberOfActiveCustomActions() const
{
	size_t numActiveCustomActions = 0;

	TActiveActions::const_iterator it = m_activeActions.begin();
	const TActiveActions::const_iterator itEnd = m_activeActions.end();
	for (; it != itEnd; ++it)
	{
		const CCustomAction& action = *it;
		if (!(action.GetCurrentState() == CAS_Ended))   // Ignore inactive actions waiting to be removed
		{
			numActiveCustomActions++;
		}
	}

	return numActiveCustomActions;
}

//------------------------------------------------------------------------------------------------------------------------
ICustomAction* CCustomActionUpr::GetActiveCustomAction(const IEntity* pObject)
{
	DRX_ASSERT(pObject != NULL);
	if (!pObject)
		return NULL;

	TActiveActions::iterator it = m_activeActions.begin();
	const TActiveActions::const_iterator itEnd = m_activeActions.end();
	for (; it != itEnd; ++it)
	{
		CCustomAction& action = *it;
		if ((!(action.GetCurrentState() == CAS_Ended)) &&  // Ignore inactive actions waiting to be removed
		    (action.m_pObjectEntity == pObject))
		{
			break;
		}
	}

	if (it != m_activeActions.end())
		return &(*it);
	else
		return NULL;
}

//------------------------------------------------------------------------------------------------------------------------
ICustomAction* CCustomActionUpr::GetActiveCustomAction(const size_t index)
{
	if (index >= m_activeActions.size())
		return NULL;

	TActiveActions::iterator it = m_activeActions.begin();
	const TActiveActions::const_iterator itEnd = m_activeActions.end();
	size_t curActiveIndex = -1;
	CCustomAction* pFoundAction = NULL;

	for (; it != itEnd; ++it)
	{
		CCustomAction& customAction = *it;
		if (!(customAction.GetCurrentState() == CAS_Ended))   // Ignore marked for deletion but awaiting cleanup
		{
			curActiveIndex++;

			if (curActiveIndex == index)
			{
				pFoundAction = &customAction;
				break;
			}
		}
	}

	return pFoundAction;
}

//------------------------------------------------------------------------------------------------------------------------
bool CCustomActionUpr::UnregisterListener(ICustomActionListener* pEventListener)
{
	TActiveActions::iterator it = m_activeActions.begin();
	const TActiveActions::const_iterator itEnd = m_activeActions.end();
	bool bSuccessfullyUnregistered = false;
	for (; it != itEnd; ++it)
	{
		CCustomAction& customAction = *it;
		if (!(customAction.GetCurrentState() == CAS_Ended))   // Ignore marked for deletion but awaiting cleanup
		{
			if (customAction.m_listeners.Contains(pEventListener) == true)
			{
				bSuccessfullyUnregistered = true;
				customAction.UnregisterListener(pEventListener);
			}
		}
	}

	return bSuccessfullyUnregistered;
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::Update()
{
	DRX_PROFILE_FUNCTION(PROFILE_ACTION)

	TActiveActions::iterator it = m_activeActions.begin();
	while (it != m_activeActions.end())
	{
		CCustomAction& action = *it;
		if (!action.m_pObjectEntity)
		{
			// The action was terminated, just remove it from the list
			m_activeActions.erase(it++);
			continue;
		}
		++it;

		if (action.GetCurrentState() == CAS_Ended)
		{
			// Remove the pointer to this action in the flow graph
			IFlowGraph* pFlowGraph = action.GetFlowGraph();
			if (pFlowGraph && pFlowGraph->GetCustomAction() != NULL)   // Might be null if terminated
				pFlowGraph->SetCustomAction(NULL);

			// Remove in the actual list
			m_activeActions.remove(action);
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------
ICustomAction* CCustomActionUpr::AddActiveCustomAction(IEntity* pObject, tukk szCustomActionGraphName, ICustomActionListener* pListener)
{
	DRX_ASSERT(pObject != NULL);
	if (!pObject)
		return NULL;

	ICustomAction* pActiveCustomAction = GetActiveCustomAction(pObject);
	DRX_ASSERT(pActiveCustomAction == NULL);
	if (pActiveCustomAction != NULL)
		return NULL;

	// Instance custom actions don't need to have a custom action graph
	IFlowGraphPtr pFlowGraph = NULL;
	if (szCustomActionGraphName != NULL && szCustomActionGraphName[0] != 0)
	{
		ICustomAction* pCustomActionFromLibrary = GetCustomActionFromLibrary(szCustomActionGraphName);
		if (pCustomActionFromLibrary == NULL)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::AddActiveCustomAction: Failed find action: %s in library", szCustomActionGraphName);
			return NULL;
		}
		// Create a clone of the flow graph

		if (IFlowGraph* pLibraryActionFlowGraph = pCustomActionFromLibrary->GetFlowGraph())
			pFlowGraph = pLibraryActionFlowGraph->Clone();

		if (!pFlowGraph)
		{
			DrxWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, "CCustomActionUpr::AddActiveCustomAction: No flow graph in action: %s in library", szCustomActionGraphName);
			return NULL;
		}
	}

	// create active action and add it to the list
	m_activeActions.push_front(CCustomAction());

	CCustomAction& addedAction = m_activeActions.front();

	addedAction.m_pFlowGraph = pFlowGraph;
	addedAction.m_customActionGraphName = szCustomActionGraphName;
	addedAction.m_pObjectEntity = pObject;

	if (pListener)
	{
		addedAction.RegisterListener(pListener, "ListenerFlowGraphCustomAction");
	}

	if (pFlowGraph)
	{
		// The Object will be first graph entity.
		pFlowGraph->SetGraphEntity(pObject->GetId(), 0);

		// Initialize the graph
		pFlowGraph->InitializeValues();

		pFlowGraph->SetCustomAction(&addedAction);

		pFlowGraph->SetSuspended(false);
	}

	return &addedAction;
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::OnEntityRemove(IEntity* pEntity)
{
	// Can't remove action here from list since will crash if the entity gets deleted while the action flow graph is being updated

	TActiveActions::iterator it, itEnd = m_activeActions.end();
	for (it = m_activeActions.begin(); it != itEnd; ++it)
	{
		CCustomAction& action = *it;

		if (pEntity == action.GetObjectEntity())
		{
			action.TerminateAction();
		}
	}
}

//------------------------------------------------------------------------------------------------------------------------
void CCustomActionUpr::Serialize(TSerialize ser)
{
	if (ser.BeginOptionalGroup("CustomActionUpr", true))
	{
		i32 numActiveActions = m_activeActions.size();
		ser.Value("numActiveActions", numActiveActions);
		if (ser.IsReading())
		{
			ClearActiveActions();
			while (numActiveActions--)
				m_activeActions.push_back(CCustomAction());
		}

		TActiveActions::iterator it, itEnd = m_activeActions.end();
		for (it = m_activeActions.begin(); it != itEnd; ++it)
			it->Serialize(ser);
		ser.EndGroup();
	}
}
