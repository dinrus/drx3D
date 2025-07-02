// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/Signal.h>

namespace sxema
{
// Forward declare interfaces.
struct IRuntimeClass;

typedef CSignal<void (const IRuntimeClass&)> ClassCompilationSignal;

struct ICompiler
{
	virtual ~ICompiler() {}

	virtual void                           CompileAll() = 0;
	virtual void                           CompileDependencies(const DrxGUID& guid) = 0;
	virtual ClassCompilationSignal::Slots& GetClassCompilationSignalSlots() = 0;
};
} // sxema
