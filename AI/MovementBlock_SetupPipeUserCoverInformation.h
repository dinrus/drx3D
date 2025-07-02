// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/MovementBlock.h>

namespace Movement
{
namespace MovementBlocks
{
class SetupPipeUserCoverInformation : public Movement::Block
{
public:
	SetupPipeUserCoverInformation(CPipeUser& pipeUser)
		: m_pipeUser(pipeUser)
	{}
	virtual void        Begin(IMovementActor& actor);
	virtual tukk GetName() const { return "SetCoverInfo"; }
private:
	CPipeUser& m_pipeUser;
};
}
}
