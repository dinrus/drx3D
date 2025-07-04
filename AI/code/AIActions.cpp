// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Entity/IEntity.h>

#include <drx3D/AI/CAISystem.h>
#include <drx3D/AI/PipeUser.h>
#include <drx3D/AI/AIActions.h>
#include <drx3D/AI/AIActor.h>
#include <drx3D/AI/AIBubblesSystem.h>

///////////////////////////////////////////////////
// CAIAction
///////////////////////////////////////////////////

bool CAIAction::TraverseAndValidateAction(EntityId idOfUser) const
{
	assert(m_pFlowGraph.get() != NULL);
	if (!m_pFlowGraph)
		return false;

	// - для всех узлов EFLN_AIACTION_START в данном flow-графе:
	// - flood the graph between that EFLN_AIACTION_START node and the terminating EFLN_AIACTION_END node
	// - проверять встреченные узлы на совместимость при пуске в контексте IAIAction
	//(читаем: узлы AISequence не должны находиться внутри к.-л. AIAction из-за какой-то
	//связанной с GoalOp ... хрени (weirdness).

	IFlowNodeIteratorPtr nodeIterator = m_pFlowGraph->CreateNodeIterator();
	TFlowNodeId nodeId;

	while (IFlowNodeData* data = nodeIterator->Next(nodeId))
	{
		SFlowNodeConfig config;
		data->GetConfiguration(config);
		if (config.GetTypeFlags() & EFLN_AIACTION_START)
		{
			std::deque<TFlowNodeId> nodesToVisit;
			std::set<TFlowNodeId> examinedNodes;

			nodesToVisit.push_back(nodeId);

			while (!nodesToVisit.empty())
			{
				nodeId = nodesToVisit.front();

				// проверка совместимости: узлы AISequence недопустимы (_not_ allowed)
				IFlowNodeData* nodeData = m_pFlowGraph->GetNodeData(nodeId);
				assert(nodeData);
				nodeData->GetConfiguration(config);
				if (config.GetTypeFlags() & EFLN_AISEQUENCE_ACTION)
				{
					stack_string message;
					message.Format("AI Action contains a flownode not supported by the AI Action system : %s", m_pFlowGraph->GetNodeTypeName(nodeId));
					AIQueueBubbleMessage("AI Action - Flownode not supported", idOfUser, message, eBNS_LogWarning | eBNS_Balloon | eBNS_BlockingPopup);
					return false;
				}

				nodesToVisit.pop_front();
				examinedNodes.insert(nodeId);

				// посетить соседей
				IFlowEdgeIteratorPtr edgeIter = m_pFlowGraph->CreateEdgeIterator();
				IFlowEdgeIterator::Edge edge;
				while (edgeIter->Next(edge))
				{
					if (edge.fromNodeId != nodeId)
						continue;

					IFlowNodeData* dataOfDestinationNode = m_pFlowGraph->GetNodeData(edge.toNodeId);
					assert(dataOfDestinationNode);
					SFlowNodeConfig configOfDestinationNode;
					dataOfDestinationNode->GetConfiguration(configOfDestinationNode);
					if (configOfDestinationNode.GetTypeFlags() & EFLN_AIACTION_END)
						continue; // "бьём" (hit) по заверщающему узлу действа

					if (examinedNodes.find(edge.toNodeId) != examinedNodes.end())
						continue; // examined this node already (can happen if being pointed to by more than one node)

					nodesToVisit.push_back(edge.toNodeId);
				}
			}
		}
	}

	return true;
}

///////////////////////////////////////////////////
// CAnimationAction
///////////////////////////////////////////////////

