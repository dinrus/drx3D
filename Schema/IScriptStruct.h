// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare classes.
class CAnyConstPtr;

struct IScriptStruct : public IScriptElementBase<EScriptElementType::Struct>
{
	virtual ~IScriptStruct() {}

	virtual u32       GetFieldCount() const = 0;
	virtual tukk  GetFieldName(u32 fieldIdx) const = 0;
	virtual CAnyConstPtr GetFieldValue(u32 fieldIdx) const = 0;
};
} // sxema
