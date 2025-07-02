// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/Enum.h>

namespace Serialization {

struct BitFlagsWrapper
{
	i32*                                  variable;
	u32                          visibleMask;
	const Serialization::EnumDescription* description;

	void                                  Serialize(IArchive& ar);
};

template<class Enum>
BitFlagsWrapper BitFlags(Enum& value)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = (i32*)&value;
	wrapper.visibleMask = ~0U;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

template<class Enum>
BitFlagsWrapper BitFlags(i32& value, i32 visibleMask = ~0)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = &value;
	wrapper.visibleMask = visibleMask;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

template<class Enum>
BitFlagsWrapper BitFlags(u32& value, u32 visibleMask = ~0)
{
	BitFlagsWrapper wrapper;
	wrapper.variable = (i32*)&value;
	wrapper.visibleMask = visibleMask;
	wrapper.description = &getEnumDescription<Enum>();
	return wrapper;
}

}

#include "BitFlagsImpl.h"