// Возвращает целевой пайп, который выполняет это AI Action (Действо ИИ)
IGoalPipe* CAnimationAction::GetGoalPipe() const
{
	// целевой пайп создаётся здесь
	IGoalPipe* pPipe = gAIEnv.pPipeUpr->CreateGoalPipe(string(bSignaledAnimation ? "$$" : "@") + sAnimationName, CPipeUpr::SilentlyReplaceDuplicate);

	GoalParameters params;
	IGoalPipe::EGroupType grouped(IGoalPipe::eGT_NOGROUP);

	if (bExactPositioning)
	{
		if (iApproachStance != -1)
		{
			params.fValue = (float)iApproachStance;
			pPipe->PushGoal(eGO_BODYPOS, false, grouped, params);
			params.fValue = 0;

			grouped = IGoalPipe::eGT_GROUPWITHPREV;
		}

		if (fApproachSpeed != -1)
		{
			params.fValue = fApproachSpeed;
			params.nValue = 0;
			pPipe->PushGoal(eGO_RUN, false, grouped, params);
			params.fValue = 0;
			params.nValue = 0;

			grouped = IGoalPipe::eGT_GROUPWITHPREV;
		}

		params.str = "refpoint";
		pPipe->PushGoal(eGO_LOCATE, false, grouped, params);
		params.str.clear();

		params.bValue = bSignaledAnimation;
		params.str = sAnimationName;
		params.vPos.x = fStartWidth;
		params.vPos.y = fDirectionTolerance;
		params.vPos.z = fStartArcAngle;
		params.vPosAux = vApproachPos;
		params.nValue = bAutoTarget ? 1 : 0;
		pPipe->PushGoal(eGO_ANIMTARGET, false, IGoalPipe::eGT_GROUPWITHPREV, params);
		params.bValue = false;
		params.str.clear();
		params.vPos.zero();
		params.nValue = 0;

		params.str = "animtarget";
		pPipe->PushGoal(eGO_LOCATE, false, IGoalPipe::eGT_GROUPWITHPREV, params);
		params.str.clear();

		params.fValue = 1.5f;    // end distance
		params.fValueAux = 1.0f; // end accuracy
		params.nValue = AILASTOPRES_USE | AI_STOP_ON_ANIMATION_START;
		pPipe->PushGoal(eGO_APPROACH, true, IGoalPipe::eGT_GROUPWITHPREV, params);
		params.fValue = 0;
		params.fValueAux = 0;
		params.nValue = 0;

		params.fValue = 0;
		params.nValue = BRANCH_ALWAYS;
		params.nValueAux = 2;
		pPipe->PushGoal(eGO_BRANCH, false, IGoalPipe::eGT_NOGROUP, params);
		params.nValue = 0;
		params.nValueAux = 0;

		params.nValueAux = 1;
		params.str = "CANCEL_CURRENT";
		params.nValue = 0;
		pPipe->PushGoal(eGO_SIGNAL, true, IGoalPipe::eGT_NOGROUP, params);
		params.nValueAux = 0;
		params.str.clear();
		params.nValue = 0;

		params.fValue = 0;
		params.nValue = BRANCH_ALWAYS;
		params.nValueAux = 1;
		pPipe->PushGoal(eGO_BRANCH, false, IGoalPipe::eGT_NOGROUP, params);
		params.nValue = 0;
		params.nValueAux = 0;

		params.nValueAux = 1;
		params.str = "OnAnimTargetReached";
		params.nValue = 0;
		pPipe->PushGoal(eGO_SIGNAL, true, IGoalPipe::eGT_NOGROUP, params);
		params.nValueAux = 0;
		params.str.clear();
		params.nValue = 0;

	}
	else
	{
		params.nValue = bSignaledAnimation ? AIANIM_SIGNAL : AIANIM_ACTION;
		params.str = sAnimationName;
		pPipe->PushGoal(eGO_ANIMATION, false, IGoalPipe::eGT_NOGROUP, params);
		params.bValue = false;
		params.str.clear();
	}

	return pPipe;
}

///////////////////////////////////////////////////
// CActiveAction представляет отдельное активное CAIAction (ДействиеИИ) или CAnimationAction
// (ДействиеАнимации)
///////////////////////////////////////////////////
void CActiveAction::EndAction()
{
	// закончить действие только, если оно ещё не отменено
	if (m_iThreshold >= 0 && !m_bDeleted)
	{
		m_bDeleted = true;

		// modify the states
		if (!m_nextUserState.empty())
		{
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(m_pUserEntity, m_nextUserState);
			m_nextUserState.clear();
		}
		if (m_pObjectEntity && !m_nextObjectState.empty())
		{
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(m_pObjectEntity, m_nextObjectState);
			m_nextObjectState.clear();
		}

		this->NotifyListeners(ActionEnd);
	}
}

void CActiveAction::CancelAction()
{
	// cancel action only if it isn't ended already
	if (!m_bDeleted && m_iThreshold >= 0)
	{
		m_iThreshold = -1;

		// modify the states
		if (!m_canceledUserState.empty())
		{
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(m_pUserEntity, m_canceledUserState);
			m_canceledUserState.clear();
		}
		if (m_pObjectEntity && !m_canceledObjectState.empty())
		{
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(m_pObjectEntity, m_canceledObjectState);
			m_canceledObjectState.clear();
		}
	}

	this->NotifyListeners(ActionCancel);
}

bool CActiveAction::AbortAction()
{
	// abort action only if it isn't aborted, ended or canceled already
	if (m_bDeleted || m_iThreshold < 0)
		return false;

	IAIObject* pUser = m_pUserEntity->GetAI();
	if (m_pFlowGraph)
	{
		CPipeUser* pPipeUser = pUser ? pUser->CastToCPipeUser() : NULL;
		if (pPipeUser)
		{
			// enumerate nodes and make all AI:ActionAbort nodes RegularlyUpdated
			bool bHasAbort = false;
			i32 idNode = 0;
			TFlowNodeTypeId nodeTypeId;
			TFlowNodeTypeId actionAbortTypeId = gEnv->pFlowSystem->GetTypeId("AI:ActionAbort");
			while ((nodeTypeId = m_pFlowGraph->GetNodeTypeId(idNode)) != InvalidFlowNodeTypeId)
			{
				if (nodeTypeId == actionAbortTypeId)
				{
					m_pFlowGraph->SetRegularlyUpdated(idNode, true);
					bHasAbort = true;
				}
				++idNode;
			}

			if (bHasAbort)
			{
				pPipeUser->AbortActionPipe(m_idGoalPipe);
				return true;
			}
		}
	}
	CancelAction();
	return false;
}

