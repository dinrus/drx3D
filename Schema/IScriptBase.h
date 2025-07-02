// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{
// Forward declare structures.
struct SElementId;
// Forward declare classes.
class CAnyConstPtr;

struct IScriptBase : public IScriptElementBase<EScriptElementType::Base>
{
	virtual ~IScriptBase() {}

	virtual SElementId   GetClassId() const = 0;
	virtual CAnyConstPtr GetEnvClassProperties() const = 0;
};
}
