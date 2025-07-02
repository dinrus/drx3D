// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>

namespace sxema
{

struct IEnvModule : public IEnvElementBase<EEnvElementType::Module>
{
	virtual ~IEnvModule() {}
};

} // sxema
