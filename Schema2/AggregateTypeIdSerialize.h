// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Schema2/AggregateTypeId.h>

namespace sxema2
{
	bool Serialize(Serialization::IArchive& archive, CAggregateTypeId& value, tukk szName, tukk szLabel);
	bool PatchAggregateTypeIdFromDocVariableTypeInfo(Serialization::IArchive& archive, CAggregateTypeId& value, tukk szName);
}
