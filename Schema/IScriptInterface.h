// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptInterface : public IScriptElementBase<EScriptElementType::Interface>
{
	virtual ~IScriptInterface() {}

	virtual tukk GetAuthor() const = 0;
	virtual tukk GetDescription() const = 0;
};
} // sxema
