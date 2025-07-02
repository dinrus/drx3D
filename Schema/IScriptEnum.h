// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptEnum : public IScriptElementBase<EScriptElementType::Enum>
{
	virtual ~IScriptEnum() {}

	virtual u32      GetConstantCount() const = 0;
	virtual u32      FindConstant(tukk szConstant) const = 0;
	virtual tukk GetConstant(u32 constantIdx) const = 0;
};
} // sxema
