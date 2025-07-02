// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/AI/StdAfx.h>
#include <drx3D/AI/MovementSystemCreator.h>
#include <drx3D/AI/MovementSystem.h>

IMovementSystem* MovementSystemCreator::CreateMovementSystem() const
{
	return new MovementSystem();
}
