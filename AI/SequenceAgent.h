// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef SequenceAgent_h
	#define SequenceAgent_h

	#include <drx3D/AI/AIBubblesSystem.h>

namespace AIActionSequence
{

class SequenceAgent
{
public:
	SequenceAgent(EntityId entityID)
		: m_aiActor(NULL)
	{
		IEntity* entity = gEnv->pEntitySystem->GetEntity(entityID);
		if (!entity)
			return;

		IAIObject* aiObject = entity->GetAI();
		if (!aiObject)
			return;

		m_aiActor = aiObject->CastToCAIActor();
		if (!m_aiActor)
		{
			stack_string message;
			message.Format("AI SequenceAgent: agent '%s' is not of type CAIActor", entity->GetName());
			AIQueueBubbleMessage("AI SequenceAgent: agent is not of type CAIActor", entityID, message, eBNS_LogWarning | eBNS_Balloon);
		}
	}

	bool ValidateAgent() const
	{
		return m_aiActor != NULL;
	}

	void SetSequenceBehavior(bool interruptible)
	{
		if (!ValidateAgent())
			return;

		if (interruptible)
			m_aiActor->SetBehaviorVariable("ExecuteInterruptibleSequence", true);
		else
			m_aiActor->SetBehaviorVariable("ExecuteSequence", true);
	}

	bool IsRunningSequenceBehavior(bool interruptible)
	{
		if (!ValidateAgent())
			return false;

		IAIActorProxy* aiActorProxy = m_aiActor->GetProxy();
		assert(aiActorProxy);
		if (!aiActorProxy)
			return false;

		tukk currentBehavior = aiActorProxy->GetCurrentBehaviorName();
		if (!currentBehavior)
			return false;

		if (interruptible)
			return !strcmp(currentBehavior, "ExecuteInterruptibleSequence");
		else
			return !strcmp(currentBehavior, "ExecuteSequence");
	}

	void ClearSequenceBehavior() const
	{
		if (!ValidateAgent() || !m_aiActor->IsEnabled())
			return;

		m_aiActor->SetBehaviorVariable("ExecuteSequence", false);
		m_aiActor->SetBehaviorVariable("ExecuteInterruptibleSequence", false);
	}

	void ClearGoalPipe() const
	{
		if (!ValidateAgent())
			return;

		if (IPipeUser* pipeUser = m_aiActor->CastToIPipeUser())
		{
			pipeUser->SelectPipe(0, "Empty", 0, 0, true);
		}
	}

	IPipeUser* GetPipeUser()
	{
		if (!ValidateAgent())
			return NULL;

		return m_aiActor->CastToIPipeUser();
	}

	IEntity* GetEntity()
	{
		if (!ValidateAgent())
			return NULL;

		return m_aiActor->GetEntity();
	}

	EntityId GetEntityId() const
	{
		if (!ValidateAgent())
			return 0;

		return m_aiActor->GetEntityID();
	}

	void SendSignal(tukk signalName, IEntity* pSender)
	{
		if (!ValidateAgent())
			return;

		IAISignalExtraData* pData = gEnv->pAISystem->CreateSignalExtraData();
		i32k goalPipeId = gEnv->pAISystem->AllocGoalPipeId();
		pData->iValue = goalPipeId;
		m_aiActor->SetSignal(10, signalName, pSender, pData);
	}

private:
	SequenceAgent()
		: m_aiActor(NULL)
	{
	}

	CAIActor* m_aiActor;
};

} //endns AIActionSequence

#endif //SequenceAgent_h
