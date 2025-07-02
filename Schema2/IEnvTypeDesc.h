// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Should type descriptor also store namespace, file-name and author?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TypeInfo.h>

namespace sxema2
{
	struct IAny;

	DECLARE_SHARED_POINTERS(IAny)

	class CTypeInfo;

	enum class EEnvTypeCategory // #SchematycTODO : Deprecate, we can use CTypeInfo to get this information.
	{
		Unknown = 0,
		Fundamental,
		Enumeration,
		Class
	};

	enum class EEnvTypeFlags
	{
		None       = 0,
		Switchable = BIT(0)
	};

	DECLARE_ENUM_CLASS_FLAGS(EEnvTypeFlags)

	struct IEnvTypeDesc
	{
		virtual ~IEnvTypeDesc() {}

		virtual EnvTypeId GetEnvTypeId() const = 0; // #SchematycTODO : Do we need this now that we have GetTypeInfo()?
		virtual SGUID GetGUID() const = 0;
		virtual tukk GetName() const = 0;
		virtual tukk GetDescription() const = 0;
		virtual CTypeInfo GetTypeInfo() const = 0;
		virtual EEnvTypeCategory GetCategory() const = 0;
		virtual EEnvTypeFlags GetFlags() const = 0;
		virtual IAnyPtr Create() const = 0;
	};

	DECLARE_SHARED_POINTERS(IEnvTypeDesc)
}
