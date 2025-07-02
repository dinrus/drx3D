
// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Enum.h>

namespace yasli {

class Archive;

struct BitFlagsWrapper{
	i32* variable;
	const EnumDescription* description;

	void YASLI_SERIALIZE_METHOD(Archive& ar);
};

template<class Enum>
BitFlagsWrapper BitFlags(Enum& value)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = (i32*)&value;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

template<class Enum>
BitFlagsWrapper BitFlags(i32& value)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = &value;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

template<class Enum>
BitFlagsWrapper BitFlags(u32& value)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = (i32*)&value;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

}