bool CActiveAction::operator==(const CActiveAction& other) const
{
	return
	  // (MATT) More thorough equality testing was needed. Can we get rid of this entirely? {2008/07/28}
	  m_idGoalPipe == other.m_idGoalPipe &&
	  m_pFlowGraph == other.m_pFlowGraph &&
	  m_pUserEntity == other.m_pUserEntity &&
	  m_pObjectEntity == other.m_pObjectEntity &&
	  m_SuspendCounter == other.m_SuspendCounter &&
	  m_sAnimationName == other.m_sAnimationName &&
	  m_vAnimationPos == other.m_vAnimationPos &&
	  m_vAnimationDir == other.m_vAnimationDir;
}

///////////////////////////////////////////////////
// CAIActionUpr отслеживает все AIActions
///////////////////////////////////////////////////

CAIActionUpr::CAIActionUpr()
{
}

CAIActionUpr::~CAIActionUpr()
{
	Reset();
	m_ActionsLib.clear();
}

// suspends all active AI Actions in which the entity is involved
// it's safe to pass pEntity == NULL
i32 CAIActionUpr::SuspendActionsOnEntity(IEntity* pEntity, i32 goalPipeId, const IAIAction* pAction, bool bHighPriority, i32& numHigherPriority)
{
	// count how many actions are suspended so we can prevent execution of too many actions at same time
	i32 count = 0;
	numHigherPriority = 0;

	if (pEntity)
	{
		// if entity is registered as CPipeUser...
		IAIObject* pAI = pEntity->GetAI();
		if (pAI)
		{
			CPipeUser* pPipeUser = pAI->CastToCPipeUser();
			if (pPipeUser)
			{
				GetAISystem()->Record(pAI, IAIRecordable::E_ACTIONSUSPEND, "");

				// insert the pipe
				IGoalPipe* pPipe = pAction->GetGoalPipe();
				if (pPipe)
				{
					pPipe = pPipeUser->InsertSubPipe(bHighPriority ? AIGOALPIPE_HIGHPRIORITY : 0, pPipe->GetName(), NILREF, goalPipeId);
					if (pPipe != NULL && ((CGoalPipe*)pPipe)->GetSubpipe() == NULL)
					{
						pPipeUser->SetRefPointPos(pAction->GetAnimationPos(), pAction->GetAnimationDir());
					}
				}
				else
				{
					pPipe = pPipeUser->InsertSubPipe(bHighPriority ? AIGOALPIPE_HIGHPRIORITY : 0, "_action_", NILREF, goalPipeId);
					if (!pPipe)
					{
						// create the goal pipe here
						GoalParameters params;
						pPipe = gAIEnv.pPipeUpr->CreateGoalPipe("_action_", CPipeUpr::SilentlyReplaceDuplicate);
						params.fValue = 0.1f;
						pPipe->PushGoal(eGO_TIMEOUT, true, IGoalPipe::eGT_NOGROUP, params);
						params.fValue = 0;
						params.nValue = BRANCH_ALWAYS;
						params.nValueAux = -1;
						pPipe->PushGoal(eGO_BRANCH, true, IGoalPipe::eGT_NOGROUP, params);

						// try to insert the pipe now once again
						pPipe = pPipeUser->InsertSubPipe(bHighPriority ? AIGOALPIPE_HIGHPRIORITY : 0, "_action_", NILREF, goalPipeId);
					}
				}

				// pPipe might be NULL if the puppet is dead
				if (!pPipe)
					return 100;
				numHigherPriority = ((CGoalPipe*)pPipe)->CountSubpipes();

				// set debug name
				string name("[");
				if (bHighPriority)
					name += '!';
				tukk szName = pAction->GetName();
				if (!szName)
					name += pPipe->GetName();
				else
					name += szName;
				name += ']';
				pPipe->SetDebugName(name);

				// and watch it
				pPipeUser->RegisterGoalPipeListener(this, goalPipeId, "CAIActionUpr::SuspendActionsOnEntity");
			}
			else // if ( CAIPlayer* pAIPlayer = pAI->CastToCAIPlayer() )
			{
				if (pAction->GetFlowGraph() == NULL)
				{
					pAI->GetProxy()->PlayAnimationAction(pAction, goalPipeId);
				}
			}
		}

		TActiveActions::iterator it, itEnd = m_ActiveActions.end();
		for (it = m_ActiveActions.begin(); it != itEnd; ++it)
		{
			if (it->m_pUserEntity == pEntity /*|| it->m_pObjectEntity == pEntity*/)
			{
				if (bHighPriority == true || it->m_bHighPriority == false)
				{
					if (!it->m_SuspendCounter)
					{
						IFlowGraph* pGraph = it->GetFlowGraph();
						if (pGraph)
							pGraph->SetSuspended(true);
					}
					++it->m_SuspendCounter;
				}
				++count;
			}
		}
	}

	// returns the number of active actions
	return count + 1;
}

