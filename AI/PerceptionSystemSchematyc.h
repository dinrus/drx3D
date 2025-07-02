// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IAuditionMap.h>

namespace Perception
{

inline void ReflectType(sxema::CTypeDesc<EStimulusObstructionHandling>& desc)
{
	desc.SetGUID("B741A80F-FBB3-40B2-BCAA-6757DA3B4434"_drx_guid);

	desc.AddConstant(EStimulusObstructionHandling::IgnoreAllObstructions, "IgnoreAllObstructions", "Ignore All Obstructions");
	desc.AddConstant(EStimulusObstructionHandling::RayCastWithLinearFallOff, "RayCastWithLinearFallOff", "Raycast with Linear Falloff");
}
}

namespace PerceptionSystemSchematyc
{
	void Register(sxema::IEnvRegistrar& registrar, sxema::CEnvRegistrationScope& parentScope);
}