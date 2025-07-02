// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{
// Forward declare structures.
typedef std::function<void (const DrxGUID&)> ScriptDependencyEnumerator;

enum class EScriptDependencyType
{
	Load,
	Compile,
	Event,
	Reference
};
} // sxema
