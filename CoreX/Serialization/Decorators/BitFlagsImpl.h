// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Decorators/BitFlags.h>

namespace Serialization {

inline void BitFlagsWrapper::Serialize(IArchive& ar)
{
	const yasli::EnumDescription& desc = *description;
	i32 count = desc.count();
	if (ar.isInput())
	{
		i32 previousValue = *variable;
		for (i32 i = 0; i < count; ++i)
		{
			i32 flagValue = desc.valueByIndex(i);
			if (!(flagValue & visibleMask))
				continue;
			bool flag = (previousValue & flagValue) == flagValue;
			bool previousFlag = flag;
			ar(flag, desc.nameByIndex(i), desc.labelByIndex(i));
			if (flag != previousFlag)
			{
				if (flag)
					*variable |= flagValue;
				else
					*variable &= ~flagValue;
			}
		}
	}
	else
	{
		for (i32 i = 0; i < count; ++i)
		{
			i32 flagValue = desc.valueByIndex(i);
			if (!(flagValue & visibleMask))
				continue;
			bool flag = (*variable & flagValue) == flagValue;
			ar(flag, desc.nameByIndex(i), desc.labelByIndex(i));
		}
	}
}

}
