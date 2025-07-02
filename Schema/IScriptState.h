// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptState : public IScriptElementBase<EScriptElementType::State>
{
	virtual ~IScriptState() {}

	virtual DrxGUID GetPartnerGUID() const = 0;
};
} // sxema
