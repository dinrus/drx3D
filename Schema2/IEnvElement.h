// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/GUID.h>

namespace sxema2
{
	enum class EEnvElementType
	{
		Function
	};

	struct IEnvElement
	{
		virtual ~IEnvElement() {}

		virtual EEnvElementType GetElementType() const = 0;
		virtual SGUID GetGUID() const = 0;
	};
}
