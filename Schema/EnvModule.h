// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/IEnvModule.h>

#define SXEMA_MAKE_ENV_MODULE(guid, name) sxema::EnvModule::MakeShared(guid, name, SXEMA_SOURCE_FILE_INFO)

namespace sxema
{

class CEnvModule : public CEnvElementBase<IEnvModule>
{
public:

	inline CEnvModule(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
		: CEnvElementBase(guid, szName, sourceFileInfo)
	{}

	// IEnvElement

	virtual bool IsValidScope(IEnvElement& scope) const override
	{
		switch (scope.GetType())
		{
		case EEnvElementType::Root:
		case EEnvElementType::Module:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}

	// ~IEnvElement
};

namespace EnvModule
{

inline std::shared_ptr<CEnvModule> MakeShared(const DrxGUID& guid, tukk szName, const SSourceFileInfo& sourceFileInfo)
{
	return std::make_shared<CEnvModule>(guid, szName, sourceFileInfo);
}

} // EnvModule
} // sxema
