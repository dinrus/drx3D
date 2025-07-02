// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

struct IScriptInterfaceFunction : public IScriptElementBase<EScriptElementType::InterfaceFunction>
{
	virtual ~IScriptInterfaceFunction() {}

	virtual tukk  GetAuthor() const = 0;
	virtual tukk  GetDescription() const = 0;
	virtual u32       GetInputCount() const = 0;
	virtual tukk  GetInputName(u32 inputIdx) const = 0;
	virtual CAnyConstPtr GetInputValue(u32 inputIdx) const = 0;
	virtual u32       GetOutputCount() const = 0;
	virtual tukk  GetOutputName(u32 outputIdx) const = 0;
	virtual CAnyConstPtr GetOutputValue(u32 outputIdx) const = 0;
};
} // sxema
