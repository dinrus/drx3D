// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/SequenceUpr.h>
#include <drx3D/AI/SequenceAgent.h>
#include <drx3D/AI/SequenceFlowNodes.h>
#include <drx3D/AI/AIBubblesSystem.h>

namespace AIActionSequence
{

void SequenceUpr::Reset()
{
	m_registeredSequences.clear();
	m_sequenceIdCounter = 1;
}

bool SequenceUpr::RegisterSequence(EntityId entityId, TFlowNodeId startNodeId, SequenceProperties sequenceProperties, IFlowGraph* flowGraph)
{
	SequenceId newSequenceId = GenerateUniqueSequenceId();
	Sequence sequence(entityId, newSequenceId, startNodeId, sequenceProperties, flowGraph);

	SequenceAgent agent(entityId);
	if (!agent.ValidateAgent())
		return false;

	if (!sequence.TraverseAndValidateSequence())
		return false;

	m_registeredSequences.insert(std::pair<SequenceId, Sequence>(newSequenceId, sequence));
	return true;
}

void SequenceUpr::UnregisterSequence(SequenceId sequenceId)
{
	m_registeredSequences.erase(sequenceId);
}

void SequenceUpr::StartSequence(SequenceId sequenceId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
	{
		DRX_ASSERT_MESSAGE(false, "Could not access sequence.");
		return;
	}

	CancelActiveSequencesForThisEntity(sequence->GetEntityId());
	sequence->Start();
}

void SequenceUpr::CancelSequence(SequenceId sequenceId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
	{
		DRX_ASSERT_MESSAGE(false, "Could not access sequence.");
		return;
	}

	if (sequence->IsActive())
	{
		sequence->Cancel();
	}
}

bool SequenceUpr::IsSequenceActive(SequenceId sequenceId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
		return false;

	return sequence->IsActive();
}

void SequenceUpr::SequenceBehaviorReady(EntityId entityId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.begin();
	SequenceMap::iterator sequenceIteratorEnd = m_registeredSequences.end();
	for (; sequenceIterator != sequenceIteratorEnd; ++sequenceIterator)
	{
		Sequence& sequence = sequenceIterator->second;
		if (sequence.IsActive() && sequence.GetEntityId() == entityId)
		{
			sequence.SequenceBehaviorReady();
			return;
		}
	}

	DRX_ASSERT_MESSAGE(false, "Entity not registered with any sequence.");
}

void SequenceUpr::SequenceInterruptibleBehaviorLeft(EntityId entityId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.begin();
	SequenceMap::iterator sequenceIteratorEnd = m_registeredSequences.end();
	for (; sequenceIterator != sequenceIteratorEnd; ++sequenceIterator)
	{
		Sequence& sequence = sequenceIterator->second;
		if (sequence.IsActive() && sequence.IsInterruptible() && sequence.GetEntityId() == entityId)
		{
			sequence.SequenceInterruptibleBehaviorLeft();
		}
	}
}

void SequenceUpr::SequenceNonInterruptibleBehaviorLeft(EntityId entityId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.begin();
	SequenceMap::iterator sequenceIteratorEnd = m_registeredSequences.end();
	for (; sequenceIterator != sequenceIteratorEnd; ++sequenceIterator)
	{
		Sequence& sequence = sequenceIterator->second;
		if (sequence.IsActive() && !sequence.IsInterruptible() && sequence.GetEntityId() == entityId)
		{
			AIQueueBubbleMessage("AI Sequence Error", entityId, "The sequence behavior has unexpectedly been deselected and the sequence has been canceled.", eBNS_LogWarning | eBNS_Balloon);
			sequence.Cancel();
		}
	}
}

void SequenceUpr::AgentDisabled(EntityId entityId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.begin();
	SequenceMap::iterator sequenceIteratorEnd = m_registeredSequences.end();
	for (; sequenceIterator != sequenceIteratorEnd; ++sequenceIterator)
	{
		Sequence& sequence = sequenceIterator->second;
		if (sequence.IsActive() && sequence.GetEntityId() == entityId)
		{
			sequence.Cancel();
		}
	}
}

void SequenceUpr::RequestActionStart(SequenceId sequenceId, TFlowNodeId actionNodeId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
	{
		DRX_ASSERT_MESSAGE(false, "Could not access sequence.");
		return;
	}

	sequence->RequestActionStart(actionNodeId);
}

void SequenceUpr::ActionCompleted(SequenceId sequenceId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
	{
		DRX_ASSERT_MESSAGE(false, "Could not access sequence.");
		return;
	}

	sequence->ActionComplete();
}

void SequenceUpr::SetBookmark(SequenceId sequenceId, TFlowNodeId bookmarkNodeId)
{
	Sequence* sequence = GetSequence(sequenceId);
	if (!sequence)
	{
		DRX_ASSERT_MESSAGE(false, "Could not access sequence.");
		return;
	}

	sequence->SetBookmark(bookmarkNodeId);
}

SequenceId SequenceUpr::GenerateUniqueSequenceId()
{
	return m_sequenceIdCounter++;
}

Sequence* SequenceUpr::GetSequence(SequenceId sequenceId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.find(sequenceId);
	if (sequenceIterator == m_registeredSequences.end())
	{
		return 0;
	}

	return &sequenceIterator->second;
}

void SequenceUpr::CancelActiveSequencesForThisEntity(EntityId entityId)
{
	SequenceMap::iterator sequenceIterator = m_registeredSequences.begin();
	SequenceMap::iterator sequenceIteratorEnd = m_registeredSequences.end();
	for (; sequenceIterator != sequenceIteratorEnd; ++sequenceIterator)
	{
		Sequence& sequence = sequenceIterator->second;
		if (sequence.IsActive() && sequence.GetEntityId() == entityId)
		{
			sequence.Cancel();
		}
	}
}

} //endns AIActionSequence
