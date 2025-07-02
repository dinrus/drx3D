// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
struct IScriptInterfaceImpl : public IScriptElementBase<EScriptElementType::InterfaceImpl>
{
	virtual ~IScriptInterfaceImpl() {}

	virtual EDomain GetDomain() const = 0;
	virtual DrxGUID   GetRefGUID() const = 0;
};
} // sxema
