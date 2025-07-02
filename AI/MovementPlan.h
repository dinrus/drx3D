// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementPlan_h
	#define MovementPlan_h

struct IMovementActor;
struct MovementUpdateContext;

	#include <drx3D/AI/MovementBlock.h>
	#include <drx3D/AI/MovementRequestID.h>

namespace Movement
{
// This is a plan that will satisfy a movement request such as
// "reach this destination" or "stop immediately".
//
// It's comprised of one or more movement blocks that each take care
// of a certain part of plan. They are executed in order.
//
// As an example a plan to satisfy the request "move to this cover" could
// be comprised of the following movement blocks:
// [FollowPath] -> [UseSmartObject] -> [FollowPath] -> [InstallInCover]
class Plan
{
public:
	static u32k NoBlockIndex = ~0u;
	
	enum Status
	{
		None,
		Running,
		Finished,
		CantBeFinished,
	};

	Plan()
		: m_current(NoBlockIndex)
		, m_lastStatus(Status::Running)
	{
	}

	template<typename BlockType>
	void AddBlock()
	{
		m_blocks.push_back(std::shared_ptr<Block>(new BlockType()));
	}

	void AddBlock(const std::shared_ptr<Block>& block)
	{
		m_blocks.push_back(block);
	}

	Status                   Execute(const MovementUpdateContext& context);
	void                     ChangeToIndex(const uint newIndex, IMovementActor& actor);
	bool                     HasBlocks() const { return !m_blocks.empty(); }
	void                     Clear(IMovementActor& actor);
	void                     CutOffAfterCurrentBlock();
	bool                     InterruptibleNow() const;
	u32                   GetCurrentBlockIndex() const { return m_current; }
	u32                   GetBlockCount() const        { return m_blocks.size(); }
	const Block*             GetBlock(u32 index) const;

	const MovementRequestID& GetRequestId() const                             { return m_requestId; }
	void                     SetRequestId(const MovementRequestID& requestId) { m_requestId = requestId; }

	Status                   GetLastStatus() const { return m_lastStatus; }

private:
	typedef std::vector<BlockPtr> Blocks;

	Blocks            m_blocks;
	u32            m_current;
	MovementRequestID m_requestId;
	Status            m_lastStatus;
};

}

#endif // MovementPlan_h
