// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IAIActionSequence.h
//  Version:     v1.00
//  Created:     2014-04-23 by Christian Werle.
//  Описание: This is the public interface for AI Action Sequence system.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _IAIActionSequence_h_
#define _IAIActionSequence_h_

#include <drx3D/FlowGraph/IFlowSystem.h> // for TFlowNodeId typedef and IFlowGraph

namespace AIActionSequence
{

typedef u32 SequenceId;

//////////////////////////////////////////////////////////////////////////

enum SequenceEvent
{
	StartSequence,
	StartAction,
	BookmarkSet,
	TriggerBookmark,
	SequenceStopped,
	SequenceEventCount
};

//////////////////////////////////////////////////////////////////////////

class SequenceActionBase
{
public:
	SequenceActionBase()
		: m_assignedSequenceId(0)
		, m_assignedEntityId(0)
	{}

	virtual void HandleSequenceEvent(SequenceEvent event) {}

	void         AssignSequence(SequenceId sequenceId, EntityId entityId)
	{
		m_assignedSequenceId = sequenceId;
		m_assignedEntityId = entityId;
	}

	void ClearSequence()
	{
		m_assignedSequenceId = 0;
		m_assignedEntityId = 0;
	}

	SequenceId GetAssignedSequenceId() const
	{
		return m_assignedSequenceId;
	}

	EntityId GetAssignedEntityId() const
	{
		return m_assignedEntityId;
	}

private:
	SequenceId m_assignedSequenceId;
	EntityId   m_assignedEntityId;
};

//////////////////////////////////////////////////////////////////////////

struct SequenceProperties
{
	SequenceProperties() {}
	SequenceProperties(bool _interruptible, bool _resumeAfterInterrupt)
		: interruptible(_interruptible)
		, resumeAfterInterrupt(_resumeAfterInterrupt)
	{}

	bool interruptible;
	bool resumeAfterInterrupt;
};

//////////////////////////////////////////////////////////////////////////

struct ISequenceUpr
{
	virtual ~ISequenceUpr() {}

	virtual bool RegisterSequence(EntityId entityId, TFlowNodeId startNodeId, SequenceProperties sequenceProperties, IFlowGraph* flowGraph) = 0;
	virtual void UnregisterSequence(SequenceId sequenceId) = 0;

	virtual void StartSequence(SequenceId sequenceId) = 0;
	virtual void CancelSequence(SequenceId sequenceId) = 0;
	virtual bool IsSequenceActive(SequenceId sequenceId) = 0;

	virtual void SequenceBehaviorReady(EntityId entityId) = 0;
	virtual void SequenceInterruptibleBehaviorLeft(EntityId entityId) = 0;
	virtual void SequenceNonInterruptibleBehaviorLeft(EntityId entityId) = 0;
	virtual void AgentDisabled(EntityId entityId) = 0;

	virtual void RequestActionStart(SequenceId sequenceId, TFlowNodeId actionNodeId) = 0;
	virtual void ActionCompleted(SequenceId sequenceId) = 0;
	virtual void SetBookmark(SequenceId sequenceId, TFlowNodeId bookmarkNodeId) = 0;
};

} //endns AIActionSequence

#endif // _IAIActionSequence_h_