// resumes all active AI Actions in which the entity is involved
// (resuming depends on it how many times it was suspended)
// note: it's safe to pass pEntity == NULL
void CAIActionUpr::ResumeActionsOnEntity(IEntity* pEntity, i32 goalPipeId)
{
	if (pEntity)
	{
		if (goalPipeId)
		{
			// if entity is registered as CPipeUser...
			IAIObject* pAI = pEntity->GetAI();
			if (pAI)
			{
				CPipeUser* pPipeUser = pAI->CastToCPipeUser();
				if (pPipeUser)
				{
					GetAISystem()->Record(pAI, IAIRecordable::E_ACTIONRESUME, "");

					// ...stop watching it...
					pPipeUser->UnRegisterGoalPipeListener(this, goalPipeId);

					// ...remove "_action_" goal pipe
					pPipeUser->RemoveSubPipe(goalPipeId);
				}
			}
		}

		TActiveActions::iterator it, itEnd = m_ActiveActions.end();
		for (it = m_ActiveActions.begin(); it != itEnd; ++it)
			if (it->m_pUserEntity == pEntity /*|| it->m_pObjectEntity == pEntity*/)
			{
				--it->m_SuspendCounter;
				if (!it->m_SuspendCounter)
				{
					IFlowGraph* pGraph = it->GetFlowGraph();
					if (pGraph)
						pGraph->SetSuspended(false);
				}
			}
	}
}

// aborts specific AI Action (specified by goalPipeId) or all AI Actions (goalPipeId == 0) in which pEntity is a user
void CAIActionUpr::AbortAIAction(IEntity* pEntity, i32 goalPipeId /*=0*/)
{
	AIAssert(pEntity);
	if (!pEntity)
		return;

	TActiveActions::iterator it, itEnd = m_ActiveActions.end();
	for (it = m_ActiveActions.begin(); it != itEnd; ++it)
		if ((goalPipeId || !it->m_bHighPriority) && it->m_pUserEntity == pEntity)
			if (!goalPipeId || goalPipeId == it->m_idGoalPipe)
				it->AbortAction();
}

// finishes specific AI Action (specified by goalPipeId) for the pEntity as a user
void CAIActionUpr::FinishAIAction(IEntity* pEntity, i32 goalPipeId)
{
	AIAssert(pEntity);
	if (!pEntity)
		return;

	TActiveActions::iterator it, itEnd = m_ActiveActions.end();
	for (it = m_ActiveActions.begin(); it != itEnd; ++it)
	{
		if (it->m_pUserEntity == pEntity && goalPipeId == it->m_idGoalPipe)
		{
			it->EndAction();
			break;
		}
	}
}

// implementation of IGoalPipeListener
void CAIActionUpr::OnGoalPipeEvent(IPipeUser* pPipeUser, EGoalPipeEvent event, i32 goalPipeId, bool& unregisterListenerAfterEvent)
{
	TActiveActions::iterator it, itEnd = m_ActiveActions.end();
	for (it = m_ActiveActions.begin(); it != itEnd; ++it)
	{
		CActiveAction& action = *it;
		if (action.m_idGoalPipe == goalPipeId)
		{
			if (event == ePN_Deselected)
			{
				action.CancelAction();
				break;
			}
			if (!action.GetFlowGraph())
			{
				// additional stuff for animations actions
				if (event == ePN_Resumed)
				{
					// restore reference point
					pPipeUser->SetRefPointPos(action.GetAnimationPos(), action.GetAnimationDir());
					break;
				}
				else if (event == ePN_Finished)
				{
					action.EndAction();
					break;
				}
			}
		}
	}
}

// stops all active actions
void CAIActionUpr::Reset()
{
	TActiveActions::iterator it, itEnd = m_ActiveActions.end();
	for (it = m_ActiveActions.begin(); it != itEnd; ++it)
	{
		CActiveAction& action = *it;
		if (action.GetFlowGraph())
			action.GetFlowGraph()->SetAIAction(0);
		if (action.GetUserEntity())
		{
			IAIObject* pAI = action.GetUserEntity()->GetAI();
			if (pAI)
			{
				CPipeUser* pPipeUser = pAI->CastToCPipeUser();
				if (pPipeUser)
					pPipeUser->UnRegisterGoalPipeListener(this, action.m_idGoalPipe);
			}
		}
	}
	m_ActiveActions.clear();
}

