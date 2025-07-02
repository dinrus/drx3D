// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/GUID.h>

namespace sxema
{
// Forward declare interfaces.
struct IRuntimeClass;

// Forward declare shared pointers.
DECLARE_SHARED_POINTERS(IRuntimeClass)

struct IRuntimeRegistry
{
	virtual ~IRuntimeRegistry() {}

	virtual IRuntimeClassConstPtr GetClass(const DrxGUID& guid) const = 0;
};
} // sxema
