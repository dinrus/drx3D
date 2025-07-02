// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare structures.
struct STimerParams;

struct IScriptTimer : public IScriptElementBase<EScriptElementType::Timer>
{
	virtual ~IScriptTimer() {}

	virtual tukk  GetAuthor() const = 0;
	virtual tukk  GetDescription() const = 0;
	virtual STimerParams GetParams() const = 0;
};
} // sxema
