// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IEnvRegistrar;

typedef std::function<void (IEnvRegistrar&)> EnvPackageCallback;

struct IEnvPackage
{
	virtual ~IEnvPackage() {}

	virtual DrxGUID              GetGUID() const = 0;
	virtual tukk        GetName() const = 0;
	virtual tukk        GetAuthor() const = 0;
	virtual tukk        GetDescription() const = 0;
	virtual EnvPackageCallback GetCallback() const = 0;
};

} // sxema
