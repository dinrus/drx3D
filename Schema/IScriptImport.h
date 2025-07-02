// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include #include  <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptImport : public IScriptElementBase<EScriptElementType::Import>
{
	virtual ~IScriptImport() {}

	virtual SGUID GetModuleGUID() const = 0;
};
} // sxema
