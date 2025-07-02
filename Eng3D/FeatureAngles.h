// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "ParticleCommon.h"

namespace pfx2
{

SERIALIZATION_DECLARE_ENUM(EAlignParticleAxis,
	Forward,
	Normal
	)

SERIALIZATION_DECLARE_ENUM(EAlignType,
	Screen,
	Camera,
	Velocity,
	Parent,
	World
	)

SERIALIZATION_DECLARE_ENUM(EAlignView,
	None,
	Screen,
	Camera
	)

ILINE Vec3 GetParticleAxis(EAlignParticleAxis particleAxis, Quat orientation);
ILINE Vec3 GetParticleOtherAxis(EAlignParticleAxis particleAxis, Quat orientation);

}

#include "FeatureAnglesImpl.h"