// returns an existing AI Action by its name specified in the rule
// or creates a new temp. action for playing the animation specified in the rule
IAIAction* CAIActionUpr::GetAIAction(const CCondition* pRule)
{
	IAIAction* pResult = NULL;
	if (pRule->sAction.empty())
		return NULL;

	if (pRule->eActionType == eAT_AnimationSignal || pRule->eActionType == eAT_AnimationAction
	    || pRule->eActionType == eAT_PriorityAnimationSignal || pRule->eActionType == eAT_PriorityAnimationAction)
	{
		if (pRule->sAnimationHelper.empty())
		{
			pResult = new CAnimationAction(pRule->sAction, pRule->eActionType == eAT_AnimationSignal || pRule->eActionType == eAT_PriorityAnimationSignal);
		}
		else
		{
			pResult = new CAnimationAction(
			  pRule->sAction,
			  pRule->eActionType == eAT_AnimationSignal || pRule->eActionType == eAT_PriorityAnimationSignal,
			  pRule->fApproachSpeed,
			  pRule->iApproachStance,
			  pRule->fStartWidth,
			  pRule->fDirectionTolerance,
			  pRule->fStartArcAngle);
		}
	}
	else if (pRule->eActionType == eAT_Action || pRule->eActionType == eAT_PriorityAction)
		pResult = GetAIAction(pRule->sAction);

	return pResult;
}

// returns an existing AI Action from the library or NULL if not found
IAIAction* CAIActionUpr::GetAIAction(tukk sName)
{
	if (!sName || !*sName)
		return NULL;

	TActionsLib::iterator it = m_ActionsLib.find(CONST_TEMP_STRING(sName));
	if (it != m_ActionsLib.end())
		return &it->second;
	else
		return NULL;
}

// returns an existing AI Action by its index in the library or NULL index is out of range
IAIAction* CAIActionUpr::GetAIAction(size_t index)
{
	if (index >= m_ActionsLib.size())
		return NULL;

	TActionsLib::iterator it = m_ActionsLib.begin();
	while (index--)
		++it;
	return &it->second;
}

// adds an AI Action in the list of active actions
void CAIActionUpr::ExecuteAIAction(const IAIAction* pAction, IEntity* pUser, IEntity* pObject, i32 maxAlertness, i32 goalPipeId,
                                       tukk userState, tukk objectState, tukk userCanceledState, tukk objectCanceledState, IAIAction::IAIActionListener* pListener)
{
	AIAssert(pAction);
	AIAssert(pUser);
	AIAssert(pObject);

	pAction->TraverseAndValidateAction(pUser->GetId());

	IAIObject* pAI = pUser->GetAI();

	if (GetAISystem()->IsRecording(pAI, IAIRecordable::E_ACTIONSTART))
	{
		string str(pAction->GetName());
		str += " on entity ";
		str += pObject->GetName();
		GetAISystem()->Record(pAI, IAIRecordable::E_ACTIONSTART, str);
	}

	// don't execute the action if the user is dead
	const bool isAIDisabled = pAI ? !pAI->IsEnabled() : false;
	if (isAIDisabled)
	{
		if (!pAction->GetFlowGraph())
			delete pAction;

		return;
	}

	// allocate an goal pipe id if not specified
	if (!goalPipeId)
		goalPipeId = GetAISystem()->AllocGoalPipeId();

	// create a clone of the flow graph
	IFlowGraphPtr pFlowGraph = NULL;
	if (IFlowGraph* pActionFlowGraph = pAction->GetFlowGraph())
		pFlowGraph = pActionFlowGraph->Clone();

	// suspend all actions executing on the User or the Object
	i32 suspendCounter = 0;
	i32 numActions = SuspendActionsOnEntity(pUser, goalPipeId, pAction, maxAlertness >= 100, suspendCounter);
	if (numActions == 100)
	{
		// can't select the goal pipe
		if (!pAction->GetFlowGraph())
			delete pAction;
		return;
	}

	// Tell entity about action start
	GetAISystem()->SendSignal(SIGNALFILTER_SENDER, 1, "OnActionStart", pAI);

	if (pFlowGraph)
	{
		// create active action and add it to the list
		CActiveAction action;
		action.m_pFlowGraph = pFlowGraph;
		action.m_Name = pAction->GetName();
		action.m_pUserEntity = pUser;
		action.m_pObjectEntity = pObject;
		action.m_SuspendCounter = suspendCounter;
		action.m_idGoalPipe = goalPipeId;
		action.m_iThreshold = numActions > 10 ? -1 : maxAlertness % 100; // cancel this action if there are already 10 actions executing
		action.m_nextUserState = userState;
		action.m_nextObjectState = objectState;
		action.m_canceledUserState = userCanceledState;
		action.m_canceledObjectState = objectCanceledState;
		action.m_bDeleted = false;
		action.m_bHighPriority = maxAlertness >= 100;
		m_ActiveActions.push_front(action);

		if (pListener)
		{
			m_ActiveActions.front().RegisterListener(pListener, "ListenerFlowGraphAction");
			pListener->OnActionEvent(IAIAction::ActionStart);
		}

		// the User will be first graph entity.
		if (pUser)
			pFlowGraph->SetGraphEntity(pUser->GetId(), 0);
		// the Object will be second graph entity.
		if (pObject)
			pFlowGraph->SetGraphEntity(pObject->GetId(), 1);

		// initialize the graph
		pFlowGraph->InitializeValues();

		pFlowGraph->SetAIAction(&m_ActiveActions.front());

		if (action.m_SuspendCounter > 0)
			pFlowGraph->SetSuspended(true);
	}
	else
	{
		// create active action and add it to the list
		CActiveAction action;
		action.m_pFlowGraph = NULL;
		action.m_Name = pAction->GetName();
		action.m_pUserEntity = pUser;
		action.m_pObjectEntity = pObject;
		action.m_SuspendCounter = suspendCounter;
		action.m_idGoalPipe = goalPipeId;
		action.m_iThreshold = numActions > 10 ? -1 : maxAlertness % 100; // cancel this action if there are already 10 actions executing
		action.m_nextUserState = userState;
		action.m_nextObjectState = objectState;
		action.m_canceledUserState = userCanceledState;
		action.m_canceledObjectState = objectCanceledState;
		action.m_bDeleted = false;
		action.m_bSignaledAnimation = pAction->IsSignaledAnimation();
		action.m_bExactPositioning = pAction->IsExactPositioning();
		action.m_sAnimationName = pAction->GetAnimationName();
		action.m_bHighPriority = maxAlertness >= 100;
		action.m_vAnimationPos = pAction->GetAnimationPos();
		action.m_vAnimationDir = pAction->GetAnimationDir();
		action.m_vApproachPos = pAction->GetApproachPos();
		action.m_bAutoTarget = pAction->IsUsingAutoAssetAlignment();
		action.m_fStartWidth = pAction->GetStartWidth();
		action.m_fStartArcAngle = pAction->GetStartArcAngle();
		action.m_fDirectionTolerance = pAction->GetDirectionTolerance();
		m_ActiveActions.push_front(action);

		if (pListener)
		{
			m_ActiveActions.front().RegisterListener(pListener, "ListenerAction");
			pListener->OnActionEvent(IAIAction::ActionStart);
		}

		// delete temp. action
		delete pAction;
	}
}

