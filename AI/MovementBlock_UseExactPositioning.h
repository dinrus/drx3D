// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementBlock_UseExactPositioning_h
	#define MovementBlock_UseExactPositioning_h

	#include <drx3D/AI/MovementBlock_UseExactPositioningBase.h>

namespace Movement
{
namespace MovementBlocks
{
// This block has two responsibilities but the logic was so closely
// mapped that I decided to combine them into one.
//
// The two parts are 'Prepare' and 'Traverse'.
//
// If the exact positioning fails to position the character it reports the
// error through the bubbles and teleports to the position, and then
// proceeds with the plan.

class UseExactPositioning : public UseExactPositioningBase
{
public:
	typedef UseExactPositioningBase Base;

	UseExactPositioning(const CNavPath& path, const MovementStyle& style);
	virtual tukk GetName() const override          { return "UseExactPositioning"; }
	virtual bool        InterruptibleNow() const override { return true; }

private:
	virtual UseExactPositioningBase::TryRequestingExactPositioningResult TryRequestingExactPositioning(const MovementUpdateContext& context) override;
	virtual void                                                         HandleExactPositioningError(const MovementUpdateContext& context) override;
};
}
}

#endif // MovementBlock_UseExactPositioning_h
