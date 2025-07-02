// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

struct IScriptVariable : public IScriptElementBase<EScriptElementType::Variable>
{
	virtual ~IScriptVariable() {}

	virtual SElementId      GetTypeId() const = 0;
	virtual bool            IsArray() const = 0;
	virtual CAnyConstPtr    GetData() const = 0;
	virtual DrxGUID           GetBaseGUID() const = 0;
	virtual EOverridePolicy GetOverridePolicy() const = 0;
};
} // sxema
