// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptClass : public IScriptElementBase<EScriptElementType::Class>
{
	virtual ~IScriptClass() {}

	virtual tukk GetAuthor() const = 0;
	virtual tukk GetDescription() const = 0;
};
} // sxema