void CAIActionUpr::ExecuteAIAction(tukk sActionName, IEntity* pUser, IEntity* pObject, i32 maxAlertness, i32 goalPipeId, IAIAction::IAIActionListener* pListener)
{
	string str(sActionName);
	str += ",\"\",\"\",\"\",\"\",";
	i32 i = str.find(',', 0);
	i32 j = str.find('\"', i + 2);
	i32 k = str.find('\"', j + 3);
	i32 l = str.find('\"', k + 3);
	i32 m = str.find('\"', l + 3);
	IAIAction* pAction = GetAIAction(str.substr(0, i));
	if (pAction)
	{
		ExecuteAIAction(pAction, pUser, pObject, maxAlertness, goalPipeId,
		                str.substr(i + 2, j - i - 2), str.substr(j + 3, k - j - 3), str.substr(k + 3, l - k - 3), str.substr(l + 3, m - l - 3), pListener);
	}
	else
	{
		AIError("Entity '%s' requested execution of undefined AI Action '%s' [Design bug]",
		        pUser ? pUser->GetName() : "<NULL>", sActionName);
	}
}
// removes deleted AI Action from the list of active actions
void CAIActionUpr::Update()
{
	DRX_PROFILE_FUNCTION(PROFILE_AI)

	TActiveActions::iterator it = m_ActiveActions.begin();
	while (it != m_ActiveActions.end())
	{
		CActiveAction& action = *it;
		if (!action.m_pUserEntity)
		{
			// the action is already deleted just remove it from the list
			m_ActiveActions.erase(it++);
			continue;
		}
		++it;

		if (action.m_bDeleted)
		{
			AILogComment("AIAction '%s' with '%s' and '%s' ended.\n", action.GetName(), action.m_pUserEntity->GetName(), action.m_pObjectEntity->GetName());
			ActionDone(action);
		}
		else
		{
			IAIObject* pAI = action.m_pUserEntity->GetAI();
			if (pAI)
			{
				CAIActor* pActor = pAI->CastToCAIActor();
				IAIActorProxy* pProxy = pActor ? pActor->GetProxy() : NULL;
				i32 alertness = pProxy ? pProxy->GetAlertnessState() : 0;
				if (alertness > action.m_iThreshold)
				{
					if (action.m_iThreshold < 0)
					{
						AILogComment("AIAction '%s' with '%s' and '%s' cancelled\n", action.GetName(), action.m_pUserEntity->GetName(), action.m_pObjectEntity->GetName());
						ActionDone(action);
					}
					else
					{
						AILogComment("AIAction '%s' with '%s' and '%s' aborted\n", action.GetName(), action.m_pUserEntity->GetName(), action.m_pObjectEntity->GetName());
						action.AbortAction();
					}
				}
			}
			else if (action.m_iThreshold < 0)
			{
				// action was canceled
				AILogComment("AIAction '%s' with '%s' and '%s' cancelled\n", action.GetName(), action.m_pUserEntity->GetName(), action.m_pObjectEntity->GetName());
				ActionDone(action);
			}
		}
	}
}

