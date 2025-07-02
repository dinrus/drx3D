// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IScriptElement.h>

namespace sxema
{

// Forward declare classes.
class CClassProperties;

struct IScriptActionInstance : public IScriptElementBase<EScriptElementType::ActionInstance>
{
	virtual ~IScriptActionInstance() {}

	virtual DrxGUID                   GetActionTypeGUID() const = 0;
	virtual DrxGUID                   GetComponentInstanceGUID() const = 0;
	virtual const CClassProperties& GetProperties() const = 0;
};

} // sxema
