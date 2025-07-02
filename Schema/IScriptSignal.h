// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

struct IScriptSignal : public IScriptElementBase<EScriptElementType::Signal>
{
	virtual ~IScriptSignal() {}

	virtual tukk  GetAuthor() const = 0;
	virtual tukk  GetDescription() const = 0;
	virtual u32       GetInputCount() const = 0;
	virtual DrxGUID        GetInputGUID(u32 inputIdx) const = 0;
	virtual tukk  GetInputName(u32 inputIdx) const = 0;
	virtual SElementId   GetInputTypeId(u32 inputIdx) const = 0;
	virtual CAnyConstPtr GetInputData(u32 inputIdx) const = 0;
};
} // sxema