// marks AI Action from the list of active actions as deleted
void CAIActionUpr::ActionDone(CActiveAction& action, bool bRemoveAction /*= true*/)
{
	IAIObject* pAI = action.m_pUserEntity->GetAI();

	if (GetAISystem()->IsRecording(pAI, IAIRecordable::E_ACTIONEND))
	{
		string str(action.GetName());
		str += " on entity ";
		str += action.m_pObjectEntity->GetName();
		GetAISystem()->Record(pAI, IAIRecordable::E_ACTIONEND, str);
	}

	// remove the pointer to this action in the flow graph
	IFlowGraph* pFlowGraph = action.GetFlowGraph();
	if (pFlowGraph)
		pFlowGraph->SetAIAction(NULL);

	// make a copy before removing
	CActiveAction copy = action;

	// find the action in the list of active actions
	if (bRemoveAction)
		m_ActiveActions.remove(copy);

	// resume last suspended action executing on the User or the Object
	ResumeActionsOnEntity(copy.m_pUserEntity, copy.m_idGoalPipe);

	// notify the current behavior script that action was finished

	if (pAI)
	{
		CAIActor* pAIActor = pAI->CastToCAIActor();
		if (pAIActor)
		{
			IAISignalExtraData* pData = GetAISystem()->CreateSignalExtraData();
			pData->SetObjectName(copy.GetName());
			pData->nID = copy.m_pObjectEntity->GetId();
			pData->iValue = copy.m_bDeleted ? 1 : 0; // if m_bDeleted is true it means the action was succeeded
			pAIActor->SetSignal(10, "OnActionDone", NULL, pData, gAIEnv.SignalCRCs.m_nOnActionDone);
			if (CPipeUser* pPipeUser = pAIActor->CastToCPipeUser())
			{
				pPipeUser->SetLastActionStatus(copy.m_bDeleted);   // if m_bDeleted is true it means the action was succeeded
			}
		}
	}

	// if it was deleted it also means that it was succesfully finished
	if (copy.m_bDeleted)
	{
		if (!copy.m_nextUserState.empty())
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(copy.m_pUserEntity, copy.m_nextUserState);
		if (copy.m_pObjectEntity && !copy.m_nextObjectState.empty())
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(copy.m_pObjectEntity, copy.m_nextObjectState);
	}
	else
	{
		if (!copy.m_canceledUserState.empty())
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(copy.m_pUserEntity, copy.m_canceledUserState);
		if (copy.m_pObjectEntity && !copy.m_canceledObjectState.empty())
			gAIEnv.pSmartObjectUpr->ModifySmartObjectStates(copy.m_pObjectEntity, copy.m_canceledObjectState);
	}

	//	if ( copy.m_pUserEntity != copy.m_pObjectEntity )
	//		ResumeActionsOnEntity( copy.m_pObjectEntity );

	// Tell entity about action end
	GetAISystem()->SendSignal(SIGNALFILTER_SENDER, 1, "OnActionEnd", pAI);
}

