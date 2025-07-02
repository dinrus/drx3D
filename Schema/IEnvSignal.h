// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>

namespace sxema
{

// Forward declare classes.
class CClassDesc;

struct IEnvSignal : public IEnvElementBase<EEnvElementType::Signal>
{
	virtual ~IEnvSignal() {}

	virtual const CClassDesc& GetDesc() const = 0;
};

} // sxema
