// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Should we be using guids to identify settings?

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/Delegate.h>

namespace sxema
{
struct ISettings
{
	virtual ~ISettings() {}

	virtual void Serialize(Serialization::IArchive& archive) = 0;
};

DECLARE_SHARED_POINTERS(ISettings)

typedef std::function<EVisitStatus (tukk , const ISettingsPtr&)> SettingsVisitor;

struct ISettingsUpr
{
	virtual ~ISettingsUpr() {}

	virtual bool       RegisterSettings(tukk szName, const ISettingsPtr& pSettings) = 0;
	virtual ISettings* GetSettings(tukk szName) const = 0;
	virtual void       VisitSettings(const SettingsVisitor& visitor) const = 0;
	virtual void       LoadAllSettings() = 0;
	virtual void       SaveAllSettings() = 0;
};
} // sxema