// loads the library of AI Action Flow Graphs
void CAIActionUpr::LoadLibrary(tukk sPath)
{
	m_ActiveActions.clear();

	// don't delete all actions - only those which are added or modified will be reloaded
	//m_ActionsLib.clear();

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
					CAIAction& action = m_ActionsLib[filename];   // this creates a new entry in m_ActionsLib
					if (!action.m_pFlowGraph)
					{
						action.m_Name = filename;
						action.m_pFlowGraph = gEnv->pFlowSystem->CreateFlowGraph();
						action.m_pFlowGraph->SetSuspended(true);
						action.m_pFlowGraph->SetAIAction(&action);
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

// notification sent by smart objects system
void CAIActionUpr::OnEntityRemove(IEntity* pEntity)
{
	//	bool bFound = false;
	TActiveActions::iterator it, itEnd = m_ActiveActions.end();
	for (it = m_ActiveActions.begin(); it != itEnd; ++it)
	{
		CActiveAction& action = *it;

		if (pEntity == action.GetUserEntity() || pEntity == action.GetObjectEntity())
		{
			action.CancelAction();
			//			bFound = true;

			AILogComment("AIAction '%s' with '%s' and '%s' canceled because '%s' was deleted.\n", action.GetName(),
			             action.m_pUserEntity->GetName(), action.m_pObjectEntity->GetName(), pEntity->GetName());
			ActionDone(action, false);
			action.OnEntityRemove();
		}
	}

	// calling Update() from here causes a crash if the entity gets deleted while the action flow graph is being updated
	//	if ( bFound )
	//	{
	//		// Update() will remove canceled actions
	//		Update();
	//	}
}

void CAIActionUpr::Serialize(TSerialize ser)
{
	if (ser.BeginOptionalGroup("AIActionUpr", true))
	{
		i32 numActiveActions = m_ActiveActions.size();
		ser.Value("numActiveActions", numActiveActions);
		if (ser.IsReading())
		{
			Reset();
			while (numActiveActions--)
				m_ActiveActions.push_back(CActiveAction());
		}

		TActiveActions::iterator it, itEnd = m_ActiveActions.end();
		for (it = m_ActiveActions.begin(); it != itEnd; ++it)
			it->Serialize(ser);
		ser.EndGroup(); //AIActionUpr
	}
}

void CAIActionUpr::ReloadActions()
{
	CTimeValue t1 = gEnv->pTimer->GetAsyncTime();
	AIAssert(gEnv->pEntitySystem);
	if (gAIEnv.pSmartObjectUpr)
	{
		this->LoadLibrary(AI_ACTIONS_PATH);
	}

	CTimeValue t2 = gEnv->pTimer->GetAsyncTime();
	AILogComment("All AI Actions reloaded in %g mSec.", (t2 - t1).GetMilliSeconds());
}

void CActiveAction::Serialize(TSerialize ser)
{
	ser.BeginGroup("ActiveAction");
	{
		ser.Value("m_Name", m_Name);

		EntityId userId = m_pUserEntity && !ser.IsReading() ? m_pUserEntity->GetId() : 0;
		ser.Value("userId", userId);
		if (ser.IsReading())
			m_pUserEntity = gEnv->pEntitySystem->GetEntity(userId);

		EntityId objectId = m_pObjectEntity && !ser.IsReading() ? m_pObjectEntity->GetId() : 0;
		ser.Value("objectId", objectId);
		if (ser.IsReading())
			m_pObjectEntity = gEnv->pEntitySystem->GetEntity(objectId);

		ser.Value("m_SuspendCounter", m_SuspendCounter);
		ser.Value("m_idGoalPipe", m_idGoalPipe);
		if (m_idGoalPipe && ser.IsReading())
		{
			IAIObject* pAI = m_pUserEntity ? m_pUserEntity->GetAI() : NULL;
			AIAssert(pAI);
			if (pAI)
			{
				CPipeUser* pPipeUser = pAI->CastToCPipeUser();
				if (pPipeUser)
				{
					pPipeUser->RegisterGoalPipeListener(gAIEnv.pAIActionUpr, m_idGoalPipe, "CActiveAction::Serialize");
					CGoalPipe* pPipe = pPipeUser->GetCurrentGoalPipe();
					while (pPipe)
					{
						if (pPipe->GetEventId() == m_idGoalPipe)
						{
							pPipe->SetDebugName((m_bHighPriority ? "[" : "[!") + m_Name + ']');
							break;
						}
						pPipe = pPipe->GetSubpipe();
					}
				}
			}
		}
		ser.Value("m_iThreshold", m_iThreshold);
		ser.Value("m_nextUserState", m_nextUserState);
		ser.Value("m_nextObjectState", m_nextObjectState);
		ser.Value("m_canceledUserState", m_canceledUserState);
		ser.Value("m_canceledObjectState", m_canceledObjectState);
		ser.Value("m_bDeleted", m_bDeleted);
		ser.Value("m_bHighPriority", m_bHighPriority);
		ser.Value("m_bSignaledAnimation", m_bSignaledAnimation);
		ser.Value("m_bExactPositioning", m_bExactPositioning);
		ser.Value("m_sAnimationName", m_sAnimationName);
		ser.Value("m_vAnimationPos", m_vAnimationPos);
		ser.Value("m_vAnimationDir", m_vAnimationDir);
		ser.Value("m_vApproachPos", m_vApproachPos);
		ser.Value("m_bAutoTarget", m_bAutoTarget);
		ser.Value("m_fStartWidth", m_fStartWidth);
		ser.Value("m_fStartArcAngle", m_fStartArcAngle);
		ser.Value("m_fDirectionTolerance", m_fDirectionTolerance);

		if (ser.BeginOptionalGroup("m_pFlowGraph", m_pFlowGraph != NULL))
		{
			if (ser.IsReading())
			{
				IAIAction* pAction = gAIEnv.pAIActionUpr->GetAIAction(m_Name);
				AIAssert(pAction);
				if (pAction)
					m_pFlowGraph = pAction->GetFlowGraph()->Clone();
				if (m_pFlowGraph)
					m_pFlowGraph->SetAIAction(this);
			}
			if (m_pFlowGraph)
			{
				m_pFlowGraph->SetGraphEntity(userId, 0);
				m_pFlowGraph->SetGraphEntity(objectId, 1);
				m_pFlowGraph->Serialize(ser);
			}
			ser.EndGroup(); //m_pFlowGraph
		}
	}
	ser.EndGroup();
}
