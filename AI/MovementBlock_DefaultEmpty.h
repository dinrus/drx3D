// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementBlock_UnsupportedCustomNavigationType_h
	#define MovementBlock_UnsupportedCustomNavigationType_h

	#include <drx3D/AI/MovementPlan.h>
	#include <drx3D/AI/MovementStyle.h>

namespace Movement
{
namespace MovementBlocks
{
class DefaultEmpty : public Movement::Block
{
public:
	DefaultEmpty() {}

	// Movement::Block
	virtual Movement::Block::Status Update(const MovementUpdateContext& context) override
	{
		AIError("Trying to use the DefaultEmpty block, an undefined implementation of a movement block.");
		return CantBeFinished;
	};
	virtual tukk GetName() const override { return "DefaultEmpty"; }
	// ~Movement::Block
};
}
}

#endif // MovementBlock_UnsupportedCustomNavigationType_h
