// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/TemplateUtils_StringHashWrapper.h>
#include <drx3D/Schema2/TemplateUtils_TypeUtils.h>

namespace sxema2
{
	typedef TemplateUtils::CStringHashWrapper<TemplateUtils::CFastStringHash, TemplateUtils::CEmptyStringConversion, TemplateUtils::CRawPtrStringStorage> EnvTypeId;

	template <typename TYPE> inline EnvTypeId GetEnvTypeId()
	{
		static const EnvTypeId envTypeId(TemplateUtils::GetTypeName<TYPE>());
		return envTypeId;
	}
}
