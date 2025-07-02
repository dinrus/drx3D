// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>

namespace sxema
{

// Forward declare classes.
class CCommonTypeDesc;

struct IEnvDataType : public IEnvElementBase<EEnvElementType::DataType>
{
	virtual ~IEnvDataType() {}

	virtual const CCommonTypeDesc& GetDesc() const = 0;
};

} // sxema
