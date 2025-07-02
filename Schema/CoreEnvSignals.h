// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IEnvRegistrar;

struct SStartSignal
{
	static void ReflectType(CTypeDesc<SStartSignal>& desc);
};

struct SStopSignal
{
	static void ReflectType(CTypeDesc<SStopSignal>& desc);
};

struct SUpdateSignal
{
	SUpdateSignal(float _time = 0.0f);

	static void ReflectType(CTypeDesc<SUpdateSignal>& desc);

	float time = 0.0f;
};

void RegisterCoreEnvSignals(IEnvRegistrar& registrar);

}
