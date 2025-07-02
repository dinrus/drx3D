// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef MovementSystemCreator_h
	#define MovementSystemCreator_h

// The creator is used as a middle step, so that the code that wants
// to create the movement system doesn't necessarily have to include
// the movement system header and be recompiled every time it changes.
class MovementSystemCreator
{
public:
	struct IMovementSystem* CreateMovementSystem() const;
};

#endif // MovementSystemCreator_h
