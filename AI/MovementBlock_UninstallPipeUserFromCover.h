// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/MovementBlock.h>
#include <drx3D/AI/IMovementActor.h>

namespace Movement
{
namespace MovementBlocks
{
class UninstallPipeUserFromCover : public Movement::Block
{
public:
	UninstallPipeUserFromCover(CPipeUser& pipeUser, const MovementStyle::Stance stance)
		: m_pipeUser(pipeUser)
		, m_stance(stance)
	{
	}

	virtual void Begin(IMovementActor& actor)
	{
		m_pipeUser.SetCoverState(ICoverUser::EStateFlags::None);
		actor.GetAdapter().SetStance(m_stance);
	}

	virtual tukk GetName() const { return "UninstallFromCover"; }
private:
	CPipeUser& m_pipeUser;
	MovementStyle::Stance m_stance;
};
}
}
