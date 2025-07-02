// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/MovementBlock.h>

namespace Movement
{
namespace MovementBlocks
{
class InstallPipeUserInCover : public Movement::Block
{
public:
	InstallPipeUserInCover(CPipeUser& pipeUser)
		: m_pipeUser(pipeUser)
	{}
	virtual void Begin(IMovementActor& actor)
	{
		if (m_pipeUser.GetCoverID() != 0)
		{
			m_pipeUser.SetCoverState(ICoverUser::EStateFlags::InCover);
			m_pipeUser.GetState().bodystate = STANCE_LOW_COVER;
		}
	}
	virtual tukk GetName() const { return "InstallInCover"; }
private:
	CPipeUser& m_pipeUser;
};
}
}

